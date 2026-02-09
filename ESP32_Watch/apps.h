/**
 * Apps - Music, Settings, Quests
 */

#ifndef APPS_H
#define APPS_H

#include "config.h"
#include <lvgl.h>

void initApps();

// Forward declarations (implemented in ui_manager.cpp)
lv_obj_t* createMusicScreen();
lv_obj_t* createSettingsScreen();
lv_obj_t* createQuestsScreen();

#endif
