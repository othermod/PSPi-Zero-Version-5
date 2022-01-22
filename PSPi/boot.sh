# disable HDMI to save power
sudo tvservice -o

# disable the built-in LED on the Pi Zero 2 W
echo 29 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio29/direction
echo 1   > /sys/class/gpio/gpio29/value

cd /home/pi/PSPi/Driver

# start driver
sudo ./osd -gamepad &
