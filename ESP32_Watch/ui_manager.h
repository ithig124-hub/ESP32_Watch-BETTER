/**
 * UI Manager - Screen Management & Navigation
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"

// Screen storage
extern lv_obj_t* screens[16];

// Display dimensions
#define LCD_WIDTH 368
#define LCD_HEIGHT 448

// Theme colors
lv_color_t getThemeColor(int index);

// Screen navigation
void showScreen(ScreenType screen);
void goBack();
void goHome();

// Screen creation
lv_obj_t* createClockScreen();
lv_obj_t* createAppsScreen();
lv_obj_t* createApps2Screen();
lv_obj_t* createStepsScreen();
lv_obj_t* createGamesScreen();
lv_obj_t* createMusicScreen();
lv_obj_t* createWeatherScreen();
lv_obj_t* createNewsScreen();
lv_obj_t* createQuestsScreen();
lv_obj_t* createSettingsScreen();
lv_obj_t* createGachaScreen();
lv_obj_t* createTrainingScreen();
lv_obj_t* createBossRushScreen();

// Screen updates
void updateClock();
void updateSteps();

// Common UI elements
lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title);
lv_obj_t* createAppButton(lv_obj_t* parent, const char* label, uint32_t color);
lv_obj_t* createProgressArc(lv_obj_t* parent, int value, int max);

#endif
