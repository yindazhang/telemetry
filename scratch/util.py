import subprocess
import argparse
import pandas as pd
import numpy as np

collectorDic = {}
switchDic = {}

def parse_switch_file(path_file):
    f = open(path_file, "r")
    text = f.read()
    lines = text.split('\n')
    total = 0
    
    for line in lines:
        numbers = line.split(' ')
        if len(numbers) == 1 and numbers[0] != '':
            total += int(numbers[0])
    
    f.close()
    print(total)


def parse_collector_file(path_file):
    f = open(path_file, "r")
    text = f.read()
    lines = text.split('\n')

    totalReceived = 0
    
    for line in lines:
        numbers = line.split(' ')
        if len(numbers) == 7:
            srcRack = (int(numbers[0]) >> 16) & 0xff
            dstRack = (int(numbers[1]) >> 16) & 0xff

            srcServer = (int(numbers[0]) >> 8) & 0xff
            dstServer = (int(numbers[1]) >> 8) & 0xff

            srcPort = int(numbers[2])
            dstPort = int(numbers[3])

            bitmap = (1 << (63 - int(numbers[6])))

            key = (srcRack, dstRack, srcServer, dstServer, srcPort, dstPort)
            if collectorDic.get(key) == None:
                collectorDic[key] = 0
            collectorDic[key] |= bitmap

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()

    common = "s_ECMP1_Orb"
    back = []
    for i in ["3", "5", "9"]:
        back.append(common + i + ".switch.util")

    for b in back:
        parse_switch_file(args.file + b)