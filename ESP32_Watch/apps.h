/**
 * Apps Module - Music, Settings, etc.
 */

#ifndef APPS_H
#define APPS_H

#include "config.h"
#include <lvgl.h>

// Initialize apps
void initApps();

// Create screens
lv_obj_t* createMusicScreen();
lv_obj_t* createSettingsScreen();
lv_obj_t* createQuestsScreen();

// Music player
void playMusic();
void pauseMusic();
void nextTrack();
void prevTrack();

// Quest system
void generateDailyQuests();
void updateQuestProgress();
void completeQuest(int id);

extern Quest dailyQuests[5];
extern int numQuests;

#endif
