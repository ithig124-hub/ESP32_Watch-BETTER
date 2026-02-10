/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  PREMIUM ANIME THEMES - Implementation
 *  Ultra High Quality Character Representation
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include "themes.h"
#include <time.h>

// Current active theme pointer
const PremiumTheme* currentTheme = nullptr;

// ═══════════════════════════════════════════════════════════════════════════════
//  🌟 GEAR 5 LUFFY - Sun God Nika Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_GEAR5_LUFFY = {
  // Character Identity
  .characterName = "Monkey D. Luffy",
  .characterTitle = "Sun God Nika",
  .seriesName = "ONE PIECE",
  .signatureMove = "Bajrang Gun",
  .catchphrase = "I'm gonna be King of the Pirates!",
  
  // Core Colors - White/Gold/Red Sun God aesthetic
  .primary = LUFFY_PRIMARY,           // Pure White
  .secondary = LUFFY_SECONDARY,       // Brilliant Gold
  .accent = LUFFY_ACCENT,             // Straw Hat Red
  .background = LUFFY_BACKGROUND,     // Deep Black
  .text = 0xFFFF,                     // White text
  .glow = LUFFY_GLOW,                 // Sun glow
  .highlight = LUFFY_HIGHLIGHT,       // Orange energy
  
  // Special Effects - Cloud & Sun visuals
  .effectColor1 = LUFFY_CLOUD,        // Cloud white
  .effectColor2 = LUFFY_GLOW,         // Golden rays
  .effectColor3 = LUFFY_ACCENT,       // Red accents
  
  // UI Style - Bouncy, joyful, rounded
  .cornerRadius = 20,                 // Very rounded (cloud-like)
  .glowIntensity = 200,               // Strong sun glow
  .animationSpeed = 8,                // Fast, energetic
  .useGradient = true,                // Sun gradient
  .useGlow = true,                    // Nika aura
  .darkMode = true,                   // Dark bg for contrast
  
  // Character Stats
  .stat1Name = "HAKI",
  .stat2Name = "GEAR",
  .stat3Name = "FREEDOM",
  .stat4Name = "WILL"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  🌑 JIN-WOO - Shadow Monarch Premium Theme  
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_SHADOW_MONARCH = {
  // Character Identity
  .characterName = "Sung Jin-Woo",
  .characterTitle = "Shadow Monarch",
  .seriesName = "SOLO LEVELING",
  .signatureMove = "ARISE",
  .catchphrase = "I alone am the honored one.",
  
  // Core Colors - Purple/Black/Blue shadow realm
  .primary = JINWOO_PRIMARY,          // Royal Purple
  .secondary = JINWOO_SECONDARY,      // Void Black
  .accent = JINWOO_ACCENT,            // Electric Blue eyes
  .background = JINWOO_BACKGROUND,    // Absolute Black
  .text = 0xBDF7,                     // Silver-white text
  .glow = JINWOO_GLOW,                // Purple glow
  .highlight = JINWOO_HIGHLIGHT,      // Ice blue
  
  // Special Effects - Shadow/Arise visuals
  .effectColor1 = JINWOO_ARISE,       // ARISE purple
  .effectColor2 = JINWOO_SHADOW,      // Shadow mist
  .effectColor3 = JINWOO_ACCENT,      // Blue daggers
  
  // UI Style - Sharp, powerful, intimidating
  .cornerRadius = 5,                  // Sharp edges
  .glowIntensity = 255,               // Maximum glow
  .animationSpeed = 6,                // Deliberate, powerful
  .useGradient = true,                // Shadow gradient
  .useGlow = true,                    // Strong purple aura
  .darkMode = true,                   // Absolute dark
  
  // Character Stats
  .stat1Name = "SHADOWS",
  .stat2Name = "POWER",
  .stat3Name = "LEVEL",
  .stat4Name = "ARMY"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  🌀 YUGO - Portal Master Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_PORTAL_MASTER = {
  // Character Identity
  .characterName = "Yugo",
  .characterTitle = "Eliatrope King",
  .seriesName = "WAKFU",
  .signatureMove = "Wakfu Portal",
  .catchphrase = "Adventure awaits!",
  
  // Core Colors - Cyan/Teal/Gold adventure aesthetic
  .primary = YUGO_PRIMARY,            // Bright Cyan
  .secondary = YUGO_SECONDARY,        // Deep Teal
  .accent = YUGO_ACCENT,              // Warm Gold
  .background = YUGO_BACKGROUND,      // Dark Blue-Grey
  .text = 0xFFFF,                     // White text
  .glow = YUGO_GLOW,                  // Cyan portal glow
  .highlight = YUGO_HIGHLIGHT,        // Seafoam energy
  
  // Special Effects - Portal ring visuals
  .effectColor1 = YUGO_PORTAL,        // Portal ring
  .effectColor2 = YUGO_ENERGY,        // Wakfu energy
  .effectColor3 = YUGO_ACCENT,        // Gold sparkles
  
  // UI Style - Rounded, friendly, adventurous
  .cornerRadius = 15,                 // Rounded (portal-like)
  .glowIntensity = 180,               // Soft portal glow
  .animationSpeed = 7,                // Smooth, flowing
  .useGradient = true,                // Portal gradient
  .useGlow = true,                    // Portal aura
  .darkMode = true,                   // Dark for portal pop
  
  // Character Stats
  .stat1Name = "WAKFU",
  .stat2Name = "PORTALS",
  .stat3Name = "SPEED",
  .stat4Name = "HEART"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  🦊 NARUTO - Sage Mode Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_SAGE_NARUTO = {
  .characterName = "Naruto Uzumaki",
  .characterTitle = "Seventh Hokage",
  .seriesName = "NARUTO",
  .signatureMove = "Rasengan",
  .catchphrase = "Believe it! Dattebayo!",
  
  .primary = NARUTO_PRIMARY,
  .secondary = NARUTO_SECONDARY,
  .accent = NARUTO_ACCENT,
  .background = NARUTO_BACKGROUND,
  .text = 0xFD20,                     // Orange text
  .glow = NARUTO_GLOW,
  .highlight = NARUTO_SAGE_EYE,
  
  .effectColor1 = NARUTO_KURAMA,      // Kurama chakra
  .effectColor2 = NARUTO_RASENGAN,    // Rasengan blue
  .effectColor3 = NARUTO_GLOW,        // Sage gold
  
  .cornerRadius = 12,
  .glowIntensity = 220,
  .animationSpeed = 9,                // Hyperactive!
  .useGradient = true,
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "CHAKRA",
  .stat2Name = "SAGE",
  .stat3Name = "KURAMA",
  .stat4Name = "BONDS"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  ⚡ GOKU - Ultra Instinct Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_UI_GOKU = {
  .characterName = "Son Goku",
  .characterTitle = "Ultra Instinct",
  .seriesName = "DRAGON BALL",
  .signatureMove = "Kamehameha",
  .catchphrase = "I am the hope of the universe!",
  
  .primary = GOKU_PRIMARY,            // Silver
  .secondary = GOKU_SECONDARY,        // Deep Blue
  .accent = GOKU_ACCENT,              // Pure White
  .background = GOKU_BACKGROUND,      // Black void
  .text = 0xC618,                     // Silver text
  .glow = GOKU_GLOW,                  // Silver glow
  .highlight = GOKU_DIVINE,
  
  .effectColor1 = GOKU_AURA,          // UI Aura
  .effectColor2 = GOKU_KI,            // Blue Ki
  .effectColor3 = GOKU_DIVINE,        // Divine silver
  
  .cornerRadius = 8,
  .glowIntensity = 240,               // Intense divine aura
  .animationSpeed = 10,               // Fastest!
  .useGradient = true,
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "POWER",
  .stat2Name = "SPEED",
  .stat3Name = "KI",
  .stat4Name = "FORMS"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  🔥 TANJIRO - Sun Breathing Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_SUN_TANJIRO = {
  .characterName = "Tanjiro Kamado",
  .characterTitle = "Sun Breathing",
  .seriesName = "DEMON SLAYER",
  .signatureMove = "Hinokami Kagura",
  .catchphrase = "I will never give up!",
  
  .primary = TANJIRO_PRIMARY,         // Crimson
  .secondary = TANJIRO_SECONDARY,     // Dark Green
  .accent = TANJIRO_ACCENT,           // Fire Orange
  .background = TANJIRO_BACKGROUND,   // Charcoal
  .text = 0xFC00,                     // Flame text
  .glow = TANJIRO_GLOW,
  .highlight = TANJIRO_FIRE,
  
  .effectColor1 = TANJIRO_FIRE,       // Sun fire
  .effectColor2 = TANJIRO_WATER,      // Water breathing
  .effectColor3 = TANJIRO_SCAR,       // Scar mark
  
  .cornerRadius = 10,
  .glowIntensity = 200,
  .animationSpeed = 7,
  .useGradient = true,
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "BREATH",
  .stat2Name = "FORMS",
  .stat3Name = "SENSE",
  .stat4Name = "WILL"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  ♾️ GOJO - Infinity Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_INFINITY_GOJO = {
  .characterName = "Satoru Gojo",
  .characterTitle = "The Strongest",
  .seriesName = "JUJUTSU KAISEN",
  .signatureMove = "Infinite Void",
  .catchphrase = "Throughout Heaven and Earth, I alone am the honored one.",
  
  .primary = GOJO_PRIMARY,            // Infinity Blue
  .secondary = GOJO_SECONDARY,        // White hair
  .accent = GOJO_ACCENT,              // Purple cursed energy
  .background = GOJO_BACKGROUND,      // Black
  .text = 0x5DDF,                     // Blue text
  .glow = GOJO_GLOW,
  .highlight = GOJO_EYES,
  
  .effectColor1 = GOJO_INFINITY,      // Infinity sphere
  .effectColor2 = GOJO_DOMAIN,        // Domain expansion
  .effectColor3 = GOJO_EYES,          // Six Eyes
  
  .cornerRadius = 25,                 // Perfect circles (infinity)
  .glowIntensity = 255,               // Maximum (untouchable)
  .animationSpeed = 8,
  .useGradient = true,
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "INFINITY",
  .stat2Name = "CURSED",
  .stat3Name = "SIX EYES",
  .stat4Name = "DOMAIN"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  ⚔️ LEVI - Captain Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_CAPTAIN_LEVI = {
  .characterName = "Levi Ackerman",
  .characterTitle = "Humanity's Strongest",
  .seriesName = "ATTACK ON TITAN",
  .signatureMove = "Spin Attack",
  .catchphrase = "Give up on your dreams and die.",
  
  .primary = LEVI_PRIMARY,            // Military Grey
  .secondary = LEVI_SECONDARY,        // Dark uniform
  .accent = LEVI_ACCENT,              // Survey Corps Green
  .background = LEVI_BACKGROUND,      // Charcoal
  .text = 0xC618,                     // Silver text
  .glow = LEVI_GLOW,
  .highlight = LEVI_BLADE,
  
  .effectColor1 = LEVI_CAPE,          // Green cape
  .effectColor2 = LEVI_BLADE,         // Steel blades
  .effectColor3 = LEVI_BLOOD,         // Titan blood
  
  .cornerRadius = 3,                  // Sharp, precise
  .glowIntensity = 120,               // Subtle, controlled
  .animationSpeed = 9,                // Lightning fast
  .useGradient = false,               // Clean, no frills
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "TITANS",
  .stat2Name = "KILLS",
  .stat3Name = "SPEED",
  .stat4Name = "CLEAN"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  👊 SAITAMA - One Punch Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_HERO_SAITAMA = {
  .characterName = "Saitama",
  .characterTitle = "One Punch Man",
  .seriesName = "ONE PUNCH MAN",
  .signatureMove = "Serious Punch",
  .catchphrase = "OK.",
  
  .primary = SAITAMA_PRIMARY,         // Yellow suit
  .secondary = SAITAMA_SECONDARY,     // Red cape
  .accent = SAITAMA_ACCENT,           // White (bald)
  .background = SAITAMA_BACKGROUND,   // Black
  .text = 0xFFE0,                     // Yellow text
  .glow = SAITAMA_GLOW,
  .highlight = SAITAMA_FIST,
  
  .effectColor1 = SAITAMA_CAPE,       // Cape flow
  .effectColor2 = SAITAMA_FIST,       // Punch impact
  .effectColor3 = SAITAMA_PRIMARY,    // Yellow flash
  
  .cornerRadius = 0,                  // Simple, flat
  .glowIntensity = 150,               // Understated power
  .animationSpeed = 5,                // Casual, bored
  .useGradient = false,               // Simple aesthetic
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "PUNCH",
  .stat2Name = "RANK",
  .stat3Name = "BORED",
  .stat4Name = "SALES"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  💚 DEKU - Plus Ultra Premium Theme
// ═══════════════════════════════════════════════════════════════════════════════
const PremiumTheme THEME_PLUS_ULTRA_DEKU = {
  .characterName = "Izuku Midoriya",
  .characterTitle = "The Greatest Hero",
  .seriesName = "MY HERO ACADEMIA",
  .signatureMove = "United States of Smash",
  .catchphrase = "PLUS ULTRA!",
  
  .primary = DEKU_PRIMARY,            // Hero Green
  .secondary = DEKU_SECONDARY,        // Black costume
  .accent = DEKU_ACCENT,              // Yellow (All Might)
  .background = DEKU_BACKGROUND,      // Dark
  .text = 0x07E0,                     // Green text
  .glow = DEKU_GLOW,
  .highlight = DEKU_LIGHTNING,
  
  .effectColor1 = DEKU_LIGHTNING,     // OFA Lightning
  .effectColor2 = DEKU_FULL_COWL,     // Full Cowl
  .effectColor3 = DEKU_VEINS,         // Power veins
  
  .cornerRadius = 12,
  .glowIntensity = 230,               // Intense OFA glow
  .animationSpeed = 8,
  .useGradient = true,
  .useGlow = true,
  .darkMode = true,
  
  .stat1Name = "OFA %",
  .stat2Name = "QUIRKS",
  .stat3Name = "SMASH",
  .stat4Name = "HERO"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  DAILY CHARACTER ARRAY
// ═══════════════════════════════════════════════════════════════════════════════
static const PremiumTheme* dailyThemes[RANDOM_CHAR_COUNT] = {
  &THEME_SAGE_NARUTO,
  &THEME_UI_GOKU,
  &THEME_SUN_TANJIRO,
  &THEME_INFINITY_GOJO,
  &THEME_CAPTAIN_LEVI,
  &THEME_HERO_SAITAMA,
  &THEME_PLUS_ULTRA_DEKU
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME FUNCTIONS IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════════════════════

void initThemes() {
  currentTheme = &THEME_GEAR5_LUFFY;
  Serial.println("[THEME] Premium themes initialized");
  Serial.printf("[THEME] Default: %s - %s\n", 
    currentTheme->characterName, 
    currentTheme->characterTitle);
}

void setTheme(ThemeType type) {
  switch (type) {
    case THEME_LUFFY:
      currentTheme = &THEME_GEAR5_LUFFY;
      break;
    case THEME_JINWOO:
      currentTheme = &THEME_SHADOW_MONARCH;
      break;
    case THEME_YUGO:
      currentTheme = &THEME_PORTAL_MASTER;
      break;
    case THEME_RANDOM:
      currentTheme = getDailyCharacterTheme();
      break;
    default:
      currentTheme = &THEME_GEAR5_LUFFY;
  }
  
  Serial.printf("[THEME] Switched to: %s - %s\n", 
    currentTheme->characterName, 
    currentTheme->characterTitle);
}

const PremiumTheme* getCurrentTheme() {
  return currentTheme;
}

const PremiumTheme* getDailyCharacterTheme() {
  // Get day of year for daily rotation
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int dayOfYear = timeinfo->tm_yday;
  
  // Rotate through 7 characters
  int charIndex = dayOfYear % RANDOM_CHAR_COUNT;
  
  return dailyThemes[charIndex];
}

lv_color_t themeColor(uint16_t rgb565) {
  return lv_color_hex(rgb565);
}

void applyThemeToScreen(lv_obj_t* screen) {
  if (!currentTheme || !screen) return;
  
  // Set background color
  lv_obj_set_style_bg_color(screen, themeColor(currentTheme->background), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  
  // Add gradient if enabled
  if (currentTheme->useGradient) {
    lv_obj_set_style_bg_grad_color(screen, themeColor(currentTheme->primary), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
  }
}

lv_obj_t* createThemedContainer(lv_obj_t* parent, int width, int height) {
  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_set_size(cont, width, height);
  lv_obj_set_style_bg_color(cont, themeColor(currentTheme->secondary), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_80, 0);
  lv_obj_set_style_radius(cont, currentTheme->cornerRadius, 0);
  lv_obj_set_style_border_width(cont, 2, 0);
  lv_obj_set_style_border_color(cont, themeColor(currentTheme->primary), 0);
  
  // Add glow effect if enabled
  if (currentTheme->useGlow) {
    lv_obj_set_style_shadow_color(cont, themeColor(currentTheme->glow), 0);
    lv_obj_set_style_shadow_width(cont, currentTheme->glowIntensity / 10, 0);
    lv_obj_set_style_shadow_spread(cont, 5, 0);
    lv_obj_set_style_shadow_opa(cont, currentTheme->glowIntensity, 0);
  }
  
  return cont;
}

lv_obj_t* createThemedButton(lv_obj_t* parent, const char* text) {
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_set_style_bg_color(btn, themeColor(currentTheme->primary), 0);
  lv_obj_set_style_bg_color(btn, themeColor(currentTheme->highlight), LV_STATE_PRESSED);
  lv_obj_set_style_radius(btn, currentTheme->cornerRadius, 0);
  
  // Add glow on focus
  if (currentTheme->useGlow) {
    lv_obj_set_style_shadow_color(btn, themeColor(currentTheme->glow), LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(btn, 15, LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(btn, 200, LV_STATE_FOCUSED);
  }
  
  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, themeColor(currentTheme->text), 0);
  lv_obj_center(label);
  
  return btn;
}

lv_obj_t* createThemedLabel(lv_obj_t* parent, const char* text, bool isTitle) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  
  if (isTitle) {
    lv_obj_set_style_text_color(label, themeColor(currentTheme->primary), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  } else {
    lv_obj_set_style_text_color(label, themeColor(currentTheme->text), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
  }
  
  return label;
}

void createSignatureEffect(lv_obj_t* parent) {
  // Create character-specific visual effect
  lv_obj_t* effect = lv_obj_create(parent);
  lv_obj_set_size(effect, 60, 60);
  lv_obj_align(effect, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_set_style_bg_color(effect, themeColor(currentTheme->effectColor1), 0);
  lv_obj_set_style_bg_opa(effect, LV_OPA_60, 0);
  lv_obj_set_style_radius(effect, currentTheme->cornerRadius, 0);
  lv_obj_clear_flag(effect, LV_OBJ_FLAG_SCROLLABLE);
  
  // Add glow
  lv_obj_set_style_shadow_color(effect, themeColor(currentTheme->glow), 0);
  lv_obj_set_style_shadow_width(effect, 20, 0);
  lv_obj_set_style_shadow_opa(effect, currentTheme->glowIntensity, 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME HELPER FUNCTIONS - Used by games.cpp, rpg.cpp, utilities.cpp
// ═══════════════════════════════════════════════════════════════════════════════

ThemeColors getThemeColors(ThemeType theme) {
  ThemeColors colors;
  const PremiumTheme* t = nullptr;
  
  switch (theme) {
    case THEME_LUFFY:
      t = &THEME_GEAR5_LUFFY;
      break;
    case THEME_JINWOO:
      t = &THEME_SHADOW_MONARCH;
      break;
    case THEME_YUGO:
      t = &THEME_PORTAL_MASTER;
      break;
    case THEME_RANDOM:
      t = getDailyCharacterTheme();
      break;
    default:
      t = &THEME_GEAR5_LUFFY;
  }
  
  colors.primary = t->primary;
  colors.secondary = t->secondary;
  colors.accent = t->accent;
  colors.background = t->background;
  colors.text = t->text;
  colors.glow = t->glow;
  colors.highlight = t->highlight;
  
  return colors;
}

const char* getRandomCharacterName(RandomCharacter character) {
  switch (character) {
    case CHAR_NARUTO:   return "Naruto Uzumaki";
    case CHAR_GOKU:     return "Son Goku";
    case CHAR_TANJIRO:  return "Tanjiro Kamado";
    case CHAR_GOJO:     return "Satoru Gojo";
    case CHAR_LEVI:     return "Levi Ackerman";
    case CHAR_SAITAMA:  return "Saitama";
    case CHAR_DEKU:     return "Izuku Midoriya";
    default:            return "Unknown";
  }
}

const char* getRandomCharacterSeries(RandomCharacter character) {
  switch (character) {
    case CHAR_NARUTO:   return "Naruto Shippuden";
    case CHAR_GOKU:     return "Dragon Ball Super";
    case CHAR_TANJIRO:  return "Demon Slayer";
    case CHAR_GOJO:     return "Jujutsu Kaisen";
    case CHAR_LEVI:     return "Attack on Titan";
    case CHAR_SAITAMA:  return "One Punch Man";
    case CHAR_DEKU:     return "My Hero Academia";
    default:            return "Unknown Series";
  }
}

ThemeColors getRandomCharacterColors(RandomCharacter character) {
  ThemeColors colors;
  const PremiumTheme* t = nullptr;
  
  if (character < RANDOM_CHAR_COUNT) {
    t = dailyThemes[character];
  } else {
    t = &THEME_SAGE_NARUTO;
  }
  
  colors.primary = t->primary;
  colors.secondary = t->secondary;
  colors.accent = t->accent;
  colors.background = t->background;
  colors.text = t->text;
  colors.glow = t->glow;
  colors.highlight = t->highlight;
  
  return colors;
}

RandomCharacter getDailyCharacter() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int dayOfYear = timeinfo->tm_yday;
  return (RandomCharacter)(dayOfYear % RANDOM_CHAR_COUNT);
}
