/**
 * UI Manager Implementation
 * Clean transitions, no bouncing effects
 */

#include "ui_manager.h"
#include "themes.h"
#include "rpg.h"

lv_obj_t* screens[16] = {nullptr};
static ScreenType screenHistory[10];
static int historyIndex = 0;

// Current theme colors
static ThemeColors currentColors;

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════════

lv_color_t getThemeColor(int index) {
  currentColors = getThemeColors(watch.theme);
  switch(index) {
    case 0: return lv_color_hex(currentColors.primary);
    case 1: return lv_color_hex(currentColors.secondary);
    case 2: return lv_color_hex(currentColors.accent);
    case 3: return lv_color_hex(currentColors.background);
    case 4: return lv_color_hex(currentColors.text);
    case 5: return lv_color_hex(currentColors.glow);
    default: return lv_color_hex(currentColors.highlight);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN NAVIGATION
// ═══════════════════════════════════════════════════════════════════════════════

void showScreen(ScreenType screen) {
  // Save history
  if (historyIndex < 9) {
    screenHistory[historyIndex++] = watch.screen;
  }

  watch.screen = screen;

  // Create screen if needed
  if (!screens[screen]) {
    switch(screen) {
      case SCREEN_CLOCK:      screens[screen] = createClockScreen(); break;
      case SCREEN_APPS:       screens[screen] = createAppsScreen(); break;
      case SCREEN_APPS2:      screens[screen] = createApps2Screen(); break;
      case SCREEN_STEPS:      screens[screen] = createStepsScreen(); break;
      case SCREEN_GAMES:      screens[screen] = createGamesScreen(); break;
      case SCREEN_MUSIC:      screens[screen] = createMusicScreen(); break;
      case SCREEN_WEATHER:    screens[screen] = createWeatherScreen(); break;
      case SCREEN_NEWS:       screens[screen] = createNewsScreen(); break;
      case SCREEN_QUESTS:     screens[screen] = createQuestsScreen(); break;
      case SCREEN_RPG:        screens[screen] = createRPGScreen(); break;
      case SCREEN_SETTINGS:   screens[screen] = createSettingsScreen(); break;
      case SCREEN_WALLPAPER:  screens[screen] = createWallpaperScreen(); break;
      case SCREEN_CHAR_STATS: screens[screen] = createCharStatsScreen(); break;
      case SCREEN_GACHA:      screens[screen] = createGachaScreen(); break;
      case SCREEN_TRAINING:   screens[screen] = createTrainingScreen(); break;
      case SCREEN_BOSS_RUSH:  screens[screen] = createBossRushScreen(); break;
    }
  }

  if (screens[screen]) {
    // Clean fade transition (no bounce)
    lv_scr_load_anim(screens[screen], LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
  }
}

void goBack() {
  if (historyIndex > 0) {
    showScreen(screenHistory[--historyIndex]);
  } else {
    showScreen(SCREEN_CLOCK);
  }
}

void goHome() {
  historyIndex = 0;
  showScreen(SCREEN_CLOCK);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CLOCK SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static lv_obj_t* timeLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* batteryLabel = nullptr;
static lv_obj_t* stepsPreview = nullptr;
static lv_obj_t* characterLabel = nullptr;

lv_obj_t* createClockScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Draw theme-specific effects (static, no animation)
  if (watch.theme == THEME_RANDOM) {
    drawRandomCharacterEffects(scr, watch.dailyCharacter);
  } else {
    drawThemeEffects(scr, watch.theme);
  }

  // Time display
  timeLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(timeLabel, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(timeLabel, "10:30");
  lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -60);

  // Date display
  dateLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(dateLabel, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(dateLabel, "Monday, Jan 6");
  lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 0);

  // Battery indicator
  batteryLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(batteryLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text(batteryLabel, "100%");
  lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -20, 20);

  // Character name for Random theme
  if (watch.theme == THEME_RANDOM) {
    characterLabel = lv_label_create(scr);
    lv_obj_set_style_text_font(characterLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(characterLabel, lv_color_hex(currentColors.primary), 0);
    lv_label_set_text_fmt(characterLabel, "Today: %s", getRandomCharacterName(watch.dailyCharacter));
    lv_obj_align(characterLabel, LV_ALIGN_TOP_LEFT, 20, 20);
  }

  // Steps preview arc
  stepsPreview = lv_arc_create(scr);
  lv_obj_set_size(stepsPreview, 120, 120);
  lv_arc_set_rotation(stepsPreview, 135);
  lv_arc_set_bg_angles(stepsPreview, 0, 270);
  lv_arc_set_value(stepsPreview, 0);
  lv_obj_set_style_arc_color(stepsPreview, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(stepsPreview, lv_color_hex(currentColors.secondary), LV_PART_MAIN);
  lv_obj_set_style_arc_opa(stepsPreview, LV_OPA_30, LV_PART_MAIN);
  lv_obj_align(stepsPreview, LV_ALIGN_CENTER, 0, 80);
  lv_obj_clear_flag(stepsPreview, LV_OBJ_FLAG_CLICKABLE);

  // Steps label inside arc
  lv_obj_t* stepsLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(stepsLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsLbl, "%lu", watch.steps);
  lv_obj_align(stepsLbl, LV_ALIGN_CENTER, 0, 80);

  // Tap to open apps (clean, no bounce)
  lv_obj_add_event_cb(scr, [](lv_event_t* e) {
    showScreen(SCREEN_APPS);
  }, LV_EVENT_CLICKED, NULL);

  return scr;
}

void updateClock() {
  if (timeLabel) {
    lv_label_set_text_fmt(timeLabel, "%02d:%02d", watch.hour, watch.minute);
  }
  if (batteryLabel) {
    lv_label_set_text_fmt(batteryLabel, "%d%%%s", watch.batteryPercent, watch.isCharging ? " +" : "");
  }
  if (stepsPreview) {
    int pct = (watch.steps * 100) / watch.stepGoal;
    if (pct > 100) pct = 100;
    lv_arc_set_value(stepsPreview, pct);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME EFFECTS (Static visual elements)
// ═══════════════════════════════════════════════════════════════════════════════

void drawThemeEffects(lv_obj_t* parent, ThemeType theme) {
  currentColors = getThemeColors(theme);
  
  switch(theme) {
    case THEME_YUGO: {
      // Subtle portal rings (static, faint glow)
      for (int i = 0; i < 3; i++) {
        lv_obj_t* ring = lv_obj_create(parent);
        int size = 250 - i * 40;
        lv_obj_set_size(ring, size, size);
        lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(ring, LV_OPA_0, 0);
        lv_obj_set_style_border_color(ring, lv_color_hex(currentColors.accent), 0);
        lv_obj_set_style_border_width(ring, 1, 0);
        lv_obj_set_style_border_opa(ring, LV_OPA_20 + i * 10, 0);
        lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
      
    case THEME_JINWOO: {
      // Shadow layer at bottom (static darkness)
      lv_obj_t* shadow = lv_obj_create(parent);
      lv_obj_set_size(shadow, LCD_WIDTH, 150);
      lv_obj_align(shadow, LV_ALIGN_BOTTOM_MID, 0, 0);
      lv_obj_set_style_bg_color(shadow, lv_color_hex(0x000000), 0);
      lv_obj_set_style_bg_opa(shadow, LV_OPA_50, 0);
      lv_obj_set_style_border_width(shadow, 0, 0);
      lv_obj_clear_flag(shadow, LV_OBJ_FLAG_CLICKABLE);
      break;
    }
      
    case THEME_LUFFY: {
      // Cloud-like shapes (static, soft curves)
      for (int i = 0; i < 4; i++) {
        lv_obj_t* cloud = lv_obj_create(parent);
        lv_obj_set_size(cloud, 80 + i * 20, 40 + i * 10);
        lv_obj_set_pos(cloud, 20 + i * 80, 350 + (i % 2) * 30);
        lv_obj_set_style_radius(cloud, 20, 0);
        lv_obj_set_style_bg_color(cloud, lv_color_hex(currentColors.glow), 0);
        lv_obj_set_style_bg_opa(cloud, LV_OPA_30, 0);
        lv_obj_set_style_border_width(cloud, 0, 0);
        lv_obj_clear_flag(cloud, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
    
    default:
      break;
  }
}

void drawRandomCharacterEffects(lv_obj_t* parent, RandomCharacter character) {
  ThemeColors colors = getRandomCharacterColors(character);
  
  switch(character) {
    case CHAR_NARUTO: {
      // Chakra spiral pattern (static)
      for (int i = 0; i < 3; i++) {
        lv_obj_t* chakra = lv_obj_create(parent);
        int size = 200 - i * 50;
        lv_obj_set_size(chakra, size, size);
        lv_obj_align(chakra, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(chakra, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(chakra, LV_OPA_0, 0);
        lv_obj_set_style_border_color(chakra, lv_color_hex(colors.glow), 0);
        lv_obj_set_style_border_width(chakra, 2, 0);
        lv_obj_set_style_border_opa(chakra, LV_OPA_30, 0);
        lv_obj_clear_flag(chakra, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
      
    case CHAR_GOKU: {
      // Aura glow (static)
      drawPowerAura(parent, colors.glow, 70);
      break;
    }
      
    case CHAR_TANJIRO: {
      // Water/Fire gradient hint
      lv_obj_t* waterLine = lv_obj_create(parent);
      lv_obj_set_size(waterLine, LCD_WIDTH, 3);
      lv_obj_align(waterLine, LV_ALIGN_BOTTOM_MID, 0, -100);
      lv_obj_set_style_bg_color(waterLine, lv_color_hex(colors.secondary), 0);
      lv_obj_set_style_bg_opa(waterLine, LV_OPA_50, 0);
      lv_obj_clear_flag(waterLine, LV_OBJ_FLAG_CLICKABLE);
      break;
    }
      
    case CHAR_GOJO: {
      // Infinity void effect (concentric fading circles)
      for (int i = 0; i < 5; i++) {
        lv_obj_t* void_ring = lv_obj_create(parent);
        int size = 300 - i * 40;
        lv_obj_set_size(void_ring, size, size);
        lv_obj_align(void_ring, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(void_ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(void_ring, LV_OPA_0, 0);
        lv_obj_set_style_border_color(void_ring, lv_color_hex(colors.primary), 0);
        lv_obj_set_style_border_width(void_ring, 1, 0);
        lv_obj_set_style_border_opa(void_ring, LV_OPA_10 + i * 5, 0);
        lv_obj_clear_flag(void_ring, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
      
    case CHAR_LEVI: {
      // Wire lines (ODM gear aesthetic)
      for (int i = 0; i < 4; i++) {
        lv_obj_t* wire = lv_obj_create(parent);
        lv_obj_set_size(wire, 2, 200);
        lv_obj_set_pos(wire, 50 + i * 90, 100);
        lv_obj_set_style_bg_color(wire, lv_color_hex(colors.accent), 0);
        lv_obj_set_style_bg_opa(wire, LV_OPA_20, 0);
        lv_obj_clear_flag(wire, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
      
    case CHAR_SAITAMA: {
      // Simple, clean - minimal effects ("OK" face style)
      // Just subtle highlight
      lv_obj_t* highlight = lv_obj_create(parent);
      lv_obj_set_size(highlight, 60, 60);
      lv_obj_align(highlight, LV_ALIGN_TOP_RIGHT, -40, 80);
      lv_obj_set_style_radius(highlight, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(highlight, lv_color_hex(colors.primary), 0);
      lv_obj_set_style_bg_opa(highlight, LV_OPA_20, 0);
      lv_obj_clear_flag(highlight, LV_OBJ_FLAG_CLICKABLE);
      break;
    }
      
    case CHAR_DEKU: {
      // Lightning sparks pattern (static lines)
      drawActionLines(parent, colors.secondary);
      break;
    }
    
    default:
      break;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  APP GRID SCREENS
// ═══════════════════════════════════════════════════════════════════════════════

typedef struct {
  const char* name;
  uint32_t color;
  ScreenType screen;
} AppInfo;

static const AppInfo apps1[] = {
  {"Steps",    0x4CAF50, SCREEN_STEPS},
  {"Games",    0xE91E63, SCREEN_GAMES},
  {"Music",    0x9C27B0, SCREEN_MUSIC},
  {"Weather",  0x2196F3, SCREEN_WEATHER},
  {"News",     0xFF9800, SCREEN_NEWS},
  {"Quests",   0xFFEB3B, SCREEN_QUESTS},
  {"RPG",      0x673AB7, SCREEN_RPG},
  {"Settings", 0x607D8B, SCREEN_SETTINGS},
  {"Themes",   0x00BCD4, SCREEN_WALLPAPER}
};

static const AppInfo apps2[] = {
  {"Gacha",    0xFF5722, SCREEN_GACHA},
  {"Training", 0x8BC34A, SCREEN_TRAINING},
  {"Boss Rush",0xF44336, SCREEN_BOSS_RUSH}
};

lv_obj_t* createAppsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Apps");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

  // App grid (3x3)
  int btnSize = 100;
  int spacing = 10;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;
  int startY = 60;

  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;

    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + col * (btnSize + spacing), startY + row * (btnSize + spacing));
    lv_obj_set_style_bg_color(btn, lv_color_hex(apps1[i].color), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps1[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(lbl, apps1[i].name);
    lv_obj_center(lbl);
  }

  // Page indicator
  lv_obj_t* pageInd = lv_label_create(scr);
  lv_obj_set_style_text_font(pageInd, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(pageInd, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(pageInd, "1/2 Swipe Down");
  lv_obj_align(pageInd, LV_ALIGN_BOTTOM_MID, 0, -20);

  return scr;
}

lv_obj_t* createApps2Screen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Anime Games");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

  // App buttons
  int btnSize = 100;
  int spacing = 10;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;

  for (int i = 0; i < 3; i++) {
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + i * (btnSize + spacing), 80);
    lv_obj_set_style_bg_color(btn, lv_color_hex(apps2[i].color), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps2[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(lbl, apps2[i].name);
    lv_obj_center(lbl);
  }

  // Page indicator
  lv_obj_t* pageInd = lv_label_create(scr);
  lv_obj_set_style_text_font(pageInd, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(pageInd, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(pageInd, "2/2 Swipe Up");
  lv_obj_align(pageInd, LV_ALIGN_BOTTOM_MID, 0, -20);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STEPS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static lv_obj_t* stepsArc = nullptr;
static lv_obj_t* stepsLabel = nullptr;

lv_obj_t* createStepsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Activity");

  // Big progress arc
  stepsArc = lv_arc_create(scr);
  lv_obj_set_size(stepsArc, 200, 200);
  lv_arc_set_rotation(stepsArc, 135);
  lv_arc_set_bg_angles(stepsArc, 0, 270);
  lv_arc_set_range(stepsArc, 0, 100);
  lv_obj_set_style_arc_width(stepsArc, 15, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(stepsArc, 15, LV_PART_MAIN);
  lv_obj_set_style_arc_color(stepsArc, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(stepsArc, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_align(stepsArc, LV_ALIGN_CENTER, 0, 20);
  lv_obj_clear_flag(stepsArc, LV_OBJ_FLAG_CLICKABLE);

  // Steps count
  stepsLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLabel, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(stepsLabel, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsLabel, "%lu", watch.steps);
  lv_obj_align(stepsLabel, LV_ALIGN_CENTER, 0, 0);

  // Goal label
  lv_obj_t* goalLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(goalLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(goalLbl, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text_fmt(goalLbl, "/ %lu steps", watch.stepGoal);
  lv_obj_align(goalLbl, LV_ALIGN_CENTER, 0, 35);

  // Distance and calories
  float distance = watch.steps * 0.0008;
  int calories = watch.steps * 0.04;

  lv_obj_t* distLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(distLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(distLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(distLabel, "%.1f km", distance);
  lv_obj_align(distLabel, LV_ALIGN_BOTTOM_LEFT, 60, -60);

  lv_obj_t* calLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(calLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(calLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(calLabel, "%d kcal", calories);
  lv_obj_align(calLabel, LV_ALIGN_BOTTOM_RIGHT, -60, -60);

  updateSteps();
  return scr;
}

void updateSteps() {
  if (stepsArc) {
    int pct = (watch.steps * 100) / watch.stepGoal;
    if (pct > 100) pct = 100;
    lv_arc_set_value(stepsArc, pct);
  }
  if (stepsLabel) {
    lv_label_set_text_fmt(stepsLabel, "%lu", watch.steps);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COMMON UI ELEMENTS
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title) {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LCD_WIDTH, 50);
  lv_obj_set_style_bg_color(bar, lv_color_hex(currentColors.secondary), 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);

  // Back button
  lv_obj_t* backBtn = lv_btn_create(bar);
  lv_obj_set_size(backBtn, 40, 35);
  lv_obj_align(backBtn, LV_ALIGN_LEFT_MID, 5, 0);
  lv_obj_set_style_bg_color(backBtn, lv_color_hex(currentColors.primary), 0);
  lv_obj_add_event_cb(backBtn, [](lv_event_t* e) { goBack(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t* backLbl = lv_label_create(backBtn);
  lv_label_set_text(backLbl, LV_SYMBOL_LEFT);
  lv_obj_center(backLbl);

  // Title
  lv_obj_t* titleLbl = lv_label_create(bar);
  lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(titleLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(titleLbl, title);
  lv_obj_align(titleLbl, LV_ALIGN_CENTER, 0, 0);

  return bar;
}

lv_obj_t* createAppButton(lv_obj_t* parent, const char* label, uint32_t color) {
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_set_size(btn, 100, 100);
  lv_obj_set_style_bg_color(btn, lv_color_hex(color), 0);
  lv_obj_set_style_radius(btn, 15, 0);

  lv_obj_t* lbl = lv_label_create(btn);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(lbl, label);
  lv_obj_center(lbl);

  return btn;
}

lv_obj_t* createProgressArc(lv_obj_t* parent, int value, int max) {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* arc = lv_arc_create(parent);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_range(arc, 0, max);
  lv_arc_set_value(arc, value);
  lv_obj_set_style_arc_color(arc, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  return arc;
}

// Placeholder screens
lv_obj_t* createGamesScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "Games");
  
  lv_obj_t* lbl = lv_label_create(scr);
  lv_obj_set_style_text_color(lbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(lbl, "6 Mini-Games Available");
  lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
  return scr;
}

lv_obj_t* createMusicScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "Music");
  return scr;
}

lv_obj_t* createWeatherScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "Weather");
  return scr;
}

lv_obj_t* createNewsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "News");
  return scr;
}

lv_obj_t* createQuestsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "Quests");
  return scr;
}

lv_obj_t* createSettingsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  currentColors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, "Settings");
  return scr;
}
