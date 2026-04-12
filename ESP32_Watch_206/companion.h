/*
 * companion.h - Virtual Pet/Companion System with NVS + Detailed Sprites
 * FUSION OS - Virtual Companion System
 */

#ifndef COMPANION_H
#define COMPANION_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"
#include "types.h"

#define COMPANION_COUNT THEME_COUNT

#define STAT_MAX 100
#define STAT_MIN 0
#define STAT_CRITICAL 20
#define STAT_LOW 40

#define FEED_COST_GEMS 5
#define FEED_HUNGER_GAIN 30
#define FEED_HAPPINESS_GAIN 10
#define PLAY_ENERGY_COST 20
#define PLAY_HAPPINESS_GAIN 25
#define PLAY_BOND_GAIN 5
#define TRAIN_ENERGY_COST 30
#define TRAIN_XP_GAIN 15
#define TRAIN_BOND_GAIN 3
#define REST_ENERGY_GAIN 50
#define REST_DURATION_MS 30000

#define BOND_MAX 100
#define BOND_RANK_1 20
#define BOND_RANK_2 40
#define BOND_RANK_3 60
#define BOND_RANK_4 80
#define BOND_RANK_5 100

#define HUNGER_DECAY_RATE 5
#define HAPPINESS_DECAY_RATE 3
#define ENERGY_DECAY_RATE 2

#define COMPANION_NVS_NAMESPACE "companion_data"

enum CompanionMood { MOOD_ECSTATIC = 0, MOOD_HAPPY, MOOD_CONTENT, MOOD_SAD, MOOD_MISERABLE };
enum CompanionEvolution { EVO_BABY = 0, EVO_CHILD, EVO_ADULT, EVO_AWAKENED };
enum CompanionType {
    COMP_SUNNY = 0, COMP_IGRIS, COMP_AZ, COMP_KURAMA, COMP_PUAR,
    COMP_NEZUKO, COMP_SPIRIT, COMP_BLADES, COMP_GENOS, COMP_ALLMIGHT, COMP_OCHOBOT
};

struct CompanionStats {
    int hunger, happiness, energy;
    int bond_level, bond_rank;
    CompanionMood mood;
    CompanionEvolution evolution;
    int total_interactions, days_together;
};

struct CompanionCare {
    unsigned long last_feed_time, last_play_time, last_train_time, last_stat_update;
    int daily_feed_count, daily_play_count, daily_train_count;
    bool is_sleeping;
    unsigned long sleep_start_time;
};

struct CompanionProfile {
    const char* name;
    const char* series;
    uint16_t primary_color;
    uint16_t secondary_color;
    const char* favorite_food;
    const char* favorite_game;
    const char* catchphrase;
};

struct CompanionData {
    CompanionType type;
    CompanionProfile* profile;
    CompanionStats stats;
    CompanionCare care;
};

struct CurrentGame {
    const char* name;
    int score, high_score;
    bool active;
    unsigned long start_time;
    int target_x, target_y;
};

struct CompanionSystemState {
    CompanionData companions[COMPANION_COUNT];
    CompanionData* current_companion;
    int current_companion_index;
    bool in_care_mode, in_mini_game;
    int care_menu_selection;
    CurrentGame current_game;
    Preferences prefs;
    bool nvs_initialized;
    int animation_frame;
    unsigned long last_animation_time;
};

extern CompanionSystemState companion_system;
extern CompanionProfile COMPANION_PROFILES[COMPANION_COUNT];

void initCompanionSystem();
void saveCompanionData();
void loadCompanionData();
void saveCompanionDataForIndex(int index);
void loadCompanionDataForIndex(int index);
void clearAllCompanionData();

void setCurrentCompanion(ThemeType theme);
CompanionData* getCurrentCompanion();
CompanionData* getCompanionByType(CompanionType type);

bool feedCompanion();
bool playWithCompanion();
bool trainCompanion();
void toggleCompanionSleep();
void wakeCompanion();

void updateCompanionStats();
void updateCompanionMood();
void updateCompanionEvolution();
void updateCompanionAnimation();
void checkCompanionDailyReset();

void addBondPoints(int points);
int getBondRank();
const char* getBondRankName();
float getXPBonus();

void startCompanionGame();
void updateCompanionGame();
void endCompanionGame();
void handleCompanionGameTouch(TouchGesture& gesture);

void drawCompanionScreen();
void drawCompanionSprite(int x, int y, CompanionType type, CompanionEvolution evo);
void drawCompanionStats();
void drawCareMenu();
void drawCompanionMood();
void drawBondLevel();
void drawCompanionGame();
void drawCompanionAnimation();

// Individual detailed sprite functions
void drawSunnySprite(int x, int y, int size, int frame);
void drawShadowSprite(int x, int y, int size, int frame);
void drawAzSprite(int x, int y, int size, int frame);
void drawKuramaSprite(int x, int y, int size, int frame);
void drawPuarSprite(int x, int y, int size, int frame);
void drawNezukoSprite(int x, int y, int size, int frame);
void drawSpiritSprite(int x, int y, int size, int frame);
void drawBladesSprite(int x, int y, int size, int frame);
void drawGenosSprite(int x, int y, int size, int frame);
void drawAllMightSprite(int x, int y, int size, int frame);
void drawOchobotSprite(int x, int y, int size, int frame);

void handleCompanionScreenTouch(TouchGesture& gesture);
void handleCareMenuTouch(TouchGesture& gesture);

const char* getMoodText(CompanionMood mood);
uint16_t getMoodColor(CompanionMood mood);
const char* getEvolutionText(CompanionEvolution evo);
int getStatPercentage(int stat);
bool isStatCritical(int stat);
bool isStatLow(int stat);

#endif
