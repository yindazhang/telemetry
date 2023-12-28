sudo apt-get update
sudo apt install python3-pip
pip3 install pandas

cd /mydata
rm -f ns-allinone-3.37.tar.bz2
git clone https://github.com/yindazhang/telemetry.git

sudo chmod -R 777 /mydata/
cd /mydata/telemetry/.git
vim config
#filemode = false

cd /mydata/ns-allinone-3.37/ns-3.37/
rm -r scratch/
rm -r src/
cp -r ../../telemetry/scratch/ ./
cp -r ../../telemetry/src/ ./
bash.sh bash.sh
nohup ./ns3 run test-runner > b.out &


cd /mydata/telemetry
git pull

cd ../ns-allinone-3.37/ns-3.37/
cp -r ../../telemetry/bash.sh ./
cp -r ../../telemetry/scratch/generate_cmd.py ./scratch/generate_cmd.py
cp -r ../../telemetry/src/point-to-point/model/collector-node.cc ./src/point-to-point/model/collector-node.cc

sudo killall -9 ns3.37-telemetry

# Modify:
# application/bulk-send-application.h/.cc
# network/node.h/.cc
# network/queue.h/.cc
# queue-disc.cc #remove 1090
# internet/ipv4-end-point-demux.cc
# internet/tcp-socket-base.h/.cc

# Add
# point-to-point/...
