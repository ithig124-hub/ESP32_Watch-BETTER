/**
 * RPG System - Solo Leveling Inspired Progression
 */

#ifndef RPG_H
#define RPG_H

#include "config.h"
#include <lvgl.h>

// Initialize RPG system
void initRPG();

// Create RPG screen
lv_obj_t* createRPGScreen();

// XP and leveling
void gainXP(int amount);
void levelUp();
bool canLevelUp();
long getXPForLevel(int level);

// Character stats
void updateStats();
const char* getCurrentTitle();

// RPG Character data
extern RPGCharacter rpgCharacter;

#endif
