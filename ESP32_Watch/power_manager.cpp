/**
 * Power Manager Implementation - FIXED VERSION
 * 
 * FIXES IMPLEMENTED:
 * ✅ NO automatic screen timeout (disabled by default)
 * ✅ Power button toggles screen on/off
 * ✅ Tap to wake works when screen is off
 * ✅ Device keeps running when screen is off (no deep sleep)
 * ✅ Smooth brightness transitions
 */

#include "power_manager.h"
#include "Arduino_GFX_Library.h"

extern Arduino_SH8601 *gfx;

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER BUTTON INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

void initPowerButton() {
#if PWR_BUTTON_PIN >= 0
  pinMode(PWR_BUTTON_PIN, INPUT_PULLUP);
  powerState.lastPwrButtonState = digitalRead(PWR_BUTTON_PIN);
  Serial.printf("[PWR] Power button initialized on GPIO %d\n", PWR_BUTTON_PIN);
#else
  Serial.println("[PWR] No power button configured - using touch-to-wake only");
#endif
  
  // DISABLE screen timeout by default
  powerState.screenTimeoutEnabled = SCREEN_TIMEOUT_ENABLED;
  powerState.screenTimeoutMs = SCREEN_TIMEOUT_DISABLED;
  
  Serial.println("[PWR] Screen timeout: DISABLED (manual control only)");
  Serial.println("[PWR] Use power button or tap to wake");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER BUTTON HANDLING - TOGGLE SCREEN ON/OFF
// ═══════════════════════════════════════════════════════════════════════════════

void handlePowerButton() {
#if PWR_BUTTON_PIN >= 0
  uint8_t currentState = digitalRead(PWR_BUTTON_PIN);
  
  // Detect button press (HIGH -> LOW transition for pullup)
  if (powerState.lastPwrButtonState == HIGH && currentState == LOW) {
    // Debounce check
    if (millis() - powerState.pwrButtonDebounceMs > PWR_BUTTON_DEBOUNCE_MS) {
      powerState.pwrButtonDebounceMs = millis();
      
      // TOGGLE screen state
      screenToggle();
    }
  }
  
  powerState.lastPwrButtonState = currentState;
#endif
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TOGGLE - MAIN FUNCTION FOR POWER BUTTON
// ═══════════════════════════════════════════════════════════════════════════════

void screenToggle() {
  if (powerState.screenOn) {
    Serial.println("[PWR] Button pressed -> Screen OFF");
    screenSleep();
  } else {
    Serial.println("[PWR] Button pressed -> Screen ON");
    screenWake();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TIMEOUT - DISABLED BY DEFAULT
// ═══════════════════════════════════════════════════════════════════════════════

void handleScreenTimeout() {
  // DISABLED - Screen only turns off via power button or explicit action
  if (!powerState.screenTimeoutEnabled) {
    return;  // Do nothing - no automatic timeout
  }
  
  // Only run if timeout is enabled
  if (!powerState.screenOn) return;
  
  unsigned long inactiveTime = millis() - powerState.lastActivityMs;
  
  if (inactiveTime >= powerState.screenTimeoutMs) {
    Serial.printf("[PWR] Screen timeout after %lu ms\n", inactiveTime);
    screenSleep();
  }
}

bool shouldScreenTimeout() {
  // DISABLED by default
  if (!powerState.screenTimeoutEnabled) return false;
  if (!powerState.screenOn) return false;
  return (millis() - powerState.lastActivityMs) >= powerState.screenTimeoutMs;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN SLEEP (SMOOTH FADE OFF) - DEVICE KEEPS RUNNING!
// ═══════════════════════════════════════════════════════════════════════════════

void screenSleep() {
  if (!powerState.screenOn) return;
  
  Serial.println("[PWR] Screen -> SLEEP (device still running)");
  
  // Start fade to black
  powerState.targetBrightness = BRIGHTNESS_MIN;
  powerState.fadeInProgress = true;
  
  // Immediate dim for responsiveness
  gfx->setBrightness(BRIGHTNESS_DIM);
  
  // Mark screen as off
  powerState.screenOn = false;
  watch.screenOn = false;
  
  // IMPORTANT: Do NOT call esp_deep_sleep_start()
  // Device continues running, only display is off
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN WAKE (TAP TO WAKE / BUTTON WAKE)
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
    
    // If faded to black, turn off display hardware (saves power)
    if (powerState.currentBrightness == BRIGHTNESS_MIN) {
      gfx->displayOff();
      Serial.println("[PWR] Display hardware OFF (device still running)");
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

void setScreenTimeoutEnabled(bool enabled) {
  powerState.screenTimeoutEnabled = enabled;
  if (enabled) {
    powerState.screenTimeoutMs = SCREEN_TIMEOUT_DEFAULT;
    Serial.println("[PWR] Screen timeout ENABLED");
  } else {
    powerState.screenTimeoutMs = SCREEN_TIMEOUT_DISABLED;
    Serial.println("[PWR] Screen timeout DISABLED");
  }
}

void applyBatterySaverMode(BatterySaverLevel level) {
  if (level >= sizeof(saverModes) / sizeof(saverModes[0])) {
    level = BATTERY_SAVER_OFF;
  }
  
  powerState.batterySaverLevel = level;
  // NOTE: Screen timeout stays DISABLED regardless of battery saver mode
  
  // Adjust brightness if screen is on
  if (powerState.screenOn) {
    powerState.targetBrightness = saverModes[level].brightness;
    powerState.fadeInProgress = true;
  }
  
  Serial.printf("[PWR] Battery Saver: %s (brightness=%d, timeout=DISABLED)\n",
    saverModes[level].name,
    saverModes[level].brightness
  );
}

void resetActivityTimer() {
  powerState.lastActivityMs = millis();
  watch.lastActivityMs = millis();
}
