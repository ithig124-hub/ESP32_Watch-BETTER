/*
 * apps.cpp - Application Implementation
 * Music, Notes, Files, Settings, Stopwatch, Wallpaper
 */

#include "apps.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "hardware.h"
#include "filesystem.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

int current_app_index = 0;
int num_registered_apps = 10;

// Stopwatch state
static bool stopwatch_mode = true;  // true = stopwatch, false = timer
static unsigned long lap_times[10];
static int lap_count = 0;

// Calculator state
static float calc_result = 0;
static String calc_display = "0";
static char calc_operator = 0;
static float calc_operand = 0;

// Flashlight state
static bool flashlight_on = false;

// Music state
static int current_track = 0;
static bool is_playing = false;

// App registry
WatchApp registered_apps[] = {
  {APP_WATCHFACE, "Watch", "", COLOR_WHITE, true, nullptr, nullptr, nullptr, nullptr},
  {APP_MUSIC, "Music", "", COLOR_GREEN, true, initMusicApp, drawMusicApp, handleMusicTouch, nullptr},
  {APP_NOTES, "Notes", "", COLOR_YELLOW, true, initNotesApp, drawNotesApp, handleNotesTouch, nullptr},
  {APP_FILES, "Files", "", COLOR_BLUE, true, initFileBrowserApp, drawFileBrowserApp, handleFileBrowserTouch, nullptr},
  {APP_SETTINGS, "Settings", "", COLOR_GRAY, true, initSettingsApp, drawSettingsApp, handleSettingsTouch, nullptr},
  {APP_WEATHER, "Weather", "", COLOR_CYAN, true, nullptr, nullptr, nullptr, nullptr},
  {APP_GAMES, "Games", "", COLOR_ORANGE, true, nullptr, nullptr, nullptr, nullptr},
  {APP_CALCULATOR, "Calc", "", COLOR_PURPLE, true, initCalculatorApp, drawCalculatorApp, handleCalculatorTouch, nullptr},
  {APP_FLASHLIGHT, "Light", "", COLOR_WHITE, true, initFlashlightApp, drawFlashlightApp, handleFlashlightTouch, nullptr},
  {APP_WALLPAPER_SELECTOR, "Wallpaper", "", COLOR_PINK, true, initWallpaperSelector, drawWallpaperSelector, handleWallpaperTouch, nullptr}
};

// =============================================================================
// APP MANAGEMENT
// =============================================================================

void initializeApps() {
  Serial.println("[Apps] Initializing app framework...");
}

void launchApp(AppType app) {
  system_state.current_app = app;
  for (int i = 0; i < num_registered_apps; i++) {
    if (registered_apps[i].type == app && registered_apps[i].init_func) {
      registered_apps[i].init_func();
      break;
    }
  }
}

void exitCurrentApp() {
  system_state.current_screen = SCREEN_APP_GRID;
}

void drawAppGrid() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(140, 10);
  gfx->print("APPS");
  
  // Draw app icons in a grid
  for (int i = 0; i < min(num_registered_apps, 9); i++) {
    int x = (i % 3) * 115 + 25;
    int y = (i / 3) * 115 + 60;
    drawAppIcon(x, y, 100, registered_apps[i]);
  }
}

void drawAppIcon(int x, int y, int size, WatchApp& app) {
  gfx->fillRoundRect(x, y, size, size, 20, app.icon_color);
  gfx->setTextColor(COLOR_BLACK);
  gfx->setTextSize(1);
  int textW = app.name.length() * 6;
  gfx->setCursor(x + (size - textW) / 2, y + size / 2 - 4);
  gfx->print(app.name);
}

void handleAppGridTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  for (int i = 0; i < min(num_registered_apps, 9); i++) {
    int bx = (i % 3) * 115 + 25;
    int by = (i / 3) * 115 + 60;
    if (x >= bx && x < bx + 100 && y >= by && y < by + 100) {
      launchApp(registered_apps[i].type);
      return;
    }
  }
}

void switchToApp(AppType app) {
  launchApp(app);
}

// =============================================================================
// MUSIC PLAYER
// =============================================================================

void initMusicApp() {
  system_state.current_screen = SCREEN_MUSIC;
}

void drawMusicApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(120, 20);
  gfx->print("MUSIC");
  
  // Album art placeholder
  gfx->fillRoundRect(100, 80, 160, 160, 20, getCurrentTheme()->shadow);
  
  // Track info
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(80, 270);
  gfx->printf("Track %d", current_track + 1);
  
  gfx->setTextSize(1);
  gfx->setCursor(100, 300);
  gfx->print(is_playing ? "Playing" : "Paused");
  
  // Controls
  drawThemeButton(30, 350, 80, 50, "Prev", false);
  drawThemeButton(140, 350, 80, 50, is_playing ? "Pause" : "Play", false);
  drawThemeButton(250, 350, 80, 50, "Next", false);
  
  // Back button
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleMusicTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  if (y >= 350 && y < 400) {
    if (x < 110) previousTrack();
    else if (x < 220) { is_playing ? pauseMusic() : playMusic(); }
    else nextTrack();
  }
  
  if (y >= 420) exitCurrentApp();
}

void playMusic() { is_playing = true; system_state.music_playing = true; }
void pauseMusic() { is_playing = false; system_state.music_playing = false; }
void nextTrack() { current_track = (current_track + 1) % max(1, system_state.total_mp3_files); }
void previousTrack() { current_track = (current_track - 1 + max(1, system_state.total_mp3_files)) % max(1, system_state.total_mp3_files); }

// =============================================================================
// NOTES APP
// =============================================================================

static String notes[5] = {"Note 1", "Note 2", "Note 3", "Note 4", "Note 5"};
static int selected_note = 0;

void initNotesApp() {
  system_state.current_screen = SCREEN_NOTES;
}

void drawNotesApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(130, 20);
  gfx->print("NOTES");
  
  for (int i = 0; i < 5; i++) {
    int y = 70 + i * 60;
    uint16_t bg = (i == selected_note) ? getCurrentTheme()->shadow : RGB565(40, 40, 40);
    gfx->fillRoundRect(20, y, 320, 50, 10, bg);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(30, y + 18);
    gfx->print(notes[i]);
  }
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleNotesTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int y = gesture.y;
  
  for (int i = 0; i < 5; i++) {
    int ny = 70 + i * 60;
    if (y >= ny && y < ny + 50) {
      selected_note = i;
      return;
    }
  }
  
  if (y >= 420) exitCurrentApp();
}

void saveNote() {}
void loadNote() {}

// =============================================================================
// FILE BROWSER
// =============================================================================

void initFileBrowserApp() {
  system_state.current_screen = SCREEN_FILE_BROWSER;
}

void drawFileBrowserApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(120, 20);
  gfx->print("FILES");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 80);
  gfx->print("SD Card Status: ");
  gfx->print(system_state.filesystem_available ? "Available" : "Not Found");
  
  gfx->setCursor(20, 110);
  gfx->printf("MP3 Files: %d", system_state.total_mp3_files);
  gfx->setCursor(20, 130);
  gfx->printf("PDF Files: %d", system_state.total_pdf_files);
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleFileBrowserTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 420) exitCurrentApp();
}

// =============================================================================
// PDF READER
// =============================================================================

void initPDFReaderApp() {
  system_state.current_screen = SCREEN_PDF_READER;
}

void drawPDFReaderApp() {
  gfx->fillScreen(COLOR_BLACK);
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 20);
  gfx->print("PDF READER");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(50, 200);
  gfx->print("No PDF loaded");
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handlePDFReaderTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 420) exitCurrentApp();
}

// =============================================================================
// SETTINGS
// =============================================================================

void initSettingsApp() {
  system_state.current_screen = SCREEN_SETTINGS;
}

void drawSettingsApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(110, 20);
  gfx->print("SETTINGS");
  
  // Settings options
  const char* options[] = {"Brightness", "Theme", "WiFi", "About"};
  for (int i = 0; i < 4; i++) {
    int y = 80 + i * 70;
    gfx->fillRoundRect(20, y, 320, 60, 10, RGB565(40, 40, 40));
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(30, y + 20);
    gfx->print(options[i]);
  }
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleSettingsTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int y = gesture.y;
  
  if (y >= 80 && y < 140) {
    // Brightness
    system_state.brightness = (system_state.brightness + 50) % 256;
    setDisplayBrightness(system_state.brightness);
  }
  else if (y >= 150 && y < 210) {
    // Theme cycle
    int theme = (int)system_state.current_theme;
    theme = (theme + 1) % 3;
    setTheme((ThemeType)theme);
  }
  
  if (y >= 420) exitCurrentApp();
}

// =============================================================================
// STOPWATCH/TIMER
// =============================================================================

void initStopwatchTimerApp() {
  lap_count = 0;
}

void drawStopwatchTimerApp() {
  if (stopwatch_mode) drawStopwatchMode();
  else drawTimerMode();
}

void handleStopwatchTimerTouch(TouchGesture& gesture) {
  if (stopwatch_mode) handleStopwatchTouch(gesture);
  else handleTimerTouch(gesture);
}

void updateStopwatchApp() {
  // Update timer countdown if running
}

void drawStopwatchMode() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 20);
  gfx->print("STOPWATCH");
  
  // Time display
  unsigned long t = getStopwatchTime();
  int mins = (t / 60000) % 60;
  int secs = (t / 1000) % 60;
  int ms = (t % 1000) / 10;
  
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d.%02d", mins, secs, ms);
  gfx->setTextSize(4);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(60, 150);
  gfx->print(timeStr);
  
  // Lap times
  gfx->setTextSize(1);
  for (int i = 0; i < min(lap_count, 5); i++) {
    int lapMins = (lap_times[i] / 60000) % 60;
    int lapSecs = (lap_times[i] / 1000) % 60;
    gfx->setCursor(20, 250 + i * 20);
    gfx->printf("Lap %d: %02d:%02d", i + 1, lapMins, lapSecs);
  }
  
  // Controls
  drawThemeButton(30, 360, 100, 45, "Start", false);
  drawThemeButton(140, 360, 80, 45, "Lap", false);
  drawThemeButton(230, 360, 100, 45, "Reset", false);
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void drawTimerMode() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(130, 20);
  gfx->print("TIMER");
  
  int remaining = getTimerRemaining();
  int mins = remaining / 60;
  int secs = remaining % 60;
  
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", mins, secs);
  gfx->setTextSize(5);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(80, 180);
  gfx->print(timeStr);
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleStopwatchTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  if (y >= 360 && y < 405) {
    if (x < 130) {
      static bool running = false;
      if (running) pauseStopwatch();
      else startStopwatch();
      running = !running;
    }
    else if (x < 220) recordLapTime();
    else resetStopwatch();
  }
  
  if (y >= 420) exitCurrentApp();
}

void handleTimerTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 420) exitCurrentApp();
}

void recordLapTime() {
  if (lap_count < 10) {
    lap_times[lap_count++] = getStopwatchTime();
  }
}

// =============================================================================
// WALLPAPER SELECTOR
// =============================================================================

static int selected_wallpaper = 0;

void initWallpaperSelector() {
  system_state.current_screen = SCREEN_WALLPAPER_SELECTOR;
}

void drawWallpaperSelector() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("WALLPAPERS");
  
  // Built-in wallpapers
  const char* wallpapers[] = {"Solid Black", "Gradient", "Grid", "Theme Color"};
  uint16_t colors[] = {COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, getCurrentTheme()->primary};
  
  for (int i = 0; i < 4; i++) {
    int x = (i % 2) * 175 + 15;
    int y = (i / 2) * 150 + 80;
    
    gfx->fillRoundRect(x, y, 160, 130, 15, colors[i]);
    if (i == selected_wallpaper) {
      gfx->drawRoundRect(x, y, 160, 130, 15, COLOR_WHITE);
      gfx->drawRoundRect(x + 2, y + 2, 156, 126, 13, COLOR_WHITE);
    }
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x + 10, y + 110);
    gfx->print(wallpapers[i]);
  }
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleWallpaperTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  for (int i = 0; i < 4; i++) {
    int wx = (i % 2) * 175 + 15;
    int wy = (i / 2) * 150 + 80;
    if (x >= wx && x < wx + 160 && y >= wy && y < wy + 130) {
      selectWallpaper(i);
      return;
    }
  }
  
  if (y >= 420) exitCurrentApp();
}

void selectWallpaper(int index) {
  selected_wallpaper = index;
  system_state.wallpaper_enabled = (index != 0);
}

void applyWallpaper(const String& path) {
  system_state.current_wallpaper_path = path;
}

// =============================================================================
// CALCULATOR
// =============================================================================

void initCalculatorApp() {}

void drawCalculatorApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 10);
  gfx->print("CALCULATOR");
  
  // Display
  gfx->fillRoundRect(20, 50, 320, 60, 10, RGB565(30, 30, 30));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(30, 70);
  gfx->print(calc_display);
  
  // Buttons
  const char* buttons[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+"};
  for (int i = 0; i < 16; i++) {
    int x = (i % 4) * 85 + 15;
    int y = (i / 4) * 70 + 130;
    drawThemeButton(x, y, 75, 60, buttons[i], false);
  }
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleCalculatorTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check calculator buttons
  const char* buttons[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+"};
  for (int i = 0; i < 16; i++) {
    int bx = (i % 4) * 85 + 15;
    int by = (i / 4) * 70 + 130;
    if (x >= bx && x < bx + 75 && y >= by && y < by + 60) {
      char btn = buttons[i][0];
      if (btn >= '0' && btn <= '9') {
        if (calc_display == "0") calc_display = String(btn);
        else calc_display += btn;
      } else if (btn == 'C') {
        calc_display = "0";
        calc_result = 0;
        calc_operator = 0;
      } else if (btn == '=') {
        float num = calc_display.toFloat();
        switch(calc_operator) {
          case '+': calc_result += num; break;
          case '-': calc_result -= num; break;
          case '*': calc_result *= num; break;
          case '/': if (num != 0) calc_result /= num; break;
        }
        calc_display = String(calc_result);
        calc_operator = 0;
      } else {
        calc_operand = calc_display.toFloat();
        if (calc_operator == 0) calc_result = calc_operand;
        else {
          switch(calc_operator) {
            case '+': calc_result += calc_operand; break;
            case '-': calc_result -= calc_operand; break;
            case '*': calc_result *= calc_operand; break;
            case '/': if (calc_operand != 0) calc_result /= calc_operand; break;
          }
        }
        calc_operator = btn;
        calc_display = "0";
      }
      return;
    }
  }
  
  if (y >= 420) exitCurrentApp();
}

// =============================================================================
// FLASHLIGHT
// =============================================================================

void initFlashlightApp() {}

void drawFlashlightApp() {
  if (flashlight_on) {
    gfx->fillScreen(COLOR_WHITE);
    gfx->setTextColor(COLOR_BLACK);
  } else {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
  }
  
  gfx->setTextSize(2);
  gfx->setCursor(100, 20);
  gfx->print("FLASHLIGHT");
  
  // Big toggle button
  if (flashlight_on) {
    gfx->fillCircle(LCD_WIDTH/2, 220, 80, COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
  } else {
    gfx->fillCircle(LCD_WIDTH/2, 220, 80, COLOR_YELLOW);
    gfx->setTextColor(COLOR_BLACK);
  }
  gfx->setTextSize(3);
  gfx->setCursor(150, 210);
  gfx->print(flashlight_on ? "OFF" : "ON");
  
  gfx->setTextColor(flashlight_on ? COLOR_BLACK : COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(120, 420);
  gfx->print("Tap anywhere to toggle");
}

void handleFlashlightTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP) {
    flashlight_on = !flashlight_on;
    if (flashlight_on) setDisplayBrightness(255);
    else setDisplayBrightness(system_state.brightness);
  }
}
