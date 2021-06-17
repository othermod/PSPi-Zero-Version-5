#include <Wire.h>
#include <avr/sleep.h>
#include "pins_arduino.h"

// ------------------------------------------
// Configuration
// ------------------------------------------
#define I2C_ADDRESS 0x18
#define SWITCH_DEBOUNCE_TIME 50 // ms

//Analog
#define ANALOG_PIN_X 7
#define ANALOG_PIN_Y 6
#define VOLTAGE_PIN 2
#define AMPERAGE_PIN 3

//Digital
#define LOWBATT_PIN 12
#define MUTE_PIN 13
int PWMarray[] = {55,62,69,75,80,90,100,255};
int PWMposition = 1;
int BTN_DISPLAY_STATUS = 0;
int MUTE_STATUS = 0;
const int rolling = 64;
uint16_t AVGvolt = 119 * rolling;
uint16_t AVGamp = 119 * rolling;

//PWM
#define PWM_PIN 5

// switch type definition
#define BTN_A           0x00
#define BTN_B           0x01
#define BTN_X           0x02
#define BTN_Y           0x03
#define BTN_TL          0x04
#define BTN_TR          0x05
#define BTN_SELECT      0x06
#define BTN_START       0x07
#define BTN_DPAD_UP     0x08
#define BTN_DPAD_DOWN   0x09
#define BTN_DPAD_LEFT   0x0A
#define BTN_DPAD_RIGHT  0x0B
#define BTN_1           0x0C
#define BTN_DISPLAY     0x0D
#define BTN_MUTE        0x0E
#define MUTE_OUTPUT     0x0F

struct InputSwitch {
  unsigned char pin;
  unsigned char state;
  unsigned long time;
  unsigned char code;
};

// the (up to 16) buttons (arduino pin #, state, time, code)
InputSwitch switches[] = {
  {4,  HIGH, 0, BTN_A},          // A
  {6,  HIGH, 0, BTN_B},          // B
  {20, HIGH, 0, BTN_X},          // X
  {21, HIGH, 0, BTN_Y},          // Y
  {14, HIGH, 0, BTN_TL},         // LTRIGGER
  {3,  HIGH, 0, BTN_TR},         // RTRIGGER
  {8,  HIGH, 0, BTN_SELECT},     // SELECT
  {7,  HIGH, 0, BTN_START},      // START
  {0,  HIGH, 0, BTN_DPAD_UP},    // UP
  {1,  HIGH, 0, BTN_DPAD_DOWN},  // DOWN
  {15, HIGH, 0, BTN_DPAD_LEFT},  // LEFT
  {2,  HIGH, 0, BTN_DPAD_RIGHT}, // RIGHT
  {10, HIGH, 0, BTN_1},          // HOME
  {9,  HIGH, 0, BTN_DISPLAY},    // DISPLAY BUTTON
  {11, HIGH, 0, BTN_MUTE},       // MUTE BUTTON
  {13, HIGH, 0, MUTE_OUTPUT},    // MUTE STATUS
};

// return true if switch state has changed!
bool updateSwitch(struct InputSwitch *sw) {
  int newState = digitalRead(sw->pin);
  if(newState != sw->state && millis() - sw->time > SWITCH_DEBOUNCE_TIME) {
    // change state!
    sw->state = newState;
    // record last update
    sw->time = millis();
    return true;
  }

  // else 
  return false;
}

// I2C data definition
struct I2CJoystickStatus {
  uint16_t buttons; // button status
  uint8_t axis0; // first axis
  uint8_t axis1; // second axis
  uint16_t voltage; //
  uint16_t amperage; //
};

I2CJoystickStatus joystickStatus;

void setup()
{
  Wire.begin(I2C_ADDRESS);      // join i2c bus 
  Wire.onRequest(requestEvent); // register event

  //Blink the Low Battery LED to show that ATMega is starting
  pinMode(LOWBATT_PIN, OUTPUT);
  digitalWrite(LOWBATT_PIN, 0);
  delay(500);
  digitalWrite(LOWBATT_PIN, 1);
  delay(500);
  digitalWrite(LOWBATT_PIN, 0);
  delay(500);
  digitalWrite(LOWBATT_PIN, 1);
  delay(500);
  digitalWrite(LOWBATT_PIN, 0);

  // default button and joystick status
  joystickStatus.buttons = 0;
  joystickStatus.axis0 = 127;
  joystickStatus.axis1 = 127;
  
  // pin configuration
  for(int i = 0; i < sizeof(switches) / sizeof(InputSwitch); i++) {
    pinMode(switches[i].pin, INPUT_PULLUP);
  }

  //Unmute audio at startup. Was previously muted by pin config
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  //Start PWM
  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, PWMarray[PWMposition]);
  //these are already done in the loop above, remove after verifying
//  pinMode(9, INPUT_PULLUP);
//  pinMode(11, INPUT_PULLUP);
}

void scanAnalog() {
  // read analog stick values, change to 8 bit because it doesnt need more accuracy
  // store them in the I2C data
  joystickStatus.axis0 = analogRead(ANALOG_PIN_X) / 4;
  joystickStatus.axis1 = analogRead(ANALOG_PIN_Y) / 4;
  
  // Read raw power status. Keep a rolling average of the readings. Divide the readings by 64 on Pi to get the ADC value.
  AVGvolt = AVGvolt - (AVGvolt / rolling) + analogRead(VOLTAGE_PIN);
  joystickStatus.voltage = AVGvolt;
  AVGamp = AVGamp - (AVGamp / rolling) + analogRead(AMPERAGE_PIN);
  joystickStatus.amperage = AVGamp;
// orange LED when battery is below 3.3v
  if (joystickStatus.voltage < 6000) {digitalWrite(LOWBATT_PIN, 1);}
//  green LED when battery is above 3.5v
  if (joystickStatus.voltage > 6300) {digitalWrite(LOWBATT_PIN, 0);}
}

void scanInput() {
  for(int i = 0; i < sizeof(switches) / sizeof(InputSwitch); i++) {
    if(updateSwitch(&switches[i])) {
      if(switches[i].state == HIGH) // button released
        joystickStatus.buttons &= ~(1 << switches[i].code);
      else // button pressed
        joystickStatus.buttons |= (1 << switches[i].code);
    }
  }
  static uint16_t oldButtons = 0;
  if(joystickStatus.buttons != oldButtons) {
    oldButtons = joystickStatus.buttons;
  }
}

void loop() {
  scanAnalog();
  scanInput();
  // clean up and maybe combine with scanInput since tasks are being performed twice
    if(digitalRead(9) == 0 and BTN_DISPLAY_STATUS == 0){         // If button is pressed
    BTN_DISPLAY_STATUS = 1;
    PWMposition++;
    if(PWMposition > 7) {PWMposition = 0;}
    analogWrite(PWM_PIN, PWMarray[PWMposition]);
    }
    if(digitalRead(9) == 1 and BTN_DISPLAY_STATUS == 1){         // If button is not pressed
    BTN_DISPLAY_STATUS = 0;
    }
    if(digitalRead(11) == 0 and MUTE_STATUS == 0){         // If button is pressed
    digitalWrite(13, (! digitalRead(13)));
    MUTE_STATUS = 1;
    }
    if(digitalRead(11) == 1 and MUTE_STATUS == 1){         // If button is not pressed
    MUTE_STATUS = 0;
    }
  delay(10);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write((char *)&joystickStatus, sizeof(I2CJoystickStatus)); 
}
