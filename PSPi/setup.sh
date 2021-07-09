#!/bin/bash
# https://www.othermod.com
if [ $(id -u) -ne 0 ]; then
	echo "Installer must be run as root."
	echo "Try 'sudo bash $0'"
	exit 1
fi

killall emulationstatio 2>/dev/null
sleep 1
echo "Copying files"
# add boot script to rc.local
grep pspi /etc/rc.local >/dev/null
if [ $? -eq 0 ]; then
#	echo "pspi already exists in rc.local. Re-creating."
	# pspi already in rc.local, but make sure correct:
	sed -i "s/^.*pspi.*$/bash \/boot\/pspi\/boot.sh/g" /etc/rc.local >/dev/null
else
#	echo "pspi doesn't exist in rc.local. Creating."
	# Insert pspi into rc.local before final 'exit 0'
	sed -i "s/^exit 0/bash \/boot\/pspi\/boot.sh\\nexit 0/g" /etc/rc.local >/dev/null
fi

# Make directory for driver and other files
rm -r /home/pi/PSPi 2>/dev/null
mkdir /home/pi/PSPi

# Startup image
cp -f /boot/PSPi/Theme/pspi.png /home/pi/RetroPie/splashscreens/pspi.png
cp -f /boot/PSPi/Theme/splashscreen.list /etc/splashscreen.list

# Controller Software
mkdir /home/pi/PSPi/Driver
cp -r /boot/PSPi/Driver/. /home/pi/PSPi/Driver

# Controller Configs
cp -f /boot/PSPi/Configs/es_input.cfg /opt/retropie/configs/all/emulationstation/es_input.cfg
cp -f /boot/PSPi/Configs/es_settings.cfg /opt/retropie/configs/all/emulationstation/es_settings.cfg
cp -f /boot/PSPi/Configs/retroarch.cfg /opt/retropie/configs/all/retroarch.cfg
cp -f "/boot/PSPi/Configs/PSPi Controller.cfg" "/opt/retropie/configs/all/retroarch-joypads/PSPi Controller.cfg"

# Console Scripts
cp -f /boot/PSPi/Theme/es_systems.cfg /etc/emulationstation/es_systems.cfg
mkdir /home/pi/PSPi/Scripts >/dev/null
cp -r /boot/PSPi/Scripts/. /home/pi/PSPi/Scripts

# Theme
cp -r /boot/PSPi/Console/. /etc/emulationstation/themes/carbon/PSPi/
cp -f /boot/pspi/Theme/background.png /etc/emulationstation/themes/carbon/art/carbon_fiber.png
cp -f /boot/PSPi/Theme/carbon.xml /etc/emulationstation/themes/carbon/carbon.xml

# Remove verbose startup/shutdown to shave time off startup
grep fastboot /boot/cmdline.txt >/dev/null
if [ $? -eq 0 ]; then
        echo "Found"
        # pspi already in rc.local, but make sure correct:
        #sed -i "s/^.*pspi.*$/bash \/boot\/pspi\/boot.sh/g" /etc/rc.local >/dev/null
else
        echo "Not Found, adding"
        # Insert pspi into rc.local before final 'exit 0'
        sed -i ' 1 s/.*/& fastboot/' /boot/cmdline.txt >/dev/null
fi
grep quiet /boot/cmdline.txt >/dev/null
if [ $? -eq 0 ]; then
        echo "Found"
        # pspi already in rc.local, but make sure correct:
        #sed -i "s/^.*pspi.*$/bash \/boot\/pspi\/boot.sh/g" /etc/rc.local >/dev/null
else
        echo "Not Found, adding"
        # Insert pspi into rc.local before final 'exit 0'
        sed -i ' 1 s/.*/& quiet/' /boot/cmdline.txt >/dev/null
fi



#remove DHCP wait, for faster bootup
rm -f /etc/systemd/system/dhcpcd.service.d/wait.conf 2>/dev/null
sleep 1
echo "Final Step --> select Interface Options, I2C, then enable I2C and reboot"
sleep 1
read -r -p "Press enter to begin the final step..." key
raspi-config
reboot
