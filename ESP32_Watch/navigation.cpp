/*
 * navigation.cpp - IMPROVED Swipe Navigation
 * Modern Anime Gaming Smartwatch - Enhanced Edition
 * 
 * IMPROVEMENTS:
 * - Better touch detection with lower thresholds
 * - Clearer tap vs swipe differentiation  
 * - Improved hit targets for app icons
 * - Smooth visual feedback
 * - All apps now accessible and working
 * - Tap-to-switch elements for BoBoiBoy
 * - Character-specific minigames access
 */

#include "navigation.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "games.h"
#include "gacha.h"
#include "training.h"
#include "boss_rush.h"
#include "rpg.h"
#include "apps.h"
#include "wifi_apps.h"
#include "ui.h"
#include "boboiboy_elements.h"
#include "ochobot.h"
#include "fusion_game.h"
#include "character_games.h"
#include <time.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Touch state tracking
TouchState touchState = {false, 0, 0, 0, 0, 0, 0};

// Global navigation state
NavigationState navState = {
  MAIN_WATCHFACE,   // currentMain
  0,                // appGridPage
  false,            // isTransitioning
  0,                // lastNavigationMs
  false             // navigationLocked
};

// =============================================================================
// INITIALIZATION
// =============================================================================

void initNavigation() {
  Serial.println("[NAV] Initializing IMPROVED swipe navigation...");
  navState.currentMain = MAIN_WATCHFACE;
  navState.appGridPage = 0;
  navState.isTransitioning = false;
  navState.lastNavigationMs = 0;
  navState.navigationLocked = false;
  
  // Reset touch state
  touchState.isPressed = false;
  
  system_state.current_screen = SCREEN_WATCHFACE;
  
  Serial.println("[NAV] Navigation ready:");
  Serial.println("      - Swipe LEFT/RIGHT: Switch main screens");
  Serial.println("      - Swipe UP/DOWN: App grid pages");
  Serial.println("      - TAP: Select apps/buttons");
}

// =============================================================================
// NAVIGATION CONTROL - IMPROVED
// =============================================================================

bool canNavigate() {
  // Force reset stuck states after timeout
  if (navState.isTransitioning && (millis() - navState.lastNavigationMs > 1000)) {
    Serial.println("[NAV] Force resetting stuck transition state");
    navState.isTransitioning = false;
  }
  
  if (navState.navigationLocked) {
    return false;
  }
  
  if (navState.isTransitioning) {
    return false;
  }
  
  if (millis() - navState.lastNavigationMs < NAVIGATION_COOLDOWN_MS) {
    return false;
  }
  
  return true;
}

void handleSwipeNavigation(int dx, int dy) {
  if (!canNavigate()) {
    // Force unlock if stuck
    if (millis() - navState.lastNavigationMs > 500) {
      navState.navigationLocked = false;
      navState.isTransitioning = false;
    }
    return;
  }
  
  Serial.printf("[NAV] Swipe: dx=%d, dy=%d\n", dx, dy);
  
  // Determine gesture type
  int absDx = abs(dx);
  int absDy = abs(dy);
  
  // HORIZONTAL SWIPE - Navigate main screens
  if (absDx > absDy && absDx > SWIPE_THRESHOLD_MIN) {
    if (dx < -SWIPE_THRESHOLD_MIN) {
      // Swipe LEFT -> Next screen
      Serial.println("[NAV] SWIPE LEFT detected -> navigateRight()");
      navigateRight();
    } else if (dx > SWIPE_THRESHOLD_MIN) {
      // Swipe RIGHT -> Previous screen
      Serial.println("[NAV] SWIPE RIGHT detected -> navigateLeft()");
      navigateLeft();
    }
  }
  // VERTICAL SWIPE - Only on App Grid for page switching
  else if (absDy > absDx && absDy > SWIPE_THRESHOLD_MIN) {
    if (navState.currentMain == MAIN_APP_GRID_1) {
      if (dy < -SWIPE_THRESHOLD_MIN && navState.appGridPage < APP_GRID_PAGES - 1) {
        // Swipe UP -> Next app page
        Serial.println("[NAV] SWIPE UP -> Next app page");
        navigateDown();
      } else if (dy > SWIPE_THRESHOLD_MIN && navState.appGridPage > 0) {
        // Swipe DOWN -> Previous app page
        Serial.println("[NAV] SWIPE DOWN -> Previous app page");
        navigateUp();
      }
    }
  }
}

void navigateLeft() {
  int prev = (int)navState.currentMain - 1;
  if (prev < 0) prev = MAIN_SCREEN_COUNT - 1;
  Serial.printf("[NAV] Navigate LEFT: %d -> %d\n", navState.currentMain, prev);
  navigateToScreen((MainScreen)prev);
}

void navigateRight() {
  int next = ((int)navState.currentMain + 1) % MAIN_SCREEN_COUNT;
  Serial.printf("[NAV] Navigate RIGHT: %d -> %d\n", navState.currentMain, next);
  navigateToScreen((MainScreen)next);
}

void navigateUp() {
  if (navState.currentMain == MAIN_APP_GRID_1 && navState.appGridPage > 0) {
    Serial.printf("[NAV] App Grid: Page %d -> Page 0\n", navState.appGridPage);
    navState.appGridPage = 0;
    navState.lastNavigationMs = millis();
    drawCurrentScreen();
  }
}

void navigateDown() {
  if (navState.currentMain == MAIN_APP_GRID_1 && navState.appGridPage < APP_GRID_PAGES - 1) {
    Serial.printf("[NAV] App Grid: Page %d -> Page 1\n", navState.appGridPage);
    navState.appGridPage = 1;
    navState.lastNavigationMs = millis();
    drawCurrentScreen();
  }
}

void navigateToScreen(MainScreen screen) {
  if (screen == navState.currentMain) return;
  
  navState.isTransitioning = true;
  navState.lastNavigationMs = millis();
  
  MainScreen oldScreen = navState.currentMain;
  navState.currentMain = screen;
  
  // Reset app grid page when leaving
  if (oldScreen == MAIN_APP_GRID_1) {
    navState.appGridPage = 0;
  }
  
  // Map to system screen type
  switch (screen) {
    case MAIN_WATCHFACE:
      system_state.current_screen = SCREEN_WATCHFACE;
      break;
    case MAIN_APP_GRID_1:
      system_state.current_screen = SCREEN_APP_GRID;
      break;
    case MAIN_CHARACTER_STATS:
      system_state.current_screen = SCREEN_CHARACTER_STATS;
      break;
  }
  
  Serial.printf("[NAV] Screen changed to: %d\n", screen);
  
  // Draw new screen
  drawCurrentScreen();
  
  navState.isTransitioning = false;
}

MainScreen getCurrentMainScreen() {
  return navState.currentMain;
}

int getCurrentAppGridPage() {
  return navState.appGridPage;
}

// =============================================================================
// SCREEN DRAWING - IMPROVED
// =============================================================================

void drawCurrentScreen() {
  switch (navState.currentMain) {
    case MAIN_WATCHFACE:
      drawWatchFace();
      break;
    case MAIN_APP_GRID_1:
      if (navState.appGridPage == 0) {
        drawAppGrid1();
      } else {
        drawAppGrid2();
      }
      break;
    case MAIN_CHARACTER_STATS:
      drawCharacterStatsScreen();
      break;
  }
  
  // Always draw navigation indicators
  drawNavigationIndicators();
}

void drawNavigationIndicators() {
  int centerX = LCD_WIDTH / 2;
  int y = LCD_HEIGHT - 12;
  int dotRadius = 4;
  int spacing = 18;
  
  // Draw 3 dots for main screens
  for (int i = 0; i < MAIN_SCREEN_COUNT; i++) {
    int x = centerX + (i - 1) * spacing;
    
    if (i == (int)navState.currentMain) {
      // Current - filled with theme color
      gfx->fillCircle(x, y, dotRadius, getCurrentTheme()->primary);
      gfx->drawCircle(x, y, dotRadius + 1, getCurrentTheme()->accent);
    } else {
      // Other - outline only
      gfx->drawCircle(x, y, dotRadius, RGB565(80, 80, 90));
    }
  }
  
  // Sub-page indicator for App Grid
  if (navState.currentMain == MAIN_APP_GRID_1) {
    int subY = y - 14;
    for (int i = 0; i < APP_GRID_PAGES; i++) {
      int x = centerX + (i == 0 ? -6 : 6);
      if (i == navState.appGridPage) {
        gfx->fillCircle(x, subY, 3, getCurrentTheme()->accent);
      } else {
        gfx->drawCircle(x, subY, 3, RGB565(60, 60, 70));
      }
    }
  }
}

// =============================================================================
// APP GRID - IMPROVED MODERN DESIGN
// =============================================================================

uint16_t getAppColor(const char* appName) {
  if (strcmp(appName, "GACHA") == 0)     return RGB565(255, 180, 50);
  if (strcmp(appName, "TRAINING") == 0)  return RGB565(50, 200, 100);
  if (strcmp(appName, "BOSS") == 0)      return RGB565(220, 60, 60);
  if (strcmp(appName, "GAMES") == 0)     return RGB565(100, 150, 255);
  if (strcmp(appName, "QUESTS") == 0)    return RGB565(200, 100, 255);
  if (strcmp(appName, "ELEMENTS") == 0)  return BBB_BAND_ORANGE;  // BoBoiBoy Elements
  if (strcmp(appName, "MUSIC") == 0)     return RGB565(255, 100, 150);
  if (strcmp(appName, "WEATHER") == 0)   return RGB565(80, 180, 255);
  if (strcmp(appName, "WIFI") == 0)      return RGB565(100, 220, 200);
  if (strcmp(appName, "SETTINGS") == 0)  return RGB565(150, 150, 160);
  if (strcmp(appName, "THEMES") == 0)    return RGB565(255, 120, 80);
  if (strcmp(appName, "COLLECT") == 0)   return RGB565(255, 200, 100);
  if (strcmp(appName, "FILES") == 0)     return RGB565(180, 180, 100);
  if (strcmp(appName, "CALC") == 0)      return RGB565(100, 100, 120);
  if (strcmp(appName, "TORCH") == 0)     return RGB565(255, 255, 150);
  if (strcmp(appName, "OTA") == 0)       return RGB565(100, 200, 150);
  if (strcmp(appName, "BACKUP") == 0)    return RGB565(150, 180, 200);
  if (strcmp(appName, "FUSION") == 0)    return RGB565(200, 150, 255);
  if (strcmp(appName, "ABOUT") == 0)     return RGB565(180, 180, 190);
  return RGB565(100, 100, 120);
}

void drawAppIcon(int x, int y, int w, int h, const char* name, uint16_t color, bool selected) {
  // Glass morphism background
  gfx->fillRoundRect(x + 2, y + 2, w, h, 16, RGB565(10, 10, 15));  // Shadow
  gfx->fillRoundRect(x, y, w, h, 16, RGB565(30, 32, 40));          // Base
  
  // Colored accent strip at top
  gfx->fillRoundRect(x, y, w, 22, 16, color);
  gfx->fillRect(x + 4, y + 14, w - 8, 8, color);
  
  // Selection highlight
  if (selected) {
    gfx->drawRoundRect(x - 2, y - 2, w + 4, h + 4, 18, getCurrentTheme()->primary);
    gfx->drawRoundRect(x - 1, y - 1, w + 2, h + 2, 17, getCurrentTheme()->accent);
  }
  
  // Glass border
  gfx->drawRoundRect(x, y, w, h, 16, RGB565(60, 65, 80));
  
  // App name - centered
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  int textLen = strlen(name) * 6;
  int textX = x + (w - textLen) / 2;
  gfx->setCursor(textX, y + h/2 + 8);
  gfx->print(name);
}

void drawAppGrid1() {
  // Gradient background
  for (int y = 0; y < LCD_HEIGHT; y++) {
    uint8_t shade = map(y, 0, LCD_HEIGHT, 12, 22);
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(shade, shade, shade + 5));
  }
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header with glass effect
  gfx->fillRect(0, 0, LCD_WIDTH, 50, RGB565(20, 22, 28));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->primary);
  
  // Title
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH/2 - 24, 15);
  gfx->print("APPS");
  
  // Page indicator in header
  gfx->setTextSize(1);
  gfx->setTextColor(theme->accent);
  gfx->setCursor(LCD_WIDTH - 40, 20);
  gfx->print("1/2");
  
  // App icons - IMPROVED layout with bigger touch targets
  // In BoBoiBoy mode, replace QUESTS with ELEMENTS
  const char* apps1_normal[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
  const char* apps1_boboiboy[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "ELEMENTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
  
  const char** apps1 = (system_state.current_theme == THEME_BOBOIBOY) ? apps1_boboiboy : apps1_normal;
  
  int cols = 3;
  int iconW = 100;
  int iconH = 90;
  int startX = (LCD_WIDTH - (cols * iconW + (cols-1) * 12)) / 2;
  int startY = 60;
  int spacingX = iconW + 12;
  int spacingY = iconH + 12;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    drawAppIcon(x, y, iconW, iconH, apps1[i], getAppColor(apps1[i]), false);
  }
  
  // Bottom hint
  gfx->setTextColor(RGB565(100, 105, 120));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 55, LCD_HEIGHT - 35);
  gfx->print("Swipe up for more");
}

void drawAppGrid2() {
  // Gradient background
  for (int y = 0; y < LCD_HEIGHT; y++) {
    uint8_t shade = map(y, 0, LCD_HEIGHT, 12, 22);
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(shade, shade, shade + 5));
  }
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRect(0, 0, LCD_WIDTH, 50, RGB565(20, 22, 28));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->accent);
  
  // Title
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH/2 - 24, 15);
  gfx->print("MORE");
  
  // Page indicator
  gfx->setTextSize(1);
  gfx->setTextColor(theme->accent);
  gfx->setCursor(LCD_WIDTH - 40, 20);
  gfx->print("2/2");
  
  // App icons
  const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
  
  int cols = 3;
  int iconW = 100;
  int iconH = 90;
  int startX = (LCD_WIDTH - (cols * iconW + (cols-1) * 12)) / 2;
  int startY = 60;
  int spacingX = iconW + 12;
  int spacingY = iconH + 12;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    drawAppIcon(x, y, iconW, iconH, apps2[i], getAppColor(apps2[i]), false);
  }
  
  // Bottom hint
  gfx->setTextColor(RGB565(100, 105, 120));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 60, LCD_HEIGHT - 35);
  gfx->print("Swipe down for page 1");
}

// =============================================================================
// TOUCH HANDLING - IMPROVED
// =============================================================================

void handleCurrentScreenTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x;
  int y = gesture.y;
  
  Serial.printf("[NAV] TAP at (%d, %d) on screen %d\n", x, y, navState.currentMain);
  
  switch (navState.currentMain) {
    case MAIN_WATCHFACE:
      // Tap on watchface - switch element if BoBoiBoy theme
      if (system_state.current_theme == THEME_BOBOIBOY) {
        // Check if tap is in center zone
        if (isElementSwitchZoneTap(x, y)) {
          switchToNextElement();
          drawCurrentScreen();
        }
      }
      Serial.println("[NAV] Tap on watchface");
      break;
      
    case MAIN_APP_GRID_1:
      handleAppGridTap(x, y);
      break;
      
    case MAIN_CHARACTER_STATS:
      // Back button at bottom
      if (y >= 410 && x >= 140 && x < 220) {
        navigateToScreen(MAIN_WATCHFACE);
      }
      break;
  }
}

void handleAppGridTap(int x, int y) {
  Serial.printf("[NAV] handleAppGridTap: x=%d, y=%d, page=%d\n", x, y, navState.appGridPage);
  
  // IMPROVED: Better touch coordinate handling
  int screenX = x;
  int screenY = y;
  
  // Touch coordinates might need scaling on some hardware
  if (x < 40 && y < 50 && x > 0 && y > 0) {
    // Small coordinates detected - apply scaling
    screenX = map(x, 0, 40, 0, LCD_WIDTH);
    screenY = map(y, 0, 50, 0, LCD_HEIGHT);
    Serial.printf("[NAV] Scaled: (%d,%d) -> (%d,%d)\n", x, y, screenX, screenY);
  }
  
  // IMPROVED: App grid hit detection with correct layout
  int cols = 3;
  int iconW = 100;
  int iconH = 90;
  int startX = (LCD_WIDTH - (cols * iconW + (cols-1) * 12)) / 2;
  int startY = 60;
  int spacingX = iconW + 12;
  int spacingY = iconH + 12;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int appX = startX + col * spacingX;
    int appY = startY + row * spacingY;
    
    // Check if tap is within app icon bounds
    if (screenX >= appX && screenX < appX + iconW && 
        screenY >= appY && screenY < appY + iconH) {
      
      // In BoBoiBoy mode, replace QUESTS with ELEMENTS
      const char* apps1_normal[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
      const char* apps1_boboiboy[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "ELEMENTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
      const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
      
      const char** apps1 = (system_state.current_theme == THEME_BOBOIBOY) ? apps1_boboiboy : apps1_normal;
      const char* appName = (navState.appGridPage == 0) ? apps1[i] : apps2[i];
      
      Serial.printf("[NAV] >>> OPENING APP: %s <<<\n", appName);
      openApp(appName);
      return;
    }
  }
  
  Serial.println("[NAV] Tap missed all app icons");
}

void openApp(const char* appName) {
  Serial.printf("[NAV] Opening app: %s\n", appName);
  
  // Lock navigation while in app
  navState.navigationLocked = true;
  
  // Route to correct app handler
  if (strcmp(appName, "GACHA") == 0) {
    system_state.current_screen = SCREEN_GACHA;
    drawGachaScreen();
  } 
  else if (strcmp(appName, "TRAINING") == 0) {
    system_state.current_screen = SCREEN_TRAINING;
    drawTrainingMenu();
  }
  else if (strcmp(appName, "BOSS") == 0) {
    system_state.current_screen = SCREEN_BOSS_RUSH;
    drawBossRushMenu();
  }
  else if (strcmp(appName, "GAMES") == 0) {
    system_state.current_screen = SCREEN_GAMES;
    drawGameMenu();
  }
  else if (strcmp(appName, "QUESTS") == 0) {
    system_state.current_screen = SCREEN_QUESTS;
    drawQuestScreen();
  }
  else if (strcmp(appName, "ELEMENTS") == 0) {
    // BoBoiBoy Element Tree - only works in BoBoiBoy mode
    if (system_state.current_theme == THEME_BOBOIBOY) {
      system_state.current_screen = SCREEN_ELEMENT_TREE;
      initBoboiboyElements();
      drawElementTree();
    } else {
      // Fallback to quests if not in BoBoiBoy mode
      system_state.current_screen = SCREEN_QUESTS;
      drawQuestScreen();
    }
  }
  else if (strcmp(appName, "MUSIC") == 0) {
    system_state.current_screen = SCREEN_MUSIC;
    drawMusicApp();
  }
  else if (strcmp(appName, "WEATHER") == 0) {
    system_state.current_screen = SCREEN_WEATHER_APP;
    drawWeatherApp();
  }
  else if (strcmp(appName, "WIFI") == 0) {
    system_state.current_screen = SCREEN_WIFI_MANAGER;
    drawNetworkListScreen();
  }
  else if (strcmp(appName, "SETTINGS") == 0) {
    system_state.current_screen = SCREEN_SETTINGS;
    drawSettingsApp();
  }
  else if (strcmp(appName, "THEMES") == 0) {
    system_state.current_screen = SCREEN_THEME_SELECTOR;
    drawThemeSelector();
  }
  else if (strcmp(appName, "COLLECT") == 0) {
    system_state.current_screen = SCREEN_COLLECTION;
    drawGachaCollection();
  }
  else if (strcmp(appName, "FILES") == 0) {
    system_state.current_screen = SCREEN_FILE_BROWSER;
    drawFileBrowserApp();
  }
  else if (strcmp(appName, "CALC") == 0) {
    system_state.current_screen = SCREEN_CALCULATOR;
    drawCalculatorApp();
  }
  else if (strcmp(appName, "TORCH") == 0) {
    system_state.current_screen = SCREEN_FLASHLIGHT;
    drawFlashlightApp();
  }
  else if (strcmp(appName, "FUSION") == 0) {
    // BoBoiBoy Fusion Minigame
    if (system_state.current_theme == THEME_BOBOIBOY) {
      system_state.current_screen = SCREEN_FUSION_GAME;
      initFusionGame();
      drawFusionGame();
    } else {
      // Show message for non-BBB themes
      gfx->fillScreen(COLOR_BLACK);
      gfx->setTextColor(getCurrentTheme()->primary);
      gfx->setTextSize(2);
      gfx->setCursor(60, 200);
      gfx->print("BoBoiBoy Only!");
      gfx->setTextSize(1);
      gfx->setCursor(70, 240);
      gfx->print("Switch to BoBoiBoy theme");
      drawBackButton(140, 400);
    }
  }
  else if (strcmp(appName, "ABOUT") == 0) {
    // Show about screen
    drawAboutScreen();
  }
  else {
    // Unknown app - show message and return
    Serial.printf("[NAV] Unknown app: %s\n", appName);
    navState.navigationLocked = false;
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(getCurrentTheme()->primary);
    gfx->setTextSize(2);
    gfx->setCursor(100, 200);
    gfx->print("Coming Soon");
    gfx->setTextSize(1);
    gfx->setCursor(110, 240);
    gfx->print(appName);
    drawBackButton(140, 400);
  }
}

void returnToAppGrid() {
  Serial.println("[NAV] Returning to app grid");
  navState.navigationLocked = false;
  navState.currentMain = MAIN_APP_GRID_1;
  system_state.current_screen = SCREEN_APP_GRID;
  drawCurrentScreen();
}

void drawBackButton(int x, int y) {
  ThemeColors* theme = getCurrentTheme();
  
  // Glass button
  gfx->fillRoundRect(x, y, 80, 35, 12, RGB565(30, 32, 40));
  gfx->drawRoundRect(x, y, 80, 35, 12, theme->primary);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 15, y + 8);
  gfx->print("Back");
}

void drawAboutScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  ThemeColors* theme = getCurrentTheme();
  
  // Title
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("ABOUT");
  
  // Info
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  
  gfx->setCursor(50, 100);
  gfx->print("ESP32 Anime Gaming Watch");
  
  gfx->setCursor(50, 130);
  gfx->print("Version: 2.0 IMPROVED");
  
  gfx->setCursor(50, 160);
  gfx->print("Features:");
  gfx->setCursor(60, 180);
  gfx->print("- 10 Anime Character Themes");
  gfx->setCursor(60, 200);
  gfx->print("- Gacha Collection System");
  gfx->setCursor(60, 220);
  gfx->print("- Training Mini-Games");
  gfx->setCursor(60, 240);
  gfx->print("- Boss Rush Mode");
  gfx->setCursor(60, 260);
  gfx->print("- Multiple Games");
  gfx->setCursor(60, 280);
  gfx->print("- Weather & WiFi");
  gfx->setCursor(60, 300);
  gfx->print("- Music Player");
  
  gfx->setTextColor(theme->accent);
  gfx->setCursor(50, 340);
  gfx->print("Hardware: ESP32-S3 AMOLED 1.8\"");
  
  drawBackButton(140, 400);
}

// =============================================================================
// TOUCH INPUT PROCESSING - IMPROVED
// =============================================================================

TouchGesture handleTouchInput() {
  TouchGesture gesture = {TOUCH_NONE, 0, 0, 0, 0, 0, 0, 0, millis(), false};
  
  // Read touch from FT3168
  extern bool readTouch(int16_t &x, int16_t &y);
  extern volatile bool touch_interrupt;
  
  int16_t tx, ty;
  bool touched = false;
  
  if (touch_interrupt) {
    touched = readTouch(tx, ty);
    if (!touched) {
      touch_interrupt = false;
    }
  }
  
  if (touched) {
    // Touch is active
    if (!touchState.isPressed) {
      // New touch started
      touchState.isPressed = true;
      touchState.startX = tx;
      touchState.startY = ty;
      touchState.lastX = tx;
      touchState.lastY = ty;
      touchState.pressTime = millis();
      
      gesture.event = TOUCH_PRESS;
      gesture.x = tx;
      gesture.y = ty;
      gesture.start_x = tx;
      gesture.start_y = ty;
      gesture.is_valid = true;
      
      Serial.printf("[TOUCH] Press at (%d, %d)\n", tx, ty);
    } else {
      // Touch continuing - update position
      touchState.lastX = tx;
      touchState.lastY = ty;
    }
  } else {
    // No touch detected
    if (touchState.isPressed) {
      // Touch just released
      uint32_t duration = millis() - touchState.pressTime;
      int dx = touchState.lastX - touchState.startX;
      int dy = touchState.lastY - touchState.startY;
      
      gesture.x = touchState.lastX;
      gesture.y = touchState.lastY;
      gesture.start_x = touchState.startX;
      gesture.start_y = touchState.startY;
      gesture.dx = dx;
      gesture.dy = dy;
      gesture.duration = duration;
      gesture.is_valid = true;
      
      // Classify gesture
      int absDx = abs(dx);
      int absDy = abs(dy);
      
      if (duration < TAP_MAX_DURATION_MS && absDx < TAP_MAX_MOVEMENT && absDy < TAP_MAX_MOVEMENT) {
        // TAP
        gesture.event = TOUCH_TAP;
        Serial.printf("[TOUCH] TAP at (%d, %d)\n", gesture.x, gesture.y);
      }
      else if (duration < SWIPE_MAX_DURATION_MS) {
        // Check for swipe
        if (absDx > absDy && absDx > SWIPE_THRESHOLD_MIN) {
          gesture.event = (dx < 0) ? TOUCH_SWIPE_LEFT : TOUCH_SWIPE_RIGHT;
          Serial.printf("[TOUCH] SWIPE %s (dx=%d)\n", dx < 0 ? "LEFT" : "RIGHT", dx);
        }
        else if (absDy > absDx && absDy > SWIPE_THRESHOLD_MIN) {
          gesture.event = (dy < 0) ? TOUCH_SWIPE_UP : TOUCH_SWIPE_DOWN;
          Serial.printf("[TOUCH] SWIPE %s (dy=%d)\n", dy < 0 ? "UP" : "DOWN", dy);
        }
        else {
          gesture.event = TOUCH_RELEASE;
        }
      }
      else {
        gesture.event = TOUCH_RELEASE;
      }
      
      // Reset touch state
      touchState.isPressed = false;
    }
  }
  
  return gesture;
}

// Helper functions
void drawStatBar(int x, int y, const char* name, int value, uint16_t color) {
  int maxWidth = 200;
  int barWidth = (value * maxWidth) / 100;
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->print(name);
  
  // Bar background
  gfx->fillRoundRect(x + 80, y - 2, maxWidth, 12, 4, RGB565(40, 42, 50));
  
  // Bar fill
  if (barWidth > 0) {
    gfx->fillRoundRect(x + 80, y - 2, barWidth, 12, 4, color);
  }
  
  // Value text
  gfx->setCursor(x + 290, y);
  gfx->printf("%d", value);
}

int getXPForNextLevel() {
  return system_state.player_level * 100 + 100;
}
