/**
 * ═══════════════════════════════════════════════════════════════════════════════
 * Pin Configuration for Waveshare ESP32-S3-Touch-AMOLED-1.8
 * FIXED VERSION - Corrected from WORKING S3_MiniOS reference
 * ═══════════════════════════════════════════════════════════════════════════════
 * 
 * CRITICAL FIXES APPLIED:
 * ✅ TP_INT changed from 21 to 38 (correct touch interrupt pin)
 * ✅ Added TP_RESET pin (GPIO 9 - required for touch controller init)
 * ✅ Added LCD_RESET pin (GPIO 8 - for display reset)
 * ✅ Added IMU_INT pin (GPIO 21 - was incorrectly used as TP_INT before)
 * ✅ Added RTC_INT pin (GPIO 39)
 * ✅ Added FT3168_DEVICE_ADDRESS for Arduino_DriveBus compatibility
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ═══════════════════════════════════════════════════════════════════════════════
//  DISPLAY - SH8601 QSPI AMOLED (1.8" 368x448)
//  Reference: Waveshare ESP32-S3-Touch-AMOLED-1.8 + S3_MiniOS
// ═══════════════════════════════════════════════════════════════════════════════
#define LCD_WIDTH   368
#define LCD_HEIGHT  448
#define LCD_CS      12
#define LCD_SCLK    11
#define LCD_SDIO0   4
#define LCD_SDIO1   5
#define LCD_SDIO2   6
#define LCD_SDIO3   7
#define LCD_RESET   8    // ADDED: Display reset pin from S3_MiniOS

// ═══════════════════════════════════════════════════════════════════════════════
//  I2C BUS
// ═══════════════════════════════════════════════════════════════════════════════
#define IIC_SDA     15
#define IIC_SCL     14

// ═══════════════════════════════════════════════════════════════════════════════
//  TOUCH CONTROLLER - FT3168
//  CRITICAL FIX: TP_INT was 21 (IMU pin!), changed to 38 per S3_MiniOS
// ═══════════════════════════════════════════════════════════════════════════════
#define TP_INT      38   // FIXED: Touch interrupt (was incorrectly 21)
#define TP_RST      9    // ADDED: Touch reset pin from S3_MiniOS
#define FT3168_DEVICE_ADDRESS  0x38  // ADDED: For Arduino_DriveBus compatibility

// ═══════════════════════════════════════════════════════════════════════════════
//  IMU - QMI8658
// ═══════════════════════════════════════════════════════════════════════════════
#define IMU_INT     21   // FIXED: IMU interrupt (was incorrectly used as TP_INT)
#define QMI8658_L_SLAVE_ADDRESS  0x6B

// ═══════════════════════════════════════════════════════════════════════════════
//  RTC - PCF85063
// ═══════════════════════════════════════════════════════════════════════════════
#define RTC_INT     39   // ADDED: RTC interrupt pin

// ═══════════════════════════════════════════════════════════════════════════════
//  I2C Addresses (for reference)
// ═══════════════════════════════════════════════════════════════════════════════
#define TOUCH_ADDR    0x38  // FT3168
#define IMU_ADDR      0x6B  // QMI8658
#define RTC_ADDR      0x51  // PCF85063
#define PMU_ADDR      0x34  // AXP2101
#define EXPANDER_ADDR 0x20  // XCA9554

// ═══════════════════════════════════════════════════════════════════════════════
//  SD CARD (SDMMC 1-bit Mode)
//  Reference: S3_MiniOS SD Card implementation
// ═══════════════════════════════════════════════════════════════════════════════
#define SDMMC_CLK   2
#define SDMMC_CMD   1
#define SDMMC_DATA  3

// Compatibility aliases
#define SD_CLK      SDMMC_CLK
#define SD_MOSI     SDMMC_CMD
#define SD_MISO     SDMMC_DATA

// ═══════════════════════════════════════════════════════════════════════════════
//  AUDIO (ES8311)
// ═══════════════════════════════════════════════════════════════════════════════
#define I2S_MCK     16
#define I2S_BCLK    9
#define I2S_WS      45
#define I2S_DO      10
#define I2S_DI      8
#define PA_EN       46

// Legacy audio defines for compatibility
#define I2S_LRCK    I2S_WS
#define I2S_DOUT    I2S_DO

// ═══════════════════════════════════════════════════════════════════════════════
//  SYSTEM BUTTONS
// ═══════════════════════════════════════════════════════════════════════════════
#define BOOT_BUTTON     0   // Boot button (can be used as power toggle)
#define PWR_BUTTON_PIN  0   // Alias for compatibility

// ═══════════════════════════════════════════════════════════════════════════════
//  DISPLAY COLOR MACROS (for compatibility with S3_MiniOS style code)
// ═══════════════════════════════════════════════════════════════════════════════
#define RGB565_BLACK    0x0000
#define RGB565_WHITE    0xFFFF
#define RGB565_RED      0xF800
#define RGB565_GREEN    0x07E0
#define RGB565_BLUE     0x001F

#endif // PIN_CONFIG_H
