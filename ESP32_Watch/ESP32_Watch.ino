/*
 * ESP32 Watch - Simplified Edition
 * All features consolidated into clean, modular code
 * 
 * Hardware: ESP32-S3-Touch-AMOLED-1.8 (Waveshare)
 * Features: RPG System, Games, Themes, WiFi, Music, Weather, Quests
 * 
 * Original: https://github.com/ithig124-hub/ESP32_Watch
 */

#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <esp_heap_caps.h>
#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include <Adafruit_XCA9554.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

#include "config.h"
#include "display.h"
#include "touch.h"
#include "hardware.h"
#include "themes.h"
#include "games.h"
#include "apps.h"
#include "wifi_apps.h"
#include "filesystem.h"
#include "rpg.h"
#include "ui.h"

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

XPowersAXP2101 PMU;
SystemState system_state;
Adafruit_XCA9554 expander;

// Display objects
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
Arduino_SH8601 *gfx = new Arduino_SH8601(
  bus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT);

// LVGL buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;
static lv_color_t *buf2 = nullptr;

// Touch interrupt
volatile bool touch_interrupt = false;
void IRAM_ATTR touch_isr() { touch_interrupt = true; }

// =============================================================================
// TOUCH HANDLING
// =============================================================================

bool readTouch(int16_t &x, int16_t &y) {
  Wire.beginTransmission(FT3168_ADDR);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) return false;
  
  Wire.requestFrom((uint8_t)FT3168_ADDR, (uint8_t)5);
  if (Wire.available() < 5) return false;
  
  uint8_t touches = Wire.read();
  if (touches == 0 || touches > 2) return false;
  
  uint8_t xh = Wire.read(), xl = Wire.read();
  uint8_t yh = Wire.read(), yl = Wire.read();
  
  x = ((xh & 0x0F) << 8) | xl;
  y = ((yh & 0x0F) << 8) | yl;
  return true;
}

// =============================================================================
// LVGL CALLBACKS
// =============================================================================

void lvgl_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
  lv_disp_flush_ready(disp);
}

void lvgl_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (!touch_interrupt) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  
  int16_t x, y;
  if (readTouch(x, y) && x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
    touch_interrupt = false;
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
    system_state.touch_active = true;
    system_state.sleep_timer = millis();
  } else {
    touch_interrupt = false;
    data->state = LV_INDEV_STATE_REL;
    system_state.touch_active = false;
  }
}

#define LVGL_TICK_PERIOD_MS 2
void lvgl_tick_increment(void *arg) { lv_tick_inc(LVGL_TICK_PERIOD_MS); }

// =============================================================================
// I2C SCAN
// =============================================================================

void scanI2C() {
  Serial.println("\n=== I2C Bus Scan ===");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X - ", addr);
      switch(addr) {
        case 0x20: Serial.println("XCA9554 GPIO Expander"); break;
        case 0x34: Serial.println("AXP2101 PMU"); break;
        case 0x38: Serial.println("FT3168 Touch"); break;
        case 0x51: Serial.println("PCF85063 RTC"); break;
        case 0x6A:
        case 0x6B: Serial.println("QMI8658 IMU"); break;
        case 0x18: Serial.println("ES8311 Audio"); break;
        default: Serial.println("Unknown"); break;
      }
      found++;
    }
  }
  Serial.printf("=== Found %d devices ===\n\n", found);
}

// =============================================================================
// SYSTEM INITIALIZATION
// =============================================================================

void initSystemState() {
  system_state.current_screen = SCREEN_SPLASH;
  system_state.current_theme = THEME_LUFFY_GEAR5;
  system_state.current_app = APP_WATCHFACE;
  system_state.battery_percentage = 100;
  system_state.is_charging = false;
  system_state.brightness = 200;
  system_state.wifi_connected = false;
  system_state.steps_today = 0;
  system_state.step_goal = 10000;
  system_state.sleep_timer = millis();
}

void initPMU() {
  if (PMU.begin(Wire, AXP2101_ADDR, IIC_SDA, IIC_SCL)) {
    Serial.println("[PMU] AXP2101 initialized OK");
    system_state.power_available = true;
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);
    PMU.enableBattDetection();
    PMU.enableBattVoltageMeasure();
    
    // Enable power rails for display
    PMU.setALDO1Voltage(1800); PMU.enableALDO1();
    PMU.setALDO2Voltage(2800); PMU.enableALDO2();
    PMU.setALDO3Voltage(3300); PMU.enableALDO3();
    PMU.setALDO4Voltage(3300); PMU.enableALDO4();
    PMU.setBLDO1Voltage(1800); PMU.enableBLDO1();
    PMU.setBLDO2Voltage(3300); PMU.enableBLDO2();
    
    delay(100);
    
    if (PMU.isBatteryConnect()) {
      system_state.battery_percentage = PMU.getBatteryPercent();
      system_state.is_charging = PMU.isCharging();
    }
  } else {
    Serial.println("[PMU] AXP2101 not found");
    system_state.power_available = false;
  }
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n========================================");
  Serial.println("  ESP32 Watch - Simplified Edition");
  Serial.println("  All features, cleaner code!");
  Serial.println("========================================\n");

  initSystemState();
  
  // Audio amp enable
  pinMode(PA_PIN, OUTPUT);
  digitalWrite(PA_PIN, HIGH);

  // Step 1: I2C Bus
  Serial.println("[INIT] Step 1: I2C Bus");
  Wire.begin(IIC_SDA, IIC_SCL);
  Wire.setClock(400000);
  delay(100);
  scanI2C();

  // Step 2: I/O Expander
  Serial.println("[INIT] Step 2: I/O Expander");
  if (expander.begin(EXPANDER_ADDR, &Wire)) {
    Serial.println("[OK] XCA9554 found");
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
    Serial.println("[OK] Display power enabled");
  } else {
    Serial.println("[FAIL] XCA9554 not found!");
  }

  // Step 3: Display
  Serial.println("[INIT] Step 3: Display");
  gfx->begin();
  gfx->setBrightness(255);
  delay(100);
  gfx->fillScreen(0x0000);
  Serial.println("[OK] Display initialized");
  system_state.display_available = true;

  // Step 4: Touch
  Serial.println("[INIT] Step 4: Touch Controller");
  pinMode(TP_INT, INPUT_PULLUP);
  Wire.beginTransmission(FT3168_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("[OK] Touch detected");
    system_state.touch_available = true;
    attachInterrupt(digitalPinToInterrupt(TP_INT), touch_isr, FALLING);
  } else {
    Serial.println("[WARN] Touch not responding");
    system_state.touch_available = false;
  }

  // Step 5: PMU
  Serial.println("[INIT] Step 5: Power Management");
  initPMU();

  // Step 6: LVGL
  Serial.println("[INIT] Step 6: LVGL");
  lv_init();
  
  size_t buf_size = LCD_WIDTH * 50;
  buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buf2 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  
  if (!buf1 || !buf2) {
    if (buf1) heap_caps_free(buf1);
    if (buf2) heap_caps_free(buf2);
    buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    buf2 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }
  
  if (buf1) {
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buf_size);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = lvgl_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    if (system_state.touch_available) {
      static lv_indev_drv_t indev_drv;
      lv_indev_drv_init(&indev_drv);
      indev_drv.type = LV_INDEV_TYPE_POINTER;
      indev_drv.read_cb = lvgl_touch_read;
      lv_indev_drv_register(&indev_drv);
    }

    const esp_timer_create_args_t timer_args = {
      .callback = &lvgl_tick_increment,
      .name = "lvgl_tick"
    };
    esp_timer_handle_t timer = NULL;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, LVGL_TICK_PERIOD_MS * 1000);

    system_state.lvgl_available = true;
    Serial.println("[OK] LVGL initialized");
  } else {
    Serial.println("[FAIL] LVGL buffer allocation failed!");
    system_state.lvgl_available = false;
  }

  // Step 7: Additional Features
  Serial.println("[INIT] Step 7: Additional Features");
  initializeFileSystem();
  initializeQuests();
  initializeGames();
  initializeThemes();

  // Step 8: Load UI
  if (system_state.lvgl_available) {
    Serial.println("[INIT] Step 8: Loading UI");
    gfx->displayOn();
    gfx->setBrightness(255);
    
    // Create initial screen
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0000FF), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "WATCH OK!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_center(label);
    
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    for (int i = 0; i < 10; i++) {
      lv_timer_handler();
      delay(10);
    }
  }

  Serial.println("\n========================================");
  Serial.println("  INITIALIZATION COMPLETE");
  Serial.printf("  Display: %s | Touch: %s | PMU: %s | LVGL: %s\n",
    system_state.display_available ? "OK" : "FAIL",
    system_state.touch_available ? "OK" : "FAIL",
    system_state.power_available ? "OK" : "N/A",
    system_state.lvgl_available ? "OK" : "FAIL");
  Serial.println("========================================\n");
}

// =============================================================================
// LOOP
// =============================================================================

void loop() {
  lv_timer_handler();
  updateBattery();
  delay(5);
}

void updateBattery() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000 && system_state.power_available) {
    lastUpdate = millis();
    if (PMU.isBatteryConnect()) {
      system_state.battery_percentage = PMU.getBatteryPercent();
      system_state.is_charging = PMU.isCharging();
    }
  }
}
