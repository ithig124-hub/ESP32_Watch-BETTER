/**
 * RPG System - 50 Level Progression
 * Supports all characters including Random Daily
 */

#ifndef RPG_H
#define RPG_H

#include <lvgl.h>
#include "config.h"

extern RPGCharacter rpgCharacter;

// Initialize RPG system
void initRPG();

// XP and leveling (50 levels max)
long getXPForLevel(int level);
void gainXP(int amount);
bool canLevelUp();
void levelUp();
void updateStats();

// Title progression (based on level tiers)
const char* getCurrentTitle();
const char* getTitleForLevel(int level, ThemeType theme);
const char* getRandomCharacterTitleForLevel(int level, RandomCharacter character);

// Character-specific special abilities
int getSpecialPower();
const char* getSpecialPowerName();

// RPG Screen
lv_obj_t* createRPGScreen();
void updateRPGScreen();

// Character Stats Screen (for Random theme)
lv_obj_t* createCharStatsScreen();

#endif
