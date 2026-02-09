# ESP32 Watch - ANIME EDITION

## Enhanced with Random Daily Character Themes

A smartwatch firmware for ESP32-S3-Touch-AMOLED-1.8 with anime-inspired UI themes.

## Themes

### Main Themes (Selectable)

1. **Gear 5 Luffy** - Clean Fun / Freedom
   - Bright whites, soft pastels, high-contrast outlines
   - Cloud-like shapes as visual motifs (static)
   - Joyful and free through color and shape, NOT motion
   - **NO bouncing, NO rubber physics**

2. **Jin-Woo Shadow** - Dark / Power / Focus  
   - Deep blacks with blue/purple glow accents
   - Shadows as static background layers
   - Sharp, precise, perfectly aligned UI
   - Zero distractions, pure control

3. **Yugo Portal** - Chill / Exploration
   - Soft cel-shaded anime art with teal/cyan
   - Faint portal rings, light particle effects (static)
   - Rounded, friendly UI elements
   - Adventurous, peaceful aesthetic

4. **Random Daily** - New character every day!
   - Rotates through 7 anime characters based on date
   - Each character has unique colors, stats, and abilities

### Random Daily Characters

| Day | Character | Series | Theme |
|-----|-----------|--------|-------|
| 1 | Naruto | Naruto | Orange chakra, Sage Mode |
| 2 | Goku | Dragon Ball | Gold aura, Power Level |
| 3 | Tanjiro | Demon Slayer | Fire/Water breathing |
| 4 | Gojo | Jujutsu Kaisen | Infinity, Six Eyes |
| 5 | Levi | Attack on Titan | ODM gear, Titan kills |
| 6 | Saitama | One Punch Man | Simple, Hero rank |
| 7 | Deku | My Hero Academia | OFA lightning |

## Features

### RPG System (50 Levels)
- Shortened progression from 100 to 50 levels
- 5 title tiers per character
- Stats: STR, SPD, MAG, END
- Character-specific special abilities

### Anime Games
- **Gacha Simulator** - Pull for character cards (35+ characters)
- **Training Mini-games** - Earn XP through activities  
- **Boss Rush** - Fight iconic anime villains
- **Card Battles** - Coming soon

### Design Guidelines
- Clean, controlled transitions (fade only)
- Static visual elements (no bouncing/rubber physics)
- Anime-style UI elements (speech bubbles, action lines)
- Theme-appropriate color palettes

## Hardware

- Board: Waveshare ESP32-S3-Touch-AMOLED-1.8
- Display: SH8601 QSPI AMOLED 368x448
- Touch: FT3168
- IMU: QMI8658
- RTC: PCF85063  
- PMU: AXP2101

## Required Libraries

1. LVGL (v8.x)
2. Arduino_GFX_Library
3. XPowersLib
4. Adafruit_XCA9554
5. ArduinoJson

## Board Settings

```
Board:              ESP32S3 Dev Module
USB CDC On Boot:    Enabled
Flash Size:         16MB
PSRAM:              OPI PSRAM
Partition Scheme:   Huge APP (3MB No OTA/1MB SPIFFS)
```

## File Structure

```
ESP32_Watch/
├── ESP32_Watch.ino     # Main sketch
├── config.h            # Types, enums, state
├── pin_config.h        # Hardware pins
├── themes.h/.cpp       # Theme system + Random daily
├── ui_manager.h/.cpp   # Screen management
├── rpg.h/.cpp          # 50-level RPG system
├── games.h/.cpp        # Gacha, Training, Boss Rush
├── apps.h/.cpp         # Music, Settings, Quests
├── wifi_apps.h/.cpp    # Weather, News
└── user_data.cpp       # Persistent storage
```

## Navigation

- **Swipe Left/Right**: Cycle between Clock → Apps → Character Stats
- **Swipe Up/Down**: Page through app grids
- **Tap**: Select items

## License

MIT License
