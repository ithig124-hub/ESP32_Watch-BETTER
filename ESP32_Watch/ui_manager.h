/**
 * Premium UI Manager Header
 * Apple Watch-Inspired Interface
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "pin_config.h"

extern lv_obj_t* screens[16];

// Screen management
void showScreen(ScreenType screen);
void goBack();
void goHome();

// Screen creators
lv_obj_t* createClockScreen();
lv_obj_t* createAppsScreen();
lv_obj_t* createApps2Screen();
lv_obj_t* createStepsScreen();
lv_obj_t* createGamesScreen();
lv_obj_t* createMusicScreen();
lv_obj_t* createWeatherScreen();
lv_obj_t* createNewsScreen();
lv_obj_t* createQuestsScreen();
lv_obj_t* createRPGScreen();
lv_obj_t* createSettingsScreen();
lv_obj_t* createCharStatsScreen();
lv_obj_t* createGachaScreen();
lv_obj_t* createTrainingScreen();
lv_obj_t* createBossRushScreen();

// Common UI elements
lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title);
lv_obj_t* createAppButton(lv_obj_t* parent, const char* label, uint32_t color);
lv_obj_t* createProgressArc(lv_obj_t* parent, int value, int max);

// Theme effects
void drawThemeEffects(lv_obj_t* parent, ThemeType theme);
void drawRandomCharacterEffects(lv_obj_t* parent, RandomCharacter character);

// Updates
void updateClock();
void updateSteps();

// Theme color helper
lv_color_t getThemeColor(int index);

#endif
