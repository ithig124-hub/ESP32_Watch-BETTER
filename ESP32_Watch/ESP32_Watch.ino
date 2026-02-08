/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  ESP32 Watch - COMPLETE Edition
 *  All Features from ESP32_Watch merged with BETTER's clean code
 *  ESP32-S3-Touch-AMOLED-1.8" Smartwatch Firmware
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *  FEATURES:
 *  ✅ Clock with multiple watch faces (Luffy, JinWoo, Yugo themes)
 *  ✅ Step counter with activity tracking
 *  ✅ RPG progression system (Solo Leveling inspired)
 *  ✅ 8 Mini-games (Battle Arena, Snake, Memory, Pong, etc.)
 *  ✅ Music player (SD card)
 *  ✅ Weather & News apps (WiFi)
 *  ✅ Quest system with rewards
 *  ✅ Wallpaper/Theme selector
 *  ✅ Touch navigation with LVGL
 *  ✅ Power management (AXP2101)
 *  ✅ Calculator, Flashlight, Coin Flip, Stopwatch utilities
 *  ✅ File Browser (SD card)
 *  ✅ RTC time keeping (PCF85063)
 *  ✅ IMU step detection (QMI8658)
 *
 *  HARDWARE:
 *  - Waveshare ESP32-S3-Touch-AMOLED-1.8
 *  - Display: SH8601 QSPI AMOLED 368x448
 *  - Touch: FT3168 (I2C 0x38)
 *  - IMU: QMI8658 (I2C 0x6B)
 *  - RTC: PCF85063 (I2C 0x51)
 *  - PMU: AXP2101 (I2C 0x34)
 *  - I/O Expander: XCA9554 (I2C 0x20)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include <lvgl.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <SD_MMC.h>

#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include <Adafruit_XCA9554.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

// Include all feature modules
#include "config.h"
#include "ui_manager.h"
#include "themes.h"
#include "apps.h"
#include "games.h"
#include "rpg.h"
#include "wifi_apps.h"
#include "utilities.h"
#include "sensors.h"
#include "rtc.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  HARDWARE OBJECTS
// ═══════════════════════════════════════════════════════════════════════════════
Adafruit_XCA9554 expander;
XPowersAXP2101 pmu;
Preferences prefs;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
Arduino_SH8601 *gfx = new Arduino_SH8601(
  bus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT);

// ═══════════════════════════════════════════════════════════════════════════════
//  LVGL BUFFERS
// ═══════════════════════════════════════════════════════════════════════════════
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;
static lv_color_t *buf2 = nullptr;

// ═══════════════════════════════════════════════════════════════════════════════
//  GLOBAL STATE
// ═══════════════════════════════════════════════════════════════════════════════
WatchState watch = {
  .screen = SCREEN_CLOCK,
  .theme = THEME_LUFFY,
  .batteryPercent = 100,
  .isCharging = false,
  .steps = 0,
  .stepGoal = 10000,
  .brightness = 200,
  .screenOn = true,
  .lastActivityMs = 0,
  .wifiConnected = false,
  .hour = 10,
  .minute = 30,
  .second = 0,
  .day = 1,
  .month = 1,
  .year = 2025,
  .dayOfWeek = 0
};

StopwatchData stopwatch = {0, 0, {0}, 0, false, false, 0};

// Hardware flags
bool hasIMU = false;
bool hasRTC = false;
bool hasPMU = false;
bool hasSD = false;

// Time
uint8_t clockHour = 10, clockMinute = 30, clockSecond = 0;

// ═══════════════════════════════════════════════════════════════════════════════
//  SWIPE NAVIGATION STATE
// ═══════════════════════════════════════════════════════════════════════════════
int currentNavCategory = NAV_CLOCK;  // 0=Clock, 1=Apps, 2=CharStats
int currentSubCard = 0;              // For App Grids: 0=Grid1, 1=Grid2
volatile SwipeDirection pendingSwipe = SWIPE_NONE;
unsigned long lastNavigationMs = 0;

// Touch tracking variables
bool touchActive = false;
int32_t touchStartX = 0;
int32_t touchStartY = 0;
int32_t touchCurrentX = 0;
int32_t touchCurrentY = 0;
unsigned long touchStartMs = 0;

// ═══════════════════════════════════════════════════════════════════════════════
//  TOUCH HANDLING WITH SWIPE DETECTION
// ═══════════════════════════════════════════════════════════════════════════════
volatile bool touchInterrupt = false;
void IRAM_ATTR touchISR() { touchInterrupt = true; }

bool readTouch(int16_t &x, int16_t &y) {
  Wire.beginTransmission(0x38);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) return false;
  
  Wire.requestFrom((uint8_t)0x38, (uint8_t)5);
  if (Wire.available() < 5) return false;
  
  uint8_t touches = Wire.read();
  if (touches == 0 || touches > 2) return false;
  
  uint8_t xh = Wire.read();
  uint8_t xl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t yl = Wire.read();
  
  x = ((xh & 0x0F) << 8) | xl;
  y = ((yh & 0x0F) << 8) | yl;
  return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SWIPE NAVIGATION HANDLER
// ═══════════════════════════════════════════════════════════════════════════════
void handleSwipeNavigation(SwipeDirection swipe) {
  if (millis() - lastNavigationMs < NAVIGATION_COOLDOWN_MS) return;
  
  int newNavCategory = currentNavCategory;
  int newSubCard = currentSubCard;
  
  // Horizontal swipes - infinite loop through categories
  if (swipe == SWIPE_LEFT) {
    // Swipe left = move right in navigation (Clock -> Apps -> CharStats -> Clock...)
    newNavCategory = (currentNavCategory + 1) % NAV_CATEGORY_COUNT;
    newSubCard = 0;  // Reset to first sub-card when changing category
    Serial.printf("[NAV] Swipe LEFT -> Category %d\n", newNavCategory);
  }
  else if (swipe == SWIPE_RIGHT) {
    // Swipe right = move left in navigation (...Clock <- Apps <- CharStats <- Clock)
    newNavCategory = currentNavCategory - 1;
    if (newNavCategory < 0) newNavCategory = NAV_CATEGORY_COUNT - 1;
    newSubCard = 0;
    Serial.printf("[NAV] Swipe RIGHT -> Category %d\n", newNavCategory);
  }
  // Vertical swipes - only work in Apps category (Grid 1 <-> Grid 2)
  else if (swipe == SWIPE_DOWN && currentNavCategory == NAV_APPS) {
    // Swipe down from App Grid 1 -> App Grid 2
    if (currentSubCard < 1) {  // Max 2 grids (0 and 1)
      newSubCard = currentSubCard + 1;
      Serial.printf("[NAV] Swipe DOWN -> SubCard %d\n", newSubCard);
    }
  }
  else if (swipe == SWIPE_UP && currentNavCategory == NAV_APPS) {
    // Swipe up from App Grid 2 -> App Grid 1
    if (currentSubCard > 0) {
      newSubCard = currentSubCard - 1;
      Serial.printf("[NAV] Swipe UP -> SubCard %d\n", newSubCard);
    }
  }
  
  // Navigate if changed
  if (newNavCategory != currentNavCategory || newSubCard != currentSubCard) {
    currentNavCategory = newNavCategory;
    currentSubCard = newSubCard;
    
    // Map navigation category to screen
    ScreenType targetScreen = SCREEN_CLOCK;
    switch (currentNavCategory) {
      case NAV_CLOCK:
        targetScreen = SCREEN_CLOCK;
        break;
      case NAV_APPS:
        targetScreen = (currentSubCard == 0) ? SCREEN_APPS : SCREEN_APPS2;
        break;
      case NAV_CHAR_STATS:
        targetScreen = SCREEN_CHAR_STATS;
        break;
    }
    
    showScreen(targetScreen);
    lastNavigationMs = millis();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LVGL CALLBACKS
// ═══════════════════════════════════════════════════════════════════════════════
void lvgl_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
  lv_disp_flush_ready(disp);
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  int16_t x, y;
  bool touching = readTouch(x, y);
  
  if (touching && x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
    watch.lastActivityMs = millis();
    
    // Wake screen on touch
    if (!watch.screenOn) {
      gfx->displayOn();
      watch.screenOn = true;
      return;
    }
    
    // Track touch start
    if (!touchActive) {
      touchActive = true;
      touchStartX = x;
      touchStartY = y;
      touchStartMs = millis();
    }
    touchCurrentX = x;
    touchCurrentY = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
    
    // Process swipe on touch release
    if (touchActive) {
      touchActive = false;
      unsigned long touchDuration = millis() - touchStartMs;
      int32_t dx = touchCurrentX - touchStartX;
      int32_t dy = touchCurrentY - touchStartY;
      
      // Check if it's a swipe (within time limit)
      if (touchDuration < SWIPE_MAX_DURATION) {
        // Horizontal swipe detection
        if (abs(dx) > SWIPE_THRESHOLD_MIN && abs(dx) > abs(dy)) {
          if (dx > 0) {
            pendingSwipe = SWIPE_RIGHT;
          } else {
            pendingSwipe = SWIPE_LEFT;
          }
        }
        // Vertical swipe detection
        else if (abs(dy) > SWIPE_THRESHOLD_MIN && abs(dy) > abs(dx)) {
          if (dy > 0) {
            pendingSwipe = SWIPE_DOWN;
          } else {
            pendingSwipe = SWIPE_UP;
          }
        }
      }
    }
  }
}

void lvgl_tick(void *arg) { lv_tick_inc(2); }

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initI2C() {
  Wire.begin(IIC_SDA, IIC_SCL);
  Wire.setClock(400000);
  delay(50);
  
  Serial.println("[I2C] Scanning...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X found\n", addr);
    }
  }
}

void initExpander() {
  if (expander.begin(0x20, &Wire)) {
    Serial.println("[OK] I/O Expander");
    expander.pinMode(0, OUTPUT);
    expander.pinMode(1, OUTPUT);
    expander.pinMode(2, OUTPUT);
    
    // Reset sequence
    expander.digitalWrite(0, LOW);
    expander.digitalWrite(1, LOW);
    expander.digitalWrite(2, LOW);
    delay(20);
    expander.digitalWrite(0, HIGH);
    expander.digitalWrite(1, HIGH);
    expander.digitalWrite(2, HIGH);
    delay(50);
  }
}

void initDisplay() {
  gfx->begin();
  gfx->setBrightness(watch.brightness);
  gfx->fillScreen(0x0000);
  Serial.println("[OK] Display");
}

void initTouch() {
  pinMode(TP_INT, INPUT_PULLUP);
  Wire.beginTransmission(0x38);
  if (Wire.endTransmission() == 0) {
    attachInterrupt(digitalPinToInterrupt(TP_INT), touchISR, FALLING);
    Serial.println("[OK] Touch");
  }
}

void initPMU() {
  if (pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
    hasPMU = true;
    pmu.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    pmu.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);
    pmu.enableBattDetection();
    pmu.enableBattVoltageMeasure();
    
    // Enable power rails
    pmu.setALDO1Voltage(1800); pmu.enableALDO1();
    pmu.setALDO2Voltage(2800); pmu.enableALDO2();
    pmu.setALDO3Voltage(3300); pmu.enableALDO3();
    pmu.setALDO4Voltage(3300); pmu.enableALDO4();
    pmu.setBLDO1Voltage(1800); pmu.enableBLDO1();
    pmu.setBLDO2Voltage(3300); pmu.enableBLDO2();
    
    Serial.println("[OK] PMU");
  }
}

void initLVGL() {
  lv_init();
  
  size_t bufSize = LCD_WIDTH * 50;
  buf1 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buf2 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  
  if (!buf1) buf1 = (lv_color_t *)malloc(bufSize * sizeof(lv_color_t));
  if (!buf2) buf2 = nullptr;
  
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, bufSize);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  disp_drv.flush_cb = lvgl_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_touch_read;
  lv_indev_drv_register(&indev_drv);
  
  // Tick timer
  const esp_timer_create_args_t timer_args = { .callback = &lvgl_tick, .name = "lvgl" };
  esp_timer_handle_t timer;
  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, 2000);
  
  Serial.println("[OK] LVGL");
}

void initSD() {
  if (SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT)) {
    hasSD = true;
    Serial.println("[OK] SD Card");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n═══════════════════════════════════════════════");
  Serial.println("  ESP32 Watch - COMPLETE Edition");
  Serial.println("  All Features Merged");
  Serial.println("═══════════════════════════════════════════════\n");
  
  initI2C();
  initExpander();
  initPMU();
  initDisplay();
  initTouch();
  initLVGL();
  initSD();
  
  // Initialize sensors (optional - may not be present)
  hasIMU = initSensors();
  hasRTC = initRTC();
  
  // Initialize feature modules
  initThemes();
  initApps();
  initGames();
  initRPG();
  initWifiApps();
  
  // Initialize utility apps
  Calculator::init();
  Flashlight::init();
  CoinFlip::init();
  Stopwatch::init();
  
  // Load saved data
  loadUserData();
  
  // Show clock screen
  showScreen(SCREEN_CLOCK);
  
  watch.lastActivityMs = millis();
  
  Serial.println("\n═══════════════════════════════════════════════");
  Serial.println("  Boot Complete!");
  Serial.printf("  SD: %s | PMU: %s | IMU: %s | RTC: %s\n", 
    hasSD ? "OK" : "NO", hasPMU ? "OK" : "NO",
    hasIMU ? "OK" : "NO", hasRTC ? "OK" : "NO");
  Serial.println("═══════════════════════════════════════════════\n");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  lv_timer_handler();
  
  // Process pending swipe events
  if (pendingSwipe != SWIPE_NONE) {
    handleSwipeNavigation(pendingSwipe);
    pendingSwipe = SWIPE_NONE;
  }
  
  // Update battery (every 5 seconds)
  static unsigned long lastBattUpdate = 0;
  if (hasPMU && millis() - lastBattUpdate > 5000) {
    lastBattUpdate = millis();
    if (pmu.isBatteryConnect()) {
      watch.batteryPercent = pmu.getBatteryPercent();
      watch.isCharging = pmu.isCharging();
    }
  }
  
  // Update RTC time (every second)
  static unsigned long lastRtcUpdate = 0;
  if (hasRTC && millis() - lastRtcUpdate > 1000) {
    lastRtcUpdate = millis();
    readRTC();
  }
  
  // Update step counter (every 100ms)
  static unsigned long lastStepUpdate = 0;
  if (hasIMU && millis() - lastStepUpdate > 100) {
    lastStepUpdate = millis();
    updateStepCounter();
  }
  
  // Update stopwatch
  Stopwatch::update();
  
  // Update games
  updatePong();
  
  // Update clock display
  static unsigned long lastClockUpdate = 0;
  if (watch.screen == SCREEN_CLOCK && millis() - lastClockUpdate > 1000) {
    lastClockUpdate = millis();
    updateClock();
  }
  
  // Screen timeout (30 seconds)
  if (watch.screenOn && millis() - watch.lastActivityMs > 30000) {
    gfx->displayOff();
    watch.screenOn = false;
  }
  
  // Wake on touch interrupt
  if (!watch.screenOn && touchInterrupt) {
    gfx->displayOn();
    watch.screenOn = true;
    watch.lastActivityMs = millis();
    touchInterrupt = false;
  }
  
  delay(5);
}
