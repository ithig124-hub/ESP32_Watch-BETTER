/**
 * Games Module - 6 Mini-Games
 */

#ifndef GAMES_H
#define GAMES_H

#include "config.h"
#include <lvgl.h>

// Initialize games
void initGames();

// Create games screen
lv_obj_t* createGamesScreen();

// Individual games
void startBattleGame();
void startSnakeGame();
void startMemoryGame();
void startDungeonGame();
void startPirateGame();
void startPortalGame();

// Battle system (Pokemon-style)
struct BattleCreature {
  const char* name;
  int hp;
  int maxHp;
  int attack;
  int defense;
  int speed;
  const char* type;
  uint32_t color;
};

void initBattle(BattleCreature* player, BattleCreature* enemy);
void executeTurn(int moveIndex);
void drawBattleScreen();

// Snake game
void updateSnake();
void drawSnake();

// Memory match
void flipCard(int index);
void checkMatch();

// Game session
struct GameSession {
  GameType game;
  int score;
  int level;
  bool playing;
  unsigned long startTime;
};

extern GameSession currentGame;

#endif
