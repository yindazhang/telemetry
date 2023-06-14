import subprocess
import argparse
import pandas as pd
import numpy as np

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    args = parser.parse_args()
    
    names = []
    stats = ['Number', 'Sum', 'Mean', '99%', '99.9%']
    
    common = "Hadoop_142_0.4_10G_0.5_"
    append = 's_Topo1_ECMP0_Gap10000.fct'
        
    dfs = {}
    for i in range(5):
        names.append("Ours_" + str(i))
        dfs[names[i]] = pd.read_csv(common + str(i) + append, header=None, delimiter=r"\s+")
    
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
    df.to_csv('csv/' + common + str(i) + append + ".csv", index=False)
    
    print("Finish CSV")
