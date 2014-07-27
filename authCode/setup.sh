#! /bin/sh

chmod 755 doortestd
sudo cp ./doortestd /etc/init.d/doortestd
sudo update-rc.d doortestd defaults
sudo cp update-amt-rfid /etc/cron.hourly/
echo "Setup complete."
