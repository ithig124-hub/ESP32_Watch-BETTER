/**
 * Anime-Themed Games - Gacha, Training, Boss Rush
 */

#ifndef GAMES_H
#define GAMES_H

#include <lvgl.h>
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  GACHA TYPES
// ═══════════════════════════════════════════════════════════════════════════════
enum GachaRarity {
  RARITY_COMMON,
  RARITY_RARE,
  RARITY_EPIC,
  RARITY_LEGENDARY,
  RARITY_MYTHIC
};

struct GachaCard {
  const char* name;
  const char* series;
  GachaRarity rarity;
  int power;
  bool owned;
};

// Game initialization
void initGames();

// Game screens
lv_obj_t* createGachaScreen();
lv_obj_t* createTrainingScreen();
lv_obj_t* createBossRushScreen();

// Gacha system
void performGachaPull();
GachaRarity rollRarity();
const char* getRarityName(GachaRarity rarity);
uint32_t getRarityColor(GachaRarity rarity);

// Training mini-games
void startTrainingGame(int gameType);
void updateTrainingGame();

// Boss rush
void startBossRush();
void attackBoss();
void updateBossRush();

#endif
