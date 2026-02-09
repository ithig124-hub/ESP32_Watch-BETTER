/**
 * Configuration & Global Types
 * Enhanced with Random Daily Theme System
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
//  THEME TYPES - NOW WITH RANDOM DAILY!
// ═══════════════════════════════════════════════════════════════════════════════
enum ThemeType {
  THEME_LUFFY,      // Orange/Gold - Gear 5 Sun God (Clean Fun)
  THEME_JINWOO,     // Purple/Dark - Shadow Monarch (Dark Power)
  THEME_YUGO,       // Teal/Blue - Portal Master (Chill Exploration)
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
//  GAME TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum GameType {
  GAME_BATTLE,      // Pokemon-style battles
  GAME_SNAKE,       // Classic snake
  GAME_MEMORY,      // Memory match
  GAME_DUNGEON,     // Shadow dungeon RPG
  GAME_PIRATE,      // Pirate adventure
  GAME_PORTAL,      // Portal puzzle
  GAME_GACHA,       // Gacha simulator
  GAME_TRAINING,    // Training mini-games
  GAME_BOSS_RUSH,   // Boss rush mode
  GAME_CARD_BATTLE  // Card battles
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
  RandomCharacter dailyCharacter;  // Current random character
  uint16_t lastRandomDay;          // Day of year for rotation
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
  uint32_t glow;       // Special effect color
  uint32_t highlight;  // UI highlight
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RPG CHARACTER (50 LEVELS - Shortened System)
// ═══════════════════════════════════════════════════════════════════════════════
struct RPGCharacter {
  const char* name;
  int level;           // Max 50 levels
  long xp;
  long xpNext;
  int strength;
  int speed;
  int magic;
  int endurance;
  int specialPower;
  const char* title;
  
  // Character-specific powers
  int shadowArmy;      // JinWoo
  int hakiLevel;       // Luffy
  int portalMastery;   // Yugo
  
  // Random character specific
  int chakraReserves;  // Naruto
  int powerLevel;      // Goku
  int breathingForm;   // Tanjiro
  int cursedEnergy;    // Gojo
  int titanKills;      // Levi
  int heroRank;        // Saitama
  int ofaPercent;      // Deku
};

extern RPGCharacter rpgCharacter;

// ═══════════════════════════════════════════════════════════════════════════════
//  QUEST DATA
// ═══════════════════════════════════════════════════════════════════════════════
struct Quest {
  const char* title;
  const char* description;
  int target;
  int progress;
  int reward;
  bool completed;
  bool daily;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  GACHA DATA
// ═══════════════════════════════════════════════════════════════════════════════
enum GachaRarity {
  RARITY_COMMON,    // 60%
  RARITY_RARE,      // 25%
  RARITY_EPIC,      // 12%
  RARITY_LEGENDARY, // 2.9%
  RARITY_MYTHIC     // 0.1%
};

struct GachaCard {
  const char* name;
  const char* series;
  GachaRarity rarity;
  int power;
  bool owned;
};

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
