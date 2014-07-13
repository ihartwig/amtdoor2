#! /bin/sh
# /etc/init.d/door_test
#

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    echo "Starting door_test.py..."
    python /home/pi/repos/amtdoor2/authCode/door_test.py &
    echo "done."
    ;;
  stop)
    echo "Stopping door_test.py..."
    pkill -f door_test.py
    echo "done."
    ;;
  *)
    echo "Usage: /etc/init.d/door_test {start|stop}"
    exit 1
    ;;
esac

exit 0