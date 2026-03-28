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
#include "navigation.h"
#include "wifi_apps.h"
#include "ui.h"

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
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->primary);
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 30, 16);
  gfx->print("Music");
  
  // Album art placeholder - rounded card
  int artX = LCD_WIDTH/2 - 75;
  gfx->fillRoundRect(artX, 70, 150, 150, 20, RGB565(22, 24, 32));
  gfx->drawRoundRect(artX, 70, 150, 150, 20, RGB565(50, 52, 65));
  gfx->fillCircle(LCD_WIDTH/2, 145, 30, theme->primary);
  gfx->fillCircle(LCD_WIDTH/2, 145, 10, RGB565(22, 24, 32));
  
  // Track info
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  int tw = 7 * 12;
  gfx->setCursor(LCD_WIDTH/2 - tw/2, 240);
  gfx->printf("Track %d", current_track + 1);
  
  gfx->setTextSize(1);
  gfx->setTextColor(is_playing ? RGB565(100, 200, 100) : RGB565(120, 120, 130));
  gfx->setCursor(LCD_WIDTH/2 - 18, 268);
  gfx->print(is_playing ? "Playing" : "Paused");
  
  // Progress bar
  gfx->fillRoundRect(40, 290, LCD_WIDTH - 80, 4, 2, RGB565(40, 42, 55));
  gfx->fillRoundRect(40, 290, (LCD_WIDTH - 80) / 3, 4, 2, theme->primary);
  
  // Controls - pill buttons
  int btnY = 320;
  gfx->fillRoundRect(40, btnY, 75, 45, 22, RGB565(25, 27, 35));
  gfx->drawRoundRect(40, btnY, 75, 45, 22, RGB565(50, 52, 65));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(58, btnY + 13);
  gfx->print("<<");
  
  gfx->fillRoundRect(LCD_WIDTH/2 - 40, btnY, 80, 45, 22, theme->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 18, btnY + 13);
  gfx->print(is_playing ? "||" : " >");
  
  gfx->fillRoundRect(LCD_WIDTH - 115, btnY, 75, 45, 22, RGB565(25, 27, 35));
  gfx->drawRoundRect(LCD_WIDTH - 115, btnY, 75, 45, 22, RGB565(50, 52, 65));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH - 97, btnY + 13);
  gfx->print(">>");
  
  drawSwipeIndicator();
}

void handleMusicTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Control buttons at y=320, height=45
  if (y >= 320 && y < 365) {
    if (x < 130) { previousTrack(); drawMusicApp(); }
    else if (x < 250) { is_playing ? pauseMusic() : playMusic(); drawMusicApp(); }
    else { nextTrack(); drawMusicApp(); }
  }
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
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, RGB565(255, 200, 60));
  gfx->setTextColor(RGB565(255, 200, 60));
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 30, 16);
  gfx->print("Notes");
  
  for (int i = 0; i < 5; i++) {
    int y = 62 + i * 62;
    uint16_t bg = (i == selected_note) ? RGB565(30, 35, 48) : RGB565(22, 24, 32);
    gfx->fillRoundRect(20, y, LCD_WIDTH - 40, 52, 14, bg);
    if (i == selected_note) {
      gfx->drawRoundRect(20, y, LCD_WIDTH - 40, 52, 14, theme->accent);
    } else {
      gfx->drawRoundRect(20, y, LCD_WIDTH - 40, 52, 14, RGB565(42, 44, 55));
    }
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(35, y + 20);
    gfx->print(notes[i]);
  }
  
  drawSwipeIndicator();
}

void handleNotesTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int y = gesture.y;
  
  for (int i = 0; i < 5; i++) {
    int ny = 62 + i * 62;
    if (y >= ny && y < ny + 52) {
      selected_note = i;
      drawNotesApp();
      return;
    }
  }
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
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, RGB565(80, 180, 255));
  gfx->setTextColor(RGB565(80, 180, 255));
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 30, 16);
  gfx->print("Files");
  
  // SD card status card
  gfx->fillRoundRect(20, 70, LCD_WIDTH - 40, 80, 16, RGB565(22, 24, 32));
  gfx->drawRoundRect(20, 70, LCD_WIDTH - 40, 80, 16, RGB565(42, 44, 55));
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(38, 88);
  gfx->print("SD Card");
  
  gfx->setTextSize(1);
  gfx->setTextColor(system_state.filesystem_available ? RGB565(100, 200, 100) : RGB565(200, 80, 80));
  gfx->setCursor(38, 118);
  gfx->print(system_state.filesystem_available ? "Connected" : "Not Found");
  
  // File counts card
  gfx->fillRoundRect(20, 170, LCD_WIDTH - 40, 80, 16, RGB565(22, 24, 32));
  gfx->drawRoundRect(20, 170, LCD_WIDTH - 40, 80, 16, RGB565(42, 44, 55));
  
  gfx->setTextColor(RGB565(120, 120, 130));
  gfx->setTextSize(1);
  gfx->setCursor(38, 188);
  gfx->printf("MP3 Files: %d", system_state.total_mp3_files);
  gfx->setCursor(38, 210);
  gfx->printf("PDF Files: %d", system_state.total_pdf_files);
  gfx->setCursor(38, 232);
  gfx->print("Images: --");
  
  drawSwipeIndicator();
}

void handleFileBrowserTouch(TouchGesture& gesture) {
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
  
  drawSwipeIndicator();
}

void handlePDFReaderTouch(TouchGesture& gesture) {
}

// =============================================================================
// SETTINGS
// =============================================================================

void initSettingsApp() {
  system_state.current_screen = SCREEN_SETTINGS;
}

void drawSettingsApp() {
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 48, 16);
  gfx->print("Settings");
  
  // Settings cards
  const char* options[] = {"Brightness", "Theme", "WiFi", "About"};
  const char* subtexts[] = {"Adjust display", "Change character", "Network setup", "Version info"};
  uint16_t icons[] = {RGB565(255, 200, 60), theme->primary, RGB565(80, 180, 255), RGB565(150, 150, 160)};
  
  for (int i = 0; i < 4; i++) {
    int y = 65 + i * 78;
    
    // Card background
    gfx->fillRoundRect(20, y, LCD_WIDTH - 40, 68, 16, RGB565(22, 24, 32));
    gfx->drawRoundRect(20, y, LCD_WIDTH - 40, 68, 16, RGB565(42, 44, 55));
    
    // Icon circle
    gfx->fillCircle(52, y + 34, 14, icons[i]);
    
    // Text
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(78, y + 15);
    gfx->print(options[i]);
    
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(100, 102, 115));
    gfx->setCursor(78, y + 42);
    gfx->print(subtexts[i]);
    
    // Arrow indicator
    gfx->setTextColor(RGB565(60, 62, 75));
    gfx->setTextSize(2);
    gfx->setCursor(LCD_WIDTH - 52, y + 22);
    gfx->print(">");
  }
  
  drawSwipeIndicator();
}

void handleSettingsTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int y = gesture.y;
  
  // Cards at: y=65, y=143, y=221, y=299 (height 68, gap 78)
  if (y >= 65 && y < 133) {
    // Brightness - cycle
    system_state.brightness = (system_state.brightness + 50) % 256;
    setDisplayBrightness(system_state.brightness);
    drawSettingsApp();
  }
  else if (y >= 143 && y < 211) {
    // Theme selector
    system_state.current_screen = SCREEN_THEME_SELECTOR;
    drawThemeSelector();
  }
  else if (y >= 221 && y < 289) {
    // WiFi
    system_state.current_screen = SCREEN_WIFI_MANAGER;
    drawNetworkListScreen();
  }
  else if (y >= 299 && y < 367) {
    // About
    system_state.current_screen = SCREEN_SETTINGS;
    drawAboutScreen();
  }
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
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->accent);
  gfx->setTextColor(theme->accent);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 54, 16);
  gfx->print("Stopwatch");
  
  // Time display card
  unsigned long t = getStopwatchTime();
  int mins = (t / 60000) % 60;
  int secs = (t / 1000) % 60;
  int ms = (t % 1000) / 10;
  
  gfx->fillRoundRect(20, 70, LCD_WIDTH - 40, 80, 18, RGB565(18, 20, 28));
  gfx->drawRoundRect(20, 70, LCD_WIDTH - 40, 80, 18, RGB565(42, 44, 55));
  
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d.%02d", mins, secs, ms);
  gfx->setTextSize(4);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(55, 92);
  gfx->print(timeStr);
  
  // Lap times
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(120, 120, 130));
  for (int i = 0; i < min(lap_count, 5); i++) {
    int lapMins = (lap_times[i] / 60000) % 60;
    int lapSecs = (lap_times[i] / 1000) % 60;
    gfx->setCursor(30, 170 + i * 22);
    gfx->printf("Lap %d: %02d:%02d", i + 1, lapMins, lapSecs);
  }
  
  // Control buttons - pill shaped
  int btnY = 330;
  gfx->fillRoundRect(24, btnY, 95, 45, 22, RGB565(30, 80, 50));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(44, btnY + 13);
  gfx->print("Start");
  
  gfx->fillRoundRect(LCD_WIDTH/2 - 42, btnY, 84, 45, 22, RGB565(25, 27, 35));
  gfx->drawRoundRect(LCD_WIDTH/2 - 42, btnY, 84, 45, 22, RGB565(50, 52, 65));
  gfx->setCursor(LCD_WIDTH/2 - 18, btnY + 13);
  gfx->print("Lap");
  
  gfx->fillRoundRect(LCD_WIDTH - 119, btnY, 95, 45, 22, RGB565(80, 30, 30));
  gfx->setCursor(LCD_WIDTH - 99, btnY + 13);
  gfx->print("Reset");
  
  drawSwipeIndicator();
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
  
  drawSwipeIndicator();
}

void handleStopwatchTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Buttons at y=330, height=45
  if (y >= 330 && y < 375) {
    static bool running = false;
    if (x < 130) {
      if (running) pauseStopwatch();
      else startStopwatch();
      running = !running;
    }
    else if (x < 250) recordLapTime();
    else resetStopwatch();
    drawStopwatchMode();
  }
}

void handleTimerTouch(TouchGesture& gesture) {
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
  
  drawSwipeIndicator();
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
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 46, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 46, LCD_WIDTH, RGB565(100, 100, 120));
  gfx->setTextColor(RGB565(180, 180, 190));
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 24, 14);
  gfx->print("Calc");
  
  // Display panel
  gfx->fillRoundRect(16, 52, LCD_WIDTH - 32, 55, 14, RGB565(22, 24, 32));
  gfx->drawRoundRect(16, 52, LCD_WIDTH - 32, 55, 14, RGB565(42, 44, 55));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(28, 66);
  gfx->print(calc_display);
  
  // Calculator buttons - modern rounded
  const char* buttons[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+"};
  for (int i = 0; i < 16; i++) {
    int bx = (i % 4) * 85 + 15;
    int by = (i / 4) * 66 + 118;
    int bw = 75;
    int bh = 56;
    
    bool isOp = (i % 4 == 3) || buttons[i][0] == 'C' || buttons[i][0] == '=';
    uint16_t bg = isOp ? RGB565(35, 30, 45) : RGB565(25, 27, 35);
    uint16_t border = isOp ? theme->primary : RGB565(45, 47, 58);
    
    gfx->fillRoundRect(bx, by, bw, bh, 14, bg);
    gfx->drawRoundRect(bx, by, bw, bh, 14, border);
    
    gfx->setTextColor(isOp ? theme->primary : COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(bx + bw/2 - 6, by + bh/2 - 8);
    gfx->print(buttons[i]);
  }
  
  drawSwipeIndicator();
}

void handleCalculatorTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check calculator buttons
  const char* buttons[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+"};
  for (int i = 0; i < 16; i++) {
    int bx = (i % 4) * 85 + 15;
    int by = (i / 4) * 66 + 118;
    if (x >= bx && x < bx + 75 && y >= by && y < by + 56) {
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
      drawCalculatorApp();
      return;
    }
  }
}

// =============================================================================
// FLASHLIGHT
// =============================================================================

void initFlashlightApp() {}

void drawFlashlightApp() {
  if (flashlight_on) {
    gfx->fillScreen(COLOR_WHITE);
  } else {
    gfx->fillScreen(RGB565(8, 8, 12));
  }
  
  int centerX = LCD_WIDTH / 2;
  int centerY = 200;
  
  if (flashlight_on) {
    // ON state - white screen with dark UI
    gfx->setTextColor(RGB565(40, 40, 50));
    gfx->setTextSize(2);
    gfx->setCursor(centerX - 48, 30);
    gfx->print("Torch");
    
    // Big OFF button
    gfx->fillCircle(centerX, centerY, 65, RGB565(40, 42, 55));
    gfx->drawCircle(centerX, centerY, 66, RGB565(80, 82, 95));
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(3);
    gfx->setCursor(centerX - 24, centerY - 10);
    gfx->print("OFF");
    
    gfx->setTextColor(RGB565(100, 100, 110));
    gfx->setTextSize(1);
    gfx->setCursor(centerX - 32, 380);
    gfx->print("Tap to toggle");
  } else {
    // OFF state - dark screen
    gfx->setTextColor(RGB565(180, 180, 190));
    gfx->setTextSize(2);
    gfx->setCursor(centerX - 48, 30);
    gfx->print("Torch");
    
    // Glow ring around button
    for (int r = 70; r > 65; r--) {
      gfx->drawCircle(centerX, centerY, r, RGB565(40, 35, 10));
    }
    
    // Big ON button
    gfx->fillCircle(centerX, centerY, 65, RGB565(255, 200, 60));
    gfx->setTextColor(RGB565(30, 30, 30));
    gfx->setTextSize(3);
    gfx->setCursor(centerX - 18, centerY - 10);
    gfx->print("ON");
    
    gfx->setTextColor(RGB565(70, 72, 85));
    gfx->setTextSize(1);
    gfx->setCursor(centerX - 32, 380);
    gfx->print("Tap to toggle");
  }
}

void handleFlashlightTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP) {
    flashlight_on = !flashlight_on;
    if (flashlight_on) setDisplayBrightness(255);
    else setDisplayBrightness(system_state.brightness);
    drawFlashlightApp();
  }
}
