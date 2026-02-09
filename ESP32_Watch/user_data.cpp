/**
 * User Data - Persistent Storage
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
  .questsCompleted = 0,
  .gachaCards = 0,
  .gachaPulls = 0,
  .bossesDefeated = 0
};

void saveUserData() {
  prefs.begin("watch", false);
  prefs.putULong("steps", userData.totalSteps);
  prefs.putULong("goal", userData.stepGoal);
  prefs.putInt("streak", userData.stepStreak);
  prefs.putInt("gamesW", userData.gamesWon);
  prefs.putInt("gamesP", userData.gamesPlayed);
  prefs.putInt("rpgLvl", userData.rpgLevel);
  prefs.putLong("rpgXP", userData.rpgXP);
  prefs.putInt("theme", (int)userData.theme);
  prefs.putUChar("bright", userData.brightness);
  prefs.putInt("quests", userData.questsCompleted);
  prefs.putInt("gacha", userData.gachaCards);
  prefs.putInt("pulls", userData.gachaPulls);
  prefs.putInt("bosses", userData.bossesDefeated);
  prefs.end();
}

void loadUserData() {
  prefs.begin("watch", true);
  userData.totalSteps = prefs.getULong("steps", 0);
  userData.stepGoal = prefs.getULong("goal", 10000);
  userData.stepStreak = prefs.getInt("streak", 0);
  userData.gamesWon = prefs.getInt("gamesW", 0);
  userData.gamesPlayed = prefs.getInt("gamesP", 0);
  userData.rpgLevel = prefs.getInt("rpgLvl", 1);
  userData.rpgXP = prefs.getLong("rpgXP", 0);
  userData.theme = (ThemeType)prefs.getInt("theme", THEME_LUFFY);
  userData.brightness = prefs.getUChar("bright", 200);
  userData.questsCompleted = prefs.getInt("quests", 0);
  userData.gachaCards = prefs.getInt("gacha", 0);
  userData.gachaPulls = prefs.getInt("pulls", 0);
  userData.bossesDefeated = prefs.getInt("bosses", 0);
  prefs.end();
  
  watch.theme = userData.theme;
  watch.stepGoal = userData.stepGoal;
  watch.brightness = userData.brightness;
}
