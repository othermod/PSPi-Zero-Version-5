#ifndef JOYSTICKDEVICE_H
#define JOYSTICKDEVICE_H

#include <linux/input.h>
#include <linux/uinput.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int createUInputDevice();
void sendInputEvent(int fd, uint16_t type, uint16_t code, int32_t value);

#endif
