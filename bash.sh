nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.2_10G_1 --record=1" > 2.out &
nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.4_10G_1 --record=1" > 4.out &
nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.6_10G_1 --record=1" > 6.out &

nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.2_10G_1 --record=1 --INT=4" > 2-INT4.out &
nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.4_10G_1 --record=1 --INT=4" > 4-INT4.out &
nohup ./ns3 run "scratch/telemetry --flow=WebSearch_143_0.6_10G_1 --record=1 --INT=4" > 6-INT4.out &

# Modify:
# application/bulk-send-application.h/.cc
# point-to-point/ppp-header.h/.cc
# point-to-point/point-to-point-net-device.h/.cc
# network/node.h/.cc
# queue-disc.cc #remove 1090

# Add
# point-to-point/switch-node.h/.cc
# point-to-point/my-queue.h/.cc