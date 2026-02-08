/**
 * Games Module Implementation
 */

#include "games.h"
#include "ui_manager.h"
#include "themes.h"

GameSession currentGame = {GAME_BATTLE, 0, 1, false, 0};

// Snake game state
static int snakeX[100], snakeY[100];
static int snakeLen = 3;
static int snakeDir = 0; // 0=right, 1=down, 2=left, 3=up
static int foodX = 100, foodY = 100;

// Memory game state
static int memoryCards[16];
static bool cardFlipped[16];
static int selectedCards[2];
static int matchedPairs = 0;

// Battle state
static BattleCreature playerCreature, enemyCreature;
static lv_obj_t* battleScreen = nullptr;

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initGames() {
  currentGame.playing = false;
  currentGame.score = 0;
  Serial.println("[OK] Games initialized");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  GAMES MENU SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
typedef struct {
  const char* name;
  uint32_t color;
  GameType type;
} GameInfo;

static const GameInfo gameList[] = {
  {"Battle\nArena",   0xE91E63, GAME_BATTLE},
  {"Snake",          0x4CAF50, GAME_SNAKE},
  {"Memory\nMatch",  0x2196F3, GAME_MEMORY},
  {"Shadow\nDungeon", 0x673AB7, GAME_DUNGEON},
  {"Pirate\nQuest",  0xFF9800, GAME_PIRATE},
  {"Portal\nPuzzle", 0x00BCD4, GAME_PORTAL}
};
#define NUM_GAMES 6

lv_obj_t* createGamesScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  // Title bar
  createTitleBar(scr, "Games");
  
  // Game grid (2x3)
  int btnW = 150, btnH = 100;
  int gapX = 15, gapY = 10;
  int startX = (LCD_WIDTH - (2 * btnW + gapX)) / 2;
  int startY = 60;
  
  for (int i = 0; i < NUM_GAMES; i++) {
    int col = i % 2;
    int row = i / 2;
    
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnW, btnH);
    lv_obj_set_pos(btn, startX + col * (btnW + gapX), startY + row * (btnH + gapY));
    lv_obj_set_style_bg_color(btn, lv_color_hex(gameList[i].color), 0);
    lv_obj_set_style_radius(btn, 12, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)gameList[i].type);
    
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      GameType game = (GameType)(intptr_t)lv_obj_get_user_data(btn);
      currentGame.game = game;
      currentGame.playing = true;
      currentGame.startTime = millis();
      
      switch(game) {
        case GAME_BATTLE: startBattleGame(); break;
        case GAME_SNAKE:  startSnakeGame(); break;
        case GAME_MEMORY: startMemoryGame(); break;
        default: break;
      }
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lbl, gameList[i].name);
    lv_obj_center(lbl);
  }
  
  // Stats
  lv_obj_t* statsLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(statsLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(statsLbl, lv_color_hex(colors.secondary), 0);
  lv_label_set_text_fmt(statsLbl, "Games: %d | Wins: %d", userData.gamesPlayed, userData.gamesWon);
  lv_obj_align(statsLbl, LV_ALIGN_BOTTOM_MID, 0, -20);
  
  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  BATTLE ARENA (Pokemon-style)
// ═══════════════════════════════════════════════════════════════════════════════
static const BattleCreature creatures[] = {
  {"Luffy",    100, 100, 25, 15, 20, "Fire",    0xFFA500},
  {"Shadow",   90,  90,  30, 10, 25, "Dark",    0x9A0EEA},
  {"Yugo",     80,  80,  20, 20, 30, "Magic",   0x00CED1},
  {"Slime",    50,  50,  10, 5,  5,  "Normal",  0x4CAF50},
  {"Goblin",   70,  70,  15, 8,  10, "Normal",  0x8BC34A},
  {"Dragon",   120, 120, 35, 25, 15, "Fire",    0xFF5722}
};

void startBattleGame() {
  // Set player based on theme
  playerCreature = creatures[watch.theme];
  
  // Random enemy
  int enemyIdx = 3 + random(3); // Slime, Goblin, or Dragon
  enemyCreature = creatures[enemyIdx];
  
  // Create battle screen
  drawBattleScreen();
}

static lv_obj_t* playerHpBar = nullptr;
static lv_obj_t* enemyHpBar = nullptr;
static lv_obj_t* battleLog = nullptr;

void drawBattleScreen() {
  battleScreen = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(battleScreen, lv_color_hex(colors.background), 0);
  
  // Enemy area (top)
  lv_obj_t* enemyName = lv_label_create(battleScreen);
  lv_obj_set_style_text_color(enemyName, lv_color_hex(colors.text), 0);
  lv_label_set_text(enemyName, enemyCreature.name);
  lv_obj_align(enemyName, LV_ALIGN_TOP_LEFT, 20, 20);
  
  enemyHpBar = lv_bar_create(battleScreen);
  lv_obj_set_size(enemyHpBar, 150, 15);
  lv_bar_set_range(enemyHpBar, 0, enemyCreature.maxHp);
  lv_bar_set_value(enemyHpBar, enemyCreature.hp, LV_ANIM_ON);
  lv_obj_set_style_bg_color(enemyHpBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  lv_obj_align(enemyHpBar, LV_ALIGN_TOP_LEFT, 20, 45);
  
  // Player area (bottom)
  lv_obj_t* playerName = lv_label_create(battleScreen);
  lv_obj_set_style_text_color(playerName, lv_color_hex(colors.text), 0);
  lv_label_set_text(playerName, playerCreature.name);
  lv_obj_align(playerName, LV_ALIGN_LEFT_MID, 20, 50);
  
  playerHpBar = lv_bar_create(battleScreen);
  lv_obj_set_size(playerHpBar, 150, 15);
  lv_bar_set_range(playerHpBar, 0, playerCreature.maxHp);
  lv_bar_set_value(playerHpBar, playerCreature.hp, LV_ANIM_ON);
  lv_obj_set_style_bg_color(playerHpBar, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  lv_obj_align(playerHpBar, LV_ALIGN_LEFT_MID, 20, 75);
  
  // Battle log
  battleLog = lv_label_create(battleScreen);
  lv_obj_set_style_text_font(battleLog, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(battleLog, lv_color_hex(colors.secondary), 0);
  lv_label_set_text(battleLog, "Battle Start!");
  lv_obj_align(battleLog, LV_ALIGN_CENTER, 0, -20);
  
  // Move buttons
  const char* moves[] = {"Attack", "Defend", "Special", "Run"};
  for (int i = 0; i < 4; i++) {
    lv_obj_t* btn = lv_btn_create(battleScreen);
    lv_obj_set_size(btn, 150, 45);
    lv_obj_set_pos(btn, 20 + (i % 2) * 160, 340 + (i / 2) * 50);
    lv_obj_set_style_bg_color(btn, lv_color_hex(colors.primary), 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      int move = (int)(intptr_t)lv_obj_get_user_data(btn);
      executeTurn(move);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, moves[i]);
    lv_obj_center(lbl);
  }
  
  lv_scr_load(battleScreen);
}

void executeTurn(int moveIndex) {
  if (moveIndex == 3) { // Run
    currentGame.playing = false;
    goBack();
    return;
  }
  
  int damage = 0;
  const char* msg = "";
  
  switch(moveIndex) {
    case 0: // Attack
      damage = playerCreature.attack - enemyCreature.defense / 2;
      if (damage < 1) damage = 1;
      enemyCreature.hp -= damage;
      msg = "Attack!";
      break;
    case 1: // Defend
      playerCreature.defense += 5;
      msg = "Defense up!";
      break;
    case 2: // Special
      damage = playerCreature.attack * 1.5;
      enemyCreature.hp -= damage;
      msg = "Special attack!";
      break;
  }
  
  if (battleLog) lv_label_set_text(battleLog, msg);
  
  // Update HP bars
  if (enemyHpBar) lv_bar_set_value(enemyHpBar, enemyCreature.hp, LV_ANIM_ON);
  
  // Enemy turn
  if (enemyCreature.hp > 0) {
    int enemyDmg = enemyCreature.attack - playerCreature.defense / 2;
    if (enemyDmg < 1) enemyDmg = 1;
    playerCreature.hp -= enemyDmg;
    if (playerHpBar) lv_bar_set_value(playerHpBar, playerCreature.hp, LV_ANIM_ON);
  }
  
  // Check win/lose
  if (enemyCreature.hp <= 0) {
    userData.gamesWon++;
    userData.gamesPlayed++;
    if (battleLog) lv_label_set_text(battleLog, "You WIN!");
  } else if (playerCreature.hp <= 0) {
    userData.gamesPlayed++;
    if (battleLog) lv_label_set_text(battleLog, "You LOSE!");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SNAKE GAME
// ═══════════════════════════════════════════════════════════════════════════════
void startSnakeGame() {
  snakeLen = 3;
  snakeDir = 0;
  currentGame.score = 0;
  
  // Initialize snake position
  for (int i = 0; i < snakeLen; i++) {
    snakeX[i] = LCD_WIDTH/2 - i*10;
    snakeY[i] = LCD_HEIGHT/2;
  }
  
  // Random food
  foodX = random(10, LCD_WIDTH - 10);
  foodY = random(60, LCD_HEIGHT - 60);
  
  drawSnake();
}

void updateSnake() {
  // Move snake
  for (int i = snakeLen - 1; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  
  switch(snakeDir) {
    case 0: snakeX[0] += 10; break; // Right
    case 1: snakeY[0] += 10; break; // Down
    case 2: snakeX[0] -= 10; break; // Left
    case 3: snakeY[0] -= 10; break; // Up
  }
  
  // Check food collision
  if (abs(snakeX[0] - foodX) < 15 && abs(snakeY[0] - foodY) < 15) {
    snakeLen++;
    currentGame.score += 10;
    foodX = random(10, LCD_WIDTH - 10);
    foodY = random(60, LCD_HEIGHT - 60);
  }
  
  // Check wall collision
  if (snakeX[0] < 0 || snakeX[0] >= LCD_WIDTH || snakeY[0] < 50 || snakeY[0] >= LCD_HEIGHT - 20) {
    currentGame.playing = false;
    userData.gamesPlayed++;
  }
}

void drawSnake() {
  // Creates a simple snake game screen with touch controls
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
  
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_color(title, lv_color_hex(colors.primary), 0);
  lv_label_set_text(title, "Snake - Tap edges to turn");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  
  lv_obj_t* scoreLbl = lv_label_create(scr);
  lv_obj_set_style_text_color(scoreLbl, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text_fmt(scoreLbl, "Score: %d", currentGame.score);
  lv_obj_align(scoreLbl, LV_ALIGN_TOP_LEFT, 20, 35);
  
  // Direction buttons
  lv_obj_t* upBtn = lv_btn_create(scr);
  lv_obj_set_size(upBtn, 80, 60);
  lv_obj_align(upBtn, LV_ALIGN_BOTTOM_MID, 0, -80);
  lv_obj_add_event_cb(upBtn, [](lv_event_t* e) { snakeDir = 3; }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* upLbl = lv_label_create(upBtn); lv_label_set_text(upLbl, LV_SYMBOL_UP); lv_obj_center(upLbl);
  
  lv_obj_t* downBtn = lv_btn_create(scr);
  lv_obj_set_size(downBtn, 80, 60);
  lv_obj_align(downBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_add_event_cb(downBtn, [](lv_event_t* e) { snakeDir = 1; }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* downLbl = lv_label_create(downBtn); lv_label_set_text(downLbl, LV_SYMBOL_DOWN); lv_obj_center(downLbl);
  
  lv_obj_t* leftBtn = lv_btn_create(scr);
  lv_obj_set_size(leftBtn, 80, 60);
  lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 20, -45);
  lv_obj_add_event_cb(leftBtn, [](lv_event_t* e) { snakeDir = 2; }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* leftLbl = lv_label_create(leftBtn); lv_label_set_text(leftLbl, LV_SYMBOL_LEFT); lv_obj_center(leftLbl);
  
  lv_obj_t* rightBtn = lv_btn_create(scr);
  lv_obj_set_size(rightBtn, 80, 60);
  lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -20, -45);
  lv_obj_add_event_cb(rightBtn, [](lv_event_t* e) { snakeDir = 0; }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* rightLbl = lv_label_create(rightBtn); lv_label_set_text(rightLbl, LV_SYMBOL_RIGHT); lv_obj_center(rightLbl);
  
  lv_scr_load(scr);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MEMORY MATCH GAME
// ═══════════════════════════════════════════════════════════════════════════════
void startMemoryGame() {
  matchedPairs = 0;
  selectedCards[0] = selectedCards[1] = -1;
  
  // Initialize pairs
  for (int i = 0; i < 8; i++) {
    memoryCards[i] = i;
    memoryCards[i + 8] = i;
    cardFlipped[i] = false;
    cardFlipped[i + 8] = false;
  }
  
  // Shuffle
  for (int i = 15; i > 0; i--) {
    int j = random(i + 1);
    int temp = memoryCards[i];
    memoryCards[i] = memoryCards[j];
    memoryCards[j] = temp;
  }
  
  // Create memory game screen
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_color(title, lv_color_hex(colors.text), 0);
  lv_label_set_text(title, "Memory Match");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  
  // 4x4 card grid
  uint32_t cardColors[] = {0xE91E63, 0x4CAF50, 0x2196F3, 0xFF9800, 0x9C27B0, 0x00BCD4, 0xFFEB3B, 0x795548};
  int cardW = 75, cardH = 85;
  int gapX = 10, gapY = 10;
  int startX = (LCD_WIDTH - (4 * cardW + 3 * gapX)) / 2;
  int startY = 50;
  
  for (int i = 0; i < 16; i++) {
    int col = i % 4;
    int row = i / 4;
    
    lv_obj_t* card = lv_btn_create(scr);
    lv_obj_set_size(card, cardW, cardH);
    lv_obj_set_pos(card, startX + col * (cardW + gapX), startY + row * (cardH + gapY));
    lv_obj_set_style_bg_color(card, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_user_data(card, (void*)(intptr_t)i);
    
    lv_obj_add_event_cb(card, [](lv_event_t* e) {
      lv_obj_t* card = lv_event_get_target(e);
      int idx = (int)(intptr_t)lv_obj_get_user_data(card);
      flipCard(idx);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* lbl = lv_label_create(card);
    lv_label_set_text(lbl, "?");
    lv_obj_center(lbl);
  }
  
  lv_scr_load(scr);
}

void flipCard(int index) {
  if (cardFlipped[index]) return;
  
  cardFlipped[index] = true;
  
  if (selectedCards[0] == -1) {
    selectedCards[0] = index;
  } else {
    selectedCards[1] = index;
    checkMatch();
  }
}

void checkMatch() {
  if (selectedCards[0] == -1 || selectedCards[1] == -1) return;
  
  if (memoryCards[selectedCards[0]] == memoryCards[selectedCards[1]]) {
    matchedPairs++;
    currentGame.score += 20;
    
    if (matchedPairs == 8) {
      userData.gamesWon++;
      userData.gamesPlayed++;
    }
  } else {
    // Flip back after delay
    cardFlipped[selectedCards[0]] = false;
    cardFlipped[selectedCards[1]] = false;
  }
  
  selectedCards[0] = selectedCards[1] = -1;
}
