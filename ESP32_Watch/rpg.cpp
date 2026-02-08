/**
 * RPG System Implementation
 */

#include "rpg.h"
#include "ui_manager.h"
#include "themes.h"

RPGCharacter rpgCharacter;

// Title progression tables
static const char* luffyTitles[] = {
  "Rookie Pirate",
  "East Blue Pirate",
  "Grand Line Rookie",
  "Supernova",
  "Yonko Challenger",
  "Sun God Nika"
};

static const char* jinwooTitles[] = {
  "E-Rank Hunter",
  "D-Rank Hunter",
  "C-Rank Hunter",
  "B-Rank Hunter",
  "A-Rank Hunter",
  "Shadow Monarch"
};

static const char* yugoTitles[] = {
  "Young Eliatrope",
  "Portal Apprentice",
  "Wakfu Wielder",
  "Dimension Walker",
  "Eliatrope King",
  "Master of Portals"
};

// XP table (exponential growth)
static const long xpTable[] = {
  0, 100, 300, 600, 1000, 1500, 2200, 3000, 4000, 5200,
  6600, 8200, 10000, 12000, 14500, 17500, 21000, 25000, 30000, 36000,
  43000, 51000, 60000, 70000, 82000, 95000, 110000, 127000, 146000, 168000,
  // ... up to level 100
};

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initRPG() {
  rpgCharacter.level = userData.rpgLevel > 0 ? userData.rpgLevel : 1;
  rpgCharacter.xp = userData.rpgXP;
  rpgCharacter.xpNext = getXPForLevel(rpgCharacter.level + 1);
  
  // Set character based on theme
  switch(watch.theme) {
    case THEME_LUFFY:
      rpgCharacter.name = "Luffy";
      rpgCharacter.strength = 20 + rpgCharacter.level * 2;
      rpgCharacter.speed = 15 + rpgCharacter.level;
      rpgCharacter.magic = 5;
      rpgCharacter.endurance = 25 + rpgCharacter.level * 2;
      rpgCharacter.hakiLevel = rpgCharacter.level / 10;
      break;
    case THEME_JINWOO:
      rpgCharacter.name = "Sung Jin-Woo";
      rpgCharacter.strength = 15 + rpgCharacter.level * 2;
      rpgCharacter.speed = 20 + rpgCharacter.level * 2;
      rpgCharacter.magic = 10 + rpgCharacter.level;
      rpgCharacter.endurance = 15 + rpgCharacter.level;
      rpgCharacter.shadowArmy = rpgCharacter.level / 5;
      break;
    case THEME_YUGO:
      rpgCharacter.name = "Yugo";
      rpgCharacter.strength = 10 + rpgCharacter.level;
      rpgCharacter.speed = 25 + rpgCharacter.level * 2;
      rpgCharacter.magic = 25 + rpgCharacter.level * 3;
      rpgCharacter.endurance = 10 + rpgCharacter.level;
      rpgCharacter.portalMastery = rpgCharacter.level / 8;
      break;
  }
  
  rpgCharacter.title = getCurrentTitle();
  Serial.println("[OK] RPG System initialized");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  XP & LEVELING
// ═══════════════════════════════════════════════════════════════════════════════
long getXPForLevel(int level) {
  if (level <= 0) return 0;
  if (level > 30) return xpTable[30] + (level - 30) * 50000;
  return xpTable[level];
}

void gainXP(int amount) {
  rpgCharacter.xp += amount;
  userData.rpgXP = rpgCharacter.xp;
  
  while (canLevelUp()) {
    levelUp();
  }
}

bool canLevelUp() {
  return rpgCharacter.xp >= rpgCharacter.xpNext && rpgCharacter.level < 100;
}

void levelUp() {
  rpgCharacter.level++;
  userData.rpgLevel = rpgCharacter.level;
  rpgCharacter.xpNext = getXPForLevel(rpgCharacter.level + 1);
  
  // Update stats
  updateStats();
  rpgCharacter.title = getCurrentTitle();
  
  Serial.printf("[RPG] Level Up! Now Lv.%d\n", rpgCharacter.level);
}

void updateStats() {
  int lvl = rpgCharacter.level;
  switch(watch.theme) {
    case THEME_LUFFY:
      rpgCharacter.strength = 20 + lvl * 2;
      rpgCharacter.speed = 15 + lvl;
      rpgCharacter.endurance = 25 + lvl * 2;
      rpgCharacter.hakiLevel = lvl / 10;
      break;
    case THEME_JINWOO:
      rpgCharacter.strength = 15 + lvl * 2;
      rpgCharacter.speed = 20 + lvl * 2;
      rpgCharacter.magic = 10 + lvl;
      rpgCharacter.shadowArmy = lvl / 5;
      break;
    case THEME_YUGO:
      rpgCharacter.speed = 25 + lvl * 2;
      rpgCharacter.magic = 25 + lvl * 3;
      rpgCharacter.portalMastery = lvl / 8;
      break;
  }
}

const char* getCurrentTitle() {
  int tier = rpgCharacter.level / 17; // 0-5 based on level 1-100
  if (tier > 5) tier = 5;
  
  switch(watch.theme) {
    case THEME_LUFFY:  return luffyTitles[tier];
    case THEME_JINWOO: return jinwooTitles[tier];
    case THEME_YUGO:   return yugoTitles[tier];
    default:           return "Unknown";
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  RPG SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createRPGScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "RPG Status");
  
  // Character name and title
  lv_obj_t* nameLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(nameLbl, lv_color_hex(colors.primary), 0);
  lv_label_set_text(nameLbl, rpgCharacter.name);
  lv_obj_align(nameLbl, LV_ALIGN_TOP_MID, 0, 60);
  
  lv_obj_t* titleLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(titleLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text(titleLbl, rpgCharacter.title);
  lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 90);
  
  // Level circle
  lv_obj_t* levelCircle = lv_obj_create(scr);
  lv_obj_set_size(levelCircle, 80, 80);
  lv_obj_set_style_radius(levelCircle, 40, 0);
  lv_obj_set_style_bg_color(levelCircle, lv_color_hex(colors.primary), 0);
  lv_obj_align(levelCircle, LV_ALIGN_TOP_MID, 0, 120);
  
  lv_obj_t* levelNum = lv_label_create(levelCircle);
  lv_obj_set_style_text_font(levelNum, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(levelNum, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text_fmt(levelNum, "%d", rpgCharacter.level);
  lv_obj_center(levelNum);
  
  // XP Progress
  lv_obj_t* xpLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(xpLabel, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(xpLabel, lv_color_hex(colors.text), 0);
  lv_label_set_text_fmt(xpLabel, "XP: %ld / %ld", rpgCharacter.xp, rpgCharacter.xpNext);
  lv_obj_align(xpLabel, LV_ALIGN_TOP_MID, 0, 210);
  
  lv_obj_t* xpBar = lv_bar_create(scr);
  lv_obj_set_size(xpBar, LCD_WIDTH - 60, 20);
  lv_obj_align(xpBar, LV_ALIGN_TOP_MID, 0, 230);
  lv_bar_set_range(xpBar, 0, rpgCharacter.xpNext);
  lv_bar_set_value(xpBar, rpgCharacter.xp, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(xpBar, lv_color_hex(colors.primary), LV_PART_INDICATOR);
  
  // Stats
  int yPos = 270;
  const char* statNames[] = {"STR", "SPD", "MAG", "END"};
  int statValues[] = {rpgCharacter.strength, rpgCharacter.speed, rpgCharacter.magic, rpgCharacter.endurance};
  
  for (int i = 0; i < 4; i++) {
    int x = 30 + (i % 2) * 170;
    int y = yPos + (i / 2) * 45;
    
    lv_obj_t* statLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(statLbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(statLbl, lv_color_hex(colors.secondary), 0);
    lv_label_set_text(statLbl, statNames[i]);
    lv_obj_set_pos(statLbl, x, y);
    
    lv_obj_t* valLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(valLbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(valLbl, lv_color_hex(colors.text), 0);
    lv_label_set_text_fmt(valLbl, "%d", statValues[i]);
    lv_obj_set_pos(valLbl, x + 50, y - 2);
  }
  
  // Special stat based on character
  lv_obj_t* specialLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(specialLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(specialLbl, lv_color_hex(colors.accent), 0);
  
  switch(watch.theme) {
    case THEME_LUFFY:
      lv_label_set_text_fmt(specialLbl, "Haki Level: %d", rpgCharacter.hakiLevel);
      break;
    case THEME_JINWOO:
      lv_label_set_text_fmt(specialLbl, "Shadow Army: %d", rpgCharacter.shadowArmy);
      break;
    case THEME_YUGO:
      lv_label_set_text_fmt(specialLbl, "Portal Mastery: %d", rpgCharacter.portalMastery);
      break;
  }
  lv_obj_align(specialLbl, LV_ALIGN_BOTTOM_MID, 0, -20);
  
  return scr;
}
