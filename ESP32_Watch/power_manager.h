/**
 * Power Manager - Premium Screen Control
 * Apple Watch-style screen timeout and power button handling
 * 
 * Features:
 * - 3-second screen timeout (configurable)
 * - Power button tap to toggle screen on/off
 * - Touch-to-wake functionality
 * - Smooth brightness fade animations
 * - Battery saver modes
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TIMEOUT CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════

// Default 3-second timeout (Apple Watch style quick timeout)
#define SCREEN_TIMEOUT_DEFAULT    3000   // 3 seconds
#define SCREEN_TIMEOUT_EXTENDED   5000   // 5 seconds
#define SCREEN_TIMEOUT_LONG       10000  // 10 seconds
#define SCREEN_TIMEOUT_ALWAYS_ON  30000  // 30 seconds (battery drain warning)

// Brightness fade settings for smooth transitions
#define BRIGHTNESS_FADE_DURATION  200    // ms for fade animation
#define BRIGHTNESS_STEP           10     // Brightness change per step
#define BRIGHTNESS_MIN            0      // Screen off
#define BRIGHTNESS_MAX            255    // Full brightness
#define BRIGHTNESS_DIM            50     // Dim before sleep

// Power button debounce
#define PWR_BUTTON_DEBOUNCE_MS    200    // Debounce time

// ═══════════════════════════════════════════════════════════════════════════════
//  BATTERY SAVER LEVELS
// ═══════════════════════════════════════════════════════════════════════════════
enum BatterySaverLevel {
  BATTERY_SAVER_OFF,      // Full brightness, 3s timeout
  BATTERY_SAVER_LIGHT,    // 50% brightness, 2s timeout
  BATTERY_SAVER_MEDIUM,   // 30% brightness, 1.5s timeout
  BATTERY_SAVER_EXTREME   // 10% brightness, 1s timeout, WiFi off
};

struct BatterySaverMode {
  uint8_t brightness;       // Max brightness (0-255)
  uint16_t screenTimeoutMs; // Screen timeout in ms
  bool wifiEnabled;         // WiFi allowed
  const char* name;         // Display name
};

static const BatterySaverMode saverModes[] = {
  {200, 3000, true,  "Off"},      // Normal mode
  {127, 2000, true,  "Light"},    // 50% brightness
  {76,  1500, true,  "Medium"},   // 30% brightness  
  {25,  1000, false, "Extreme"}   // 10% brightness, no WiFi
};

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER STATE
// ═══════════════════════════════════════════════════════════════════════════════
struct PowerState {
  bool screenOn;                    // Current screen state
  unsigned long lastActivityMs;     // Last user interaction timestamp
  unsigned long screenTimeoutMs;    // Current timeout setting
  BatterySaverLevel batterySaverLevel;
  uint8_t currentBrightness;        // Current brightness level
  uint8_t targetBrightness;         // Target brightness for fade
  bool fadeInProgress;              // Fade animation active
  uint8_t lastPwrButtonState;       // For debouncing
  unsigned long pwrButtonDebounceMs;
};

extern PowerState powerState;

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER MANAGEMENT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Handle power button press
 * - Tap: Toggle screen on/off
 * - Long press (optional): Enter deep sleep or settings
 */
void handlePowerButton();

/**
 * Handle screen timeout
 * - Checks inactivity time against configured timeout
 * - Initiates smooth screen fade-off when timeout reached
 */
void handleScreenTimeout();

/**
 * Turn screen off with smooth fade
 */
void screenSleep();

/**
 * Turn screen on with smooth fade
 * Called by touch-to-wake or power button
 */
void screenWake();

/**
 * Handle brightness fade animation
 * Call in main loop for smooth transitions
 */
void handleBrightnessFade();

/**
 * Set screen timeout duration
 * @param timeoutMs Timeout in milliseconds
 */
void setScreenTimeout(unsigned long timeoutMs);

/**
 * Apply battery saver mode
 * @param level Battery saver level
 */
void applyBatterySaverMode(BatterySaverLevel level);

/**
 * Reset activity timer
 * Call on any user interaction
 */
void resetActivityTimer();

/**
 * Check if screen should timeout
 * @return true if timeout threshold exceeded
 */
bool shouldScreenTimeout();

#endif
