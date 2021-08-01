// http://elinux.org/Interfacing_with_I2C_Devices
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "I2C.h"
#include "JoystickDevice.h"

#define I2C_GAMEPAD_ADDRESS 0x18
#define UPDATE_FREQ 1000000/60 // ms (60Hz)

const int magicNumber = 17; // Number that corrects for internal resistance on LiPo battery
const int rolling = 64; // Number of readings being averaged together. This has to match the atmega variable
_Bool isCharging = 0;
_Bool previousIsCharging = 0;
_Bool isMute = 0;
_Bool previousIsMute = 0;
int line = 0;
int chargeStatus = 11;
int previousChargeStatus = 0;
int indicationVoltage = 0;
int rollingVoltage = 0;
int amperageDifference = 0;
int calculatedVoltage = 0;
int resolution;
uint16_t rawVolt;
uint16_t rawAmp;

typedef struct {
	uint16_t buttons; // button status
	uint8_t axis0; // first axis
	uint8_t axis1; // second axis
	uint16_t voltage; // raw voltage
	uint16_t amperage; // raw amperage
} I2CJoystickStatus;

int readI2CJoystick(int file, I2CJoystickStatus *status) {
	int s = readI2CSlave(file, I2C_GAMEPAD_ADDRESS, status, sizeof(I2CJoystickStatus));
	if(s != sizeof(I2CJoystickStatus))
		return -1; // error
	return 0; // no error
}

#define TestBitAndSendKeyEvent(oldValue, newValue, bit, event) if((oldValue & (1 << bit)) != (newValue & (1 << bit))) sendInputEvent(UInputFIle, EV_KEY, event, (newValue & (1 << bit)) == 0 ? 0 : 1);

void updateButtons(int UInputFIle, I2CJoystickStatus *newStatus, I2CJoystickStatus *status) {
	// update button event
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x00, BTN_A);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x01, BTN_B);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x02, BTN_X);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x03, BTN_Y);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x04, BTN_TL);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x05, BTN_TR);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x06, BTN_SELECT);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x07, BTN_START);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x08, BTN_DPAD_UP);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x09, BTN_DPAD_DOWN);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0A, BTN_DPAD_LEFT);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0B, BTN_DPAD_RIGHT);
	TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0C, BTN_1);
	// TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0D, BTN_2);
	// TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0E, BTN_3);
	// TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0F, BTN_4);
	uint8_t joystickValue = newStatus->axis0;
	if (joystickValue > 107 & joystickValue < 147) {joystickValue = 127;}
	if (joystickValue != status->axis0) {sendInputEvent(UInputFIle, EV_ABS, ABS_X, joystickValue);}
	joystickValue = newStatus->axis1;
	if (joystickValue > 107 & joystickValue < 147) {joystickValue = 127;}
	if (joystickValue != status->axis1) {sendInputEvent(UInputFIle, EV_ABS, ABS_Y, joystickValue);}
}

void updateJoystick(int UInputFIle, I2CJoystickStatus *newStatus, I2CJoystickStatus *status) {

}

void startLog() {
	FILE * fp;
	fp = fopen ("log.csv","w");
	fprintf (fp, "Line,rollingVoltage,amperageDifference,calculatedVoltage,indicationVoltage\n");
	fclose(fp);
}

void writeLog() {
	FILE * fp;
	fp = fopen ("log.csv","a");
	line++;
	fprintf (fp, "%d,%d,%d,%d,%d",line,rollingVoltage,amperageDifference,calculatedVoltage,indicationVoltage);
	fclose(fp);
}

void readController() {
}

void calculateBattery() {
	// a lot of this math is just converting the raw values to readable voltage
	// test efficiency to see whether it is worth working with raw values instead
	rollingVoltage = rawVolt * 11 * 3300 / 1024 / rolling;
	amperageDifference = (rawVolt - rawAmp) * 10 / 11;
	calculatedVoltage = rollingVoltage + amperageDifference * 10 / magicNumber;
	previousIsCharging = isCharging;
	if (indicationVoltage == 0) {indicationVoltage = calculatedVoltage;}
	if (isCharging == 0) {
		if (calculatedVoltage < indicationVoltage) { indicationVoltage--;}
		if (amperageDifference < 0 || rollingVoltage > 4200) {isCharging = 1;}
	} else {
		if (calculatedVoltage > indicationVoltage) { indicationVoltage++;}
		if (amperageDifference > 50) {isCharging = 0;}
	}
	previousChargeStatus = chargeStatus;
	chargeStatus = 0;
	if (indicationVoltage > 3478) {chargeStatus = 1;}
	if (indicationVoltage > 3549) {chargeStatus = 2;}
	if (indicationVoltage > 3619) {chargeStatus = 3;}
	if (indicationVoltage > 3655) {chargeStatus = 4;}
	if (indicationVoltage > 3725) {chargeStatus = 5;}
	if (indicationVoltage > 3761) {chargeStatus = 6;}
	if (indicationVoltage > 3866) {chargeStatus = 7;}
	if (indicationVoltage > 3927) {chargeStatus = 8;}
	if (indicationVoltage > 4027) {chargeStatus = 9;}
	if (indicationVoltage > 4175) {chargeStatus = 99;}
	if ((previousChargeStatus != chargeStatus) || (previousIsCharging != isCharging) || (previousIsMute != isMute)) {
		// Changing Battery Status
		char temp[512];
		system ("sudo killall pngview");
		sprintf(temp, "/home/pi/PSPi/Driver/./pngview -n -b 0 -l 100000 -x %d -y 2 /home/pi/PSPi/Driver/PNG/battery%d%d%d.png &",resolution - 46,isMute,isCharging,chargeStatus);
		system((char *)temp);
	}
}

int main(int argc, char *argv[]) {
	int I2CFile = openI2C(1); // open I2C device
	I2CJoystickStatus status; // current joystick status
	status.buttons = 0;
	status.axis0 = 0;
	status.axis1 = 0;
	int UInputFIle = createUInputDevice(); // create uinput device
	printf("PSPi Controller Starting\n");
	I2CJoystickStatus newStatus;
	if(readI2CJoystick(I2CFile, &newStatus) != 0) { // check for I2C connection
		printf("Controller is not detected on the I2C bus.\n");
		sleep(1);
    }
	sleep(1);
	// stores horizontal resolution to variable so this works with both LCD types
	// would be better to grab it directly, but I'll have to work out the method
	FILE *f = fopen("pspi.cfg","r");
	int i;
	char buf[4];
	for (i = 0 ; i != 0 ; i++) {
    		fgets(buf, 4, f);
	}
	fscanf(f, "%d", &resolution);

	while(1) {
	    I2CJoystickStatus newStatus; // read new status from I2C
		if(readI2CJoystick(I2CFile, &newStatus) != 0) {
			printf("Controller is not detected on the I2C bus.\n");
			sleep(1);
		} else { // everything is ok
			updateButtons(UInputFIle, &newStatus, &status);
			status = newStatus;
		}
		rawVolt = status.voltage;
		rawAmp = status.amperage;
		previousIsMute = isMute;
		isMute = (status.buttons >> 0x0F) & 1;
		calculateBattery();
		usleep(UPDATE_FREQ);
	}

	close(I2CFile); // close file
	ioctl(UInputFIle, UI_DEV_DESTROY);
}
