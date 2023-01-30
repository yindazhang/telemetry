nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1" > 5.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1" > 6.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1" > 7.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1 --OrbWeaver=1" > 5-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1 --OrbWeaver=1" > 6-Orb1.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1 --OrbWeaver=1" > 7-Orb1.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1 --OrbWeaver=3" > 5-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1 --OrbWeaver=3" > 6-Orb3.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1 --OrbWeaver=3" > 7-Orb3.out &

nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.5_10G_0.2 --record=1 --OrbWeaver=7" > 5-Orb7.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.6_10G_0.2 --record=1 --OrbWeaver=7" > 6-Orb7.out &
nohup ./ns3 run "scratch/telemetry --flow=Hadoop_143_0.7_10G_0.2 --record=1 --OrbWeaver=7" > 7-Orb7.out &

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

# Add
# point-to-point/...