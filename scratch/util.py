import subprocess
import argparse
import pandas as pd
import numpy as np

collectorDic = {}
switchDic = {}

totalUnit = 0
findUnit = 0

portMp = {
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

            if switchDic.get(microtime) == None:
                switchDic[microtime] = {}

            if switchDic[microtime].get(nodeId) == None:
                switchDic[microtime][nodeId] = {}
            
            if switchDic[microtime][nodeId].get(port) == None:
                switchDic[microtime][nodeId][port] = byte
    
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

            if collectorDic.get(microtime) == None:
                collectorDic[microtime] = {}

            if collectorDic[microtime].get(nodeId) == None:
                collectorDic[microtime][nodeId] = {}
            
            if collectorDic[microtime][nodeId].get(port) == None:
                collectorDic[microtime][nodeId][port] = byte
    
    f.close()

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()

    parse_switch_file(args.file + ".switch.util.data")
    parse_collector_file(args.file + ".collector.util")

    totalECMP = 0
    findECMP = 0
    rates = []

    for microtime in switchDic:
        if len(switchDic[microtime]) != len(portMp):
            print("Error!")

        utils = [0 for i in range(9)]
        for nodeId in switchDic[microtime]:
            if len(switchDic[microtime][nodeId]) != len(portMp[nodeId]):
                print("Error!")

            for port in switchDic[microtime][nodeId]:
                utils[(nodeId % 3) * 3 + port - 4] += switchDic[microtime][nodeId][port]

            
        maximum = 0
        minimum = 0xffffff
        rate = 0

        for util in utils:
            #print(util, end = ",")
            maximum = max(util, maximum)
            minimum = min(util, minimum)
        #print()
            
        if minimum == 0 and maximum == 0:
            rate = 1
        elif minimum == 0:
            rate = 10000
        else:
            rate = (maximum - minimum) / minimum

        rates.append(rate)
        
        
        if rate > 1:
            totalECMP += 1

            isIn = True

            if collectorDic.get(microtime) == None:
                isIn = False
            else:
                for nodeId in switchDic[microtime]:
                    if collectorDic[microtime].get(nodeId) == None:
                        isIn = False
                    elif len(collectorDic[microtime][nodeId]) != len(portMp[nodeId]):
                        isIn = False
            
            if isIn:
                findECMP += 1

    #rates.sort()
    #for i in range(1, 100):
    #    print(rates[int(len(rates) * i / 100)])   
    
    print((1 - (findUnit / totalUnit), totalUnit))
    print((1 - (findECMP / totalECMP), totalECMP))