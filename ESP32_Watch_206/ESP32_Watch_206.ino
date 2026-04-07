//✅ Timeout changed: 3 seconds → 5 seconds
//✅ Power button changed: GPIO 0 → GPIO 10
//✅ Screen timeout logic: Uses simple 5-second check (bypasses power manager)
/*
 * ESP32_Watch.ino - Main Firmware with Power Management
 * Modern Anime Gaming Smartwatch - Enhanced Edition
 * 
 * PORTED for ESP32-S3-Touch-AMOLED-2.06" (Waveshare)
 * 
 * POWER MANAGEMENT FEATURES:
 * - 5-second screen timeout (display off)
 * - Touch to wake (display on)
 * - Power button (GPIO 10) to toggle screen on/off
 * - Watchdog timer (10 sec) for system stability
 * 
 * NO LIGHT SLEEP / NO DEEP SLEEP - Simple screen on/off only
 * 
 * Hardware: ESP32-S3-Touch-AMOLED-2.06"
 * Display: 410x502 CO5300 AMOLED (QSPI)
 * Touch: FT3168 Capacitive (I2C)
 * PMU: AXP2101
 * RTC: PCF85063
 * IMU: QMI8658
 */

#include <esp_task_wdt.h>

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
#include "wifi_apps.h"
#include "filesystem.h"
#include "boboiboy_elements.h"
#include "ochobot.h"
#include "dynamic_bg.h"
#include "fusion_game.h"
#include "character_games.h"
#include "steps_tracker.h"
#include "daily_quests.h"
#include "ui.h"
#include "sd_manager.h"
#include "power_manager.h"
#include "xp_system.h"
#include "wifi_sync.h"
#include "time_edit.h"

// =============================================================================
// POWER MANAGEMENT DEFINES
// =============================================================================
#define PWR_BUTTON              10      // Power button GPIO (CHANGED from 0)
#define BUTTON_DEBOUNCE_MS      50      // Button debounce time
#define SCREEN_OFF_TIMEOUT_MS   5000    // 5 seconds to turn screen off (CHANGED from 3000)
#define WATCHDOG_TIMEOUT_SEC    10      // Watchdog timeout in seconds

// =============================================================================
// DISPLAY SETUP - CO5300 for 2.06" AMOLED
// =============================================================================
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);

// CO5300 display driver with column offset for 2.06" board
Arduino_CO5300 *gfx = new Arduino_CO5300(
    bus, LCD_RESET, 0 /* rotation */, LCD_WIDTH, LCD_HEIGHT,
    22 /* col_offset1 */, 0 /* row_offset1 */,
    0 /* col_offset2 */, 0 /* row_offset2 */);

// =============================================================================
// POWER MANAGEMENT STATE
// =============================================================================
volatile bool screenOn = true;
volatile bool touchWakeFlag = false;
volatile bool buttonWakeFlag = false;
volatile unsigned long lastActivityMs = 0;
static bool lastPwrButtonState = HIGH;
static unsigned long lastPwrButtonChange = 0;

// =============================================================================
// GLOBAL SYSTEM STATE
// =============================================================================
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

// =============================================================================
// INTERRUPT SERVICE ROUTINES
// =============================================================================

void IRAM_ATTR touchWakeISR() {
    touchWakeFlag = true;
    lastActivityMs = millis();
}

void IRAM_ATTR powerButtonISR() {
    buttonWakeFlag = true;
}

// =============================================================================
// WATCHDOG FUNCTIONS
// =============================================================================

void initWatchdog() {
    Serial.println("[WDT] Initializing watchdog timer...");
    
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT_SEC * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    
    esp_task_wdt_reconfigure(&wdt_config);
    esp_task_wdt_add(xTaskGetCurrentTaskHandle());
    
    Serial.printf("[WDT] Watchdog initialized: %d sec timeout\n", WATCHDOG_TIMEOUT_SEC);
}

void feedWatchdog() {
    esp_task_wdt_reset();
}

// =============================================================================
// SCREEN CONTROL FUNCTIONS
// =============================================================================
void screenOff() {
    if (!screenOn) return;
    
    Serial.println("[POWER] Screen OFF - smooth fade");
    
    screenOn = false;
    
    int currentBrightness = system_state.brightness;
    for (int b = currentBrightness; b >= 0; b -= 20) {
        gfx->setBrightness(max(0, b));
        delay(8);
    }
    gfx->setBrightness(0);
    
    gfx->displayOff();
}

void screenOnFunc() {
    if (screenOn) return;
    
    Serial.println("[POWER] Screen ON - smooth fade");
    
    gfx->displayOn();
    gfx->setBrightness(0);
    
    drawCurrentScreen();
    
    delay(15);
    
    for (int b = 0; b <= system_state.brightness; b += 20) {
        gfx->setBrightness(min(b, (int)system_state.brightness));
        delay(8);
    }
    gfx->setBrightness(system_state.brightness);
    
    screenOn = true;
    lastActivityMs = millis();
}

void checkScreenTimeout() {
    if (!screenOn) return;
    
    unsigned long elapsed = millis() - lastActivityMs;
    if (elapsed >= SCREEN_OFF_TIMEOUT_MS) {
        screenOff();
    }
}

void checkPowerButton() {
    bool currentState = digitalRead(PWR_BUTTON);
    
    // Debounce
    if (currentState != lastPwrButtonState) {
        if (millis() - lastPwrButtonChange > BUTTON_DEBOUNCE_MS) {
            lastPwrButtonChange = millis();
            lastPwrButtonState = currentState;
            
            // Button pressed (LOW because of pullup)
            if (currentState == LOW) {
                Serial.println("[POWER] Power button pressed (GPIO 10)");
                
                // Toggle screen
                if (screenOn) {
                    screenOff();
                } else {
                    screenOnFunc();
                }
                
                lastActivityMs = millis();
            }
        }
    }
    
    // Also check button wake flag from ISR
    if (buttonWakeFlag) {
        buttonWakeFlag = false;
        if (!screenOn) {
            screenOnFunc();
        }
        lastActivityMs = millis();
    }
}

void checkTouchWake() {
    if (touchWakeFlag) {
        touchWakeFlag = false;
        if (!screenOn) {
            screenOnFunc();
        }
        lastActivityMs = millis();
    }
}

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
void updateCurrentScreen();
void handleTouchGesture(TouchGesture& gesture);
void saveAllData();

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n===================================");
  Serial.println(" ESP32 Anime Gaming Watch IMPROVED");
  Serial.println(" With Watchdog & Screen Timeout");
  Serial.println("===================================\n");
  
  initWatchdog();
  feedWatchdog();
  
  initializeHardware();
  feedWatchdog();
  
  initDisplay();
  feedWatchdog();
  
  drawSplashScreen();
  delay(2000);
  feedWatchdog();
  
  if (initTouch()) {
    Serial.println("[INIT] Touch initialized");
    system_state.touch_available = true;
    
    attachInterrupt(digitalPinToInterrupt(TP_INT), touchISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(TP_INT), touchWakeISR, FALLING);
  }
  
  // Initialize power button on GPIO 10
  pinMode(PWR_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PWR_BUTTON), powerButtonISR, FALLING);
  Serial.println("[INIT] Power button: GPIO 10");
  
  initializeThemes();
  feedWatchdog();
  
  loadAllGameData();
  feedWatchdog();
  
  initNavigation();
  
  initializeApps();
  feedWatchdog();
  
  initializeGames();
  feedWatchdog();
  
  initGachaSystem();
  feedWatchdog();
  
  initTrainingSystem();
  
  initBossRush();
  
  initXPSystem();
  checkDailyLoginBonus();
  feedWatchdog();
  
  Serial.println("[INIT] Initializing filesystem...");
  feedWatchdog();
  initFilesystem();
  feedWatchdog();
  
  Serial.println("[INIT] Initializing WiFi sync...");
  feedWatchdog();
  initWiFiSync();
  feedWatchdog();
  
  Serial.println("\n[BOOT] Starting WiFi boot sync (hardcoded only)...");
  feedWatchdog();
  
  unsigned long wifiSyncStart = millis();
  bool wifi_synced = false;
  
  Serial.println("[WiFi] WiFi sync timeout: 8 seconds maximum");
  
  unsigned long beforeSync = millis();
  wifi_synced = performBootSync();
  unsigned long syncDuration = millis() - beforeSync;
  
  Serial.printf("[WiFi] Sync attempt took %lu ms\n", syncDuration);
  
  if (wifi_synced) {
    Serial.println("[BOOT] ✓ WiFi sync successful!");
  } else {
    Serial.println("[BOOT] ✗ WiFi sync failed or timed out");
    Serial.println("[BOOT] → Continuing boot anyway (WiFi optional)");
  }
  
  feedWatchdog();
  
  initWifiApps();
  feedWatchdog();
  
  initStepsTracker();
  initDailyQuests();
  feedWatchdog();
  
  lastActivityMs = millis();
  screenOn = true;
  
  initPowerManager();
  feedWatchdog();
  
  system_state.current_screen = SCREEN_WATCHFACE;
  drawWatchFace();
  drawNavigationIndicators();
  
  Serial.println("\n[INIT] ===== FUSION OS READY! =====");
  Serial.println("       - Screen Timeout: 5 seconds");
  Serial.println("       - Wake: Touch or GPIO 10 button");
  Serial.println("       - Watchdog: 10 seconds");
  Serial.println("       Battery Target: 8-12 hours 🔋");
}

// =============================================================================
// MAIN LOOP - FUSION OS OPTIMIZED
// =============================================================================

void loop() {
  feedWatchdog();
  
  static unsigned long lastAutoSave = 0;
  if (millis() - lastAutoSave > 300000) {
    lastAutoSave = millis();
    saveAllData();
    Serial.println("[AUTO-SAVE] All game data saved (5 min interval)");
  }
  
  updatePowerState();
  
  checkPowerButton();
  
  checkTouchWake();
  
  if (screenOn) {
    TouchGesture gesture = handleTouchInput();
    
    if (gesture.is_valid && gesture.event != TOUCH_NONE) {
      recordInteraction();
      lastActivityMs = millis();  // Reset 5-second timer on touch
      
      extern bool time_edit_active;
      if (time_edit_active && gesture.event == TOUCH_TAP) {
        extern void handleTimeEditTouch(int x, int y);
        handleTimeEditTouch(gesture.x, gesture.y);
      }
      else if (gesture.event != TOUCH_PRESS && gesture.event != TOUCH_MOVE) {
        handleTouchGesture(gesture);
      }
    }
    
    updateCurrentScreen();
    
    static unsigned long lastStepUpdate = 0;
    int step_interval = getSensorPollInterval();
    if (millis() - lastStepUpdate > step_interval) {
      updateStepCount();
      lastStepUpdate = millis();
    }
    
    static unsigned long lastQuestCheck = 0;
    if (millis() - lastQuestCheck > 60000) {
      checkDailyReset();
      lastQuestCheck = millis();
    }
    
    // SIMPLE 5-SECOND TIMEOUT CHECK (bypasses power manager)
    if (millis() - lastActivityMs >= SCREEN_OFF_TIMEOUT_MS) {
      screenOff();
    }
  }
  
  int loop_delay = getPowerLoopDelay();
  delay(loop_delay);
}

// =============================================================================
// TOUCH GESTURE HANDLER
// =============================================================================

void handleTouchGesture(TouchGesture& gesture) {
  Serial.printf("[MAIN] Gesture: %d at (%d, %d)\n", gesture.event, gesture.x, gesture.y);
  
  lastActivityMs = millis();
  
  if (gesture.event == TOUCH_SWIPE_UP) {
    if (system_state.current_screen == SCREEN_CHARACTER_GAME ||
        system_state.current_screen == SCREEN_BOSS_RUSH ||
        system_state.current_screen == SCREEN_GAMES) {
    }
    else {
      switch (system_state.current_screen) {
        case SCREEN_WATCHFACE:
        case SCREEN_STEPS_TRACKER:
          handleSwipeNavigation(gesture.dx, gesture.dy);
          break;
        
        case SCREEN_CHARACTER_STATS:
          system_state.current_screen = SCREEN_PROGRESSION;
          drawProgressionScreen();
          break;
        
        case SCREEN_APP_GRID:
          if (navState.appGridPage < APP_GRID_PAGES - 1) {
            navState.appGridPage++;
            navState.lastNavigationMs = millis();
            drawCurrentScreen();
          }
          break;
        
        case SCREEN_COLLECTION:
        case SCREEN_CARD_EVOLUTION:
        case SCREEN_DECK_BUILDER:
          system_state.current_screen = SCREEN_GACHA;
          drawGachaScreen();
          break;
        
        case SCREEN_THEME_SELECTOR:
          system_state.current_screen = SCREEN_SETTINGS;
          drawSettingsApp();
          break;
        
        default:
          returnToAppGrid();
          break;
      }
      return;
    }
  }
  
  if (gesture.event == TOUCH_SWIPE_DOWN) {
    if (system_state.current_screen == SCREEN_APP_GRID) {
      if (navState.appGridPage > 0) {
        navState.appGridPage--;
        navState.lastNavigationMs = millis();
        drawCurrentScreen();
      }
      return;
    }
  }
  
  switch (system_state.current_screen) {
    case SCREEN_WATCHFACE:
    case SCREEN_APP_GRID:
      if (gesture.event == TOUCH_TAP) {
        handleCurrentScreenTouch(gesture);
      } else if (gesture.event >= TOUCH_SWIPE_LEFT && gesture.event <= TOUCH_SWIPE_DOWN) {
        handleSwipeNavigation(gesture.dx, gesture.dy);
      }
      break;
    
    case SCREEN_CHARACTER_STATS:
      if (gesture.event == TOUCH_TAP) {
        handleCurrentScreenTouch(gesture);
      } else if (gesture.event == TOUCH_SWIPE_LEFT || gesture.event == TOUCH_SWIPE_RIGHT) {
        handleSwipeNavigation(gesture.dx, gesture.dy);
      }
      break;
    
    case SCREEN_PROGRESSION:
      handleProgressionTouch(gesture);
      break;
    
    case SCREEN_STEPS_TRACKER:
      if (gesture.event == TOUCH_TAP) {
        handleStepsCardTouch(gesture);
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
      handleDailyQuestsTouch(gesture);
      break;
    
    case SCREEN_DAILY_QUESTS:
      handleDailyQuestsTouch(gesture);
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
    
    case SCREEN_WIFI_MANAGER:
      handleWifiManagerTouch(gesture);
      break;
    
    case SCREEN_COLLECTION:
      handleCollectionTouch(gesture);
      break;
    
    case SCREEN_CARD_EVOLUTION:
      if (gesture.event == TOUCH_TAP) {
        handleCardEvolutionTap(gesture.x, gesture.y);
      }
      break;
    
    case SCREEN_DECK_BUILDER:
      if (gesture.event == TOUCH_TAP) {
        handleDeckBuilderTap(gesture.x, gesture.y);
      }
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
    
    case SCREEN_TIMER:
      handleTimerTouch(gesture);
      break;
    
    case SCREEN_CONVERTER:
      handleConverterTouch(gesture);
      break;
    
    case SCREEN_ACHIEVEMENTS:
      handleAchievementsTouch(gesture);
      break;
    
    case SCREEN_SHOP:
      handleShopTouch(gesture);
      break;
    
    case SCREEN_GALLERY:
      handleGalleryTouch(gesture);
      break;
    
    default:
      break;
  }
}

// =============================================================================
// SCREEN UPDATE - FUSION OS OPTIMIZED
// =============================================================================

void updateCurrentScreen() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastTimerUpdate = 0;
  static unsigned long lastBatteryUpdate = 0;
  static int last_minute = -1;
  static ScreenType last_screen = SCREEN_SPLASH;
  
  bool screen_changed = (system_state.current_screen != last_screen);
  if (screen_changed) {
    last_screen = system_state.current_screen;
    Serial.printf("[SCREEN] Changed to: %d\n", system_state.current_screen);
    
    extern void forceWatchfaceRedraw();
    forceWatchfaceRedraw();
  }
  
  if (millis() - lastBatteryUpdate > 30000) {
    lastBatteryUpdate = millis();
    updateBatteryStatus();
  }
  
  if (system_state.current_screen == SCREEN_WATCHFACE && 
      navState.currentMain == MAIN_WATCHFACE) {
    WatchTime current_time = getCurrentTime();
    
    if (current_time.minute != last_minute) {
      last_minute = current_time.minute;
      
      updateWatchFaceTime();
      
      Serial.printf("[CLOCK] Minute changed → %02d:%02d\n", 
                    current_time.hour, current_time.minute);
    }
  }
  
  if (system_state.current_screen == SCREEN_TIMER &&
      millis() - lastTimerUpdate > 50) {
    lastTimerUpdate = millis();
    updateTimerDisplay();
  }
  
  if (system_state.current_screen == SCREEN_GAMES && shouldAnimationsRun()) {
    AdvancedGameManager::updateGame();
  }
}

// =============================================================================
// SAVE ALL DATA - Persistent Storage using Preferences (NVS)
// =============================================================================

#include <Preferences.h>
Preferences gamePrefs;

void saveAllGameData() {
  gamePrefs.begin("watchgame", false);
  
  gamePrefs.putInt("gems", system_state.player_gems);
  gamePrefs.putInt("level", system_state.player_level);
  gamePrefs.putInt("xp", system_state.player_xp);
  gamePrefs.putInt("theme", (int)system_state.current_theme);
  
  gamePrefs.putInt("cards", system_state.gacha_cards_collected);
  gamePrefs.putInt("bosses", system_state.bosses_defeated);
  gamePrefs.putInt("streak", system_state.training_streak);
  gamePrefs.putInt("logins", system_state.daily_login_count);
  gamePrefs.putInt("pity", system_state.pity_counter);
  gamePrefs.putInt("pityleg", system_state.pity_legendary_counter);
  
  gamePrefs.putInt("steps", system_state.steps_today);
  
  gamePrefs.putInt("bright", system_state.brightness);
  
  gamePrefs.end();
  
  Serial.println("[SAVE] Game data saved to NVS");
  Serial.printf("       Gems: %d, Level: %d, XP: %d, Theme: %d\n", 
    system_state.player_gems, system_state.player_level, 
    system_state.player_xp, system_state.current_theme);
}

void loadAllGameData() {
  gamePrefs.begin("watchgame", true);
  
  system_state.player_gems = gamePrefs.getInt("gems", 500);
  system_state.player_level = gamePrefs.getInt("level", 1);
  system_state.player_xp = gamePrefs.getInt("xp", 0);
  system_state.current_theme = (ThemeType)gamePrefs.getInt("theme", THEME_LUFFY_GEAR5);
  
  system_state.gacha_cards_collected = gamePrefs.getInt("cards", 0);
  system_state.bosses_defeated = gamePrefs.getInt("bosses", 0);
  system_state.training_streak = gamePrefs.getInt("streak", 0);
  system_state.daily_login_count = gamePrefs.getInt("logins", 0);
  system_state.pity_counter = gamePrefs.getInt("pity", 0);
  system_state.pity_legendary_counter = gamePrefs.getInt("pityleg", 0);
  
  system_state.steps_today = gamePrefs.getInt("steps", 0);
  
  system_state.brightness = gamePrefs.getInt("bright", 200);
  
  gamePrefs.end();
  
  setTheme(system_state.current_theme);
  
  Serial.println("[LOAD] Game data loaded from NVS");
  Serial.printf("       Gems: %d, Level: %d, XP: %d, Theme: %d\n", 
    system_state.player_gems, system_state.player_level, 
    system_state.player_xp, system_state.current_theme);
}

void saveAllData() {
  saveAllGameData();
  saveGachaProgress();
  saveGameProgress();
  saveTrainingProgress();
  saveBossProgress();
  saveXPData();
  saveStepsData();
  Serial.println("[SAVE] All data saved");
}