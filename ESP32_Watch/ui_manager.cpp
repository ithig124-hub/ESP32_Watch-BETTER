/**
 * Premium UI Manager Implementation
 * Apple Watch-Inspired Interface with Anime Themes
 * 
 * Design Features:
 * - OLED-optimized dark backgrounds
 * - Premium watch complications
 * - Smooth 60fps animations
 * - Glassmorphism card effects
 * - SF-style typography hierarchy
 */

#include "ui_manager.h"
#include "themes.h"
#include "power_manager.h"

lv_obj_t* screens[16] = {nullptr};
static ScreenType screenHistory[10];
static int historyIndex = 0;
static ThemeColors currentColors;

// ═══════════════════════════════════════════════════════════════════════════════
//  CLOCK SCREEN - PREMIUM WATCH FACE
// ═══════════════════════════════════════════════════════════════════════════════

static lv_obj_t* timeLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* batteryLabel = nullptr;
static lv_obj_t* batteryIcon = nullptr;
static lv_obj_t* stepsPreview = nullptr;
static lv_obj_t* characterLabel = nullptr;
static lv_obj_t* stepsValueLabel = nullptr;

lv_obj_t* createClockScreen() {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Draw premium background effects
  if (watch.theme == THEME_RANDOM) {
    drawRandomCharacterEffects(scr, watch.dailyCharacter);
  } else {
    drawThemeEffects(scr, watch.theme);
  }

  // ─── STATUS BAR (Top) ───────────────────────────────────────────────────────
  
  // Battery percentage with icon
  batteryLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(batteryLabel, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text_fmt(batteryLabel, "%d%%", watch.batteryPercent);
  lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -15, 12);

  // Theme/Character indicator (top left)
  if (watch.theme == THEME_RANDOM) {
    characterLabel = lv_label_create(scr);
    lv_obj_set_style_text_font(characterLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(characterLabel, lv_color_hex(currentColors.primary), 0);
    lv_label_set_text(characterLabel, getRandomCharacterName(watch.dailyCharacter));
    lv_obj_align(characterLabel, LV_ALIGN_TOP_LEFT, 15, 12);
  } else {
    lv_obj_t* themeLbl = lv_label_create(scr);
    lv_obj_set_style_text_font(themeLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(themeLbl, lv_color_hex(currentColors.primary), 0);
    lv_label_set_text(themeLbl, getThemeName(watch.theme));
    lv_obj_align(themeLbl, LV_ALIGN_TOP_LEFT, 15, 12);
  }

  // ─── MAIN TIME DISPLAY ──────────────────────────────────────────────────────
  
  // Time - Large premium font
  timeLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(timeLabel, lv_color_hex(currentColors.text), 0);
  lv_obj_set_style_text_letter_space(timeLabel, -2, 0);  // Tight kerning
  lv_label_set_text_fmt(timeLabel, "%02d:%02d", watch.hour, watch.minute);
  lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -70);

  // Colon accent glow (subtle)
  lv_obj_t* colonGlow = lv_obj_create(scr);
  lv_obj_set_size(colonGlow, 8, 8);
  lv_obj_align(colonGlow, LV_ALIGN_CENTER, 0, -70);
  lv_obj_set_style_bg_color(colonGlow, lv_color_hex(currentColors.glow), 0);
  lv_obj_set_style_bg_opa(colonGlow, LV_OPA_20, 0);
  lv_obj_set_style_radius(colonGlow, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(colonGlow, 0, 0);
  lv_obj_clear_flag(colonGlow, LV_OBJ_FLAG_CLICKABLE);

  // Date - Elegant secondary text
  dateLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(dateLabel, lv_color_hex(currentColors.textSecondary), 0);
  
  const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  lv_label_set_text_fmt(dateLabel, "%s, %s %d", 
    days[watch.dayOfWeek % 7], 
    months[(watch.month - 1) % 12], 
    watch.day);
  lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, -20);

  // ─── COMPLICATIONS (Bottom) ─────────────────────────────────────────────────
  
  // Steps complication card
  drawGlassmorphismCard(scr, LCD_WIDTH/2 - 75, 280, 150, 100, currentColors.primary);
  
  // Steps progress ring
  stepsPreview = lv_arc_create(scr);
  lv_obj_set_size(stepsPreview, 70, 70);
  lv_arc_set_rotation(stepsPreview, 135);
  lv_arc_set_bg_angles(stepsPreview, 0, 270);
  lv_arc_set_range(stepsPreview, 0, 100);
  lv_obj_set_style_arc_width(stepsPreview, 6, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(stepsPreview, 6, LV_PART_MAIN);
  lv_obj_set_style_arc_color(stepsPreview, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(stepsPreview, lv_color_hex(currentColors.border), LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(stepsPreview, true, LV_PART_INDICATOR);
  lv_obj_align(stepsPreview, LV_ALIGN_CENTER, -40, 95);
  lv_obj_clear_flag(stepsPreview, LV_OBJ_FLAG_CLICKABLE);
  
  int stepPercent = (watch.steps * 100) / watch.stepGoal;
  if (stepPercent > 100) stepPercent = 100;
  lv_arc_set_value(stepsPreview, stepPercent);

  // Steps value inside ring
  stepsValueLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsValueLabel, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(stepsValueLabel, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsValueLabel, "%lu", watch.steps);
  lv_obj_align(stepsValueLabel, LV_ALIGN_CENTER, -40, 95);

  // Steps label
  lv_obj_t* stepsLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(stepsLbl, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text(stepsLbl, "steps");
  lv_obj_align(stepsLbl, LV_ALIGN_CENTER, -40, 115);

  // Goal info
  lv_obj_t* goalLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(goalLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(goalLbl, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text_fmt(goalLbl, "/ %lu goal", watch.stepGoal);
  lv_obj_align(goalLbl, LV_ALIGN_CENTER, 35, 95);

  // Activity stats
  float distance = watch.steps * 0.0008f;
  int calories = watch.steps * 0.04f;
  
  lv_obj_t* distLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(distLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(distLbl, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(distLbl, "%.1f km", distance);
  lv_obj_align(distLbl, LV_ALIGN_CENTER, 35, 75);

  lv_obj_t* calLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(calLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(calLbl, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(calLbl, "%d cal", calories);
  lv_obj_align(calLbl, LV_ALIGN_CENTER, 35, 115);

  // ─── SWIPE HINT ─────────────────────────────────────────────────────────────
  
  lv_obj_t* swipeHint = lv_label_create(scr);
  lv_obj_set_style_text_font(swipeHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(swipeHint, lv_color_hex(currentColors.border), 0);
  lv_label_set_text(swipeHint, "swipe for apps");
  lv_obj_align(swipeHint, LV_ALIGN_BOTTOM_MID, 0, -15);

  return scr;
}

void updateClock() {
  if (timeLabel) {
    lv_label_set_text_fmt(timeLabel, "%02d:%02d", watch.hour, watch.minute);
  }
  if (batteryLabel) {
    lv_label_set_text_fmt(batteryLabel, "%d%%%s", watch.batteryPercent, watch.isCharging ? "+" : "");
  }
  if (stepsPreview) {
    int pct = (watch.steps * 100) / watch.stepGoal;
    if (pct > 100) pct = 100;
    lv_arc_set_value(stepsPreview, pct);
  }
  if (stepsValueLabel) {
    lv_label_set_text_fmt(stepsValueLabel, "%lu", watch.steps);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME EFFECTS - Premium Background Art
// ═══════════════════════════════════════════════════════════════════════════════

void drawThemeEffects(lv_obj_t* parent, ThemeType theme) {
  currentColors = getThemeColors(theme);

  switch(theme) {
    case THEME_LUFFY: {
      // Sun God rays - subtle golden beams
      for (int i = 0; i < 8; i++) {
        lv_obj_t* ray = lv_obj_create(parent);
        lv_obj_set_size(ray, 3, 200);
        lv_obj_align(ray, LV_ALIGN_TOP_MID, 0, 50);
        lv_obj_set_style_transform_angle(ray, i * 450, 0);  // Rotate each ray
        lv_obj_set_style_bg_color(ray, lv_color_hex(currentColors.glow), 0);
        lv_obj_set_style_bg_opa(ray, 13, 0);  // ~5% opacity (LV_OPA_5 not available, use raw value)
        lv_obj_set_style_border_width(ray, 0, 0);
        lv_obj_clear_flag(ray, LV_OBJ_FLAG_CLICKABLE);
      }
      
      // Subtle cloud shapes at bottom
      for (int i = 0; i < 3; i++) {
        lv_obj_t* cloud = lv_obj_create(parent);
        lv_obj_set_size(cloud, 100 + i * 30, 30);
        lv_obj_set_pos(cloud, 30 + i * 100, 400 - i * 15);
        lv_obj_set_style_bg_color(cloud, lv_color_hex(currentColors.glow), 0);
        lv_obj_set_style_bg_opa(cloud, LV_OPA_10, 0);
        lv_obj_set_style_radius(cloud, 15, 0);
        lv_obj_set_style_border_width(cloud, 0, 0);
        lv_obj_clear_flag(cloud, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }

    case THEME_JINWOO: {
      // Shadow void at bottom - multi-layer
      for (int i = 0; i < 3; i++) {
        lv_obj_t* shadow = lv_obj_create(parent);
        lv_obj_set_size(shadow, LCD_WIDTH, 120 - i * 30);
        lv_obj_align(shadow, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(shadow, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(shadow, LV_OPA_30 + i * 15, 0);
        lv_obj_set_style_border_width(shadow, 0, 0);
        lv_obj_clear_flag(shadow, LV_OBJ_FLAG_CLICKABLE);
      }
      
      // Purple glow orbs
      drawAnimePowerAura(parent, currentColors.glow, 50);
      break;
    }

    case THEME_YUGO: {
      // Portal rings - concentric circles
      for (int i = 0; i < 4; i++) {
        lv_obj_t* ring = lv_obj_create(parent);
        int size = 320 - i * 50;
        lv_obj_set_size(ring, size, size);
        lv_obj_align(ring, LV_ALIGN_CENTER, 0, 30);
        lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(ring, LV_OPA_0, 0);
        lv_obj_set_style_border_color(ring, lv_color_hex(currentColors.glow), 0);
        lv_obj_set_style_border_width(ring, 1, 0);
        lv_obj_set_style_border_opa(ring, LV_OPA_10 + i * 5, 0);
        lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }

    default:
      break;
  }
}

void drawRandomCharacterEffects(lv_obj_t* parent, RandomCharacter character) {
  ThemeColors colors = getRandomCharacterColors(character);
  
  // Universal subtle glow
  drawPremiumGlow(parent, colors.glow, 40);
  
  // Character-specific accent lines
  switch(character) {
    case CHAR_NARUTO:
    case CHAR_GOKU: {
      // Energy aura rings
      drawAnimePowerAura(parent, colors.glow, 60);
      break;
    }
    case CHAR_GOJO: {
      // Infinity void effect
      for (int i = 0; i < 5; i++) {
        lv_obj_t* ring = lv_obj_create(parent);
        int size = 350 - i * 50;
        lv_obj_set_size(ring, size, size);
        lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(ring, LV_OPA_0, 0);
        lv_obj_set_style_border_color(ring, lv_color_hex(colors.primary), 0);
        lv_obj_set_style_border_width(ring, 1, 0);
        lv_obj_set_style_border_opa(ring, 13 + i * 3, 0);  // ~5% base opacity
        lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
    case CHAR_LEVI: {
      // ODM wire lines
      for (int i = 0; i < 4; i++) {
        lv_obj_t* wire = lv_obj_create(parent);
        lv_obj_set_size(wire, 1, 300);
        lv_obj_set_pos(wire, 50 + i * 90, 80);
        lv_obj_set_style_bg_color(wire, lv_color_hex(colors.accent), 0);
        lv_obj_set_style_bg_opa(wire, LV_OPA_10, 0);
        lv_obj_clear_flag(wire, LV_OBJ_FLAG_CLICKABLE);
      }
      break;
    }
    default:
      break;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SCREEN NAVIGATION
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

void showScreen(ScreenType screen) {
  if (historyIndex < 9) {
    screenHistory[historyIndex++] = watch.screen;
  }

  watch.screen = screen;

  if (!screens[screen]) {
    switch(screen) {
      case SCREEN_CLOCK:      screens[screen] = createClockScreen(); break;
      case SCREEN_APPS:       screens[screen] = createAppsScreen(); break;
      case SCREEN_APPS2:      screens[screen] = createApps2Screen(); break;
      case SCREEN_STEPS:      screens[screen] = createStepsScreen(); break;
      case SCREEN_WALLPAPER:  screens[screen] = createWallpaperScreen(); break;
      case SCREEN_SETTINGS:   screens[screen] = createSettingsScreen(); break;
      // Placeholder screens
      case SCREEN_GAMES:      screens[screen] = createGamesScreen(); break;
      case SCREEN_MUSIC:      screens[screen] = createMusicScreen(); break;
      case SCREEN_WEATHER:    screens[screen] = createWeatherScreen(); break;
      case SCREEN_NEWS:       screens[screen] = createNewsScreen(); break;
      case SCREEN_QUESTS:     screens[screen] = createQuestsScreen(); break;
      case SCREEN_RPG:        screens[screen] = createRPGScreen(); break;
      case SCREEN_CHAR_STATS: screens[screen] = createCharStatsScreen(); break;
      case SCREEN_GACHA:      screens[screen] = createGachaScreen(); break;
      case SCREEN_TRAINING:   screens[screen] = createTrainingScreen(); break;
      case SCREEN_BOSS_RUSH:  screens[screen] = createBossRushScreen(); break;
    }
  }

  if (screens[screen]) {
    lv_scr_load_anim(screens[screen], LV_SCR_LOAD_ANIM_FADE_IN, 150, 0, false);
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
//  APP GRID - PREMIUM DESIGN
// ═══════════════════════════════════════════════════════════════════════════════

typedef struct {
  const char* name;
  const char* icon;
  uint32_t color;
  ScreenType screen;
} AppInfo;

static const AppInfo apps1[] = {
  {"Steps",    LV_SYMBOL_LOOP,     0x32CD32, SCREEN_STEPS},
  {"Games",    LV_SYMBOL_PLAY,     0xE91E63, SCREEN_GAMES},
  {"Music",    LV_SYMBOL_AUDIO,    0x9C27B0, SCREEN_MUSIC},
  {"Weather",  LV_SYMBOL_IMAGE,    0x2196F3, SCREEN_WEATHER},
  {"News",     LV_SYMBOL_LIST,     0xFF9800, SCREEN_NEWS},
  {"Quests",   LV_SYMBOL_GPS,      0xFFEB3B, SCREEN_QUESTS},
  {"RPG",      LV_SYMBOL_HOME,     0x673AB7, SCREEN_RPG},
  {"Settings", LV_SYMBOL_SETTINGS, 0x607D8B, SCREEN_SETTINGS},
  {"Themes",   LV_SYMBOL_TINT,     0x00BCD4, SCREEN_WALLPAPER}
};

static const AppInfo apps2[] = {
  {"Gacha",    LV_SYMBOL_SHUFFLE,   0xFF5722, SCREEN_GACHA},
  {"Training", LV_SYMBOL_CHARGE,    0x8BC34A, SCREEN_TRAINING},
  {"Boss",     LV_SYMBOL_WARNING,   0xF44336, SCREEN_BOSS_RUSH}
};

lv_obj_t* createAppsScreen() {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Apps");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

  // App grid (3x3) with premium styling
  int btnSize = 95;
  int spacing = 12;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;
  int startY = 50;

  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;

    // Button with glassmorphism
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + col * (btnSize + spacing), startY + row * (btnSize + spacing));
    lv_obj_set_style_bg_color(btn, lv_color_hex(currentColors.surface), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_90, 0);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(apps1[i].color), 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_60, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps1[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    // Icon
    lv_obj_t* icon = lv_label_create(btn);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(apps1[i].color), 0);
    lv_label_set_text(icon, apps1[i].icon);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -10);

    // Label
    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(currentColors.text), 0);
    lv_label_set_text(lbl, apps1[i].name);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 20);
  }

  // Page indicator dots
  for (int i = 0; i < 2; i++) {
    lv_obj_t* dot = lv_obj_create(scr);
    lv_obj_set_size(dot, i == 0 ? 8 : 6, i == 0 ? 8 : 6);
    lv_obj_align(dot, LV_ALIGN_BOTTOM_MID, -8 + i * 16, -20);
    lv_obj_set_style_bg_color(dot, lv_color_hex(i == 0 ? currentColors.primary : currentColors.border), 0);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
  }

  return scr;
}

lv_obj_t* createApps2Screen() {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Anime Games");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

  int btnSize = 95;
  int spacing = 12;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;

  for (int i = 0; i < 3; i++) {
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + i * (btnSize + spacing), 80);
    lv_obj_set_style_bg_color(btn, lv_color_hex(currentColors.surface), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_90, 0);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(apps2[i].color), 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_60, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps2[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* icon = lv_label_create(btn);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(apps2[i].color), 0);
    lv_label_set_text(icon, apps2[i].icon);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(currentColors.text), 0);
    lv_label_set_text(lbl, apps2[i].name);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 20);
  }

  // Page indicator
  for (int i = 0; i < 2; i++) {
    lv_obj_t* dot = lv_obj_create(scr);
    lv_obj_set_size(dot, i == 1 ? 8 : 6, i == 1 ? 8 : 6);
    lv_obj_align(dot, LV_ALIGN_BOTTOM_MID, -8 + i * 16, -20);
    lv_obj_set_style_bg_color(dot, lv_color_hex(i == 1 ? currentColors.primary : currentColors.border), 0);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
  }

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COMMON UI ELEMENTS
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title) {
  currentColors = getThemeColors(watch.theme);

  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LCD_WIDTH, 45);
  lv_obj_set_style_bg_color(bar, lv_color_hex(currentColors.surface), 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_80, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);

  // Back button
  lv_obj_t* backBtn = lv_btn_create(bar);
  lv_obj_set_size(backBtn, 35, 30);
  lv_obj_align(backBtn, LV_ALIGN_LEFT_MID, 8, 0);
  lv_obj_set_style_bg_color(backBtn, lv_color_hex(currentColors.primary), 0);
  lv_obj_set_style_bg_opa(backBtn, LV_OPA_20, 0);
  lv_obj_set_style_radius(backBtn, 8, 0);
  lv_obj_add_event_cb(backBtn, [](lv_event_t* e) { goBack(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t* backLbl = lv_label_create(backBtn);
  lv_obj_set_style_text_color(backLbl, lv_color_hex(currentColors.primary), 0);
  lv_label_set_text(backLbl, LV_SYMBOL_LEFT);
  lv_obj_center(backLbl);

  // Title
  lv_obj_t* titleLbl = lv_label_create(bar);
  lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(titleLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(titleLbl, title);
  lv_obj_align(titleLbl, LV_ALIGN_CENTER, 0, 0);

  return bar;
}

lv_obj_t* createAppButton(lv_obj_t* parent, const char* label, uint32_t color) {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_set_size(btn, 95, 95);
  lv_obj_set_style_bg_color(btn, lv_color_hex(currentColors.surface), 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_90, 0);
  lv_obj_set_style_radius(btn, 20, 0);
  lv_obj_set_style_border_color(btn, lv_color_hex(color), 0);
  lv_obj_set_style_border_width(btn, 2, 0);

  lv_obj_t* lbl = lv_label_create(btn);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(lbl, lv_color_hex(currentColors.text), 0);
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
  lv_obj_set_style_arc_color(arc, lv_color_hex(currentColors.border), LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  return arc;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STEPS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

static lv_obj_t* stepsArc = nullptr;
static lv_obj_t* stepsLabel = nullptr;

lv_obj_t* createStepsScreen() {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Activity");

  // Main progress arc
  stepsArc = lv_arc_create(scr);
  lv_obj_set_size(stepsArc, 200, 200);
  lv_arc_set_rotation(stepsArc, 135);
  lv_arc_set_bg_angles(stepsArc, 0, 270);
  lv_arc_set_range(stepsArc, 0, 100);
  lv_obj_set_style_arc_width(stepsArc, 12, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(stepsArc, 12, LV_PART_MAIN);
  lv_obj_set_style_arc_color(stepsArc, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(stepsArc, lv_color_hex(currentColors.border), LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(stepsArc, true, LV_PART_INDICATOR);
  lv_obj_align(stepsArc, LV_ALIGN_CENTER, 0, 30);
  lv_obj_clear_flag(stepsArc, LV_OBJ_FLAG_CLICKABLE);

  int pct = (watch.steps * 100) / watch.stepGoal;
  if (pct > 100) pct = 100;
  lv_arc_set_value(stepsArc, pct);

  // Steps count
  stepsLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLabel, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(stepsLabel, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsLabel, "%lu", watch.steps);
  lv_obj_align(stepsLabel, LV_ALIGN_CENTER, 0, 10);

  // Goal label
  lv_obj_t* goalLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(goalLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(goalLbl, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text_fmt(goalLbl, "/ %lu steps", watch.stepGoal);
  lv_obj_align(goalLbl, LV_ALIGN_CENTER, 0, 50);

  // Stats row
  float distance = watch.steps * 0.0008f;
  int calories = watch.steps * 0.04f;

  lv_obj_t* distLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(distLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(distLbl, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(distLbl, "%.1f km", distance);
  lv_obj_align(distLbl, LV_ALIGN_BOTTOM_LEFT, 60, -50);

  lv_obj_t* calLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(calLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(calLbl, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(calLbl, "%d kcal", calories);
  lv_obj_align(calLbl, LV_ALIGN_BOTTOM_RIGHT, -60, -50);

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
//  SETTINGS SCREEN - Power Management Settings
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createSettingsScreen() {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Settings");

  // Screen timeout option
  lv_obj_t* timeoutCard = lv_obj_create(scr);
  lv_obj_set_size(timeoutCard, LCD_WIDTH - 30, 70);
  lv_obj_align(timeoutCard, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(timeoutCard, lv_color_hex(currentColors.surface), 0);
  lv_obj_set_style_bg_opa(timeoutCard, LV_OPA_80, 0);
  lv_obj_set_style_radius(timeoutCard, 16, 0);
  lv_obj_set_style_border_width(timeoutCard, 0, 0);

  lv_obj_t* timeoutLbl = lv_label_create(timeoutCard);
  lv_obj_set_style_text_font(timeoutLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(timeoutLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(timeoutLbl, "Screen Timeout");
  lv_obj_align(timeoutLbl, LV_ALIGN_LEFT_MID, 15, -10);

  lv_obj_t* timeoutVal = lv_label_create(timeoutCard);
  lv_obj_set_style_text_font(timeoutVal, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(timeoutVal, lv_color_hex(currentColors.primary), 0);
  lv_label_set_text(timeoutVal, "3 seconds");
  lv_obj_align(timeoutVal, LV_ALIGN_LEFT_MID, 15, 10);

  // Battery saver option
  lv_obj_t* saverCard = lv_obj_create(scr);
  lv_obj_set_size(saverCard, LCD_WIDTH - 30, 70);
  lv_obj_align(saverCard, LV_ALIGN_TOP_MID, 0, 140);
  lv_obj_set_style_bg_color(saverCard, lv_color_hex(currentColors.surface), 0);
  lv_obj_set_style_bg_opa(saverCard, LV_OPA_80, 0);
  lv_obj_set_style_radius(saverCard, 16, 0);
  lv_obj_set_style_border_width(saverCard, 0, 0);

  lv_obj_t* saverLbl = lv_label_create(saverCard);
  lv_obj_set_style_text_font(saverLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(saverLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(saverLbl, "Battery Saver");
  lv_obj_align(saverLbl, LV_ALIGN_LEFT_MID, 15, -10);

  lv_obj_t* saverVal = lv_label_create(saverCard);
  lv_obj_set_style_text_font(saverVal, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(saverVal, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text(saverVal, "Off");
  lv_obj_align(saverVal, LV_ALIGN_LEFT_MID, 15, 10);

  // About info
  lv_obj_t* aboutLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(aboutLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(aboutLbl, lv_color_hex(currentColors.textSecondary), 0);
  lv_label_set_text(aboutLbl, "ESP32 Watch Premium\nAnime Edition v1.0");
  lv_obj_set_style_text_align(aboutLbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(aboutLbl, LV_ALIGN_BOTTOM_MID, 0, -30);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PLACEHOLDER SCREENS
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createPlaceholderScreen(const char* title, const char* message) {
  currentColors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  createTitleBar(scr, title);

  lv_obj_t* lbl = lv_label_create(scr);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lbl, lv_color_hex(currentColors.textSecondary), 0);
  lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(lbl, message);
  lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 20);

  return scr;
}

lv_obj_t* createGamesScreen() { return createPlaceholderScreen("Games", "6 Mini-Games\nAvailable"); }
lv_obj_t* createMusicScreen() { return createPlaceholderScreen("Music", "SD Card\nMusic Player"); }
lv_obj_t* createWeatherScreen() { return createPlaceholderScreen("Weather", "Connect WiFi\nfor weather"); }
lv_obj_t* createNewsScreen() { return createPlaceholderScreen("News", "Connect WiFi\nfor news"); }
lv_obj_t* createQuestsScreen() { return createPlaceholderScreen("Quests", "Daily challenges\ncoming soon"); }
// createRPGScreen() - defined in rpg.cpp
// createCharStatsScreen() - defined in rpg.cpp
// createGachaScreen() - defined in games.cpp
// createTrainingScreen() - defined in games.cpp
// createBossRushScreen() - defined in games.cpp
