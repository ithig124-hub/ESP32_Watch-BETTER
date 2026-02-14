/*
 * themes.cpp - Theme Implementation
 * Character-specific visual themes and watch faces
 */

#include "themes.h"
#include "config.h"
#include "display.h"
#include "hardware.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Theme color definitions
ThemeColors luffy_gear5_theme = {
  .primary = LUFFY_GOLD,
  .secondary = LUFFY_WHITE,
  .accent = COLOR_ORANGE,
  .background = COLOR_BLACK,
  .text = COLOR_WHITE,
  .shadow = LUFFY_PURPLE
};

ThemeColors sung_jinwoo_theme = {
  .primary = JINWOO_PURPLE,
  .secondary = JINWOO_DARK,
  .accent = JINWOO_VIOLET,
  .background = COLOR_BLACK,
  .text = COLOR_WHITE,
  .shadow = RGB565(50, 0, 80)
};

ThemeColors yugo_wakfu_theme = {
  .primary = YUGO_TEAL,
  .secondary = YUGO_BLUE,
  .accent = YUGO_ENERGY,
  .background = COLOR_BLACK,
  .text = COLOR_WHITE,
  .shadow = RGB565(0, 50, 80)
};

ThemeColors* current_theme = &luffy_gear5_theme;

// =============================================================================
// THEME MANAGEMENT
// =============================================================================

void initializeThemes() {
  Serial.println("[Themes] Initializing theme system...");
  setTheme(system_state.current_theme);
}

void setTheme(ThemeType theme) {
  system_state.current_theme = theme;
  switch(theme) {
    case THEME_LUFFY_GEAR5:  current_theme = &luffy_gear5_theme; break;
    case THEME_SUNG_JINWOO:  current_theme = &sung_jinwoo_theme; break;
    case THEME_YUGO_WAKFU:   current_theme = &yugo_wakfu_theme; break;
    default: current_theme = &luffy_gear5_theme; break;
  }
}

ThemeColors* getCurrentTheme() {
  return current_theme;
}

// =============================================================================
// WATCH FACES
// =============================================================================

void drawWatchFace() {
  switch(system_state.current_theme) {
    case THEME_LUFFY_GEAR5: drawLuffyWatchFace(); break;
    case THEME_SUNG_JINWOO: drawJinwooWatchFace(); break;
    case THEME_YUGO_WAKFU: drawYugoWatchFace(); break;
    default: drawLuffyWatchFace(); break;
  }
}

void drawLuffyWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Title
  gfx->setTextColor(LUFFY_GOLD);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("GEAR 5");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(80, 180);
  gfx->print(timeStr);
  
  // Activity rings
  drawLuffyActivityRings(LCD_WIDTH/2, 350);
  
  // Battery
  drawBatteryIndicator();
}

void drawJinwooWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Title
  gfx->setTextColor(JINWOO_PURPLE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 30);
  gfx->print("SHADOW MONARCH");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(JINWOO_VIOLET);
  gfx->setCursor(80, 180);
  gfx->print(timeStr);
  
  // Activity rings
  drawJinwooActivityRings(LCD_WIDTH/2, 350);
  
  // Shadow effects
  drawJinwooShadows();
  drawBatteryIndicator();
}

void drawYugoWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Title
  gfx->setTextColor(YUGO_TEAL);
  gfx->setTextSize(2);
  gfx->setCursor(90, 30);
  gfx->print("ELIATROPE");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(YUGO_ENERGY);
  gfx->setCursor(80, 180);
  gfx->print(timeStr);
  
  // Activity rings
  drawYugoActivityRings(LCD_WIDTH/2, 350);
  
  // Portal effects
  drawYugoPortals();
  drawBatteryIndicator();
}

void drawSleepWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(4);
  gfx->setTextColor(RGB565(50, 50, 50));
  gfx->setCursor(100, 200);
  gfx->print(timeStr);
}

void drawBatteryIndicator() {
  int x = LCD_WIDTH - 50, y = 10;
  gfx->drawRect(x, y, 40, 18, COLOR_WHITE);
  gfx->fillRect(x + 40, y + 4, 4, 10, COLOR_WHITE);
  
  int fillWidth = (system_state.battery_percentage * 36) / 100;
  uint16_t color = system_state.battery_percentage > 20 ? COLOR_GREEN : COLOR_RED;
  if (system_state.is_charging) color = COLOR_YELLOW;
  gfx->fillRect(x + 2, y + 2, fillWidth, 14, color);
}

void drawCustomWallpaperWatchFace() {
  // Load wallpaper from SD if available
  drawMinimalTimeOverlay();
}

void drawWallpaperOverlayElements() {
  drawMinimalTimeOverlay();
  drawBatteryIndicator();
}

void drawMinimalTimeOverlay() {
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  // Semi-transparent background
  gfx->fillRoundRect(60, 180, 250, 80, 15, RGB565(0, 0, 0));
  
  gfx->setTextSize(4);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(90, 200);
  gfx->print(timeStr);
}

// =============================================================================
// ANIMATIONS
// =============================================================================

void playLuffyAnimation() {
  // Gear 5 transformation animation
  for (int i = 0; i < 5; i++) {
    gfx->fillCircle(LCD_WIDTH/2, LCD_HEIGHT/2, 50 + i*20, LUFFY_GOLD);
    delay(50);
  }
}

void playJinwooAnimation() {
  // Shadow emergence animation
  for (int i = 0; i < 10; i++) {
    int x = random(50, LCD_WIDTH - 50);
    int y = random(100, LCD_HEIGHT - 100);
    gfx->fillCircle(x, y, 20, JINWOO_PURPLE);
    delay(30);
  }
}

void playYugoAnimation() {
  // Portal opening animation
  for (int r = 10; r < 80; r += 5) {
    gfx->drawCircle(LCD_WIDTH/2, LCD_HEIGHT/2, r, YUGO_TEAL);
    delay(30);
  }
}

// =============================================================================
// UI ELEMENTS
// =============================================================================

void drawThemeButton(int x, int y, int w, int h, const char* text, bool pressed) {
  uint16_t bg = pressed ? current_theme->accent : current_theme->primary;
  gfx->fillRoundRect(x, y, w, h, 10, bg);
  gfx->drawRoundRect(x, y, w, h, 10, current_theme->text);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  int textW = strlen(text) * 12;
  gfx->setCursor(x + (w - textW)/2, y + (h - 16)/2);
  gfx->print(text);
}

void drawGameButton(int x, int y, int w, int h, const char* text, bool pressed) {
  drawThemeButton(x, y, w, h, text, pressed);
}

void drawThemeProgressBar(int x, int y, int w, int h, float progress, const char* label) {
  progress = constrain(progress, 0.0f, 1.0f);
  
  // Background
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(40, 40, 40));
  
  // Progress fill
  int fillW = (int)(w * progress);
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, current_theme->primary);
  }
  
  // Label
  if (label && strlen(label) > 0) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x, y - 12);
    gfx->print(label);
  }
}

void drawThemeNotification(const char* title, const char* message) {
  int w = 300, h = 100;
  int x = (LCD_WIDTH - w) / 2;
  int y = 50;
  
  gfx->fillRoundRect(x, y, w, h, 15, current_theme->shadow);
  gfx->drawRoundRect(x, y, w, h, 15, current_theme->primary);
  
  gfx->setTextColor(current_theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(x + 15, y + 15);
  gfx->print(title);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 15, y + 50);
  gfx->print(message);
}

// =============================================================================
// CHARACTER EFFECTS
// =============================================================================

void drawLuffyGear5Effects() {
  // Draw golden aura particles
  for (int i = 0; i < 8; i++) {
    int x = random(0, LCD_WIDTH);
    int y = random(0, LCD_HEIGHT);
    gfx->fillCircle(x, y, 3, LUFFY_GOLD);
  }
}

void drawLuffyCartoonEffects() {
  // Cartoonish effects
  for (int i = 0; i < 5; i++) {
    int x = random(50, LCD_WIDTH - 50);
    int y = random(50, LCD_HEIGHT - 50);
    gfx->drawCircle(x, y, 10, COLOR_WHITE);
  }
}

void drawJinwooShadows() {
  // Draw shadow soldiers
  for (int i = 0; i < 5; i++) {
    int x = 50 + i * 60;
    int y = LCD_HEIGHT - 80;
    gfx->fillEllipse(x, y, 15, 25, JINWOO_DARK);
    gfx->fillCircle(x, y - 20, 10, JINWOO_PURPLE);
  }
}

void drawYugoPortals() {
  // Draw portal effects
  gfx->drawCircle(60, 100, 30, YUGO_TEAL);
  gfx->drawCircle(60, 100, 25, YUGO_ENERGY);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 30, YUGO_TEAL);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 25, YUGO_ENERGY);
}

// =============================================================================
// ACTIVITY RINGS
// =============================================================================

void drawLuffyActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, LUFFY_GOLD, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, COLOR_ORANGE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, COLOR_RED, 5);
}

void drawJinwooActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, JINWOO_PURPLE, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, JINWOO_VIOLET, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, JINWOO_DARK, 5);
}

void drawYugoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, YUGO_TEAL, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, YUGO_BLUE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, YUGO_ENERGY, 5);
}
