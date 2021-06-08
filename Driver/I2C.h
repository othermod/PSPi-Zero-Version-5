#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

// return a file handler to the I2C device
int openI2C(int deviceIdx);
int readI2CSlave(int file, int slaveAddress, void *buf, size_t count);

#endif
