/*
 * ESP32_Watch_IMPROVED.ino - Main Firmware
 * Modern Anime Gaming Smartwatch - Enhanced Edition
 * 
 * IMPROVEMENTS OVER ORIGINAL:
 * - Better swipe navigation with lower thresholds
 * - All apps are now accessible via tap
 * - Modern glass morphism UI throughout
 * - Improved touch responsiveness
 * - 11 anime character themes with unique visuals (including BoBoiBoy!)
 * - BoBoiBoy Element Tree with 20 forms
 * - Better visual hierarchy and contrast
 * 
 * Hardware: ESP32-S3-Touch-AMOLED-1.8"
 * Display: 368x448 SH8601 AMOLED (QSPI)
 * Touch: FT3168 Capacitive (I2C)
 * 
 * Free to modify and distribute!
 */

#include "config.h"
#include "display.h"
#include "hardware.h"
#include "touch.h"
#include "navigation.h"
#include "themes.h"
#include "apps.h"
#include "games.h"
#include "gacha.h"
#include "training.h"
#include "boss_rush.h"
#include "rpg.h"
#include "wifi_apps.h"
#include "filesystem.h"
#include "boboiboy_elements.h"
#include "ochobot.h"
#include "dynamic_bg.h"
#include "fusion_game.h"
#include "character_games.h"
#include "ui.h"

// Display
Arduino_SH8601 *gfx = nullptr;

// Global state
SystemState system_state = {
  .current_screen = SCREEN_SPLASH,
  .current_theme = THEME_LUFFY_GEAR5,
  .current_app = APP_WATCHFACE,
  .brightness = 200,
  .display_available = false,
  .touch_available = false,
  .touch_active = false,
  .power_available = false,
  .battery_percentage = 85,
  .is_charging = false,
  .low_battery_warning = false,
  .lvgl_available = false,
  .sleep_timer = 0,
  .wifi_connected = false,
  .wifi_ssid = "",
  .steps_today = 3456,
  .step_goal = 10000,
  .player_gems = 500,
  .player_level = 1,
  .player_xp = 0,
  .gacha_cards_collected = 0,
  .bosses_defeated = 0,
  .training_streak = 0,
  .daily_login_count = 0,
  .music_playing = false,
  .total_mp3_files = 0,
  .total_pdf_files = 0,
  .filesystem_available = false,
  .current_wallpaper_path = "",
  .wallpaper_enabled = false
};

// Touch interrupt flag
volatile bool touch_interrupt = false;
bool sdCardInitialized = false;
bool wifiConnected = false;

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n===================================");
  Serial.println(" ESP32 Anime Gaming Watch IMPROVED");
  Serial.println(" Modern UI | Better Navigation");
  Serial.println("===================================\n");
  
  // Initialize hardware
  initializeHardware();
  
  // Initialize display
  initDisplay();
  
  // Show splash screen
  drawSplashScreen();
  delay(2000);
  
  // Initialize touch
  if (initTouch()) {
    Serial.println("[INIT] Touch initialized");
    system_state.touch_available = true;
    
    // Attach touch interrupt
    attachInterrupt(digitalPinToInterrupt(TP_INT), touchISR, FALLING);
  }
  
  // Initialize themes
  initializeThemes();
  
  // Initialize navigation system
  initNavigation();
  
  // Initialize apps
  initializeApps();
  
  // Initialize games
  initializeGames();
  
  // Initialize gacha
  initGachaSystem();
  
  // Initialize training
  initTrainingSystem();
  
  // Initialize boss rush
  initBossRush();
  
  // Initialize RPG
  initRPGSystem();
  
  // Initialize filesystem
  initFilesystem();
  
  // Initialize WiFi apps
  initWifiApps();
  
  // Go to watch face
  system_state.current_screen = SCREEN_WATCHFACE;
  drawWatchFace();
  drawNavigationIndicators();
  
  Serial.println("\n[INIT] Ready! Swipe or tap to navigate");
  Serial.println("       LEFT/RIGHT: Main screens");
  Serial.println("       TAP: Open apps");
  Serial.println("       UP/DOWN: App pages (on grid)");
}

// =============================================================================
// TOUCH INTERRUPT HANDLER
// =============================================================================

void IRAM_ATTR touchISR() {
  touch_interrupt = true;
}

// =============================================================================
// SPLASH SCREEN
// =============================================================================

void drawSplashScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Animated loading
  int centerX = LCD_WIDTH / 2;
  int centerY = LCD_HEIGHT / 2;
  
  // Logo/Title
  gfx->setTextColor(RGB565(255, 215, 50));
  gfx->setTextSize(2);
  gfx->setCursor(centerX - 80, centerY - 60);
  gfx->print("ANIME WATCH");
  
  gfx->setTextColor(RGB565(100, 180, 255));
  gfx->setTextSize(1);
  gfx->setCursor(centerX - 55, centerY - 30);
  gfx->print("IMPROVED EDITION");
  
  // Loading bar
  gfx->drawRoundRect(centerX - 80, centerY + 30, 160, 20, 10, RGB565(80, 80, 90));
  
  for (int i = 0; i <= 150; i += 5) {
    gfx->fillRoundRect(centerX - 75, centerY + 35, i, 10, 5, RGB565(100, 200, 150));
    delay(15);
  }
  
  // Features
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setTextSize(1);
  gfx->setCursor(centerX - 70, centerY + 70);
  gfx->print("10 Character Themes");
  gfx->setCursor(centerX - 60, centerY + 90);
  gfx->print("Gacha Collection");
  gfx->setCursor(centerX - 50, centerY + 110);
  gfx->print("Mini-Games");
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  // Process touch input
  TouchGesture gesture = handleTouchInput();
  
  if (gesture.is_valid && gesture.event != TOUCH_NONE && gesture.event != TOUCH_PRESS) {
    handleTouchGesture(gesture);
  }
  
  // Update dynamic content based on screen
  updateCurrentScreen();
  
  // Reduce CPU usage
  delay(16);  // ~60 FPS
}

// =============================================================================
// TOUCH GESTURE HANDLER
// =============================================================================

void handleTouchGesture(TouchGesture& gesture) {
  Serial.printf("[MAIN] Gesture: %d at (%d, %d)\n", gesture.event, gesture.x, gesture.y);
  
  // Handle based on current screen type
  switch (system_state.current_screen) {
    case SCREEN_WATCHFACE:
    case SCREEN_APP_GRID:
    case SCREEN_CHARACTER_STATS:
      // Main navigation screens - handle swipes and taps
      if (gesture.event == TOUCH_TAP) {
        handleCurrentScreenTouch(gesture);
      } else if (gesture.event >= TOUCH_SWIPE_LEFT && gesture.event <= TOUCH_SWIPE_DOWN) {
        handleSwipeNavigation(gesture.dx, gesture.dy);
      }
      break;
    
    case SCREEN_GACHA:
      if (gesture.event == TOUCH_TAP && gesture.y >= 380) {
        returnToAppGrid();
      } else {
        handleGachaTouch(gesture);
      }
      break;
    
    case SCREEN_TRAINING:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleTrainingMenuTouch(gesture);
      }
      break;
    
    case SCREEN_BOSS_RUSH:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleBossRushTouch(gesture);
      }
      break;
    
    case SCREEN_GAMES:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420 && gesture.x >= 290) {
        returnToAppGrid();
      } else {
        handleGameMenuTouch(gesture);
      }
      break;
    
    case SCREEN_QUESTS:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleQuestTouch(gesture);
      }
      break;
    
    case SCREEN_ELEMENT_TREE:
      // BoBoiBoy Element Tree - handle swipes for page changes and taps for selection
      handleElementTreeTouch(gesture);
      break;
    
    case SCREEN_FUSION_GAME:
      // BoBoiBoy Fusion Minigame
      handleFusionGameTouch(gesture);
      break;
    
    case SCREEN_CHARACTER_GAME:
      // Character-specific minigames
      handleCharacterGameTouch(gesture);
      break;
    
    case SCREEN_SETTINGS:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleSettingsTouch(gesture);
      }
      break;
    
    case SCREEN_THEME_SELECTOR:
      if (gesture.event == TOUCH_TAP && gesture.y >= 410) {
        system_state.current_screen = SCREEN_SETTINGS;
        drawSettingsApp();
      } else {
        handleThemeSelectorTouch(gesture);
        drawThemeSelector();
      }
      break;
    
    case SCREEN_MUSIC:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleMusicTouch(gesture);
      }
      break;
    
    case SCREEN_WEATHER_APP:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      }
      break;
    
    case SCREEN_WIFI_MANAGER:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleWifiManagerTouch(gesture);
      }
      break;
    
    case SCREEN_COLLECTION:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        system_state.current_screen = SCREEN_GACHA;
        drawGachaScreen();
      } else {
        handleCollectionTouch(gesture);
      }
      break;
    
    case SCREEN_FILE_BROWSER:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleFileBrowserTouch(gesture);
      }
      break;
    
    case SCREEN_CALCULATOR:
      if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
        returnToAppGrid();
      } else {
        handleCalculatorTouch(gesture);
      }
      break;
    
    case SCREEN_FLASHLIGHT:
      if (gesture.event == TOUCH_SWIPE_RIGHT || gesture.event == TOUCH_SWIPE_LEFT) {
        returnToAppGrid();
      } else {
        handleFlashlightTouch(gesture);
      }
      break;
    
    default:
      // For any other screen, tap on back button area returns to app grid
      if (gesture.event == TOUCH_TAP && gesture.y >= 400) {
        returnToAppGrid();
      }
      break;
  }
}

// =============================================================================
// SCREEN UPDATE
// =============================================================================

void updateCurrentScreen() {
  static unsigned long lastUpdate = 0;
  
  // Update every second for watch face
  if (system_state.current_screen == SCREEN_WATCHFACE && millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    drawWatchFace();
    drawNavigationIndicators();
  }
  
  // Update games
  if (system_state.current_screen == SCREEN_GAMES) {
    AdvancedGameManager::updateGame();
  }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

WatchTime getCurrentTime() {
  WatchTime t;
  
  // Read from RTC if available
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  if (timeinfo) {
    t.hour = timeinfo->tm_hour;
    t.minute = timeinfo->tm_min;
    t.second = timeinfo->tm_sec;
    t.day = timeinfo->tm_mday;
    t.month = timeinfo->tm_mon + 1;
    t.year = timeinfo->tm_year + 1900;
    t.weekday = timeinfo->tm_wday;
  } else {
    // Fallback
    t.hour = (millis() / 3600000) % 24;
    t.minute = (millis() / 60000) % 60;
    t.second = (millis() / 1000) % 60;
    t.day = 15;
    t.month = 6;
    t.year = 2025;
    t.weekday = 3;
  }
  
  return t;
}

void saveAllData() {
  saveGachaProgress();
  saveGameProgress();
  saveTrainingProgress();
  saveBossProgress();
  saveRPGProgress();
  Serial.println("[SAVE] All data saved");
}
