sudo apt install python3-pip
pip3 install pandas

# Modify:
# application/bulk-send-application.h/.cc
# network/node.h/.cc
# network/queue.h/.cc
# queue-disc.cc #remove 1090
# internet/ipv4-end-point-demux.cc
# internet/tcp-socket-base.h/.cc

# Add
# point-to-point/...