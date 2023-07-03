import argparse
import json
import sys, os, time

sys.path.append(os.path.dirname(os.path.realpath(__file__))+"/libs")
from mgr import *


def config():
    print("PID: " + str(os.getpid())) 
    m = Manager(p4name="main")

    mc_pipe0_ports = [0<<7|i for i in range(0, 61, 4)]
    mc_pipe1_ports = [1<<7|i for i in range(0, 61, 4)]

    data = {}
    with open('libs/servers.json', 'r') as f:
        servers_json=json.loads(f.read())
    server_ports = []
    for val in servers_json.values():
        server_ports.append(val['port_id'])

    # 1/0 <-> 2/0
    loopback_ports = [128, 136]
    redyellow_ports = [172, 164]

    print("--- Set up ports (physically occupied for successful enabling) ---")
    m.enab_ports_sym_bw(mc_pipe0_ports, "100G")
    m.enab_ports_sym_bw(server_ports, "25G")
    m.enab_ports_sym_bw(loopback_ports, "25G")
    m.enab_ports_sym_bw_b(redyellow_ports, "100G")

    print("--- Configure DP states ---")
    
    for server in servers_json.values():
        print(server)
        m.add_rule_ternary_read_action_arg(tbl_name="ti_forward_user",
                                    action_name='ai_forward_user',
                    match_arg_name="hdr.ipv4.dst_addr",
                    match_arg_ternary_annotation="ipv4",
                    match_arg_ternary=server['ip_addr'],
                    match_arg_ternary_mask="255.255.255.255",
                        priority=0x2,
                    action_arg_name="egress_port",
                    action_arg=server['port_id'])

    m.disconnect()


def debug(log_time, log_dir):

    print("PID: " + str(os.getpid()))
    m = Manager(p4name="main")

    presize = 0
    while True:
        time.sleep(log_time)

        size = m.read_reg_element_for_pipe("buffer_index_", 0, pipeid=1)
        if size != presize:
            print("Size: " + str(size))
            presize = size

            #for index in range(size):
            #    portId = m.read_reg_element_for_pipe("buf_utils_", index, pipeid=1)
            #    print(portId)
            #print()
        
        '''
        for port in range(256):
            value = m.read_reg_element_for_pipe("reg_util_", port, pipeid=port_to_pipe(port))
            if value != 0:
                print("Port: " + str(port))
                print("Util: " + str(value))
                value = m.read_reg_element_for_pipe("reg_time_", port, pipeid=port_to_pipe(port))
                print("Time: " + str(value))
        '''

    '''
    create_dir(log_dir)

    print("--- Trigger debug modules to record for {} ---".format(args.time))                            
    m.set_default_rule_action_arg("ti_set_record_", "ai_set_record_", "flag", 1)
    m.set_default_rule_action_arg("te_set_record_", "ae_set_record_", "flag", 1)
    time.sleep(log_time)
    m.set_default_rule_action_arg("ti_set_record_", "ai_set_record_", "flag", 0)
    m.set_default_rule_action_arg("te_set_record_", "ae_set_record_", "flag", 0)

    print("pipe0&1 ri_port2ctr_")
    for port in range(256):
        value = m.read_reg_element_for_pipe("ri_port2ctr_", port, pipeid=port_to_pipe(port))
        if value != 0:
            print(port, value)

    print("pipe0&1 re_port2ctr_")
    port2ctr = {}
    for port in range(256):
        value = m.read_reg_element_for_pipe("re_port2ctr_", port, pipeid=port_to_pipe(port))
        if value != 0:
            print(port, value)
            port2ctr[port] = value
    with open(log_dir+"/re_port2ctr.json", "w") as f:
        json.dump(port2ctr, f, indent=4, sort_keys=True)
    '''

    #  print("pipe0 ri_mcgid2ctr_")
    #  for index in range(2048):
    #    value = long(m.read_reg_element_for_pipe("ri_mcgid2ctr_", index, pipeid=0))
    #    if value != 0:
    #      print(index, value)
    #  print("pipe1 ri_mcgid2ctr_")
    #  for index in range(2048):
    #    value = long(m.read_reg_element_for_pipe("ri_mcgid2ctr_", index, pipeid=1))
    #    if value != 0:
    #      print(index, value)

    #  for appid in [1, 2]:
    #    m.print_pktgen_counter_for_app(appid)

    #  print("pipe0 ri_gap_hist_ (strictly 0 otherwise):")
    #  print("# gap, count")
    #  gap2count = {}
    #  sum_gap = 0.0
    #  sum_count = 0
    #  for index in range(131072):
    #    value = long(m.read_reg_element_for_pipe("ri_gap_hist_", index, pipeid=0))
    #    if value != 0:
    #      gap2count[index] = value
    #    sum_count += value
    #    sum_gap += (index*value)
    #  print("Total sample #: {}".format(sum_count))
    #  if sum_count != 0:
    #    for index in range(131072):
    #      if index in gap2count.keys():
    #        print("{0}[ns], {1}, {2}%".format(index, gap2count[index], 1.0*gap2count[index]/sum_count*100.0))
    #    print("Mean gap: {0}".format(1.0*sum_gap/sum_count))
    #
    #  print("pipe0 re_gap_hist_ (strictly 0 otherwise):")
    #  print("# gap, count")
    #  gap2count = {}
    #  sum_gap = 0.0
    #  sum_count = 0
    #  for index in range(131072):
    #    value = long(m.read_reg_element_for_pipe("re_gap_hist_", index, pipeid=0))
    #    if value != 0:
    #     gap2count[index] = value
    #   sum_count += value
    #   sum_gap += (index*value)
    #  print("Total sample #: {}".format(sum_count))
    #  if sum_count != 0:
    #    for index in range(131072):
    #      if index in gap2count.keys():
    #        print("{0}[ns], {1}, {2}%".format(index, gap2count[index], 1.0*gap2count[index]/sum_count*100.0))
    #    print("Mean gap: {0}".format(1.0*sum_gap/sum_count))
    #  with open(log_dir+"/re_gap_hist.json", "w") as f:
    #    json.dump(gap2count, f, indent=4, sort_keys=True)

    #  seq2gap = {}
    #  for seq in range(131072):
    #    gap = long(m.read_reg_element_for_pipe("re_gap_rb_", seq, pipeid=0))
    #    if gap != 0:
    #      seq2gap[seq] = gap 
    #  with open(log_dir+"/re_gap_rb_pipe0.json", "w") as f:
    #    json.dump(seq2gap, f, indent=4, sort_keys=True)

    m.disconnect()


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    subcmds = parser.add_subparsers(dest="cmd")

    cmd_config = subcmds.add_parser("config")

    cmd_debug = subcmds.add_parser("debug")
    cmd_debug.add_argument("-t", "--time", type=float, required=False, default=0.1, help="Number of seconds to record the debugging stats")
    cmd_debug.add_argument("-d", "--dir", type=str, required=False, default="logs", help="Name of the directory to hold the debugging stats")

    args = parser.parse_args(['config'] if len(sys.argv)==1 else None)

    if args.cmd == "config":
        config()
    elif args.cmd == "debug":
        debug(args.time, args.dir)
    else:
        print("ERR")