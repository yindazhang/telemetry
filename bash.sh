sudo /usr/local/etc/emulab/mkextrafs.pl /mydata

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1" > 5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1" > 6.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1" > 7.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.1_10G_0.5 --record=1" > 1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.1_10G_0.5 --record=1 --OrbWeaver=1" > 1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.1_10G_0.5 --record=1 --OrbWeaver=3" > 1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.1_10G_0.5 --record=1 --OrbWeaver=5" > 1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.1_10G_0.5 --record=1 --OrbWeaver=9" > 1-Orb9.out &


nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1" > 5-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1" > 6-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1" > 7-ECMP1.out &



nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=3" > 5-Fail-Orb3-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=3" > 6-Fail-Orb3-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=3" > 7-Fail-Orb3-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=5" > 5-Fail-Orb5-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=5" > 6-Fail-Orb5-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=5" > 7-Fail-Orb5-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=9" > 5-Fail-Orb9-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=9" > 6-Fail-Orb9-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=9" > 7-Fail-Orb9-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=17" > 5-Fail-Orb17-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=17" > 6-Fail-Orb17-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --Failure=1 --record=1 --taskId=1 --OrbWeaver=17" > 7-Fail-Orb17-1.out &




nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=3" > 5-ECMP1-Orb3-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=3" > 6-ECMP1-Orb3-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=3" > 7-ECMP1-Orb3-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=5" > 5-ECMP1-Orb5-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=5" > 6-ECMP1-Orb5-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=5" > 7-ECMP1-Orb5-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=9" > 5-ECMP1-Orb9-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=9" > 6-ECMP1-Orb9-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=9" > 7-ECMP1-Orb9-1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=17" > 5-ECMP1-Orb17-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=17" > 6-ECMP1-Orb17-1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --taskId=1 --OrbWeaver=17" > 7-ECMP1-Orb17-1.out &



nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=3" > 5-Topo0-ECMP1-Orb3-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=3" > 6-Topo0-ECMP1-Orb3-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=3" > 7-Topo0-ECMP1-Orb3-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=5" > 5-Topo0-ECMP1-Orb5-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=5" > 6-Topo0-ECMP1-Orb5-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=5" > 7-Topo0-ECMP1-Orb5-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=9" > 5-Topo0-ECMP1-Orb9-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=9" > 6-Topo0-ECMP1-Orb9-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=9" > 7-Topo0-ECMP1-Orb9-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=17" > 5-Topo0-ECMP1-Orb17-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=17" > 6-Topo0-ECMP1-Orb17-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=0 --utilGap=10000 --OrbWeaver=17" > 7-Topo0-ECMP1-Orb17-2.out &



nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=3" > 5-Topo1-ECMP1-Orb3-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=3" > 6-Topo1-ECMP1-Orb3-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=3" > 7-Topo1-ECMP1-Orb3-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=5" > 5-Topo1-ECMP1-Orb5-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=5" > 6-Topo1-ECMP1-Orb5-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=5" > 7-Topo1-ECMP1-Orb5-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=9" > 5-Topo1-ECMP1-Orb9-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=9" > 6-Topo1-ECMP1-Orb9-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=9" > 7-Topo1-ECMP1-Orb9-2.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=17" > 5-Topo1-ECMP1-Orb17-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=17" > 6-Topo1-ECMP1-Orb17-2.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --fctRecord=1 --taskId=2 --Topology=1 --utilGap=10000 --OrbWeaver=17" > 7-Topo1-ECMP1-Orb17-2.out &





nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.2_10G_0.2 --record=1 --INT=4" > 5-INT4.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.4_10G_0.2 --record=1 --INT=4" > 6-INT4.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1 --INT=4" > 7-INT4.out &

# Modify:
# application/bulk-send-application.h/.cc
# point-to-point/ppp-header.h/.cc
# point-to-point/point-to-point-net-device.h/.cc
# network/node.h/.cc
# network/queue.h/.cc
# queue-disc.cc #remove 1090
# internet/ipv4-end-point-demux.cc
# internet/tcp-socket-base.h/.cc

# Add
# point-to-point/...