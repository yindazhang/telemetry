nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1" > 5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1" > 6.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1" > 7.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --Failure=1" > 5-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --Failure=1" > 6-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --Failure=1" > 7-ECMP1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 5-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 6-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 7-ECMP1-Orb1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 5-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 6-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 7-ECMP1-Orb3.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 5-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 6-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 7-ECMP1-Orb5.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 5-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 6-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 7-ECMP1-Orb9.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1" > 5-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1" > 6-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1" > 7-ECMP1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1" > 5-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1" > 6-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=1" > 7-ECMP1-Orb1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3" > 5-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3" > 6-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=3" > 7-ECMP1-Orb3.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5" > 5-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5" > 6-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=5" > 7-ECMP1-Orb5.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9" > 5-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9" > 6-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --ECMP=1 --record=1 --OrbWeaver=9" > 7-ECMP1-Orb9.out &


nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1" > 5-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1" > 6-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1" > 7-ECMP1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1" > 5-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1" > 6-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1" > 7-ECMP1-Orb1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3" > 5-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3" > 6-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3" > 7-ECMP1-Orb3.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5" > 5-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5" > 6-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5" > 7-ECMP1-Orb5.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9" > 5-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9" > 6-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9" > 7-ECMP1-Orb9.out &


nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --Failure=1" > 5-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --Failure=1" > 6-ECMP1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --Failure=1" > 7-ECMP1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 5-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 6-ECMP1-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=1 --Failure=1" > 7-ECMP1-Orb1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 5-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 6-ECMP1-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=3 --Failure=1" > 7-ECMP1-Orb3.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 5-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 6-ECMP1-Orb5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=5 --Failure=1" > 7-ECMP1-Orb5.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 5-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 6-ECMP1-Orb9.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.5 --ECMP=1 --record=1 --OrbWeaver=9 --Failure=1" > 7-ECMP1-Orb9.out &


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