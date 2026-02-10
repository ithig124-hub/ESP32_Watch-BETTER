/**
 * RPG System Implementation
 * 50 Level Progression with all character themes
 */

#include "rpg.h"
#include "ui_manager.h"
#include "themes.h"

// rpgCharacter is defined in user_data.cpp

// ═══════════════════════════════════════════════════════════════════════════════
//  TITLE PROGRESSION TABLES (5 tiers for 50 levels)
// ═══════════════════════════════════════════════════════════════════════════════

// Main characters
static const char* luffyTitles[] = {
  "Rookie Pirate",
  "Grand Line Pirate",
  "Supernova",
  "Yonko Challenger",
  "Sun God Nika"
};

static const char* jinwooTitles[] = {
  "E-Rank Hunter",
  "C-Rank Hunter",
  "A-Rank Hunter",
  "National Hunter",
  "Shadow Monarch"
};

static const char* yugoTitles[] = {
  "Young Eliatrope",
  "Portal Apprentice",
  "Wakfu Wielder",
  "Eliatrope King",
  "Master of Portals"
};

// Random characters (5 tiers each)
static const char* narutoTitles[] = {
  "Academy Student",
  "Chunin",
  "Jonin",
  "Sage",
  "Seventh Hokage"
};

static const char* gokuTitles[] = {
  "Martial Artist",
  "Super Saiyan",
  "Super Saiyan God",
  "Ultra Instinct Sign",
  "Mastered Ultra Instinct"
};

static const char* tanjiroTitles[] = {
  "Demon Slayer Trainee",
  "Water Breather",
  "Hinokami User",
  "Demon Slayer Mark",
  "Sun Hashira"
};

static const char* gojoTitles[] = {
  "Jujutsu Student",
  "Grade 1 Sorcerer",
  "Special Grade",
  "Six Eyes Master",
  "The Strongest"
};

static const char* leviTitles[] = {
  "Survey Corps Recruit",
  "Squad Leader",
  "Captain",
  "Titan Slayer",
  "Humanity's Strongest"
};

static const char* saitamaTitles[] = {
  "C-Class Hero",
  "B-Class Hero",
  "A-Class Hero",
  "S-Class Candidate",
  "One Punch Man"
};

static const char* dekuTitles[] = {
  "Quirkless Student",
  "U.A. Student",
  "Pro Hero Intern",
  "Rising Hero",
  "Symbol of Peace"
};

// XP table for 50 levels (exponential growth)
static const long xpTable[] = {
  0, 100, 250, 450, 700,           // 1-5
  1000, 1400, 1900, 2500, 3200,    // 6-10
  4000, 5000, 6200, 7600, 9200,    // 11-15
  11000, 13000, 15500, 18500, 22000, // 16-20
  26000, 31000, 37000, 44000, 52000, // 21-25
  61000, 72000, 85000, 100000, 118000, // 26-30
  140000, 165000, 195000, 230000, 270000, // 31-35
  320000, 380000, 450000, 530000, 620000, // 36-40
  730000, 860000, 1000000, 1170000, 1370000, // 41-45
  1600000, 1870000, 2180000, 2550000, 3000000  // 46-50
};

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

void initRPG() {
  rpgCharacter.level = userData.rpgLevel > 0 ? userData.rpgLevel : 1;
  if (rpgCharacter.level > 50) rpgCharacter.level = 50;  // Cap at 50
  
  rpgCharacter.xp = userData.rpgXP;
  rpgCharacter.xpNext = getXPForLevel(rpgCharacter.level + 1);

  updateStats();
  rpgCharacter.title = getCurrentTitle();
  Serial.printf("[OK] RPG System initialized - Lv.%d\n", rpgCharacter.level);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  XP & LEVELING (50 LEVELS MAX)
// ═══════════════════════════════════════════════════════════════════════════════

long getXPForLevel(int level) {
  if (level <= 0) return 0;
  if (level > 50) return xpTable[50];
  return xpTable[level];
}

void gainXP(int amount) {
  rpgCharacter.xp += amount;
  userData.rpgXP = rpgCharacter.xp;

  while (canLevelUp()) {
    levelUp();
  }
  saveUserData();
}

bool canLevelUp() {
  return rpgCharacter.xp >= rpgCharacter.xpNext && rpgCharacter.level < 50;
}

void levelUp() {
  rpgCharacter.level++;
  userData.rpgLevel = rpgCharacter.level;
  rpgCharacter.xpNext = getXPForLevel(rpgCharacter.level + 1);

  updateStats();
  rpgCharacter.title = getCurrentTitle();

  Serial.printf("[RPG] Level Up! Now Lv.%d - %s\n", rpgCharacter.level, rpgCharacter.title);
}

void updateStats() {
  int lvl = rpgCharacter.level;
  
  // Reset all special stats
  rpgCharacter.shadowArmy = 0;
  rpgCharacter.hakiLevel = 0;
  rpgCharacter.portalMastery = 0;
  rpgCharacter.chakraReserves = 0;
  rpgCharacter.powerLevel = 0;
  rpgCharacter.breathingForm = 0;
  rpgCharacter.cursedEnergy = 0;
  rpgCharacter.titanKills = 0;
  rpgCharacter.heroRank = 0;
  rpgCharacter.ofaPercent = 0;
  
  if (watch.theme == THEME_RANDOM) {
    // Random character stats
    switch(watch.dailyCharacter) {
      case CHAR_NARUTO:
        rpgCharacter.name = "Naruto Uzumaki";
        rpgCharacter.strength = 18 + lvl * 2;
        rpgCharacter.speed = 20 + lvl * 2;
        rpgCharacter.magic = 25 + lvl * 3;  // Chakra
        rpgCharacter.endurance = 22 + lvl * 2;
        rpgCharacter.chakraReserves = 100 + lvl * 10;
        break;
        
      case CHAR_GOKU:
        rpgCharacter.name = "Son Goku";
        rpgCharacter.strength = 25 + lvl * 3;
        rpgCharacter.speed = 22 + lvl * 2;
        rpgCharacter.magic = 15 + lvl;  // Ki
        rpgCharacter.endurance = 20 + lvl * 2;
        rpgCharacter.powerLevel = 1000 + lvl * 500;
        break;
        
      case CHAR_TANJIRO:
        rpgCharacter.name = "Tanjiro Kamado";
        rpgCharacter.strength = 20 + lvl * 2;
        rpgCharacter.speed = 22 + lvl * 2;
        rpgCharacter.magic = 18 + lvl * 2;  // Breathing
        rpgCharacter.endurance = 25 + lvl * 2;
        rpgCharacter.breathingForm = 1 + lvl / 10;
        break;
        
      case CHAR_GOJO:
        rpgCharacter.name = "Satoru Gojo";
        rpgCharacter.strength = 15 + lvl;
        rpgCharacter.speed = 25 + lvl * 2;
        rpgCharacter.magic = 30 + lvl * 4;  // Cursed Energy
        rpgCharacter.endurance = 18 + lvl;
        rpgCharacter.cursedEnergy = 100 + lvl * 20;
        break;
        
      case CHAR_LEVI:
        rpgCharacter.name = "Levi Ackerman";
        rpgCharacter.strength = 22 + lvl * 2;
        rpgCharacter.speed = 28 + lvl * 3;  // Best speed
        rpgCharacter.magic = 5;  // No magic
        rpgCharacter.endurance = 20 + lvl * 2;
        rpgCharacter.titanKills = lvl * 3;
        break;
        
      case CHAR_SAITAMA:
        rpgCharacter.name = "Saitama";
        rpgCharacter.strength = 50 + lvl * 5;  // OP strength
        rpgCharacter.speed = 50 + lvl * 5;
        rpgCharacter.magic = 0;  // No special powers
        rpgCharacter.endurance = 50 + lvl * 5;
        rpgCharacter.heroRank = lvl;  // C to S rank
        break;
        
      case CHAR_DEKU:
        rpgCharacter.name = "Izuku Midoriya";
        rpgCharacter.strength = 15 + lvl * 3;
        rpgCharacter.speed = 15 + lvl * 2;
        rpgCharacter.magic = 20 + lvl * 2;  // OFA
        rpgCharacter.endurance = 18 + lvl * 2;
        rpgCharacter.ofaPercent = 5 + lvl * 2;  // 5% to 100%+
        if (rpgCharacter.ofaPercent > 100) rpgCharacter.ofaPercent = 100;
        break;
        
      default:
        rpgCharacter.name = "Unknown";
        break;
    }
  } else {
    // Main theme characters
    switch(watch.theme) {
      case THEME_LUFFY:
        rpgCharacter.name = "Monkey D. Luffy";
        rpgCharacter.strength = 22 + lvl * 2;
        rpgCharacter.speed = 18 + lvl * 2;
        rpgCharacter.magic = 5;
        rpgCharacter.endurance = 28 + lvl * 3;
        rpgCharacter.hakiLevel = 1 + lvl / 10;
        break;
        
      case THEME_JINWOO:
        rpgCharacter.name = "Sung Jin-Woo";
        rpgCharacter.strength = 18 + lvl * 2;
        rpgCharacter.speed = 22 + lvl * 3;
        rpgCharacter.magic = 15 + lvl * 2;
        rpgCharacter.endurance = 18 + lvl * 2;
        rpgCharacter.shadowArmy = lvl * 2;
        break;
        
      case THEME_YUGO:
        rpgCharacter.name = "Yugo";
        rpgCharacter.strength = 12 + lvl;
        rpgCharacter.speed = 25 + lvl * 2;
        rpgCharacter.magic = 28 + lvl * 4;
        rpgCharacter.endurance = 12 + lvl;
        rpgCharacter.portalMastery = 1 + lvl / 8;
        break;
        
      default:
        rpgCharacter.name = "Unknown";
        break;
    }
  }
}

const char* getCurrentTitle() {
  int tier = (rpgCharacter.level - 1) / 10;  // 0-4 for levels 1-50
  if (tier > 4) tier = 4;
  
  if (watch.theme == THEME_RANDOM) {
    return getRandomCharacterTitleForLevel(rpgCharacter.level, watch.dailyCharacter);
  }
  
  switch(watch.theme) {
    case THEME_LUFFY:  return luffyTitles[tier];
    case THEME_JINWOO: return jinwooTitles[tier];
    case THEME_YUGO:   return yugoTitles[tier];
    default:           return "Unknown";
  }
}

const char* getRandomCharacterTitleForLevel(int level, RandomCharacter character) {
  int tier = (level - 1) / 10;
  if (tier > 4) tier = 4;
  
  switch(character) {
    case CHAR_NARUTO:  return narutoTitles[tier];
    case CHAR_GOKU:    return gokuTitles[tier];
    case CHAR_TANJIRO: return tanjiroTitles[tier];
    case CHAR_GOJO:    return gojoTitles[tier];
    case CHAR_LEVI:    return leviTitles[tier];
    case CHAR_SAITAMA: return saitamaTitles[tier];
    case CHAR_DEKU:    return dekuTitles[tier];
    default:           return "Unknown";
  }
}

const char* getSpecialPowerName() {
  if (watch.theme == THEME_RANDOM) {
    switch(watch.dailyCharacter) {
      case CHAR_NARUTO:  return "Chakra";
      case CHAR_GOKU:    return "Power Level";
      case CHAR_TANJIRO: return "Breathing Form";
      case CHAR_GOJO:    return "Cursed Energy";
      case CHAR_LEVI:    return "Titan Kills";
      case CHAR_SAITAMA: return "Hero Rank";
      case CHAR_DEKU:    return "OFA %";
      default:           return "Special";
    }
  }
  
  switch(watch.theme) {
    case THEME_LUFFY:  return "Haki Level";
    case THEME_JINWOO: return "Shadow Army";
    case THEME_YUGO:   return "Portal Mastery";
    default:           return "Special";
  }
}

int getSpecialPower() {
  if (watch.theme == THEME_RANDOM) {
    switch(watch.dailyCharacter) {
      case CHAR_NARUTO:  return rpgCharacter.chakraReserves;
      case CHAR_GOKU:    return rpgCharacter.powerLevel;
      case CHAR_TANJIRO: return rpgCharacter.breathingForm;
      case CHAR_GOJO:    return rpgCharacter.cursedEnergy;
      case CHAR_LEVI:    return rpgCharacter.titanKills;
      case CHAR_SAITAMA: return rpgCharacter.heroRank;
      case CHAR_DEKU:    return rpgCharacter.ofaPercent;
      default:           return 0;
    }
  }
  
  switch(watch.theme) {
    case THEME_LUFFY:  return rpgCharacter.hakiLevel;
    case THEME_JINWOO: return rpgCharacter.shadowArmy;
    case THEME_YUGO:   return rpgCharacter.portalMastery;
    default:           return 0;
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
  lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(nameLbl, lv_color_hex(colors.primary), 0);
  lv_label_set_text(nameLbl, rpgCharacter.name);
  lv_obj_align(nameLbl, LV_ALIGN_TOP_MID, 0, 60);

  lv_obj_t* titleLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(titleLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text(titleLbl, rpgCharacter.title);
  lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 85);

  // Level circle with glow effect
  lv_obj_t* glowCircle = lv_obj_create(scr);
  lv_obj_set_size(glowCircle, 90, 90);
  lv_obj_set_style_radius(glowCircle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(glowCircle, lv_color_hex(colors.glow), 0);
  lv_obj_set_style_bg_opa(glowCircle, LV_OPA_30, 0);
  lv_obj_set_style_border_width(glowCircle, 0, 0);
  lv_obj_align(glowCircle, LV_ALIGN_TOP_MID, 0, 110);
  
  lv_obj_t* levelCircle = lv_obj_create(scr);
  lv_obj_set_size(levelCircle, 70, 70);
  lv_obj_set_style_radius(levelCircle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(levelCircle, lv_color_hex(colors.primary), 0);
  lv_obj_set_style_border_width(levelCircle, 0, 0);
  lv_obj_align(levelCircle, LV_ALIGN_TOP_MID, 0, 120);

  lv_obj_t* levelNum = lv_label_create(levelCircle);
  lv_obj_set_style_text_font(levelNum, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(levelNum, lv_color_hex(colors.text), 0);
  lv_label_set_text_fmt(levelNum, "%d", rpgCharacter.level);
  lv_obj_center(levelNum);
  
  lv_obj_t* maxLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(maxLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(maxLbl, lv_color_hex(colors.secondary), 0);
  lv_label_set_text(maxLbl, "/ 50");
  lv_obj_align(maxLbl, LV_ALIGN_TOP_MID, 0, 195);

  // XP Progress
  lv_obj_t* xpLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(xpLabel, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(xpLabel, lv_color_hex(colors.text), 0);
  lv_label_set_text_fmt(xpLabel, "XP: %ld / %ld", rpgCharacter.xp, rpgCharacter.xpNext);
  lv_obj_align(xpLabel, LV_ALIGN_TOP_MID, 0, 215);

  lv_obj_t* xpBar = lv_bar_create(scr);
  lv_obj_set_size(xpBar, LCD_WIDTH - 60, 16);
  lv_obj_align(xpBar, LV_ALIGN_TOP_MID, 0, 235);
  lv_bar_set_range(xpBar, 0, rpgCharacter.xpNext);
  lv_bar_set_value(xpBar, rpgCharacter.xp, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(xpBar, lv_color_hex(colors.primary), LV_PART_INDICATOR);
  lv_obj_set_style_radius(xpBar, 8, 0);
  lv_obj_set_style_radius(xpBar, 8, LV_PART_INDICATOR);

  // Stats grid
  int yPos = 265;
  const char* statNames[] = {"STR", "SPD", "MAG", "END"};
  int statValues[] = {rpgCharacter.strength, rpgCharacter.speed, rpgCharacter.magic, rpgCharacter.endurance};

  for (int i = 0; i < 4; i++) {
    int x = 25 + (i % 2) * 170;
    int y = yPos + (i / 2) * 40;

    lv_obj_t* statLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(statLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(statLbl, lv_color_hex(colors.secondary), 0);
    lv_label_set_text(statLbl, statNames[i]);
    lv_obj_set_pos(statLbl, x, y);

    lv_obj_t* valLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(valLbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(valLbl, lv_color_hex(colors.text), 0);
    lv_label_set_text_fmt(valLbl, "%d", statValues[i]);
    lv_obj_set_pos(valLbl, x + 45, y - 2);
  }

  // Special stat based on character
  lv_obj_t* specialLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(specialLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(specialLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text_fmt(specialLbl, "%s: %d", getSpecialPowerName(), getSpecialPower());
  lv_obj_align(specialLbl, LV_ALIGN_BOTTOM_MID, 0, -20);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER STATS SCREEN (Swipeable)
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createCharStatsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  // Draw subtle power aura in background
  drawAnimePowerAura(scr, colors.glow, 50);

  // Character portrait area (placeholder circle)
  lv_obj_t* portrait = lv_obj_create(scr);
  lv_obj_set_size(portrait, 100, 100);
  lv_obj_align(portrait, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_radius(portrait, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(portrait, lv_color_hex(colors.primary), 0);
  lv_obj_set_style_border_color(portrait, lv_color_hex(colors.accent), 0);
  lv_obj_set_style_border_width(portrait, 3, 0);
  
  // Character initial
  lv_obj_t* initial = lv_label_create(portrait);
  lv_obj_set_style_text_font(initial, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(initial, lv_color_hex(colors.text), 0);
  lv_label_set_text_fmt(initial, "%c", rpgCharacter.name[0]);
  lv_obj_center(initial);

  // Character name
  lv_obj_t* nameLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(nameLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text(nameLbl, rpgCharacter.name);
  lv_obj_align(nameLbl, LV_ALIGN_TOP_MID, 0, 140);

  // Title
  lv_obj_t* titleLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(titleLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text(titleLbl, rpgCharacter.title);
  lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 165);
  
  // Show series for random characters
  if (watch.theme == THEME_RANDOM) {
    lv_obj_t* seriesLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(seriesLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(seriesLbl, lv_color_hex(colors.secondary), 0);
    lv_label_set_text_fmt(seriesLbl, "From: %s", getRandomCharacterSeries(watch.dailyCharacter));
    lv_obj_align(seriesLbl, LV_ALIGN_TOP_MID, 0, 185);
  }

  // Level display
  lv_obj_t* levelLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(levelLbl, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(levelLbl, lv_color_hex(colors.primary), 0);
  lv_label_set_text_fmt(levelLbl, "Lv. %d", rpgCharacter.level);
  lv_obj_align(levelLbl, LV_ALIGN_TOP_MID, 0, 210);

  // Stats bars
  const char* statNames[] = {"STR", "SPD", "MAG", "END"};
  int statValues[] = {rpgCharacter.strength, rpgCharacter.speed, rpgCharacter.magic, rpgCharacter.endurance};
  int maxStat = 200;  // Visual max for bars
  
  for (int i = 0; i < 4; i++) {
    int y = 250 + i * 35;
    
    lv_obj_t* statName = lv_label_create(scr);
    lv_obj_set_style_text_font(statName, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(statName, lv_color_hex(colors.text), 0);
    lv_label_set_text(statName, statNames[i]);
    lv_obj_set_pos(statName, 20, y);
    
    lv_obj_t* bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 200, 12);
    lv_obj_set_pos(bar, 60, y + 2);
    lv_bar_set_range(bar, 0, maxStat);
    lv_bar_set_value(bar, statValues[i] > maxStat ? maxStat : statValues[i], LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_hex(colors.primary), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, 0);
    lv_obj_set_style_radius(bar, 6, LV_PART_INDICATOR);
    
    lv_obj_t* valLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(valLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(valLbl, lv_color_hex(colors.text), 0);
    lv_label_set_text_fmt(valLbl, "%d", statValues[i]);
    lv_obj_set_pos(valLbl, 270, y);
  }

  // Special power display
  lv_obj_t* specialBox = lv_obj_create(scr);
  lv_obj_set_size(specialBox, LCD_WIDTH - 40, 50);
  lv_obj_align(specialBox, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(specialBox, lv_color_hex(colors.secondary), 0);
  lv_obj_set_style_bg_opa(specialBox, LV_OPA_50, 0);
  lv_obj_set_style_radius(specialBox, 10, 0);
  lv_obj_set_style_border_width(specialBox, 0, 0);
  
  lv_obj_t* specialLbl = lv_label_create(specialBox);
  lv_obj_set_style_text_font(specialLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(specialLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text_fmt(specialLbl, "%s: %d", getSpecialPowerName(), getSpecialPower());
  lv_obj_center(specialLbl);

  return scr;
}
