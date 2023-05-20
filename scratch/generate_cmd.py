loads = range(4, 9)
topologies = [1]
taskIds = [2] #[1,2]
utilGaps = [5000]
#OrbWeavers = [17]
OrbWeavers = [3,9,17]

def AddLoad(start, outFile):
    for load in loads:
        cmd = start
        cmd += "--flow=Hadoop_142_" + str(load/10) + "_10G_0.5" +"\" > "
        print(cmd + outFile + "-" + str(load) + ".out &")
    print()

def AddStore(start, outFile):
    cmd = start + "--Temp=0 --Store=0 "
    AddLoad(cmd, outFile + "-Store0")
    #cmd = start + "--Temp=1 --Store=0 "
    #AddLoad(cmd, outFile + "-Store1")
    #cmd = start + "--Temp=0 --Store=1 "
    #AddLoad(cmd, outFile + "-Store2")

def AddECMPFail(start, outFile):
    cmd = start + "--ECMP=1 --Failure=0 "
    AddStore(cmd, outFile + "-ECMP1-Fail0")
    cmd = start + "--ECMP=0 --Failure=1 "
    AddStore(cmd, outFile + "-ECMP0-Fail1")
    cmd = start + "--ECMP=0 --Failure=0 "
    AddStore(cmd, outFile + "-ECMP0-Fail0")

def AddUtilGap(start, outFile):
    for utilGap in utilGaps:
        cmd = start
        cmd += "--utilGap=" + str(utilGap) + " "
        AddECMPFail(cmd, outFile + "-Gap" + str(utilGap))

def AddTaskId(start, outFile):
    for taskId in taskIds:
        cmd = start
        cmd += "--taskId=" + str(taskId) + " "
        AddUtilGap(cmd, outFile + "-Task" + str(taskId))

def AddTopology(start, outFile):
    for topology in topologies:
        cmd = start
        cmd += "--Topology=" + str(topology) + " "
        AddTaskId(cmd, outFile + "-Topo" + str(topology))

def AddOrbWeaver(start, outFile):
    for OrbWeaver in OrbWeavers:
        cmd = start
        if OrbWeaver != 0:
            cmd += "--OrbWeaver=" + str(OrbWeaver) + " "
        AddTopology(cmd, outFile + "Orb" + str(OrbWeaver))
            
if __name__=="__main__":
    start = "nohup ./ns3 run \"scratch/telemetry --record=1 --fctRecord=1 "
    outFile = ""
    AddOrbWeaver(start, outFile)               