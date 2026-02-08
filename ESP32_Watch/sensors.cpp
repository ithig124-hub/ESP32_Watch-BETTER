/**
 * Sensors Implementation - Fixed function order
 */

#include "sensors.h"
#include <Wire.h>

IMUData imuData = {0, 0, 0, 0, 0, 0, 0};

static uint8_t imuAddr = 0;
static float lastMagnitude = 1.0;
static unsigned long lastStepTime = 0;

// Step detection parameters
#define STEP_THRESHOLD 1.15
#define STEP_MIN_INTERVAL 300 // ms between steps

// Forward declarations of helper functions
static void writeReg(uint8_t reg, uint8_t val);
static uint8_t readReg(uint8_t reg);
static bool initQMI8658();

// ═══════════════════════════════════════════════════════════════════════════════
//  HELPER FUNCTIONS (defined first)
// ═══════════════════════════════════════════════════════════════════════════════
static void writeReg(uint8_t reg, uint8_t val) {
  if (imuAddr == 0) return;
  Wire.beginTransmission(imuAddr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

static uint8_t readReg(uint8_t reg) {
  if (imuAddr == 0) return 0;
  Wire.beginTransmission(imuAddr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(imuAddr, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

static bool initQMI8658() {
  if (imuAddr == 0) return false;
  
  // Reset the sensor
  writeReg(0x60, 0x01); // CTRL1
  delay(50);
  
  // Configure accelerometer: +/- 4g, 256Hz ODR
  writeReg(0x03, 0x44); // CTRL2
  
  // Configure gyroscope: +/- 512 dps, 256Hz ODR
  writeReg(0x04, 0x44); // CTRL3
  
  // Enable sensors
  writeReg(0x08, 0x03); // CTRL7
  
  hasIMU = true;
  Serial.println("[OK] QMI8658 initialized");
  return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PUBLIC FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
bool initSensors() {
  // Try primary address first
  Wire.beginTransmission(QMI8658_ADDR_PRIMARY);
  if (Wire.endTransmission() == 0) {
    imuAddr = QMI8658_ADDR_PRIMARY;
    Serial.println("[OK] IMU found at 0x6A");
    return initQMI8658();
  }
  
  // Try secondary address
  Wire.beginTransmission(QMI8658_ADDR_SECONDARY);
  if (Wire.endTransmission() == 0) {
    imuAddr = QMI8658_ADDR_SECONDARY;
    Serial.println("[OK] IMU found at 0x6B");
    return initQMI8658();
  }
  
  Serial.println("[WARN] IMU not found");
  return false;
}

bool readIMU() {
  if (imuAddr == 0 || !hasIMU) return false;
  
  uint8_t buf[12];
  
  // Read accel and gyro data
  Wire.beginTransmission(imuAddr);
  Wire.write(0x35); // Data start register
  if (Wire.endTransmission(false) != 0) return false;
  
  Wire.requestFrom(imuAddr, (uint8_t)12);
  for (int i = 0; i < 12 && Wire.available(); i++) {
    buf[i] = Wire.read();
  }
  
  // Parse accelerometer (2g scale factor)
  int16_t ax = (buf[1] << 8) | buf[0];
  int16_t ay = (buf[3] << 8) | buf[2];
  int16_t az = (buf[5] << 8) | buf[4];
  
  imuData.accelX = ax / 16384.0; // Convert to g
  imuData.accelY = ay / 16384.0;
  imuData.accelZ = az / 16384.0;
  
  // Parse gyroscope
  int16_t gx = (buf[7] << 8) | buf[6];
  int16_t gy = (buf[9] << 8) | buf[8];
  int16_t gz = (buf[11] << 8) | buf[10];
  
  imuData.gyroX = gx / 64.0; // Convert to dps
  imuData.gyroY = gy / 64.0;
  imuData.gyroZ = gz / 64.0;
  
  return true;
}

void updateStepCounter() {
  if (!readIMU()) return;
  
  // Calculate acceleration magnitude
  float magnitude = sqrt(
    imuData.accelX * imuData.accelX +
    imuData.accelY * imuData.accelY +
    imuData.accelZ * imuData.accelZ
  );
  
  unsigned long now = millis();
  
  // Simple step detection: threshold crossing with debounce
  if (lastMagnitude < STEP_THRESHOLD && magnitude >= STEP_THRESHOLD) {
    if (now - lastStepTime > STEP_MIN_INTERVAL) {
      watch.steps++;
      userData.totalSteps++;
      lastStepTime = now;
    }
  }
  
  lastMagnitude = magnitude;
}

void resetSteps() {
  watch.steps = 0;
}

bool detectWristRaise() {
  if (!readIMU()) return false;
  
  // Detect wrist raise gesture (tilt towards user)
  return (imuData.accelX > 0.5 && imuData.accelZ < 0.3);
}

bool detectDoubleTap() {
  static unsigned long lastTapTime = 0;
  static int tapCount = 0;
  
  if (!readIMU()) return false;
  
  // Simple tap detection based on acceleration spike
  float magnitude = sqrt(
    imuData.accelX * imuData.accelX +
    imuData.accelY * imuData.accelY +
    imuData.accelZ * imuData.accelZ
  );
  
  if (magnitude > 2.0) { // Strong tap
    unsigned long now = millis();
    if (now - lastTapTime < 400) { // Within double-tap window
      tapCount++;
      if (tapCount >= 2) {
        tapCount = 0;
        return true;
      }
    } else {
      tapCount = 1;
    }
    lastTapTime = now;
  }
  
  return false;
}
