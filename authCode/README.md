amtdoor
=======

Code that manages authenticating to and opening our door.

`check.py` is a utility to look for syntax errors in a python script.



`door_force_open.py` will force the door to open in emergencies (can only run if door_test.py is NOT running, since it talks to the arduino).


`door_test.py` listens on the usb serial port (aka an arduino) for rfids coming in. Once it reads an rfid, it compares it against a local database of valid rfids, if it matches, it sends a command to the arduino to open the door

It also listens to the arduino for the door sensor, which reports when the door opens and closes.

All of the above are also sent via http to our server for logging.


`door_test.sh` is a wrapper around door_test.py that auto-restarts on error. This is started by the doortestd at system boot.


`doortestd` is a debian system daemon script that can be used to start and stop the door test but keep it running when there are no active users on the auth computer.


`getrfids.php` updates the local database of valid rfids from the server. Gets called from cron via update-amt-rfid script.


`setup.sh` provisions the system to act as an authentication server for our door by setting up the startup and cron jobs. This can be done manually like this:
```
$ sudo cp ./doortestd /etc/init.d/doortestd
$ sudo update-rc.d door_test defaults
$ sudo cp update-amt-rfid /etc/cron.hourly/
```
Will likely need to be call as su/sudo.

The system can be unprovisioned with:
```
$ sudo update-rc.d doortestd remove
$ sudo rm /etc/init.d/door_test
$ sudo rm /etc/cron.hourly/update-amt-rfid
