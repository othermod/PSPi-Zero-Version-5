#include "JoystickDevice.h"

int createUInputDevice() {
  int fd;

  fd = open("/dev/uinput", O_WRONLY | O_NDELAY);
  if(fd < 0) {
    fprintf(stderr, "Can't open uinput device!\n");
    exit(1);
  }
  
    // device structure
  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));

  // init event  
  int ret = 0;
  ret |= ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ret |= ioctl(fd, UI_SET_EVBIT, EV_REL);

  // button
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_A);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_B);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_X);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_Y);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_TL);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_TR);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_START);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_UP);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);
  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_1);
//  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_2);
//  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_3);
//  ret |= ioctl(fd, UI_SET_KEYBIT, BTN_4);


  // axis
  ret |= ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ret |= ioctl(fd, UI_SET_ABSBIT, ABS_X);
  uidev.absmin[ABS_X] = 55;
  uidev.absmax[ABS_X] = 200;
  
  ret |= ioctl(fd, UI_SET_ABSBIT, ABS_Y);
  uidev.absmin[ABS_Y] = 55;
  uidev.absmax[ABS_Y] = 200;

  
  if(ret) {
    fprintf(stderr, "Error while configuring uinput device!\n");
    exit(1);
  }

  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "PSPi Controller");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 1;
  uidev.id.product = 5;
  uidev.id.version = 1;

  ret = write(fd, &uidev, sizeof(uidev));
  if(ioctl(fd, UI_DEV_CREATE)) {
    fprintf(stderr, "Error while creating uinput device!\n");
    exit(1);    
  }

  return fd;
}

void sendInputEvent(int fd, uint16_t type, uint16_t code, int32_t value) {
  struct input_event ev;

  memset(&ev, 0, sizeof(ev));

  ev.type = type;
  ev.code = code;
  ev.value = value;
  
  if(write(fd, &ev, sizeof(ev)) < 0) {
    fprintf(stderr, "Error while sending event to uinput device!\n");
  }

  // need to send a sync event
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(fd, &ev, sizeof(ev));
  if (write(fd, &ev, sizeof(ev)) < 0) {
    fprintf(stderr, "Error while sending event to uinput device!\n");
  }
}
