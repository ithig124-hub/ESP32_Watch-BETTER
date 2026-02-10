# ESP32 Watch - PREMIUM APPLE STYLE Edition

Enhanced Anime Themes with Apple Watch-inspired Premium UI for ESP32-S3-Touch-AMOLED-1.8" Smartwatch

![Version](https://img.shields.io/badge/version-1.0-gold)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Premium Features

### 🎨 Apple Watch-Inspired Design
- **OLED-optimized** deep black backgrounds
- **Glassmorphism** card effects with subtle transparency
- **Premium color palettes** inspired by luxury watch editions
- **Smooth 60fps** fade animations
- **San Francisco-style** typography hierarchy

### ⚡ Power Management (NEW)
- **3-second screen timeout** (Apple Watch style quick timeout)
- **Power button tap** to toggle screen on/off
- **Touch-to-wake** - tap anywhere to wake display
- **Smooth brightness fade** animations
- **Battery saver modes** with configurable levels:
  - Off: Full brightness, 3s timeout
  - Light: 50% brightness, 2s timeout
  - Medium: 30% brightness, 1.5s timeout
  - Extreme: 10% brightness, 1s timeout

### 🎭 Premium Anime Themes

#### Main Themes
| Theme | Aesthetic | Colors |
|-------|-----------|--------|
| **Gear 5 Sun God** | Clean white/gold luxury | Gold, cream white, warm pastels |
| **Shadow Monarch** | Deep purple/black premium | Purple, indigo, silver accents |
| **Portal Master** | Teal/cyan exploration | Cyan, turquoise, dark teal |
| **Random Daily** | New hero every day! | Character-specific palettes |

#### Daily Random Characters (7 total)
| Day | Character | Series | Theme Colors |
|-----|-----------|--------|--------------|
| 1 | Naruto | Naruto | Orange/Gold chakra |
| 2 | Goku | Dragon Ball | Gold/Blue power |
| 3 | Tanjiro | Demon Slayer | Red/Orange flames |
| 4 | Gojo | Jujutsu Kaisen | Cyan/White infinity |
| 5 | Levi | Attack on Titan | Green/Grey military |
| 6 | Saitama | One Punch Man | Yellow/White simple |
| 7 | Deku | My Hero Academia | Green/Lightning |

## 🔧 Hardware

- **Board:** Waveshare ESP32-S3-Touch-AMOLED-1.8
- **Display:** SH8601 QSPI AMOLED 368x448
- **Touch:** FT3168 (I2C 0x38)
- **IMU:** QMI8658 (I2C 0x6B)
- **RTC:** PCF85063 (I2C 0x51)
- **PMU:** AXP2101 (I2C 0x34)
- **I/O Expander:** XCA9554 (I2C 0x20)

## 📚 Required Libraries

1. **LVGL** (v8.x)
2. **Arduino_GFX_Library** (from Waveshare repo)
3. **Arduino_DriveBus_Library** (from Waveshare repo - for touch)
4. **XPowersLib**
5. **Adafruit_XCA9554**
6. **ArduinoJson**

### Installing Waveshare Libraries

Download from: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/tree/main/examples/Arduino-v3.3.5/libraries

Copy these folders to your Arduino libraries folder:
- `Arduino_DriveBus` 
- `GFX_Library_for_Arduino`
- `Mylibrary` (contains pin_config.h)

## ⚙️ Board Settings (Arduino IDE)

```
Board:              ESP32S3 Dev Module
USB CDC On Boot:    Enabled
Flash Size:         16MB
PSRAM:              OPI PSRAM
Partition Scheme:   Huge APP (3MB No OTA/1MB SPIFFS)
```

## 📁 File Structure

```
ESP32_Watch_Premium/
├── ESP32_Watch_Premium.ino  # Main sketch
├── config.h                  # Types, enums, state
├── pin_config.h              # Hardware pins
├── power_manager.h/.cpp      # Screen timeout & power button
├── themes.h/.cpp             # Premium theme system
├── ui_manager.h/.cpp         # Screen management
├── user_data.cpp             # Persistent storage
└── README.md                 # This file
```

## 🎮 Navigation

| Gesture | Action |
|---------|--------|
| **Swipe Left/Right** | Cycle: Clock → Apps → Character |
| **Swipe Up/Down** | Page through app grids |
| **Tap** | Select items / Wake screen |
| **Power Button** | Toggle screen on/off |

## 🔋 Power Button Behavior

Based on the reference implementation from `S3_MiniOS_206.ino`:

```cpp
// Power button tap toggles screen state
if (powerButton pressed) {
  if (screenOn) screenSleep();  // Turn off with fade
  else screenWake();             // Turn on with fade
}

// 3-second inactivity timeout
if (screenOn && inactiveTime >= 3000ms) {
  screenSleep();
}

// Touch-to-wake
if (!screenOn && touchDetected) {
  screenWake();
}
```

## 🎨 Theme Color System

Each theme includes:
- `primary` - Main accent color
- `secondary` - Secondary accent
- `accent` - Highlight/glow color
- `background` - OLED black optimized
- `surface` - Card/container background
- `text` - Primary text color
- `textSecondary` - Muted text
- `glow` - Special effect color
- `gradient1/2` - Gradient colors
- `border` - Subtle borders
- `highlight` - Interactive highlights

## 📝 Changelog

### v1.0 - Premium Edition
- Apple Watch-inspired premium UI design
- 3-second screen timeout implementation
- Power button tap to toggle screen
- Touch-to-wake functionality
- Enhanced anime theme color palettes
- Glassmorphism card effects
- Smooth brightness fade animations
- Battery saver modes

## 📄 License

MIT License - Feel free to modify and share!

---

**Made with ❤️ for anime fans and watch enthusiasts**
