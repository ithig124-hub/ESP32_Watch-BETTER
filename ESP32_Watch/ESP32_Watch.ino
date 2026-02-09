/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  ESP32 Watch - ANIME EDITION
 *  Enhanced with Random Daily Character Themes
 *  
 *  THEMES:
 *  - Gear 5 Luffy: Clean Fun, bright whites, pastels (NO BOUNCING)
 *  - Jin-Woo Shadow: Dark Power, purple/black, minimal
 *  - Yugo Portal: Chill Exploration, teal/cyan, magical calm
 *  - Random Daily: Rotates through 7 anime characters
 *    • Naruto (Sage Mode)
 *    • Goku (Super Saiyan)
 *    • Tanjiro (Sun Breathing)
 *    • Gojo (Infinity)
 *    • Levi (ODM Gear)
 *    • Saitama (Hero Training)
 *    • Deku (One For All)
 *  
 *  FEATURES:
 *  ✓ 50 Level RPG System (shortened progression)
 *  ✓ Gacha Simulator with anime characters
 *  ✓ Training Mini-games
 *  ✓ Boss Rush Mode
 *  ✓ Clean swipe navigation
 *  ✓ Anime-style UI elements (static, no bounce)
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include <lvgl.h>
#include <Wire.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>
#include <SD_MMC.h>

#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include <Adafruit_XCA9554.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

// Include all modules
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
  .dailyCharacter = CHAR_NARUTO,
  .lastRandomDay = 0,
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
  .day = 8,
  .month = 2,
  .year = 2026,
  .dayOfWeek = 0
};

StopwatchData stopwatch = {0, 0, {0}, 0, false, false, 0};

// Hardware flags
bool hasIMU = false;
bool hasRTC = false;
bool hasPMU = false;
bool hasSD = false;

uint8_t clockHour = 10, clockMinute = 30, clockSecond = 0;

// Navigation
int currentNavCategory = NAV_CLOCK;
int currentSubCard = 0;
volatile SwipeDirection pendingSwipe = SWIPE_NONE;
unsigned long lastNavigationMs = 0;

// Touch tracking
bool touchActive = false;
int32_t touchStartX = 0;
int32_t touchStartY = 0;
int32_t touchCurrentX = 0;
int32_t touchCurrentY = 0;
unsigned long touchStartMs = 0;

// ═══════════════════════════════════════════════════════════════════════════════
//  TOUCH HANDLING
// ═══════════════════════════════════════════════════════════════════════════════
volatile bool touchInterrupt = false;
void IRAM_ATTR touchISR() { touchInterrupt = true; }

bool readTouch(int16_t &x, int16_t &y) {
  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) return false;

  Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)5);
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
//  SWIPE NAVIGATION (Clean, no bounce)
// ═══════════════════════════════════════════════════════════════════════════════
void handleSwipeNavigation(SwipeDirection swipe) {
  if (millis() - lastNavigationMs < NAVIGATION_COOLDOWN_MS) return;

  int newNavCategory = currentNavCategory;
  int newSubCard = currentSubCard;

  if (swipe == SWIPE_LEFT) {
    newNavCategory = (currentNavCategory + 1) % NAV_CATEGORY_COUNT;
    newSubCard = 0;
  } else if (swipe == SWIPE_RIGHT) {
    newNavCategory = currentNavCategory - 1;
    if (newNavCategory < 0) newNavCategory = NAV_CATEGORY_COUNT - 1;
    newSubCard = 0;
  } else if (swipe == SWIPE_DOWN && currentNavCategory == NAV_APPS) {
    if (currentSubCard < 1) newSubCard = currentSubCard + 1;
  } else if (swipe == SWIPE_UP && currentNavCategory == NAV_APPS) {
    if (currentSubCard > 0) newSubCard = currentSubCard - 1;
  }

  if (newNavCategory != currentNavCategory || newSubCard != currentSubCard) {
    currentNavCategory = newNavCategory;
    currentSubCard = newSubCard;

    ScreenType targetScreen = SCREEN_CLOCK;
    switch (currentNavCategory) {
      case NAV_CLOCK:      targetScreen = SCREEN_CLOCK; break;
      case NAV_APPS:       targetScreen = (currentSubCard == 0) ? SCREEN_APPS : SCREEN_APPS2; break;
      case NAV_CHAR_STATS: targetScreen = SCREEN_CHAR_STATS; break;
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

    if (!watch.screenOn) {
      gfx->displayOn();
      watch.screenOn = true;
      return;
    }

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

    if (touchActive) {
      touchActive = false;
      unsigned long touchDuration = millis() - touchStartMs;
      int32_t dx = touchCurrentX - touchStartX;
      int32_t dy = touchCurrentY - touchStartY;

      if (touchDuration < SWIPE_MAX_DURATION) {
        if (abs(dx) > SWIPE_THRESHOLD_MIN && abs(dx) > abs(dy)) {
          pendingSwipe = (dx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
        } else if (abs(dy) > SWIPE_THRESHOLD_MIN && abs(dy) > abs(dx)) {
          pendingSwipe = (dy > 0) ? SWIPE_DOWN : SWIPE_UP;
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
  Serial.println("[OK] I2C Bus");
}

void initExpander() {
  if (expander.begin(EXPANDER_ADDR, &Wire)) {
    expander.pinMode(0, OUTPUT);
    expander.pinMode(1, OUTPUT);
    expander.pinMode(2, OUTPUT);
    expander.digitalWrite(0, LOW);
    expander.digitalWrite(1, LOW);
    expander.digitalWrite(2, LOW);
    delay(20);
    expander.digitalWrite(0, HIGH);
    expander.digitalWrite(1, HIGH);
    expander.digitalWrite(2, HIGH);
    delay(50);
    Serial.println("[OK] I/O Expander");
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
  Wire.beginTransmission(TOUCH_ADDR);
  if (Wire.endTransmission() == 0) {
    attachInterrupt(digitalPinToInterrupt(TP_INT), touchISR, FALLING);
    Serial.println("[OK] Touch");
  }
}

void initPMU() {
  if (pmu.begin(Wire, PMU_ADDR, IIC_SDA, IIC_SCL)) {
    hasPMU = true;
    pmu.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    pmu.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);
    pmu.enableBattDetection();
    pmu.enableBattVoltageMeasure();
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
  Serial.println("  ESP32 Watch - ANIME EDITION");
  Serial.println("  Random Daily Character Themes");
  Serial.println("═══════════════════════════════════════════════\n");

  initI2C();
  initExpander();
  initPMU();
  initDisplay();
  initTouch();
  initLVGL();
  initSD();

  // Load user data and initialize modules
  loadUserData();
  initThemes();
  initApps();
  initGames();
  initRPG();
  initWifiApps();

  // Show clock screen
  showScreen(SCREEN_CLOCK);
  watch.lastActivityMs = millis();

  Serial.println("\n═══════════════════════════════════════════════");
  Serial.println("  Boot Complete!");
  Serial.printf("  Theme: %s\n", getThemeName(watch.theme));
  if (watch.theme == THEME_RANDOM) {
    Serial.printf("  Today's Character: %s\n", getRandomCharacterName(watch.dailyCharacter));
  }
  Serial.println("═══════════════════════════════════════════════\n");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  lv_timer_handler();

  // Process pending swipes
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

  // Update daily character check (once per minute)
  static unsigned long lastDailyCheck = 0;
  if (watch.theme == THEME_RANDOM && millis() - lastDailyCheck > 60000) {
    lastDailyCheck = millis();
    updateDailyCharacter();
  }

  // Update clock display (every second)
  static unsigned long lastClockUpdate = 0;
  if (watch.screen == SCREEN_CLOCK && millis() - lastClockUpdate > 1000) {
    lastClockUpdate = millis();
    watch.second++;
    if (watch.second >= 60) {
      watch.second = 0;
      watch.minute++;
      if (watch.minute >= 60) {
        watch.minute = 0;
        watch.hour++;
        if (watch.hour >= 24) watch.hour = 0;
      }
    }
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
