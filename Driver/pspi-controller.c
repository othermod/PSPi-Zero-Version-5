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
  uint8_t voltage; // raw voltage, will shift averaging to microcontroller before release
  uint8_t amperage; // raw amperage, will shift averaging to microcontroller before release

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
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x00, BTN_DPAD_UP);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x01, BTN_DPAD_DOWN);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x02, BTN_DPAD_RIGHT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x03, BTN_TR);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x04, BTN_A);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x05, BTN_B);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x06, BTN_START);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x07, BTN_SELECT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x08, BTN_1);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x09, BTN_2);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0A, BTN_TL);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0B, BTN_DPAD_LEFT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0C, BTN_X);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0D, BTN_Y);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0E, BTN_3);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0F, BTN_4);


  // joystick axis
  uint8_t val = newStatus->axis0;
//temp fix
//	val = 127;

  if (val > 107 & val < 147) {
    val = 127;
  }
  if(val != status->axis0) {
	sendInputEvent(UInputFIle, EV_ABS, ABS_X, val);
  }
//	printf("X Axis: %d\n", val);
  val = newStatus->axis1;
  if (val > 107 & val < 147) {
    val = 127;
  }
//temp fix
//	val = 127;
//	printf("Y Axis: %d\n", val);
  if(val != status->axis1) {
	sendInputEvent(UInputFIle, EV_ABS, ABS_Y, val);
  }
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


int rolling = 64;
//add short loop and spend a couple seconds calculating the actual voltage and condition so the initial icon is correct
//edit names so its more clear what is what
int BatteryVoltage = 4200;
//0 means discharging
int IsCharging = 0;
int MagicNumber = 30;
int ChargeStatus = 1;
uint16_t AVGvolt = 119 * rolling;
uint16_t AVGamp = 119 * rolling;
FILE * fp;
fp = fopen ("log.csv","w");
fprintf (fp, "Line,RollingVoltage,difference,truevoltage,BatteryVoltage\n");
fclose(fp);

//int var = 0;
int line = 0;
int count = 0;
system("/home/pi/driver/./pngview -n -b 0 -l 100000 -x 765 -y 5 /home/pi/driver/battery10.png &");
  while(1) {
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
	AVGvolt = AVGvolt - (AVGvolt / rolling) + (status.voltage);
	AVGamp = AVGamp - (AVGamp / rolling) + (status.amperage);

//	printf("%d\n", status.axis0);
//	printf("%d\n", status.axis1);
//	printf("%d\n", status.amperage);
	int RollingVoltage = AVGvolt * 11 * 3300 / 1024 / rolling;
	int difference = (AVGvolt - AVGamp) * 10 / 11;
	int truevoltage = RollingVoltage + difference * 10 / MagicNumber;
	int PreviousIsCharging = IsCharging;
		if (IsCharging == 0) {
//			printf("\nDischarging");
			if (truevoltage < BatteryVoltage) { BatteryVoltage--;}
			if (difference < 10) {IsCharging = 1;}
		} else {
//			printf("\nCharging");
			if (truevoltage > BatteryVoltage) { BatteryVoltage++;}
			if (difference > 100) {IsCharging = 0;}
		}
		int PreviousChargeStatus = ChargeStatus;
		ChargeStatus = 0;
		if (BatteryVoltage > 3478) {ChargeStatus = 1;}
		if (BatteryVoltage > 3549) {ChargeStatus = 2;}
		if (BatteryVoltage > 3619) {ChargeStatus = 3;}
		if (BatteryVoltage > 3655) {ChargeStatus = 4;}
		if (BatteryVoltage > 3725) {ChargeStatus = 5;}
		if (BatteryVoltage > 3761) {ChargeStatus = 6;}
		if (BatteryVoltage > 3866) {ChargeStatus = 7;}
		if (BatteryVoltage > 3927) {ChargeStatus = 8;}
		if (BatteryVoltage > 4027) {ChargeStatus = 9;}
		if (BatteryVoltage > 4200) {ChargeStatus = 99;}
		if ((PreviousChargeStatus != ChargeStatus) || (PreviousIsCharging != IsCharging)) {
			printf("\nChanging Battery Status");
			char temp[512];
			sprintf(temp, "/home/pi/driver/./pngviewtemp -n -b 0 -l 100000 -x 765 -y 5 /home/pi/driver/battery%d%d.png &",IsCharging,ChargeStatus);
			system((char *)temp);
			system ("sudo killall pngview");
			sprintf(temp, "/home/pi/driver/./pngview -n -b 0 -l 100000 -x 765 -y 5 /home/pi/driver/battery%d%d.png &",IsCharging,ChargeStatus);
			system((char *)temp);
			system ("sudo killall pngviewtemp");
		}
	count++;
	//60 count means log every second. 
	//Logging will be disabled before release of the PSPi Zero.
	if (count == 60) {
		FILE * fp;
		fp = fopen ("log.csv","a");
		line++;
		fprintf (fp, "%d",line);
		fprintf (fp, ",%d",RollingVoltage);
		fprintf (fp, ",%d",difference);
		fprintf (fp, ",%d",truevoltage);
	        fprintf (fp, ",%d\n",BatteryVoltage);
		fclose(fp);
		//	printf("Difference is %d\n\n", ampavg);
		//this corrects for the fact that I put the volt detect at bat-. might be doubled or halved. verify
		//	printf("Battery voltage should be %d\n", calcvolt - calcamp / 20);
		//	if (calcamp > calcvolt) 	{printf("Battery is charging at %d milliamps\n", (calcamp-calcvolt) * 15);}
		//	else 			{printf("Battery is discharging at %d milliamps\n", (calcvolt-calcamp) * 15 );}
		//	printf("Calculated battery voltage is %d\n", calcvolt - calcamp / 2 );
			count = 0;
	}
    // sleep until next update
    usleep(UPDATE_FREQ);
  }

  // close file
  close(I2CFile);
  ioctl(UInputFIle, UI_DEV_DESTROY);
}
