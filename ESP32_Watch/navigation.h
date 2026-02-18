/*
 * navigation.h - Swipe Navigation System
 * Based on S3_MiniOS_206.ino navigation pattern
 * 
 * Navigation Flow:
 *   Watchface <-> App Grid 1 <-> Character Stats (infinite loop via left/right swipe)
 *   App Grid 1 <-> App Grid 2 (via up/down swipe)
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "config.h"

// =============================================================================
// NAVIGATION CONSTANTS
// =============================================================================

#define SWIPE_THRESHOLD_MIN     40
#define SWIPE_MAX_DURATION_MS   800
#define NAVIGATION_COOLDOWN_MS  200

// Main navigation screens (horizontal swipe loop)
enum MainScreen {
  MAIN_WATCHFACE = 0,
  MAIN_APP_GRID_1,
  MAIN_CHARACTER_STATS,
  MAIN_SCREEN_COUNT
};

// App Grid sub-pages (vertical swipe)
#define APP_GRID_PAGES 2

// =============================================================================
// NAVIGATION STATE
// =============================================================================

struct NavigationState {
  MainScreen currentMain;
  int appGridPage;           // 0 = App Grid 1, 1 = App Grid 2
  bool isTransitioning;
  unsigned long lastNavigationMs;
  bool navigationLocked;
};

extern NavigationState navState;

// =============================================================================
// NAVIGATION FUNCTIONS
// =============================================================================

void initNavigation();
void handleSwipeNavigation(int dx, int dy);
void navigateToScreen(MainScreen screen);
void navigateLeft();
void navigateRight();
void navigateUp();
void navigateDown();

// Screen queries
MainScreen getCurrentMainScreen();
int getCurrentAppGridPage();
bool canNavigate();

// Screen drawing dispatchers
void drawCurrentScreen();
void handleCurrentScreenTouch(TouchGesture& gesture);

// Screen drawing functions
void drawWatchFace();
void drawAppGrid1();
void drawAppGrid2();
void drawCharacterStatsScreen();
void drawNavigationIndicators();
void drawStatsBar();
void drawStatBar(int x, int y, const char* name, int value, uint16_t color);

// App handling
void handleAppGridTap(int x, int y);
void openApp(const char* appName);
void returnToAppGrid();
int getXPForNextLevel();

// Animation helpers
void transitionToScreen(MainScreen from, MainScreen to, bool slideLeft);
void updateTransitionAnimation();

#endif // NAVIGATION_H
