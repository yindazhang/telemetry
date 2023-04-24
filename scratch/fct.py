

import subprocess
import argparse
import pandas as pd
import numpy as np

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()
    
    names = ['Ours']
    stats = ['Number', 'Sum', 'Mean', '99%', '99.9%']
    
    common = "s_Topo0_ECMP0"
    # common = "s_ECMP1_Fail"
    appends = [common + "_Orb2.fct"]
    
    if len(appends) != len(names):
        print("Error in length")
        
    dfs = {}
    for i in range(len(appends)):
        dfs[names[i]] = pd.read_csv(args.file + appends[i], header=None, delimiter=r"\s+")
    
    dic = {'Time' : []}
    for name in names:
        for stat in stats:
            dic[name + "_" + stat] = []
            
    for t in range(2000000000, 3000000000, 10000000):
        dic['Time'].append(t)
        for name in names:
            df = dfs[name]
            dic[name + "_Sum"].append(df[df[3] < t][1].sum())
            df = df[df[3] < t][2].sort_values()
            size = len(df)
            if size != 0:
                dic[name + "_Number"].append(size)
                dic[name + "_Mean"].append(df.mean())
                dic[name + "_99%"].append(df.iloc[int(0.99 * size)])
                dic[name + "_99.9%"].append(df.iloc[int(0.999 * size)])
            else:
                dic[name + "_Number"].append(0)
                dic[name + "_Mean"].append(0)
                dic[name + "_99%"].append(0)
                dic[name + "_99.9%"].append(0)
    
    df = pd.DataFrame.from_dict(dic)
    df.to_csv('csv/' + args.file + common + ".csv", index=False)
    
    print("Finish CSV")
