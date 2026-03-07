/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  PIN CONFIGURATION - ESP32-S3-Touch-AMOLED-1.8" (Waveshare)
 *  
 *  Display: SH8601 QSPI AMOLED
 *  Resolution: 368×448
 *  
 *  Source: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#pragma once

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER MANAGEMENT - AXP2101
// ═══════════════════════════════════════════════════════════════════════════════
#define XPOWERS_CHIP_AXP2101

// ═══════════════════════════════════════════════════════════════════════════════
//  DISPLAY - SH8601 QSPI AMOLED (368×448)
// ═══════════════════════════════════════════════════════════════════════════════
#define LCD_SDIO0           4       // QSPI Data 0
#define LCD_SDIO1           5       // QSPI Data 1
#define LCD_SDIO2           6       // QSPI Data 2
#define LCD_SDIO3           7       // QSPI Data 3
#define LCD_SCLK            11      // QSPI Clock
#define LCD_CS              12      // Chip Select

#define LCD_WIDTH           368
#define LCD_HEIGHT          448

// ═══════════════════════════════════════════════════════════════════════════════
//  I2C BUS (Touch, IMU, RTC, PMU)
// ═══════════════════════════════════════════════════════════════════════════════
#define IIC_SDA             15
#define IIC_SCL             14

// ═══════════════════════════════════════════════════════════════════════════════
//  TOUCH - FT3168
// ═══════════════════════════════════════════════════════════════════════════════
#define TP_INT              21      // Touch Interrupt
#define FT3168_ADDR         0x38    // I2C Address

// ═══════════════════════════════════════════════════════════════════════════════
//  I2C DEVICE ADDRESSES
// ═══════════════════════════════════════════════════════════════════════════════
#define AXP2101_ADDR        0x34    // PMU
#define RTC_ADDR            0x51    // PCF85063 RTC
#define QMI8658_ADDR        0x6B    // IMU
#define EXPANDER_ADDR       0x20    // XCA9554 I/O Expander

// ═══════════════════════════════════════════════════════════════════════════════
//  AUDIO - ES8311
// ═══════════════════════════════════════════════════════════════════════════════
#define I2S_MCK_IO          16      // Master Clock
#define I2S_BCK_IO          9       // Bit Clock
#define I2S_WS_IO           45      // Word Select
#define I2S_DO_IO           10      // Data Out
#define I2S_DI_IO           8       // Data In
#define PA_PIN              46      // Power Amplifier Enable

// Alternative names
#define MCLKPIN             16
#define BCLKPIN             9
#define WSPIN               45
#define DOPIN               10
#define DIPIN               8
#define PA                  46

// ═══════════════════════════════════════════════════════════════════════════════
//  SD CARD (SDMMC 1-bit mode)
// ═══════════════════════════════════════════════════════════════════════════════
#define SDMMC_CLK           2
#define SDMMC_CMD           1
#define SDMMC_DATA          3

// ═══════════════════════════════════════════════════════════════════════════════
//  MISC
// ═══════════════════════════════════════════════════════════════════════════════
#define BOOT_BUTTON         0       // Boot/Flash button
