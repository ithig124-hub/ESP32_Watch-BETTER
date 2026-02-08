/**
 * UI Manager Implementation - Updated with all screens
 */

#include "ui_manager.h"
#include "themes.h"
#include "games.h"
#include "apps.h"
#include "wifi_apps.h"
#include "rpg.h"
#include "utilities.h"

lv_obj_t* screens[SCREEN_COUNT] = {nullptr};
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
      case SCREEN_CLOCK:      screens[screen] = createClockScreen(); break;
      case SCREEN_APPS:       screens[screen] = createAppsScreen(); break;
      case SCREEN_APPS2:      screens[screen] = createApps2Screen(); break;
      case SCREEN_CHAR_STATS: screens[screen] = createCharStatsScreen(); break;
      case SCREEN_STEPS:      screens[screen] = createStepsScreen(); break;
      case SCREEN_GAMES:      screens[screen] = createGamesScreen(); break;
      case SCREEN_MUSIC:      screens[screen] = createMusicScreen(); break;
      case SCREEN_WEATHER:    screens[screen] = createWeatherScreen(); break;
      case SCREEN_NEWS:       screens[screen] = createNewsScreen(); break;
      case SCREEN_QUESTS:     screens[screen] = createQuestsScreen(); break;
      case SCREEN_RPG:        screens[screen] = createRPGScreen(); break;
      case SCREEN_SETTINGS:   screens[screen] = createSettingsScreen(); break;
      case SCREEN_WALLPAPER:  screens[screen] = createWallpaperScreen(); break;
      case SCREEN_CALCULATOR: screens[screen] = Calculator::createScreen(); break;
      case SCREEN_FLASHLIGHT: screens[screen] = Flashlight::createScreen(); break;
      case SCREEN_COIN_FLIP:  screens[screen] = CoinFlip::createScreen(); break;
      case SCREEN_STOPWATCH:  screens[screen] = Stopwatch::createScreen(); break;
      case SCREEN_FILE_BROWSER: screens[screen] = createFileBrowserScreen(); break;
      default: break;
    }
  }

  if (screens[screen]) {
    // Use slide animation based on navigation direction
    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_FADE_IN;
    lv_scr_load_anim(screens[screen], anim, 200, 0, false);
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
  currentNavCategory = NAV_CLOCK;
  currentSubCard = 0;
  showScreen(SCREEN_CLOCK);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CLOCK SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
static lv_obj_t* timeLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* batteryLabel = nullptr;
static lv_obj_t* stepsPreview = nullptr;
static lv_obj_t* stepsPreviewLbl = nullptr;

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
  stepsPreviewLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(stepsPreviewLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(stepsPreviewLbl, lv_color_hex(currentColors.text), 0);
  lv_label_set_text_fmt(stepsPreviewLbl, "%lu", watch.steps);
  lv_obj_align(stepsPreviewLbl, LV_ALIGN_CENTER, 0, 80);

  // Swipe indicator hint
  lv_obj_t* swipeHint = lv_label_create(scr);
  lv_obj_set_style_text_font(swipeHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(swipeHint, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(swipeHint, "< Swipe to navigate >");
  lv_obj_align(swipeHint, LV_ALIGN_BOTTOM_MID, 0, -20);

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
  if (stepsPreviewLbl) {
    lv_label_set_text_fmt(stepsPreviewLbl, "%lu", watch.steps);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  APP GRID SCREEN (Expanded with utility apps)
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
#define APPS_PER_PAGE 9

lv_obj_t* createAppsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Title with page indicator
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Apps (Page 1)");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  // App grid (3x3) - First 9 apps
  int btnSize = 95;
  int spacing = 8;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;
  int startY = 50;
  
  int displayApps = min(APPS_PER_PAGE, NUM_APPS);

  for (int i = 0; i < displayApps; i++) {
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

  // Swipe down hint for Page 2
  lv_obj_t* downHint = lv_label_create(scr);
  lv_obj_set_style_text_font(downHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(downHint, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text(downHint, LV_SYMBOL_DOWN " Swipe down for more apps");
  lv_obj_align(downHint, LV_ALIGN_BOTTOM_MID, 0, -40);
  
  // Navigation hint
  lv_obj_t* navHint = lv_label_create(scr);
  lv_obj_set_style_text_font(navHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(navHint, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(navHint, "< Swipe L/R for other screens >");
  lv_obj_align(navHint, LV_ALIGN_BOTTOM_MID, 0, -15);

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
//  FILE BROWSER SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createFileBrowserScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  createTitleBar(scr, "Files");

  // SD Card status
  lv_obj_t* statusLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(statusLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(statusLbl, 
    lv_color_hex(hasSD ? 0x4CAF50 : 0xFF5722), 0);
  lv_label_set_text(statusLbl, hasSD ? "SD Card: Connected" : "SD Card: Not Found");
  lv_obj_align(statusLbl, LV_ALIGN_TOP_MID, 0, 60);

  if (hasSD) {
    // File list placeholder
    lv_obj_t* listBg = lv_obj_create(scr);
    lv_obj_set_size(listBg, LCD_WIDTH - 20, 280);
    lv_obj_align(listBg, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(listBg, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(listBg, 10, 0);
    lv_obj_set_scrollbar_mode(listBg, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t* infoLbl = lv_label_create(listBg);
    lv_obj_set_style_text_color(infoLbl, lv_color_hex(currentColors.text), 0);
    lv_label_set_text(infoLbl, "Music & PDF files\nwill appear here");
    lv_obj_center(infoLbl);
  } else {
    lv_obj_t* hintLbl = lv_label_create(scr);
    lv_obj_set_style_text_color(hintLbl, lv_color_hex(currentColors.secondary), 0);
    lv_label_set_text(hintLbl, "Insert SD card to\nbrowse files");
    lv_obj_align(hintLbl, LV_ALIGN_CENTER, 0, 0);
  }

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COMMON UI ELEMENTS
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createTitleBar(lv_obj_t* parent, const char* title) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LCD_WIDTH, 50);
  lv_obj_set_style_bg_color(bar, lv_color_hex(currentColors.secondary), 0);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);

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

// ═══════════════════════════════════════════════════════════════════════════════
//  APP GRID 2 SCREEN (Second page - accessed via swipe down from Apps1)
// ═══════════════════════════════════════════════════════════════════════════════
static const AppInfo apps2[] = {
  {"Files",    0x795548, SCREEN_FILE_BROWSER},
  {"Coin",     0xCDDC39, SCREEN_COIN_FLIP},
  {"Light",    0xFFC107, SCREEN_FLASHLIGHT},
  {"Timer",    0x009688, SCREEN_STOPWATCH},
  {"Calc",     0xFF5722, SCREEN_CALCULATOR}
};
#define NUM_APPS2 5

lv_obj_t* createApps2Screen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);

  // Title with page indicator
  lv_obj_t* title = lv_label_create(scr);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(currentColors.text), 0);
  lv_label_set_text(title, "Apps (Page 2)");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  
  // Swipe up hint
  lv_obj_t* upHint = lv_label_create(scr);
  lv_obj_set_style_text_font(upHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(upHint, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text(upHint, LV_SYMBOL_UP " Swipe up for Page 1");
  lv_obj_align(upHint, LV_ALIGN_TOP_MID, 0, 35);

  // App grid (3x3 layout but fewer apps)
  int btnSize = 95;
  int spacing = 8;
  int startX = (LCD_WIDTH - (3 * btnSize + 2 * spacing)) / 2;
  int startY = 60;

  for (int i = 0; i < NUM_APPS2; i++) {
    int col = i % 3;
    int row = i / 3;

    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, btnSize, btnSize);
    lv_obj_set_pos(btn, startX + col * (btnSize + spacing), startY + row * (btnSize + spacing));
    lv_obj_set_style_bg_color(btn, lv_color_hex(apps2[i].color), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)apps2[i].screen);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ScreenType scr = (ScreenType)(intptr_t)lv_obj_get_user_data(btn);
      showScreen(scr);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(lbl, apps2[i].name);
    lv_obj_center(lbl);
  }

  // Navigation hints at bottom
  lv_obj_t* navHint = lv_label_create(scr);
  lv_obj_set_style_text_font(navHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(navHint, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(navHint, "< Swipe L/R for other screens >");
  lv_obj_align(navHint, LV_ALIGN_BOTTOM_MID, 0, -20);

  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER STATS SCREEN (Theme-specific stats)
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createCharStatsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(currentColors.background), 0);
  
  // Get character info based on current theme
  const char* charName = "Unknown";
  const char* charTitle = "Adventurer";
  const char* specialAbility = "None";
  uint32_t accentColor = currentColors.accent;
  
  switch (watch.theme) {
    case THEME_LUFFY:
      charName = "Monkey D. Luffy";
      charTitle = "Sun God Nika";
      specialAbility = "Gear 5 Awakened";
      break;
    case THEME_JINWOO:
      charName = "Sung Jin-Woo";
      charTitle = "Shadow Monarch";
      specialAbility = "Arise!";
      break;
    case THEME_YUGO:
      charName = "Yugo";
      charTitle = "Portal Master";
      specialAbility = "Wakfu Portals";
      break;
  }
  
  // Character name (large)
  lv_obj_t* nameLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(nameLabel, lv_color_hex(currentColors.primary), 0);
  lv_label_set_text(nameLabel, charName);
  lv_obj_align(nameLabel, LV_ALIGN_TOP_MID, 0, 30);
  
  // Title
  lv_obj_t* titleLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(titleLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text(titleLabel, charTitle);
  lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 60);
  
  // Stats panel
  lv_obj_t* statsPanel = lv_obj_create(scr);
  lv_obj_set_size(statsPanel, LCD_WIDTH - 30, 200);
  lv_obj_align(statsPanel, LV_ALIGN_CENTER, 0, 30);
  lv_obj_set_style_bg_color(statsPanel, lv_color_hex(0x222222), 0);
  lv_obj_set_style_radius(statsPanel, 15, 0);
  lv_obj_set_style_border_color(statsPanel, lv_color_hex(currentColors.primary), 0);
  lv_obj_set_style_border_width(statsPanel, 2, 0);
  lv_obj_set_scrollbar_mode(statsPanel, LV_SCROLLBAR_MODE_OFF);
  
  // Stats with progress bars
  const char* statNames[] = {"STR", "SPD", "MAG", "END"};
  int statValues[] = {85, 90, 75, 80};  // Base values, can be made dynamic
  
  // Adjust stats per character
  if (watch.theme == THEME_LUFFY) {
    statValues[0] = 95; statValues[1] = 85; statValues[2] = 60; statValues[3] = 100;
  } else if (watch.theme == THEME_JINWOO) {
    statValues[0] = 90; statValues[1] = 95; statValues[2] = 100; statValues[3] = 85;
  } else if (watch.theme == THEME_YUGO) {
    statValues[0] = 70; statValues[1] = 100; statValues[2] = 95; statValues[3] = 75;
  }
  
  for (int i = 0; i < 4; i++) {
    int yPos = 15 + i * 45;
    
    // Stat name
    lv_obj_t* statLbl = lv_label_create(statsPanel);
    lv_obj_set_style_text_font(statLbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(statLbl, lv_color_hex(currentColors.text), 0);
    lv_label_set_text(statLbl, statNames[i]);
    lv_obj_set_pos(statLbl, 10, yPos);
    
    // Progress bar
    lv_obj_t* bar = lv_bar_create(statsPanel);
    lv_obj_set_size(bar, 180, 20);
    lv_obj_set_pos(bar, 60, yPos);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, statValues[i], LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(currentColors.primary), LV_PART_INDICATOR);
    
    // Value label
    lv_obj_t* valLbl = lv_label_create(statsPanel);
    lv_obj_set_style_text_font(valLbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(valLbl, lv_color_hex(currentColors.accent), 0);
    lv_label_set_text_fmt(valLbl, "%d", statValues[i]);
    lv_obj_set_pos(valLbl, 250, yPos);
  }
  
  // Special ability
  lv_obj_t* abilityTitle = lv_label_create(scr);
  lv_obj_set_style_text_font(abilityTitle, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(abilityTitle, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(abilityTitle, "Special Ability:");
  lv_obj_align(abilityTitle, LV_ALIGN_BOTTOM_MID, 0, -70);
  
  lv_obj_t* abilityLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(abilityLabel, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(abilityLabel, lv_color_hex(currentColors.accent), 0);
  lv_label_set_text(abilityLabel, specialAbility);
  lv_obj_align(abilityLabel, LV_ALIGN_BOTTOM_MID, 0, -45);
  
  // Navigation hint
  lv_obj_t* navHint = lv_label_create(scr);
  lv_obj_set_style_text_font(navHint, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(navHint, lv_color_hex(currentColors.secondary), 0);
  lv_label_set_text(navHint, "< Swipe to navigate >");
  lv_obj_align(navHint, LV_ALIGN_BOTTOM_MID, 0, -15);
  
  return scr;
}

