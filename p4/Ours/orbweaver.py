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
    m.enab_ports_sym_bw(mc_pipe0_ports, "25G")
    m.enab_ports_sym_bw(server_ports, "25G")
    m.enab_ports_sym_bw(loopback_ports, "25G")
    m.enab_ports_sym_bw_b(redyellow_ports, "100G")

    m.create_mc_grp(201, [0, 8, 20, 24, 68, 196])

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

    presize = [0, 0]
    predrop = [0, 0]
    preadd = [0, 0]
    while True:
        time.sleep(log_time)

        for pipeid in [0, 1]:
            size = m.read_reg_element_for_pipe("buffer_index_", 0, pipeid=pipeid)
            if size != presize[pipeid]:
                print(str(pipeid) + " Size: " + str(size))
                presize[pipeid] = size

            drop = m.read_reg_element_for_pipe("reg_tele_drop_", 0, pipeid=pipeid)
            if drop != predrop[pipeid]:
                print(str(pipeid) + " Drop: " + str(drop))
                predrop[pipeid] = drop

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