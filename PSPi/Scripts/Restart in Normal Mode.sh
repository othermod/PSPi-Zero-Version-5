#!/bin/bash
echo "Checking status in config.txt"
grep '#dtoverlay=pi3-disable-wifi' /boot/config.txt >/dev/null
if [ $? -eq 0 ]; then
	echo "WiFi and BT appear to already be enabled"
	sleep 1
else
	echo "Enabling WiFi and BT in config.txt"
	sudo sed -i 's/dtoverlay=pi3-disable-wifi/#dtoverlay=pi3-disable-wifi/' /boot/config.txt
	sudo sed -i 's/dtoverlay=pi3-disable-bt/#dtoverlay=pi3-disable-bt/' /boot/config.txt
	echo "Enabling services"
#	sudo systemctl enable wpa_supplicant 2> /dev/null
#	echo "-wpa_supplicant"
#	sudo systemctl enable bluetooth 2> /dev/null
#	echo "-bluetooth"
#	sudo systemctl enable hciuart 2> /dev/null
#	echo "-hciuart"
	sudo update-rc.d nmbd enable 2> /dev/null
	echo "-smbd"
	sudo systemctl enable smbd 2> /dev/null
	echo "Rebooting"
	sleep 1
	sudo reboot
fi

