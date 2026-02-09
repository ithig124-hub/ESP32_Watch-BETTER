/**
 * Sensor Interface - IMU (QMI8658)
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

bool initSensors();
void updateStepCounter();
int readAccelerometer(float* x, float* y, float* z);
int readGyroscope(float* x, float* y, float* z);

#endif
