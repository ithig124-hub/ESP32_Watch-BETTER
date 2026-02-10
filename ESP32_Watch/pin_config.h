/**
 * Pin Configuration for Waveshare ESP32-S3-Touch-AMOLED-1.8
 * FIXED VERSION - Corrected pins from official Waveshare reference
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ═══════════════════════════════════════════════════════════════════════════════
//  DISPLAY (SH8601 QSPI AMOLED) - CORRECTED FROM WAVESHARE REFERENCE
// ═══════════════════════════════════════════════════════════════════════════════
#define LCD_WIDTH  368
#define LCD_HEIGHT 448
#define LCD_CS     12
#define LCD_SCLK   11
#define LCD_SDIO0  4
#define LCD_SDIO1  5
#define LCD_SDIO2  6
#define LCD_SDIO3  7

// ═══════════════════════════════════════════════════════════════════════════════
//  TOUCH (FT3168) - CORRECTED PINS
// ═══════════════════════════════════════════════════════════════════════════════
#define TP_INT     21
#define TP_RST     -1

// ═══════════════════════════════════════════════════════════════════════════════
//  I2C - CORRECTED FROM WAVESHARE REFERENCE
// ═══════════════════════════════════════════════════════════════════════════════
#define IIC_SDA    15
#define IIC_SCL    14

// I2C Addresses
#define TOUCH_ADDR    0x38  // FT3168
#define IMU_ADDR      0x6B  // QMI8658
#define RTC_ADDR      0x51  // PCF85063
#define PMU_ADDR      0x34  // AXP2101
#define EXPANDER_ADDR 0x20  // XCA9554

// ═══════════════════════════════════════════════════════════════════════════════
//  SD CARD (SDMMC) - CORRECTED FROM WAVESHARE REFERENCE
// ═══════════════════════════════════════════════════════════════════════════════
#define SDMMC_CMD  1
#define SDMMC_CLK  2
#define SDMMC_DATA 3

// ═══════════════════════════════════════════════════════════════════════════════
//  AUDIO (ES8311) - FROM WAVESHARE REFERENCE
// ═══════════════════════════════════════════════════════════════════════════════
#define I2S_MCK    16
#define I2S_BCLK   9
#define I2S_WS     45
#define I2S_DO     10
#define I2S_DI     8
#define PA_EN      46

// Legacy audio defines for compatibility
#define I2S_LRCK   I2S_WS
#define I2S_DOUT   I2S_DO

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER BUTTON
// ═══════════════════════════════════════════════════════════════════════════════
#define PWR_BUTTON_PIN  0   // Boot button can be used as power toggle

#endif
