/**
 * Power Manager Implementation
 * Premium screen control with Apple Watch-style behavior
 */

#include "power_manager.h"
#include "Arduino_GFX_Library.h"

extern Arduino_SH8601 *gfx;

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER BUTTON HANDLING
// ═══════════════════════════════════════════════════════════════════════════════

void handlePowerButton() {
  // Waveshare ESP32-S3-Touch-AMOLED-1.8 doesn't have a physical power button
  // Screen control is via touch-to-wake and screen timeout only
  // This function is a placeholder for boards that do have a power button
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TIMEOUT HANDLING (3 SECONDS DEFAULT)
// ═══════════════════════════════════════════════════════════════════════════════

void handleScreenTimeout() {
  if (!powerState.screenOn) return;
  
  // Check if timeout threshold exceeded
  unsigned long inactiveTime = millis() - powerState.lastActivityMs;
  
  if (inactiveTime >= powerState.screenTimeoutMs) {
    Serial.printf("[PWR] Screen timeout after %lu ms\n", inactiveTime);
    screenSleep();
  }
}

bool shouldScreenTimeout() {
  if (!powerState.screenOn) return false;
  return (millis() - powerState.lastActivityMs) >= powerState.screenTimeoutMs;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN SLEEP (SMOOTH FADE OFF)
// ═══════════════════════════════════════════════════════════════════════════════

void screenSleep() {
  if (!powerState.screenOn) return;
  
  Serial.println("[PWR] Screen -> SLEEP");
  
  // Start fade to black
  powerState.targetBrightness = BRIGHTNESS_MIN;
  powerState.fadeInProgress = true;
  
  // Immediate dim for responsiveness, then fade completes in loop
  gfx->setBrightness(BRIGHTNESS_DIM);
  
  // Mark screen as off
  powerState.screenOn = false;
  watch.screenOn = false;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN WAKE (SMOOTH FADE ON) - TOUCH TO WAKE / BUTTON WAKE
// ═══════════════════════════════════════════════════════════════════════════════

void screenWake() {
  if (powerState.screenOn) return;
  
  Serial.println("[PWR] Screen -> WAKE");
  
  // Enable display hardware
  gfx->displayOn();
  
  // Get target brightness based on battery saver mode
  uint8_t targetBright = saverModes[powerState.batterySaverLevel].brightness;
  
  // Start fade to full brightness
  powerState.targetBrightness = targetBright;
  powerState.fadeInProgress = true;
  
  // Immediate partial brightness for responsiveness
  gfx->setBrightness(targetBright / 2);
  powerState.currentBrightness = targetBright / 2;
  
  // Mark screen as on
  powerState.screenOn = true;
  watch.screenOn = true;
  
  // Reset activity timer
  powerState.lastActivityMs = millis();
  watch.lastActivityMs = millis();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  BRIGHTNESS FADE ANIMATION
// ═══════════════════════════════════════════════════════════════════════════════

void handleBrightnessFade() {
  if (!powerState.fadeInProgress) return;
  
  static unsigned long lastFadeStep = 0;
  unsigned long fadeInterval = BRIGHTNESS_FADE_DURATION / (BRIGHTNESS_MAX / BRIGHTNESS_STEP);
  
  if (millis() - lastFadeStep < fadeInterval) return;
  lastFadeStep = millis();
  
  // Calculate step direction
  if (powerState.currentBrightness < powerState.targetBrightness) {
    // Fade in
    powerState.currentBrightness = min(
      (int)powerState.currentBrightness + BRIGHTNESS_STEP,
      (int)powerState.targetBrightness
    );
  } else if (powerState.currentBrightness > powerState.targetBrightness) {
    // Fade out
    powerState.currentBrightness = max(
      (int)powerState.currentBrightness - BRIGHTNESS_STEP,
      (int)powerState.targetBrightness
    );
  }
  
  // Apply brightness
  gfx->setBrightness(powerState.currentBrightness);
  
  // Check if fade complete
  if (powerState.currentBrightness == powerState.targetBrightness) {
    powerState.fadeInProgress = false;
    
    // If faded to black, turn off display hardware
    if (powerState.currentBrightness == BRIGHTNESS_MIN) {
      gfx->displayOff();
      Serial.println("[PWR] Display hardware OFF");
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CONFIGURATION FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

void setScreenTimeout(unsigned long timeoutMs) {
  powerState.screenTimeoutMs = timeoutMs;
  Serial.printf("[PWR] Screen timeout set to %lu ms\n", timeoutMs);
}

void applyBatterySaverMode(BatterySaverLevel level) {
  if (level >= sizeof(saverModes) / sizeof(saverModes[0])) {
    level = BATTERY_SAVER_OFF;
  }
  
  powerState.batterySaverLevel = level;
  powerState.screenTimeoutMs = saverModes[level].screenTimeoutMs;
  
  // Adjust brightness if screen is on
  if (powerState.screenOn) {
    powerState.targetBrightness = saverModes[level].brightness;
    powerState.fadeInProgress = true;
  }
  
  Serial.printf("[PWR] Battery Saver: %s (brightness=%d, timeout=%dms)\n",
    saverModes[level].name,
    saverModes[level].brightness,
    saverModes[level].screenTimeoutMs
  );
}

void resetActivityTimer() {
  powerState.lastActivityMs = millis();
  watch.lastActivityMs = millis();
}
