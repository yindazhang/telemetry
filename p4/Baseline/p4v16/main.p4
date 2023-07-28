#include <core.p4>
#include <tna.p4>

#define ETHERTYPE_IPV4 0x0800

#define SWITCH_NUM 4

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

header time_h{
    bit<32> time;
}

struct header_t {
    ethernet_h ethernet;
    time_h time;
    ipv4_h ipv4;
}

/*================================
=            Metadata            =
================================*/


struct metadata_t {
    bit<32> diff_;
    bit<32> user_num_;
    bit<32> switch_;
}

/*========================================
=            Ingress parsing             =
========================================*/


parser EthIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md){

    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }
}


parser IngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md) {
    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(64);
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
        pkt.emit(hdr);
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
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.time);
        pkt.extract(hdr.ipv4);
        transition accept;
    }
}

parser EgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        pkt.extract(eg_intr_md);
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
        pkt.emit(hdr);
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
    }
    table ti_forward_user {
        key = {
            ig_intr_md.ingress_port : exact;
	        hdr.ipv4.dst_addr : ternary;
        }
        actions = {
            ai_forward_user;
            ai_drop;
        }
        default_action = ai_drop;
    }
    
    apply {
        if(hdr.ethernet.ether_type == ETHERTYPE_IPV4) {
            ti_forward_user.apply();
            ig_tm_md.qid = 0x0;
            hdr.time.setValid();
            hdr.time.time = ig_prsr_md.global_tstamp[31:0];
	    }
        else
            ig_dprsr_md.drop_ctl = 0x1;
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
    
    action action_switch_id_(bit<32> switch_id){
        md.switch_ = switch_id;
    }
    table table_switch_id_ {
        key = {
            eg_intr_md.egress_port : exact;
        }
        actions = {
            action_switch_id_;
        }
        default_action = action_switch_id_(0);
    }

    Register<bit<32>, _>(SWITCH_NUM) reg_user_num_;
    RegisterAction<bit<32>, _, bit<32>>(reg_user_num_) action_user_num_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
            val = val + 1;
        }
    };
    action action_user_nums_() {
        md.user_num_ = action_user_num_.execute(md.switch_);
    }
    table table_user_num_ {
        actions = { action_user_nums_; }
	    const default_action = action_user_nums_();
    }

    Register<bit<32>, _>(131072) reg_diff_order_;
    RegisterAction<bit<32>, _, bit<8>>(reg_diff_order_) action_diff_order_ = {
        void apply(inout bit<32> val) {
            val = md.diff_;
            //val = (bit<32>)eg_intr_md.enq_qdepth;
        }
    };
    action action_diff_orders_() {
        action_diff_order_.execute(md.user_num_);
    }
    table table_diff_order_ {
        actions = { action_diff_orders_; }
	    const default_action = action_diff_orders_();
    }

    apply {
        table_switch_id_.apply();
        md.diff_ = eg_prsr_md.global_tstamp[31:0] - hdr.time.time;
        
        if(hdr.ethernet.ether_type == ETHERTYPE_IPV4){
            table_user_num_.apply();
            hdr.time.setInvalid();
            table_diff_order_.apply();
	    }
        else
            eg_dprsr_md.drop_ctl = 0x1;
    }
}

/*==============================================
=            The switch's pipeline             =
==============================================*/
Pipeline(
    IngressParser(), Ingress(), IngressDeparser(),
    EgressParser(), Egress(), EgressDeparser()) pipe;

Switch(pipe) main;