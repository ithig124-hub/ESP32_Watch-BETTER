/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  ESP32 Watch - FIXED VERSION
 *  
 *  FIXES:
 *  ✅ NO automatic screen timeout (screen stays on)
 *  ✅ Tap to wake (screen turns on when touched)
 *  ✅ Power button toggles screen on/off
 *  ✅ Device keeps running when screen is off
 *  ✅ Swipe animations preserved
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
#include "Arduino_DriveBus_Library.h"
#include <Adafruit_XCA9554.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

// Include all feature modules
#include "config.h"
#include "ui_manager.h"
#include "themes.h"
#include "power_manager.h"

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
//  TOUCH CONTROLLER - Official Waveshare Arduino_DriveBus Library
// ═══════════════════════════════════════════════════════════════════════════════
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

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
  .day = 1,
  .month = 1,
  .year = 2025,
  .dayOfWeek = 0
};

// Power management state - FIXED: Screen timeout disabled
PowerState powerState = {
  .screenOn = true,
  .lastActivityMs = 0,
  .screenTimeoutMs = SCREEN_TIMEOUT_DISABLED,  // DISABLED!
  .batterySaverLevel = BATTERY_SAVER_OFF,
  .currentBrightness = 200,
  .targetBrightness = 200,
  .fadeInProgress = false,
  .lastPwrButtonState = HIGH,
  .pwrButtonDebounceMs = 0,
  .screenTimeoutEnabled = false  // DISABLED!
};

// Hardware flags
bool hasIMU = false;
bool hasRTC = false;
bool hasPMU = false;
bool hasSD = false;

// Time
uint8_t clockHour = 10, clockMinute = 30, clockSecond = 0;

// ═══════════════════════════════════════════════════════════════════════════════
//  SWIPE NAVIGATION STATE (PRESERVED)
// ═══════════════════════════════════════════════════════════════════════════════
int currentNavCategory = NAV_CLOCK;
int currentSubCard = 0;
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
//  TOUCH HANDLING - Using Official Waveshare Arduino_DriveBus Library
// ═══════════════════════════════════════════════════════════════════════════════
volatile bool touchInterrupt = false;

static int32_t lastTouchX = 0;
static int32_t lastTouchY = 0;
static bool lastTouchValid = false;

bool readTouch(int16_t &x, int16_t &y) {
  if (!FT3168->IIC_Interrupt_Flag) {
    return false;
  }

  int32_t touchX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t touchY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (touchX >= 0 && touchX < LCD_WIDTH && touchY >= 0 && touchY < LCD_HEIGHT) {
    x = touchX;
    y = touchY;
    lastTouchX = touchX;
    lastTouchY = touchY;
    lastTouchValid = true;
    return true;
  }

  return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SWIPE NAVIGATION HANDLER (PRESERVED - WITH ANIMATIONS)
// ═══════════════════════════════════════════════════════════════════════════════
void handleSwipeNavigation(SwipeDirection swipe) {
  if (millis() - lastNavigationMs < NAVIGATION_COOLDOWN_MS) return;

  int newNavCategory = currentNavCategory;
  int newSubCard = currentSubCard;

  if (swipe == SWIPE_LEFT) {
    newNavCategory = (currentNavCategory + 1) % NAV_CATEGORY_COUNT;
    newSubCard = 0;
  }
  else if (swipe == SWIPE_RIGHT) {
    newNavCategory = currentNavCategory - 1;
    if (newNavCategory < 0) newNavCategory = NAV_CATEGORY_COUNT - 1;
    newSubCard = 0;
  }
  else if (swipe == SWIPE_DOWN && currentNavCategory == NAV_APPS) {
    if (currentSubCard < 1) newSubCard = currentSubCard + 1;
  }
  else if (swipe == SWIPE_UP && currentNavCategory == NAV_APPS) {
    if (currentSubCard > 0) newSubCard = currentSubCard - 1;
  }

  if (newNavCategory != currentNavCategory || newSubCard != currentSubCard) {
    currentNavCategory = newNavCategory;
    currentSubCard = newSubCard;

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

  // Check touch interrupt flag from official library
  if (FT3168->IIC_Interrupt_Flag) {
    // Read coordinates
    int32_t touchX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    int32_t touchY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

    // Clear interrupt flag
    FT3168->IIC_Interrupt_Flag = false;

    if (touchX >= 0 && touchX < LCD_WIDTH && touchY >= 0 && touchY < LCD_HEIGHT) {
      data->state = LV_INDEV_STATE_PR;
      data->point.x = touchX;
      data->point.y = touchY;

      // Update activity timestamp
      powerState.lastActivityMs = millis();
      watch.lastActivityMs = millis();

      // ═══════════════════════════════════════════════════════════════════
      //  TAP TO WAKE: Wake screen on any touch when screen is off
      // ═══════════════════════════════════════════════════════════════════
      if (!powerState.screenOn) {
        Serial.println("[TOUCH] Tap to wake triggered");
        screenWake();
        return;  // Don't process this touch as input
      }

      // Track touch start for swipe detection
      if (!touchActive) {
        touchActive = true;
        touchStartX = touchX;
        touchStartY = touchY;
        touchStartMs = millis();
      }
      touchCurrentX = touchX;
      touchCurrentY = touchY;
      return;
    }
  }

  // No touch or invalid touch
  data->state = LV_INDEV_STATE_REL;

  // Process swipe on touch release (SWIPE ANIMATIONS)
  if (touchActive) {
    touchActive = false;
    unsigned long touchDuration = millis() - touchStartMs;
    int32_t dx = touchCurrentX - touchStartX;
    int32_t dy = touchCurrentY - touchStartY;

    if (touchDuration < SWIPE_MAX_DURATION) {
      if (abs(dx) > SWIPE_THRESHOLD_MIN && abs(dx) > abs(dy)) {
        pendingSwipe = (dx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
      }
      else if (abs(dy) > SWIPE_THRESHOLD_MIN && abs(dy) > abs(dx)) {
        pendingSwipe = (dy > 0) ? SWIPE_DOWN : SWIPE_UP;
      }
    }
  }
}

void lvgl_tick(void *arg) { lv_tick_inc(2); }

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION - FIXED I2C INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initI2C() {
  // Initialize I2C bus with correct pins from Waveshare reference
  // SDA=GPIO15, SCL=GPIO14, 400kHz
  Wire.begin(IIC_SDA, IIC_SCL);
  Wire.setClock(400000);
  delay(100);  // Give bus time to stabilize
  
  Serial.println("[OK] I2C Bus initialized");
  Serial.printf("     SDA: GPIO%d, SCL: GPIO%d\n", IIC_SDA, IIC_SCL);
  
  // Scan I2C bus for debugging
  Serial.println("[I2C] Scanning bus...");
  int deviceCount = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("     Found device at 0x%02X", addr);
      if (addr == 0x38) Serial.print(" (FT3168 Touch)");
      else if (addr == 0x6B) Serial.print(" (QMI8658 IMU)");
      else if (addr == 0x51) Serial.print(" (PCF85063 RTC)");
      else if (addr == 0x34) Serial.print(" (AXP2101 PMU)");
      else if (addr == 0x20) Serial.print(" (XCA9554 Expander)");
      Serial.println();
      deviceCount++;
    }
  }
  Serial.printf("[I2C] Found %d device(s)\n", deviceCount);
}

void initExpander() {
  // I/O Expander is optional on some boards
  if (expander.begin(0x20, &Wire)) {
    Serial.println("[OK] I/O Expander (XCA9554)");
    expander.pinMode(0, OUTPUT);
    expander.pinMode(1, OUTPUT);
    expander.pinMode(2, OUTPUT);

    // Reset sequence for peripherals
    expander.digitalWrite(0, LOW);
    expander.digitalWrite(1, LOW);
    expander.digitalWrite(2, LOW);
    delay(20);
    expander.digitalWrite(0, HIGH);
    expander.digitalWrite(1, HIGH);
    expander.digitalWrite(2, HIGH);
    delay(50);
  } else {
    Serial.println("[INFO] I/O Expander not found (optional)");
  }
}

void initDisplay() {
  Serial.println("[DISP] Initializing SH8601 QSPI AMOLED...");
  Serial.printf("       Size: %dx%d, CS: GPIO%d, SCLK: GPIO%d\n", 
                LCD_WIDTH, LCD_HEIGHT, LCD_CS, LCD_SCLK);
  
  gfx->begin();
  gfx->setBrightness(powerState.currentBrightness);
  gfx->fillScreen(0x0000);  // Clear to black
  Serial.println("[OK] Display initialized");
}

void initTouch() {
  Serial.println("[TOUCH] Initializing FT3168...");
  Serial.printf("        INT Pin: GPIO%d\n", TP_INT);

  // Configure interrupt pin
  pinMode(TP_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TP_INT), Arduino_IIC_Touch_Interrupt, FALLING);

  int retries = 5;  // Increased retries
  while (retries > 0) {
    if (FT3168->begin()) {
      Serial.println("[OK] Touch controller (FT3168) initialized");

      // Set touch to monitor mode for power efficiency
      FT3168->IIC_Write_Device_State(
        FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
        FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR
      );

      return;
    }

    Serial.printf("[TOUCH] Init attempt %d failed, retrying...\n", 6 - retries);
    delay(200);  // Longer delay between retries
    retries--;
  }

  Serial.println("[WARN] Touch controller not responding - check wiring!");
  Serial.println("       Expected: FT3168 at I2C address 0x38");
}

void initPMU() {
  Serial.println("[PMU] Initializing AXP2101...");
  
  // Try to initialize PMU
  if (pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
    hasPMU = true;
    
    // Disable all IRQs during setup
    pmu.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    
    // Configure charging
    pmu.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);
    pmu.enableBattDetection();
    pmu.enableBattVoltageMeasure();

    // Configure power rails for peripherals
    pmu.setALDO1Voltage(1800); pmu.enableALDO1();  // 1.8V rail
    pmu.setALDO2Voltage(2800); pmu.enableALDO2();  // 2.8V rail
    pmu.setALDO3Voltage(3300); pmu.enableALDO3();  // 3.3V rail
    pmu.setALDO4Voltage(3300); pmu.enableALDO4();  // 3.3V rail
    pmu.setBLDO1Voltage(1800); pmu.enableBLDO1();  // 1.8V rail
    pmu.setBLDO2Voltage(3300); pmu.enableBLDO2();  // 3.3V rail

    Serial.println("[OK] PMU (AXP2101) configured");
    Serial.printf("     Battery: %d%%, Charging: %s\n", 
                  pmu.getBatteryPercent(),
                  pmu.isCharging() ? "Yes" : "No");
  } else {
    Serial.println("[INFO] PMU not found (optional on some boards)");
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
  Serial.println("[SD] Initializing SD Card...");
  Serial.printf("     CLK: GPIO%d, CMD: GPIO%d, DATA: GPIO%d\n",
                SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);
  
  if (SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA)) {
    if (SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT)) {
      hasSD = true;
      Serial.printf("[OK] SD Card mounted (%llu MB)\n", SD_MMC.cardSize() / (1024 * 1024));
    } else {
      Serial.println("[INFO] SD Card not inserted");
    }
  } else {
    Serial.println("[WARN] SD Card pins config failed");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SETUP - FIXED INITIALIZATION ORDER
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial and power stabilization

  Serial.println("\n═══════════════════════════════════════════════");
  Serial.println("  ESP32 Watch - FIXED VERSION");
  Serial.println("  Pin config from Waveshare reference");
  Serial.println("═══════════════════════════════════════════════\n");

  // 1. Initialize I2C bus first (required for all I2C devices)
  initI2C();
  
  // 2. Initialize I/O expander (may control power to other peripherals)
  initExpander();
  
  // 3. Initialize PMU (controls power rails)
  initPMU();
  
  // Small delay after PMU to let power rails stabilize
  delay(50);
  
  // 4. Initialize display
  initDisplay();
  
  // 5. Initialize touch controller
  initTouch();
  
  // 6. Initialize LVGL graphics library
  initLVGL();
  
  // 7. Initialize SD card (optional)
  initSD();
  
  // 8. Initialize power button
  initPowerButton();

  // 9. Initialize themes
  initThemes();

  // 10. Load saved user data
  loadUserData();

  // 11. Show clock screen
  showScreen(SCREEN_CLOCK);

  // Reset activity timers
  powerState.lastActivityMs = millis();
  watch.lastActivityMs = millis();

  Serial.println("\n═══════════════════════════════════════════════");
  Serial.println("  Boot Complete!");
  Serial.printf("  Touch: %s | SD: %s | PMU: %s\n", 
                FT3168->IIC_Interrupt_Flag ? "ERR" : "OK",
                hasSD ? "OK" : "NO", 
                hasPMU ? "OK" : "NO");
  Serial.println("  Screen Timeout: DISABLED");
  Serial.println("  Tap to Wake: ENABLED");
  Serial.println("  Power Button: ENABLED");
  Serial.println("═══════════════════════════════════════════════\n");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  lv_timer_handler();

  // ═══════════════════════════════════════════════════════════════════════════
  //  POWER BUTTON: Toggle screen on/off
  // ═══════════════════════════════════════════════════════════════════════════
  handlePowerButton();

  // ═══════════════════════════════════════════════════════════════════════════
  //  SCREEN TIMEOUT: DISABLED - screen stays on until button press
  // ═══════════════════════════════════════════════════════════════════════════
  handleScreenTimeout();  // Does nothing when timeout disabled

  // Handle brightness fade animations
  handleBrightnessFade();

  // ═══════════════════════════════════════════════════════════════════════════
  //  SWIPE NAVIGATION (PRESERVED)
  // ═══════════════════════════════════════════════════════════════════════════
  if (pendingSwipe != SWIPE_NONE) {
    handleSwipeNavigation(pendingSwipe);
    pendingSwipe = SWIPE_NONE;
  }

  // ═══════════════════════════════════════════════════════════════════════════
  //  TAP TO WAKE: Check if touch occurred while screen is off
  // ═══════════════════════════════════════════════════════════════════════════
  if (!powerState.screenOn && FT3168->IIC_Interrupt_Flag) {
    Serial.println("[WAKE] Touch detected - waking screen");
    FT3168->IIC_Interrupt_Flag = false;
    screenWake();
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

  // Update clock display
  static unsigned long lastClockUpdate = 0;
  if (watch.screen == SCREEN_CLOCK && millis() - lastClockUpdate > 1000) {
    lastClockUpdate = millis();
    updateClock();
  }

  delay(5);
}
