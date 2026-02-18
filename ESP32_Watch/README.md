# ESP32 Watch - COMPLETE EDITION

## All Features from COMPLETE_FEATURES_LIST.md Implemented

A premium anime-themed smartwatch firmware for the ESP32-S3-Touch-AMOLED-1.8 (Waveshare).

**Now with Fusion Labs Web Serial Integration!**

---

## 🆕 NEW: Fusion Labs Integration

### Web Serial Features (via USB)
Connect your watch to the Fusion Labs website for:
- **WiFi Configuration** - Edit WiFi settings via USB (no SD card editing needed)
- **Watch Face Studio** - Create and install custom watch faces
- **Firmware Updates** - OTA updates via the website
- **Data Dashboard** - View your gacha collection, boss progress, stats
- **Backup/Restore** - Backup all your data to your computer

### Fusion Labs Website
**https://ithig124-hub.github.io/fusion-labs**

### Web Serial Commands
| Command | Description |
|---------|-------------|
| `WIDGET_PING` | Test connection (responds `WIDGET_PONG`) |
| `WIDGET_STATUS` | Get full device status |
| `WIDGET_READ_WIFI` | Read WiFi config from SD card |
| `WIDGET_WRITE_WIFI` | Write WiFi config to SD card |
| `WIDGET_CHECK_UPDATE` | Check for firmware updates |
| `WIDGET_DOWNLOAD_UPDATE` | Download firmware to SD |
| `WIDGET_BACKUP` | Create data backup |

---

## 💾 SD Card Storage

### Automatic Folder Structure
On first boot, the watch creates:
```
/WATCH/
├── data/           # Player progress
├── wifi/           # WiFi config (config.txt)
├── gacha/          # Card collection
├── boss_rush/      # Boss progress
├── FIRMWARE/       # OTA updates
├── BACKUPS/        # Auto-backups
├── LOGS/           # Boot logs
└── FACES/          # Custom watch faces
```

### WiFi Configuration
Create `/WATCH/wifi/config.txt`:
```
SSID1=YourHomeWiFi
PASSWORD1=YourPassword
OPEN1=false
CITY=Perth
COUNTRY=AU
GMT_OFFSET=8
```

### NTP Time Sync
- Automatically syncs time from `pool.ntp.org` when WiFi connects
- Timezone configured via GMT_OFFSET in config.txt

### Auto-Backup
- Backs up all game data every 24 hours
- Manual backup via Fusion Labs website or serial command

---

## 🚀 Performance Optimizations

### Memory Management
- Smart allocation (PSRAM preferred, fallback to internal RAM)
- Memory status monitoring
- Low memory warning at 50KB threshold

### Power Saving
- CPU frequency scaling (80/160/240 MHz)
- 30-second screen timeout with auto-dim
- Auto-sleep after 1 minute of inactivity

### Stability
- Watchdog timer (30-second timeout with auto-reboot)
- Touch debouncing (50ms)
- Frame rate limiting (30 FPS target)

### Data Persistence (NVS)
- Player progress (level, XP, gems)
- Theme preference
- Daily steps (auto-reset on new day)
- Training streak
- Boss defeat progress
- Gacha card collection
- High scores
- Settings (brightness, sound)
- Auto-save every 5 minutes

### Smooth Animations
- Non-blocking delays
- Lerp interpolation
- Ease-out/ease-in-out curves

---

## 🎭 10 Premium Anime Character Themes

Each character has unique:
- Color palette (10 unique colors)
- Watch face design
- Activity rings
- Visual effects (particles, auras, glows)
- Signature moves and catchphrases
- Character stats

### Characters:

1. **Luffy - Sun God Nika (Gear 5)** - One Piece
   - Colors: Sun Gold, Nika White, Energy Orange
   - Effects: Sun rays, cloud wisps, rubber stretch
   - Rounded, bouncy UI (20px corners)

2. **Sung Jin-Woo - Shadow Monarch** - Solo Leveling
   - Colors: Monarch Purple, Void Black, Arise Glow
   - Effects: Shadow soldiers, void rifts, ARISE text
   - Sharp, angular UI (5px corners)

3. **Yugo - Portal Master** - Wakfu
   - Colors: Portal Cyan, Eliatrope Teal, Hat Gold
   - Effects: Portals, wakfu particles, dimensional rifts
   - Rounded portal-shaped UI (15px corners)

4. **Naruto - Sage Mode** - Naruto
   - Colors: Chakra Orange, Kurama Red, Sage Gold
   - Effects: Sage aura, Kurama flames, Rasengan

5. **Goku - Ultra Instinct** - Dragon Ball
   - Colors: UI Silver, Divine Silver, Aura White
   - Effects: Silver glow, speed lines, ki blasts

6. **Tanjiro - Sun Breathing** - Demon Slayer
   - Colors: Fire Orange, Hanafuda Red, Flame Glow
   - Effects: Sun flames, water techniques, Nichirin blade

7. **Gojo - Infinity** - Jujutsu Kaisen
   - Colors: Infinity Blue, Six Eyes Blue, Hollow Purple
   - Effects: Infinity aura, Six Eyes glow, domain expansion

8. **Levi - Humanity's Strongest** - Attack on Titan
   - Colors: Survey Green, Military Grey, Silver Blade
   - Effects: Blade shine, ODM gear motion, speed blur

9. **Saitama - One Punch** - One Punch Man
   - Colors: Hero Yellow, Cape Red, Golden Punch
   - Effects: Punch impact (minimal - he's too powerful)
   - Minimalist UI (0px corners)

10. **Deku - Plus Ultra** - My Hero Academia
    - Colors: Hero Green, OFA Lightning, Full Cowl
    - Effects: Lightning crackles, Full Cowl aura, power veins

---

## 🎮 Complete Game Systems

### Enhanced Snake Game
- 4 speed levels (Normal → Fast → Faster → Extreme)
- 3 lives system with respawn
- Themed visuals matching current character
- Animated food with pulsing effect
- Snake head with eyes based on direction
- XP and gem rewards

### Gacha Collection System
- **100 unique cards** across 10 anime series (10 per series)
- **5 rarity tiers**:
  - Common (50%) - White border
  - Rare (30%) - Blue border
  - Epic (15%) - Purple border
  - Legendary (4%) - Gold border
  - MYTHIC (1%) - Rainbow animated border
- Single pull: 100 gems
- 10x pull: 900 gems (guaranteed Epic+)
- Collection rewards at 25/50/75/100 cards
- Duplicate tracking

### Training Mini-Games Dojo
- **Reflex Test**: Tap flashing buttons, measures reaction time
  - Perfect: <150ms, Great: <250ms, Good: <400ms
- **Target Shoot**: 30-second shooting gallery
- **Speed Tap**: Tap as fast as possible in 10 seconds
- **Memory Match**: Remember increasingly long patterns
- Streak bonuses (3/7/14/30 days)
- XP rewards with perfect bonuses

### Boss Rush Mode
- **20 anime bosses** across 4 tiers:
  - Tier 1 (Lv.1-5): Buggy, Zabuza, Raditz, Hand Demon, Cursed Spirit
  - Tier 2 (Lv.6-10): Crocodile, Orochimaru, Frieza, Rui, Mahito
  - Tier 3 (Lv.11-15): Doflamingo, Pain, Cell, Akaza, Sukuna (4 Fingers)
  - Tier 4 (Lv.16-20): Kaido, Madara, Jiren, Muzan, Sukuna (Full Power)
- Turn-based combat with:
  - Attack (combo multiplier)
  - Defend (50% damage reduction)
  - Special (100 energy, 3x damage)
  - Item (3 potions, 50% heal)
- Combo system (up to +50% damage)
- No-damage and fast-clear bonuses
- Gem and XP rewards

### Battle Arena
- Pokemon Showdown-style battles
- 10 playable anime characters
- 4 moves per character
- Type effectiveness system

### Adventure Games
- Shadow Dungeon (Jin-Woo themed)
- Pirate Adventure (Luffy themed)
- Wakfu Quest (Yugo themed)
- Memory Match (4x4 card matching)

---

## 💎 Gem Economy

### Earning Gems:
- Daily login: 50 gems
- Quest completion: 20-100 gems
- Level up: 100 gems
- Boss defeat: 50-1000 gems
- Game wins: 10-50 gems
- Step goal completion: 30 gems
- Collection milestones: 500-5000 gems

### Spending Gems:
- Single gacha pull: 100 gems
- 10x gacha pull: 900 gems

---

## 🌟 Glass Morphism UI

All UI components feature:
- Semi-transparent backgrounds
- Soft blur effects (simulated)
- Themed borders with glow
- Character-specific corner radius
- Animated stat bars
- Pulsing glow effects

---

## 📊 Character Stats Screen

- Character portrait area
- 4 animated stat bars per character
- Level display with stars
- Signature catchphrase
- Glass panel containers

---

## ⚙️ Hardware Requirements

- **Board**: ESP32-S3-Touch-AMOLED-1.8 (Waveshare)
- **Display**: 368x448 AMOLED (QSPI)
- **Touch**: FT3168 Capacitive Touch
- **PMU**: AXP2101 Power Management
- **IMU**: QMI8658 (accelerometer/gyroscope)
- **RTC**: PCF85063 Real-time Clock
- **Audio**: ES8311 Codec
- **Storage**: SD Card (FAT32)

---

## 📁 File Structure

```
ESP32_Watch/
├── ESP32_Watch.ino    # Main entry point
├── config.h           # All configurations, enums, structures
├── optimizations.h    # NEW: Performance & stability helpers
├── themes.h/cpp       # 10 character themes, watch faces
├── games.h/cpp        # Game menu, Battle Arena, Snake, Memory
├── gacha.h/cpp        # Gacha collection system
├── training.h/cpp     # Training mini-games
├── boss_rush.h/cpp    # Boss Rush mode
├── rpg.h/cpp          # RPG progression system
├── apps.h/cpp         # Standard apps (settings, music, etc.)
├── ui.h/cpp           # UI utilities
├── display.h/cpp      # Display driver
├── touch.h/cpp        # Touch handling
├── hardware.h/cpp     # Hardware abstraction
├── filesystem.h/cpp   # SD card file system
└── wifi_apps.h/cpp    # WiFi and network features
```

---

## 🛠️ Troubleshooting

### Watch freezes/resets
- Watchdog timer will auto-reboot after 30 seconds of freeze
- Check Serial monitor for crash logs

### Memory issues
- `printMemoryStatus()` shows current heap usage
- Keep free heap above 50KB for stable operation
- Large arrays automatically use PSRAM if available

### Touch not responding
- 50ms debounce prevents accidental double-taps
- `onTouchActivity()` resets screen timeout

### Data not saving
- Auto-save runs every 5 minutes
- Manual save: `saveAllData()`
- Check NVS namespace "esp32watch"

---

## 🚀 Building & Flashing

1. Install Arduino IDE or PlatformIO
2. Install required libraries:
   - `Arduino_GFX_Library`
   - `LVGL`
   - `XPowersLib`
   - `Adafruit_XCA9554`
3. Select board: ESP32-S3 Dev Module
4. Set partition scheme: Huge APP (3MB)
5. Upload to device

---

## 📝 Changelog

### v2.0 - Complete Edition
- Added 7 new character themes (Naruto, Goku, Tanjiro, Gojo, Levi, Saitama, Deku)
- Implemented complete Gacha collection system with 100 cards
- Added Training Dojo with 4 mini-games
- Added Boss Rush mode with 20 bosses
- Enhanced Snake game with lives, speed levels, themed visuals
- Glass morphism UI throughout
- Gem economy system
- Daily rotation for themes
- Character stats screen

### v1.0 - Original
- 3 character themes (Luffy, Jin-Woo, Yugo)
- Basic games (Battle Arena, Snake, Memory)
- RPG progression system
- Quest system
- WiFi apps

---

## 🙏 Credits

- Original concept and code base
- Anime references from their respective creators
- ESP32 and Arduino community

---

**Enjoy your anime smartwatch! 🎮⌚**
