/**
 * User Data Persistence
 */

#include "config.h"
#include <Preferences.h>

extern Preferences prefs;

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
  .questsCompleted = 0
};

void saveUserData() {
  prefs.begin("watch", false);
  prefs.putULong("steps", userData.totalSteps);
  prefs.putULong("goal", userData.stepGoal);
  prefs.putInt("streak", userData.stepStreak);
  prefs.putInt("wins", userData.gamesWon);
  prefs.putInt("played", userData.gamesPlayed);
  prefs.putInt("rpgLvl", userData.rpgLevel);
  prefs.putLong("rpgXP", userData.rpgXP);
  prefs.putInt("theme", (int)userData.theme);
  prefs.putInt("bright", userData.brightness);
  prefs.putInt("quests", userData.questsCompleted);
  prefs.end();
  Serial.println("[OK] User data saved");
}

void loadUserData() {
  prefs.begin("watch", true);
  userData.totalSteps = prefs.getULong("steps", 0);
  userData.stepGoal = prefs.getULong("goal", 10000);
  userData.stepStreak = prefs.getInt("streak", 0);
  userData.gamesWon = prefs.getInt("wins", 0);
  userData.gamesPlayed = prefs.getInt("played", 0);
  userData.rpgLevel = prefs.getInt("rpgLvl", 1);
  userData.rpgXP = prefs.getLong("rpgXP", 0);
  userData.theme = (ThemeType)prefs.getInt("theme", THEME_LUFFY);
  userData.brightness = prefs.getInt("bright", 200);
  userData.questsCompleted = prefs.getInt("quests", 0);
  prefs.end();
  
  // Apply loaded settings
  watch.theme = userData.theme;
  watch.brightness = userData.brightness;
  watch.stepGoal = userData.stepGoal;
  watch.steps = userData.totalSteps;
  
  Serial.println("[OK] User data loaded");
}
