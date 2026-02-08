/**
 * UI Manager - LVGL Screen Management
 * Updated with all screens from both repos
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"

// Screen objects (expanded)
extern lv_obj_t* screens[SCREEN_COUNT];

// Create all screens
void createAllScreens();

// Screen navigation
void showScreen(ScreenType screen);
void goBack();
void goHome();

// Clock screen
lv_obj_t* createClockScreen();
void updateClock();

// App grid screen
lv_obj_t* createAppsScreen();

// Steps screen
lv_obj_t* createStepsScreen();
void updateSteps();

// File browser screen
lv_obj_t* createFileBrowserScreen();

// Common UI elements
lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title);
lv_obj_t* createAppButton(lv_obj_t* parent, const char* label, uint32_t color);
lv_obj_t* createProgressArc(lv_obj_t* parent, int value, int max);
void createNavButtons(lv_obj_t* parent);

// Theme application
void applyTheme(ThemeType theme);
lv_color_t getThemeColor(int index);

#endif
