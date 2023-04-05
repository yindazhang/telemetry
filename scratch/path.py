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
            if switchDic.get(key) == None:
                switchDic[key] = 0
            switchDic[key] |= bitmap
    
    f.close()


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

    common = "s_ECMP0_Fail"
    parse_switch_file(args.file + common + "_Orb17.switch.path")
    parse_collector_file(args.file + common + "_Orb17.collector.path")

    totalUnit = 0
    unitLoss = 0
    partialLoss = 0
    totalLoss = 0
    for key in switchDic:
        ones = bin(switchDic[key]).count("1")
        totalUnit += ones
        if collectorDic.get(key) == None:
            totalLoss += 1
            partialLoss += 1
            unitLoss += ones
        elif switchDic[key] != collectorDic[key]:
            partialLoss += 1
            bitmap = switchDic[key] ^ collectorDic[key]
            unitLoss += bin(bitmap).count("1")

    print( (unitLoss / totalUnit, unitLoss) )
    print( (partialLoss / len(switchDic), partialLoss) )