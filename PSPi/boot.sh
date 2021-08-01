
# check resolution and create config file
cat /sys/class/graphics/fb0/virtual_size | cut -d, -f1 > pspi.cfg
mv pspi.cfg /home/pi/PSPi/Driver/pspi.cfg

cd /home/pi/PSPi/Driver
sudo ./pspi-controller &
