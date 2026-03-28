/*
 * character_games.cpp - Character-Specific Minigames
 * Unique gameplay for each anime character
 */

#include "character_games.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"
#include "ochobot.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

CharacterGameState char_game_state = {
  GAME_LUFFY_PUNCH, false, 0, 0, 3, 1, 0, 0
};

ElementBattleState element_battle = {
  0, 0, 100, 100, 0, false, false, 0, 0
};

// Element colors for battle
uint16_t battleElementColors[] = {
  BBB_LIGHTNING_YELLOW, BBB_WIND_BLUE, BBB_EARTH_BROWN,
  BBB_FIRE_RED, BBB_WATER_CYAN, BBB_LEAF_GREEN, BBB_LIGHT_GOLD
};

const char* battleElementNames[] = {
  "Lightning", "Wind", "Earth", "Fire", "Ice", "Leaf", "Light"
};

// Luffy game state
static struct {
  int target_x;
  int target_y;
  int target_size;
  bool target_active;
  unsigned long target_spawn_time;
  int stretch_power;
  bool is_stretching;
} luffy_game;

// Gojo game state
static struct {
  int player_x;
  int player_y;
  int projectiles[10][3];  // x, y, speed
  int projectile_count;
  unsigned long last_spawn;
  int domain_radius;
  bool domain_expanding;
} gojo_game;

// Levi game state
static struct {
  int titans[5][4];  // x, y, hp, type
  int titan_count;
  int slash_x;
  int slash_y;
  bool slashing;
  int slash_angle;
  int kill_count;
} levi_game;

// =============================================================================
// INITIALIZATION
// =============================================================================

void initCharacterGames() {
  char_game_state.is_active = false;
  char_game_state.score = 0;
  char_game_state.combo = 0;
  char_game_state.lives = 3;
  char_game_state.level = 1;
}

CharacterGameType getGameForTheme(ThemeType theme) {
  switch (theme) {
    case THEME_LUFFY_GEAR5:    return GAME_LUFFY_PUNCH;
    case THEME_GOJO_INFINITY:  return GAME_GOJO_DODGE;
    case THEME_LEVI_STRONGEST: return GAME_LEVI_SLASH;
    case THEME_NARUTO_SAGE:    return GAME_NARUTO_JUTSU;
    case THEME_GOKU_UI:        return GAME_GOKU_KAMEHAMEHA;
    case THEME_TANJIRO_SUN:    return GAME_TANJIRO_BREATH;
    case THEME_SUNG_JINWOO:    return GAME_JINWOO_ARISE;
    case THEME_DEKU_PLUSULTRA: return GAME_DEKU_SMASH;
    case THEME_SAITAMA_OPM:    return GAME_SAITAMA_PUNCH;
    case THEME_BOBOIBOY:       return GAME_BOBOIBOY_ELEMENT;
    default: return GAME_LUFFY_PUNCH;
  }
}

void startCharacterGame(CharacterGameType game) {
  char_game_state.game_type = game;
  char_game_state.is_active = true;
  char_game_state.score = 0;
  char_game_state.combo = 0;
  char_game_state.lives = 3;
  char_game_state.level = 1;
  char_game_state.game_start_time = millis();
  char_game_state.last_update = millis();
  
  // Initialize specific game
  switch (game) {
    case GAME_LUFFY_PUNCH:
      luffy_game.target_active = false;
      luffy_game.stretch_power = 0;
      luffy_game.is_stretching = false;
      break;
      
    case GAME_GOJO_DODGE:
      gojo_game.player_x = LCD_WIDTH / 2;
      gojo_game.player_y = LCD_HEIGHT - 80;
      gojo_game.projectile_count = 0;
      gojo_game.domain_expanding = false;
      break;
      
    case GAME_LEVI_SLASH:
      levi_game.titan_count = 0;
      levi_game.slashing = false;
      levi_game.kill_count = 0;
      break;
      
    case GAME_BOBOIBOY_ELEMENT:
      initElementBattle();
      break;
      
    default:
      break;
  }
  
  Serial.printf("[GAME] Started character game: %d\n", game);
  drawCharacterGame();
}

void updateCharacterGame() {
  if (!char_game_state.is_active) return;
  
  switch (char_game_state.game_type) {
    case GAME_LUFFY_PUNCH:
      updateLuffyPunchGame();
      break;
    case GAME_GOJO_DODGE:
      updateGojoDodgeGame();
      break;
    case GAME_LEVI_SLASH:
      updateLeviSlashGame();
      break;
    case GAME_BOBOIBOY_ELEMENT:
      updateElementBattle();
      break;
    case GAME_SAITAMA_PUNCH:
      // Saitama game is instant - one punch ends it
      break;
    default:
      break;
  }
  
  char_game_state.last_update = millis();
}

void handleCharacterGameTouch(TouchGesture& gesture) {
  // Back button
  if (gesture.event == TOUCH_TAP && gesture.y > 410 && gesture.x < 80) {
    endCharacterGame();
    returnToAppGrid();
    return;
  }
  
  switch (char_game_state.game_type) {
    case GAME_LUFFY_PUNCH:
      handleLuffyPunchTouch(gesture);
      break;
    case GAME_GOJO_DODGE:
      handleGojoDodgeTouch(gesture);
      break;
    case GAME_LEVI_SLASH:
      handleLeviSlashTouch(gesture);
      break;
    case GAME_BOBOIBOY_ELEMENT:
      handleElementBattleTouch(gesture);
      break;
    case GAME_SAITAMA_PUNCH:
      // Any tap wins instantly
      if (gesture.event == TOUCH_TAP) {
        char_game_state.score = 9999;
        endCharacterGame();
      }
      break;
    default:
      break;
  }
}

void drawCharacterGame() {
  switch (char_game_state.game_type) {
    case GAME_LUFFY_PUNCH:
      drawLuffyPunchGame();
      break;
    case GAME_GOJO_DODGE:
      drawGojoDodgeGame();
      break;
    case GAME_LEVI_SLASH:
      drawLeviSlashGame();
      break;
    case GAME_BOBOIBOY_ELEMENT:
      drawElementBattle();
      break;
    case GAME_SAITAMA_PUNCH:
      // Draw simple one-punch screen
      gfx->fillScreen(COLOR_BLACK);
      gfx->setTextColor(SAITAMA_HERO_YELLOW);
      gfx->setTextSize(2);
      gfx->setCursor(80, 180);
      gfx->print("TAP TO PUNCH");
      gfx->setTextSize(3);
      gfx->setCursor(140, 230);
      gfx->print("OK.");
      break;
    default:
      break;
  }
}

void endCharacterGame() {
  char_game_state.is_active = false;
  
  // Award gems based on score
  int gems = char_game_state.score / 10;
  system_state.player_gems += gems;
  
  Serial.printf("[GAME] Ended. Score: %d, Gems: +%d\n", char_game_state.score, gems);
}

// =============================================================================
// LUFFY RUBBER PUNCH GAME
// Tap when arm is stretched to max for perfect timing!
// =============================================================================

void drawLuffyPunchGame() {
  // Golden background
  gfx->fillScreen(RGB565(15, 12, 5));
  
  // Header
  gfx->fillRect(0, 0, LCD_WIDTH, 45, RGB565(30, 25, 15));
  gfx->setTextColor(LUFFY_SUN_GOLD);
  gfx->setTextSize(2);
  gfx->setCursor(60, 12);
  gfx->print("GOMU GOMU PUNCH!");
  
  // Score and combo
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(20, 55);
  gfx->printf("Score: %d  Combo: %d  Lives: %d", 
              char_game_state.score, char_game_state.combo, char_game_state.lives);
  
  // Draw target
  if (luffy_game.target_active) {
    // Target circle
    gfx->drawCircle(luffy_game.target_x, luffy_game.target_y, 
                    luffy_game.target_size, LUFFY_ENERGY_ORANGE);
    gfx->drawCircle(luffy_game.target_x, luffy_game.target_y, 
                    luffy_game.target_size - 5, LUFFY_SUN_GOLD);
    
    // Shrinking timing ring
    unsigned long elapsed = millis() - luffy_game.target_spawn_time;
    int ringSize = luffy_game.target_size + 30 - (elapsed / 30);
    if (ringSize > luffy_game.target_size) {
      gfx->drawCircle(luffy_game.target_x, luffy_game.target_y, ringSize, COLOR_WHITE);
    }
  }
  
  // Draw stretching arm
  int armStartX = 50;
  int armStartY = LCD_HEIGHT - 100;
  int armLength = 50 + luffy_game.stretch_power * 2;
  
  // Arm (rubber stretch effect)
  for (int i = 0; i < 5; i++) {
    int wave = sin((millis() + i * 20) / 50.0) * (luffy_game.is_stretching ? 3 : 1);
    gfx->drawLine(armStartX + i * (armLength / 5), armStartY + wave,
                  armStartX + (i + 1) * (armLength / 5), armStartY - wave,
                  LUFFY_RUBBER_PINK);
  }
  
  // Fist
  int fistX = armStartX + armLength;
  gfx->fillCircle(fistX, armStartY, 15, LUFFY_RUBBER_PINK);
  gfx->drawCircle(fistX, armStartY, 15, COLOR_WHITE);
  
  // Power meter
  gfx->drawRect(50, LCD_HEIGHT - 50, 200, 20, COLOR_WHITE);
  int meterWidth = luffy_game.stretch_power * 2;
  uint16_t meterColor = luffy_game.stretch_power > 80 ? COLOR_GREEN : 
                        luffy_game.stretch_power > 50 ? LUFFY_SUN_GOLD : COLOR_RED;
  gfx->fillRect(52, LCD_HEIGHT - 48, meterWidth, 16, meterColor);
  
  gfx->setTextSize(1);
  gfx->setCursor(260, LCD_HEIGHT - 45);
  gfx->print("POWER");
  
  // Instructions
  gfx->setTextColor(RGB565(150, 140, 100));
  gfx->setCursor(80, 400);
  gfx->print("Hold to stretch, release at target!");
  
  // Back button
  drawSwipeIndicator();
}

void updateLuffyPunchGame() {
  // Spawn target if none active
  if (!luffy_game.target_active && random(100) < 5) {
    luffy_game.target_x = 150 + random(150);
    luffy_game.target_y = 150 + random(150);
    luffy_game.target_size = 30 + random(20);
    luffy_game.target_active = true;
    luffy_game.target_spawn_time = millis();
  }
  
  // Check target timeout
  if (luffy_game.target_active) {
    if (millis() - luffy_game.target_spawn_time > 2000) {
      luffy_game.target_active = false;
      char_game_state.combo = 0;
      char_game_state.lives--;
      
      if (char_game_state.lives <= 0) {
        endCharacterGame();
      }
    }
  }
  
  // Update stretch if holding
  if (luffy_game.is_stretching && luffy_game.stretch_power < 100) {
    luffy_game.stretch_power += 2;
  }
  
  drawLuffyPunchGame();
}

void handleLuffyPunchTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_PRESS) {
    luffy_game.is_stretching = true;
    luffy_game.stretch_power = 0;
  }
  else if (gesture.event == TOUCH_RELEASE || gesture.event == TOUCH_TAP) {
    if (luffy_game.is_stretching && luffy_game.target_active) {
      // Check if power is in good range (70-90 is perfect)
      int power = luffy_game.stretch_power;
      
      if (power >= 70 && power <= 90) {
        // Perfect hit!
        char_game_state.score += 100 * (1 + char_game_state.combo / 5);
        char_game_state.combo++;
        luffy_game.target_active = false;
      } else if (power >= 50) {
        // OK hit
        char_game_state.score += 50;
        char_game_state.combo++;
        luffy_game.target_active = false;
      } else {
        // Miss
        char_game_state.combo = 0;
      }
    }
    
    luffy_game.is_stretching = false;
    luffy_game.stretch_power = 0;
  }
}

// =============================================================================
// GOJO DOMAIN DODGE GAME
// Dodge cursed energy projectiles in Unlimited Void!
// =============================================================================

void drawGojoDodgeGame() {
  // Void domain background
  gfx->fillScreen(GOJO_VOID_BLACK);
  
  // Infinity pattern
  for (int i = 0; i < 30; i++) {
    float angle = (millis() / 1000.0 + i * 0.2);
    int x = LCD_WIDTH / 2 + cos(angle) * (50 + i * 5);
    int y = LCD_HEIGHT / 2 + sin(angle * 2) * (30 + i * 3);
    gfx->drawPixel(x, y, GOJO_INFINITY_BLUE);
  }
  
  // Header
  gfx->setTextColor(GOJO_INFINITY_BLUE);
  gfx->setTextSize(2);
  gfx->setCursor(60, 10);
  gfx->print("UNLIMITED VOID");
  
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(20, 40);
  gfx->printf("Score: %d  Lives: %d", char_game_state.score, char_game_state.lives);
  
  // Draw projectiles
  for (int i = 0; i < gojo_game.projectile_count; i++) {
    int px = gojo_game.projectiles[i][0];
    int py = gojo_game.projectiles[i][1];
    
    gfx->fillCircle(px, py, 8, GOJO_HOLLOW_PURPLE);
    gfx->drawCircle(px, py, 10, GOJO_CURSED_PURPLE);
  }
  
  // Draw player (Gojo silhouette)
  int px = gojo_game.player_x;
  int py = gojo_game.player_y;
  
  // Infinity barrier
  gfx->drawCircle(px, py, 25, GOJO_INFINITY_BLUE);
  gfx->drawCircle(px, py, 23, GOJO_LIGHT_BLUE_GLOW);
  
  // Player body
  gfx->fillCircle(px, py, 15, GOJO_SNOW_WHITE);
  
  // Six eyes
  gfx->fillCircle(px - 5, py - 3, 3, GOJO_SIX_EYES_BLUE);
  gfx->fillCircle(px + 5, py - 3, 3, GOJO_SIX_EYES_BLUE);
  
  // Instructions
  gfx->setTextColor(RGB565(100, 120, 160));
  gfx->setTextSize(1);
  gfx->setCursor(90, 410);
  gfx->print("Swipe to dodge!");
  
  // Back button
  drawSwipeIndicator();
}

void updateGojoDodgeGame() {
  // Spawn projectiles
  if (millis() - gojo_game.last_spawn > 1000 && gojo_game.projectile_count < 10) {
    gojo_game.projectiles[gojo_game.projectile_count][0] = random(50, LCD_WIDTH - 50);
    gojo_game.projectiles[gojo_game.projectile_count][1] = 50;
    gojo_game.projectiles[gojo_game.projectile_count][2] = 3 + char_game_state.level;
    gojo_game.projectile_count++;
    gojo_game.last_spawn = millis();
  }
  
  // Move projectiles
  for (int i = 0; i < gojo_game.projectile_count; i++) {
    gojo_game.projectiles[i][1] += gojo_game.projectiles[i][2];
    
    // Check collision with player
    int dx = gojo_game.projectiles[i][0] - gojo_game.player_x;
    int dy = gojo_game.projectiles[i][1] - gojo_game.player_y;
    int dist = sqrt(dx * dx + dy * dy);
    
    if (dist < 30) {
      // Hit!
      char_game_state.lives--;
      
      // Remove projectile
      for (int j = i; j < gojo_game.projectile_count - 1; j++) {
        gojo_game.projectiles[j][0] = gojo_game.projectiles[j + 1][0];
        gojo_game.projectiles[j][1] = gojo_game.projectiles[j + 1][1];
        gojo_game.projectiles[j][2] = gojo_game.projectiles[j + 1][2];
      }
      gojo_game.projectile_count--;
      i--;
      
      if (char_game_state.lives <= 0) {
        endCharacterGame();
        return;
      }
    }
    
    // Remove if off screen
    if (gojo_game.projectiles[i][1] > LCD_HEIGHT) {
      char_game_state.score += 10;
      
      for (int j = i; j < gojo_game.projectile_count - 1; j++) {
        gojo_game.projectiles[j][0] = gojo_game.projectiles[j + 1][0];
        gojo_game.projectiles[j][1] = gojo_game.projectiles[j + 1][1];
        gojo_game.projectiles[j][2] = gojo_game.projectiles[j + 1][2];
      }
      gojo_game.projectile_count--;
      i--;
    }
  }
  
  drawGojoDodgeGame();
}

void handleGojoDodgeTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP || gesture.event == TOUCH_PRESS) {
    // Move player to tap position
    gojo_game.player_x = constrain(gesture.x, 30, LCD_WIDTH - 30);
    gojo_game.player_y = constrain(gesture.y, 60, LCD_HEIGHT - 60);
  }
  else if (gesture.event >= TOUCH_SWIPE_LEFT && gesture.event <= TOUCH_SWIPE_DOWN) {
    // Quick dodge
    if (gesture.event == TOUCH_SWIPE_LEFT) gojo_game.player_x -= 60;
    if (gesture.event == TOUCH_SWIPE_RIGHT) gojo_game.player_x += 60;
    if (gesture.event == TOUCH_SWIPE_UP) gojo_game.player_y -= 60;
    if (gesture.event == TOUCH_SWIPE_DOWN) gojo_game.player_y += 60;
    
    gojo_game.player_x = constrain(gojo_game.player_x, 30, LCD_WIDTH - 30);
    gojo_game.player_y = constrain(gojo_game.player_y, 60, LCD_HEIGHT - 60);
  }
}

// =============================================================================
// LEVI TITAN SLASH GAME
// Swipe to slash titans!
// =============================================================================

void drawLeviSlashGame() {
  // Wall background
  gfx->fillScreen(LEVI_CHARCOAL_DARK);
  
  // Header
  gfx->setTextColor(LEVI_SURVEY_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(80, 10);
  gfx->print("TITAN SLAYER");
  
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(20, 40);
  gfx->printf("Kills: %d  Score: %d", levi_game.kill_count, char_game_state.score);
  
  // Draw titans
  for (int i = 0; i < levi_game.titan_count; i++) {
    int tx = levi_game.titans[i][0];
    int ty = levi_game.titans[i][1];
    int hp = levi_game.titans[i][2];
    int type = levi_game.titans[i][3];
    
    // Titan body
    uint16_t titanColor = type == 0 ? RGB565(180, 140, 120) : RGB565(200, 100, 100);
    int size = 30 + type * 10;
    
    gfx->fillEllipse(tx, ty, size, size + 10, titanColor);
    
    // Eyes
    gfx->fillCircle(tx - 8, ty - 10, 5, COLOR_WHITE);
    gfx->fillCircle(tx + 8, ty - 10, 5, COLOR_WHITE);
    gfx->fillCircle(tx - 8, ty - 10, 2, COLOR_BLACK);
    gfx->fillCircle(tx + 8, ty - 10, 2, COLOR_BLACK);
    
    // Weak spot (nape)
    gfx->fillRect(tx - 5, ty + size - 5, 10, 10, COLOR_RED);
    
    // HP bar
    gfx->drawRect(tx - 20, ty - size - 10, 40, 6, COLOR_WHITE);
    int hpWidth = (hp * 38) / 100;
    gfx->fillRect(tx - 19, ty - size - 9, hpWidth, 4, COLOR_RED);
  }
  
  // Draw slash effect
  if (levi_game.slashing) {
    for (int i = 0; i < 3; i++) {
      float angle = (levi_game.slash_angle + i * 10) * PI / 180.0;
      int len = 60;
      int x2 = levi_game.slash_x + cos(angle) * len;
      int y2 = levi_game.slash_y + sin(angle) * len;
      
      gfx->drawLine(levi_game.slash_x, levi_game.slash_y, x2, y2, LEVI_SILVER_BLADE);
    }
  }
  
  // Instructions
  gfx->setTextColor(RGB565(120, 125, 135));
  gfx->setTextSize(1);
  gfx->setCursor(80, 400);
  gfx->print("Swipe across nape to kill!");
  
  // Back button
  drawSwipeIndicator();
}

void updateLeviSlashGame() {
  // Spawn titans
  if (levi_game.titan_count < 3 && random(100) < 3) {
    int idx = levi_game.titan_count;
    levi_game.titans[idx][0] = 60 + random(LCD_WIDTH - 120);
    levi_game.titans[idx][1] = 100 + random(200);
    levi_game.titans[idx][2] = 100;  // HP
    levi_game.titans[idx][3] = random(2);  // Type
    levi_game.titan_count++;
  }
  
  // Move titans slowly
  for (int i = 0; i < levi_game.titan_count; i++) {
    levi_game.titans[i][1] += 1;
    
    // If titan reaches bottom, lose life
    if (levi_game.titans[i][1] > LCD_HEIGHT - 50) {
      char_game_state.lives--;
      
      // Remove titan
      for (int j = i; j < levi_game.titan_count - 1; j++) {
        for (int k = 0; k < 4; k++) {
          levi_game.titans[j][k] = levi_game.titans[j + 1][k];
        }
      }
      levi_game.titan_count--;
      i--;
      
      if (char_game_state.lives <= 0) {
        endCharacterGame();
        return;
      }
    }
  }
  
  // Clear slash effect
  if (levi_game.slashing && millis() % 200 < 50) {
    levi_game.slashing = false;
  }
  
  drawLeviSlashGame();
}

void handleLeviSlashTouch(TouchGesture& gesture) {
  if (gesture.event >= TOUCH_SWIPE_LEFT && gesture.event <= TOUCH_SWIPE_DOWN) {
    // Perform slash
    levi_game.slashing = true;
    levi_game.slash_x = gesture.start_x;
    levi_game.slash_y = gesture.start_y;
    levi_game.slash_angle = atan2(gesture.dy, gesture.dx) * 180 / PI;
    
    // Check if slash hits any titan nape
    for (int i = 0; i < levi_game.titan_count; i++) {
      int tx = levi_game.titans[i][0];
      int ty = levi_game.titans[i][1];
      int size = 30 + levi_game.titans[i][3] * 10;
      
      // Nape position
      int napeX = tx;
      int napeY = ty + size - 5;
      
      // Check if slash line crosses nape area
      int dist = abs((gesture.y - gesture.start_y) * napeX - 
                     (gesture.x - gesture.start_x) * napeY +
                     gesture.x * gesture.start_y - 
                     gesture.y * gesture.start_x) /
                 sqrt(gesture.dx * gesture.dx + gesture.dy * gesture.dy + 1);
      
      if (dist < 30) {
        // Hit!
        levi_game.titans[i][2] -= 50;
        
        if (levi_game.titans[i][2] <= 0) {
          // Titan killed!
          levi_game.kill_count++;
          char_game_state.score += 100;
          
          // Remove titan
          for (int j = i; j < levi_game.titan_count - 1; j++) {
            for (int k = 0; k < 4; k++) {
              levi_game.titans[j][k] = levi_game.titans[j + 1][k];
            }
          }
          levi_game.titan_count--;
          i--;
        }
      }
    }
  }
}

// =============================================================================
// BOBOIBOY ELEMENT BATTLE
// Rock-paper-scissors style element combat!
// =============================================================================

/*
 * Element advantages (like Pokemon):
 * Lightning > Wind > Fire > Leaf > Earth > Ice > Lightning
 * Light beats all dark enemies, weak to nothing
 */

void initElementBattle() {
  element_battle.player_element = getCurrentBoboiboyElement();
  element_battle.enemy_element = random(7);
  element_battle.player_hp = 100;
  element_battle.enemy_hp = 100;
  element_battle.player_power = 0;
  element_battle.is_attacking = false;
  element_battle.enemy_attacking = false;
  element_battle.enemy_attack_timer = 0;
  element_battle.battle_phase = 1;  // Start in fight phase
}

void drawElementBattle() {
  // Background based on player element
  gfx->fillScreen(RGB565(15, 15, 20));
  
  // Header
  gfx->setTextColor(BBB_BAND_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 10);
  gfx->print("ELEMENT BATTLE");
  
  // Player side (bottom)
  int playerX = 80;
  int playerY = 350;
  
  // Player element orb
  gfx->fillCircle(playerX, playerY, 40, battleElementColors[element_battle.player_element]);
  gfx->drawCircle(playerX, playerY, 42, COLOR_WHITE);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(playerX - 30, playerY + 50);
  gfx->print(battleElementNames[element_battle.player_element]);
  
  // Player HP bar
  gfx->drawRect(140, playerY - 20, 150, 15, COLOR_WHITE);
  int playerHPWidth = (element_battle.player_hp * 146) / 100;
  gfx->fillRect(142, playerY - 18, playerHPWidth, 11, COLOR_GREEN);
  gfx->setCursor(145, playerY - 16);
  gfx->printf("%d/100", element_battle.player_hp);
  
  // Power meter
  gfx->drawRect(140, playerY + 5, 150, 12, BBB_BAND_ORANGE);
  int powerWidth = (element_battle.player_power * 146) / 100;
  gfx->fillRect(142, playerY + 7, powerWidth, 8, BBB_BAND_GLOW);
  gfx->setCursor(300, playerY + 5);
  gfx->print("PWR");
  
  // Enemy side (top)
  int enemyX = LCD_WIDTH - 80;
  int enemyY = 120;
  
  // Enemy element orb
  gfx->fillCircle(enemyX, enemyY, 40, battleElementColors[element_battle.enemy_element]);
  gfx->drawCircle(enemyX, enemyY, 42, COLOR_RED);
  
  // Enemy name
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(enemyX - 30, enemyY - 60);
  gfx->print("ENEMY");
  gfx->setCursor(enemyX - 30, enemyY + 50);
  gfx->print(battleElementNames[element_battle.enemy_element]);
  
  // Enemy HP bar
  gfx->drawRect(30, enemyY - 20, 150, 15, COLOR_WHITE);
  int enemyHPWidth = (element_battle.enemy_hp * 146) / 100;
  gfx->fillRect(32, enemyY - 18, enemyHPWidth, 11, COLOR_RED);
  gfx->setCursor(35, enemyY - 16);
  gfx->printf("%d/100", element_battle.enemy_hp);
  
  // Battle arena center
  int centerY = 235;
  
  // Attack animation
  if (element_battle.is_attacking) {
    // Player attack animation
    for (int i = 0; i < 5; i++) {
      float t = millis() / 100.0 + i * 0.5;
      int ax = playerX + (enemyX - playerX) * (sin(t) * 0.3 + 0.5);
      int ay = playerY + (enemyY - playerY) * (sin(t) * 0.3 + 0.5);
      gfx->fillCircle(ax, ay, 8, battleElementColors[element_battle.player_element]);
    }
  }
  
  if (element_battle.enemy_attacking) {
    // Enemy attack animation
    for (int i = 0; i < 5; i++) {
      float t = millis() / 100.0 + i * 0.5;
      int ax = enemyX + (playerX - enemyX) * (sin(t) * 0.3 + 0.5);
      int ay = enemyY + (playerY - enemyY) * (sin(t) * 0.3 + 0.5);
      gfx->fillCircle(ax, ay, 8, battleElementColors[element_battle.enemy_element]);
    }
  }
  
  // Element switch buttons
  gfx->setTextColor(RGB565(150, 155, 170));
  gfx->setTextSize(1);
  gfx->setCursor(120, 410);
  gfx->print("TAP: Attack  SWIPE: Switch Element");
  
  // Back button
  drawSwipeIndicator();
  
  // Win/Lose screen
  if (element_battle.battle_phase == 2) {
    gfx->fillRect(50, 180, 270, 80, RGB565(20, 100, 50));
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(3);
    gfx->setCursor(110, 200);
    gfx->print("YOU WIN!");
    char_game_state.score += 200;
  } else if (element_battle.battle_phase == 3) {
    gfx->fillRect(50, 180, 270, 80, RGB565(100, 30, 30));
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(3);
    gfx->setCursor(100, 200);
    gfx->print("YOU LOSE");
  }
}

void updateElementBattle() {
  if (element_battle.battle_phase != 1) return;  // Not in fight phase
  
  // Clear attack animations
  if (element_battle.is_attacking && millis() % 500 < 50) {
    element_battle.is_attacking = false;
  }
  if (element_battle.enemy_attacking && millis() % 500 < 50) {
    element_battle.enemy_attacking = false;
  }
  
  // Enemy AI - attack periodically
  element_battle.enemy_attack_timer++;
  if (element_battle.enemy_attack_timer > 100) {
    enemyAttack();
    element_battle.enemy_attack_timer = 0;
  }
  
  // Charge power slowly
  if (element_battle.player_power < 100) {
    element_battle.player_power += 1;
  }
  
  // Check win/lose conditions
  if (element_battle.enemy_hp <= 0) {
    element_battle.battle_phase = 2;  // Win
  } else if (element_battle.player_hp <= 0) {
    element_battle.battle_phase = 3;  // Lose
  }
  
  drawElementBattle();
}

void handleElementBattleTouch(TouchGesture& gesture) {
  if (element_battle.battle_phase != 1) {
    // Tap to restart
    if (gesture.event == TOUCH_TAP) {
      initElementBattle();
    }
    return;
  }
  
  if (gesture.event == TOUCH_TAP) {
    // Attack!
    if (element_battle.player_power >= 30) {
      elementPlayerAttack();
    }
  }
  else if (gesture.event == TOUCH_SWIPE_LEFT || gesture.event == TOUCH_SWIPE_RIGHT) {
    // Switch element
    if (gesture.event == TOUCH_SWIPE_RIGHT) {
      element_battle.player_element = (element_battle.player_element + 1) % 7;
    } else {
      element_battle.player_element = (element_battle.player_element + 6) % 7;
    }
    switchToElement(element_battle.player_element);
  }
}

void elementPlayerAttack() {
  element_battle.is_attacking = true;
  
  int advantage = getElementAdvantage(element_battle.player_element, element_battle.enemy_element);
  int damage = 15 + advantage * 10 + (element_battle.player_power / 10);
  
  element_battle.enemy_hp -= damage;
  element_battle.player_power = 0;
  
  if (element_battle.enemy_hp < 0) element_battle.enemy_hp = 0;
}

void enemyAttack() {
  element_battle.enemy_attacking = true;
  
  int advantage = getElementAdvantage(element_battle.enemy_element, element_battle.player_element);
  int damage = 10 + advantage * 8;
  
  element_battle.player_hp -= damage;
  
  if (element_battle.player_hp < 0) element_battle.player_hp = 0;
}

int getElementAdvantage(int attacker, int defender) {
  // Circular advantage: 0>1>3>5>2>4>0, 6 (Light) beats all
  if (attacker == 6) return 1;  // Light always has advantage
  if (defender == 6) return -1; // Light always resists
  
  // Simplified rock-paper-scissors
  int diff = (attacker - defender + 7) % 7;
  
  if (diff == 1 || diff == 2) return 1;   // Advantage
  if (diff == 5 || diff == 6) return -1;  // Disadvantage
  return 0;  // Neutral
}
