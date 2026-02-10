/**
 * User Data Management
 */

#include "config.h"
#include <Preferences.h>

extern Preferences prefs;

UserData userData = {
  .totalSteps = 0,
  .stepGoal = 10000,
  .theme = THEME_LUFFY,
  .brightness = 200,
  .screenTimeoutIndex = 0
};

void saveUserData() {
  prefs.begin("watch", false);
  prefs.putUInt("totalSteps", userData.totalSteps);
  prefs.putUInt("stepGoal", userData.stepGoal);
  prefs.putUChar("theme", (uint8_t)userData.theme);
  prefs.putUChar("brightness", userData.brightness);
  prefs.putUChar("timeout", userData.screenTimeoutIndex);
  prefs.end();
}

void loadUserData() {
  prefs.begin("watch", true);
  userData.totalSteps = prefs.getUInt("totalSteps", 0);
  userData.stepGoal = prefs.getUInt("stepGoal", 10000);
  userData.theme = (ThemeType)prefs.getUChar("theme", THEME_LUFFY);
  userData.brightness = prefs.getUChar("brightness", 200);
  userData.screenTimeoutIndex = prefs.getUChar("timeout", 0);
  prefs.end();
  
  // Apply loaded settings
  watch.theme = userData.theme;
  watch.brightness = userData.brightness;
  watch.stepGoal = userData.stepGoal;
  watch.steps = userData.totalSteps;
}
