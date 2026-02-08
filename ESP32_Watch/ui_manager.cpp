
/**
 * UI Manager Implementation
 */

#include "ui_manager.h"
#include "themes.h"
#include "games.h"
#include "apps.h"
#include "wifi_apps.h"
#include "rpg.h"

lv_obj_t* screens[11] = {nullptr};
static ScreenType screenHistory[10];
static int historyIndex = 0;

// Current theme colors
static ThemeColors currentColors;

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════════
void applyTheme(ThemeType theme) {
  currentColors = getThemeColors(theme);
  watch.theme = theme;
}

lv_color_t getThemeColor(int index) {
  switch(index) {
    case 0: return lv_color_hex(currentColors.primary);
    case 1: return lv_color_hex(currentColors.secondary);
    case 2: return lv_color_hex(currentColors.accent);
    case 3: return lv_color_hex(currentColors.background);
    default: return lv_color_hex(currentColors.text);
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
      case SCREEN_CLOCK: screens[screen] = createClockScreen(); break;
      case SCREEN_APPS:  screens[screen] = createAppsScreen(); break;
      case SCREEN_STEPS: screens[screen] = createStepsScreen(); break;
      case SCREEN_GAMES: screens[screen] = createGamesScreen(); break;
      case SCREEN_MUSIC: screens[screen] = createMusicScreen(); break;
      case SCREEN_WEATHER: screens[screen] = createWeatherScreen(); break;
      case SCREEN_NEWS:  screens[screen] = createNewsScreen(); break;
      case SCREEN_QUESTS: screens[screen] = createQuestsScreen(); break;
      case SCREEN_RPG:   screens[screen] = createRPGScreen(); break;
      case SCREEN_SETTINGS: screens[screen] = createSettingsScreen(); break;
      case SCREEN_WALLPAPER: screens[screen] = createWallpaperScreen(); break;
    }
  }

  if (screens[screen]) {
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

lv_obj_t* createClockScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

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

  // Steps preview arc
  stepsPreview = lv_arc_create(scr);
  lv_obj_set_size(stepsPreview, 120, 120);
  lv_arc_set_rotation(stepsPreview, 135);
  lv_arc_set_bg_angles(stepsPreview, 0, 270);
  lv_arc_set_value(stepsPreview, 0);
  lv_obj_set_style_arc_color(stepsPreview, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_align(stepsPreview, LV_ALIGN_CENTER, 0, 80);
  lv_obj_clear_flag(stepsPreview, LV_OBJ_FLAG_CLICKABLE);

  // Steps label inside arc
  lv_obj_t* stepsLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(stepsLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsLbl, "%lu", watch.steps);
  lv_obj_align(stepsLbl, LV_ALIGN_CENTER, 0, 80);

  // Tap to open apps
  lv_obj_add_event_cb(scr, [](lv_event_t* e) {
    showScreen(SCREEN_APPS);
  }, LV_EVENT_CLICKED, NULL);

  return scr;
}

void updateClock() {
  if (timeLabel) {
    lv_label_set_text_fmt(timeLabel, "%02d:%02d", clockHour, clockMinute);
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
//  APP GRID SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
typedef struct {
  const char* name;
  uint32_t color;
  ScreenType screen;
} AppInfo;

static const AppInfo apps[] = {
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
#define NUM_APPS 9

lv_obj_t* createAppsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
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

  for (int i = 0; i < NUM_APPS; i++) {
    int col = i % 3;
    int row = i / 3;

    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + col * (btnSize + spacing), startY + row * (btnSize + spacing));
    lv_obj_set_style_bg_color(btn, lv_color_hex(apps[i].color), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(lbl, apps[i].name);
    lv_obj_center(lbl);
  }

  // Home button
  lv_obj_t* homeBtn = lv_btn_create(scr);
  lv_obj_set_size(homeBtn, 60, 40);
  lv_obj_align(homeBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(homeBtn, lv_color_hex(currentColors.secondary), 0);
  lv_obj_add_event_cb(homeBtn, [](lv_event_t* e) { goHome(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t* homeLbl = lv_label_create(homeBtn);
  lv_label_set_text(homeLbl, LV_SYMBOL_HOME);
  lv_obj_center(homeLbl);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STEPS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
static lv_obj_t* stepsArc = nullptr;
static lv_obj_t* stepsLabel = nullptr;
static lv_obj_t* distLabel = nullptr;
static lv_obj_t* calLabel = nullptr;

lv_obj_t* createStepsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Back button
  lv_obj_t* backBtn = lv_btn_create(scr);
  lv_obj_set_size(backBtn, 50, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_set_style_bg_color(backBtn, lv_color_hex(currentColors.secondary), 0);
  lv_obj_add_event_cb(backBtn, [](lv_event_t* e) { goBack(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t* backLbl = lv_label_create(backBtn);
  lv_label_set_text(backLbl, LV_SYMBOL_LEFT);
  lv_obj_center(backLbl);

  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Activity");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

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
  lv_obj_align(stepsArc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(stepsArc, LV_OBJ_FLAG_CLICKABLE);

  // Steps count
  stepsLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsLabel, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(stepsLabel, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsLabel, "%lu", watch.steps);
  lv_obj_align(stepsLabel, LV_ALIGN_CENTER, 0, -20);

  // Goal label
  lv_obj_t* goalLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(goalLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(goalLbl, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text_fmt(goalLbl, "/ %lu steps", watch.stepGoal);
  lv_obj_align(goalLbl, LV_ALIGN_CENTER, 0, 15);

  // Distance and calories
  float distance = watch.steps * 0.0008; // km
  int calories = watch.steps * 0.04;

  distLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(distLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(distLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(distLabel, "%.1f km", distance);
  lv_obj_align(distLabel, LV_ALIGN_BOTTOM_LEFT, 50, -60);

  calLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(calLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(calLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text_fmt(calLabel, "%d kcal", calories);
  lv_obj_align(calLabel, LV_ALIGN_BOTTOM_RIGHT, -50, -60);

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
  if (distLabel) {
    float distance = watch.steps * 0.0008;
    lv_label_set_text_fmt(distLabel, "%.1f km", distance);
  }
  if (calLabel) {
    int calories = watch.steps * 0.04;
    lv_label_set_text_fmt(calLabel, "%d kcal", calories);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COMMON UI ELEMENTS
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LCD_WIDTH, 50);
  lv_obj_set_style_bg_color(bar, lv_color_hex(currentColors.secondary), 0);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);

  // Back button
  lv_obj_t* backBtn = lv_btn_create(bar);
  lv_obj_set_size(backBtn, 40, 35);
  lv_obj_align(backBtn, LV_ALIGN_LEFT_MID, 5, 0);
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
  lv_obj_t* arc = lv_arc_create(parent);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_range(arc, 0, max);
  lv_arc_set_value(arc, value);
  lv_obj_set_style_arc_color(arc, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  return arc;
}