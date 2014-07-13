#!/usr/bin/env python

import urllib 
import serial
import os
import sys

from datetime import datetime
from time import sleep


def connect(device, rate):
	return serial.Serial(device, rate)


ARDUINO_RATE = 9600

try:
    arduino = connect("/dev/ttyACM0", ARDUINO_RATE)
    print "Opened on ACM0"
except serial.serialutil.SerialException:
    try:
        arduino = connect("/dev/ttyACM1", ARDUINO_RATE)
        print "Opened on ACM1"
    except serial.serialutil.SerialException:
        print "Couldn't open USB? Try 'ls /dev/ttyACM*' and then modify the code"
	sys.exit()

while True:
	print "Sending open command..."
	arduino.write('D')
	sleep(30)
#	else:
#		arduino.write('G')
#		arduino.close()
#		arduino = connect(ARDUINO_PORT, ARDUINO_RATE)
