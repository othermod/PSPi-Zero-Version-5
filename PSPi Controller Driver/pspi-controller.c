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
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x02, BTN_DPAD_LEFT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x03, BTN_DPAD_RIGHT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x04, BTN_START);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x05, BTN_SELECT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x06, BTN_A);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x07, BTN_B);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x08, BTN_X);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x09, BTN_Y);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0A, BTN_TL);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0B, BTN_TR);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0C, BTN_1);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0D, BTN_2);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0E, BTN_3);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0x0F, BTN_4);


  // joystick axis
  uint8_t val = newStatus->axis0;
  if(val != status->axis0) {
	sendInputEvent(UInputFIle, EV_ABS, ABS_X, val);
  }

  val = newStatus->axis1;
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
      printf("Attempting to flash ATmega328P firmware using SPI\n");
      int system(const char *command);
      system("sudo avrdude -p m328p -c gpio -e -U flash:w:pspi.hex");
	sleep(1);
    }


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

    // sleep until next update
    usleep(UPDATE_FREQ);
  }

  // close file
  close(I2CFile);
  ioctl(UInputFIle, UI_DEV_DESTROY);
}
