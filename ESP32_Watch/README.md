# ESP32 Watch - Simplified Edition

A streamlined version of the ESP32 RPG smartwatch firmware, consolidating 70+ files into ~15 clean modules while maintaining **all features**.

## Hardware
- **Board**: ESP32-S3-Touch-AMOLED-1.8 (Waveshare)
- **Display**: 368x448 SH8601 AMOLED (QSPI)
- **Touch**: FT3168 Capacitive
- **PMU**: AXP2101 Power Management
- **IMU**: QMI8658 Accelerometer/Gyroscope
- **RTC**: PCF85063 Real-Time Clock
- **Audio**: ES8311 + I2S
- **Storage**: SD Card (SDMMC)

## Features (All Preserved)

### Watch Faces & Themes
- **Luffy Gear 5** - One Piece inspired with golden aura effects
- **Sung Jin-Woo** - Solo Leveling inspired with shadow effects
- **Yugo Wakfu** - Portal-based theme with teal energy
- Custom wallpaper support from SD card

### RPG System
- Character progression (Level 1-100)
- 22 unique titles per character path
- Experience from steps, quests, and activities
- Stats: STR, SPD, INT, END, MAG, Special Power
- Yugo endgame path selection (Level 90+)

### Games
- **Battle Arena** - Pokemon Showdown-style battles
- **Shadow Dungeon** - Jin-Woo themed dungeon crawler
- **Pirate Adventure** - Luffy treasure hunting
- **Wakfu Quest** - Yugo puzzle game
- **Snake** - Classic snake game
- **Memory Match** - Card matching game

### Applications
- Music Player (MP3 from SD)
- Notes
- File Browser
- PDF Reader
- Stopwatch/Timer
- Calculator
- Flashlight
- Wallpaper Selector
- Settings

### Internet Features
- WiFi Manager with network scanning
- Weather App
- News Reader
- NTP Time Sync

### Health & Activity
- Step Counter
- Daily Quests
- Activity Rings
- Calorie Tracking

## File Structure (Simplified)

```
ESP32_Watch_Simplified/
├── ESP32_Watch.ino    # Main entry point
├── config.h           # Unified configuration (pins, enums, structs)
├── display.h/cpp      # Display & LVGL UI
├── touch.h/cpp        # Touch input handling
├── hardware.h/cpp     # Power, Sensors, RTC
├── themes.h/cpp       # Watch faces & visual themes
├── games.h/cpp        # All games
├── apps.h/cpp         # All applications
├── wifi_apps.h/cpp    # WiFi & internet apps
├── filesystem.h/cpp   # SD card & file management
├── rpg.h/cpp          # RPG system & quests
├── ui.h/cpp           # UI framework & navigation
├── lv_conf.h          # LVGL configuration
└── README.md          # This file
```

## Dependencies (Arduino Libraries)

- Arduino_GFX_Library (for SH8601 display)
- LVGL (v8.x)
- XPowersLib (for AXP2101)
- Adafruit XCA9554 (I/O Expander)
- SD_MMC (built-in ESP32)

## Installation

1. Install Arduino IDE with ESP32 board support
2. Install required libraries via Library Manager
3. Copy this folder to your Arduino sketches
4. Select board: `ESP32S3 Dev Module`
5. Configure:
   - PSRAM: "OPI PSRAM"
   - Flash Mode: "QIO 80MHz"
   - Flash Size: "16MB"
   - USB CDC On Boot: "Enabled"
6. Upload!

## SD Card Structure (Optional)

```
/sdcard/
├── Music/             # MP3 files
├── Documents/         # PDF files
├── Wallpapers/
│   ├── Luffy/        # Gear 5 wallpapers
│   ├── JinWoo/       # Shadow Monarch wallpapers
│   └── Yugo/         # Eliatrope wallpapers
└── settings.json     # Saved preferences
```

## Controls

- **Swipe Left**: Open app grid (from watchface)
- **Swipe Right**: Go back
- **Swipe Up**: View quests
- **Swipe Down**: Settings
- **Tap**: Select/interact
- **Long Press**: Context actions

## Original Repository

https://github.com/ithig124-hub/ESP32_Watch

## License

Same as original project.
