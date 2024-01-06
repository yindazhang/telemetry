from optparse import OptionParser

loads = [4,5,6,7,8]
topologies = [1]
taskIds = [7] #[1,2,3,4,7]
utilGaps = [1000]
generateBps = [128]
#utilGaps = [6000, 7000, 8000, 9000, 10000]
#OrbWeavers = [0,2,3,9,33]
OrbWeavers = [2,3,9,33]
hG = 1

def AddLoad(start, outFile):
    global hG
    for load in loads:
        cmd = start
        if hG == 1:
            cmd += "--hG=1 "
            cmd += "--time=0.1 "
            cmd += "--flow=Hadoop_144_" + str(load/10) + "_100G_0.1"
        else:
            cmd += "--time=0.5 "
            cmd += "--flow=Hadoop_144_" + str(load/10) + "_10G_0.5"
        cmd += "\" > "
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

def AddGenerateBps(start, outFile):
    for bps in generateBps:
        cmd = start
        cmd += "--GenerateBps=" + str(bps*1024*1024) + " "
        AddECMPFail(cmd, outFile + "-Bps" + str(bps))

def AddTaskId(start, outFile):
    for taskId in taskIds:
        cmd = start
        cmd += "--taskId=" + str(taskId) + " "
        if taskId != 3:
            AddUtilGap(cmd, outFile + "-Task" + str(taskId))
        else:
            AddGenerateBps(cmd, outFile + "-Task" + str(taskId))

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
    parser = OptionParser()
    parser.add_option("-g", "--hG", dest = "hG", help = "100Gbps", default = "1")
    options, args = parser.parse_args()
    hG = int(options.hG)

    start = "nohup ./ns3 run \"scratch/telemetry --record=1 --fctRecord=1 "
    outFile = ""
    AddOrbWeaver(start, outFile)               