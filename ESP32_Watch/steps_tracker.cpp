/*
 * steps_tracker.cpp - Step Counter Implementation
 */

#include "steps_tracker.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"
#include "hardware.h"
#include <Wire.h>
#include <Preferences.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

StepsData steps_data = {0, 10000, 0.0, 0, 0, 0, {}, {}, 0};
ActivityThresholds activity_config = {1.2, 250, 0.75};

static float last_magnitude = 1.0;
static unsigned long last_step_time = 0;

Preferences prefs;

void initStepsTracker() {
  loadStepsData();
  WatchTime current_time = getCurrentTime();
  if (current_time.hour == 0 && steps_data.current_hour != 0) {
    for (int i = 6; i > 0; i--) {
      steps_data.weekly_steps[i] = steps_data.weekly_steps[i-1];
    }
    steps_data.weekly_steps[0] = steps_data.steps_today;
    resetStepsToday();
  }
  steps_data.current_hour = current_time.hour;
  Serial.printf("[Steps] Today: %d steps\n", steps_data.steps_today);
}

void drawStepsCard() {
  gfx->fillScreen(RGB565(8, 8, 12));
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 55, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 55, LCD_WIDTH, theme->primary);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH/2 - 54, 18);
  gfx->print("Activity");
  
  int centerX = LCD_WIDTH / 2;
  int centerY = 160;
  int radius = 90;
  float progress = (float)steps_data.steps_today / steps_data.steps_goal;
  if (progress > 1.0) progress = 1.0;
  
  // Background ring
  for (int i = 0; i < 360; i += 3) {
    float a = i * PI / 180.0 - PI/2;
    int x = centerX + cos(a) * radius;
    int y = centerY + sin(a) * radius;
    gfx->fillCircle(x, y, 6, RGB565(30, 30, 40));
  }
  
  // Progress arc
  int progressDeg = progress * 360;
  for (int i = 0; i < progressDeg; i += 3) {
    float a = i * PI / 180.0 - PI/2;
    int x = centerX + cos(a) * radius;
    int y = centerY + sin(a) * radius;
    uint16_t color = (progress < 0.5) ? RGB565(255, 100, 100) :
                     (progress < 0.8) ? RGB565(255, 200, 50) : RGB565(100, 255, 100);
    gfx->fillCircle(x, y, 7, color);
  }
  
  // Center step count
  gfx->setTextSize(4);
  gfx->setTextColor(theme->primary);
  char stepsStr[10];
  sprintf(stepsStr, "%d", steps_data.steps_today);
  int strLen = strlen(stepsStr) * 24;
  gfx->setCursor(centerX - strLen/2, centerY - 20);
  gfx->print(stepsStr);
  
  gfx->setTextSize(1);
  gfx->setTextColor(theme->accent);
  gfx->setCursor(centerX - 15, centerY + 15);
  gfx->print("STEPS");
  
  // Goal
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(centerX - 36, centerY + 35);
  gfx->printf("Goal: %d", steps_data.steps_goal);
  
  // Stats cards
  int cardY = 280;
  int cardW = 110;
  int cardH = 70;
  
  // Distance
  gfx->fillRoundRect(10, cardY, cardW, cardH, 12, RGB565(25, 27, 35));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(20, cardY + 10);
  gfx->print("Distance");
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_CYAN);
  gfx->setCursor(20, cardY + 30);
  gfx->printf("%.2f", steps_data.distance_km);
  gfx->setTextSize(1);
  gfx->setCursor(20, cardY + 52);
  gfx->print("km");
  
  // Calories
  gfx->fillRoundRect(130, cardY, cardW, cardH, 12, RGB565(25, 27, 35));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(140, cardY + 10);
  gfx->print("Calories");
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_ORANGE);
  gfx->setCursor(140, cardY + 30);
  gfx->printf("%d", steps_data.calories_burned);
  gfx->setTextSize(1);
  gfx->setCursor(140, cardY + 52);
  gfx->print("kcal");
  
  // Active minutes
  gfx->fillRoundRect(250, cardY, cardW, cardH, 12, RGB565(25, 27, 35));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(260, cardY + 10);
  gfx->print("Active");
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_GREEN);
  gfx->setCursor(260, cardY + 30);
  gfx->printf("%d", steps_data.active_minutes);
  gfx->setTextSize(1);
  gfx->setCursor(260, cardY + 52);
  gfx->print("min");
  
  // Weekly chart
  int chartY = 370;
  int barW = 45;
  int barMaxH = 60;
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(20, chartY);
  gfx->print("7-Day History");
  
  for (int i = 0; i < 7; i++) {
    int x = 20 + i * (barW + 5);
    int y = chartY + 20;
    uint32_t daySteps = steps_data.weekly_steps[6 - i];
    int barH = (daySteps * barMaxH) / steps_data.steps_goal;
    if (barH > barMaxH) barH = barMaxH;
    
    uint16_t barColor = (i == 6) ? theme->primary : RGB565(80, 80, 100);
    gfx->fillRoundRect(x, y + (barMaxH - barH), barW, barH, 4, barColor);
    
    const char* days[] = {"M", "T", "W", "T", "F", "S", "S"};
    gfx->setCursor(x + 18, y + barMaxH + 5);
    gfx->print(days[i]);
  }
  
  drawSwipeIndicator();
}

void updateStepCount() {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(0x35);
  Wire.endTransmission(false);
  Wire.requestFrom(QMI8658_ADDR, 6);
  
  if (Wire.available() >= 6) {
    int16_t ax = Wire.read() | (Wire.read() << 8);
    int16_t ay = Wire.read() | (Wire.read() << 8);
    int16_t az = Wire.read() | (Wire.read() << 8);
    
    float ax_g = ax / 16384.0;
    float ay_g = ay / 16384.0;
    float az_g = az / 16384.0;
    
    processAccelerometerData(ax_g, ay_g, az_g);
  }
  
  steps_data.distance_km = steps_data.steps_today * activity_config.stride_length_m / 1000.0;
  steps_data.calories_burned = steps_data.steps_today * 0.04;
  
  if (steps_data.steps_today % 100 == 0 && steps_data.steps_today > 0) {
    saveStepsData();
  }
}

void processAccelerometerData(float ax, float ay, float az) {
  float magnitude = sqrt(ax*ax + ay*ay + az*az);
  
  if (detectStep(magnitude)) {
    steps_data.steps_today++;
    steps_data.hourly_steps[steps_data.current_hour]++;
    
    if (millis() - last_step_time < 60000) {
      steps_data.active_minutes++;
    }
    last_step_time = millis();
  }
  last_magnitude = magnitude;
}

bool detectStep(float magnitude) {
  unsigned long now = millis();
  if (now - last_step_time < activity_config.step_cooldown_ms) return false;
  
  float delta = magnitude - last_magnitude;
  return (delta > activity_config.step_threshold);
}

void resetStepsToday() {
  steps_data.steps_today = 0;
  steps_data.distance_km = 0.0;
  steps_data.calories_burned = 0;
  steps_data.active_minutes = 0;
  memset(steps_data.hourly_steps, 0, sizeof(steps_data.hourly_steps));
  saveStepsData();
}

uint32_t getTodaySteps() { return steps_data.steps_today; }
float getDistanceKm() { return steps_data.distance_km; }
uint16_t getCaloriesBurned() { return steps_data.calories_burned; }

void saveStepsData() {
  prefs.begin("steps", false);
  prefs.putUInt("steps_today", steps_data.steps_today);
  prefs.putUInt("steps_goal", steps_data.steps_goal);
  prefs.putFloat("distance", steps_data.distance_km);
  for (int i = 0; i < 7; i++) {
    char key[12];
    sprintf(key, "week_%d", i);
    prefs.putUInt(key, steps_data.weekly_steps[i]);
  }
  prefs.end();
}

void loadStepsData() {
  prefs.begin("steps", true);
  steps_data.steps_today = prefs.getUInt("steps_today", 0);
  steps_data.steps_goal = prefs.getUInt("steps_goal", 10000);
  steps_data.distance_km = prefs.getFloat("distance", 0.0);
  for (int i = 0; i < 7; i++) {
    char key[12];
    sprintf(key, "week_%d", i);
    steps_data.weekly_steps[i] = prefs.getUInt(key, 0);
  }
  prefs.end();
}

void handleStepsCardTouch(TouchGesture& gesture) {
  // Swipes are now handled by main navigation loop
  // Steps is part of: Watchface → Steps → App Grid → Stats → loop
  
  // Only handle tap to change goal
  if (gesture.event == TOUCH_TAP && gesture.y >= 180 && gesture.y <= 210) {
    uint32_t goals[] = {5000, 10000, 15000, 20000};
    for (int i = 0; i < 4; i++) {
      if (steps_data.steps_goal == goals[i]) {
        steps_data.steps_goal = goals[(i + 1) % 4];
        break;
      }
    }
    saveStepsData();
    drawStepsCard();
  }
}
