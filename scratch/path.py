import subprocess
import argparse
import pandas as pd
import numpy as np

dic = {0 : 0, 1 : 0}

def parse_tr_file(tr_file):
    f = open(tr_file, "r")
    text = f.read()
    lines = text.split('\n')
    
    for line in lines:
        numbers = line.split(' ')
        if len(numbers) == 4:
            src = int(numbers[0])
            dst = int(numbers[1])

            if (src // 16) == (dst // 16):
                dic[0] += 1
            else:
                dic[1] += 1

def parse_path_file(path_file):
    f = open(path_file, "r")
    text = f.read()
    lines = text.split('\n')

    resultDic = {0 : 0, 1 : 0}
    pathDic = {}
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

            # nodeId = int(numbers[4])

            bitmap = (1 << (63 - int(numbers[6])))

            key = (srcRack, dstRack, srcServer, dstServer, srcPort, dstPort)
            if pathDic.get(key) == None:
                pathDic[key] = 0
            pathDic[key] |= bitmap
        
            totalReceived += 1

    for key, bitmap in pathDic.items():
        if key[0] == key[1]:
            if bitmap == 1:
                resultDic[0] += 1
        else:
            if bitmap == 0x7:
                resultDic[1] += 1
    
    loss_unit = dic[0] * 2 + dic[1] * 6 - totalReceived
    print((loss_unit / (dic[0] * 2 + dic[1] * 6), loss_unit, (dic[0] * 2 + dic[1] * 6)))

    loss_path = [dic[0] * 2 - resultDic[0], dic[1] * 2 - resultDic[1]]
    print(((loss_path[0] + loss_path[1]) / (dic[0] + dic[1]) / 2, loss_path[0] + loss_path[1], (dic[0] + dic[1]) * 2))

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()
    
    parse_tr_file(args.file + ".tr")
    parse_path_file(args.file + "s_ECMP1_Orb9.path")