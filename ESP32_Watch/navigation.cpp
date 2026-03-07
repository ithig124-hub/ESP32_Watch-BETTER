/*
 * navigation.cpp - Swipe Navigation Implementation
 * Based on S3_MiniOS_206.ino navigation pattern
 * 
 * Horizontal swipe (left/right): Infinite loop through:
 *   Watchface -> App Grid 1 -> Character Stats -> Watchface...
 * 
 * Vertical swipe (on App Grid only):
 *   Swipe Down: App Grid 1 -> App Grid 2
 *   Swipe Up: App Grid 2 -> App Grid 1
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
#include <time.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Global navigation state
NavigationState navState = {
  MAIN_WATCHFACE,  // currentMain
  0,               // appGridPage
  false,           // isTransitioning
  0,               // lastNavigationMs
  false            // navigationLocked
};

// =============================================================================
// INITIALIZATION
// =============================================================================

void initNavigation() {
  Serial.println("[NAV] Initializing swipe navigation system...");
  navState.currentMain = MAIN_WATCHFACE;
  navState.appGridPage = 0;
  navState.isTransitioning = false;
  navState.lastNavigationMs = 0;
  navState.navigationLocked = false;
  
  // Set initial system state
  system_state.current_screen = SCREEN_WATCHFACE;
  
  Serial.println("[NAV] Navigation: Swipe left/right for Watchface<->AppGrid<->Stats loop");
  Serial.println("[NAV] On App Grid: Swipe down for Page 2, up for Page 1");
}

// =============================================================================
// NAVIGATION CONTROL
// =============================================================================

bool canNavigate() {
  if (navState.navigationLocked) {
    Serial.println("[NAV] BLOCKED: navigationLocked");
    return false;
  }
  if (navState.isTransitioning) {
    Serial.println("[NAV] BLOCKED: isTransitioning");
    return false;
  }
  if (millis() - navState.lastNavigationMs < NAVIGATION_COOLDOWN_MS) {
    Serial.printf("[NAV] BLOCKED: cooldown (%lu ms remaining)\n", 
                  NAVIGATION_COOLDOWN_MS - (millis() - navState.lastNavigationMs));
    return false;
  }
  return true;
}

void handleSwipeNavigation(int dx, int dy) {
  Serial.printf("[NAV] handleSwipeNavigation called: dx=%d, dy=%d\n", dx, dy);
  
  if (!canNavigate()) {
    Serial.println("[NAV] Navigation blocked - resetting locks");
    // Force reset locks to prevent stuck state
    navState.navigationLocked = false;
    navState.isTransitioning = false;
    return;
  }
  
  Serial.printf("[NAV] Swipe detected: dx=%d, dy=%d\n", dx, dy);
  
  // Determine if horizontal or vertical swipe
  if (abs(dx) > abs(dy) && abs(dx) > SWIPE_THRESHOLD_MIN) {
    // HORIZONTAL SWIPE - Navigate main screens in loop
    if (dx < 0) {
      // Swipe LEFT -> Next screen
      navigateRight();
    } else {
      // Swipe RIGHT -> Previous screen
      navigateLeft();
    }
  } else if (abs(dy) > abs(dx) && abs(dy) > SWIPE_THRESHOLD_MIN) {
    // VERTICAL SWIPE - Only works on App Grid for sub-pages
    if (navState.currentMain == MAIN_APP_GRID_1) {
      if (dy < 0 && navState.appGridPage < APP_GRID_PAGES - 1) {
        // Swipe UP (finger moves up, content scrolls down) -> App Grid 2
        navigateDown();
      } else if (dy > 0 && navState.appGridPage > 0) {
        // Swipe DOWN (finger moves down, content scrolls up) -> App Grid 1
        navigateUp();
      }
    }
  }
}

void navigateLeft() {
  // Previous screen in loop
  int prev = (int)navState.currentMain - 1;
  if (prev < 0) prev = MAIN_SCREEN_COUNT - 1;
  
  Serial.printf("[NAV] Navigate LEFT: %d -> %d\n", navState.currentMain, prev);
  navigateToScreen((MainScreen)prev);
}

void navigateRight() {
  // Next screen in loop
  int next = ((int)navState.currentMain + 1) % MAIN_SCREEN_COUNT;
  
  Serial.printf("[NAV] Navigate RIGHT: %d -> %d\n", navState.currentMain, next);
  navigateToScreen((MainScreen)next);
}

void navigateUp() {
  // Only for App Grid - go to page 1
  if (navState.currentMain == MAIN_APP_GRID_1 && navState.appGridPage > 0) {
    Serial.printf("[NAV] App Grid: Page %d -> Page 0\n", navState.appGridPage);
    navState.appGridPage = 0;
    navState.lastNavigationMs = millis();
    drawCurrentScreen();
  }
}

void navigateDown() {
  // Only for App Grid - go to page 2
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
  
  // Update state
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
  
  Serial.printf("[NAV] Navigated to screen: %d\n", screen);
  
  // Draw the new screen
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
// SCREEN DRAWING
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
  
  // Draw navigation indicators
  drawNavigationIndicators();
}

// Draw small dots indicating current position in the navigation loop
void drawNavigationIndicators() {
  int centerX = LCD_WIDTH / 2;
  int y = LCD_HEIGHT - 15;
  int dotRadius = 4;
  int spacing = 15;
  
  // Draw 3 dots for the main screens
  for (int i = 0; i < MAIN_SCREEN_COUNT; i++) {
    int x = centerX + (i - 1) * spacing;
    
    if (i == (int)navState.currentMain) {
      // Current screen - filled dot with theme color
      gfx->fillCircle(x, y, dotRadius, getCurrentTheme()->primary);
    } else {
      // Other screens - hollow dot
      gfx->drawCircle(x, y, dotRadius, COLOR_GRAY);
    }
  }
  
  // If on App Grid, draw sub-page indicator
  if (navState.currentMain == MAIN_APP_GRID_1) {
    int subY = y - 12;
    for (int i = 0; i < APP_GRID_PAGES; i++) {
      int x = centerX + (i == 0 ? -5 : 5);
      if (i == navState.appGridPage) {
        gfx->fillCircle(x, subY, 2, getCurrentTheme()->accent);
      } else {
        gfx->drawCircle(x, subY, 2, COLOR_GRAY);
      }
    }
  }
}

// =============================================================================
// WATCH FACE SCREEN - Defined in themes.cpp
// =============================================================================

// drawWatchFace() is defined in themes.cpp with full themed implementations

void drawStatsBar() {
  int y = 320;
  ThemeColors* theme = getCurrentTheme();
  
  // Battery
  gfx->setTextColor(system_state.battery_percentage > 20 ? COLOR_GREEN : COLOR_RED);
  gfx->setTextSize(1);
  gfx->setCursor(30, y);
  gfx->printf("BAT: %d%%", system_state.battery_percentage);
  
  // Steps
  gfx->setTextColor(theme->accent);
  gfx->setCursor(130, y);
  gfx->printf("STEPS: %d", system_state.steps_today);
  
  // Gems
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(250, y);
  gfx->printf("GEMS: %d", system_state.player_gems);
  
  // Level
  gfx->setTextColor(theme->primary);
  gfx->setCursor(LCD_WIDTH / 2 - 20, y + 25);
  gfx->printf("Lv.%d", system_state.player_level);
}

// =============================================================================
// APP GRID SCREENS - Modern UI Design
// =============================================================================

// App icon colors for visual variety
uint16_t getAppColor(const char* appName) {
  if (strcmp(appName, "GACHA") == 0) return RGB565(255, 180, 50);   // Gold
  if (strcmp(appName, "TRAINING") == 0) return RGB565(50, 200, 100); // Green
  if (strcmp(appName, "BOSS") == 0) return RGB565(220, 60, 60);      // Red
  if (strcmp(appName, "GAMES") == 0) return RGB565(100, 150, 255);   // Blue
  if (strcmp(appName, "QUESTS") == 0) return RGB565(200, 100, 255);  // Purple
  if (strcmp(appName, "MUSIC") == 0) return RGB565(255, 100, 150);   // Pink
  if (strcmp(appName, "WEATHER") == 0) return RGB565(80, 180, 255);  // Sky blue
  if (strcmp(appName, "WIFI") == 0) return RGB565(100, 220, 200);    // Teal
  if (strcmp(appName, "SETTINGS") == 0) return RGB565(150, 150, 160);// Gray
  if (strcmp(appName, "THEMES") == 0) return RGB565(255, 120, 80);   // Orange
  if (strcmp(appName, "COLLECT") == 0) return RGB565(255, 200, 100); // Light gold
  if (strcmp(appName, "FILES") == 0) return RGB565(180, 180, 100);   // Tan
  if (strcmp(appName, "CALC") == 0) return RGB565(100, 100, 120);    // Dark gray
  if (strcmp(appName, "TORCH") == 0) return RGB565(255, 255, 150);   // Yellow
  return RGB565(100, 100, 120);
}

// App Grid 1 - Main apps
void drawAppGrid1() {
  // Gradient background
  for (int y = 0; y < LCD_HEIGHT; y++) {
    uint8_t shade = map(y, 0, LCD_HEIGHT, 15, 25);
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(shade, shade, shade + 5));
  }
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header bar
  gfx->fillRect(0, 0, LCD_WIDTH, 55, RGB565(25, 25, 30));
  gfx->drawFastHLine(0, 55, LCD_WIDTH, theme->primary);
  
  // Title
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(155, 18);
  gfx->print("APPS");
  
  // Page indicator dots
  int dotY = 42;
  gfx->fillCircle(LCD_WIDTH/2 - 8, dotY, 4, theme->primary);  // Active
  gfx->drawCircle(LCD_WIDTH/2 + 8, dotY, 4, RGB565(80, 80, 80)); // Inactive
  
  // App icons in 3x3 grid
  const char* apps1[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
  int cols = 3, rows = 3;
  int iconSize = 75;
  int startX = 35, startY = 70;
  int spacingX = (LCD_WIDTH - 70) / cols;
  int spacingY = 110;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    uint16_t appColor = getAppColor(apps1[i]);
    
    // Icon shadow
    gfx->fillRoundRect(x + 3, y + 3, iconSize, iconSize, 18, RGB565(10, 10, 12));
    
    // Icon background - gradient effect
    gfx->fillRoundRect(x, y, iconSize, iconSize, 18, RGB565(35, 35, 45));
    
    // Colored accent at top of icon
    gfx->fillRoundRect(x, y, iconSize, 25, 18, appColor);
    gfx->fillRect(x, y + 15, iconSize, 10, appColor);
    
    // App name - centered
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    int textLen = strlen(apps1[i]) * 6;
    int textX = x + (iconSize - textLen) / 2;
    gfx->setCursor(textX, y + iconSize / 2 + 8);
    gfx->print(apps1[i]);
  }
  
  // Bottom hint
  gfx->setTextColor(RGB565(100, 100, 110));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 50, LCD_HEIGHT - 25);
  gfx->print("Swipe for more");
}

// App Grid 2 - More apps
void drawAppGrid2() {
  // Gradient background
  for (int y = 0; y < LCD_HEIGHT; y++) {
    uint8_t shade = map(y, 0, LCD_HEIGHT, 15, 25);
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(shade, shade, shade + 5));
  }
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header bar
  gfx->fillRect(0, 0, LCD_WIDTH, 55, RGB565(25, 25, 30));
  gfx->drawFastHLine(0, 55, LCD_WIDTH, theme->accent);
  
  // Title
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(155, 18);
  gfx->print("MORE");
  
  // Page indicator dots
  int dotY = 42;
  gfx->drawCircle(LCD_WIDTH/2 - 8, dotY, 4, RGB565(80, 80, 80)); // Inactive
  gfx->fillCircle(LCD_WIDTH/2 + 8, dotY, 4, theme->accent);       // Active
  
  // App icons in 3x3 grid
  const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
  int cols = 3, rows = 3;
  int iconSize = 75;
  int startX = 35, startY = 70;
  int spacingX = (LCD_WIDTH - 70) / cols;
  int spacingY = 110;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    uint16_t appColor = getAppColor(apps2[i]);
    
    // Icon shadow
    gfx->fillRoundRect(x + 3, y + 3, iconSize, iconSize, 18, RGB565(10, 10, 12));
    
    // Icon background
    gfx->fillRoundRect(x, y, iconSize, iconSize, 18, RGB565(35, 35, 45));
    
    // Colored accent at top
    gfx->fillRoundRect(x, y, iconSize, 25, 18, appColor);
    gfx->fillRect(x, y + 15, iconSize, 10, appColor);
    
    // App name
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    int textLen = strlen(apps2[i]) * 6;
    int textX = x + (iconSize - textLen) / 2;
    gfx->setCursor(textX, y + iconSize / 2 + 8);
    gfx->print(apps2[i]);
  }
  
  // Bottom hint
  gfx->setTextColor(RGB565(100, 100, 110));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 50, LCD_HEIGHT - 25);
  gfx->print("Swipe for more");
}

// =============================================================================
// CHARACTER STATS SCREEN - Defined in themes.cpp
// =============================================================================

// drawCharacterStatsScreen() is defined in themes.cpp with full themed implementations

void drawStatBar(int x, int y, const char* name, int value, uint16_t color) {
  int maxWidth = 200;
  int barWidth = (value * maxWidth) / 100;
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->print(name);
  
  // Bar background
  gfx->fillRoundRect(x + 80, y - 2, maxWidth, 12, 4, RGB565(60, 60, 60));
  
  // Bar fill
  if (barWidth > 0) {
    gfx->fillRoundRect(x + 80, y - 2, barWidth, 12, 4, color);
  }
  
  // Value text
  gfx->setCursor(x + 290, y);
  gfx->printf("%d", value);
}

int getXPForNextLevel() {
  // Simple XP curve
  return system_state.player_level * 100 + 100;
}

// =============================================================================
// TOUCH HANDLING FOR CURRENT SCREEN
// =============================================================================

void handleCurrentScreenTouch(TouchGesture& gesture) {
  // Only handle taps - swipes are handled by navigation system
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  switch (navState.currentMain) {
    case MAIN_WATCHFACE:
      // Tap on watchface - could cycle watch face styles
      Serial.println("[NAV] Tap on watchface");
      break;
      
    case MAIN_APP_GRID_1:
      handleAppGridTap(x, y);
      break;
      
    case MAIN_CHARACTER_STATS:
      // Tap on stats - could show detailed view
      Serial.println("[NAV] Tap on character stats");
      break;
  }
}

void handleAppGridTap(int x, int y) {
  Serial.printf("[NAV] handleAppGridTap: x=%d, y=%d, page=%d\n", x, y, navState.appGridPage);
  
  // The touch coordinates from FT3168 might be in a smaller range
  // Let's scale them to screen coordinates if needed
  // Based on logs showing coords 0-32, the touch panel reports in its own coordinate system
  
  // Scale touch coordinates to screen resolution
  // FT3168 appears to report 0-368 for X and 0-448 for Y on this board
  int screenX = x;
  int screenY = y;
  
  // If coordinates seem too small, they might need scaling
  // Detect if we're getting raw small coordinates
  if (x < 50 && y < 50) {
    // Likely raw coordinates that need scaling
    screenX = map(x, 0, 32, 0, LCD_WIDTH);
    screenY = map(y, 0, 48, 0, LCD_HEIGHT);
    Serial.printf("[NAV] Scaled coords: (%d,%d) -> (%d,%d)\n", x, y, screenX, screenY);
  }
  
  // Determine which app was tapped
  int cols = 3;
  int iconSize = 80;
  int startX = 30, startY = 80;
  int spacingX = (LCD_WIDTH - 60) / cols;
  int spacingY = 100;
  
  Serial.printf("[NAV] Grid: startX=%d, startY=%d, spacingX=%d, spacingY=%d, iconSize=%d\n",
                startX, startY, spacingX, spacingY, iconSize);
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int appX = startX + col * spacingX;
    int appY = startY + row * spacingY;
    
    Serial.printf("[NAV] App %d: bounds (%d,%d)-(%d,%d)\n", i, appX, appY, appX+iconSize, appY+iconSize);
    
    if (screenX >= appX && screenX < appX + iconSize && screenY >= appY && screenY < appY + iconSize) {
      // App tapped!
      if (navState.appGridPage == 0) {
        // Page 1 apps
        const char* apps1[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
        Serial.printf("[NAV] >>> APP TAPPED: %s <<<\n", apps1[i]);
        openApp(apps1[i]);
      } else {
        // Page 2 apps
        const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
        Serial.printf("[NAV] >>> APP TAPPED: %s <<<\n", apps2[i]);
        openApp(apps2[i]);
      }
      return;
    }
  }
  
  Serial.println("[NAV] Tap didn't hit any app icon");
}

void openApp(const char* appName) {
  Serial.printf("[NAV] Opening app: %s\n", appName);
  
  // Lock navigation while in app
  navState.navigationLocked = true;
  
  if (strcmp(appName, "GACHA") == 0) {
    system_state.current_screen = SCREEN_GACHA;
    drawGachaScreen();
  } else if (strcmp(appName, "TRAINING") == 0) {
    system_state.current_screen = SCREEN_TRAINING;
    drawTrainingMenu();  // Fixed: was drawTrainingScreen
  } else if (strcmp(appName, "BOSS") == 0) {
    system_state.current_screen = SCREEN_BOSS_RUSH;
    drawBossRushMenu();
  } else if (strcmp(appName, "GAMES") == 0) {
    system_state.current_screen = SCREEN_GAMES;
    drawGameMenu();
  } else if (strcmp(appName, "QUESTS") == 0) {
    system_state.current_screen = SCREEN_QUESTS;
    drawQuestScreen();
  } else if (strcmp(appName, "SETTINGS") == 0) {
    system_state.current_screen = SCREEN_SETTINGS;
    drawSettingsApp();  // This exists in apps.cpp
  } else if (strcmp(appName, "THEMES") == 0) {
    system_state.current_screen = SCREEN_THEME_SELECTOR;
    // Theme selector not implemented yet
  } else if (strcmp(appName, "COLLECT") == 0) {
    system_state.current_screen = SCREEN_COLLECTION;
    drawGachaCollection();
  } else if (strcmp(appName, "MUSIC") == 0) {
    system_state.current_screen = SCREEN_MUSIC;
    drawMusicApp();  // This exists in apps.cpp
  } else if (strcmp(appName, "WEATHER") == 0) {
    system_state.current_screen = SCREEN_WEATHER_APP;
    drawWeatherApp();  // This exists in wifi_apps.cpp
  } else if (strcmp(appName, "WIFI") == 0) {
    system_state.current_screen = SCREEN_WIFI_MANAGER;
    drawNetworkListScreen();  // Fixed: correct function name
  } else if (strcmp(appName, "CALC") == 0) {
    system_state.current_screen = SCREEN_CALCULATOR;
    drawCalculatorApp();  // This exists in apps.cpp
  } else if (strcmp(appName, "TORCH") == 0) {
    system_state.current_screen = SCREEN_FLASHLIGHT;
    drawFlashlightApp();  // This exists in apps.cpp
  } else if (strcmp(appName, "FILES") == 0) {
    system_state.current_screen = SCREEN_FILE_BROWSER;
    drawFileBrowserApp();  // This exists in apps.cpp
  } else {
    Serial.printf("[NAV] Unknown app: %s\n", appName);
    navState.navigationLocked = false;
  }
}

void returnToAppGrid() {
  navState.navigationLocked = false;
  navState.currentMain = MAIN_APP_GRID_1;
  system_state.current_screen = SCREEN_APP_GRID;
  drawCurrentScreen();
}
