/**
 * Pin Configuration for Waveshare ESP32-S3-Touch-AMOLED-1.8
 * OFFICIAL WAVESHARE PIN CONFIGURATION
 * Premium Apple Style Edition
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define XPOWERS_CHIP_AXP2101

// Display (SH8601 QSPI AMOLED) - OFFICIAL PINS
#define LCD_SDIO0  4
#define LCD_SDIO1  5
#define LCD_SDIO2  6
#define LCD_SDIO3  7
#define LCD_SCLK   11
#define LCD_CS     12
#define LCD_WIDTH  368
#define LCD_HEIGHT 448

// I2C - OFFICIAL PINS
#define IIC_SDA    15
#define IIC_SCL    14

// Touch (FT3168) - OFFICIAL PIN
#define TP_INT     21

// I2C Addresses
#define TOUCH_ADDR    0x38  // FT3168
#define FT3168_DEVICE_ADDRESS 0x38
#define IMU_ADDR      0x6B  // QMI8658
#define RTC_ADDR      0x51  // PCF85063
#define PMU_ADDR      0x34  // AXP2101
#define EXPANDER_ADDR 0x20  // XCA9554

// ES8311 Audio Codec
#define I2S_MCK_IO 16
#define I2S_BCK_IO 9
#define I2S_DI_IO  10
#define I2S_WS_IO  45
#define I2S_DO_IO  8
#define PA_PIN     46

// SD Card (SDMMC)
#define SDMMC_CLK  2
#define SDMMC_CMD  1
#define SDMMC_DATA 3

// Power Button - Not available on this board
// The board doesn't have a dedicated power button GPIO
// Screen control is done via PMU and touch-to-wake

#endif
