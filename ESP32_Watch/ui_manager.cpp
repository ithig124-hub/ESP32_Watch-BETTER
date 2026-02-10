/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  PREMIUM UI MANAGER - Implementation
 *  Ultra High Quality Character Screens & Watch Faces
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include "ui_manager.h"
#include "themes.h"
#include "config.h"
#include "power_manager.h"

// Screen objects
static lv_obj_t* currentScreen = nullptr;
static lv_obj_t* clockLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* characterLabel = nullptr;
static lv_obj_t* batteryLabel = nullptr;

// ═══════════════════════════════════════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

void applyPremiumTextStyle(lv_obj_t* label, bool isTitle, bool useGlow) {
  const PremiumTheme* theme = getCurrentTheme();
  if (!theme) return;
  
  if (isTitle) {
    lv_obj_set_style_text_color(label, themeColor(theme->primary), 0);
  } else {
    lv_obj_set_style_text_color(label, themeColor(theme->text), 0);
  }
  
  // Add text shadow/glow effect
  if (useGlow && theme->useGlow) {
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    // LVGL doesn't have direct text glow, but we can fake it with layering
  }
}

lv_obj_t* createGlassContainer(lv_obj_t* parent, int x, int y, int w, int h) {
  const PremiumTheme* theme = getCurrentTheme();
  
  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_set_pos(cont, x, y);
  lv_obj_set_size(cont, w, h);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  
  // Glassmorphism effect
  lv_obj_set_style_bg_color(cont, themeColor(theme->secondary), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_40, 0);
  lv_obj_set_style_radius(cont, theme->cornerRadius, 0);
  lv_obj_set_style_border_width(cont, 1, 0);
  lv_obj_set_style_border_color(cont, themeColor(theme->primary), 0);
  lv_obj_set_style_border_opa(cont, LV_OPA_60, 0);
  
  // Shadow/glow
  if (theme->useGlow) {
    lv_obj_set_style_shadow_color(cont, themeColor(theme->glow), 0);
    lv_obj_set_style_shadow_width(cont, 15, 0);
    lv_obj_set_style_shadow_spread(cont, 2, 0);
    lv_obj_set_style_shadow_opa(cont, theme->glowIntensity / 2, 0);
  }
  
  return cont;
}

lv_obj_t* createPowerMeter(lv_obj_t* parent, const char* labelText, int value, int max) {
  const PremiumTheme* theme = getCurrentTheme();
  
  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_set_size(cont, 280, 40);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  
  // Label
  lv_obj_t* label = lv_label_create(cont);
  lv_label_set_text(label, labelText);
  lv_obj_set_style_text_color(label, themeColor(theme->text), 0);
  lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, -8);
  
  // Value
  char valStr[16];
  snprintf(valStr, sizeof(valStr), "%d/%d", value, max);
  lv_obj_t* valLabel = lv_label_create(cont);
  lv_label_set_text(valLabel, valStr);
  lv_obj_set_style_text_color(valLabel, themeColor(theme->primary), 0);
  lv_obj_align(valLabel, LV_ALIGN_RIGHT_MID, 0, -8);
  
  // Progress bar background
  lv_obj_t* barBg = lv_obj_create(cont);
  lv_obj_set_size(barBg, 280, 8);
  lv_obj_align(barBg, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(barBg, themeColor(theme->secondary), 0);
  lv_obj_set_style_bg_opa(barBg, LV_OPA_60, 0);
  lv_obj_set_style_radius(barBg, 4, 0);
  lv_obj_clear_flag(barBg, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(barBg, 0, 0);
  lv_obj_set_style_pad_all(barBg, 0, 0);
  
  // Progress bar fill
  int fillWidth = (280 * value) / max;
  if (fillWidth > 0) {
    lv_obj_t* barFill = lv_obj_create(barBg);
    lv_obj_set_size(barFill, fillWidth, 8);
    lv_obj_align(barFill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(barFill, themeColor(theme->primary), 0);
    lv_obj_set_style_bg_opa(barFill, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(barFill, 4, 0);
    lv_obj_clear_flag(barFill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(barFill, 0, 0);
    lv_obj_set_style_pad_all(barFill, 0, 0);
    
    // Glow effect on bar
    if (theme->useGlow) {
      lv_obj_set_style_shadow_color(barFill, themeColor(theme->glow), 0);
      lv_obj_set_style_shadow_width(barFill, 8, 0);
      lv_obj_set_style_shadow_opa(barFill, 180, 0);
    }
  }
  
  return cont;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STATUS BAR
// ═══════════════════════════════════════════════════════════════════════════════

void createStatusBar(lv_obj_t* parent) {
  const PremiumTheme* theme = getCurrentTheme();
  
  // Status bar container
  lv_obj_t* statusBar = lv_obj_create(parent);
  lv_obj_set_size(statusBar, SCREEN_W, 30);
  lv_obj_align(statusBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(statusBar, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(statusBar, 0, 0);
  lv_obj_clear_flag(statusBar, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(statusBar, 0, 0);
  
  // Character name (left)
  characterLabel = lv_label_create(statusBar);
  lv_label_set_text(characterLabel, theme->characterTitle);
  lv_obj_set_style_text_color(characterLabel, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(characterLabel, &lv_font_montserrat_12, 0);
  lv_obj_align(characterLabel, LV_ALIGN_LEFT_MID, 10, 0);
  
  // Battery (right)
  char battStr[16];
  snprintf(battStr, sizeof(battStr), "%d%%", watch.batteryPercent);
  batteryLabel = lv_label_create(statusBar);
  lv_label_set_text(batteryLabel, battStr);
  lv_obj_set_style_text_color(batteryLabel, themeColor(theme->text), 0);
  lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_12, 0);
  lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, -10, 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER SIGNATURE VISUALS
// ═══════════════════════════════════════════════════════════════════════════════

void createCharacterSignature(lv_obj_t* parent) {
  const PremiumTheme* theme = getCurrentTheme();
  
  // Create signature visual based on character
  // This adds unique character flair to each theme
  
  // Outer glow ring
  lv_obj_t* glowRing = lv_obj_create(parent);
  lv_obj_set_size(glowRing, 320, 320);
  lv_obj_center(glowRing);
  lv_obj_set_style_bg_opa(glowRing, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(glowRing, 3, 0);
  lv_obj_set_style_border_color(glowRing, themeColor(theme->glow), 0);
  lv_obj_set_style_border_opa(glowRing, LV_OPA_30, 0);
  lv_obj_set_style_radius(glowRing, 160, 0);
  lv_obj_clear_flag(glowRing, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
  
  // Shadow/glow effect
  lv_obj_set_style_shadow_color(glowRing, themeColor(theme->glow), 0);
  lv_obj_set_style_shadow_width(glowRing, 30, 0);
  lv_obj_set_style_shadow_spread(glowRing, 5, 0);
  lv_obj_set_style_shadow_opa(glowRing, theme->glowIntensity / 3, 0);
  
  // Inner accent ring
  lv_obj_t* accentRing = lv_obj_create(parent);
  lv_obj_set_size(accentRing, 280, 280);
  lv_obj_center(accentRing);
  lv_obj_set_style_bg_opa(accentRing, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(accentRing, 2, 0);
  lv_obj_set_style_border_color(accentRing, themeColor(theme->accent), 0);
  lv_obj_set_style_border_opa(accentRing, LV_OPA_20, 0);
  lv_obj_set_style_radius(accentRing, 140, 0);
  lv_obj_clear_flag(accentRing, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  NAVIGATION DOTS
// ═══════════════════════════════════════════════════════════════════════════════

void createNavDots(lv_obj_t* parent, int current, int total) {
  const PremiumTheme* theme = getCurrentTheme();
  
  // Container for dots
  lv_obj_t* dotsContainer = lv_obj_create(parent);
  lv_obj_set_size(dotsContainer, total * 20, 10);
  lv_obj_align(dotsContainer, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_opa(dotsContainer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(dotsContainer, 0, 0);
  lv_obj_clear_flag(dotsContainer, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(dotsContainer, 0, 0);
  lv_obj_set_flex_flow(dotsContainer, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(dotsContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  for (int i = 0; i < total; i++) {
    lv_obj_t* dot = lv_obj_create(dotsContainer);
    lv_obj_set_size(dot, (i == current) ? 12 : 8, (i == current) ? 12 : 8);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    
    if (i == current) {
      lv_obj_set_style_bg_color(dot, themeColor(theme->primary), 0);
      lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
      
      // Glow on active dot
      if (theme->useGlow) {
        lv_obj_set_style_shadow_color(dot, themeColor(theme->glow), 0);
        lv_obj_set_style_shadow_width(dot, 8, 0);
        lv_obj_set_style_shadow_opa(dot, 200, 0);
      }
    } else {
      lv_obj_set_style_bg_color(dot, themeColor(theme->secondary), 0);
      lv_obj_set_style_bg_opa(dot, LV_OPA_60, 0);
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM CLOCK SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

void createClockScreen() {
  const PremiumTheme* theme = getCurrentTheme();
  
  currentScreen = lv_obj_create(NULL);
  
  // Background with gradient
  lv_obj_set_style_bg_color(currentScreen, themeColor(theme->background), 0);
  lv_obj_set_style_bg_opa(currentScreen, LV_OPA_COVER, 0);
  
  if (theme->useGradient) {
    lv_obj_set_style_bg_grad_color(currentScreen, themeColor(theme->secondary), 0);
    lv_obj_set_style_bg_grad_dir(currentScreen, LV_GRAD_DIR_VER, 0);
  }
  
  // Character signature visual (rings)
  createCharacterSignature(currentScreen);
  
  // Status bar
  createStatusBar(currentScreen);
  
  // Series name (top)
  lv_obj_t* seriesLabel = lv_label_create(currentScreen);
  lv_label_set_text(seriesLabel, theme->seriesName);
  lv_obj_set_style_text_color(seriesLabel, themeColor(theme->accent), 0);
  lv_obj_set_style_text_font(seriesLabel, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_letter_space(seriesLabel, 3, 0);
  lv_obj_align(seriesLabel, LV_ALIGN_TOP_MID, 0, 50);
  
  // Character name
  lv_obj_t* nameLabel = lv_label_create(currentScreen);
  lv_label_set_text(nameLabel, theme->characterName);
  lv_obj_set_style_text_color(nameLabel, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_20, 0);
  lv_obj_align(nameLabel, LV_ALIGN_TOP_MID, 0, 75);
  
  // Main time display (HUGE)
  clockLabel = lv_label_create(currentScreen);
  char timeStr[8];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", watch.hour, watch.minute);
  lv_label_set_text(clockLabel, timeStr);
  lv_obj_set_style_text_color(clockLabel, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(clockLabel, &lv_font_montserrat_48, 0);
  lv_obj_center(clockLabel);
  lv_obj_align(clockLabel, LV_ALIGN_CENTER, 0, -20);
  
  // Seconds display
  lv_obj_t* secLabel = lv_label_create(currentScreen);
  char secStr[8];
  snprintf(secStr, sizeof(secStr), ":%02d", watch.second);
  lv_label_set_text(secLabel, secStr);
  lv_obj_set_style_text_color(secLabel, themeColor(theme->glow), 0);
  lv_obj_set_style_text_font(secLabel, &lv_font_montserrat_24, 0);
  lv_obj_align_to(secLabel, clockLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
  
  // Date display
  dateLabel = lv_label_create(currentScreen);
  const char* dayNames[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  char dateStr[32];
  snprintf(dateStr, sizeof(dateStr), "%s  %02d/%02d/%04d", 
    dayNames[watch.dayOfWeek], watch.month, watch.day, watch.year);
  lv_label_set_text(dateLabel, dateStr);
  lv_obj_set_style_text_color(dateLabel, themeColor(theme->text), 0);
  lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_14, 0);
  lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 50);
  
  // Signature move / catchphrase
  lv_obj_t* quoteLabel = lv_label_create(currentScreen);
  lv_label_set_text(quoteLabel, theme->signatureMove);
  lv_obj_set_style_text_color(quoteLabel, themeColor(theme->effectColor1), 0);
  lv_obj_set_style_text_font(quoteLabel, &lv_font_montserrat_16, 0);
  lv_obj_align(quoteLabel, LV_ALIGN_BOTTOM_MID, 0, -60);
  
  // Glow effect on quote
  if (theme->useGlow) {
    lv_obj_set_style_text_opa(quoteLabel, LV_OPA_90, 0);
  }
  
  // Navigation dots
  createNavDots(currentScreen, 0, 3);
  
  lv_scr_load(currentScreen);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  APPS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static const char* appNames[] = {
  "Steps", "Games", "Music", "Weather",
  "Quests", "Stats", "Settings", "Theme"
};

static const char* appIcons[] = {
  LV_SYMBOL_SHUFFLE, LV_SYMBOL_PLAY, LV_SYMBOL_AUDIO, LV_SYMBOL_GPS,
  LV_SYMBOL_LIST, LV_SYMBOL_SETTINGS, LV_SYMBOL_SETTINGS, LV_SYMBOL_IMAGE
};

void createAppsScreen() {
  const PremiumTheme* theme = getCurrentTheme();
  
  currentScreen = lv_obj_create(NULL);
  
  // Background
  lv_obj_set_style_bg_color(currentScreen, themeColor(theme->background), 0);
  lv_obj_set_style_bg_opa(currentScreen, LV_OPA_COVER, 0);
  
  // Status bar
  createStatusBar(currentScreen);
  
  // Title
  lv_obj_t* title = lv_label_create(currentScreen);
  lv_label_set_text(title, "APPS");
  lv_obj_set_style_text_color(title, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_letter_space(title, 4, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
  
  // App grid (4x2)
  int startY = 90;
  int iconSize = 70;
  int spacing = 85;
  int cols = 4;
  
  for (int i = 0; i < 8; i++) {
    int row = i / cols;
    int col = i % cols;
    int x = 25 + col * spacing;
    int y = startY + row * 100;
    
    // App button
    lv_obj_t* appBtn = createGlassContainer(currentScreen, x, y, iconSize, iconSize);
    lv_obj_clear_flag(appBtn, LV_OBJ_FLAG_SCROLLABLE);
    
    // Icon
    lv_obj_t* icon = lv_label_create(appBtn);
    lv_label_set_text(icon, appIcons[i]);
    lv_obj_set_style_text_color(icon, themeColor(theme->primary), 0);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
    lv_obj_center(icon);
    
    // Label
    lv_obj_t* label = lv_label_create(currentScreen);
    lv_label_set_text(label, appNames[i]);
    lv_obj_set_style_text_color(label, themeColor(theme->text), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_10, 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, x + iconSize/2, y + iconSize + 5);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  }
  
  // Navigation dots
  createNavDots(currentScreen, 1, 3);
  
  lv_scr_load(currentScreen);
}

void createAppsScreen2() {
  // Same structure as createAppsScreen but with different apps
  createAppsScreen();  // Simplified for now
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER STATS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

void createCharacterStatsScreen() {
  const PremiumTheme* theme = getCurrentTheme();
  
  currentScreen = lv_obj_create(NULL);
  
  // Background
  lv_obj_set_style_bg_color(currentScreen, themeColor(theme->background), 0);
  lv_obj_set_style_bg_opa(currentScreen, LV_OPA_COVER, 0);
  
  // Status bar
  createStatusBar(currentScreen);
  
  // Character portrait area (glass container)
  lv_obj_t* portrait = createGlassContainer(currentScreen, 20, 50, 100, 120);
  
  // Character initial
  lv_obj_t* initial = lv_label_create(portrait);
  char initStr[2] = {theme->characterName[0], '\0'};
  lv_label_set_text(initial, initStr);
  lv_obj_set_style_text_color(initial, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(initial, &lv_font_montserrat_48, 0);
  lv_obj_center(initial);
  
  // Character info
  lv_obj_t* nameLabel = lv_label_create(currentScreen);
  lv_label_set_text(nameLabel, theme->characterName);
  lv_obj_set_style_text_color(nameLabel, themeColor(theme->primary), 0);
  lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_18, 0);
  lv_obj_align(nameLabel, LV_ALIGN_TOP_LEFT, 130, 55);
  
  lv_obj_t* titleLabel = lv_label_create(currentScreen);
  lv_label_set_text(titleLabel, theme->characterTitle);
  lv_obj_set_style_text_color(titleLabel, themeColor(theme->accent), 0);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_14, 0);
  lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 130, 80);
  
  lv_obj_t* seriesLabel = lv_label_create(currentScreen);
  lv_label_set_text(seriesLabel, theme->seriesName);
  lv_obj_set_style_text_color(seriesLabel, themeColor(theme->text), 0);
  lv_obj_set_style_text_font(seriesLabel, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_opa(seriesLabel, LV_OPA_70, 0);
  lv_obj_align(seriesLabel, LV_ALIGN_TOP_LEFT, 130, 100);
  
  // Level display
  lv_obj_t* levelLabel = lv_label_create(currentScreen);
  char levelStr[16];
  snprintf(levelStr, sizeof(levelStr), "LVL %d", rpgCharacter.level);
  lv_label_set_text(levelLabel, levelStr);
  lv_obj_set_style_text_color(levelLabel, themeColor(theme->glow), 0);
  lv_obj_set_style_text_font(levelLabel, &lv_font_montserrat_20, 0);
  lv_obj_align(levelLabel, LV_ALIGN_TOP_LEFT, 130, 130);
  
  // Stats section
  int statsY = 190;
  
  // XP bar
  lv_obj_t* xpMeter = createPowerMeter(currentScreen, "XP", rpgCharacter.xp, rpgCharacter.xpNext);
  lv_obj_align(xpMeter, LV_ALIGN_TOP_MID, 0, statsY);
  
  // Stat bars
  lv_obj_t* stat1 = createPowerMeter(currentScreen, theme->stat1Name, 75, 100);
  lv_obj_align(stat1, LV_ALIGN_TOP_MID, 0, statsY + 55);
  
  lv_obj_t* stat2 = createPowerMeter(currentScreen, theme->stat2Name, 82, 100);
  lv_obj_align(stat2, LV_ALIGN_TOP_MID, 0, statsY + 110);
  
  lv_obj_t* stat3 = createPowerMeter(currentScreen, theme->stat3Name, 68, 100);
  lv_obj_align(stat3, LV_ALIGN_TOP_MID, 0, statsY + 165);
  
  // Catchphrase at bottom
  lv_obj_t* catchphrase = lv_label_create(currentScreen);
  lv_label_set_text(catchphrase, theme->catchphrase);
  lv_obj_set_style_text_color(catchphrase, themeColor(theme->effectColor1), 0);
  lv_obj_set_style_text_font(catchphrase, &lv_font_montserrat_12, 0);
  lv_obj_set_width(catchphrase, 300);
  lv_label_set_long_mode(catchphrase, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(catchphrase, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(catchphrase, LV_ALIGN_BOTTOM_MID, 0, -50);
  
  // Navigation dots
  createNavDots(currentScreen, 2, 3);
  
  lv_scr_load(currentScreen);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CLOCK UPDATE
// ═══════════════════════════════════════════════════════════════════════════════

void updateClock() {
  // Increment seconds
  watch.second++;
  if (watch.second >= 60) {
    watch.second = 0;
    watch.minute++;
    if (watch.minute >= 60) {
      watch.minute = 0;
      watch.hour++;
      if (watch.hour >= 24) {
        watch.hour = 0;
      }
    }
  }
  
  // Update clock label if on clock screen
  if (clockLabel && watch.screen == SCREEN_CLOCK) {
    char timeStr[8];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", watch.hour, watch.minute);
    lv_label_set_text(clockLabel, timeStr);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SHOW SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

void showScreen(ScreenType screen) {
  watch.screen = screen;
  
  // Delete old screen
  if (currentScreen) {
    lv_obj_del(currentScreen);
    currentScreen = nullptr;
    clockLabel = nullptr;
    dateLabel = nullptr;
  }
  
  // Create new screen
  switch (screen) {
    case SCREEN_CLOCK:
      createClockScreen();
      break;
    case SCREEN_APPS:
    case SCREEN_APPS2:
      createAppsScreen();
      break;
    case SCREEN_CHAR_STATS:
      createCharacterStatsScreen();
      break;
    default:
      createClockScreen();
      break;
  }
  
  Serial.printf("[UI] Screen: %d\n", screen);
}
