#include <Wire.h>
#include <avr/sleep.h>

// ------------------------------------------
// Configuration
// ------------------------------------------
#define I2C_ADDRESS 0x18
#define SWITCH_DEBOUNCE_TIME 50 // ms

//Analog
#define ANALOG_PIN_X 6
#define ANALOG_PIN_Y 7

// switch type definition
#define BTN_0           0x00
#define BTN_1           0x01
#define BTN_2           0x02
#define BTN_3           0x03
#define BTN_4           0x04
#define BTN_5           0x05
#define BTN_6           0x06
#define BTN_7           0x07
#define BTN_8           0x08
#define BTN_9           0x09
#define BTN_10          0x0A
#define BTN_11          0x0B
#define BTN_12          0x0C
#define BTN_13          0x0D
#define BTN_14          0x0E
#define BTN_15          0x0F

struct InputSwitch {
  unsigned char pin;
  unsigned char state;
  unsigned long time;
  unsigned char code;
};

// the (up to 16) switches (arduino pin #, state, time, code)
InputSwitch switches[] = {
  {0, HIGH, 0, BTN_0},
  {1, HIGH, 0, BTN_1},
  {2, HIGH, 0, BTN_2},
  {3, HIGH, 0, BTN_3},
  {4, HIGH, 0, BTN_4},
  {5, HIGH, 0, BTN_5},
  {6, HIGH, 0, BTN_6},
  {7, HIGH, 0, BTN_7},
  {8, HIGH, 0, BTN_8},
  {9, HIGH, 0, BTN_9},
  {10, HIGH, 0, BTN_10},
  {11, HIGH, 0, BTN_11},
  {12, HIGH, 0, BTN_12},
  {13, HIGH, 0, BTN_13},
  {14, HIGH, 0, BTN_14},
  {15, HIGH, 0, BTN_15},
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
};

I2CJoystickStatus joystickStatus;

void (*stateFunction)(void);
unsigned long wakeUpTime = 0;

void runState() {
  // update switch etc
  scanAnalog();
  scanInput();
}

void setup()
{
  Wire.begin(I2C_ADDRESS);      // join i2c bus 
  Wire.onRequest(requestEvent); // register event

  // default status
  joystickStatus.buttons = 0;
  joystickStatus.axis0 = 127;
  joystickStatus.axis1 = 127;
  
  // pin configuration
  for(int i = 0; i < sizeof(switches) / sizeof(InputSwitch); i++) {
    pinMode(switches[i].pin, INPUT_PULLUP);
  }
  stateFunction = runState;
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

void scanAnalog() {
  // read analog stick values, reduce to uint8_t
  uint8_t x = analogRead(ANALOG_PIN_X) / 4;
  uint8_t y = analogRead(ANALOG_PIN_Y) / 4;
  // deadzone
  if (x > 122 & x < 132) {
    x = 127;
  }
  if (y > 122 & y < 132) {
    y = 127;
  }
  // store them in the I2C data
  joystickStatus.axis0 = x;
  joystickStatus.axis1 = y;
}

void loop() {
  // execute state function
  stateFunction();
  delay(10);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write((char *)&joystickStatus, sizeof(I2CJoystickStatus)); 
}
