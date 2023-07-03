python make.py compile main.p4 /home/yindaz/OrbWeaver/p4v16/out
python make.py switchd /home/yindaz/OrbWeaver/p4v16/main.cpp /home/yindaz/OrbWeaver/p4v16/out/ -t 0x1234 -g 59 -p 0x11
python orbweaver.py debug -t 10

python make.py compile udp-low.p4 /home/yindaz/port/test/p4v16/udp-low/out
python make.py compile ours.p4 /home/yindaz/test/p4v16/out

python make.py kill
python make.py switchd /home/yindaz/port/p4v16/main.cpp /home/yindaz/port/p4v16/out/ -t 0x1234 -g 59 -p 0x11
python orbweaver.py config
python orbweaver.py debug -t 1

sudo ip route add 10.1.1.0/24 dev enp33s0