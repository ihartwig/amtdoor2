#! /bin/sh

chmod 755 door_test.sh
sudo cp ./door_test.sh /etc/init.d/door_test
sudo update-rc.d door_test defaults
sudo cp update-amt-rfid /etc/cron.hourly/
echo "Setup complete."
