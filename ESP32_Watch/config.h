/**
 * Configuration & Global Types
 * Premium Apple Style Edition with Enhanced Anime Themes
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "pin_config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum ScreenType {
  SCREEN_CLOCK,
  SCREEN_APPS,
  SCREEN_APPS2,
  SCREEN_STEPS,
  SCREEN_GAMES,
  SCREEN_MUSIC,
  SCREEN_WEATHER,
  SCREEN_NEWS,
  SCREEN_QUESTS,
  SCREEN_RPG,
  SCREEN_SETTINGS,
  SCREEN_WALLPAPER,
  SCREEN_CHAR_STATS,
  SCREEN_GACHA,
  SCREEN_TRAINING,
  SCREEN_BOSS_RUSH
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME TYPES - PREMIUM ANIME EDITION
// ═══════════════════════════════════════════════════════════════════════════════
enum ThemeType {
  THEME_LUFFY,      // Gear 5 Sun God - Clean white/gold premium
  THEME_JINWOO,     // Shadow Monarch - Deep purple/black luxury
  THEME_YUGO,       // Portal Master - Teal/cyan exploration
  THEME_RANDOM      // Daily rotating character theme
};

// Random Theme Characters (7 characters for daily rotation)
enum RandomCharacter {
  CHAR_NARUTO,      // Sage Mode - Orange/Yellow chakra
  CHAR_GOKU,        // Super Saiyan - Gold/Blue power
  CHAR_TANJIRO,     // Sun Breathing - Red/Orange flames
  CHAR_GOJO,        // Infinity - Blue/White domain
  CHAR_LEVI,        // Humanity's Strongest - Green/Grey military
  CHAR_SAITAMA,     // One Punch - Yellow/White simple
  CHAR_DEKU,        // One For All - Green/Lightning
  RANDOM_CHAR_COUNT = 7
};

// ═══════════════════════════════════════════════════════════════════════════════
//  NAVIGATION
// ═══════════════════════════════════════════════════════════════════════════════
enum NavCategory {
  NAV_CLOCK,
  NAV_APPS,
  NAV_CHAR_STATS,
  NAV_CATEGORY_COUNT = 3
};

enum SwipeDirection {
  SWIPE_NONE,
  SWIPE_LEFT,
  SWIPE_RIGHT,
  SWIPE_UP,
  SWIPE_DOWN
};

#define SWIPE_THRESHOLD_MIN 50
#define SWIPE_MAX_DURATION 500
#define NAVIGATION_COOLDOWN_MS 300

// ═══════════════════════════════════════════════════════════════════════════════
//  WATCH STATE
// ═══════════════════════════════════════════════════════════════════════════════
struct WatchState {
  ScreenType screen;
  ThemeType theme;
  RandomCharacter dailyCharacter;
  uint16_t lastRandomDay;
  uint8_t batteryPercent;
  bool isCharging;
  uint32_t steps;
  uint32_t stepGoal;
  uint8_t brightness;
  bool screenOn;
  unsigned long lastActivityMs;
  bool wifiConnected;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day;
  uint8_t month;
  uint16_t year;
  uint8_t dayOfWeek;
};

extern WatchState watch;
extern bool hasIMU, hasRTC, hasPMU, hasSD;
extern uint8_t clockHour, clockMinute, clockSecond;

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME COLORS - PREMIUM PALETTE
// ═══════════════════════════════════════════════════════════════════════════════
struct ThemeColors {
  uint32_t primary;       // Main accent color
  uint32_t secondary;     // Secondary accent
  uint32_t accent;        // Highlight/glow color
  uint32_t background;    // Main background
  uint32_t surface;       // Card/container background
  uint32_t text;          // Primary text
  uint32_t textSecondary; // Secondary/muted text
  uint32_t glow;          // Special effect glow
  uint32_t gradient1;     // Gradient start
  uint32_t gradient2;     // Gradient end
  uint32_t border;        // Subtle borders
  uint32_t highlight;     // Interactive highlights
};

// ═══════════════════════════════════════════════════════════════════════════════
//  USER DATA (Persistent)
// ═══════════════════════════════════════════════════════════════════════════════
struct UserData {
  uint32_t totalSteps;
  uint32_t stepGoal;
  ThemeType theme;
  uint8_t brightness;
  uint8_t screenTimeoutIndex;  // 0=3s, 1=5s, 2=10s, 3=30s
};

extern UserData userData;

// ═══════════════════════════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════════════════════════
void saveUserData();
void loadUserData();
void showScreen(ScreenType screen);

// Random theme utility
RandomCharacter getDailyCharacter();
const char* getRandomCharacterName(RandomCharacter character);
ThemeColors getRandomCharacterColors(RandomCharacter character);

#endif
