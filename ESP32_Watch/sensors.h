/**
 * Sensors Module - IMU (QMI8658) and Step Detection
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

// IMU I2C addresses
#define QMI8658_ADDR_PRIMARY   0x6A
#define QMI8658_ADDR_SECONDARY 0x6B

// Sensor data structure
struct IMUData {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float temperature;
};

extern IMUData imuData;

// Initialize sensors
bool initSensors();

// Read IMU data
bool readIMU();

// Step detection
void updateStepCounter();
void resetSteps();

// Gesture detection
bool detectWristRaise();
bool detectDoubleTap();

#endif
