import subprocess
import argparse
import pandas as pd
import numpy as np

collectorDic = {}
switchDic = {}

totalUnit = 0
findUnit = 0

portMp = {
    144: {13 : 1, 14 : 1, 15 : 1}, 145: {13 : 1, 14 : 1, 15 : 1},\
    146: {13 : 1, 14 : 1, 15 : 1}, 147: {13 : 1, 14 : 1, 15 : 1},\
    148: {13 : 1, 14 : 1, 15 : 1}, 149: {13 : 1, 14 : 1, 15 : 1},\
    150: {13 : 1, 14 : 1, 15 : 1}, 151: {13 : 1, 14 : 1, 15 : 1},\
    152: {13 : 1, 14 : 1, 15 : 1}, 153: {13 : 1, 14 : 1, 15 : 1},\
    154: {13 : 1, 14 : 1, 15 : 1}, 155: {13 : 1, 14 : 1, 15 : 1},\
    156: {4 : 1, 5 : 1, 6 : 1}, 157: {4 : 1, 5 : 1, 6 : 1},\
    158: {4 : 1, 5 : 1, 6 : 1}, 159: {4 : 1, 5 : 1, 6 : 1},\
    160: {4 : 1, 5 : 1, 6 : 1}, 161: {4 : 1, 5 : 1, 6 : 1},\
    162: {4 : 1, 5 : 1, 6 : 1}, 163: {4 : 1, 5 : 1, 6 : 1},\
    164: {4 : 1, 5 : 1, 6 : 1}, 164: {4 : 1, 5 : 1, 6 : 1},\
    166: {4 : 1, 5 : 1, 6 : 1}, 167: {4 : 1, 5 : 1, 6 : 1},\
}

def parse_switch_file(path_file):
    global totalUnit

    f = open(path_file, "r")
    text = f.read()
    lines = text.split('\n')
    
    for line in lines:
        numbers = line.split(' ')
        if len(numbers) == 4:
            totalUnit += 1

            nodeId = int(numbers[0])
            port = int(numbers[1])

            microtime = int(numbers[2]) // 20
            byte = int(numbers[3])

            if portMp.get(nodeId) == None:
                continue
            if portMp[nodeId].get(port) == None:
                continue

            if switchDic.get(nodeId) == None:
                switchDic[nodeId] = {}
            
            if switchDic[nodeId].get(microtime) == None:
                switchDic[nodeId][microtime] = {}
            
            if switchDic[nodeId][microtime].get(port) == None:
                switchDic[nodeId][microtime][port] = byte
    
    f.close()


def parse_collector_file(path_file):
    global findUnit

    f = open(path_file, "r")
    text = f.read()
    lines = text.split('\n')
    
    for line in lines:
        numbers = line.split(' ')
        if len(numbers) == 4:
            findUnit += 1

            nodeId = int(numbers[0])
            port = int(numbers[1])

            microtime = int(numbers[2]) // 20
            byte = int(numbers[3])

            if portMp.get(nodeId) == None:
                continue
            if portMp[nodeId].get(port) == None:
                continue

            if collectorDic.get(nodeId) == None:
                collectorDic[nodeId] = {}
            
            if collectorDic[nodeId].get(microtime) == None:
                collectorDic[nodeId][microtime] = {}
            
            if collectorDic[nodeId][microtime].get(port) == None:
                collectorDic[nodeId][microtime][port] = byte
    
    f.close()

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()

    parse_switch_file(args.file + ".switch.util.data")
    parse_collector_file(args.file + ".collector.util")

    totalECMP = 0
    findECMP = 0

    for nodeId in switchDic:
        for microtime in switchDic[nodeId]:
            if len(switchDic[nodeId][microtime]) != len(portMp[nodeId]):
                print("Error!")
            
            maximum = 0
            minimum = 0xffffff
            rate = 0

            for port in switchDic[nodeId][microtime]:
                maximum = max(switchDic[nodeId][microtime][port], maximum)
                minimum = min(switchDic[nodeId][microtime][port], minimum)
            
            if minimum == 0 and maximum == 0:
                rate = 1
            elif minimum == 0:
                rate = 10000
            else:
                rate = maximum - minimum / minimum

            if rate > 10:
                totalECMP += 1

                if collectorDic.get(nodeId) == None:
                    continue
                elif collectorDic[nodeId].get(microtime) == None:
                    continue
                elif len(collectorDic[nodeId][microtime]) != len(portMp[nodeId]):
                    continue
                else:
                    maximum = 0
                    minimum = 0xffffff
                    newRate = 0

                    for port in collectorDic[nodeId][microtime]:
                        maximum = max(collectorDic[nodeId][microtime][port], maximum)
                        minimum = min(collectorDic[nodeId][microtime][port], minimum)
                    
                    if minimum == 0 and maximum == 0:
                        newRate = 1
                    elif minimum == 0:
                        newRate = 10000
                    else:
                        newRate = maximum - minimum / minimum
                    
                    if newRate != rate:
                        print("ECMP not match")

                    findECMP += 1
            
    print(1 - (findUnit / totalUnit))
    print(1 - (findECMP / totalECMP))