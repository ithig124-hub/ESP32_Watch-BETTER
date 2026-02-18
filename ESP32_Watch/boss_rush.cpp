/*
 * boss_rush.cpp - Boss Rush Challenge Implementation
 * Complete boss battle system with 20 anime bosses
 */

#include "boss_rush.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "gacha.h"
#include "rpg.h"
#include <SD_MMC.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Boss database
BossData boss_database[TOTAL_BOSSES];
bool bosses_defeated[TOTAL_BOSSES];

// Combat state
int current_boss_index = 0;
int player_hp, player_max_hp;
int player_attack, player_defense;
int player_energy;
int potions_remaining;
int combo_count;
int turn_count;
bool player_defending;
bool battle_active;

// Battle log
String battle_log[5];
int log_index = 0;

// =============================================================================
// BOSS DATABASE INITIALIZATION
// =============================================================================

void initBossDatabase() {
  // Tier 1 Bosses (Level 1-5, HP: 5000-7000)
  boss_database[0] = {"Buggy", "One Piece", BOSS_TIER_1, 5000, 5000, 300, 100, 1, false, 50};
  boss_database[1] = {"Zabuza", "Naruto", BOSS_TIER_1, 6000, 6000, 400, 150, 2, false, 60};
  boss_database[2] = {"Raditz", "Dragon Ball", BOSS_TIER_1, 7000, 7000, 500, 120, 3, false, 70};
  boss_database[3] = {"Hand Demon", "Demon Slayer", BOSS_TIER_1, 5500, 5500, 350, 80, 4, false, 55};
  boss_database[4] = {"Cursed Spirit", "Jujutsu Kaisen", BOSS_TIER_1, 6500, 6500, 380, 130, 5, false, 65};
  
  // Tier 2 Bosses (Level 6-10, HP: 15000-20000)
  boss_database[5] = {"Crocodile", "One Piece", BOSS_TIER_2, 15000, 15000, 800, 300, 6, false, 150};
  boss_database[6] = {"Orochimaru", "Naruto", BOSS_TIER_2, 18000, 18000, 900, 350, 7, false, 180};
  boss_database[7] = {"Frieza", "Dragon Ball", BOSS_TIER_2, 20000, 20000, 1000, 400, 8, false, 200};
  boss_database[8] = {"Rui", "Demon Slayer", BOSS_TIER_2, 16000, 16000, 850, 280, 9, false, 160};
  boss_database[9] = {"Mahito", "Jujutsu Kaisen", BOSS_TIER_2, 19000, 19000, 950, 320, 10, false, 190};
  
  // Tier 3 Bosses (Level 11-15, HP: 40000-50000)
  boss_database[10] = {"Doflamingo", "One Piece", BOSS_TIER_3, 40000, 40000, 1500, 600, 11, false, 300};
  boss_database[11] = {"Pain", "Naruto", BOSS_TIER_3, 45000, 45000, 1800, 700, 12, false, 350};
  boss_database[12] = {"Cell", "Dragon Ball", BOSS_TIER_3, 50000, 50000, 2000, 800, 13, false, 400};
  boss_database[13] = {"Akaza", "Demon Slayer", BOSS_TIER_3, 42000, 42000, 1600, 550, 14, false, 320};
  boss_database[14] = {"Sukuna (4 Fingers)", "Jujutsu Kaisen", BOSS_TIER_3, 48000, 48000, 1900, 750, 15, false, 380};
  
  // Tier 4 Bosses (Level 16-20, HP: 100000-150000) - FINAL BOSSES
  boss_database[15] = {"Kaido", "One Piece", BOSS_TIER_4, 100000, 100000, 3000, 1200, 16, false, 600};
  boss_database[16] = {"Madara", "Naruto", BOSS_TIER_4, 110000, 110000, 3500, 1400, 17, false, 700};
  boss_database[17] = {"Jiren", "Dragon Ball", BOSS_TIER_4, 120000, 120000, 4000, 1600, 18, false, 800};
  boss_database[18] = {"Muzan", "Demon Slayer", BOSS_TIER_4, 105000, 105000, 3200, 1100, 19, false, 650};
  boss_database[19] = {"Sukuna (Full Power)", "Jujutsu Kaisen", BOSS_TIER_4, 150000, 150000, 5000, 2000, 20, false, 1000};
  
  Serial.println("[BossRush] Database initialized with 20 bosses");
}

// =============================================================================
// BOSS RUSH SYSTEM
// =============================================================================

void initBossRush() {
  Serial.println("[BossRush] Initializing boss rush system...");
  initBossDatabase();
  
  if (!loadBossProgress()) {
    for (int i = 0; i < TOTAL_BOSSES; i++) {
      bosses_defeated[i] = false;
    }
    system_state.bosses_defeated = 0;
  }
}

void saveBossProgress() {
  extern bool sdCardInitialized;
  if (!sdCardInitialized) {
    Serial.println("[BossRush] SD Card not available");
    return;
  }
  
  File dataFile = SD_MMC.open("/WATCH/boss_rush/progress.dat", FILE_WRITE);
  if (!dataFile) {
    Serial.println("[BossRush] Cannot save progress");
    return;
  }
  
  dataFile.printf("VERSION=1\n");
  dataFile.printf("TOTAL_DEFEATED=%d\n", system_state.bosses_defeated);
  
  // Save each boss's defeated status and current HP
  for (int i = 0; i < TOTAL_BOSSES; i++) {
    dataFile.printf("BOSS_%d_DEFEATED=%d\n", i, bosses_defeated[i] ? 1 : 0);
    dataFile.printf("BOSS_%d_HP=%d\n", i, boss_database[i].hp);
    dataFile.printf("BOSS_%d_MAX_HP=%d\n", i, boss_database[i].max_hp);
  }
  
  dataFile.close();
  Serial.println("[BossRush] Progress saved to SD card");
}

bool loadBossProgress() {
  extern bool sdCardInitialized;
  if (!sdCardInitialized) {
    return false;
  }
  
  File dataFile = SD_MMC.open("/WATCH/boss_rush/progress.dat", FILE_READ);
  if (!dataFile) {
    Serial.println("[BossRush] No saved progress found");
    return false;
  }
  
  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    line.trim();
    
    int eqPos = line.indexOf('=');
    if (eqPos < 0) continue;
    
    String key = line.substring(0, eqPos);
    int value = line.substring(eqPos + 1).toInt();
    
    if (key == "TOTAL_DEFEATED") {
      system_state.bosses_defeated = value;
    } else if (key.startsWith("BOSS_")) {
      // Parse boss index from key like "BOSS_0_DEFEATED"
      int firstUnderscore = key.indexOf('_');
      int secondUnderscore = key.indexOf('_', firstUnderscore + 1);
      if (firstUnderscore > 0 && secondUnderscore > firstUnderscore) {
        int bossIndex = key.substring(firstUnderscore + 1, secondUnderscore).toInt();
        String field = key.substring(secondUnderscore + 1);
        
        if (bossIndex >= 0 && bossIndex < TOTAL_BOSSES) {
          if (field == "DEFEATED") {
            bosses_defeated[bossIndex] = (value == 1);
          } else if (field == "HP") {
            boss_database[bossIndex].hp = value;
          } else if (field == "MAX_HP") {
            boss_database[bossIndex].max_hp = value;
          }
        }
      }
    }
  }
  
  dataFile.close();
  Serial.printf("[BossRush] Progress loaded: %d bosses defeated\n", system_state.bosses_defeated);
  return true;
}

BossData* getCurrentBoss() {
  return &boss_database[current_boss_index];
}

BossData* getBoss(int index) {
  if (index < 0 || index >= TOTAL_BOSSES) return nullptr;
  return &boss_database[index];
}

int getBossesDefeated() {
  int count = 0;
  for (int i = 0; i < TOTAL_BOSSES; i++) {
    if (bosses_defeated[i]) count++;
  }
  return count;
}

bool isBossDefeated(int boss_index) {
  if (boss_index < 0 || boss_index >= TOTAL_BOSSES) return false;
  return bosses_defeated[boss_index];
}

void markBossDefeated(int boss_index) {
  if (boss_index < 0 || boss_index >= TOTAL_BOSSES) return;
  
  if (!bosses_defeated[boss_index]) {
    bosses_defeated[boss_index] = true;
    system_state.bosses_defeated++;
    boss_database[boss_index].defeated = true;
  }
}

// =============================================================================
// COMBAT SYSTEM
// =============================================================================

void startBossFight(int boss_index) {
  if (boss_index < 0 || boss_index >= TOTAL_BOSSES) return;
  
  current_boss_index = boss_index;
  battle_active = true;
  
  // Initialize player stats based on level
  player_max_hp = 1000 + system_state.player_level * 100;
  player_hp = player_max_hp;
  player_attack = 500 + system_state.player_level * 50;
  player_defense = 300 + system_state.player_level * 30;
  player_energy = 0;
  potions_remaining = MAX_POTIONS;
  combo_count = 0;
  turn_count = 0;
  player_defending = false;
  
  // Reset boss HP to max
  boss_database[boss_index].hp = boss_database[boss_index].max_hp;
  
  // Clear battle log
  log_index = 0;
  for (int i = 0; i < 5; i++) battle_log[i] = "";
  
  addBattleLogEntry("Battle Start!");
  
  system_state.current_screen = SCREEN_BOSS_RUSH;
}

void addBattleLogEntry(const char* message) {
  if (log_index < 5) {
    battle_log[log_index++] = String(message);
  } else {
    for (int i = 0; i < 4; i++) {
      battle_log[i] = battle_log[i + 1];
    }
    battle_log[4] = String(message);
  }
}

void playerAttack() {
  if (!battle_active) return;
  
  BossData* boss = getCurrentBoss();
  
  // Calculate damage with combo multiplier
  int base_damage = player_attack - boss->defense / 2;
  float multiplier = 1.0 + (getComboMultiplier() * 0.1);
  
  // Critical hit chance
  bool critical = random(0, 100) < CRITICAL_CHANCE;
  if (critical) multiplier *= CRITICAL_MULTIPLIER;
  
  int damage = max(1, (int)(base_damage * multiplier));
  boss->hp -= damage;
  
  incrementCombo();
  player_energy = min(100, player_energy + 25);
  
  char log[64];
  sprintf(log, "%s Attack! %d dmg", critical ? "CRITICAL " : "", damage);
  addBattleLogEntry(log);
  
  processCombatRound();
}

void playerDefend() {
  if (!battle_active) return;
  
  player_defending = true;
  player_energy = min(100, player_energy + 25);
  
  addBattleLogEntry("Defending!");
  
  processCombatRound();
}

void playerSpecial() {
  if (!battle_active || player_energy < SPECIAL_ATTACK_COST) return;
  
  BossData* boss = getCurrentBoss();
  player_energy = 0;
  
  // Triple damage, always critical
  int damage = (player_attack * 3) - boss->defense / 4;
  damage = max(100, damage);
  boss->hp -= damage;
  
  incrementCombo();
  
  char log[64];
  sprintf(log, "SPECIAL ATTACK! %d dmg!", damage);
  addBattleLogEntry(log);
  
  processCombatRound();
}

void playerUseItem() {
  if (!battle_active || potions_remaining <= 0) return;
  
  potions_remaining--;
  int heal = player_max_hp * POTION_HEAL_PERCENT / 100;
  player_hp = min(player_max_hp, player_hp + heal);
  
  char log[64];
  sprintf(log, "Used Potion! +%d HP", heal);
  addBattleLogEntry(log);
  
  processCombatRound();
}

void bossAction() {
  if (!battle_active) return;
  
  BossData* boss = getCurrentBoss();
  
  // Boss AI based on HP percentage
  int hp_percent = (boss->hp * 100) / boss_database[current_boss_index].hp;
  
  int damage = boss->attack - player_defense / 2;
  
  // Berserk mode at low HP
  if (hp_percent < 25) {
    damage = (int)(damage * 1.5);
    addBattleLogEntry("Boss is BERSERK!");
  }
  
  // Defending reduces damage
  if (player_defending) {
    damage = damage / 2;
    player_defending = false;
  }
  
  damage = max(1, damage);
  player_hp -= damage;
  
  resetCombo();  // Getting hit resets combo
  
  char log[64];
  sprintf(log, "%s attacks! %d dmg", boss->name.c_str(), damage);
  addBattleLogEntry(log);
}

void processCombatRound() {
  turn_count++;
  
  // Check if battle ended
  if (checkBattleEnd()) return;
  
  // Boss takes action
  bossAction();
  
  // Check again after boss action
  checkBattleEnd();
}

bool checkBattleEnd() {
  BossData* boss = getCurrentBoss();
  
  if (boss->hp <= 0) {
    // Victory!
    battle_active = false;
    markBossDefeated(current_boss_index);
    
    bool no_damage = (player_hp == player_max_hp);
    bool fast_clear = (turn_count < 10);
    int reward = calculateBossReward(*boss, no_damage, fast_clear);
    
    addGems(reward, "Boss Defeated");
    gainExperience(boss->level * 100, "Boss Victory");
    
    drawBossVictory(*boss);
    return true;
  }
  
  if (player_hp <= 0) {
    // Defeat
    battle_active = false;
    drawBossDefeat();
    return true;
  }
  
  return false;
}

void incrementCombo() {
  combo_count++;
}

void resetCombo() {
  combo_count = 0;
}

int getComboMultiplier() {
  if (combo_count >= 5) return 5;  // +50% damage
  if (combo_count >= 3) return 3;  // +30% damage
  if (combo_count >= 2) return 2;  // +20% damage
  return 0;
}

int calculateBossReward(BossData& boss, bool no_damage, bool fast_clear) {
  int base_reward = boss.gem_reward;
  
  if (no_damage) base_reward = (int)(base_reward * 1.5);
  if (fast_clear) base_reward = (int)(base_reward * 1.25);
  
  // First clear bonus
  if (!boss.defeated) base_reward *= 2;
  
  return base_reward;
}

void giveBossReward(int boss_index) {
  BossData* boss = getBoss(boss_index);
  if (boss) {
    addGems(boss->gem_reward, "Boss Reward");
  }
}

// =============================================================================
// BOSS RUSH UI
// =============================================================================

void drawBossRushMenu() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(90, 20);
  gfx->print("BOSS RUSH");
  
  // Progress
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(100, 50);
  gfx->printf("Defeated: %d/%d", getBossesDefeated(), TOTAL_BOSSES);
  
  // Player stats
  gfx->setCursor(20, 80);
  gfx->printf("Level: %d  ATK: %d  DEF: %d", 
              system_state.player_level, 
              500 + system_state.player_level * 50,
              300 + system_state.player_level * 30);
  
  // Tier buttons
  const char* tiers[] = {"Tier 1", "Tier 2", "Tier 3", "Tier 4"};
  uint16_t colors[] = {COLOR_GREEN, COLOR_BLUE, COLOR_PURPLE, COLOR_RED};
  
  for (int i = 0; i < 4; i++) {
    int y = 120 + i * 70;
    
    gfx->fillRoundRect(40, y, 280, 60, 12, colors[i]);
    gfx->drawRoundRect(40, y, 280, 60, 12, COLOR_WHITE);
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(60, y + 10);
    gfx->print(tiers[i]);
    
    // Show completion
    int start = i * 5;
    int defeated = 0;
    for (int j = start; j < start + 5; j++) {
      if (bosses_defeated[j]) defeated++;
    }
    
    gfx->setTextSize(1);
    gfx->setCursor(60, y + 38);
    gfx->printf("Completed: %d/5", defeated);
  }
  
  drawGlassButton(140, 410, 80, 35, "Back", false);
}

void drawBossSelection() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("SELECT BOSS");
  
  // Show 5 bosses from current tier
  int tier_start = 0;  // Would be set based on selected tier
  
  for (int i = 0; i < 5; i++) {
    int boss_idx = tier_start + i;
    BossData* boss = getBoss(boss_idx);
    int y = 70 + i * 65;
    
    uint16_t bg = bosses_defeated[boss_idx] ? RGB565(0, 60, 0) : RGB565(40, 40, 40);
    gfx->fillRoundRect(30, y, 300, 55, 10, bg);
    gfx->drawRoundRect(30, y, 300, 55, 10, getBossTierColor(boss->tier));
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(45, y + 8);
    gfx->print(boss->name);
    
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_GRAY);
    gfx->setCursor(45, y + 32);
    gfx->printf("%s | Lv.%d", boss->series.c_str(), boss->level);
    
    // Reward
    gfx->setTextColor(COLOR_GOLD);
    gfx->setCursor(250, y + 32);
    gfx->printf("%dG", boss->gem_reward);
  }
  
  drawGlassButton(140, 410, 80, 35, "Back", false);
}

void drawBossBattle() {
  gfx->fillScreen(COLOR_BLACK);
  
  BossData* boss = getCurrentBoss();
  
  // Boss area (top)
  gfx->setTextColor(getBossTierColor(boss->tier));
  gfx->setTextSize(2);
  gfx->setCursor(50, 15);
  gfx->print(boss->name);
  
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->setCursor(50, 40);
  gfx->printf("Lv.%d - %s", boss->level, boss->series.c_str());
  
  // Boss HP bar
  drawBossHealthBar(30, 60, 300, 25, *boss);
  
  // Player area (middle)
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(50, 200);
  CharacterProfile* profile = getCharacterProfile(system_state.current_theme);
  gfx->print(profile->name);
  
  // Player HP bar
  drawPlayerHealthBar(30, 230, 300, 20);
  
  // Energy bar
  drawEnergyBar(30, 260, 200, 15);
  
  // Combo indicator
  if (combo_count > 0) {
    gfx->setTextColor(COLOR_GOLD);
    gfx->setTextSize(1);
    gfx->setCursor(250, 260);
    gfx->printf("x%d COMBO!", combo_count);
  }
  
  // Potions
  gfx->setTextColor(COLOR_GREEN);
  gfx->setCursor(250, 230);
  gfx->printf("Potions: %d", potions_remaining);
  
  // Battle log
  drawBattleLog();
  
  // Action buttons
  drawCombatActions();
}

void drawBossHealthBar(int x, int y, int w, int h, BossData& boss) {
  // Use max_hp field for proper health bar calculation
  float progress = (float)boss.hp / boss.max_hp;
  
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(60, 30, 30));
  
  int fillW = (int)(w * constrain(progress, 0.0f, 1.0f));
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, COLOR_RED);
  }
  
  gfx->drawRoundRect(x, y, w, h, h/2, COLOR_WHITE);
  
  // HP text
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 10, y + h/2 - 4);
  gfx->printf("%d / %d", max(0, boss.hp), boss.max_hp);
}

void drawPlayerHealthBar(int x, int y, int w, int h) {
  float progress = (float)player_hp / player_max_hp;
  
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(30, 60, 30));
  
  int fillW = (int)(w * constrain(progress, 0.0f, 1.0f));
  if (fillW > 0) {
    uint16_t color = progress > 0.3 ? COLOR_GREEN : COLOR_RED;
    gfx->fillRoundRect(x, y, fillW, h, h/2, color);
  }
  
  gfx->drawRoundRect(x, y, w, h, h/2, COLOR_WHITE);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 10, y + h/2 - 4);
  gfx->printf("HP: %d/%d", max(0, player_hp), player_max_hp);
}

void drawEnergyBar(int x, int y, int w, int h) {
  float progress = player_energy / 100.0f;
  
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(30, 30, 60));
  
  int fillW = (int)(w * progress);
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, COLOR_BLUE);
  }
  
  gfx->drawRoundRect(x, y, w, h, h/2, COLOR_WHITE);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 5, y + 2);
  gfx->printf("SP: %d%%", player_energy);
}

void drawCombatActions() {
  int y = 350;
  int btnW = 80, btnH = 40;
  
  // Attack
  gfx->fillRoundRect(20, y, btnW, btnH, 8, COLOR_RED);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(35, y + 15);
  gfx->print("ATTACK");
  
  // Defend
  gfx->fillRoundRect(110, y, btnW, btnH, 8, COLOR_BLUE);
  gfx->setCursor(125, y + 15);
  gfx->print("DEFEND");
  
  // Special (grayed out if no energy)
  uint16_t spColor = player_energy >= 100 ? COLOR_PURPLE : COLOR_GRAY;
  gfx->fillRoundRect(200, y, btnW, btnH, 8, spColor);
  gfx->setCursor(210, y + 15);
  gfx->print("SPECIAL");
  
  // Item (grayed out if no potions)
  uint16_t itemColor = potions_remaining > 0 ? COLOR_GREEN : COLOR_GRAY;
  gfx->fillRoundRect(290, y, btnW - 10, btnH, 8, itemColor);
  gfx->setCursor(300, y + 15);
  gfx->print("ITEM");
}

void drawBattleLog() {
  int y = 290;
  gfx->fillRoundRect(20, y, 320, 50, 8, RGB565(20, 20, 30));
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  
  for (int i = 0; i < min(3, log_index); i++) {
    gfx->setCursor(30, y + 8 + i * 14);
    gfx->print(battle_log[log_index - 1 - i]);
  }
}

void drawBossVictory(BossData& boss) {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(COLOR_GOLD);
  gfx->setTextSize(3);
  gfx->setCursor(100, 100);
  gfx->print("VICTORY!");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(80, 180);
  gfx->printf("%s defeated!", boss.name.c_str());
  
  gfx->setTextColor(COLOR_GREEN);
  gfx->setCursor(100, 240);
  gfx->printf("+%d Gems", boss.gem_reward);
  
  gfx->setTextSize(1);
  gfx->setCursor(100, 280);
  gfx->printf("+%d XP", boss.level * 100);
  
  drawGlassButton(100, 350, 160, 50, "Continue", false);
}

void drawBossDefeat() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(COLOR_RED);
  gfx->setTextSize(3);
  gfx->setCursor(100, 150);
  gfx->print("DEFEAT");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(80, 250);
  gfx->print("Train harder and try again!");
  
  drawGlassButton(100, 350, 160, 50, "Continue", false);
}

void handleBossRushMenuTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int y = gesture.y;
  
  // Tier buttons
  for (int i = 0; i < 4; i++) {
    int by = 120 + i * 70;
    if (y >= by && y < by + 60) {
      // Start first available boss in tier
      int start = i * 5;
      for (int j = start; j < start + 5; j++) {
        if (!bosses_defeated[j]) {
          startBossFight(j);
          return;
        }
      }
      // All bosses in tier defeated - still allow replay
      startBossFight(start);
      return;
    }
  }
  
  // Back button
  if (y >= 410) {
    system_state.current_screen = SCREEN_GAMES;
  }
}

void handleBossSelectionTouch(TouchGesture& gesture) {
  // Select specific boss from list
  if (gesture.event == TOUCH_TAP && gesture.y >= 410) {
    system_state.current_screen = SCREEN_BOSS_RUSH;
  }
}

void handleBossBattleTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  if (!battle_active) {
    // Victory/Defeat screen - continue
    system_state.current_screen = SCREEN_BOSS_RUSH;
    return;
  }
  
  int x = gesture.x, y = gesture.y;
  
  // Action buttons at y = 350
  if (y >= 350 && y < 390) {
    if (x < 100) playerAttack();
    else if (x < 190) playerDefend();
    else if (x < 280) playerSpecial();
    else playerUseItem();
  }
}

const char* getBossTierName(BossTier tier) {
  const char* names[] = {"Tier 1", "Tier 2", "Tier 3", "Tier 4"};
  return names[(int)tier - 1];
}

uint16_t getBossTierColor(BossTier tier) {
  uint16_t colors[] = {COLOR_GREEN, COLOR_BLUE, COLOR_PURPLE, COLOR_RED};
  return colors[(int)tier - 1];
}
