/**
 * Premium Theme System Implementation
 * Apple Watch-Inspired Luxury Anime Themes
 * 
 * Design Philosophy:
 * - Deep, rich backgrounds with subtle gradients
 * - Glassmorphism effects for cards and overlays
 * - Premium color palettes inspired by luxury watches
 * - Clean typography with San Francisco-like fonts
 * - Subtle glow effects for emphasis
 */

#include "themes.h"
#include "ui_manager.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM THEME DEFINITIONS - APPLE WATCH LUXURY STYLE
// ═══════════════════════════════════════════════════════════════════════════════

// Luffy Gear 5 - Sun God Nika (Premium White/Gold)
// Inspired by Apple Watch Hermes Edition + Sun God aesthetic
static const ThemeColors luffyColors = {
  .primary       = 0xFFD700,  // Premium Gold
  .secondary     = 0xFFF5E6,  // Warm cream white
  .accent        = 0xFFB347,  // Soft orange gold
  .background    = 0x0D0D0D,  // Pure black (OLED perfect black)
  .surface       = 0x1A1A1A,  // Elevated card surface
  .text          = 0xFFFFFF,  // Pure white text
  .textSecondary = 0xB3B3B3,  // Muted silver text
  .glow          = 0xFFE4B5,  // Warm golden glow
  .gradient1     = 0xFFD700,  // Gold gradient start
  .gradient2     = 0xFFFFFF,  // White gradient end
  .border        = 0x333333,  // Subtle borders
  .highlight     = 0xFFF8DC   // Cornsilk highlight
};

// Jin-Woo Shadow Monarch - Solo Leveling (Premium Purple/Black)
// Inspired by Apple Watch midnight + Shadow King aesthetic
static const ThemeColors jinwooColors = {
  .primary       = 0x9370DB,  // Medium purple (Shadow energy)
  .secondary     = 0x6A0DAD,  // Royal purple
  .accent        = 0xE6E6FA,  // Lavender highlights
  .background    = 0x050508,  // Deep void black
  .surface       = 0x12101A,  // Shadow card surface
  .text          = 0xE8E8E8,  // Silver text
  .textSecondary = 0x8A8A9A,  // Muted purple-grey
  .glow          = 0x8B00FF,  // Violet glow
  .gradient1     = 0x4B0082,  // Indigo gradient start
  .gradient2     = 0x9370DB,  // Purple gradient end
  .border        = 0x2A2030,  // Dark purple border
  .highlight     = 0xB19CD9   // Light purple highlight
};

// Yugo Portal Master - Wakfu (Premium Teal/Cyan)
// Inspired by Apple Watch Pacific Blue + Portal energy
static const ThemeColors yugoColors = {
  .primary       = 0x00D4AA,  // Wakfu teal
  .secondary     = 0x0097A7,  // Deep cyan
  .accent        = 0x40E0D0,  // Turquoise accent
  .background    = 0x030D0D,  // Deep dark teal
  .surface       = 0x0A1A1A,  // Elevated teal surface
  .text          = 0xE0FFFF,  // Light cyan text
  .textSecondary = 0x7FDBDB,  // Muted cyan
  .glow          = 0x00FFFF,  // Cyan glow
  .gradient1     = 0x006666,  // Dark teal gradient
  .gradient2     = 0x00D4AA,  // Bright teal gradient
  .border        = 0x1A3333,  // Subtle teal border
  .highlight     = 0x20B2AA   // Light sea green
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RANDOM CHARACTER PREMIUM THEMES
// ═══════════════════════════════════════════════════════════════════════════════

// Naruto - Sage Mode (Premium Orange/Gold)
static const ThemeColors narutoColors = {
  .primary       = 0xFF8C00,  // Dark orange (Chakra)
  .secondary     = 0xFFB347,  // Soft orange
  .accent        = 0xFFD700,  // Gold accents
  .background    = 0x0D0805,  // Warm dark
  .surface       = 0x1A1410,  // Warm card surface
  .text          = 0xFFF8DC,  // Cornsilk text
  .textSecondary = 0xD2B48C,  // Tan muted
  .glow          = 0xFF6600,  // Orange glow
  .gradient1     = 0xFF4500,  // Orange-red
  .gradient2     = 0xFFD700,  // Gold
  .border        = 0x2A1A10,  // Warm border
  .highlight     = 0xFFA500   // Orange highlight
};

// Goku - Super Saiyan God (Premium Gold/Blue)
static const ThemeColors gokuColors = {
  .primary       = 0xFFD700,  // Super Saiyan Gold
  .secondary     = 0x4169E1,  // Royal Blue (SSGSS)
  .accent        = 0x00BFFF,  // Deep sky blue
  .background    = 0x050510,  // Deep space black
  .surface       = 0x101020,  // Space card surface
  .text          = 0xF0F8FF,  // Alice blue text
  .textSecondary = 0xADD8E6,  // Light blue muted
  .glow          = 0xFFFF00,  // Yellow aura
  .gradient1     = 0x4169E1,  // Blue
  .gradient2     = 0xFFD700,  // Gold
  .border        = 0x1A1A30,  // Blue-ish border
  .highlight     = 0x87CEEB   // Sky blue
};

// Tanjiro - Sun Breathing (Premium Red/Fire)
static const ThemeColors tanjiroColors = {
  .primary       = 0xDC143C,  // Crimson (Hinokami)
  .secondary     = 0x4169E1,  // Royal Blue (Water)
  .accent        = 0xFF6347,  // Tomato flame
  .background    = 0x0A0505,  // Deep dark
  .surface       = 0x1A1010,  // Dark red surface
  .text          = 0xFFE4E1,  // Misty rose text
  .textSecondary = 0xCD853F,  // Peru muted
  .glow          = 0xFF4500,  // Orange-red glow
  .gradient1     = 0x8B0000,  // Dark red
  .gradient2     = 0xFF6347,  // Tomato
  .border        = 0x2A1515,  // Dark red border
  .highlight     = 0xFF7F50   // Coral
};

// Gojo - Infinity (Premium Cyan/White)
static const ThemeColors gojoColors = {
  .primary       = 0x00D4FF,  // Cyan infinity
  .secondary     = 0xFFFFFF,  // Pure white
  .accent        = 0x87CEEB,  // Sky blue
  .background    = 0x000510,  // Void black with blue tint
  .surface       = 0x0A1020,  // Blue void surface
  .text          = 0xE0FFFF,  // Light cyan text
  .textSecondary = 0x7EC8E3,  // Muted cyan
  .glow          = 0x00FFFF,  // Aqua glow
  .gradient1     = 0x000033,  // Dark blue
  .gradient2     = 0x00D4FF,  // Cyan
  .border        = 0x102030,  // Blue border
  .highlight     = 0xB0E0E6   // Powder blue
};

// Levi - Humanity's Strongest (Premium Military Green/Grey)
static const ThemeColors leviColors = {
  .primary       = 0x2E8B57,  // Sea Green (Survey Corps)
  .secondary     = 0x708090,  // Slate grey
  .accent        = 0xC0C0C0,  // Silver (blades)
  .background    = 0x0A0A0A,  // Pure dark
  .surface       = 0x151515,  // Military dark surface
  .text          = 0xF5F5F5,  // White smoke text
  .textSecondary = 0xA9A9A9,  // Dark grey muted
  .glow          = 0x00FF7F,  // Spring green
  .gradient1     = 0x1A3320,  // Dark green
  .gradient2     = 0x2E8B57,  // Sea green
  .border        = 0x202020,  // Dark border
  .highlight     = 0x8FBC8F   // Dark sea green
};

// Saitama - One Punch (Premium Yellow/Simple)
static const ThemeColors saitamaColors = {
  .primary       = 0xFFD700,  // Gold (cape)
  .secondary     = 0xFFFFFF,  // White (suit)
  .accent        = 0xDC143C,  // Crimson (gloves)
  .background    = 0x0D0D0D,  // Pure black
  .surface       = 0x1A1A1A,  // Simple dark surface
  .text          = 0xFFFFFF,  // Pure white text
  .textSecondary = 0xB0B0B0,  // Grey muted
  .glow          = 0xFFFF00,  // Yellow glow
  .gradient1     = 0xFFD700,  // Gold
  .gradient2     = 0xFFFFFF,  // White
  .border        = 0x252525,  // Subtle border
  .highlight     = 0xFFF44F   // Bright yellow
};

// Deku - One For All (Premium Green/Lightning)
static const ThemeColors dekuColors = {
  .primary       = 0x32CD32,  // Lime green (OFA)
  .secondary     = 0x00CED1,  // Dark cyan (lightning)
  .accent        = 0xFFD700,  // Gold (All Might)
  .background    = 0x050A05,  // Dark green
  .surface       = 0x101A10,  // Green surface
  .text          = 0xF0FFF0,  // Honeydew text
  .textSecondary = 0x90EE90,  // Light green muted
  .glow          = 0x00FF00,  // Lime glow
  .gradient1     = 0x006400,  // Dark green
  .gradient2     = 0x32CD32,  // Lime green
  .border        = 0x152015,  // Green border
  .highlight     = 0x7CFC00   // Lawn green
};

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER METADATA
// ═══════════════════════════════════════════════════════════════════════════════

static const char* randomCharNames[] = {
  "Naruto", "Goku", "Tanjiro", "Gojo", "Levi", "Saitama", "Deku"
};

static const char* randomCharTitles[] = {
  "Seventh Hokage", "Super Saiyan God", "Sun Hashira", 
  "The Strongest", "Humanity's Hope", "Hero for Fun", "Symbol of Peace"
};

static const char* randomCharSeries[] = {
  "Naruto", "Dragon Ball", "Demon Slayer",
  "Jujutsu Kaisen", "Attack on Titan", "One Punch Man", "My Hero Academia"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

void initThemes() {
  updateDailyCharacter();
  applyTheme(watch.theme);
  Serial.println("[OK] Premium Themes Initialized");
}

RandomCharacter calculateDailyCharacter() {
  int dayOfYear = watch.day;
  for (int m = 1; m < watch.month; m++) {
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    dayOfYear += daysInMonth[m - 1];
  }
  return (RandomCharacter)(dayOfYear % RANDOM_CHAR_COUNT);
}

void updateDailyCharacter() {
  int dayOfYear = watch.day;
  for (int m = 1; m < watch.month; m++) {
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    dayOfYear += daysInMonth[m - 1];
  }

  if (watch.lastRandomDay != dayOfYear) {
    watch.lastRandomDay = dayOfYear;
    watch.dailyCharacter = calculateDailyCharacter();
    Serial.printf("[THEME] Today's character: %s - %s\n", 
      getRandomCharacterName(watch.dailyCharacter),
      getRandomCharacterTitle(watch.dailyCharacter));
  }
}

ThemeColors getRandomCharacterColors(RandomCharacter character) {
  switch(character) {
    case CHAR_NARUTO:  return narutoColors;
    case CHAR_GOKU:    return gokuColors;
    case CHAR_TANJIRO: return tanjiroColors;
    case CHAR_GOJO:    return gojoColors;
    case CHAR_LEVI:    return leviColors;
    case CHAR_SAITAMA: return saitamaColors;
    case CHAR_DEKU:    return dekuColors;
    default:           return narutoColors;
  }
}

ThemeColors getThemeColors(ThemeType theme) {
  switch(theme) {
    case THEME_LUFFY:  return luffyColors;
    case THEME_JINWOO: return jinwooColors;
    case THEME_YUGO:   return yugoColors;
    case THEME_RANDOM: return getRandomCharacterColors(watch.dailyCharacter);
    default:           return luffyColors;
  }
}

const char* getThemeName(ThemeType theme) {
  switch(theme) {
    case THEME_LUFFY:  return "Gear 5 Sun God";
    case THEME_JINWOO: return "Shadow Monarch";
    case THEME_YUGO:   return "Portal Master";
    case THEME_RANDOM: return "Daily Hero";
    default:           return "Unknown";
  }
}

const char* getRandomCharacterName(RandomCharacter character) {
  return (character < RANDOM_CHAR_COUNT) ? randomCharNames[character] : "Unknown";
}

const char* getRandomCharacterTitle(RandomCharacter character) {
  return (character < RANDOM_CHAR_COUNT) ? randomCharTitles[character] : "Unknown";
}

const char* getRandomCharacterSeries(RandomCharacter character) {
  return (character < RANDOM_CHAR_COUNT) ? randomCharSeries[character] : "Unknown";
}

void applyTheme(ThemeType theme) {
  watch.theme = theme;
  if (theme == THEME_RANDOM) {
    updateDailyCharacter();
  }
  userData.theme = theme;
  saveUserData();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PREMIUM UI ELEMENTS (Apple Watch Style)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Glassmorphism Card - Premium frosted glass effect
 * Simulated on LVGL with layered transparency
 */
void drawGlassmorphismCard(lv_obj_t* parent, int x, int y, int w, int h, uint32_t tint) {
  ThemeColors colors = getThemeColors(watch.theme);
  
  // Outer glow/shadow layer
  lv_obj_t* glow = lv_obj_create(parent);
  lv_obj_set_size(glow, w + 4, h + 4);
  lv_obj_set_pos(glow, x - 2, y - 2);
  lv_obj_set_style_bg_color(glow, lv_color_hex(colors.glow), 0);
  lv_obj_set_style_bg_opa(glow, LV_OPA_10, 0);
  lv_obj_set_style_radius(glow, 18, 0);
  lv_obj_set_style_border_width(glow, 0, 0);
  lv_obj_clear_flag(glow, LV_OBJ_FLAG_CLICKABLE);
  
  // Main glass card
  lv_obj_t* card = lv_obj_create(parent);
  lv_obj_set_size(card, w, h);
  lv_obj_set_pos(card, x, y);
  lv_obj_set_style_bg_color(card, lv_color_hex(colors.surface), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_80, 0);  // Semi-transparent
  lv_obj_set_style_radius(card, 16, 0);
  lv_obj_set_style_border_color(card, lv_color_hex(colors.border), 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_border_opa(card, LV_OPA_50, 0);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
  
  // Inner highlight (top edge reflection)
  lv_obj_t* highlight = lv_obj_create(card);
  lv_obj_set_size(highlight, w - 20, 2);
  lv_obj_align(highlight, LV_ALIGN_TOP_MID, 0, 4);
  lv_obj_set_style_bg_color(highlight, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(highlight, LV_OPA_10, 0);
  lv_obj_set_style_radius(highlight, 1, 0);
  lv_obj_set_style_border_width(highlight, 0, 0);
  lv_obj_clear_flag(highlight, LV_OBJ_FLAG_CLICKABLE);
}

/**
 * Premium Glow Effect - Subtle ambient lighting
 */
void drawPremiumGlow(lv_obj_t* parent, uint32_t color, int intensity) {
  // Multi-layer glow for premium effect
  for (int i = 3; i > 0; i--) {
    lv_obj_t* glow = lv_obj_create(parent);
    int size = 150 + (i * 50 * intensity / 100);
    lv_obj_set_size(glow, size, size);
    lv_obj_align(glow, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(glow, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_TRANSP + (4 - i) * 3, 0);
    lv_obj_set_style_border_width(glow, 0, 0);
    lv_obj_clear_flag(glow, LV_OBJ_FLAG_CLICKABLE);
  }
}

/**
 * Anime Power Aura - Character-specific glow effects
 */
void drawAnimePowerAura(lv_obj_t* parent, uint32_t color, int intensity) {
  // Concentric rings with fade
  for (int i = 0; i < 4; i++) {
    lv_obj_t* ring = lv_obj_create(parent);
    int size = 280 - i * 40;
    lv_obj_set_size(ring, size, size);
    lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_0, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(ring, 1, 0);
    lv_obj_set_style_border_opa(ring, LV_OPA_10 + i * 8, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME SELECTOR SCREEN (Premium Style)
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createWallpaperScreen() {
  ThemeColors colors = getThemeColors(watch.theme);
  
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);

  createTitleBar(scr, "Themes");

  // Premium theme cards
  const char* names[] = {
    "Gear 5\nSun God",
    "Jin-Woo\nShadow Monarch",
    "Yugo\nPortal Master",
    "Random\nDaily Hero"
  };
  
  const char* subtitles[] = {
    "Clean white & gold",
    "Deep purple luxury",
    "Teal exploration",
    "New hero every day"
  };
  
  const uint32_t cardColors[] = {0xFFD700, 0x9370DB, 0x00D4AA, 0xFF8C00};

  for (int i = 0; i < 4; i++) {
    // Card background with glow
    lv_obj_t* cardBg = lv_obj_create(scr);
    lv_obj_set_size(cardBg, LCD_WIDTH - 30, 85);
    lv_obj_align(cardBg, LV_ALIGN_TOP_MID, 0, 58 + i * 92);
    lv_obj_set_style_bg_color(cardBg, lv_color_hex(colors.surface), 0);
    lv_obj_set_style_bg_opa(cardBg, LV_OPA_90, 0);
    lv_obj_set_style_radius(cardBg, 16, 0);
    lv_obj_set_style_border_color(cardBg, lv_color_hex(cardColors[i]), 0);
    lv_obj_set_style_border_width(cardBg, 2, 0);
    lv_obj_set_style_border_opa(cardBg, i == (int)watch.theme ? LV_OPA_100 : LV_OPA_30, 0);
    lv_obj_set_user_data(cardBg, (void*)(intptr_t)i);

    lv_obj_add_event_cb(cardBg, [](lv_event_t* e) {
      lv_obj_t* card = lv_event_get_target(e);
      ThemeType theme = (ThemeType)(intptr_t)lv_obj_get_user_data(card);
      applyTheme(theme);

      // Refresh to show new theme
      for (int j = 0; j < 16; j++) {
        if (screens[j]) {
          lv_obj_del(screens[j]);
          screens[j] = nullptr;
        }
      }
      showScreen(SCREEN_CLOCK);
    }, LV_EVENT_CLICKED, NULL);

    // Accent stripe on left
    lv_obj_t* stripe = lv_obj_create(cardBg);
    lv_obj_set_size(stripe, 4, 65);
    lv_obj_align(stripe, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_color(stripe, lv_color_hex(cardColors[i]), 0);
    lv_obj_set_style_radius(stripe, 2, 0);
    lv_obj_set_style_border_width(stripe, 0, 0);
    lv_obj_clear_flag(stripe, LV_OBJ_FLAG_CLICKABLE);

    // Theme name
    lv_obj_t* nameLbl = lv_label_create(cardBg);
    lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(nameLbl, lv_color_hex(colors.text), 0);
    lv_label_set_text(nameLbl, names[i]);
    lv_obj_align(nameLbl, LV_ALIGN_LEFT_MID, 22, -10);

    // Subtitle
    lv_obj_t* subLbl = lv_label_create(cardBg);
    lv_obj_set_style_text_font(subLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(subLbl, lv_color_hex(colors.textSecondary), 0);
    lv_label_set_text(subLbl, subtitles[i]);
    lv_obj_align(subLbl, LV_ALIGN_LEFT_MID, 22, 20);

    // Selected indicator
    if (i == (int)watch.theme) {
      lv_obj_t* check = lv_label_create(cardBg);
      lv_obj_set_style_text_color(check, lv_color_hex(cardColors[i]), 0);
      lv_obj_set_style_text_font(check, &lv_font_montserrat_20, 0);
      lv_label_set_text(check, LV_SYMBOL_OK);
      lv_obj_align(check, LV_ALIGN_RIGHT_MID, -15, 0);
    }

    // Today's character for Random
    if (i == THEME_RANDOM) {
      lv_obj_t* charLbl = lv_label_create(cardBg);
      lv_obj_set_style_text_font(charLbl, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(charLbl, lv_color_hex(cardColors[i]), 0);
      lv_label_set_text_fmt(charLbl, "Today: %s", getRandomCharacterName(watch.dailyCharacter));
      lv_obj_align(charLbl, LV_ALIGN_RIGHT_MID, -15, 15);
    }
  }

  return scr;
}
