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
#include "sd_manager.h"

// Display - MUST be properly allocated (not nullptr!)
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */);

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
// sdCardInitialized and wifiConnected are defined in sd_manager.cpp
// and declared extern in sd_manager.h

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

// touchISR() is defined in touch.cpp

// =============================================================================
// SPLASH SCREEN - defined in ui.cpp
// =============================================================================

// Forward declarations for functions defined after loop()
void updateCurrentScreen();
void handleTouchGesture(TouchGesture& gesture);

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
  
  // ==========================================================
  // SWIPE UP = EXIT from any app back to app grid (Apple Watch style)
  // ==========================================================
  if (gesture.event == TOUCH_SWIPE_UP) {
    switch (system_state.current_screen) {
      // Main navigation screens - swipe up changes page or navigates
      case SCREEN_WATCHFACE:
      case SCREEN_CHARACTER_STATS:
        handleSwipeNavigation(gesture.dx, gesture.dy);
        break;
      
      // App grid - swipe up goes to page 2, or exits if already on page 2
      case SCREEN_APP_GRID:
        if (navState.appGridPage == 0) {
          navState.appGridPage = 1;
          navState.lastNavigationMs = millis();
          drawCurrentScreen();
        }
        // Already on page 2 - do nothing (swipe down goes back to page 1)
        break;
      
      // Collection goes back to Gacha
      case SCREEN_COLLECTION:
        system_state.current_screen = SCREEN_GACHA;
        drawGachaScreen();
        break;
      
      // Theme selector goes back to Settings
      case SCREEN_THEME_SELECTOR:
        system_state.current_screen = SCREEN_SETTINGS;
        drawSettingsApp();
        break;
      
      // ALL other screens → exit to app grid
      default:
        returnToAppGrid();
        break;
    }
    return;
  }
  
  // ==========================================================
  // Handle other gestures per screen
  // ==========================================================
  switch (system_state.current_screen) {
    case SCREEN_WATCHFACE:
    case SCREEN_APP_GRID:
    case SCREEN_CHARACTER_STATS:
      if (gesture.event == TOUCH_TAP) {
        handleCurrentScreenTouch(gesture);
      } else if (gesture.event >= TOUCH_SWIPE_LEFT && gesture.event <= TOUCH_SWIPE_DOWN) {
        handleSwipeNavigation(gesture.dx, gesture.dy);
      }
      break;
    
    case SCREEN_GACHA:
      handleGachaTouch(gesture);
      break;
    
    case SCREEN_TRAINING:
      handleTrainingMenuTouch(gesture);
      break;
    
    case SCREEN_BOSS_RUSH:
      handleBossRushTouch(gesture);
      break;
    
    case SCREEN_GAMES:
      handleGameMenuTouch(gesture);
      break;
    
    case SCREEN_QUESTS:
      handleQuestTouch(gesture);
      break;
    
    case SCREEN_ELEMENT_TREE:
      handleElementTreeTouch(gesture);
      break;
    
    case SCREEN_FUSION_GAME:
      handleFusionGameTouch(gesture);
      break;
    
    case SCREEN_CHARACTER_GAME:
      handleCharacterGameTouch(gesture);
      break;
    
    case SCREEN_SETTINGS:
      handleSettingsTouch(gesture);
      break;
    
    case SCREEN_THEME_SELECTOR:
      handleThemeSelectorTouch(gesture);
      break;
    
    case SCREEN_MUSIC:
      handleMusicTouch(gesture);
      break;
    
    case SCREEN_WEATHER_APP:
      // Weather is display-only, taps ignored
      break;
    
    case SCREEN_WIFI_MANAGER:
      handleWifiManagerTouch(gesture);
      break;
    
    case SCREEN_COLLECTION:
      handleCollectionTouch(gesture);
      break;
    
    case SCREEN_FILE_BROWSER:
      handleFileBrowserTouch(gesture);
      break;
    
    case SCREEN_CALCULATOR:
      handleCalculatorTouch(gesture);
      break;
    
    case SCREEN_FLASHLIGHT:
      handleFlashlightTouch(gesture);
      break;
    
    default:
      break;
  }
}

// =============================================================================
// SCREEN UPDATE
// =============================================================================

void updateCurrentScreen() {
  static unsigned long lastUpdate = 0;
  
  // Update watchface - use PARTIAL update to avoid flicker
  if (system_state.current_screen == SCREEN_WATCHFACE && millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    updateWatchFaceTime();  // Only redraw time area, no fillScreen!
  }
  
  // Update games
  if (system_state.current_screen == SCREEN_GAMES) {
    AdvancedGameManager::updateGame();
  }
}

// getCurrentTime() is defined in hardware.cpp

void saveAllData() {
  saveGachaProgress();
  saveGameProgress();
  saveTrainingProgress();
  saveBossProgress();
  saveRPGProgress();
  Serial.println("[SAVE] All data saved");
}
