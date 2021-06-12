#!/bin/bash
# https://www.othermod.com
if [ $(id -u) -ne 0 ]; then
	echo "Installer must be run as root."
	echo "Try 'sudo bash $0'"
	exit 1
fi


# turn power LED on and off
# this is routed to the RESET pin on the ATmega328p and it also disables audio buffers when set low
# should be set high to turn on the ATmega328p
# can also possibly be used to disable the ATmega328p and reduce power usage when the power switch is set to Hold
# dtparam=act_led_trigger=none in config.txt to prevent changes on the pin
echo 1 | sudo tee /sys/class/leds/led1/brightness
#echo 0 | sudo tee /sys/class/leds/led1/brightness

# see whether this does anything on CM4
# /usr/bin/tvservice -o

