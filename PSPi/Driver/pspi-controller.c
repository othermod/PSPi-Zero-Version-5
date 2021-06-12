

// http://elinux.org/Interfacing_with_I2C_Devices
#include <stdio.h>
#include <stdlib.h>
#include "I2C.h"
#include "JoystickDevice.h"

#define I2C_GAMEPAD_ADDRESS 0x18
#define UPDATE_FREQ 16666 // ms (60Hz)

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

void updateUInputDevice(int UInputFIle, I2CJoystickStatus *newStatus, I2CJoystickStatus *status) {
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
//  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0D, BTN_2);
//  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0E, BTN_3);
//  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0F, BTN_4);
  // joystick axis
  uint8_t val = newStatus->axis0;
  if (val > 107 & val < 147) {val = 127;}
  if (val != status->axis0) {sendInputEvent(UInputFIle, EV_ABS, ABS_X, val);}
  val = newStatus->axis1;
  if (val > 107 & val < 147) {val = 127;}
  if (val != status->axis1) {sendInputEvent(UInputFIle, EV_ABS, ABS_Y, val);}
}

int main(int argc, char *argv[]) {
  // open I2C device
  int I2CFile = openI2C(1);

  // current joystick status
  I2CJoystickStatus status;
  status.buttons = 0;
  status.axis0 = 0;
  status.axis1 = 0;

  // create uinput device
  int UInputFIle = createUInputDevice();

  // check for I2C connection and attempt to update firmware if not detected
  printf("PSPi Controller Starting\n");
  I2CJoystickStatus newStatus;
   if(readI2CJoystick(I2CFile, &newStatus) != 0) {

      printf("Controller is not detected on the I2C bus.\n");
	  sleep(1);
    }

//edit names so its more clear what is what

sleep(1);

int IsCharging = 0;
int PreviousIsCharging = 0;
const int MagicNumber = 17;
int ChargeStatus = 0;
int PreviousChargeStatus = 0;

//AVGvolt and AVGamp are the ADC readings averaged over the most recent 64 readings
uint16_t AVGvolt = status.voltage;
uint16_t AVGamp = status.amperage;

//rolling is the number is number of readings being averaged together
const int rolling = 64;

int IndicationVoltage = 0;

FILE * fp;
fp = fopen ("log.csv","w");
fprintf (fp, "Line,RollingVoltage,AmperageDifference,CalculatedVoltage,IndicationVoltage\n");
fclose(fp);

int line = 0;
int count = 0;
system("/boot/PSPi/Driver/./pngview -n -b 0 -l 100000 -x 765 -y 5 /boot/PSPi/Driver/PNG/battery10.png &");
  while(1) {
//	printf("%d\n", status.voltage);
//	printf("%d\n", status.amperage);
    // read new status from I2C
    I2CJoystickStatus newStatus;
    if(readI2CJoystick(I2CFile, &newStatus) != 0) {
      printf("Controller is not detected on the I2C bus.\n");
	sleep(1);
    } else {
      // everything is ok
      updateUInputDevice(UInputFIle, &newStatus, &status);
      status = newStatus;
    }

	AVGvolt = status.voltage;
	AVGamp = status.amperage;
//	printf("%d\n", IndicationVoltage);
	//printf("%d\n", status.axis1);
	//printf("%d\n", status.amperage);
	int RollingVoltage = AVGvolt * 11 * 3300 / 1024 / rolling;
	int AmperageDifference = (AVGvolt - AVGamp) * 10 / 11;
	int CalculatedVoltage = RollingVoltage + AmperageDifference * 10 / MagicNumber;
	PreviousIsCharging = IsCharging;
	if (IndicationVoltage == 0) {IndicationVoltage = CalculatedVoltage;}
	if (IsCharging == 0) {
	//printf("\nDischarging");
		if (CalculatedVoltage < IndicationVoltage) { IndicationVoltage--;}
		if (AmperageDifference < 10 || RollingVoltage > 4200) {IsCharging = 1;}
	} else {
	//printf("\nCharging");
		if (CalculatedVoltage > IndicationVoltage) { IndicationVoltage++;}
		if (AmperageDifference > 100) {IsCharging = 0;}
		}
	PreviousChargeStatus = ChargeStatus;
	ChargeStatus = 0;
	if (IndicationVoltage > 3478) {ChargeStatus = 1;}
	if (IndicationVoltage > 3549) {ChargeStatus = 2;}
	if (IndicationVoltage > 3619) {ChargeStatus = 3;}
	if (IndicationVoltage > 3655) {ChargeStatus = 4;}
	if (IndicationVoltage > 3725) {ChargeStatus = 5;}
	if (IndicationVoltage > 3761) {ChargeStatus = 6;}
	if (IndicationVoltage > 3866) {ChargeStatus = 7;}
	if (IndicationVoltage > 3927) {ChargeStatus = 8;}
	if (IndicationVoltage > 4027) {ChargeStatus = 9;}
	if (IndicationVoltage > 4200) {ChargeStatus = 99;}
	if ((PreviousChargeStatus != ChargeStatus) || (PreviousIsCharging != IsCharging)) {
		//printf("\nChanging Battery Status");
		char temp[512];
		sprintf(temp, "/boot/PSPi/Driver/./pngviewtemp -n -b 0 -l 100000 -x 765 -y 5 /boot/PSPi/Driver/PNG/battery%d%d.png &",IsCharging,ChargeStatus);
		system((char *)temp);
		system ("sudo killall pngview");
		sprintf(temp, "/boot/PSPi/Driver/./pngview -n -b 0 -l 100000 -x 765 -y 5 /boot/PSPi/Driver/PNG/battery%d%d.png &",IsCharging,ChargeStatus);
		system((char *)temp);
		system ("sudo killall pngviewtemp");
	}
	count++;
	if (count == 60) { //log once a second
		FILE * fp;
		fp = fopen ("log.csv","a");
		line++;
		fprintf (fp, "%d",line);
		fprintf (fp, ",%d",RollingVoltage);
		fprintf (fp, ",%d",AmperageDifference);
		fprintf (fp, ",%d",CalculatedVoltage);
		fprintf (fp, ",%d\n",IndicationVoltage);
		fclose(fp);
		count = 0;
	}
    // sleep until next update
    usleep(UPDATE_FREQ);
  }

  // close file
  close(I2CFile);
  ioctl(UInputFIle, UI_DEV_DESTROY);
}
