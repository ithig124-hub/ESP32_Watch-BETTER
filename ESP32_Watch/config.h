/**
 * Configuration & Global Types
 * Merged from both repositories with all features
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "pin_config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN TYPES (All screens from both repos)
// ═══════════════════════════════════════════════════════════════════════════════
enum ScreenType {
  SCREEN_CLOCK,
  SCREEN_APPS,
  SCREEN_STEPS,
  SCREEN_GAMES,
  SCREEN_MUSIC,
  SCREEN_WEATHER,
  SCREEN_NEWS,
  SCREEN_QUESTS,
  SCREEN_RPG,
  SCREEN_SETTINGS,
  SCREEN_WALLPAPER,
  // New screens from ESP32_Watch
  SCREEN_CALCULATOR,
  SCREEN_FLASHLIGHT,
  SCREEN_COIN_FLIP,
  SCREEN_STOPWATCH,
  SCREEN_FILE_BROWSER,
  SCREEN_COUNT  // Total count
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum ThemeType {
  THEME_LUFFY,      // Orange/Gold - Sun God Nika
  THEME_JINWOO,     // Purple/Dark - Shadow Monarch
  THEME_YUGO        // Teal/Blue - Portal Master
};

// ═══════════════════════════════════════════════════════════════════════════════
//  GAME TYPES (All games)
// ═══════════════════════════════════════════════════════════════════════════════
enum GameType {
  GAME_BATTLE,      // Pokemon-style battles
  GAME_SNAKE,       // Classic snake
  GAME_MEMORY,      // Memory match
  GAME_DUNGEON,     // Shadow dungeon RPG
  GAME_PIRATE,      // Pirate adventure
  GAME_PORTAL,      // Portal puzzle
  GAME_PONG,        // Classic pong
  GAME_BREAKOUT     // Breakout game
};

// ═══════════════════════════════════════════════════════════════════════════════
//  WATCH STATE
// ═══════════════════════════════════════════════════════════════════════════════
struct WatchState {
  ScreenType screen;
  ThemeType theme;
  uint8_t batteryPercent;
  bool isCharging;
  uint32_t steps;
  uint32_t stepGoal;
  uint8_t brightness;
  bool screenOn;
  unsigned long lastActivityMs;
  bool wifiConnected;
  // RTC time
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
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RPG CHARACTER (Enhanced)
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
  int shadowArmy;      // JinWoo only
  int hakiLevel;       // Luffy only
  int portalMastery;   // Yugo only
  bool awakened;       // Awakened form unlocked
  int questsCompleted;
  int battlesWon;
};

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
  // Stopwatch
  unsigned long stopwatchTime;
  bool stopwatchRunning;
  // High scores
  int snakeHighScore;
  int pongHighScore;
};

extern UserData userData;

// ═══════════════════════════════════════════════════════════════════════════════
//  STOPWATCH DATA
// ═══════════════════════════════════════════════════════════════════════════════
struct StopwatchData {
  unsigned long startTime;
  unsigned long elapsed;
  unsigned long lapTimes[10];
  int lapCount;
  bool running;
  bool paused;
  unsigned long pausedTime;
};

extern StopwatchData stopwatch;

// ═══════════════════════════════════════════════════════════════════════════════
//  FILE INFO (SD Card)
// ═══════════════════════════════════════════════════════════════════════════════
struct FileInfo {
  char name[64];
  uint32_t size;
  bool isDirectory;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════════════════════════
void saveUserData();
void loadUserData();
void showScreen(ScreenType screen);

// Color macros
#define RGB565(r,g,b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#endif
