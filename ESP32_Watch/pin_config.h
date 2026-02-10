/**
 * Pin Configuration for Waveshare ESP32-S3-Touch-AMOLED-1.8
 * FIXED VERSION - Added Power Button Pin
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Display (SH8601 QSPI AMOLED)
#define LCD_WIDTH  368
#define LCD_HEIGHT 448
#define LCD_CS     6
#define LCD_SCLK   47
#define LCD_SDIO0  18
#define LCD_SDIO1  7
#define LCD_SDIO2  48
#define LCD_SDIO3  5

// Touch (FT3168)
#define TP_INT     8
#define TP_RST     -1

// I2C
#define IIC_SDA    10
#define IIC_SCL    11

// I2C Addresses
#define TOUCH_ADDR    0x38  // FT3168
#define IMU_ADDR      0x6B  // QMI8658
#define RTC_ADDR      0x51  // PCF85063
#define PMU_ADDR      0x34  // AXP2101
#define EXPANDER_ADDR 0x20  // XCA9554

// SD Card (SDMMC)
#define SDMMC_CMD  13
#define SDMMC_CLK  12
#define SDMMC_DATA 9

// ═══════════════════════════════════════════════════════════════════════════════
//  POWER BUTTON - DIRECTLY CONTROL SCREEN ON/OFF
// ═══════════════════════════════════════════════════════════════════════════════
// If your board has a physical button, define it here
// Common GPIO pins for side button: 0, 14, 21, 38, 39, etc.
// Set to -1 if no physical button available
#define PWR_BUTTON_PIN  0   // Boot button can be used as power toggle
                            // Change to your actual button GPIO if different

// Audio (optional)
#define I2S_BCLK   -1
#define I2S_LRCK   -1
#define I2S_DOUT   -1

#endif
