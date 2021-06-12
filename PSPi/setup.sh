# add boot script to rc.local
grep pspi /etc/rc.local >/dev/null
if [ $? -eq 0 ]; then
	echo "pspi already exists in rc.local. Re-creating."
	# pspi already in rc.local, but make sure correct:
	sed -i "s/^.*pspi.*$/bash \/boot\/pspi\/boot.sh/g" /etc/rc.local >/dev/null
else
	echo "pspi doesn't exist in rc.local. Creating."
	# Insert pspi into rc.local before final 'exit 0'
	sed -i "s/^exit 0/bash \/boot\/pspi\/boot.sh\\nexit 0/g" /etc/rc.local >/dev/null
fi

# enable I2C on Raspberry Pi
echo '>>> Enable I2C'
if grep -q 'i2c-bcm2708' /etc/modules; then
  echo 'Seems i2c-bcm2708 module already exists, skip this step.'
else
  echo 'i2c-bcm2708' >> /etc/modules
fi
if grep -q 'i2c-dev' /etc/modules; then
  echo 'Seems i2c-dev module already exists, skip this step.'
else
  echo 'i2c-dev' >> /etc/modules
fi

echo '>>> Copying Controller Configs'
cp -f /boot/pspi/configs/es_input.cfg /opt/retropie/configs/all/emulationstation/es_input.cfg
cp -f /boot/pspi/configs/retroarch.cfg /opt/retropie/configs/all/retroarch.cfg
cp -f "/boot/pspi/configs/PSPi Controller.cfg" "/opt/retropie/configs/all/retroarch-joypads/PSPi Controller.cfg"
