amtdoor
=======

Code that manages opening our door

door_test.py listens on the usb serial port (aka an arduino) for rfids coming in. 
once it reads an rfid, it compares it against a local database of valid rfids, if it matches,
it sends a command to the arduino to open the door

it also listens to the arduino for the door sensor, which reports when the door opens and closes

all of the above are also sent via http to our server for logging


getrfids.php updates the local database of valid rfids from the server. gets called from cron via update-amt-rfid script


autorun.sh gets called via /etc/rc.local and it runs the door_test.py script in a loop in case it crashes


check.py is a utility to look for syntax errors in a python script


door_force_open.py will force the door to open in emergencies 
(can only run if door_test.py is NOT running, since it talks to the arduino)

