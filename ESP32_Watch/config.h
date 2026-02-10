/**
 * Configuration & Global Types - FIXED VERSION
 * Screen timeout disabled, tap to wake, power button support
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
//  THEME TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum ThemeType {
  THEME_LUFFY,      // Orange/Gold - Gear 5 Sun God
  THEME_JINWOO,     // Purple/Dark - Shadow Monarch
  THEME_YUGO,       // Teal/Blue - Portal Master
  THEME_RANDOM      // Daily rotating character theme
};

// Random Theme Characters
enum RandomCharacter {
  CHAR_NARUTO,
  CHAR_GOKU,
  CHAR_TANJIRO,
  CHAR_GOJO,
  CHAR_LEVI,
  CHAR_SAITAMA,
  CHAR_DEKU,
  RANDOM_CHAR_COUNT = 7
};

// ═══════════════════════════════════════════════════════════════════════════════
//  GAME TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum GameType {
  GAME_BATTLE,
  GAME_SNAKE,
  GAME_MEMORY,
  GAME_DUNGEON,
  GAME_PIRATE,
  GAME_PORTAL,
  GAME_GACHA,
  GAME_TRAINING,
  GAME_BOSS_RUSH,
  GAME_CARD_BATTLE
};

// ═══════════════════════════════════════════════════════════════════════════════
//  NAVIGATION (SWIPE SUPPORT)
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
//  THEME COLORS
// ═══════════════════════════════════════════════════════════════════════════════
struct ThemeColors {
  uint32_t primary;
  uint32_t secondary;
  uint32_t accent;
  uint32_t background;
  uint32_t text;
  uint32_t glow;
  uint32_t highlight;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RPG CHARACTER
// ═══════════════════════════════════════════════════════════════════════════════
struct RPGCharacter {
  const char* name;
  int level;
  long xp;
  long xpNext;
  int strength;
  int speed;
  int magic;
  int endurance;
  int specialPower;
  const char* title;
  int shadowArmy;
  int hakiLevel;
  int portalMastery;
  int chakraReserves;
  int powerLevel;
  int breathingForm;
  int cursedEnergy;
  int titanKills;
  int heroRank;
  int ofaPercent;
};

extern RPGCharacter rpgCharacter;

// ═══════════════════════════════════════════════════════════════════════════════
//  USER DATA (Persistent)
// ═══════════════════════════════════════════════════════════════════════════════
struct UserData {
  uint32_t totalSteps;
  uint32_t stepGoal;
  int stepStreak;
  int gamesWon;
  int gamesPlayed;
  int rpgLevel;
  long rpgXP;
  ThemeType theme;
  uint8_t brightness;
  int questsCompleted;
  int gachaCards;
  int gachaPulls;
  int bossesDefeated;
};

extern UserData userData;

// ═══════════════════════════════════════════════════════════════════════════════
//  STOPWATCH
// ═══════════════════════════════════════════════════════════════════════════════
struct StopwatchData {
  unsigned long elapsedMs;
  unsigned long startTime;
  unsigned long laps[10];
  int lapCount;
  bool running;
  bool paused;
  unsigned long pausedTime;
};

extern StopwatchData stopwatch;

// ═══════════════════════════════════════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void saveUserData();
void loadUserData();
void showScreen(ScreenType screen);
void updateClock();
void initThemes();

RandomCharacter getDailyCharacter();
const char* getRandomCharacterName(RandomCharacter character);
const char* getRandomCharacterSeries(RandomCharacter character);
ThemeColors getRandomCharacterColors(RandomCharacter character);
ThemeColors getThemeColors(ThemeType theme);

#endif
