/*
 * games.cpp - Games Implementation
 * Full game functionality with battle system
 */

#include "games.h"
#include "config.h"
#include "display.h"
#include "themes.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

GameSession current_game_session;

// Battle creatures database
BattleCreature battle_creatures[] = {
  {"Luffy", "Fighter", 50, 100, 100, 80, 60, 70, {"Gum Gum Pistol", "Gear Second", "Red Hawk", "King Kong Gun"}, {30, 45, 60, 80}, true, COLOR_ORANGE},
  {"Jinwoo", "Shadow", 50, 100, 100, 90, 70, 85, {"Dagger Strike", "Shadow Exchange", "Ruler's Authority", "Dragon's Fear"}, {35, 50, 70, 90}, true, JINWOO_PURPLE},
  {"Yugo", "Portal", 50, 100, 100, 70, 80, 75, {"Portal Slash", "Wakfu Beam", "Stasis", "Eliacube Power"}, {25, 40, 55, 75}, true, YUGO_TEAL},
  {"Shadow Soldier", "Dark", 20, 60, 60, 40, 30, 35, {"Slash", "Dark Bite", "", ""}, {20, 30, 0, 0}, false, RGB565(50, 0, 80)},
  {"Pirate", "Normal", 15, 50, 50, 35, 25, 30, {"Cutlass", "Cannonball", "", ""}, {15, 25, 0, 0}, false, COLOR_RED},
  {"Portal Beast", "Magic", 25, 70, 70, 50, 40, 45, {"Teleport", "Energy Blast", "Confusion", ""}, {25, 35, 20, 0}, false, YUGO_ENERGY}
};
int num_battle_creatures = 6;

// AdvancedGameManager static members
GameType AdvancedGameManager::current_game = GAME_BATTLE_ARENA;
GameState AdvancedGameManager::current_state = GAME_MENU;

// =============================================================================
// GAME INITIALIZATION
// =============================================================================

void initializeGames() {
  Serial.println("[Games] Initializing game system...");
  current_game_session.state = GAME_MENU;
  current_game_session.score = 0;
  current_game_session.level = 1;
  current_game_session.lives = 3;
  current_game_session.sound_enabled = true;
  loadGameProgress();
}

void launchGame(GameType game) {
  current_game_session.current_game = game;
  current_game_session.start_time = millis();
  
  switch(game) {
    case GAME_BATTLE_ARENA:    initBattleArena(); break;
    case GAME_SHADOW_DUNGEON:  initShadowDungeon(); break;
    case GAME_PIRATE_ADVENTURE: initPirateAdventure(); break;
    case GAME_WAKFU_QUEST:     initWakfuQuest(); break;
    case GAME_MINI_SNAKE:      initSnakeGame(); break;
    case GAME_MEMORY_MATCH:    initMemoryGame(); break;
  }
}

void drawGameMenu() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Title
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(3);
  gfx->setCursor(100, 30);
  gfx->print("GAMES");
  
  // Game buttons
  const char* games[] = {"Battle Arena", "Shadow Dungeon", "Pirate Quest", "Wakfu", "Snake", "Memory"};
  for (int i = 0; i < 6; i++) {
    int x = (i % 2) * 180 + 10;
    int y = (i / 2) * 90 + 100;
    drawGameButton(x, y, 170, 80, games[i], false);
  }
  
  // Back button
  drawThemeButton(130, 400, 100, 40, "Back", false);
}

void handleGameMenuTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check game buttons
  for (int i = 0; i < 6; i++) {
    int bx = (i % 2) * 180 + 10;
    int by = (i / 2) * 90 + 100;
    if (x >= bx && x < bx + 170 && y >= by && y < by + 80) {
      launchGame((GameType)i);
      return;
    }
  }
  
  // Back button
  if (y >= 400 && y < 440 && x >= 130 && x < 230) {
    system_state.current_screen = SCREEN_APP_GRID;
  }
}

// =============================================================================
// BATTLE ARENA
// =============================================================================

void initBattleArena() {
  current_game_session.state = GAME_BATTLE_SELECT;
  
  // Set player creature based on theme
  switch(system_state.current_theme) {
    case THEME_LUFFY_GEAR5:  current_game_session.player_creature = battle_creatures[0]; break;
    case THEME_SUNG_JINWOO:  current_game_session.player_creature = battle_creatures[1]; break;
    case THEME_YUGO_WAKFU:   current_game_session.player_creature = battle_creatures[2]; break;
    default: current_game_session.player_creature = battle_creatures[0]; break;
  }
  
  generateRandomEnemy();
  current_game_session.log_count = 0;
}

void drawBattleArena() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Title
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 10);
  gfx->print("BATTLE ARENA");
  
  // Enemy
  BattleCreature& enemy = current_game_session.enemy_creature;
  gfx->setTextColor(enemy.color);
  gfx->setTextSize(2);
  gfx->setCursor(20, 50);
  gfx->print(enemy.name);
  gfx->setCursor(20, 70);
  gfx->printf("Lv.%d", enemy.level);
  drawHealthBar(20, 95, 200, 20, enemy.hp, enemy.max_hp, enemy.color);
  
  // Player
  BattleCreature& player = current_game_session.player_creature;
  gfx->setTextColor(player.color);
  gfx->setTextSize(2);
  gfx->setCursor(20, 200);
  gfx->print(player.name);
  gfx->setCursor(20, 220);
  gfx->printf("Lv.%d", player.level);
  drawHealthBar(20, 245, 200, 20, player.hp, player.max_hp, player.color);
  
  // Move buttons
  for (int i = 0; i < 4; i++) {
    if (player.moves[i].length() > 0) {
      int x = (i % 2) * 180 + 10;
      int y = (i / 2) * 60 + 300;
      drawGameButton(x, y, 170, 55, player.moves[i].c_str(), current_game_session.selected_move == i);
    }
  }
  
  // Battle log
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  for (int i = 0; i < min(3, current_game_session.log_count); i++) {
    gfx->setCursor(10, 130 + i * 12);
    gfx->print(current_game_session.battle_log[i]);
  }
}

void handleBattleTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check move buttons
  for (int i = 0; i < 4; i++) {
    int bx = (i % 2) * 180 + 10;
    int by = (i / 2) * 60 + 300;
    if (x >= bx && x < bx + 170 && y >= by && y < by + 55) {
      selectMove(i);
      return;
    }
  }
}

void startBattle() {
  current_game_session.state = GAME_BATTLE_FIGHT;
  addBattleLog("Battle Start!");
}

void selectMove(int move_index) {
  current_game_session.selected_move = move_index;
  executeBattleTurn();
}

void executeBattleTurn() {
  BattleCreature& player = current_game_session.player_creature;
  BattleCreature& enemy = current_game_session.enemy_creature;
  
  // Player attacks
  int move_idx = current_game_session.selected_move;
  int damage = calculateDamage(player, enemy, player.move_power[move_idx]);
  enemy.hp -= damage;
  addBattleLog(player.name + " used " + player.moves[move_idx] + "!");
  addBattleLog("Dealt " + String(damage) + " damage!");
  
  if (enemy.hp <= 0) {
    enemy.hp = 0;
    addBattleLog("Enemy defeated!");
    current_game_session.state = GAME_WIN;
    current_game_session.score += enemy.level * 10;
    return;
  }
  
  // Enemy attacks
  int enemy_move = random(0, 2);
  damage = calculateDamage(enemy, player, enemy.move_power[enemy_move]);
  player.hp -= damage;
  addBattleLog(enemy.name + " used " + enemy.moves[enemy_move] + "!");
  
  if (player.hp <= 0) {
    player.hp = 0;
    addBattleLog("You were defeated!");
    current_game_session.state = GAME_OVER;
  }
}

void dealDamage(BattleCreature& attacker, BattleCreature& defender, int move_index) {
  int damage = calculateDamage(attacker, defender, attacker.move_power[move_index]);
  defender.hp -= damage;
  if (defender.hp < 0) defender.hp = 0;
}

void addBattleLog(String message) {
  if (current_game_session.log_count < 5) {
    current_game_session.battle_log[current_game_session.log_count++] = message;
  } else {
    for (int i = 0; i < 4; i++) {
      current_game_session.battle_log[i] = current_game_session.battle_log[i + 1];
    }
    current_game_session.battle_log[4] = message;
  }
}

void checkBattleEnd() {
  if (current_game_session.enemy_creature.hp <= 0) {
    current_game_session.state = GAME_WIN;
  }
  if (current_game_session.player_creature.hp <= 0) {
    current_game_session.state = GAME_OVER;
  }
}

void generateRandomEnemy() {
  int idx = random(3, num_battle_creatures);
  current_game_session.enemy_creature = battle_creatures[idx];
  current_game_session.enemy_creature.level = current_game_session.level * 5 + random(1, 10);
  current_game_session.enemy_creature.max_hp = 50 + current_game_session.enemy_creature.level * 2;
  current_game_session.enemy_creature.hp = current_game_session.enemy_creature.max_hp;
}

// =============================================================================
// SHADOW DUNGEON
// =============================================================================

void initShadowDungeon() {
  current_game_session.state = GAME_PLAYING;
  current_game_session.player_x = 5;
  current_game_session.player_y = 5;
  current_game_session.map_level = 1;
  current_game_session.items_collected = 0;
}

void drawShadowDungeon() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(JINWOO_PURPLE);
  gfx->setTextSize(2);
  gfx->setCursor(60, 10);
  gfx->print("SHADOW DUNGEON");
  
  // Draw map grid
  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      int px = x * 35 + 20;
      int py = y * 35 + 60;
      gfx->drawRect(px, py, 32, 32, JINWOO_DARK);
      
      if (x == current_game_session.player_x && y == current_game_session.player_y) {
        gfx->fillCircle(px + 16, py + 16, 12, JINWOO_PURPLE);
      }
    }
  }
  
  // Stats
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 420);
  gfx->printf("Floor: %d  Items: %d", current_game_session.map_level, current_game_session.items_collected);
}

void handleDungeonTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_SWIPE_UP) movePlayer(0, -1);
  else if (gesture.event == TOUCH_SWIPE_DOWN) movePlayer(0, 1);
  else if (gesture.event == TOUCH_SWIPE_LEFT) movePlayer(-1, 0);
  else if (gesture.event == TOUCH_SWIPE_RIGHT) movePlayer(1, 0);
}

void movePlayer(int dx, int dy) {
  int newX = current_game_session.player_x + dx;
  int newY = current_game_session.player_y + dy;
  
  if (newX >= 0 && newX < 10 && newY >= 0 && newY < 10) {
    current_game_session.player_x = newX;
    current_game_session.player_y = newY;
  }
}

void fightShadowMonster() {
  current_game_session.score += 10;
}

void levelUpPlayer() {
  current_game_session.level++;
}

// =============================================================================
// PIRATE ADVENTURE
// =============================================================================

void initPirateAdventure() {
  current_game_session.state = GAME_PLAYING;
  current_game_session.items_collected = 0;
}

void drawPirateAdventure() {
  gfx->fillScreen(COLOR_BLACK);
  gfx->setTextColor(COLOR_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(50, 10);
  gfx->print("PIRATE ADVENTURE");
  
  // Simple treasure hunt UI
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 200);
  gfx->printf("Treasures Found: %d", current_game_session.items_collected);
  
  drawThemeButton(100, 300, 160, 50, "Explore", false);
}

void handlePirateTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 300 && gesture.y < 350) {
    exploreIsland();
  }
}

void exploreIsland() {
  if (random(0, 100) < 30) {
    findTreasure();
  }
}

void findTreasure() {
  current_game_session.items_collected++;
  current_game_session.score += 20;
}

// =============================================================================
// WAKFU QUEST
// =============================================================================

void initWakfuQuest() {
  current_game_session.state = GAME_PLAYING;
}

void drawWakfuQuest() {
  gfx->fillScreen(COLOR_BLACK);
  gfx->setTextColor(YUGO_TEAL);
  gfx->setTextSize(2);
  gfx->setCursor(80, 10);
  gfx->print("WAKFU QUEST");
}

void handleWakfuTouch(TouchGesture& gesture) {
  // Handle puzzle interactions
}

void solvePuzzle() {
  current_game_session.score += 15;
}

void openPortal() {
  current_game_session.map_level++;
}

// =============================================================================
// SNAKE GAME
// =============================================================================

void initSnakeGame() {
  current_game_session.state = GAME_PLAYING;
  current_game_session.snake_length = 3;
  current_game_session.direction = 1;  // 0=up, 1=right, 2=down, 3=left
  
  for (int i = 0; i < current_game_session.snake_length; i++) {
    current_game_session.snake_x[i] = 5 - i;
    current_game_session.snake_y[i] = 5;
  }
  
  current_game_session.food_x = random(1, 18);
  current_game_session.food_y = random(1, 18);
}

void drawSnakeGame() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(COLOR_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(130, 10);
  gfx->print("SNAKE");
  
  // Draw grid
  gfx->drawRect(20, 50, 320, 320, COLOR_GREEN);
  
  // Draw snake
  for (int i = 0; i < current_game_session.snake_length; i++) {
    int x = current_game_session.snake_x[i] * 16 + 20;
    int y = current_game_session.snake_y[i] * 16 + 50;
    gfx->fillRect(x, y, 14, 14, i == 0 ? COLOR_GREEN : RGB565(0, 150, 0));
  }
  
  // Draw food
  int fx = current_game_session.food_x * 16 + 20;
  int fy = current_game_session.food_y * 16 + 50;
  gfx->fillRect(fx, fy, 14, 14, COLOR_RED);
  
  // Score
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 400);
  gfx->printf("Score: %d  Length: %d", current_game_session.score, current_game_session.snake_length);
}

void handleSnakeTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_SWIPE_UP && current_game_session.direction != 2) 
    current_game_session.direction = 0;
  else if (gesture.event == TOUCH_SWIPE_RIGHT && current_game_session.direction != 3) 
    current_game_session.direction = 1;
  else if (gesture.event == TOUCH_SWIPE_DOWN && current_game_session.direction != 0) 
    current_game_session.direction = 2;
  else if (gesture.event == TOUCH_SWIPE_LEFT && current_game_session.direction != 1) 
    current_game_session.direction = 3;
}

void updateSnake() {
  static unsigned long lastMove = 0;
  if (millis() - lastMove < 200) return;
  lastMove = millis();
  
  // Move body
  for (int i = current_game_session.snake_length - 1; i > 0; i--) {
    current_game_session.snake_x[i] = current_game_session.snake_x[i - 1];
    current_game_session.snake_y[i] = current_game_session.snake_y[i - 1];
  }
  
  // Move head
  switch (current_game_session.direction) {
    case 0: current_game_session.snake_y[0]--; break;
    case 1: current_game_session.snake_x[0]++; break;
    case 2: current_game_session.snake_y[0]++; break;
    case 3: current_game_session.snake_x[0]--; break;
  }
  
  // Check food collision
  if (current_game_session.snake_x[0] == current_game_session.food_x &&
      current_game_session.snake_y[0] == current_game_session.food_y) {
    current_game_session.snake_length++;
    current_game_session.score += 10;
    current_game_session.food_x = random(1, 18);
    current_game_session.food_y = random(1, 18);
  }
  
  checkSnakeCollision();
}

void checkSnakeCollision() {
  int x = current_game_session.snake_x[0];
  int y = current_game_session.snake_y[0];
  
  // Wall collision
  if (x < 0 || x >= 20 || y < 0 || y >= 20) {
    current_game_session.state = GAME_OVER;
    return;
  }
  
  // Self collision
  for (int i = 1; i < current_game_session.snake_length; i++) {
    if (x == current_game_session.snake_x[i] && y == current_game_session.snake_y[i]) {
      current_game_session.state = GAME_OVER;
      return;
    }
  }
}

// =============================================================================
// MEMORY MATCH
// =============================================================================

static int memory_cards[16];
static bool memory_revealed[16];
static int first_card = -1, second_card = -1;
static int matches_found = 0;

void initMemoryGame() {
  current_game_session.state = GAME_PLAYING;
  matches_found = 0;
  first_card = second_card = -1;
  
  // Setup card pairs
  for (int i = 0; i < 8; i++) {
    memory_cards[i * 2] = i;
    memory_cards[i * 2 + 1] = i;
  }
  
  // Shuffle
  for (int i = 15; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = memory_cards[i];
    memory_cards[i] = memory_cards[j];
    memory_cards[j] = temp;
  }
  
  for (int i = 0; i < 16; i++) memory_revealed[i] = false;
}

void drawMemoryGame() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 10);
  gfx->print("MEMORY MATCH");
  
  uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, 
                       COLOR_ORANGE, COLOR_CYAN, COLOR_PINK, JINWOO_PURPLE};
  
  for (int i = 0; i < 16; i++) {
    int x = (i % 4) * 85 + 25;
    int y = (i / 4) * 85 + 60;
    
    if (memory_revealed[i]) {
      gfx->fillRoundRect(x, y, 75, 75, 10, colors[memory_cards[i]]);
    } else {
      gfx->fillRoundRect(x, y, 75, 75, 10, RGB565(60, 60, 60));
      gfx->drawRoundRect(x, y, 75, 75, 10, getCurrentTheme()->primary);
    }
  }
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 420);
  gfx->printf("Matches: %d/8  Score: %d", matches_found, current_game_session.score);
}

void handleMemoryTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int cardX = (gesture.x - 25) / 85;
  int cardY = (gesture.y - 60) / 85;
  
  if (cardX >= 0 && cardX < 4 && cardY >= 0 && cardY < 4) {
    flipCard(cardX, cardY);
  }
}

void flipCard(int x, int y) {
  int idx = y * 4 + x;
  if (memory_revealed[idx]) return;
  
  memory_revealed[idx] = true;
  
  if (first_card < 0) {
    first_card = idx;
  } else if (second_card < 0) {
    second_card = idx;
    checkMatch();
  }
}

void checkMatch() {
  if (memory_cards[first_card] == memory_cards[second_card]) {
    matches_found++;
    current_game_session.score += 20;
    
    if (matches_found >= 8) {
      current_game_session.state = GAME_WIN;
    }
  } else {
    delay(500);
    memory_revealed[first_card] = false;
    memory_revealed[second_card] = false;
  }
  
  first_card = second_card = -1;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

void drawHealthBar(int x, int y, int w, int h, int current_hp, int max_hp, uint16_t color) {
  gfx->drawRect(x, y, w, h, COLOR_WHITE);
  int fillW = (current_hp * (w - 4)) / max_hp;
  if (fillW > 0) gfx->fillRect(x + 2, y + 2, fillW, h - 4, color);
}

void drawProgressBar(int x, int y, int w, int h, float progress, uint16_t color) {
  progress = constrain(progress, 0.0f, 1.0f);
  gfx->drawRect(x, y, w, h, COLOR_WHITE);
  int fillW = (int)(progress * (w - 4));
  if (fillW > 0) gfx->fillRect(x + 2, y + 2, fillW, h - 4, color);
}

void showGameNotification(String title, String message) {
  drawThemeNotification(title.c_str(), message.c_str());
}

int calculateDamage(BattleCreature& attacker, BattleCreature& defender, int move_power) {
  float base = (attacker.attack * move_power) / (float)(defender.defense + 50);
  return max(1, (int)(base * (0.85 + random(0, 30) / 100.0)));
}

String getTypeEffectiveness(String attack_type, String defend_type) {
  // Type effectiveness chart (simplified)
  return "Normal";
}

void saveGameProgress() {
  // Save to EEPROM or file
}

void loadGameProgress() {
  // Load from EEPROM or file
}

// AdvancedGameManager implementation
void AdvancedGameManager::initGame(GameType type) {
  current_game = type;
  current_state = GAME_MENU;
  launchGame(type);
}

void AdvancedGameManager::updateGame() {
  if (current_game == GAME_MINI_SNAKE && current_state == GAME_PLAYING) {
    updateSnake();
  }
}

void AdvancedGameManager::drawGame() {
  switch(current_game) {
    case GAME_BATTLE_ARENA: drawBattleArena(); break;
    case GAME_SHADOW_DUNGEON: drawShadowDungeon(); break;
    case GAME_PIRATE_ADVENTURE: drawPirateAdventure(); break;
    case GAME_WAKFU_QUEST: drawWakfuQuest(); break;
    case GAME_MINI_SNAKE: drawSnakeGame(); break;
    case GAME_MEMORY_MATCH: drawMemoryGame(); break;
  }
}

void AdvancedGameManager::handleGameInput(TouchGesture& gesture) {
  switch(current_game) {
    case GAME_BATTLE_ARENA: handleBattleTouch(gesture); break;
    case GAME_SHADOW_DUNGEON: handleDungeonTouch(gesture); break;
    case GAME_PIRATE_ADVENTURE: handlePirateTouch(gesture); break;
    case GAME_WAKFU_QUEST: handleWakfuTouch(gesture); break;
    case GAME_MINI_SNAKE: handleSnakeTouch(gesture); break;
    case GAME_MEMORY_MATCH: handleMemoryTouch(gesture); break;
  }
}

GameState AdvancedGameManager::getCurrentState() { return current_state; }
void AdvancedGameManager::setCurrentState(GameState state) { current_state = state; }
