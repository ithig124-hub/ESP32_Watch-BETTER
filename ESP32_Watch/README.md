# ESP32 Watch - PREMIUM ANIME EDITION

## 🎨 ULTRA PREMIUM Character Themes

Each theme is meticulously crafted to represent the essence, power, and aesthetic of each anime character.

---

## 🌟 MAIN THEMES

### ☀️ GEAR 5 LUFFY - Sun God Nika
**ONE PIECE**
- **Aesthetic:** Pure white, brilliant gold, straw hat red
- **Feel:** Joy, freedom, boundless energy
- **UI Style:** Very rounded (cloud-like), strong sun glow
- **Signature:** "Bajrang Gun"
- **Stats:** HAKI | GEAR | FREEDOM | WILL

### 🌑 JIN-WOO SHADOW MONARCH - Arise
**SOLO LEVELING**
- **Aesthetic:** Royal purple, void black, electric blue
- **Feel:** Dark power, overwhelming dominance
- **UI Style:** Sharp edges, maximum glow intensity
- **Signature:** "ARISE"
- **Stats:** SHADOWS | POWER | LEVEL | ARMY

### 🌀 YUGO PORTAL MASTER - Eliatrope King
**WAKFU**
- **Aesthetic:** Bright cyan, deep teal, warm gold
- **Feel:** Adventure, mystery, exploration
- **UI Style:** Rounded (portal-like), soft glow
- **Signature:** "Wakfu Portal"
- **Stats:** WAKFU | PORTALS | SPEED | HEART

---

## 🎲 DAILY RANDOM CHARACTERS

Rotates daily through 7 iconic anime characters:

### Day 1: 🦊 NARUTO - Sage Mode
**NARUTO**
- Orange chakra, black outfit, Kurama red
- "Believe it! Dattebayo!"
- Stats: CHAKRA | SAGE | KURAMA | BONDS

### Day 2: ⚡ GOKU - Ultra Instinct
**DRAGON BALL**
- Silver aura, divine blue, pure white
- "I am the hope of the universe!"
- Stats: POWER | SPEED | KI | FORMS

### Day 3: 🔥 TANJIRO - Sun Breathing
**DEMON SLAYER**
- Deep crimson, dark green checkered, fire orange
- "I will never give up!"
- Stats: BREATH | FORMS | SENSE | WILL

### Day 4: ♾️ GOJO - Infinity
**JUJUTSU KAISEN**
- Infinity blue, white hair, purple cursed energy
- "Throughout Heaven and Earth, I alone am the honored one."
- Stats: INFINITY | CURSED | SIX EYES | DOMAIN

### Day 5: ⚔️ LEVI - Humanity's Strongest
**ATTACK ON TITAN**
- Military grey, Survey Corps green, steel silver
- "Give up on your dreams and die."
- Stats: TITANS | KILLS | SPEED | CLEAN

### Day 6: 👊 SAITAMA - One Punch Man
**ONE PUNCH MAN**
- Yellow suit, red cape, simple and clean
- "OK."
- Stats: PUNCH | RANK | BORED | SALES

### Day 7: 💚 DEKU - Plus Ultra
**MY HERO ACADEMIA**
- Hero green, black costume, OFA lightning
- "PLUS ULTRA!"
- Stats: OFA % | QUIRKS | SMASH | HERO

---

## 🔧 Feature Summary

### ✅ NO Screen Timeout
- Screen stays on indefinitely
- Manual control only (power button / tap)

### ✅ Tap to Wake
- Touch screen when off to wake

### ✅ Power Button Toggle
- Press to turn screen on/off
- Default GPIO: 0 (Boot button)

### ✅ Swipe Animations
- Left/Right: Navigate screens
- Up/Down: Scroll app pages

### ✅ Device Keeps Running
- No deep sleep when screen off
- Background processes continue

---

## 📁 File Structure

```
ESP32_Watch_FIXED/
├── ESP32_Watch.ino      # Main firmware
├── config.h             # Types, enums, state
├── pin_config.h         # Hardware pins + PWR_BUTTON
├── power_manager.h/cpp  # Screen timeout DISABLED
├── themes.h/cpp         # PREMIUM character themes
├── ui_manager.h/cpp     # Premium UI screens
├── user_data.cpp        # Persistent storage
└── README.md            # This file
```

---

## 🎮 Premium UI Features

### Watch Face
- Character-themed background with glow rings
- Series name header
- Character name and title
- Large time display with seconds
- Signature move display
- Navigation dots

### Character Stats Screen
- Glass-morphism portrait container
- Character info (name, title, series)
- Level display
- Animated stat bars with glow
- Character catchphrase

### Apps Screen
- Glass-morphism app buttons
- Themed icons with glow
- Grid layout

---

## 🎨 Color Palette Reference

| Character | Primary | Secondary | Accent | Glow |
|-----------|---------|-----------|--------|------|
| Luffy | White | Gold | Red | Sun Yellow |
| Jin-Woo | Purple | Black | Blue | Purple |
| Yugo | Cyan | Teal | Gold | Light Cyan |
| Naruto | Orange | Black | Red | Gold |
| Goku | Silver | Blue | White | Silver |
| Tanjiro | Crimson | Green | Orange | Flame |
| Gojo | Blue | White | Purple | Light Blue |
| Levi | Grey | Dark Grey | Green | Silver |
| Saitama | Yellow | Red | White | Gold |
| Deku | Green | Black | Yellow | Lightning |

---

## ⚙️ Configuration

### Change Power Button GPIO
```cpp
// In pin_config.h
#define PWR_BUTTON_PIN 14  // Change to your GPIO
```

### Enable Screen Timeout
```cpp
// In power_manager.h
#define SCREEN_TIMEOUT_ENABLED true
#define SCREEN_TIMEOUT_DEFAULT 3000  // 3 seconds
```

### Change Default Theme
```cpp
// In themes.cpp, initThemes()
currentTheme = &THEME_SHADOW_MONARCH;  // Change default
```

---

## 🛠️ Hardware

- **Board:** Waveshare ESP32-S3-Touch-AMOLED-1.8
- **Display:** SH8601 QSPI AMOLED 368x448
- **Touch:** FT3168
- **PMU:** AXP2101
- **IMU:** QMI8658
- **RTC:** PCF85063

## 📚 Required Libraries

1. LVGL (v8.x)
2. Arduino_GFX_Library
3. Arduino_DriveBus_Library
4. XPowersLib
5. Adafruit_XCA9554
6. ArduinoJson

---

**Enjoy your PREMIUM anime smartwatch!** 🎉
