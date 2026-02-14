/*
 * touch.cpp - Touch Input Implementation
 * Direct I2C communication with FT3168
 */

#include "touch.h"
#include "config.h"

struct TouchCalibration {
  int min_x, max_x, min_y, max_y;
  bool calibrated;
} touch_cal;

static int last_touch_x = -1, last_touch_y = -1;
static unsigned long touch_start_time = 0;
static bool touch_pressed = false;
static int digital_crown_value = 0;

bool touchRead(uint16_t &x, uint16_t &y) {
  static unsigned long last_touch_read = 0;
  static uint16_t cached_x = 0, cached_y = 0;
  static bool cached_valid = false;
  
  if (millis() - last_touch_read < 20) {
    if (cached_valid) { x = cached_x; y = cached_y; return true; }
    return false;
  }
  last_touch_read = millis();
  
  Wire.beginTransmission(FT3168_ADDR);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) { cached_valid = false; return false; }
  
  Wire.requestFrom((uint8_t)FT3168_ADDR, (uint8_t)5);
  if (Wire.available() < 5) { cached_valid = false; return false; }
  
  uint8_t touches = Wire.read();
  if (touches == 0 || touches > 2) { cached_valid = false; return false; }
  
  uint8_t xh = Wire.read(), xl = Wire.read();
  uint8_t yh = Wire.read(), yl = Wire.read();
  
  x = ((xh & 0x0F) << 8) | xl;
  y = ((yh & 0x0F) << 8) | yl;
  cached_x = x; cached_y = y; cached_valid = true;
  return true;
}

bool touchReadEx(uint16_t &x, uint16_t &y, uint8_t &pressure) {
  Wire.beginTransmission(FT3168_ADDR);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) return false;
  
  Wire.requestFrom((uint8_t)FT3168_ADDR, (uint8_t)6);
  if (Wire.available() < 6) return false;
  
  uint8_t touches = Wire.read();
  if (touches == 0 || touches > 2) return false;
  
  uint8_t xh = Wire.read(), xl = Wire.read();
  uint8_t yh = Wire.read(), yl = Wire.read();
  pressure = Wire.read();
  
  x = ((xh & 0x0F) << 8) | xl;
  y = ((yh & 0x0F) << 8) | yl;
  return true;
}

bool initializeTouch() {
  Serial.println("[Touch] Initializing FT3168...");
  
  if (TP_INT >= 0) pinMode(TP_INT, INPUT_PULLUP);
  delay(100);
  
  Wire.beginTransmission(FT3168_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("[Touch] FT3168 detected");
    loadTouchCalibration();
    return true;
  }
  Serial.println("[Touch] FT3168 not responding");
  loadTouchCalibration();
  return false;
}

TouchGesture handleTouchInput() {
  TouchGesture gesture = {TOUCH_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, millis(), 0, false};
  
  if (TP_INT >= 0 && digitalRead(TP_INT) == HIGH) {
    if (touch_pressed) {
      touch_pressed = false;
      unsigned long duration = millis() - touch_start_time;
      gesture.event = TOUCH_RELEASE;
      gesture.x = last_touch_x; gesture.y = last_touch_y;
      gesture.duration = duration;
      gesture.is_valid = true;
      if (duration > 800) gesture.event = TOUCH_LONG_PRESS;
      else if (duration < 200) gesture.event = TOUCH_TAP;
    }
    return gesture;
  }
  
  uint16_t raw_x, raw_y;
  if (!touchRead(raw_x, raw_y)) {
    if (touch_pressed) {
      touch_pressed = false;
      unsigned long duration = millis() - touch_start_time;
      gesture.event = TOUCH_RELEASE;
      gesture.x = last_touch_x; gesture.y = last_touch_y;
      gesture.duration = duration;
      gesture.is_valid = true;
      if (duration > 800) gesture.event = TOUCH_LONG_PRESS;
      else if (duration < 200) gesture.event = TOUCH_TAP;
    }
    return gesture;
  }
  
  if (raw_x > 4095 || raw_y > 4095) return gesture;
  
  int touch_x = map(raw_x, touch_cal.min_x, touch_cal.max_x, 0, DISPLAY_WIDTH);
  int touch_y = map(raw_y, touch_cal.min_y, touch_cal.max_y, 0, DISPLAY_HEIGHT);
  touch_x = constrain(touch_x, 0, DISPLAY_WIDTH - 1);
  touch_y = constrain(touch_y, 0, DISPLAY_HEIGHT - 1);
  
  if (!touch_pressed) {
    touch_pressed = true;
    touch_start_time = millis();
    last_touch_x = touch_x; last_touch_y = touch_y;
    gesture.event = TOUCH_PRESS;
    gesture.x = touch_x; gesture.y = touch_y;
    gesture.start_x = touch_x; gesture.start_y = touch_y;
    gesture.is_valid = true;
  } else {
    int dx = touch_x - last_touch_x;
    int dy = touch_y - last_touch_y;
    
    if (abs(dx) > 5 || abs(dy) > 5) {
      gesture.event = TOUCH_MOVE;
      gesture.x = touch_x; gesture.y = touch_y;
      gesture.start_x = last_touch_x; gesture.start_y = last_touch_y;
      gesture.end_x = touch_x; gesture.end_y = touch_y;
      gesture.dx = dx; gesture.dy = dy;
      gesture.is_valid = true;
      digital_crown_value += dy;
      
      unsigned long duration = millis() - touch_start_time;
      if (duration > 100 && duration < 500) {
        if (abs(dx) > abs(dy) && abs(dx) > 50)
          gesture.event = (dx > 0) ? TOUCH_SWIPE_RIGHT : TOUCH_SWIPE_LEFT;
        else if (abs(dy) > abs(dx) && abs(dy) > 50)
          gesture.event = (dy > 0) ? TOUCH_SWIPE_DOWN : TOUCH_SWIPE_UP;
      }
      last_touch_x = touch_x; last_touch_y = touch_y;
    }
  }
  return gesture;
}

bool isTouchPressed() { return touch_pressed; }

void getTouchPosition(int& x, int& y) {
  x = last_touch_x; y = last_touch_y;
}

TouchEvent recognizeGesture(int start_x, int start_y, int end_x, int end_y, unsigned long duration) {
  int dx = end_x - start_x;
  int dy = end_y - start_y;
  int distance = sqrt(dx*dx + dy*dy);
  
  if (duration > 800 && distance < 20) return TOUCH_LONG_PRESS;
  if (duration < 300 && distance < 20) return TOUCH_TAP;
  if (distance > 50 && duration < 500) {
    if (abs(dx) > abs(dy)) return (dx > 0) ? TOUCH_SWIPE_RIGHT : TOUCH_SWIPE_LEFT;
    else return (dy > 0) ? TOUCH_SWIPE_DOWN : TOUCH_SWIPE_UP;
  }
  return TOUCH_NONE;
}

bool isSwipeGesture(int start_x, int start_y, int end_x, int end_y) {
  int dx = abs(end_x - start_x);
  int dy = abs(end_y - start_y);
  return (dx > 50 || dy > 50);
}

bool isTapGesture(unsigned long duration, int movement) {
  return duration < 300 && movement < 20;
}

void calibrateTouch() {
  touch_cal.min_x = 0; touch_cal.max_x = DISPLAY_WIDTH;
  touch_cal.min_y = 0; touch_cal.max_y = DISPLAY_HEIGHT;
  touch_cal.calibrated = true;
}

void saveTouchCalibration() {}

void loadTouchCalibration() {
  touch_cal.min_x = 0; touch_cal.max_x = DISPLAY_WIDTH;
  touch_cal.min_y = 0; touch_cal.max_y = DISPLAY_HEIGHT;
  touch_cal.calibrated = true;
}

int getDigitalCrownValue() { return digital_crown_value; }
void resetDigitalCrown() { digital_crown_value = 0; }

bool recoverTouchI2C() {
  Wire.beginTransmission(FT3168_ADDR);
  return Wire.endTransmission() == 0;
}

bool checkAndRecoverTouchI2C() {
  static unsigned long last_attempt = 0;
  if (millis() - last_attempt < 5000) return false;
  last_attempt = millis();
  return recoverTouchI2C();
}
