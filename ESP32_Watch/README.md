# ESP32 Watch Simplified

**All Features, Clean Modular Code**

A simplified version of ESP32_Watch with all features working but in a clean, maintainable modular structure.

## Features

- **Clock** with themed watch faces (Luffy, JinWoo, Yugo)
- **Step Counter** with activity tracking and daily goals
- **6 Mini-Games** (Battle Arena, Snake, Memory Match, Dungeon, Pirate, Portal)
- **RPG Progression System** (Solo Leveling inspired - level 1-100)
- **Music Player** (SD card MP3 support)
- **Weather App** (OpenWeatherMap API)
- **News Reader** 
- **Quest System** with daily challenges and XP rewards
- **Theme Selector** with 3 character themes
- **Settings** with brightness, step goals, WiFi status

## Hardware

- **Board**: Waveshare ESP32-S3-Touch-AMOLED-1.8
- **Display**: SH8601 QSPI AMOLED 368x448
- **Touch**: FT3168 (I2C 0x38)
- **IMU**: QMI8658 (I2C 0x6B)
- **RTC**: PCF85063 (I2C 0x51)
- **PMU**: AXP2101 (I2C 0x34)
- **I/O Expander**: XCA9554 (I2C 0x20)

## File Structure (Simplified from 70+ files to 15)

```
ESP32_Watch_Simplified/
├── ESP32_Watch_Simplified.ino  # Main sketch
├── pin_config.h                # Hardware pin definitions
├── config.h                    # Types, enums, global state
├── ui_manager.h/.cpp           # LVGL screen management
├── themes.h/.cpp               # Theme colors and wallpapers
├── games.h/.cpp                # 6 mini-games
├── apps.h/.cpp                 # Music, Settings, Quests
├── rpg.h/.cpp                  # RPG progression system
├── wifi_apps.h/.cpp            # Weather & News
├── user_data.cpp               # Persistent storage
└── README.md                   # This file
```

## Required Libraries

Install via Arduino Library Manager:

1. **LVGL** (v8.x or v9.x)
2. **Arduino_GFX_Library** by moononournation
3. **XPowersLib** for AXP2101 PMU
4. **Adafruit_XCA9554** for I/O expander
5. **ArduinoJson** for weather/news APIs

## Board Settings (Arduino IDE)

```
Board:              ESP32S3 Dev Module
USB CDC On Boot:    Enabled
Flash Size:         16MB
PSRAM:              OPI PSRAM
Partition Scheme:   Huge APP (3MB No OTA/1MB SPIFFS)
```

## Themes

### Luffy - Sun God Nika
- Colors: Orange, Gold, Red-Orange
- Special: Haki Level progression
- Titles: Rookie Pirate → Sun God Nika

### Jin-Woo - Shadow Monarch
- Colors: Purple, Indigo, Violet
- Special: Shadow Army size
- Titles: E-Rank Hunter → Shadow Monarch

### Yugo - Portal Master
- Colors: Teal, Blue, Turquoise
- Special: Portal Mastery
- Titles: Young Eliatrope → Master of Portals

## Games

1. **Battle Arena** - Pokemon-style turn-based battles
2. **Snake** - Classic snake game with touch controls
3. **Memory Match** - 4x4 card matching game
4. **Shadow Dungeon** - JinWoo-themed RPG dungeon
5. **Pirate Quest** - Luffy-themed adventure
6. **Portal Puzzle** - Yugo-themed puzzle game

## RPG System

- 100 levels with exponential XP curve
- 6 title tiers based on level progression
- Stats: STR, SPD, MAG, END
- Character-specific special abilities
- XP earned from steps, quests, and games

## Quest System

Daily quests that track steps:
- Morning Walk (2000 steps) - 50 XP
- Power Up! (5000 steps) - 100 XP
- Step Master (8000 steps) - 150 XP
- Explorer (10000 steps) - 200 XP
- Champion (15000 steps) - 300 XP

## Upload Instructions

1. Install all required libraries
2. Select board settings above
3. Connect ESP32-S3 via USB
4. Upload `ESP32_Watch_Simplified.ino`
5. All other files will be included automatically

## SD Card (Optional)

SD card is optional. Features work without it.
For music playback, place MP3 files in:
```
/Music/
```

## Comparison to Original

| Aspect | Original | Simplified |
|--------|----------|------------|
| Files | 70+ | 15 |
| Lines of Code | ~15,000+ | ~2,500 |
| All Features | ✓ | ✓ |
| LVGL UI | ✓ | ✓ |
| Touch | ✓ | ✓ |
| Games | 6 | 6 |
| RPG System | ✓ | ✓ |
| Themes | 3 | 3 |
| WiFi Apps | ✓ | ✓ |
| Maintainability | Complex | Clean |

## License

MIT License - Feel free to use and modify!
