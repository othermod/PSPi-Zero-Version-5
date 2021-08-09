import RPi.GPIO as GPIO
import time
import os

GPIO_PIN=11

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.IN, pull_up_down = GPIO.PUD_UP)

os.system("sudo killall BLINKY 2>/dev/null")
os.system("./BLINKY &")
os.system("sudo killall -SIGSTOP BLINKY")
os.system("./LED_OFF")
while 1:
	if (GPIO.input(11)==0):
		os.system("sudo killall -SIGSTOP retroarch 2>/dev/null")
		os.system("sudo killall -SIGSTOP emulationstatio 2>/dev/null")
		os.system("sudo killall -SIGSTOP pspi-controller 2>/dev/null")
		time.sleep(1)
		os.system("./LCD_OFF")
		os.system("./AUDIO_OFF")
		os.system("sudo killall -CONT BLINKY 2>/dev/null")
		while(GPIO.input(11)==0):
			time.sleep(1)
		os.system("sudo killall -SIGSTOP BLINKY 2>/dev/null")
		os.system("./LED_OFF")
		os.system("sudo killall -CONT retroarch 2>/dev/null")
		os.system("sudo killall -CONT pspi-controller 2>/dev/null")
		os.system("sudo killall -CONT emulationstatio 2>/dev/null")
		os.system("./LCD_ON")
		os.system("./AUDIO_ON")
	time.sleep(1)

