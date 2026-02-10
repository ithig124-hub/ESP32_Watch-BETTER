/**
 * Power Manager - FIXED VERSION
 * NO SCREEN TIMEOUT + Power Button Toggle + Tap to Wake
 *
 * CHANGES:
 * ✅ Screen timeout DISABLED (screen stays on until manual control)
 * ✅ Power button tap toggles screen on/off
 * ✅ Touch-to-wake when screen is off
 * ✅ Device keeps running when screen is off
 * ✅ Smooth brightness fade animations
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "pin_config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TIMEOUT CONFIGURATION - DISABLED BY DEFAULT
// ═══════════════════════════════════════════════════════════════════════════════

// DISABLED - No automatic screen timeout!
// Screen only turns off via power button or explicit user action
#define SCREEN_TIMEOUT_DISABLED   0xFFFFFFFF  // Max value = never timeout
#define SCREEN_TIMEOUT_ENABLED    false       // Set to true to enable timeout

// If you want to re-enable timeout, change these:
#define SCREEN_TIMEOUT_DEFAULT    3000   // 3 seconds (only used if enabled)
#define SCREEN_TIMEOUT_EXTENDED   5000   // 5 seconds
#define SCREEN_TIMEOUT_LONG       10000  // 10 seconds
#define SCREEN_TIMEOUT_ALWAYS_ON  30000  // 30 seconds

// Brightness fade settings for smooth transitions
#define BRIGHTNESS_FADE_DURATION  200    // ms for fade animation
#define BRIGHTNESS_STEP           15     // Brightness change per step
#define BRIGHTNESS_MIN            0      // Screen off
#define BRIGHTNESS_MAX            255    // Full brightness
#define BRIGHTNESS_DIM            50     // Dim before sleep

// Power button debounce
#define PWR_BUTTON_DEBOUNCE_MS    200    // Debounce time

// ═══════════════════════════════════════════════════════════════════════════════
//  BATTERY SAVER LEVELS
// ═══════════════════════════════════════════════════════════════════════════════
enum BatterySaverLevel {
  BATTERY_SAVER_OFF,      // Full brightness, no timeout
  BATTERY_SAVER_LIGHT,    // 50% brightness, no timeout
  BATTERY_SAVER_MEDIUM,   // 30% brightness, no timeout
  BATTERY_SAVER_EXTREME   // 10% brightness, no timeout
};

struct BatterySaverMode {
  uint8_t brightness;       // Max brightness (0-255)
  uint32_t screenTimeoutMs; // DISABLED - kept for compatibility
  bool wifiEnabled;         // WiFi allowed
  const char* name;         // Display name
};

// NOTE: screenTimeoutMs is IGNORED when SCREEN_TIMEOUT_ENABLED = false
static const BatterySaverMode saverModes[] = {
  {200, 0xFFFFFFFF, true,  "Off"},      // Full brightness, NO timeout
  {127, 0xFFFFFFFF, true,  "Light"},    // 50% brightness, NO timeout
  {76,  0xFFFFFFFF, true,  "Medium"},   // 30% brightness, NO timeout
  {25,  0xFFFFFFFF, false, "Extreme"}   // 10% brightness, NO timeout
};

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER STATE
// ═══════════════════════════════════════════════════════════════════════════════
struct PowerState {
  bool screenOn;                    // Current screen state
  unsigned long lastActivityMs;     // Last user interaction timestamp
  uint32_t screenTimeoutMs;         // Current timeout setting (DISABLED)
  BatterySaverLevel batterySaverLevel;
  uint8_t currentBrightness;        // Current brightness level
  uint8_t targetBrightness;         // Target brightness for fade
  bool fadeInProgress;              // Fade animation active
  uint8_t lastPwrButtonState;       // For debouncing
  unsigned long pwrButtonDebounceMs;
  bool screenTimeoutEnabled;        // NEW: Toggle for screen timeout
};

extern PowerState powerState;

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER MANAGEMENT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Initialize power button GPIO
 */
void initPowerButton();

/**
 * Handle power button press
 * - Tap: Toggle screen on/off
 */
void handlePowerButton();

/**
 * Handle screen timeout - DISABLED BY DEFAULT
 * - Does nothing when SCREEN_TIMEOUT_ENABLED = false
 */
void handleScreenTimeout();

/**
 * Turn screen off with smooth fade
 * Device keeps running in background!
 */
void screenSleep();

/**
 * Turn screen on with smooth fade
 * Called by touch-to-wake or power button
 */
void screenWake();

/**
 * Toggle screen on/off
 * Main function for power button
 */
void screenToggle();

/**
 * Handle brightness fade animation
 * Call in main loop for smooth transitions
 */
void handleBrightnessFade();

/**
 * Set screen timeout duration (if enabled)
 */
void setScreenTimeout(unsigned long timeoutMs);

/**
 * Enable or disable screen timeout
 */
void setScreenTimeoutEnabled(bool enabled);

/**
 * Apply battery saver mode
 */
void applyBatterySaverMode(BatterySaverLevel level);

/**
 * Reset activity timer
 */
void resetActivityTimer();

/**
 * Check if screen should timeout
 */
bool shouldScreenTimeout();

#endif
