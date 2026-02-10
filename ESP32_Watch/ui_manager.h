/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  PREMIUM UI MANAGER - Ultra High Quality Character Screens
 *  Stunning watch faces and screens for each anime character
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "themes.h"

// Screen dimensions
#define SCREEN_W LCD_WIDTH
#define SCREEN_H LCD_HEIGHT

// UI element sizes
#define CLOCK_FONT_SIZE 72
#define TITLE_FONT_SIZE 24
#define BODY_FONT_SIZE 16
#define SMALL_FONT_SIZE 12

// Animation timings
#define FADE_TIME_MS 200
#define SLIDE_TIME_MS 300

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Show specific screen with theme-aware styling
 */
void showScreen(ScreenType screen);

/**
 * Create premium clock screen
 * Character-themed watch face with signature elements
 */
void createClockScreen();

/**
 * Create apps grid screen
 * Themed app icons with character styling
 */
void createAppsScreen();
void createAppsScreen2();

/**
 * Create character stats screen
 * RPG-style character sheet with theme colors
 */
void createCharacterStatsScreen();

/**
 * Update clock display with current time
 */
void updateClock();

/**
 * Create premium status bar
 * Battery, time, character indicator
 */
void createStatusBar(lv_obj_t* parent);

/**
 * Create character signature visual element
 * Unique to each character (clouds for Luffy, shadows for JinWoo, etc.)
 */
void createCharacterSignature(lv_obj_t* parent);

/**
 * Create themed navigation dots
 * Show current position in screen navigation
 */
void createNavDots(lv_obj_t* parent, int current, int total);

/**
 * Apply premium text styling
 */
void applyPremiumTextStyle(lv_obj_t* label, bool isTitle, bool useGlow);

/**
 * Create glassmorphism container
 * Premium frosted glass effect
 */
lv_obj_t* createGlassContainer(lv_obj_t* parent, int x, int y, int w, int h);

/**
 * Create character power meter
 * Animated stat bar with theme colors
 */
lv_obj_t* createPowerMeter(lv_obj_t* parent, const char* label, int value, int max);

#endif
