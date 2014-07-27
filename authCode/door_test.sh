#!/bin/sh
#
# Automatically restart the door test program on error (like loosing panelMcu
# connection or not being able to connect at all).
#

while true; do
  echo "(Re)starting door test..."
  ./door_test.py
  sleep 10
done
