/**
 * Configuration & Global Types
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
  SCREEN_STEPS,
  SCREEN_GAMES,
  SCREEN_MUSIC,
  SCREEN_WEATHER,
  SCREEN_NEWS,
  SCREEN_QUESTS,
  SCREEN_RPG,
  SCREEN_SETTINGS,
  SCREEN_WALLPAPER
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
//  GAME TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum GameType {
  GAME_BATTLE,      // Pokemon-style battles
  GAME_SNAKE,       // Classic snake
  GAME_MEMORY,      // Memory match
  GAME_DUNGEON,     // Shadow dungeon RPG
  GAME_PIRATE,      // Pirate adventure
  GAME_PORTAL       // Portal puzzle
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
  int shadowArmy;      // JinWoo only
  int hakiLevel;       // Luffy only
  int portalMastery;   // Yugo only
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
};

extern UserData userData;

// ═══════════════════════════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════════════════════════
void saveUserData();
void loadUserData();
void showScreen(ScreenType screen);

#endif
