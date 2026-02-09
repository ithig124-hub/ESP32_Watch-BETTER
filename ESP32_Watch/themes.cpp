/**
 * Theme System Implementation
 * Clean, controlled aesthetics - no bouncing/rubber physics
 * 
 * Design Guidelines:
 * - Yugo: Chill/Exploration - magical but calm, soft cel-shaded
 * - Jinwoo: Dark/Power/Focus - minimal, intense, zero distractions
 * - Luffy Gear 5: Clean Fun - bright whites, soft pastels, joyful through color
 * - Random: Daily rotating anime characters with unique themes
 */

#include "themes.h"
#include "ui_manager.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN THEME DEFINITIONS (Static, Clean)
// ═══════════════════════════════════════════════════════════════════════════════

// Yugo - Wakfu Portal Master (Chill/Exploration)
static const ThemeColors yugoColors = {
  .primary    = 0x00CED1,  // Dark Cyan - Wakfu energy
  .secondary  = 0x0077BE,  // Ocean Blue
  .accent     = 0x40E0D0,  // Turquoise portal glow
  .background = 0x001419,  // Deep dark teal
  .text       = 0xE0FFFF,  // Light cyan
  .glow       = 0x00FFFF,  // Cyan glow
  .highlight  = 0x20B2AA   // Light sea green
};

// Jin-Woo - Shadow Monarch (Dark/Power/Focus)
static const ThemeColors jinwooColors = {
  .primary    = 0x9A0EEA,  // Deep Purple
  .secondary  = 0x4B0082,  // Indigo
  .accent     = 0xB266FF,  // Light purple glow
  .background = 0x0A0014,  // Near black purple
  .text       = 0xE8E8E8,  // Silver
  .glow       = 0x8B00FF,  // Violet
  .highlight  = 0x6A0DAD   // Royal purple
};

// Luffy Gear 5 - Sun God Nika (Clean Fun/Freedom - NO BOUNCING)
static const ThemeColors luffyColors = {
  .primary    = 0xFFFFFF,  // Pure White (Gear 5 aesthetic)
  .secondary  = 0xFFF0F5,  // Lavender blush (soft pastel)
  .accent     = 0xFFD700,  // Gold accents
  .background = 0xFFFAFA,  // Snow white background
  .text       = 0x2F2F2F,  // Dark grey for contrast
  .glow       = 0xFFE4E1,  // Misty rose
  .highlight  = 0xFFA07A   // Light salmon
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RANDOM CHARACTER THEME DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════════════

// Naruto - Sage Mode (Orange/Yellow chakra)
static const ThemeColors narutoColors = {
  .primary    = 0xFF6B00,  // Naruto Orange
  .secondary  = 0xFFD93D,  // Sage yellow
  .accent     = 0xFF4500,  // Kurama red-orange
  .background = 0x1A0A00,  // Dark warm
  .text       = 0xFFF8DC,  // Cornsilk
  .glow       = 0xFFA500,  // Orange glow
  .highlight  = 0xFFB347   // Pastel orange
};

// Goku - Super Saiyan (Gold/Blue power)
static const ThemeColors gokuColors = {
  .primary    = 0xFFD700,  // Super Saiyan Gold
  .secondary  = 0x00BFFF,  // Deep Sky Blue (SSGSS)
  .accent     = 0xFFFF00,  // Yellow aura
  .background = 0x0D0D1A,  // Dark space
  .text       = 0xF0F8FF,  // Alice blue
  .glow       = 0xFFFACD,  // Lemon chiffon
  .highlight  = 0x87CEEB   // Sky blue
};

// Tanjiro - Sun Breathing (Red/Orange flames)
static const ThemeColors tanjiroColors = {
  .primary    = 0x8B0000,  // Dark Red (Hinokami)
  .secondary  = 0x4169E1,  // Royal Blue (Water)
  .accent     = 0xFF4500,  // Orange-red flames
  .background = 0x0A0A0F,  // Deep night
  .text       = 0xFFE4E1,  // Misty rose
  .glow       = 0xFF6347,  // Tomato (flame)
  .highlight  = 0x228B22   // Forest green (haori)
};

// Gojo - Infinity (Blue/White domain)
static const ThemeColors gojoColors = {
  .primary    = 0x00D4FF,  // Cyan infinity
  .secondary  = 0xFFFFFF,  // Pure white
  .accent     = 0x4169E1,  // Royal blue (Six Eyes)
  .background = 0x000814,  // Void black
  .text       = 0xE0FFFF,  // Light cyan
  .glow       = 0x00FFFF,  // Aqua
  .highlight  = 0x87CEFA   // Light sky blue
};

// Levi - Humanity's Strongest (Green/Grey military)
static const ThemeColors leviColors = {
  .primary    = 0x2E8B57,  // Sea Green (Survey Corps)
  .secondary  = 0x708090,  // Slate grey
  .accent     = 0xC0C0C0,  // Silver (blades)
  .background = 0x1A1A1A,  // Dark grey
  .text       = 0xF5F5F5,  // White smoke
  .glow       = 0x00FF7F,  // Spring green (gas)
  .highlight  = 0x8B4513   // Saddle brown (leather)
};

// Saitama - One Punch (Yellow/White simple)
static const ThemeColors saitamaColors = {
  .primary    = 0xFFD700,  // Gold (cape)
  .secondary  = 0xFFFFFF,  // White (suit)
  .accent     = 0xDC143C,  // Crimson (gloves)
  .background = 0xFFFAF0,  // Floral white
  .text       = 0x2F2F2F,  // Dark grey
  .glow       = 0xFFF44F,  // Yellow
  .highlight  = 0xFFE4B5   // Moccasin
};

// Deku - One For All (Green/Lightning)
static const ThemeColors dekuColors = {
  .primary    = 0x228B22,  // Forest Green
  .secondary  = 0x00CED1,  // Dark Cyan (lightning)
  .accent     = 0xFFD700,  // Gold (All Might)
  .background = 0x0A140A,  // Dark green
  .text       = 0xF0FFF0,  // Honeydew
  .glow       = 0x00FF00,  // Lime (OFA sparks)
  .highlight  = 0x7CFC00   // Lawn green
};

// ═══════════════════════════════════════════════════════════════════════════════
//  CHARACTER NAMES AND TITLES
// ═══════════════════════════════════════════════════════════════════════════════

static const char* randomCharNames[] = {
  "Naruto",
  "Goku",
  "Tanjiro",
  "Gojo",
  "Levi",
  "Saitama",
  "Deku"
};

static const char* randomCharTitles[] = {
  "Seventh Hokage",
  "Super Saiyan God",
  "Sun Hashira",
  "The Strongest",
  "Humanity's Hope",
  "Hero for Fun",
  "Symbol of Peace"
};

static const char* randomCharSeries[] = {
  "Naruto",
  "Dragon Ball",
  "Demon Slayer",
  "Jujutsu Kaisen",
  "Attack on Titan",
  "One Punch Man",
  "My Hero Academia"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

void initThemes() {
  updateDailyCharacter();
  applyTheme(watch.theme);
}

RandomCharacter calculateDailyCharacter() {
  // Calculate day of year for rotation
  int dayOfYear = watch.day;
  for (int m = 1; m < watch.month; m++) {
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    dayOfYear += daysInMonth[m - 1];
  }
  // Rotate through 7 characters based on day
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
    Serial.printf("[THEME] Daily character: %s\n", getRandomCharacterName(watch.dailyCharacter));
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
    case THEME_LUFFY:  return "Gear 5 - Sun God";
    case THEME_JINWOO: return "Jin-Woo - Shadow";
    case THEME_YUGO:   return "Yugo - Portal";
    case THEME_RANDOM: return "Random Daily";
    default:           return "Unknown";
  }
}

const char* getRandomCharacterName(RandomCharacter character) {
  if (character < RANDOM_CHAR_COUNT) {
    return randomCharNames[character];
  }
  return "Unknown";
}

const char* getRandomCharacterTitle(RandomCharacter character) {
  if (character < RANDOM_CHAR_COUNT) {
    return randomCharTitles[character];
  }
  return "Unknown";
}

const char* getRandomCharacterSeries(RandomCharacter character) {
  if (character < RANDOM_CHAR_COUNT) {
    return randomCharSeries[character];
  }
  return "Unknown";
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
//  ANIME UI ELEMENTS (Static, Clean)
// ═══════════════════════════════════════════════════════════════════════════════

void drawSpeechBubble(lv_obj_t* parent, const char* text, int x, int y) {
  lv_obj_t* bubble = lv_obj_create(parent);
  lv_obj_set_size(bubble, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_pos(bubble, x, y);
  lv_obj_set_style_bg_color(bubble, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(bubble, 15, 0);
  lv_obj_set_style_border_color(bubble, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(bubble, 2, 0);
  lv_obj_set_style_pad_all(bubble, 10, 0);
  
  lv_obj_t* lbl = lv_label_create(bubble);
  lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_label_set_text(lbl, text);
}

void drawActionLines(lv_obj_t* parent, uint32_t color) {
  // Static action lines radiating from center (no animation)
  for (int i = 0; i < 8; i++) {
    lv_obj_t* line = lv_line_create(parent);
    static lv_point_t points[2];
    
    int angle = i * 45;
    int cx = LCD_WIDTH / 2;
    int cy = LCD_HEIGHT / 2;
    int r1 = 80;
    int r2 = 150;
    
    points[0].x = cx + r1 * cos(angle * PI / 180);
    points[0].y = cy + r1 * sin(angle * PI / 180);
    points[1].x = cx + r2 * cos(angle * PI / 180);
    points[1].y = cy + r2 * sin(angle * PI / 180);
    
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_line_color(line, lv_color_hex(color), 0);
    lv_obj_set_style_line_width(line, 2, 0);
    lv_obj_set_style_line_opa(line, LV_OPA_50, 0);
  }
}

void drawPowerAura(lv_obj_t* parent, uint32_t color, int intensity) {
  // Static glow effect (concentric circles, no animation)
  for (int i = 3; i > 0; i--) {
    lv_obj_t* circle = lv_obj_create(parent);
    int size = 100 + (i * 30 * intensity / 100);
    lv_obj_set_size(circle, size, size);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_10 + (3 - i) * 10, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WALLPAPER/THEME SELECTOR SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

lv_obj_t* createWallpaperScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);

  createTitleBar(scr, "Themes");

  // Theme buttons including Random
  const char* names[] = {
    "Gear 5\nClean Fun", 
    "Jin-Woo\nDark Power", 
    "Yugo\nExploration",
    "Random\nDaily Hero"
  };
  const uint32_t btnColors[] = {0xFFFFFF, 0x9A0EEA, 0x00CED1, 0xFF6B00};
  const uint32_t textColors[] = {0x2F2F2F, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};

  for (int i = 0; i < 4; i++) {
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, LCD_WIDTH - 40, 85);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 60 + i * 95);
    lv_obj_set_style_bg_color(btn, lv_color_hex(btnColors[i]), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ThemeType theme = (ThemeType)(intptr_t)lv_obj_get_user_data(btn);
      applyTheme(theme);

      // Refresh screens with new theme
      for (int j = 0; j < 16; j++) {
        if (screens[j]) {
          lv_obj_del(screens[j]);
          screens[j] = nullptr;
        }
      }
      showScreen(SCREEN_CLOCK);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(textColors[i]), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lbl, names[i]);
    lv_obj_center(lbl);

    // Current indicator
    if (i == (int)watch.theme) {
      lv_obj_t* check = lv_label_create(btn);
      lv_obj_set_style_text_color(check, lv_color_hex(textColors[i]), 0);
      lv_label_set_text(check, LV_SYMBOL_OK);
      lv_obj_align(check, LV_ALIGN_RIGHT_MID, -10, 0);
    }
    
    // Show today's character for Random theme
    if (i == THEME_RANDOM) {
      lv_obj_t* charLbl = lv_label_create(btn);
      lv_obj_set_style_text_font(charLbl, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(charLbl, lv_color_hex(0xFFFFFF), 0);
      lv_label_set_text_fmt(charLbl, "Today: %s", getRandomCharacterName(watch.dailyCharacter));
      lv_obj_align(charLbl, LV_ALIGN_BOTTOM_MID, 0, -5);
    }
  }

  return scr;
}
