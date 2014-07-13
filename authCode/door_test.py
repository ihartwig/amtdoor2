#!/usr/bin/env python

import urllib 
import serial
import time
import os
import sys
import socket
import random

from datetime import datetime


def connect(device, rate):
	return serial.Serial(device, rate)


socket.setdefaulttimeout(10);

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


def parse_number(num):
	valid_cards = open('rfid.inc','r').read().strip().split('\n')
	return num in valid_cards

lastnum = "q"
lasttime = 0

while True:
	print "READY"
	while not arduino.inWaiting():
		if os.path.isfile('forceopen.now'):
			print datetime.today().isoformat(' ')
			print "!!FORCING DOOR OPEN REMOTELY!!"
			arduino.write('D')
			time.sleep(1);
			os.remove('forceopen.now')
			try:
				urllib.urlretrieve("https://ssl.acemonstertoys.org/member/logrfid.php?rfid=-1&valid=True")
			except:
				print "Timed-out or some other error reporting to AMT server"
		time.sleep(0.2)
	line = arduino.readline()
	print datetime.today().isoformat(' ')
	line = line.strip()
	print line
	parts = line.split('|')
	cmd = parts[0]
	if cmd == "BADREAD":
		print "invalid rfid read (noise?)"
	elif cmd == "SENSOR":
		try:
			urllib.urlretrieve("https://ssl.acemonstertoys.org/member/sensor.php?door="+parts[1])
		except:
			print "Timed-out or some other error reporting to AMT server"
	elif cmd == "TAG":
		num = parts[1]
		valid = parse_number(num)
		if valid:
			if num == lastnum and (time.time() - lasttime) < 15:
				print "Too many reads in a row"
			else:
				arduino.write('D')
				print "ACCESS GRANTED"
		else:
			print "DENIED"

		lastnum = num
		lasttime = time.time()

		try:
			urllib.urlretrieve("https://ssl.acemonstertoys.org/member/logrfid.php?rfid="+num+"&valid="+str(valid))
		except:
			print "Timed-out or some other error reporting to AMT server"
		while arduino.inWaiting():
			line = arduino.readline()
			line = line.strip()
			print "Ignored queued: "+line

#	else:
#		arduino.write('G')
#		arduino.close()
#		arduino = connect(ARDUINO_PORT, ARDUINO_RATE)
