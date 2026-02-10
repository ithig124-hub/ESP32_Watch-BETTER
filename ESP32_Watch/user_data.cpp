/**
 * User Data - Persistent Storage
 * Save/Load user preferences and RPG progress
 */

#include "config.h"
#include <Preferences.h>

extern Preferences prefs;

// Initialize user data
UserData userData = {
  .totalSteps = 0,
  .stepGoal = 10000,
  .stepStreak = 0,
  .gamesWon = 0,
  .gamesPlayed = 0,
  .rpgLevel = 1,
  .rpgXP = 0,
  .theme = THEME_LUFFY,
  .brightness = 200,
  .questsCompleted = 0,
  .gachaCards = 0,
  .gachaPulls = 0,
  .bossesDefeated = 0
};

// Initialize RPG character
RPGCharacter rpgCharacter = {
  .name = "Player",
  .level = 1,
  .xp = 0,
  .xpNext = 100,
  .strength = 10,
  .speed = 10,
  .magic = 10,
  .endurance = 10,
  .specialPower = 0,
  .title = "Novice",
  .shadowArmy = 0,
  .hakiLevel = 0,
  .portalMastery = 0,
  .chakraReserves = 100,
  .powerLevel = 9001,
  .breathingForm = 1,
  .cursedEnergy = 100,
  .titanKills = 0,
  .heroRank = 0,
  .ofaPercent = 5
};

// Stopwatch data
StopwatchData stopwatch = {
  .elapsedMs = 0,
  .startTime = 0,
  .laps = {0},
  .lapCount = 0,
  .running = false,
  .paused = false,
  .pausedTime = 0
};

void saveUserData() {
  prefs.begin("watchdata", false);
  
  prefs.putUInt("totalSteps", userData.totalSteps);
  prefs.putUInt("stepGoal", userData.stepGoal);
  prefs.putInt("stepStreak", userData.stepStreak);
  prefs.putInt("gamesWon", userData.gamesWon);
  prefs.putInt("gamesPlayed", userData.gamesPlayed);
  prefs.putInt("rpgLevel", userData.rpgLevel);
  prefs.putLong("rpgXP", userData.rpgXP);
  prefs.putInt("theme", (int)userData.theme);
  prefs.putUChar("brightness", userData.brightness);
  prefs.putInt("questsComp", userData.questsCompleted);
  prefs.putInt("gachaCards", userData.gachaCards);
  prefs.putInt("gachaPulls", userData.gachaPulls);
  prefs.putInt("bosses", userData.bossesDefeated);
  
  // Save RPG character
  prefs.putInt("charLevel", rpgCharacter.level);
  prefs.putLong("charXP", rpgCharacter.xp);
  prefs.putInt("charStr", rpgCharacter.strength);
  prefs.putInt("charSpd", rpgCharacter.speed);
  prefs.putInt("charMag", rpgCharacter.magic);
  prefs.putInt("charEnd", rpgCharacter.endurance);
  
  prefs.end();
  
  Serial.println("[DATA] User data saved");
}

void loadUserData() {
  prefs.begin("watchdata", true);
  
  userData.totalSteps = prefs.getUInt("totalSteps", 0);
  userData.stepGoal = prefs.getUInt("stepGoal", 10000);
  userData.stepStreak = prefs.getInt("stepStreak", 0);
  userData.gamesWon = prefs.getInt("gamesWon", 0);
  userData.gamesPlayed = prefs.getInt("gamesPlayed", 0);
  userData.rpgLevel = prefs.getInt("rpgLevel", 1);
  userData.rpgXP = prefs.getLong("rpgXP", 0);
  userData.theme = (ThemeType)prefs.getInt("theme", THEME_LUFFY);
  userData.brightness = prefs.getUChar("brightness", 200);
  userData.questsCompleted = prefs.getInt("questsComp", 0);
  userData.gachaCards = prefs.getInt("gachaCards", 0);
  userData.gachaPulls = prefs.getInt("gachaPulls", 0);
  userData.bossesDefeated = prefs.getInt("bosses", 0);
  
  // Load RPG character
  rpgCharacter.level = prefs.getInt("charLevel", 1);
  rpgCharacter.xp = prefs.getLong("charXP", 0);
  rpgCharacter.strength = prefs.getInt("charStr", 10);
  rpgCharacter.speed = prefs.getInt("charSpd", 10);
  rpgCharacter.magic = prefs.getInt("charMag", 10);
  rpgCharacter.endurance = prefs.getInt("charEnd", 10);
  
  // Calculate XP needed for next level
  rpgCharacter.xpNext = rpgCharacter.level * 100;
  
  prefs.end();
  
  Serial.println("[DATA] User data loaded");
  Serial.printf("[DATA] Level: %d, XP: %ld, Steps: %u\n", 
    rpgCharacter.level, rpgCharacter.xp, userData.totalSteps);
}
