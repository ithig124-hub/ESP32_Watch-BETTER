# ESP32 Anime Gaming Smartwatch - IMPROVED Edition

A modern anime-themed smartwatch firmware with **11 anime character themes** (including BoBoiBoy!), gacha collection system, mini-games, and improved navigation.

## NEW! Latest Features

### Dynamic Day/Night Backgrounds
- All watch faces now change based on time of day
- **Dawn** (5-8am): Purple-blue to orange-pink gradient, few stars, rising sun
- **Morning** (8-12pm): Bright blue sky with clouds
- **Afternoon** (12-5pm): Clear blue sky, full sun
- **Evening** (5-8pm): Deep purple to orange sunset, stars appearing
- **Night** (8pm-5am): Dark void with moon and twinkling stars
- Each character has unique ambient particle effects!

### BoBoiBoy: Tap-to-Switch Elements
- **Tap the center** of the watch face to manually switch elements
- No more waiting for auto-cycle - instant element changes!
- Ochobot announces each element change

### Ochobot Assistant
- Cute Power Sphere helper appears in BoBoiBoy mode
- Gives tips and notifications with speech bubbles
- Bouncing idle animation with blinking eyes
- Shows element-specific tips

### Fusion Minigame (BoBoiBoy)
- Drag and drop elements (base or evolved) to the fusion zone
- Valid combinations unlock new fusion forms!

**Official BoBoiBoy Fusions:**
| Fusion | Elements | First Appearance |
|--------|----------|------------------|
| **FrostFire** | Blaze + Ice | BoBoiBoy Movie 2 |
| **Glacier** | Quake + Ice | BoBoiBoy Movie 2 |
| **Supra** | Thunderstorm + Solar | BoBoiBoy Movie 2 |
| **Sori** | Thorn + Solar | Galaxy S2 Issue 9 |
| **Rumble** | Quake + Thunderstorm | Galaxy S2 Issue 15 |
| **Sopan** | Solar + Cyclone | Galaxy S2 Issue 20 |

### Character-Specific Minigames
| Character | Game | Description |
|-----------|------|-------------|
| **Luffy** | Gomu Gomu Punch | Hold to stretch, release at perfect timing |
| **Gojo** | Domain Dodge | Dodge cursed energy in Unlimited Void |
| **Levi** | Titan Slayer | Swipe to slash titan napes |
| **Saitama** | One Punch | Tap once to win (joke game) |
| **BoBoiBoy** | Element Battle | Rock-paper-scissors combat with elements |

### Element Battle System
- Choose your element wisely - each has strengths and weaknesses!
- Charge power meter by waiting
- Swipe left/right to switch elements mid-battle
- Tap to attack when power is charged

## Key Improvements Over Original

### Navigation
- **Better Swipe Detection**: Reduced thresholds (30px vs 50px) for easier swipes
- **All Apps Accessible**: Every app can now be tapped and opened
- **Clear Touch Targets**: Bigger app icons (100x90px) with better hit detection
- **Smooth Transitions**: Proper navigation state management

### Visual Design
- **Modern Glass Morphism**: Frosted glass UI throughout
- **Anime Color Themes**: Each character has a unique color palette
- **Activity Rings**: Apple Watch-style progress rings
- **Premium Watch Faces**: 11 distinct designs, one for each character

### Usability
- **Navigation Indicators**: Dots showing current screen position
- **Touch Feedback**: Visual feedback on button presses
- **Better Contrast**: High contrast text on AMOLED
- **Faster Responsiveness**: Reduced navigation cooldown (200ms)

## 11 Anime Character Themes

| # | Character | Anime | Primary Colors |
|---|-----------|-------|----------------|
| 1 | **Luffy (Gear 5)** | One Piece | Sun Gold / White |
| 2 | **Sung Jin-Woo** | Solo Leveling | Monarch Purple / Void Black |
| 3 | **Yugo** | Wakfu | Portal Cyan / Teal |
| 4 | **Naruto (Sage)** | Naruto | Chakra Orange / Gold |
| 5 | **Goku (UI)** | Dragon Ball | Divine Silver / Blue |
| 6 | **Tanjiro** | Demon Slayer | Fire Orange / Green |
| 7 | **Gojo** | Jujutsu Kaisen | Infinity Blue / Purple |
| 8 | **Levi** | Attack on Titan | Military Green / Silver |
| 9 | **Saitama** | One Punch Man | Hero Yellow / Cape Red |
| 10 | **Deku** | My Hero Academia | Hero Green / Lightning |
| 11 | **BoBoiBoy** | BoBoiBoy Galaxy | **7 Elemental Colors** |

### BoBoiBoy Special Features
The BoBoiBoy theme includes all **7 elemental forms**:
- **Halilintar** (Lightning) - Electric Yellow/Black
- **Taufan** (Wind/Cyclone) - Sky Blue/Cyan  
- **Gempa** (Earth/Quake) - Brown/Orange
- **Blaze** (Fire) - Crimson Red
- **Ice** (Water) - Pure Cyan/White
- **Thorn** (Leaf) - Forest Green
- **Solar** (Light) - Golden/Orange

**NEW: Tap center to switch elements manually!** Ochobot will announce the change.

### BoBoiBoy Element Tree App
When in BoBoiBoy mode, the **QUESTS** app is replaced with **ELEMENTS** - a dedicated element tree showing:

**20 Total Forms:**
- **7 Base Elements** (Tier 1) - Always unlocked
- **7 Evolved Forms** (Tier 2) - Thunderstorm, Cyclone, Quake, Inferno, Glacier, Darkwood, Supernova
- **6 Fusions** - FrostFire, Volcanice, ThunderBlaze, Supra, Gentar, Beliung

**Features:**
- Visual tree with connections showing evolution paths
- Tap elements to see details (name, power, description)
- Swipe between Base/Evolved/Fusion pages
- Unlock progression system (Tier 2 & Fusions can be unlocked through gameplay)

## Apps Included

### Page 1 (Main)
- **GACHA** - Collect anime character cards
- **TRAINING** - Mini-games for rewards
- **BOSS** - Boss Rush mode
- **GAMES** - Battle Arena, Snake, Memory Match
- **QUESTS** - Daily/weekly missions
- **MUSIC** - Audio player
- **WEATHER** - Weather display
- **WIFI** - Network manager
- **SETTINGS** - System settings

### Page 2 (More)
- **THEMES** - Change character themes
- **COLLECT** - View card collection
- **FILES** - File browser
- **CALC** - Calculator
- **TORCH** - Flashlight
- **OTA** - Firmware update
- **BACKUP** - Data backup
- **FUSION** - Card fusion (coming soon)
- **ABOUT** - System info

## Navigation

- **Swipe LEFT/RIGHT**: Switch between main screens
  - Watchface → App Grid → Character Stats
- **Swipe UP/DOWN**: Switch app grid pages (on App Grid)
- **TAP**: Open apps or interact with buttons
- **Navigation Dots**: Bottom of screen shows current position

## Hardware Requirements

- **MCU**: ESP32-S3 (PSRAM recommended)
- **Display**: 368x448 SH8601 AMOLED (QSPI)
- **Touch**: FT3168 Capacitive (I2C)
- **Power**: AXP2101 PMU
- **Storage**: SD card for save data

Compatible with: **ESP32-S3-Touch-AMOLED-1.8"**

## Installation

1. Open Arduino IDE 2.0+
2. Install ESP32 board support
3. Install required libraries:
   - Arduino_GFX (for SH8601)
4. Open `ESP32_Watch_IMPROVED.ino`
5. Select board: **ESP32-S3 Dev Module**
6. Configure:
   - Flash Mode: QIO
   - Flash Size: 16MB
   - PSRAM: OPI PSRAM
7. Upload!

## File Structure

```
ESP32_Watch_IMPROVED/
├── ESP32_Watch_IMPROVED.ino  # Main firmware
├── config.h                   # Configuration & colors
├── navigation.h/cpp           # Improved navigation
├── themes.h/cpp              # 10 character themes
├── apps.h/cpp                # App implementations
├── games.h/cpp               # Games system
├── gacha.h/cpp               # Gacha collection
├── training.h/cpp            # Training mini-games
├── boss_rush.h/cpp           # Boss Rush mode
├── rpg.h/cpp                 # RPG progression
└── ... (other modules)
```

## Customization

### Adding New Themes
Edit `themes.cpp` and add:
1. New `ThemeColors` struct
2. New `CharacterProfile`
3. New watch face draw function
4. Update `setTheme()` switch case

### Changing Colors
Colors use RGB565 format: `RGB565(r, g, b)`
Edit the color definitions in `config.h`

## Credits

- Original firmware concept and base code
- Enhanced by community contributions
- Free to modify and distribute!

## License

Open Source - Free to use, modify, and distribute
