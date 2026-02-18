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
  if (navState.navigationLocked) return false;
  if (navState.isTransitioning) return false;
  if (millis() - navState.lastNavigationMs < NAVIGATION_COOLDOWN_MS) return false;
  return true;
}

void handleSwipeNavigation(int dx, int dy) {
  if (!canNavigate()) return;
  
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
// WATCH FACE SCREEN
// =============================================================================

void drawWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Default time if RTC not available
    timeinfo.tm_hour = 12;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
  }
  
  // Theme colors
  ThemeColors* theme = getCurrentTheme();
  
  // Big time display
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(6);
  char timeStr[8];
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  
  // Center the time
  int textWidth = 5 * 6 * 6;  // 5 chars, size 6
  gfx->setCursor((LCD_WIDTH - textWidth) / 2, 150);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(theme->accent);
  gfx->setCursor(LCD_WIDTH / 2 + 70, 180);
  gfx->printf("%02d", timeinfo.tm_sec);
  
  // Date
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                          "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  gfx->setCursor(LCD_WIDTH / 2 - 60, 250);
  gfx->printf("%s %s %d", days[timeinfo.tm_wday], months[timeinfo.tm_mon], timeinfo.tm_mday);
  
  // Character name at top
  CharacterProfile* profile = getCurrentCharacterProfile();
  if (profile) {
    gfx->setTextColor(theme->primary);
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH / 2 - strlen(profile->name) * 3, 30);
    gfx->print(profile->name);
  }
  
  // Stats bar at bottom
  drawStatsBar();
  
  // Swipe hint
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 50, 400);
  gfx->print("< SWIPE FOR APPS >");
}

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
// APP GRID SCREENS
// =============================================================================

// App Grid 1 - Main apps
void drawAppGrid1() {
  gfx->fillScreen(COLOR_BLACK);
  
  ThemeColors* theme = getCurrentTheme();
  
  // Title
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(130, 20);
  gfx->print("APPS");
  
  // Page indicator
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 20, 50);
  gfx->print("Page 1/2");
  
  // App icons in 3x3 grid
  const char* apps1[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
  int cols = 3, rows = 3;
  int iconSize = 80;
  int startX = 30, startY = 80;
  int spacingX = (LCD_WIDTH - 60) / cols;
  int spacingY = 100;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    // Icon background
    gfx->fillRoundRect(x, y, iconSize, iconSize, 15, RGB565(40, 40, 50));
    gfx->drawRoundRect(x, y, iconSize, iconSize, 15, theme->primary);
    
    // App name
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    int textX = x + (iconSize - strlen(apps1[i]) * 6) / 2;
    gfx->setCursor(textX, y + iconSize / 2 - 4);
    gfx->print(apps1[i]);
  }
  
  // Swipe hints
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 60, LCD_HEIGHT - 45);
  gfx->print("SWIPE DOWN: PAGE 2");
}

// App Grid 2 - More apps
void drawAppGrid2() {
  gfx->fillScreen(COLOR_BLACK);
  
  ThemeColors* theme = getCurrentTheme();
  
  // Title
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(130, 20);
  gfx->print("APPS");
  
  // Page indicator
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 20, 50);
  gfx->print("Page 2/2");
  
  // App icons in 3x3 grid
  const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
  int cols = 3, rows = 3;
  int iconSize = 80;
  int startX = 30, startY = 80;
  int spacingX = (LCD_WIDTH - 60) / cols;
  int spacingY = 100;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacingX;
    int y = startY + row * spacingY;
    
    // Icon background
    gfx->fillRoundRect(x, y, iconSize, iconSize, 15, RGB565(40, 40, 50));
    gfx->drawRoundRect(x, y, iconSize, iconSize, 15, theme->accent);
    
    // App name
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    int textX = x + (iconSize - strlen(apps2[i]) * 6) / 2;
    gfx->setCursor(textX, y + iconSize / 2 - 4);
    gfx->print(apps2[i]);
  }
  
  // Swipe hints
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 50, LCD_HEIGHT - 45);
  gfx->print("SWIPE UP: PAGE 1");
}

// =============================================================================
// CHARACTER STATS SCREEN
// =============================================================================

void drawCharacterStatsScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  ThemeColors* theme = getCurrentTheme();
  CharacterProfile* profile = getCurrentCharacterProfile();
  
  // Character name and title
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  if (profile) {
    gfx->setCursor(LCD_WIDTH / 2 - strlen(profile->name) * 6, 30);
    gfx->print(profile->name);
    
    gfx->setTextColor(theme->accent);
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH / 2 - strlen(profile->title) * 3, 55);
    gfx->print(profile->title);
  }
  
  // Level and XP
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(LCD_WIDTH / 2 - 50, 90);
  gfx->printf("Lv.%d", system_state.player_level);
  
  // XP bar
  int barX = 40, barY = 140, barW = LCD_WIDTH - 80, barH = 20;
  int xpProgress = (system_state.player_xp * barW) / max(1, getXPForNextLevel());
  
  gfx->fillRoundRect(barX, barY, barW, barH, barH/2, RGB565(40, 40, 40));
  if (xpProgress > 0) {
    gfx->fillRoundRect(barX, barY, xpProgress, barH, barH/2, theme->primary);
  }
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(barX, barY + 25);
  gfx->printf("%d / %d XP", system_state.player_xp, getXPForNextLevel());
  
  // Stats panel
  int statsY = 190;
  gfx->fillRoundRect(30, statsY, LCD_WIDTH - 60, 180, 15, RGB565(30, 30, 40));
  
  // Stats display
  if (profile) {
    drawStatBar(50, statsY + 20, profile->stats.stat1_name, profile->stats.stat1_value, theme->primary);
    drawStatBar(50, statsY + 50, profile->stats.stat2_name, profile->stats.stat2_value, theme->accent);
    drawStatBar(50, statsY + 80, profile->stats.stat3_name, profile->stats.stat3_value, theme->effect1);
    drawStatBar(50, statsY + 110, profile->stats.stat4_name, profile->stats.stat4_value, theme->effect2);
    
    // Catchphrase
    gfx->setTextColor(theme->primary);
    gfx->setTextSize(1);
    gfx->setCursor(50, statsY + 150);
    gfx->print(profile->catchphrase);
  }
  
  // Game stats
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(50, 390);
  gfx->printf("Gems: %d  |  Cards: %d/%d  |  Bosses: %d/%d",
              system_state.player_gems,
              system_state.gacha_cards_collected, GACHA_TOTAL_CARDS,
              system_state.bosses_defeated, TOTAL_BOSSES);
  
  // Swipe hint
  gfx->setTextColor(COLOR_GRAY);
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH / 2 - 60, LCD_HEIGHT - 45);
  gfx->print("< SWIPE FOR WATCH >");
}

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
  // Determine which app was tapped
  int cols = 3;
  int iconSize = 80;
  int startX = 30, startY = 80;
  int spacingX = (LCD_WIDTH - 60) / cols;
  int spacingY = 100;
  
  for (int i = 0; i < 9; i++) {
    int col = i % cols;
    int row = i / cols;
    int appX = startX + col * spacingX;
    int appY = startY + row * spacingY;
    
    if (x >= appX && x < appX + iconSize && y >= appY && y < appY + iconSize) {
      // App tapped!
      if (navState.appGridPage == 0) {
        // Page 1 apps
        const char* apps1[] = {"GACHA", "TRAINING", "BOSS", "GAMES", "QUESTS", "MUSIC", "WEATHER", "WIFI", "SETTINGS"};
        Serial.printf("[NAV] App tapped: %s\n", apps1[i]);
        openApp(apps1[i]);
      } else {
        // Page 2 apps
        const char* apps2[] = {"THEMES", "COLLECT", "FILES", "CALC", "TORCH", "OTA", "BACKUP", "FUSION", "ABOUT"};
        Serial.printf("[NAV] App tapped: %s\n", apps2[i]);
        openApp(apps2[i]);
      }
      return;
    }
  }
}

void openApp(const char* appName) {
  // Lock navigation while in app
  navState.navigationLocked = true;
  
  if (strcmp(appName, "GACHA") == 0) {
    system_state.current_screen = SCREEN_GACHA;
  } else if (strcmp(appName, "TRAINING") == 0) {
    system_state.current_screen = SCREEN_TRAINING;
  } else if (strcmp(appName, "BOSS") == 0) {
    system_state.current_screen = SCREEN_BOSS_RUSH;
  } else if (strcmp(appName, "GAMES") == 0) {
    system_state.current_screen = SCREEN_GAMES;
  } else if (strcmp(appName, "QUESTS") == 0) {
    system_state.current_screen = SCREEN_QUESTS;
  } else if (strcmp(appName, "SETTINGS") == 0) {
    system_state.current_screen = SCREEN_SETTINGS;
  } else if (strcmp(appName, "THEMES") == 0) {
    system_state.current_screen = SCREEN_THEME_SELECTOR;
  } else if (strcmp(appName, "COLLECT") == 0) {
    system_state.current_screen = SCREEN_COLLECTION;
  } else if (strcmp(appName, "OTA") == 0) {
    system_state.current_screen = SCREEN_OTA_UPDATE;
  } else if (strcmp(appName, "BACKUP") == 0) {
    system_state.current_screen = SCREEN_BACKUP;
  } else if (strcmp(appName, "FUSION") == 0) {
    system_state.current_screen = SCREEN_FUSION_LABS;
  } else if (strcmp(appName, "WIFI") == 0) {
    system_state.current_screen = SCREEN_WIFI_MANAGER;
  } else if (strcmp(appName, "WEATHER") == 0) {
    system_state.current_screen = SCREEN_WEATHER_APP;
  }
  
  // Note: In a full implementation, each app would have its own draw function
  // and would unlock navigation when closed (back button)
}

void returnToAppGrid() {
  navState.navigationLocked = false;
  navState.currentMain = MAIN_APP_GRID_1;
  system_state.current_screen = SCREEN_APP_GRID;
  drawCurrentScreen();
}
