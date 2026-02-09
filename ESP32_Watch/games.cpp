/**
 * Anime-Themed Games Implementation
 * Gacha Simulator, Training Mini-games, Boss Rush
 */

#include "games.h"
#include "ui_manager.h"
#include "themes.h"
#include "rpg.h"

// Gacha pool of characters
static GachaCard gachaPool[] = {
  // Common (60%)
  {"Konohamaru", "Naruto", RARITY_COMMON, 50, false},
  {"Krillin", "Dragon Ball", RARITY_COMMON, 45, false},
  {"Zenitsu", "Demon Slayer", RARITY_COMMON, 55, false},
  {"Yuji Itadori", "Jujutsu Kaisen", RARITY_COMMON, 60, false},
  {"Connie", "Attack on Titan", RARITY_COMMON, 40, false},
  {"Mumen Rider", "One Punch Man", RARITY_COMMON, 35, false},
  {"Mineta", "My Hero Academia", RARITY_COMMON, 30, false},
  
  // Rare (25%)
  {"Kakashi", "Naruto", RARITY_RARE, 150, false},
  {"Vegeta", "Dragon Ball", RARITY_RARE, 160, false},
  {"Inosuke", "Demon Slayer", RARITY_RARE, 140, false},
  {"Megumi", "Jujutsu Kaisen", RARITY_RARE, 145, false},
  {"Mikasa", "Attack on Titan", RARITY_RARE, 155, false},
  {"Genos", "One Punch Man", RARITY_RARE, 165, false},
  {"Bakugo", "My Hero Academia", RARITY_RARE, 150, false},
  
  // Epic (12%)
  {"Minato", "Naruto", RARITY_EPIC, 300, false},
  {"Gohan", "Dragon Ball", RARITY_EPIC, 320, false},
  {"Rengoku", "Demon Slayer", RARITY_EPIC, 350, false},
  {"Todo", "Jujutsu Kaisen", RARITY_EPIC, 280, false},
  {"Erwin", "Attack on Titan", RARITY_EPIC, 290, false},
  {"Tatsumaki", "One Punch Man", RARITY_EPIC, 400, false},
  {"All Might", "My Hero Academia", RARITY_EPIC, 380, false},
  
  // Legendary (2.9%)
  {"Naruto (Baryon)", "Naruto", RARITY_LEGENDARY, 800, false},
  {"Goku (MUI)", "Dragon Ball", RARITY_LEGENDARY, 850, false},
  {"Tanjiro (Mark)", "Demon Slayer", RARITY_LEGENDARY, 750, false},
  {"Gojo", "Jujutsu Kaisen", RARITY_LEGENDARY, 900, false},
  {"Levi", "Attack on Titan", RARITY_LEGENDARY, 820, false},
  {"Saitama", "One Punch Man", RARITY_LEGENDARY, 999, false},
  {"Deku (100%)", "My Hero Academia", RARITY_LEGENDARY, 780, false},
  
  // Mythic (0.1%)
  {"Kurama Mode Naruto", "Naruto", RARITY_MYTHIC, 2000, false},
  {"Whis", "Dragon Ball", RARITY_MYTHIC, 2500, false},
  {"Yoriichi", "Demon Slayer", RARITY_MYTHIC, 3000, false},
  {"Sukuna", "Jujutsu Kaisen", RARITY_MYTHIC, 2800, false},
  {"Founding Titan", "Attack on Titan", RARITY_MYTHIC, 2200, false},
  {"Blast", "One Punch Man", RARITY_MYTHIC, 2700, false},
  {"Prime All Might", "My Hero Academia", RARITY_MYTHIC, 2600, false}
};

#define GACHA_POOL_SIZE 35

static int lastPulledIndex = -1;
static GachaRarity lastPulledRarity = RARITY_COMMON;
static ThemeColors currentColors;

void initGames() {
  Serial.println("[OK] Games System initialized");
}

GachaRarity rollRarity() {
  int roll = random(1000);  // 0-999
  
  if (roll < 1) return RARITY_MYTHIC;        // 0.1%
  if (roll < 30) return RARITY_LEGENDARY;    // 2.9%
  if (roll < 150) return RARITY_EPIC;        // 12%
  if (roll < 400) return RARITY_RARE;        // 25%
  return RARITY_COMMON;                       // 60%
}

const char* getRarityName(GachaRarity rarity) {
  switch(rarity) {
    case RARITY_COMMON:    return "Common";
    case RARITY_RARE:      return "Rare";
    case RARITY_EPIC:      return "Epic";
    case RARITY_LEGENDARY: return "Legendary";
    case RARITY_MYTHIC:    return "MYTHIC!";
    default:               return "Unknown";
  }
}

uint32_t getRarityColor(GachaRarity rarity) {
  switch(rarity) {
    case RARITY_COMMON:    return 0x9E9E9E;  // Grey
    case RARITY_RARE:      return 0x2196F3;  // Blue
    case RARITY_EPIC:      return 0x9C27B0;  // Purple
    case RARITY_LEGENDARY: return 0xFFD700;  // Gold
    case RARITY_MYTHIC:    return 0xFF1744;  // Red/Pink
    default:               return 0xFFFFFF;
  }
}

void performGachaPull() {
  GachaRarity rarity = rollRarity();
  lastPulledRarity = rarity;
  
  // Find a character with matching rarity
  int startIndex = random(GACHA_POOL_SIZE);
  for (int i = 0; i < GACHA_POOL_SIZE; i++) {
    int idx = (startIndex + i) % GACHA_POOL_SIZE;
    if (gachaPool[idx].rarity == rarity) {
      lastPulledIndex = idx;
      gachaPool[idx].owned = true;
      userData.gachaCards++;
      userData.gachaPulls++;
      
      // Award XP based on rarity
      int xpGain = 10 + (int)rarity * 20;
      gainXP(xpGain);
      
      saveUserData();
      return;
    }
  }
  
  // Fallback
  lastPulledIndex = 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  GACHA SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static lv_obj_t* gachaResultPanel = nullptr;
static lv_obj_t* gachaNameLabel = nullptr;
static lv_obj_t* gachaRarityLabel = nullptr;
static lv_obj_t* gachaSeriesLabel = nullptr;

lv_obj_t* createGachaScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Gacha Summon");

  // Stats
  lv_obj_t* statsLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(statsLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(statsLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(statsLbl, "Cards: %d | Pulls: %d", userData.gachaCards, userData.gachaPulls);
  lv_obj_align(statsLbl, LV_ALIGN_TOP_MID, 0, 60);

  // Result panel (card display)
  gachaResultPanel = lv_obj_create(scr);
  lv_obj_set_size(gachaResultPanel, LCD_WIDTH - 60, 180);
  lv_obj_align(gachaResultPanel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(gachaResultPanel, lv_color_hex(0x1A1A2E), 0);
  lv_obj_set_style_radius(gachaResultPanel, 15, 0);
  lv_obj_set_style_border_color(gachaResultPanel, lv_color_hex(currentColors.accent), 0);
  lv_obj_set_style_border_width(gachaResultPanel, 2, 0);
  
  // Pull instruction
  lv_obj_t* instrLbl = lv_label_create(gachaResultPanel);
  lv_obj_set_style_text_font(instrLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(instrLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(instrLbl, "Tap PULL to summon!");
  lv_obj_center(instrLbl);

  // Gacha name (hidden initially)
  gachaNameLabel = lv_label_create(gachaResultPanel);
  lv_obj_set_style_text_font(gachaNameLabel, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(gachaNameLabel, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(gachaNameLabel, "");
  lv_obj_align(gachaNameLabel, LV_ALIGN_TOP_MID, 0, 40);
  
  gachaRarityLabel = lv_label_create(gachaResultPanel);
  lv_obj_set_style_text_font(gachaRarityLabel, &lv_font_montserrat_16, 0);
  lv_label_set_text(gachaRarityLabel, "");
  lv_obj_align(gachaRarityLabel, LV_ALIGN_CENTER, 0, 0);
  
  gachaSeriesLabel = lv_label_create(gachaResultPanel);
  lv_obj_set_style_text_font(gachaSeriesLabel, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(gachaSeriesLabel, lv_color_hex(0xAAAAAA), 0);
  lv_label_set_text(gachaSeriesLabel, "");
  lv_obj_align(gachaSeriesLabel, LV_ALIGN_BOTTOM_MID, 0, -20);

  // Pull button
  lv_obj_t* pullBtn = lv_btn_create(scr);
  lv_obj_set_size(pullBtn, 150, 60);
  lv_obj_align(pullBtn, LV_ALIGN_BOTTOM_MID, 0, -40);
  lv_obj_set_style_bg_color(pullBtn, lv_color_hex(0xFF5722), 0);
  lv_obj_set_style_radius(pullBtn, 30, 0);
  
  lv_obj_add_event_cb(pullBtn, [](lv_event_t* e) {
    performGachaPull();
    
    // Update display
    if (lastPulledIndex >= 0) {
      GachaCard* card = &gachaPool[lastPulledIndex];
      
      lv_label_set_text(gachaNameLabel, card->name);
      lv_label_set_text_fmt(gachaRarityLabel, "[%s]", getRarityName(card->rarity));
      lv_obj_set_style_text_color(gachaRarityLabel, lv_color_hex(getRarityColor(card->rarity)), 0);
      lv_label_set_text(gachaSeriesLabel, card->series);
      
      // Update border to match rarity
      lv_obj_set_style_border_color(gachaResultPanel, lv_color_hex(getRarityColor(card->rarity)), 0);
    }
  }, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* pullLbl = lv_label_create(pullBtn);
  lv_obj_set_style_text_font(pullLbl, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(pullLbl, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(pullLbl, "PULL!");
  lv_obj_center(pullLbl);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  TRAINING SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createTrainingScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Training");

  // Training description
  lv_obj_t* descLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(descLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(descLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(descLbl, "Tap to train and level up!");
  lv_obj_align(descLbl, LV_ALIGN_TOP_MID, 0, 70);

  // Training types
  const char* trainNames[] = {"Push-ups", "Meditation", "Sparring"};
  const uint32_t trainColors[] = {0xE91E63, 0x9C27B0, 0xF44336};
  const int xpRewards[] = {50, 30, 80};
  
  for (int i = 0; i < 3; i++) {
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, LCD_WIDTH - 60, 80);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 110 + i * 100);
    lv_obj_set_style_bg_color(btn, lv_color_hex(trainColors[i]), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)xpRewards[i]);
    
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      int xp = (int)(intptr_t)lv_obj_get_user_data(btn);
      gainXP(xp);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* nameLbl = lv_label_create(btn);
    lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(nameLbl, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(nameLbl, trainNames[i]);
    lv_obj_align(nameLbl, LV_ALIGN_LEFT_MID, 20, 0);
    
    lv_obj_t* xpLbl = lv_label_create(btn);
    lv_obj_set_style_text_font(xpLbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(xpLbl, lv_color_hex(0xFFE082), 0);
    lv_label_set_text_fmt(xpLbl, "+%d XP", xpRewards[i]);
    lv_obj_align(xpLbl, LV_ALIGN_RIGHT_MID, -20, 0);
  }

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  BOSS RUSH SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static int currentBossHP = 100;
static int maxBossHP = 100;
static int bossLevel = 1;
static lv_obj_t* bossHPBar = nullptr;
static lv_obj_t* bossNameLbl = nullptr;

static const char* bossNames[] = {
  "Orochimaru", "Frieza", "Muzan", "Sukuna", 
  "Beast Titan", "Boros", "All For One"
};

lv_obj_t* createBossRushScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Boss Rush");

  // Boss defeated counter
  lv_obj_t* defeatLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(defeatLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(defeatLbl, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(defeatLbl, "Bosses Defeated: %d", userData.bossesDefeated);
  lv_obj_align(defeatLbl, LV_ALIGN_TOP_MID, 0, 60);

  // Boss name
  bossNameLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(bossNameLbl, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(bossNameLbl, lv_color_hex(0xF44336), 0);
  int bossIdx = userData.bossesDefeated % 7;
  lv_label_set_text(bossNameLbl, bossNames[bossIdx]);
  lv_obj_align(bossNameLbl, LV_ALIGN_TOP_MID, 0, 100);
  
  // Boss level
  lv_obj_t* lvlLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(lvlLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lvlLbl, lv_color_hex(currentColors.text), 0);
  bossLevel = 1 + userData.bossesDefeated;
  maxBossHP = 100 + userData.bossesDefeated * 50;
  currentBossHP = maxBossHP;
  lv_label_set_text_fmt(lvlLbl, "Lv. %d", bossLevel);
  lv_obj_align(lvlLbl, LV_ALIGN_TOP_MID, 0, 130);

  // HP Bar
  bossHPBar = lv_bar_create(scr);
  lv_obj_set_size(bossHPBar, LCD_WIDTH - 60, 30);
  lv_obj_align(bossHPBar, LV_ALIGN_TOP_MID, 0, 170);
  lv_bar_set_range(bossHPBar, 0, maxBossHP);
  lv_bar_set_value(bossHPBar, currentBossHP, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(bossHPBar, lv_color_hex(0xF44336), LV_PART_INDICATOR);
  lv_obj_set_style_radius(bossHPBar, 15, 0);
  lv_obj_set_style_radius(bossHPBar, 15, LV_PART_INDICATOR);
  
  // HP Text
  lv_obj_t* hpText = lv_label_create(scr);
  lv_obj_set_style_text_font(hpText, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(hpText, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(hpText, "%d / %d HP", currentBossHP, maxBossHP);
  lv_obj_align(hpText, LV_ALIGN_TOP_MID, 0, 210);

  // Attack button
  lv_obj_t* attackBtn = lv_btn_create(scr);
  lv_obj_set_size(attackBtn, 200, 80);
  lv_obj_align(attackBtn, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_color(attackBtn, lv_color_hex(currentColors.primary), 0);
  lv_obj_set_style_radius(attackBtn, 15, 0);
  
  lv_obj_add_event_cb(attackBtn, [](lv_event_t* e) {
    // Calculate damage based on character stats
    int damage = rpgCharacter.strength + random(10);
    currentBossHP -= damage;
    
    if (currentBossHP <= 0) {
      // Boss defeated!
      userData.bossesDefeated++;
      int xpGain = 100 + bossLevel * 20;
      gainXP(xpGain);
      
      // Reset for next boss
      currentBossHP = maxBossHP;
      saveUserData();
    }
    
    // Update HP bar
    if (bossHPBar) {
      lv_bar_set_value(bossHPBar, currentBossHP > 0 ? currentBossHP : 0, LV_ANIM_OFF);
    }
  }, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* atkLbl = lv_label_create(attackBtn);
  lv_obj_set_style_text_font(atkLbl, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(atkLbl, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(atkLbl, "ATTACK!");
  lv_obj_center(atkLbl);

  // Your stats
  lv_obj_t* yourStats = lv_label_create(scr);
  lv_obj_set_style_text_font(yourStats, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(yourStats, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(yourStats, "Your STR: %d | DMG: %d-%d", 
    rpgCharacter.strength, rpgCharacter.strength, rpgCharacter.strength + 10);
  lv_obj_align(yourStats, LV_ALIGN_BOTTOM_MID, 0, -30);

  return scr;
}
