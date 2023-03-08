#include <core.p4>
#include <tna.p4>

#define ETHERTYPE_IPV4 0x0800

#define ETHERTYPE_USER 0x0101
#define ETHERTYPE_SEED 0x0170
#define ETHERTYPE_PUSH 0x0171

#define IP_TCP 6
#define IP_UDP 17

#define TABLE_SIZE 65536

#define BUFFER_SIZE 1024

#define SWITCH_ID 17

/*==============================
=            Header            =
==============================*/

header ethernet_h {
    bit<48> dst_addr;
    bit<48> src_addr;
    bit<16> ether_type;
}

header ipv4_h {
    bit<4> version;
    bit<4> ihl;
    bit<8> tos;
    bit<16> total_len;
    bit<16> identification;
    bit<3> flags;
    bit<13> frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdr_checksum;
    bit<32> src_addr;
    bit<32> dst_addr;
}

header port_h{
    bit<32> port;
}

header entry_h{
    bit<16> ttl_protocol;
    bit<32> nodeId;
    bit<32> port;
    bit<32> src_addr;
    bit<32> dst_addr;
}

struct header_t {
    ethernet_h ethernet;
    ipv4_h ipv4;
    port_h port;
    entry_h batches;
}

/*================================
=            Metadata            =
================================*/

struct entry_t{
    bit<16> ttl_protocol;
    bit<32> nodeId;
    bit<32> port;
    bit<32> src_addr;
    bit<32> dst_addr;
}

header ow_h {
    bit<8> evict_;
    bit<16> index_;

    bit<16> type_; 
    bit<16> hash_;

    entry_t entry_;
    entry_t old_entry_;
}

struct metadata_t {
    ow_h ow_md;
}

/*========================================
=            Ingress parsing             =
========================================*/

parser EthIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md){

    ParserPriority() parser_prio;
    
    state start {
        pkt.extract(ig_intr_md);
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_SEED : parse_seed;
            ETHERTYPE_PUSH : parse_push;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol){
            IP_TCP: parse_port;
            IP_UDP: parse_port;
            default: accept;
        }
    }
    state parse_port {
        pkt.extract(hdr.port);
        md.ow_md.type_ = ETHERTYPE_USER;
        transition accept;
    }
    state parse_seed {
        parser_prio.set(7);
        md.ow_md.type_ = ETHERTYPE_SEED;
        transition accept;
    }
    state parse_push {
        parser_prio.set(6);
        pkt.extract(hdr.batches);
        md.ow_md.type_ = ETHERTYPE_PUSH;
        transition accept;
    }
}

parser IngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md) {
    state start {
        md.ow_md.type_ = 0;
        EthIngressParser.apply(pkt, hdr, md, ig_intr_md);
        transition accept;
    }
}

control IngressDeparser(
        packet_out pkt, 
        inout header_t hdr,
        in metadata_t md,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.port);
        pkt.emit(hdr.batches);
    }
}

/*========================================
=            Egress parsing             =
========================================*/

parser EthEgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out egress_intrinsic_metadata_t eg_intr_md){
    
    state start {
        pkt.extract(eg_intr_md);
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_SEED : parse_seed;
            ETHERTYPE_PUSH : parse_push;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol){
            IP_TCP: parse_port;
            IP_UDP: parse_port;
            default: accept;
        }
    }
    state parse_port {
        pkt.extract(hdr.port);
        md.ow_md.type_ = ETHERTYPE_USER;
        transition accept;
    }
    state parse_seed {
        md.ow_md.type_ = ETHERTYPE_SEED;
        transition accept;
    }
    state parse_push {
        pkt.extract(hdr.batches);
        md.ow_md.type_ = ETHERTYPE_PUSH;
        transition accept;
    }
}

parser EgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        EthEgressParser.apply(pkt, hdr, md, eg_intr_md);
        transition accept;
    }
}

control EgressDeparser(
        packet_out pkt, 
        inout header_t hdr,
        in metadata_t md,
        in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.port);
        pkt.emit(hdr.batches);
    }
}

/*===========================================
=            Ingress match-action           =
===========================================*/

control Ingress(
        inout header_t hdr, 
        inout metadata_t md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    action ai_drop() {
        ig_dprsr_md.drop_ctl = 0x1;
    }
    action ai_forward_user(bit<9> egress_port){
        ig_tm_md.ucast_egress_port = egress_port;
        ig_tm_md.qid = 0x0;
    }
    table ti_forward_user {
        key = {
	        hdr.ipv4.dst_addr : ternary;
        }
        actions = {
            ai_forward_user;
            ai_drop;
        }
        default_action = ai_forward_user(0x4);
    }

    Hash<bit<16>>(HashAlgorithm_t.CRC16) hash_index;

    action compute_hash_index_() {
        md.ow_md.hash_ = hash_index.get(
            {
                md.ow_md.entry_.ttl_protocol,
                md.ow_md.entry_.nodeId,
                md.ow_md.entry_.port,
                md.ow_md.entry_.src_addr,
                md.ow_md.entry_.dst_addr
            }
	    );
    }
    table table_hash_index_ {
        actions = {compute_hash_index_;}
	    default_action = compute_hash_index_();
    }

    apply {
        md.ow_md.evict_ = 0;
        if(md.ow_md.type_ == ETHERTYPE_USER) {
            md.ow_md.entry_.nodeId = SWITCH_ID;
            md.ow_md.entry_.port = hdr.port.port;
            md.ow_md.entry_.src_addr = hdr.ipv4.src_addr;
            md.ow_md.entry_.dst_addr = hdr.ipv4.dst_addr;
            md.ow_md.entry_.ttl_protocol = (bit<16>)hdr.ipv4.ttl;
            md.ow_md.entry_.ttl_protocol = (md.ow_md.entry_.ttl_protocol << 8);
            md.ow_md.entry_.ttl_protocol = (md.ow_md.entry_.ttl_protocol | ((bit<16>)hdr.ipv4.protocol));

            table_hash_index_.apply();
	        ti_forward_user.apply();
	    } 
        else if(md.ow_md.type_ == ETHERTYPE_PUSH) {
            md.ow_md.evict_ = 1;
            md.ow_md.entry_.nodeId = hdr.batches.nodeId;
            md.ow_md.entry_.port = hdr.batches.port;
            md.ow_md.entry_.src_addr = hdr.batches.src_addr;
            md.ow_md.entry_.dst_addr = hdr.batches.dst_addr;
            md.ow_md.entry_.ttl_protocol = hdr.batches.ttl_protocol;

            // Forward to recirculate port?
            ig_tm_md.ucast_egress_port = 15;
            ig_tm_md.qid = 0x6;
	    }
        else if(md.ow_md.type_ == ETHERTYPE_SEED) {
            // Multicast to forward port?
	        ig_tm_md.mcast_grp_a = 15;
	        ig_tm_md.qid = 0x7;
	    }
    }
}

/*===========================================
=            Egress match-action           =
===========================================*/

control Egress(
        inout header_t hdr, 
        inout metadata_t md,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_oport_md){

    
    Register<bit<32>, _>(TABLE_SIZE) reg_srcip_;
    RegisterAction<bit<32>, _, bit<32>>(reg_srcip_) action_reg_srcip_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val; 
            val = md.ow_md.entry_.src_addr;
        }
    };
    action action_srcip_() {
        md.ow_md.old_entry_.src_addr = action_reg_srcip_.execute(md.ow_md.hash_);
    }
    table table_srcip_ {
        actions = { action_srcip_; }
	    const default_action = action_srcip_();
    }

    Register<bit<32>, _>(TABLE_SIZE) reg_dstip_;
    RegisterAction<bit<32>, _, bit<32>>(reg_dstip_) action_reg_dstip_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val; 
            val = md.ow_md.entry_.dst_addr;
        }
    };
    action action_dstip_() {
        md.ow_md.old_entry_.dst_addr = action_reg_dstip_.execute(md.ow_md.hash_);
    }
    table table_dstip_ {
        actions = { action_dstip_; }
	    const default_action = action_dstip_();
    }

    Register<bit<32>, _>(TABLE_SIZE) reg_port_;
    RegisterAction<bit<32>, _, bit<32>>(reg_port_) action_reg_port_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val; 
            val = md.ow_md.entry_.port;
        }
    };
    action action_port_() {
        md.ow_md.old_entry_.port = action_reg_port_.execute(md.ow_md.hash_);
    }
    table table_port_ {
        actions = { action_port_; }
	    const default_action = action_port_();
    }

    Register<bit<32>, _>(TABLE_SIZE) reg_nodeId_;
    RegisterAction<bit<32>, _, bit<32>>(reg_nodeId_) action_reg_nodeId_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val; 
            val = md.ow_md.entry_.nodeId;
        }
    };
    action action_nodeId_() {
        md.ow_md.old_entry_.nodeId = action_reg_nodeId_.execute(md.ow_md.hash_);
    }
    table table_nodeId_ {
        actions = { action_nodeId_; }
	    const default_action = action_nodeId_();
    }

    Register<bit<16>, _>(TABLE_SIZE) reg_ttl_protocol_;
    RegisterAction<bit<16>, _, bit<16>>(reg_ttl_protocol_) action_reg_ttl_protocol_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val; 
            val = md.ow_md.entry_.ttl_protocol;
        }
    };
    action action_ttl_protocol_() {
        md.ow_md.old_entry_.ttl_protocol = action_reg_ttl_protocol_.execute(md.ow_md.hash_);
    }
    table table_ttl_protocol_ {
        actions = { action_ttl_protocol_; }
	    const default_action = action_ttl_protocol_();
    }
    

    Register<bit<16>, _>(1, 0) buffer_index_;

    RegisterAction<bit<16>, _, bit<16>>(buffer_index_) action_buffer_index_push_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val;
            if(val < BUFFER_SIZE)
                val = val + 1;
        }
    };
    action action_buf_push_() {
        md.ow_md.index_ = action_buffer_index_push_.execute(0);
    }
    table table_buf_push_ {
        actions = { action_buf_push_; }
	    const default_action = action_buf_push_();
    }

    RegisterAction<bit<16>, _, bit<16>>(buffer_index_) action_buffer_index_pop_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val - 1;
            if(val > 0)
                val = val - 1;
        }
    };
    action action_buf_pop_() {
        md.ow_md.index_ = action_buffer_index_pop_.execute(0);
    }
    table table_buf_pop_ {
        actions = { action_buf_pop_; }
	    const default_action = action_buf_pop_();
    }

    Register<bit<32>, _>(BUFFER_SIZE) buf_srcip_;
    RegisterAction<bit<32>, _, bit<32>>(buf_srcip_) action_buffer_push_srcip_ = {
        void apply(inout bit<32> val) {
            val = md.ow_md.entry_.src_addr;
        }
    };
    action action_buf_push_srcip_() {
        action_buffer_push_srcip_.execute(md.ow_md.index_);
    }
    table table_buf_push_srcip_ {
        actions = { action_buf_push_srcip_; }
	    const default_action = action_buf_push_srcip_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_srcip_) action_buffer_pop_srcip_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_srcip_() {
        hdr.batches.src_addr = 
            action_buffer_pop_srcip_.execute(md.ow_md.index_);
    }
    table table_buf_pop_srcip_ {
        actions = { action_buf_pop_srcip_; }
	    const default_action = action_buf_pop_srcip_();
    }

    Register<bit<32>, _>(BUFFER_SIZE) buf_dstip_;
    RegisterAction<bit<32>, _, bit<32>>(buf_dstip_) action_buffer_push_dstip_ = {
        void apply(inout bit<32> val) {
            val = md.ow_md.entry_.dst_addr;
        }
    };
    action action_buf_push_dstip_() {
        action_buffer_push_dstip_.execute(md.ow_md.index_);
    }
    table table_buf_push_dstip_ {
        actions = { action_buf_push_dstip_; }
	    const default_action = action_buf_push_dstip_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_dstip_) action_buffer_pop_dstip_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_dstip_() {
        hdr.batches.dst_addr = 
            action_buffer_pop_dstip_.execute(md.ow_md.index_);
    }
    table table_buf_pop_dstip_ {
        actions = { action_buf_pop_dstip_; }
	    const default_action = action_buf_pop_dstip_();
    }

    Register<bit<32>, _>(BUFFER_SIZE) buf_port_;
    RegisterAction<bit<32>, _, bit<32>>(buf_port_) action_buffer_push_port_ = {
        void apply(inout bit<32> val) {
            val = md.ow_md.entry_.port;
        }
    };
    action action_buf_push_port_() {
        action_buffer_push_port_.execute(md.ow_md.index_);
    }
    table table_buf_push_port_ {
        actions = { action_buf_push_port_; }
	    const default_action = action_buf_push_port_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_port_) action_buffer_pop_port_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_port_() {
        hdr.batches.port = 
            action_buffer_pop_port_.execute(md.ow_md.index_);
    }
    table table_buf_pop_port_ {
        actions = { action_buf_pop_port_; }
	    const default_action = action_buf_pop_port_();
    }

    Register<bit<32>, _>(BUFFER_SIZE) buf_nodeId_;
    RegisterAction<bit<32>, _, bit<32>>(buf_nodeId_) action_buffer_push_nodeId_ = {
        void apply(inout bit<32> val) {
            val = md.ow_md.entry_.nodeId;
        }
    };
    action action_buf_push_nodeId_() {
        action_buffer_push_nodeId_.execute(md.ow_md.index_);
    }
    table table_buf_push_nodeId_ {
        actions = { action_buf_push_nodeId_; }
	    const default_action = action_buf_push_nodeId_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_nodeId_) action_buffer_pop_nodeId_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_nodeId_() {
        hdr.batches.nodeId = 
            action_buffer_pop_nodeId_.execute(md.ow_md.index_);
    }
    table table_buf_pop_nodeId_ {
        actions = { action_buf_pop_nodeId_; }
	    const default_action = action_buf_pop_nodeId_();
    }

    Register<bit<16>, _>(BUFFER_SIZE) buf_ttl_protocol_;
    RegisterAction<bit<16>, _, bit<16>>(buf_ttl_protocol_) action_buffer_push_ttl_protocol_ = {
        void apply(inout bit<16> val) {
            val = md.ow_md.entry_.ttl_protocol;
        }
    };
    action action_buf_push_ttl_protocol_() {
        action_buffer_push_ttl_protocol_.execute(md.ow_md.index_);
    }
    table table_buf_push_ttl_protocol_ {
        actions = { action_buf_push_ttl_protocol_; }
	    const default_action = action_buf_push_ttl_protocol_();
    }
    RegisterAction<bit<16>, _, bit<16>>(buf_ttl_protocol_) action_buffer_pop_ttl_protocol_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val;
        }
    };
    action action_buf_pop_ttl_protocol_() {
        hdr.batches.ttl_protocol = 
            action_buffer_pop_ttl_protocol_.execute(md.ow_md.index_);
    }
    table table_buf_pop_ttl_protocol_ {
        actions = { action_buf_pop_ttl_protocol_; }
	    const default_action = action_buf_pop_ttl_protocol_();
    }

    apply {
        if(md.ow_md.type_ == ETHERTYPE_USER) {
            table_port_.apply();
            table_srcip_.apply();
            table_dstip_.apply();
            table_nodeId_.apply();
            table_ttl_protocol_.apply();

            if(md.ow_md.entry_.port != md.ow_md.old_entry_.port)
                md.ow_md.evict_ = 1;
            else if(md.ow_md.entry_.src_addr != md.ow_md.old_entry_.src_addr)
                md.ow_md.evict_ = 1;
            else if(md.ow_md.entry_.dst_addr != md.ow_md.old_entry_.dst_addr)
                md.ow_md.evict_ = 1;
            else if(md.ow_md.entry_.nodeId != md.ow_md.old_entry_.nodeId)
                md.ow_md.evict_ = 1;
            else if(md.ow_md.entry_.ttl_protocol != md.ow_md.old_entry_.ttl_protocol)
                md.ow_md.evict_ = 1;
	    }
        else if(md.ow_md.type_ == ETHERTYPE_PUSH) {
            eg_dprsr_md.drop_ctl = 0x1;
	    }

        if(md.ow_md.evict_ == 1) {
            table_buf_push_.apply();
            if(md.ow_md.index_ < BUFFER_SIZE){
                table_buf_push_srcip_.apply();
                table_buf_push_dstip_.apply();
                table_buf_push_port_.apply();
                table_buf_push_nodeId_.apply();
                table_buf_push_ttl_protocol_.apply();
            }
        }
        else if(md.ow_md.type_ == ETHERTYPE_SEED) {
            table_buf_pop_.apply();
            if(md.ow_md.index_ >= 0){
                hdr.ethernet.ether_type = ETHERTYPE_PUSH;
                hdr.batches.setValid();
                table_buf_pop_srcip_.apply();
                table_buf_pop_dstip_.apply();
                table_buf_pop_port_.apply();
                table_buf_pop_nodeId_.apply();
                table_buf_pop_ttl_protocol_.apply();
            }
            else{
                eg_dprsr_md.drop_ctl = 0x1;
            }
	    }
    }
}

/*==============================================
=            The switch's pipeline             =
==============================================*/
Pipeline(
    IngressParser(), Ingress(), IngressDeparser(),
    EgressParser(), Egress(), EgressDeparser()) pipe;

Switch(pipe) main;