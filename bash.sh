sudo apt install python3-pip
pip3 install pandas

cd /mydata
rm -f ns-allinone-3.37.tar.bz2
git clone https://github.com/yindazhang/telemetry.git

cd ns-allinone-3.37/ns-3.37/
rm -r scratch/
rm -r src/
cp -r ../../telemetry/scratch/ ./
cp -r ../../telemetry/src/ ./

nohup ./ns3 run test-runner > b.out &


# Modify:
# application/bulk-send-application.h/.cc
# network/node.h/.cc
# network/queue.h/.cc
# queue-disc.cc #remove 1090
# internet/ipv4-end-point-demux.cc
# internet/tcp-socket-base.h/.cc

# Add
# point-to-point/...