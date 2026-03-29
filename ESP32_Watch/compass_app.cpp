/*
 * compass_app.cpp - Digital Compass Implementation
 * Quick Win Feature: Magnetometer-based compass
 */

#include "compass_app.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"
#include <Wire.h>
#include <math.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Compass state
CompassData compass = {0, 0, 0, 0, false, 0};

// QMI8658 Registers
#define QMI8658_MAG_X_L    0x33
#define QMI8658_MAG_X_H    0x34
#define QMI8658_MAG_Y_L    0x35
#define QMI8658_MAG_Y_H    0x36
#define QMI8658_MAG_Z_L    0x37
#define QMI8658_MAG_Z_H    0x38

// =============================================================================
// INITIALIZATION
// =============================================================================

void initCompassApp() {
  compass.calibrated = false;
  compass.heading = 0;
  compass.last_update = 0;
  
  // Note: QMI8658 initialization is done in hardware.cpp
  Serial.println("[Compass] Compass app initialized");
}

// =============================================================================
// COMPASS SCREEN
// =============================================================================

void drawCompassApp() {
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 55, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 55, LCD_WIDTH, theme->primary);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH/2 - 48, 18);
  gfx->print("Compass");
  
  int centerX = LCD_WIDTH / 2;
  int centerY = 240;
  int outerRadius = 120;
  int innerRadius = 100;
  
  // Update compass reading
  updateCompassReading();
  
  // Outer compass ring
  for (int i = 0; i < 360; i += 3) {
    float a = i * PI / 180.0 - PI/2;
    int x = centerX + cos(a) * outerRadius;
    int y = centerY + sin(a) * outerRadius;
    
    // Major marks every 30 degrees
    if (i % 30 == 0) {
      gfx->fillCircle(x, y, 3, theme->primary);
    } else {
      gfx->drawPixel(x, y, RGB565(80, 80, 90));
    }
  }
  
  // Cardinal directions
  const char* cardinals[] = {"N", "E", "S", "W"};
  uint16_t cardinalColors[] = {COLOR_RED, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE};
  int cardinalAngles[] = {0, 90, 180, 270};
  
  for (int i = 0; i < 4; i++) {
    float a = cardinalAngles[i] * PI / 180.0 - PI/2;
    int x = centerX + cos(a) * (outerRadius + 20);
    int y = centerY + sin(a) * (outerRadius + 20);
    
    gfx->setTextSize(2);
    gfx->setTextColor(cardinalColors[i]);
    gfx->setCursor(x - 6, y - 8);
    gfx->print(cardinals[i]);
  }
  
  // Compass needle (rotated by heading)
  float needleAngle = compass.heading * PI / 180.0 - PI/2;
  
  // North pointer (red)
  int northX = centerX + cos(needleAngle) * innerRadius;
  int northY = centerY + sin(needleAngle) * innerRadius;
  for (int i = 0; i < 8; i++) {
    float a = needleAngle;
    int x = centerX + cos(a) * (i * 12);
    int y = centerY + sin(a) * (i * 12);
    gfx->fillCircle(x, y, 4, COLOR_RED);
  }
  
  // South pointer (white)
  float southAngle = needleAngle + PI;
  int southX = centerX + cos(southAngle) * (innerRadius / 2);
  int southY = centerY + sin(southAngle) * (innerRadius / 2);
  for (int i = 0; i < 5; i++) {
    float a = southAngle;
    int x = centerX + cos(a) * (i * 10);
    int y = centerY + sin(a) * (i * 10);
    gfx->fillCircle(x, y, 3, COLOR_WHITE);
  }
  
  // Center dot
  gfx->fillCircle(centerX, centerY, 8, RGB565(40, 40, 50));
  gfx->fillCircle(centerX, centerY, 5, theme->accent);
  
  // Heading display
  gfx->setTextSize(4);
  gfx->setTextColor(theme->primary);
  char headingStr[8];
  sprintf(headingStr, "%3d°", (int)compass.heading);
  gfx->setCursor(centerX - 54, 100);
  gfx->print(headingStr);
  
  // Cardinal direction text
  const char* direction = getCardinalDirection(compass.heading);
  gfx->setTextSize(2);
  gfx->setTextColor(theme->accent);
  int dirLen = strlen(direction) * 12;
  gfx->setCursor(centerX - dirLen/2, 145);
  gfx->print(direction);
  
  // Calibration status
  if (!compass.calibrated) {
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(255, 200, 0));
    gfx->setCursor(90, 400);
    gfx->print("Rotate watch to calibrate");
  }
  
  drawSwipeIndicator();
}

// =============================================================================
// COMPASS LOGIC
// =============================================================================

void updateCompassReading() {
  // Read magnetometer data from QMI8658
  // Note: This is a simplified version. Real implementation needs proper I2C reads
  
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(QMI8658_MAG_X_L);
  Wire.endTransmission(false);
  Wire.requestFrom(QMI8658_ADDR, 6);
  
  if (Wire.available() >= 6) {
    int16_t mx = Wire.read() | (Wire.read() << 8);
    int16_t my = Wire.read() | (Wire.read() << 8);
    int16_t mz = Wire.read() | (Wire.read() << 8);
    
    compass.mag_x = mx;
    compass.mag_y = my;
    compass.mag_z = mz;
    
    compass.heading = calculateHeading(mx, my, mz);
    compass.last_update = millis();
    
    // Simple calibration check
    if (abs(mx) > 50 || abs(my) > 50) {
      compass.calibrated = true;
    }
  }
}

float calculateHeading(float mx, float my, float mz) {
  // Calculate heading from magnetometer data
  // Simplified 2D heading (assumes watch is held flat)
  
  float heading = atan2(my, mx) * 180.0 / PI;
  
  // Normalize to 0-359
  if (heading < 0) {
    heading += 360;
  }
  
  // Adjust for magnetic declination if needed
  // (Varies by location, typically -20 to +20 degrees)
  
  return heading;
}

const char* getCardinalDirection(float heading) {
  // 16-point compass rose
  const char* directions[] = {
    "N", "NNE", "NE", "ENE",
    "E", "ESE", "SE", "SSE",
    "S", "SSW", "SW", "WSW",
    "W", "WNW", "NW", "NNW"
  };
  
  int index = (int)((heading + 11.25) / 22.5) % 16;
  return directions[index];
}

// =============================================================================
// TOUCH HANDLING
// =============================================================================

void handleCompassTouch(TouchGesture& gesture) {
  // Swipe UP to exit
  if (gesture.event == TOUCH_SWIPE_UP) {
    returnToAppGrid();
    return;
  }
  
  // Tap to recalibrate
  if (gesture.event == TOUCH_TAP) {
    compass.calibrated = false;
    drawCompassApp();
  }
}
