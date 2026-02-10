/**
 * ═══════════════════════════════════════════════════════════════════════════════
 *  PREMIUM ANIME THEMES - Ultra High Quality Character Representation
 *  Each theme captures the essence, power, and aesthetic of the character
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *  MAIN THEMES:
 *  🌟 GEAR 5 LUFFY    - Sun God Nika | White/Gold/Red | Joy & Freedom
 *  🌑 JIN-WOO SHADOW  - Shadow Monarch | Purple/Black/Blue | Dark Power
 *  🌀 YUGO PORTAL     - Eliatrope | Cyan/Teal/Gold | Adventure & Mystery
 *
 *  DAILY RANDOM CHARACTERS:
 *  🦊 NARUTO   - Sage Mode | Orange/Black/Red | Will of Fire
 *  ⚡ GOKU     - Ultra Instinct | Silver/Blue/White | Limitless Power
 *  🔥 TANJIRO  - Sun Breathing | Red/Orange/Black | Determination
 *  ♾️ GOJO     - Infinity | Blue/Purple/White | Six Eyes
 *  ⚔️ LEVI     - Humanity's Strongest | Green/Grey/Silver | Precision
 *  👊 SAITAMA  - One Punch | Yellow/Red/White | Simple Strength
 *  💚 DEKU     - One For All | Green/Black/Lightning | Plus Ultra
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#ifndef THEMES_H
#define THEMES_H

#include <Arduino.h>
#include <lvgl.h>
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM COLOR PALETTES - Carefully crafted for each character
// ═══════════════════════════════════════════════════════════════════════════════

// RGB565 Color Macros
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// ───────────────────────────────────────────────────────────────────────────────
//  🌟 GEAR 5 LUFFY - Sun God Nika Theme
//  White clouds, golden sun rays, freedom and joy
// ───────────────────────────────────────────────────────────────────────────────
#define LUFFY_PRIMARY       0xFFFF  // Pure White (Nika form)
#define LUFFY_SECONDARY     0xFE60  // Brilliant Gold (Sun God power)
#define LUFFY_ACCENT        0xF800  // Vivid Red (Straw Hat)
#define LUFFY_BACKGROUND    0x0000  // Deep Black (contrast)
#define LUFFY_GLOW          0xFFE0  // Warm Yellow (sun aura)
#define LUFFY_HIGHLIGHT     0xFD20  // Orange (energy)
#define LUFFY_CLOUD         0xF7BE  // Soft Cloud White
#define LUFFY_SHADOW        0x2104  // Subtle shadow

// ───────────────────────────────────────────────────────────────────────────────
//  🌑 JIN-WOO SHADOW MONARCH - Arise Theme
//  Deep shadows, purple power, overwhelming dominance
// ───────────────────────────────────────────────────────────────────────────────
#define JINWOO_PRIMARY      0x780F  // Royal Purple (Monarch power)
#define JINWOO_SECONDARY    0x0010  // Void Black (Shadow realm)
#define JINWOO_ACCENT       0x001F  // Electric Blue (eyes)
#define JINWOO_BACKGROUND   0x0000  // Absolute Black
#define JINWOO_GLOW         0x881F  // Purple Glow (arise)
#define JINWOO_HIGHLIGHT    0x4A5F  // Ice Blue (daggers)
#define JINWOO_SHADOW       0x4008  // Dark Purple mist
#define JINWOO_ARISE        0xA01F  // Bright Purple (ARISE text)

// ───────────────────────────────────────────────────────────────────────────────
//  🌀 YUGO PORTAL MASTER - Eliatrope Theme
//  Cyan portals, mystical energy, adventure spirit
// ───────────────────────────────────────────────────────────────────────────────
#define YUGO_PRIMARY        0x07FF  // Bright Cyan (Wakfu portal)
#define YUGO_SECONDARY      0x0410  // Deep Teal (Eliatrope)
#define YUGO_ACCENT         0xFE00  // Warm Gold (hat/details)
#define YUGO_BACKGROUND     0x1082  // Dark Blue-Grey
#define YUGO_GLOW           0x5FFF  // Light Cyan (portal glow)
#define YUGO_HIGHLIGHT      0x07F0  // Seafoam (energy)
#define YUGO_PORTAL         0x2E7F  // Portal Ring Blue
#define YUGO_ENERGY         0x97FF  // Bright Wakfu energy

// ═══════════════════════════════════════════════════════════════════════════════
//  🎲 DAILY RANDOM CHARACTER PALETTES
// ═══════════════════════════════════════════════════════════════════════════════

// ───────────────────────────────────────────────────────────────────────────────
//  🦊 NARUTO - Sage Mode / Kurama
//  Orange chakra, sage eyes, Will of Fire
// ───────────────────────────────────────────────────────────────────────────────
#define NARUTO_PRIMARY      0xFD20  // Vibrant Orange (chakra)
#define NARUTO_SECONDARY    0x0000  // Black (outfit)
#define NARUTO_ACCENT       0xF800  // Red (Kurama)
#define NARUTO_BACKGROUND   0x10A2  // Dark slate
#define NARUTO_GLOW         0xFE60  // Golden (Sage Mode)
#define NARUTO_SAGE_EYE     0xF780  // Orange-Yellow (Sage eyes)
#define NARUTO_KURAMA       0xFA00  // Deep Orange-Red
#define NARUTO_RASENGAN     0x5DDF  // Blue Rasengan

// ───────────────────────────────────────────────────────────────────────────────
//  ⚡ GOKU - Ultra Instinct
//  Silver aura, divine power, limitless potential
// ───────────────────────────────────────────────────────────────────────────────
#define GOKU_PRIMARY        0xC618  // Silver (UI hair)
#define GOKU_SECONDARY      0x001F  // Deep Blue (gi)
#define GOKU_ACCENT         0xFFFF  // Pure White (aura)
#define GOKU_BACKGROUND     0x0000  // Black void
#define GOKU_GLOW           0xBDF7  // Silver-White glow
#define GOKU_AURA           0x9CF3  // Light Silver
#define GOKU_KI             0x5DFF  // Blue Ki blast
#define GOKU_DIVINE         0xEF7D  // Divine silver

// ───────────────────────────────────────────────────────────────────────────────
//  🔥 TANJIRO - Sun Breathing / Hinokami Kagura
//  Fire and water, determination, family bond
// ───────────────────────────────────────────────────────────────────────────────
#define TANJIRO_PRIMARY     0xB000  // Deep Crimson (Hanafuda)
#define TANJIRO_SECONDARY   0x0208  // Dark Green (checkered)
#define TANJIRO_ACCENT      0xFD60  // Fire Orange
#define TANJIRO_BACKGROUND  0x0841  // Dark charcoal
#define TANJIRO_GLOW        0xFC00  // Flame glow
#define TANJIRO_WATER       0x3DDF  // Water Breathing blue
#define TANJIRO_FIRE        0xF800  // Sun Breathing red
#define TANJIRO_SCAR        0xA000  // Scar burgundy

// ───────────────────────────────────────────────────────────────────────────────
//  ♾️ GOJO - Infinity / Six Eyes
//  Blue infinity, white blindfold, overwhelming power
// ───────────────────────────────────────────────────────────────────────────────
#define GOJO_PRIMARY        0x5DDF  // Infinity Blue
#define GOJO_SECONDARY      0xFFFF  // Pure White (hair)
#define GOJO_ACCENT         0x781F  // Purple (cursed energy)
#define GOJO_BACKGROUND     0x0000  // Black
#define GOJO_GLOW           0x6F3F  // Light Blue glow
#define GOJO_INFINITY       0x4A9F  // Deep Infinity blue
#define GOJO_DOMAIN         0x280F  // Domain purple
#define GOJO_EYES           0x5DFF  // Six Eyes blue

// ───────────────────────────────────────────────────────────────────────────────
//  ⚔️ LEVI - Humanity's Strongest
//  Military precision, Survey Corps, titan slayer
// ───────────────────────────────────────────────────────────────────────────────
#define LEVI_PRIMARY        0x3186  // Military Grey
#define LEVI_SECONDARY      0x2124  // Dark Grey (uniform)
#define LEVI_ACCENT         0x07E0  // Survey Corps Green
#define LEVI_BACKGROUND     0x0841  // Dark charcoal
#define LEVI_GLOW           0xC618  // Silver blade
#define LEVI_CAPE           0x0540  // Green cape
#define LEVI_BLADE          0xE71C  // Steel silver
#define LEVI_BLOOD          0x8000  // Titan blood

// ───────────────────────────────────────────────────────────────────────────────
//  👊 SAITAMA - One Punch Man
//  Simple, clean, overwhelming strength
// ───────────────────────────────────────────────────────────────────────────────
#define SAITAMA_PRIMARY     0xFFE0  // Yellow (suit)
#define SAITAMA_SECONDARY   0xF800  // Red (cape/gloves)
#define SAITAMA_ACCENT      0xFFFF  // White (bald head)
#define SAITAMA_BACKGROUND  0x0000  // Black
#define SAITAMA_GLOW        0xFEE0  // Golden punch
#define SAITAMA_CAPE        0xC000  // Deep Red
#define SAITAMA_FIST        0xFD20  // Orange impact
#define SAITAMA_EYES        0x0000  // Black dot eyes

// ───────────────────────────────────────────────────────────────────────────────
//  💚 DEKU - One For All
//  Green lightning, heroic spirit, Plus Ultra
// ───────────────────────────────────────────────────────────────────────────────
#define DEKU_PRIMARY        0x07E0  // Hero Green
#define DEKU_SECONDARY      0x0000  // Black (costume)
#define DEKU_ACCENT         0xFFE0  // Yellow (All Might legacy)
#define DEKU_BACKGROUND     0x0841  // Dark
#define DEKU_GLOW           0x5FE0  // Green lightning
#define DEKU_LIGHTNING      0xAFE5  // OFA Lightning
#define DEKU_FULL_COWL      0x37E6  // Full Cowl green
#define DEKU_VEINS          0xF800  // Power veins

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM THEME STRUCTURE
// ═══════════════════════════════════════════════════════════════════════════════

struct PremiumTheme {
  // Character Identity
  const char* characterName;
  const char* characterTitle;
  const char* seriesName;
  const char* signatureMove;
  const char* catchphrase;
  
  // Core Colors
  uint16_t primary;
  uint16_t secondary;
  uint16_t accent;
  uint16_t background;
  uint16_t text;
  uint16_t glow;
  uint16_t highlight;
  
  // Special Effect Colors
  uint16_t effectColor1;    // Primary effect
  uint16_t effectColor2;    // Secondary effect
  uint16_t effectColor3;    // Tertiary effect
  
  // UI Styling
  uint8_t cornerRadius;     // Border radius (0-25)
  uint8_t glowIntensity;    // Glow strength (0-255)
  uint8_t animationSpeed;   // Animation timing (1-10)
  bool useGradient;         // Enable gradients
  bool useGlow;             // Enable glow effects
  bool darkMode;            // Dark or light base
  
  // Character Stats Display
  const char* stat1Name;
  const char* stat2Name;
  const char* stat3Name;
  const char* stat4Name;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM THEME INSTANCES
// ═══════════════════════════════════════════════════════════════════════════════

extern const PremiumTheme THEME_GEAR5_LUFFY;
extern const PremiumTheme THEME_SHADOW_MONARCH;
extern const PremiumTheme THEME_PORTAL_MASTER;

extern const PremiumTheme THEME_SAGE_NARUTO;
extern const PremiumTheme THEME_UI_GOKU;
extern const PremiumTheme THEME_SUN_TANJIRO;
extern const PremiumTheme THEME_INFINITY_GOJO;
extern const PremiumTheme THEME_CAPTAIN_LEVI;
extern const PremiumTheme THEME_HERO_SAITAMA;
extern const PremiumTheme THEME_PLUS_ULTRA_DEKU;

// Current active theme
extern const PremiumTheme* currentTheme;

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Initialize all premium themes
 */
void initThemes();

/**
 * Set active theme by type
 */
void setTheme(ThemeType type);

/**
 * Get current theme colors
 */
const PremiumTheme* getCurrentTheme();

/**
 * Get random daily character theme
 */
const PremiumTheme* getDailyCharacterTheme();

/**
 * Apply theme to LVGL objects
 */
void applyThemeToScreen(lv_obj_t* screen);

/**
 * Create themed container with glow effect
 */
lv_obj_t* createThemedContainer(lv_obj_t* parent, int width, int height);

/**
 * Create themed button with character style
 */
lv_obj_t* createThemedButton(lv_obj_t* parent, const char* text);

/**
 * Create themed label with proper styling
 */
lv_obj_t* createThemedLabel(lv_obj_t* parent, const char* text, bool isTitle);

/**
 * Create character signature effect (visual flair)
 */
void createSignatureEffect(lv_obj_t* parent);

/**
 * Get theme color as LVGL color
 */
lv_color_t themeColor(uint16_t rgb565);

#endif
