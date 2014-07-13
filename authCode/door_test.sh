#! /bin/sh
#
# copy to:
# /etc/init.d/door_test
#

DOOR_TEST_DIR=/home/pi/repos/amtdoor2/authCode

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    echo "Starting door_test.py..."
    cd DOOR_TEST_DIR
    python door_test.py > door_test.log &
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