#!/bin/sh
#
#put this in /etc/rc.local:
#
# cd /home/davr
# screen -d -m -S door ./door_test.py
#

while true; do
  ./door_test.py
  sleep 10
done
