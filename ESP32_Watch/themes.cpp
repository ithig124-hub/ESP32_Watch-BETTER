/*
 * themes.cpp - IMPROVED Theme Implementation
 * Modern Anime Gaming Smartwatch - Enhanced Edition
 * 
 * IMPROVEMENTS:
 * - Modern glass morphism UI throughout
 * - Better visual hierarchy
 * - Animated effects for each character
 * - Improved color contrast
 * - Premium watch face designs
 * - Dynamic day/night backgrounds
 * - Tap-to-switch elements for BoBoiBoy
 */

#include "themes.h"
#include "config.h"
#include "display.h"
#include "hardware.h"
#include "dynamic_bg.h"
#include "ochobot.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// =============================================================================
// THEME COLOR DEFINITIONS - ALL 10 CHARACTERS (IMPROVED)
// =============================================================================

// Luffy Gear 5 - Sun God Nika
ThemeColors luffy_gear5_theme = {
  .primary = LUFFY_SUN_GOLD,
  .secondary = LUFFY_NIKA_WHITE,
  .accent = LUFFY_ENERGY_ORANGE,
  .background = LUFFY_DEEP_BLACK,
  .text = COLOR_WHITE,
  .shadow = LUFFY_CLOUD_WHITE,
  .effect1 = LUFFY_SUN_GLOW,
  .effect2 = LUFFY_JOY_YELLOW,
  .effect3 = LUFFY_FREEDOM_SKY,
  .effect4 = LUFFY_RUBBER_PINK,
  .corner_radius = 20,
  .glow_intensity = 200,
  .animation_speed = 8,
  .border_width = 3
};

// Jin-Woo - Shadow Monarch
ThemeColors sung_jinwoo_theme = {
  .primary = JINWOO_MONARCH_PURPLE,
  .secondary = JINWOO_VOID_BLACK,
  .accent = JINWOO_ARISE_GLOW,
  .background = JINWOO_ABSOLUTE_DARK,
  .text = COLOR_WHITE,
  .shadow = JINWOO_DEATH_BLACK,
  .effect1 = JINWOO_ARISE_GLOW,
  .effect2 = JINWOO_DAGGER_BLUE,
  .effect3 = JINWOO_MIST_PURPLE,
  .effect4 = JINWOO_SOUL_PURPLE,
  .corner_radius = 5,
  .glow_intensity = 255,
  .animation_speed = 10,
  .border_width = 2
};

// Yugo - Portal Master
ThemeColors yugo_wakfu_theme = {
  .primary = YUGO_PORTAL_CYAN,
  .secondary = YUGO_ELIATROPE_TEAL,
  .accent = YUGO_HAT_GOLD,
  .background = YUGO_SKY_BLUE_GREY,
  .text = COLOR_WHITE,
  .shadow = YUGO_MYSTERY_BLUE,
  .effect1 = YUGO_PORTAL_GLOW,
  .effect2 = YUGO_WAKFU_ENERGY,
  .effect3 = YUGO_PORTAL_RING,
  .effect4 = YUGO_ENERGY_BURST,
  .corner_radius = 15,
  .glow_intensity = 150,
  .animation_speed = 7,
  .border_width = 3
};

// Naruto - Sage Mode
ThemeColors naruto_sage_theme = {
  .primary = NARUTO_CHAKRA_ORANGE,
  .secondary = NARUTO_OUTFIT_BLACK,
  .accent = NARUTO_SAGE_GOLD,
  .background = NARUTO_SLATE_GREY,
  .text = COLOR_WHITE,
  .shadow = NARUTO_SHADOW_ORANGE,
  .effect1 = NARUTO_KURAMA_FLAME,
  .effect2 = NARUTO_RASENGAN_BLUE,
  .effect3 = NARUTO_SAGE_EYES,
  .effect4 = NARUTO_WILL_FIRE,
  .corner_radius = 10,
  .glow_intensity = 180,
  .animation_speed = 7,
  .border_width = 2
};

// Goku - Ultra Instinct
ThemeColors goku_ui_theme = {
  .primary = GOKU_UI_SILVER,
  .secondary = GOKU_GI_BLUE,
  .accent = GOKU_AURA_WHITE,
  .background = GOKU_VOID_BLACK,
  .text = COLOR_WHITE,
  .shadow = GOKU_SILVER_GLOW,
  .effect1 = GOKU_DIVINE_SILVER,
  .effect2 = GOKU_KI_BLAST_BLUE,
  .effect3 = GOKU_SPEED_LINES,
  .effect4 = GOKU_LIGHT_SILVER,
  .corner_radius = 10,
  .glow_intensity = 255,
  .animation_speed = 10,
  .border_width = 2
};

// Tanjiro - Sun Breathing
ThemeColors tanjiro_sun_theme = {
  .primary = TANJIRO_FIRE_ORANGE,
  .secondary = TANJIRO_CHECK_GREEN,
  .accent = TANJIRO_HANAFUDA_RED,
  .background = TANJIRO_DARK_CHARCOAL,
  .text = COLOR_WHITE,
  .shadow = TANJIRO_SCAR_BURGUNDY,
  .effect1 = TANJIRO_FLAME_GLOW,
  .effect2 = TANJIRO_WATER_BLUE,
  .effect3 = TANJIRO_SUN_FIRE,
  .effect4 = TANJIRO_SPIRIT_WHITE,
  .corner_radius = 8,
  .glow_intensity = 200,
  .animation_speed = 6,
  .border_width = 2
};

// Gojo - Infinity
ThemeColors gojo_infinity_theme = {
  .primary = GOJO_INFINITY_BLUE,
  .secondary = GOJO_SNOW_WHITE,
  .accent = GOJO_HOLLOW_PURPLE,
  .background = GOJO_VOID_BLACK,
  .text = COLOR_WHITE,
  .shadow = GOJO_DOMAIN_PURPLE,
  .effect1 = GOJO_LIGHT_BLUE_GLOW,
  .effect2 = GOJO_SIX_EYES_BLUE,
  .effect3 = GOJO_DEEP_INFINITY,
  .effect4 = GOJO_CURSED_PURPLE,
  .corner_radius = 10,
  .glow_intensity = 230,
  .animation_speed = 9,
  .border_width = 2
};

// Levi - Humanity's Strongest
ThemeColors levi_strongest_theme = {
  .primary = LEVI_SURVEY_GREEN,
  .secondary = LEVI_DARK_UNIFORM,
  .accent = LEVI_SILVER_BLADE,
  .background = LEVI_CHARCOAL_DARK,
  .text = COLOR_WHITE,
  .shadow = LEVI_MILITARY_GREY,
  .effect1 = LEVI_STEEL_METAL,
  .effect2 = LEVI_CAPE_GREEN,
  .effect3 = LEVI_SPEED_BLUR,
  .effect4 = LEVI_CLEAN_WHITE,
  .corner_radius = 5,
  .glow_intensity = 150,
  .animation_speed = 8,
  .border_width = 2
};

// Saitama - One Punch
ThemeColors saitama_opm_theme = {
  .primary = SAITAMA_HERO_YELLOW,
  .secondary = SAITAMA_CAPE_RED,
  .accent = SAITAMA_GOLDEN_PUNCH,
  .background = SAITAMA_SIMPLE_BLACK,
  .text = COLOR_WHITE,
  .shadow = SAITAMA_DEEP_RED_CAPE,
  .effect1 = SAITAMA_IMPACT_ORANGE,
  .effect2 = SAITAMA_GLOVE_RED,
  .effect3 = SAITAMA_BOOT_YELLOW,
  .effect4 = SAITAMA_BALD_WHITE,
  .corner_radius = 0,
  .glow_intensity = 100,
  .animation_speed = 1,
  .border_width = 3
};

// Deku - Plus Ultra
ThemeColors deku_plusultra_theme = {
  .primary = DEKU_HERO_GREEN,
  .secondary = DEKU_COSTUME_BLACK,
  .accent = DEKU_OFA_LIGHTNING,
  .background = DEKU_DARK_HERO,
  .text = COLOR_WHITE,
  .shadow = DEKU_BLACKWHIP_DARK,
  .effect1 = DEKU_FULL_COWL,
  .effect2 = DEKU_AIR_FORCE,
  .effect3 = DEKU_POWER_VEINS,
  .effect4 = DEKU_ALLMIGHT_GOLD,
  .corner_radius = 10,
  .glow_intensity = 220,
  .animation_speed = 9,
  .border_width = 2
};

ThemeColors* current_theme = &luffy_gear5_theme;

// =============================================================================
// BOBOIBOY ELEMENTAL THEME - Malaysian Anime Hero
// Power Band with 7 Element Transformations
// =============================================================================

ThemeColors boboiboy_elemental_theme = {
  .primary = BBB_BAND_ORANGE,           // Power Band Orange
  .secondary = BBB_LIGHTNING_YELLOW,    // Default Lightning
  .accent = BBB_OCHOBOT_WHITE,          // Ochobot helper
  .background = RGB565(15, 15, 20),     // Dark background
  .text = COLOR_WHITE,
  .shadow = BBB_THUNDERSTORM_BLACK,
  .effect1 = BBB_FIRE_RED,              // Element effect
  .effect2 = BBB_WIND_BLUE,             // Element effect
  .effect3 = BBB_EARTH_BROWN,           // Element effect
  .effect4 = BBB_LIGHT_GOLD,            // Solar effect
  .corner_radius = 12,
  .glow_intensity = 220,
  .animation_speed = 8,
  .border_width = 2
};

// =============================================================================
// CHARACTER PROFILES (11 characters including BoBoiBoy)
// =============================================================================

CharacterProfile character_profiles[11] = {
  // Luffy
  {"Monkey D. Luffy", "Sun God Nika", "One Piece", "Bajrang Gun", 
   "I'm gonna be King of the Pirates!", THEME_LUFFY_GEAR5,
   {"Haki", "All 3 types", 80, "Gear", "5th unlocked", 100, 
    "Freedom", "Unbound", 100, "Dream", "Pirate King", 85}},
  
  // Jin-Woo
  {"Sung Jin-Woo", "Shadow Monarch", "Solo Leveling", "Ruler's Authority", 
   "ARISE!", THEME_SUNG_JINWOO,
   {"Shadows", "10,000+", 85, "Power", "Monarch", 100,
    "Rank", "SSS", 87, "Mana", "Infinite", 70}},
  
  // Yugo
  {"Yugo", "Portal Master", "Wakfu", "Portal Slash", 
   "Adventure awaits!", THEME_YUGO_WAKFU,
   {"Wakfu", "Life force", 75, "Portals", "Dimensional", 80,
    "Speed", "Lightning", 95, "Heart", "Adventurer", 85}},
  
  // Naruto
  {"Naruto Uzumaki", "Sage Mode", "Naruto", "Rasengan", 
   "Believe it! Dattebayo!", THEME_NARUTO_SAGE,
   {"Chakra", "Nine-Tails", 100, "Sage", "Frog Kumite", 80,
    "Kurama", "Best friends", 95, "Bonds", "Never breaks", 100}},
  
  // Goku
  {"Son Goku", "Ultra Instinct", "Dragon Ball", "Kamehameha", 
   "I am the Saiyan who came from Earth!", THEME_GOKU_UI,
   {"Power", "Breaking limits", 100, "Speed", "Body moves", 100,
    "Ki", "Perfect", 95, "Forms", "SSJ to UI", 80}},
  
  // Tanjiro
  {"Tanjiro Kamado", "Sun Breathing", "Demon Slayer", "Hinokami Kagura", 
   "The bond between us can never be severed!", THEME_TANJIRO_SUN,
   {"Breathing", "13 forms", 80, "Forms", "All mastered", 95,
    "Smell", "Track demons", 100, "Will", "Never gives up", 100}},
  
  // Gojo
  {"Satoru Gojo", "Infinity", "Jujutsu Kaisen", "Hollow Purple", 
   "Throughout Heaven and Earth, I alone am the honored one.", THEME_GOJO_INFINITY,
   {"Infinity", "Perfect", 100, "Energy", "Unlimited", 100,
    "Six Eyes", "See all", 100, "Domain", "Unlimited Void", 100}},
  
  // Levi
  {"Levi Ackerman", "Humanity's Strongest", "Attack on Titan", "Spinning Slash", 
   "Give up on your dreams and die.", THEME_LEVI_STRONGEST,
   {"Kills", "58+ Titans", 100, "Speed", "Seconds/Titan", 100,
    "ODM Gear", "Unmatched", 100, "Clean", "OBSESSED", 100}},
  
  // Saitama
  {"Saitama", "One Punch", "One Punch Man", "Serious Punch", 
   "OK.", THEME_SAITAMA_OPM,
   {"Punch", "Infinite", 100, "Rank", "C-Class #7", 7,
    "Boredom", "No opponents", 100, "Bargains", "Expert", 100}},
  
  // Deku
  {"Izuku Midoriya", "Plus Ultra", "My Hero Academia", "United States of Smash", 
   "A hero is someone who saves people!", THEME_DEKU_PLUSULTRA,
   {"OFA", "45% control", 45, "Quirks", "7 of 9", 77,
    "Smash", "Building level", 80, "Spirit", "Saves all", 100}},
  
  // BoBoiBoy - NEW
  {"BoBoiBoy", "Elemental Hero", "BoBoiBoy Galaxy", "Elemental Fusion", 
   "BoBoiBoy Kuasa Tujuh!", THEME_BOBOIBOY,
   {"Elements", "7 Forms", 100, "Fusions", "6 Types", 85,
    "Power Band", "Ochobot", 90, "Teamwork", "TAPOPS", 95}}
};

// =============================================================================
// THEME MANAGEMENT
// =============================================================================

void initializeThemes() {
  Serial.println("[Themes] Initializing 10 anime character themes...");
  setTheme(system_state.current_theme);
}

void setTheme(ThemeType theme) {
  system_state.current_theme = theme;
  current_theme = getThemeColors(theme);
  Serial.printf("[Themes] Active: %s\n", getThemeName(theme));
}

ThemeColors* getCurrentTheme() {
  return current_theme;
}

ThemeColors* getThemeColors(ThemeType theme) {
  switch(theme) {
    case THEME_LUFFY_GEAR5:    return &luffy_gear5_theme;
    case THEME_SUNG_JINWOO:    return &sung_jinwoo_theme;
    case THEME_YUGO_WAKFU:     return &yugo_wakfu_theme;
    case THEME_NARUTO_SAGE:    return &naruto_sage_theme;
    case THEME_GOKU_UI:        return &goku_ui_theme;
    case THEME_TANJIRO_SUN:    return &tanjiro_sun_theme;
    case THEME_GOJO_INFINITY:  return &gojo_infinity_theme;
    case THEME_LEVI_STRONGEST: return &levi_strongest_theme;
    case THEME_SAITAMA_OPM:    return &saitama_opm_theme;
    case THEME_DEKU_PLUSULTRA: return &deku_plusultra_theme;
    case THEME_BOBOIBOY:       return &boboiboy_elemental_theme;
    default: return &luffy_gear5_theme;
  }
}

CharacterProfile* getCharacterProfile(ThemeType theme) {
  if (theme < THEME_COUNT) {
    return &character_profiles[(int)theme];
  }
  return &character_profiles[0];
}

CharacterProfile* getCurrentCharacterProfile() {
  return getCharacterProfile(system_state.current_theme);
}

const char* getThemeName(ThemeType theme) {
  const char* names[] = {
    "Gear 5 Luffy", "Shadow Monarch", "Portal Master", "Sage Mode",
    "Ultra Instinct", "Sun Breathing", "Infinity", "Humanity's Strongest",
    "One Punch", "Plus Ultra", "BoBoiBoy", "Custom"
  };
  if (theme <= THEME_CUSTOM) return names[(int)theme];
  return "Unknown";
}

bool isMainCharacter(ThemeType theme) {
  return theme == THEME_LUFFY_GEAR5 || 
         theme == THEME_SUNG_JINWOO || 
         theme == THEME_YUGO_WAKFU;
}

ThemeType getDailyRotationTheme(int day_of_week) {
  ThemeType rotation[] = {
    THEME_NARUTO_SAGE, THEME_GOKU_UI, THEME_TANJIRO_SUN,
    THEME_GOJO_INFINITY, THEME_LEVI_STRONGEST, 
    THEME_SAITAMA_OPM, THEME_DEKU_PLUSULTRA
  };
  return rotation[day_of_week % 7];
}

void updateDailyCharacter() {
  WatchTime time = getCurrentTime();
  Serial.printf("[Themes] Daily: %s\n", getThemeName(getDailyRotationTheme(time.weekday)));
}

// =============================================================================
// WATCH FACE DISPATCHER
// =============================================================================

void drawWatchFace() {
  switch(system_state.current_theme) {
    case THEME_LUFFY_GEAR5:    drawLuffyWatchFace(); break;
    case THEME_SUNG_JINWOO:    drawJinwooWatchFace(); break;
    case THEME_YUGO_WAKFU:     drawYugoWatchFace(); break;
    case THEME_NARUTO_SAGE:    drawNarutoWatchFace(); break;
    case THEME_GOKU_UI:        drawGokuWatchFace(); break;
    case THEME_TANJIRO_SUN:    drawTanjiroWatchFace(); break;
    case THEME_GOJO_INFINITY:  drawGojoWatchFace(); break;
    case THEME_LEVI_STRONGEST: drawLeviWatchFace(); break;
    case THEME_SAITAMA_OPM:    drawSaitamaWatchFace(); break;
    case THEME_DEKU_PLUSULTRA: drawDekuWatchFace(); break;
    case THEME_BOBOIBOY:       drawBoboiboyWatchFace(); break;
    default: drawLuffyWatchFace(); break;
  }
}

// =============================================================================
// LUFFY GEAR 5 WATCH FACE - MODERN SUN GOD
// =============================================================================

void drawLuffyWatchFace() {
  // Pure AMOLED black
  gfx->fillScreen(0x0000);
  
  WatchTime time = getCurrentTime();
  int centerX = LCD_WIDTH / 2;
  int centerY = 160;
  
  // === AMBIENT SUN GLOW ===
  for (int r = 150; r > 0; r -= 4) {
    uint8_t alpha = map(r, 0, 150, 30, 0);
    gfx->drawCircle(centerX, centerY - 20, r, RGB565(alpha, alpha/3, 0));
  }
  
  // === TIME DISPLAY ===
  char hourStr[3], minStr[3];
  sprintf(hourStr, "%02d", time.hour);
  sprintf(minStr, "%02d", time.minute);
  
  int timeY = 100;
  
  // Glow effect
  gfx->setTextSize(9);
  gfx->setTextColor(RGB565(40, 30, 5));
  gfx->setCursor(18, timeY);
  gfx->print(hourStr);
  gfx->setCursor(195, timeY);
  gfx->print(minStr);
  
  // Main time
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(20, timeY);
  gfx->print(hourStr);
  gfx->setCursor(197, timeY);
  gfx->print(minStr);
  
  // Animated colon
  int colonX = 175;
  int colonY = timeY + 35;
  uint16_t colonColor = (time.second % 2) ? LUFFY_SUN_GOLD : RGB565(200, 150, 50);
  gfx->fillCircle(colonX, colonY - 20, 6, colonColor);
  gfx->fillCircle(colonX, colonY + 20, 6, colonColor);
  
  // Seconds arc
  float secAngle = (time.second / 60.0) * 2 * PI - PI/2;
  for (float a = -PI/2; a < secAngle; a += 0.05) {
    int sx = centerX + cos(a) * 135;
    int sy = centerY - 20 + sin(a) * 60;
    gfx->fillCircle(sx, sy, 2, LUFFY_ENERGY_ORANGE);
  }
  
  // === DATE SECTION ===
  const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                          "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  
  int dateY = 235;
  gfx->fillRoundRect(80, dateY, 210, 30, 15, RGB565(20, 15, 8));
  gfx->drawRoundRect(80, dateY, 210, 30, 15, RGB565(80, 60, 25));
  
  gfx->setTextSize(1);
  gfx->setTextColor(LUFFY_SUN_GOLD);
  gfx->setCursor(95, dateY + 10);
  gfx->print(days[time.weekday % 7]);
  
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(180, dateY + 6);
  gfx->printf("%02d", time.day);
  
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(180, 140, 60));
  gfx->setCursor(220, dateY + 10);
  gfx->print(months[(time.month - 1) % 12]);
  
  // === CHARACTER TITLE ===
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(100, 80, 30));
  gfx->setCursor(centerX - 30, 25);
  gfx->print("GEAR 5");
  
  // === STATS CARDS ===
  drawLuffyStatsCards();
  
  // === ACTIVITY RING ===
  drawLuffyActivityRings(centerX, 390);
}

void drawLuffyStatsCards() {
  int cardY = 290;
  int cardH = 52;
  int cardW = 105;
  int cardGap = 8;
  int cardStartX = (LCD_WIDTH - (3 * cardW + 2 * cardGap)) / 2;
  
  // Steps card
  gfx->fillRoundRect(cardStartX, cardY, cardW, cardH, 12, RGB565(15, 20, 15));
  gfx->drawRoundRect(cardStartX, cardY, cardW, cardH, 12, RGB565(50, 80, 50));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(100, 160, 100));
  gfx->setCursor(cardStartX + 10, cardY + 8);
  gfx->print("STEPS");
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(150, 230, 150));
  gfx->setCursor(cardStartX + 10, cardY + 25);
  gfx->printf("%d", system_state.steps_today);
  
  // Battery card
  int card2X = cardStartX + cardW + cardGap;
  int battPct = system_state.battery_percentage;
  uint16_t battColor = battPct > 20 ? RGB565(100, 180, 255) : RGB565(255, 100, 100);
  gfx->fillRoundRect(card2X, cardY, cardW, cardH, 12, RGB565(15, 18, 22));
  gfx->drawRoundRect(card2X, cardY, cardW, cardH, 12, RGB565(50, 70, 90));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(80, 120, 160));
  gfx->setCursor(card2X + 10, cardY + 8);
  gfx->print("BATTERY");
  gfx->setTextSize(2);
  gfx->setTextColor(battColor);
  gfx->setCursor(card2X + 10, cardY + 25);
  gfx->printf("%d%%", battPct);
  
  // Gems card
  int card3X = card2X + cardW + cardGap;
  gfx->fillRoundRect(card3X, cardY, cardW, cardH, 12, RGB565(22, 18, 10));
  gfx->drawRoundRect(card3X, cardY, cardW, cardH, 12, RGB565(80, 65, 30));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(140, 110, 50));
  gfx->setCursor(card3X + 10, cardY + 8);
  gfx->print("GEMS");
  gfx->setTextSize(2);
  gfx->setTextColor(LUFFY_SUN_GOLD);
  gfx->setCursor(card3X + 10, cardY + 25);
  gfx->printf("%d", system_state.player_gems);
}

// =============================================================================
// JINWOO SHADOW MONARCH WATCH FACE - DARK POWER
// =============================================================================

void drawJinwooWatchFace() {
  gfx->fillScreen(0x0000);
  
  WatchTime time = getCurrentTime();
  int centerX = LCD_WIDTH / 2;
  int centerY = 160;
  
  // === SHADOW PARTICLES ===
  static uint8_t particleY[15];
  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < 15; i++) particleY[i] = random(0, LCD_HEIGHT);
    initialized = true;
  }
  
  for (int i = 0; i < 15; i++) {
    particleY[i] = (particleY[i] - 1 + LCD_HEIGHT) % LCD_HEIGHT;
    int px = 25 + (i * 23) % (LCD_WIDTH - 50);
    int size = 1 + (i % 3);
    uint8_t bright = 15 + (particleY[i] * 25) / LCD_HEIGHT;
    gfx->fillCircle(px, particleY[i], size, RGB565(bright, bright/3, bright + 15));
  }
  
  // === MONARCH HEXAGON ===
  for (int ring = 0; ring < 3; ring++) {
    int r = 100 + ring * 20;
    for (int i = 0; i < 6; i++) {
      float a1 = (i * 60) * PI / 180.0;
      float a2 = ((i + 1) * 60) * PI / 180.0;
      int x1 = centerX + cos(a1) * r;
      int y1 = centerY - 20 + sin(a1) * (r * 0.4);
      int x2 = centerX + cos(a2) * r;
      int y2 = centerY - 20 + sin(a2) * (r * 0.4);
      gfx->drawLine(x1, y1, x2, y2, RGB565(25 - ring*7, 12 - ring*3, 45 - ring*10));
    }
  }
  
  // === TIME ===
  char hourStr[3], minStr[3];
  sprintf(hourStr, "%02d", time.hour);
  sprintf(minStr, "%02d", time.minute);
  
  int timeY = 100;
  
  // Purple glow
  gfx->setTextSize(9);
  gfx->setTextColor(RGB565(30, 15, 50));
  gfx->setCursor(18, timeY);
  gfx->print(hourStr);
  gfx->setCursor(195, timeY);
  gfx->print(minStr);
  
  // Main time
  gfx->setTextColor(RGB565(230, 220, 255));
  gfx->setCursor(20, timeY);
  gfx->print(hourStr);
  gfx->setCursor(197, timeY);
  gfx->print(minStr);
  
  // Pulsing colon
  int colonX = 175;
  int colonY = timeY + 35;
  float pulse = 0.5 + 0.5 * sin(millis() / 300.0);
  uint8_t pulseVal = 100 + pulse * 155;
  gfx->fillCircle(colonX, colonY - 20, 6, RGB565(pulseVal/2, pulseVal/3, pulseVal));
  gfx->fillCircle(colonX, colonY + 20, 6, RGB565(pulseVal/2, pulseVal/3, pulseVal));
  
  // === ARISE BADGE ===
  int ariseY = 240;
  for (int i = 2; i >= 0; i--) {
    gfx->fillRoundRect(centerX - 55 - i*2, ariseY - i, 110 + i*4, 30 + i, 8,
                       RGB565(20 + i*8, 10 + i*4, 40 + i*12));
  }
  gfx->fillRoundRect(centerX - 55, ariseY, 110, 28, 8, RGB565(15, 8, 30));
  gfx->drawRoundRect(centerX - 55, ariseY, 110, 28, 8, JINWOO_ARISE_GLOW);
  
  gfx->setTextSize(2);
  gfx->setTextColor(JINWOO_ARISE_GLOW);
  gfx->setCursor(centerX - 35, ariseY + 5);
  gfx->print("ARISE!");
  
  // === DATE ===
  const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(100, 80, 140));
  char dateStr[15];
  sprintf(dateStr, "%s %02d.%02d", days[time.weekday % 7], time.day, time.month);
  gfx->setCursor(centerX - 35, 280);
  gfx->print(dateStr);
  
  // === SHADOW STATS ===
  int statsY = 305;
  
  // Shadows
  gfx->fillRoundRect(20, statsY, 100, 48, 10, RGB565(12, 8, 20));
  gfx->drawRoundRect(20, statsY, 100, 48, 10, RGB565(50, 35, 80));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(80, 60, 120));
  gfx->setCursor(30, statsY + 8);
  gfx->print("SHADOWS");
  gfx->setTextSize(2);
  gfx->setTextColor(JINWOO_MONARCH_PURPLE);
  gfx->setCursor(30, statsY + 24);
  gfx->printf("%d", system_state.gacha_cards_collected);
  
  // Level
  gfx->fillRoundRect(130, statsY, 110, 48, 10, RGB565(12, 8, 20));
  gfx->drawRoundRect(130, statsY, 110, 48, 10, RGB565(50, 35, 80));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(80, 60, 120));
  gfx->setCursor(155, statsY + 8);
  gfx->print("LEVEL");
  gfx->setTextSize(3);
  gfx->setTextColor(JINWOO_ARISE_GLOW);
  gfx->setCursor(160, statsY + 20);
  gfx->printf("%d", system_state.player_level);
  
  // Mana
  gfx->fillRoundRect(250, statsY, 100, 48, 10, RGB565(12, 8, 20));
  gfx->drawRoundRect(250, statsY, 100, 48, 10, RGB565(50, 35, 80));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(80, 60, 120));
  gfx->setCursor(265, statsY + 8);
  gfx->print("MANA");
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(150, 120, 220));
  gfx->setCursor(260, statsY + 24);
  gfx->printf("%d", system_state.player_gems);
  
  // Activity rings
  drawJinwooActivityRings(centerX, 395);
}

// =============================================================================
// REMAINING CHARACTER WATCH FACES (Simplified for space)
// =============================================================================

void drawYugoWatchFace() {
  gfx->fillScreen(YUGO_SKY_BLUE_GREY);
  drawYugoPortals();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(YUGO_PORTAL_CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(80, 30);
  gfx->print("PORTAL MASTER");
  
  gfx->setTextSize(5);
  gfx->setTextColor(YUGO_WAKFU_ENERGY);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextSize(2);
  gfx->setTextColor(YUGO_HAT_GOLD);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  drawYugoActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawNarutoWatchFace() {
  gfx->fillScreen(NARUTO_SLATE_GREY);
  drawNarutoSageAura();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(NARUTO_CHAKRA_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(110, 30);
  gfx->print("SAGE MODE");
  
  gfx->setTextSize(5);
  gfx->setTextColor(NARUTO_SAGE_GOLD);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextSize(2);
  gfx->setTextColor(NARUTO_KURAMA_FLAME);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  gfx->setTextColor(NARUTO_CHAKRA_ORANGE);
  gfx->setTextSize(1);
  gfx->setCursor(120, 260);
  gfx->print("Believe it!");
  
  drawNarutoActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawGokuWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  drawGokuUIAura();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(GOKU_UI_SILVER);
  gfx->setTextSize(2);
  gfx->setCursor(70, 30);
  gfx->print("ULTRA INSTINCT");
  
  gfx->setTextSize(5);
  gfx->setTextColor(GOKU_DIVINE_SILVER);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  drawGokuSpeedLines();
  drawGokuActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawTanjiroWatchFace() {
  gfx->fillScreen(TANJIRO_DARK_CHARCOAL);
  drawTanjiroSunFlames();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(TANJIRO_FIRE_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(80, 30);
  gfx->print("SUN BREATHING");
  
  gfx->setTextSize(5);
  gfx->setTextColor(TANJIRO_FLAME_GLOW);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextColor(TANJIRO_WATER_BLUE);
  gfx->setTextSize(1);
  gfx->setCursor(100, 260);
  gfx->print("Hinokami Kagura");
  
  drawTanjiroActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawGojoWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  drawGojoInfinityAura();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(GOJO_INFINITY_BLUE);
  gfx->setTextSize(2);
  gfx->setCursor(120, 30);
  gfx->print("INFINITY");
  
  gfx->setTextSize(5);
  gfx->setTextColor(GOJO_SIX_EYES_BLUE);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  drawGojoSixEyesGlow();
  
  gfx->setTextColor(GOJO_LIGHT_BLUE_GLOW);
  gfx->setTextSize(1);
  gfx->setCursor(60, 260);
  gfx->print("I alone am the honored one");
  
  drawGojoActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawLeviWatchFace() {
  gfx->fillScreen(LEVI_CHARCOAL_DARK);
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(LEVI_SURVEY_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(45, 30);
  gfx->print("HUMANITY'S STRONGEST");
  
  gfx->setTextSize(5);
  gfx->setTextColor(LEVI_CLEAN_WHITE);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextSize(2);
  gfx->setTextColor(LEVI_SILVER_BLADE);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  drawLeviBladeShine();
  drawLeviActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawSaitamaWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(SAITAMA_HERO_YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(110, 30);
  gfx->print("ONE PUNCH");
  
  gfx->setTextSize(5);
  gfx->setTextColor(SAITAMA_BALD_WHITE);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextColor(SAITAMA_CAPE_RED);
  gfx->setTextSize(3);
  gfx->setCursor(160, 260);
  gfx->print("OK.");
  
  drawSaitamaActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawDekuWatchFace() {
  gfx->fillScreen(DEKU_DARK_HERO);
  drawDekuOFALightning();
  
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  gfx->setTextColor(DEKU_HERO_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(110, 30);
  gfx->print("PLUS ULTRA");
  
  gfx->setTextSize(5);
  gfx->setTextColor(DEKU_FULL_COWL);
  gfx->setCursor(80, 170);
  gfx->print(timeStr);
  
  gfx->setTextColor(DEKU_ALLMIGHT_GOLD);
  gfx->setTextSize(1);
  gfx->setCursor(120, 260);
  gfx->print("Full Cowl: 45%");
  
  drawDekuActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawSleepWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(4);
  gfx->setTextColor(RGB565(50, 50, 50));
  gfx->setCursor(100, 200);
  gfx->print(timeStr);
}

// =============================================================================
// BOBOIBOY WATCH FACE - ELEMENTAL HERO
// 7 Elements with Power Band UI
// =============================================================================

// Track current element for cycling animation
static int boboiboy_current_element = 0;
static unsigned long boboiboy_last_cycle = 0;

void drawBoboiboyWatchFace() {
  // Use dynamic background based on time
  drawDynamicBackground(THEME_BOBOIBOY);
  
  WatchTime time = getCurrentTime();
  int centerX = LCD_WIDTH / 2;
  int centerY = 160;
  
  // Element colors array
  uint16_t elementColors[] = {
    BBB_LIGHTNING_YELLOW, BBB_WIND_BLUE, BBB_EARTH_BROWN,
    BBB_FIRE_RED, BBB_WATER_CYAN, BBB_LEAF_GREEN, BBB_LIGHT_GOLD
  };
  const char* elementNames[] = {
    "HALILINTAR", "TAUFAN", "GEMPA", "BLAZE", "ICE", "THORN", "SOLAR"
  };
  
  // Use manually selected element (tap-to-switch) instead of auto-cycle
  int currentElement = getCurrentBoboiboyElement();
  uint16_t currentColor = elementColors[currentElement];
  
  // === POWER BAND RING ===
  // Outer ring with element color
  for (int r = 130; r > 115; r -= 3) {
    gfx->drawCircle(centerX, centerY - 10, r, currentColor);
  }
  
  // Inner glow effect
  for (int r = 110; r > 95; r -= 5) {
    uint8_t alpha = map(r, 95, 110, 10, 40);
    gfx->drawCircle(centerX, centerY - 10, r, RGB565(alpha, alpha, alpha + 10));
  }
  
  // === ELEMENT INDICATOR ===
  // Draw 7 element dots around the ring
  for (int i = 0; i < 7; i++) {
    float angle = (i * 51.4 - 90) * PI / 180.0;  // 360/7 = ~51.4 degrees
    int dotX = centerX + cos(angle) * 125;
    int dotY = centerY - 10 + sin(angle) * 50;
    
    if (i == currentElement) {
      // Active element - filled and larger
      gfx->fillCircle(dotX, dotY, 8, elementColors[i]);
      gfx->drawCircle(dotX, dotY, 10, COLOR_WHITE);
    } else {
      // Inactive - smaller outline
      gfx->drawCircle(dotX, dotY, 5, elementColors[i]);
    }
  }
  
  // === TIME DISPLAY ===
  char hourStr[3], minStr[3];
  sprintf(hourStr, "%02d", time.hour);
  sprintf(minStr, "%02d", time.minute);
  
  int timeY = 95;
  
  // Glow effect with current element color
  gfx->setTextSize(8);
  gfx->setTextColor(RGB565(30, 25, 20));
  gfx->setCursor(28, timeY);
  gfx->print(hourStr);
  gfx->setCursor(190, timeY);
  gfx->print(minStr);
  
  // Main time
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(30, timeY);
  gfx->print(hourStr);
  gfx->setCursor(192, timeY);
  gfx->print(minStr);
  
  // Animated colon with element color
  int colonX = 170;
  int colonY = timeY + 30;
  float pulse = 0.5 + 0.5 * sin(millis() / 250.0);
  uint8_t colAlpha = 150 + pulse * 105;
  gfx->fillCircle(colonX, colonY - 15, 6, currentColor);
  gfx->fillCircle(colonX, colonY + 15, 6, currentColor);
  
  // Seconds arc
  float secAngle = (time.second / 60.0) * 2 * PI - PI/2;
  for (float a = -PI/2; a < secAngle; a += 0.05) {
    int sx = centerX + cos(a) * 108;
    int sy = centerY - 10 + sin(a) * 45;
    gfx->fillCircle(sx, sy, 2, currentColor);
  }
  
  // === ELEMENT NAME BADGE ===
  int badgeY = 235;
  gfx->fillRoundRect(centerX - 70, badgeY, 140, 28, 14, RGB565(20, 22, 28));
  gfx->drawRoundRect(centerX - 70, badgeY, 140, 28, 14, currentColor);
  
  gfx->setTextColor(currentColor);
  gfx->setTextSize(2);
  int nameLen = strlen(elementNames[currentElement]) * 12;
  gfx->setCursor(centerX - nameLen/2, badgeY + 5);
  gfx->print(elementNames[currentElement]);
  
  // === DATE SECTION ===
  const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  int dateY = 275;
  
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  char dateStr[20];
  sprintf(dateStr, "%s  %02d.%02d.%04d", days[time.weekday % 7], time.day, time.month, time.year);
  gfx->setCursor(centerX - 55, dateY);
  gfx->print(dateStr);
  
  // === STATS CARDS ===
  int cardY = 300;
  int cardH = 50;
  int cardW = 105;
  int cardGap = 8;
  int cardStartX = (LCD_WIDTH - (3 * cardW + 2 * cardGap)) / 2;
  
  // Power card
  gfx->fillRoundRect(cardStartX, cardY, cardW, cardH, 10, RGB565(20, 15, 10));
  gfx->drawRoundRect(cardStartX, cardY, cardW, cardH, 10, BBB_BAND_ORANGE);
  gfx->setTextSize(1);
  gfx->setTextColor(BBB_BAND_ORANGE);
  gfx->setCursor(cardStartX + 10, cardY + 8);
  gfx->print("POWER");
  gfx->setTextSize(2);
  gfx->setTextColor(BBB_BAND_GLOW);
  gfx->setCursor(cardStartX + 10, cardY + 25);
  gfx->printf("%d%%", 100);
  
  // Steps card
  int card2X = cardStartX + cardW + cardGap;
  gfx->fillRoundRect(card2X, cardY, cardW, cardH, 10, RGB565(15, 20, 15));
  gfx->drawRoundRect(card2X, cardY, cardW, cardH, 10, BBB_LEAF_GREEN);
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(100, 160, 100));
  gfx->setCursor(card2X + 10, cardY + 8);
  gfx->print("STEPS");
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(150, 230, 150));
  gfx->setCursor(card2X + 10, cardY + 25);
  gfx->printf("%d", system_state.steps_today);
  
  // Battery card
  int card3X = card2X + cardW + cardGap;
  int battPct = system_state.battery_percentage;
  uint16_t battColor = battPct > 20 ? BBB_WIND_BLUE : BBB_FIRE_RED;
  gfx->fillRoundRect(card3X, cardY, cardW, cardH, 10, RGB565(15, 18, 22));
  gfx->drawRoundRect(card3X, cardY, cardW, cardH, 10, battColor);
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(80, 120, 160));
  gfx->setCursor(card3X + 10, cardY + 8);
  gfx->print("BATTERY");
  gfx->setTextSize(2);
  gfx->setTextColor(battColor);
  gfx->setCursor(card3X + 10, cardY + 25);
  gfx->printf("%d%%", battPct);
  
  // === ACTIVITY RINGS ===
  drawBoboiboyActivityRings(centerX, 395);
  
  // === OCHOBOT ASSISTANT ===
  // Draw Ochobot in corner
  if (ochobot_pos.visible) {
    drawOchobot(LCD_WIDTH - 45, 70, OCHOBOT_IDLE, 0.8);
  }
  
  // === TAP HINT ===
  gfx->setTextColor(RGB565(80, 85, 95));
  gfx->setTextSize(1);
  gfx->setCursor(centerX - 65, 425);
  gfx->print("Tap center to switch!");
}

// =============================================================================
// CHARACTER EFFECTS
// =============================================================================

void drawYugoPortals() {
  gfx->drawCircle(60, 100, 30, YUGO_PORTAL_CYAN);
  gfx->drawCircle(60, 100, 25, YUGO_WAKFU_ENERGY);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 30, YUGO_PORTAL_CYAN);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 25, YUGO_WAKFU_ENERGY);
}

void drawNarutoSageAura() {
  int cx = LCD_WIDTH / 2;
  int cy = LCD_HEIGHT / 2 - 50;
  for (int r = 60; r < 80; r += 5) {
    gfx->drawCircle(cx, cy, r, NARUTO_CHAKRA_ORANGE);
  }
}

void drawGokuUIAura() {
  int cx = LCD_WIDTH / 2;
  int cy = 130;
  for (int r = 50; r < 70; r += 3) {
    gfx->drawCircle(cx, cy, r, GOKU_SILVER_GLOW);
  }
}

void drawGokuSpeedLines() {
  for (int i = 0; i < 10; i++) {
    int x = random(0, LCD_WIDTH);
    int y1 = random(50, 150);
    gfx->drawLine(x, y1, x + random(-5, 5), y1 + random(20, 40), GOKU_SPEED_LINES);
  }
}

void drawTanjiroSunFlames() {
  for (int i = 0; i < 7; i++) {
    int x = 50 + i * 45;
    int y = 80;
    gfx->fillTriangle(x, y, x - 10, y + 25, x + 10, y + 25, TANJIRO_FLAME_GLOW);
  }
}

void drawGojoInfinityAura() {
  int cx = LCD_WIDTH / 2;
  int cy = 130;
  for (int r = 40; r < 65; r += 4) {
    gfx->drawCircle(cx, cy, r, GOJO_INFINITY_BLUE);
  }
}

void drawGojoSixEyesGlow() {
  gfx->fillCircle(LCD_WIDTH / 2 - 20, 55, 8, GOJO_SIX_EYES_BLUE);
  gfx->fillCircle(LCD_WIDTH / 2 + 20, 55, 8, GOJO_SIX_EYES_BLUE);
}

void drawLeviBladeShine() {
  gfx->drawLine(30, 80, 50, 140, LEVI_SILVER_BLADE);
  gfx->drawLine(LCD_WIDTH - 30, 80, LCD_WIDTH - 50, 140, LEVI_SILVER_BLADE);
}

void drawDekuOFALightning() {
  for (int i = 0; i < 5; i++) {
    int x1 = random(30, LCD_WIDTH - 30);
    int y1 = random(50, 100);
    gfx->drawLine(x1, y1, x1 + random(-20, 20), y1 + random(20, 40), DEKU_OFA_LIGHTNING);
  }
}

// =============================================================================
// ACTIVITY RINGS
// =============================================================================

void drawActivityRing(int centerX, int centerY, int radius, float progress, uint16_t color, int thickness) {
  progress = constrain(progress, 0.0f, 1.0f);
  
  // Background ring
  for (int i = 0; i < 360; i += 3) {
    float a = i * PI / 180.0 - PI/2;
    int px = centerX + cos(a) * radius;
    int py = centerY + sin(a) * radius;
    gfx->drawPixel(px, py, RGB565(40, 40, 45));
  }
  
  // Progress arc
  int progressDeg = progress * 360;
  for (int i = 0; i < progressDeg; i += 3) {
    float a = i * PI / 180.0 - PI/2;
    int px = centerX + cos(a) * radius;
    int py = centerY + sin(a) * radius;
    gfx->fillCircle(px, py, thickness/2, color);
  }
}

void drawLuffyActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, LUFFY_SUN_GOLD, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, LUFFY_ENERGY_ORANGE, 5);
  
  // Center percentage
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 150));
  int pct = min(100, (int)(stepProgress * 100));
  gfx->setCursor(centerX - 10, centerY - 4);
  gfx->printf("%d%%", pct);
}

void drawJinwooActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, JINWOO_MONARCH_PURPLE, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, JINWOO_ARISE_GLOW, 5);
}

void drawYugoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, YUGO_PORTAL_CYAN, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, YUGO_HAT_GOLD, 5);
}

void drawNarutoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, NARUTO_CHAKRA_ORANGE, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, NARUTO_SAGE_GOLD, 5);
}

void drawGokuActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, GOKU_UI_SILVER, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, GOKU_KI_BLAST_BLUE, 5);
}

void drawTanjiroActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, TANJIRO_FIRE_ORANGE, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, TANJIRO_WATER_BLUE, 5);
}

void drawGojoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, GOJO_INFINITY_BLUE, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, GOJO_HOLLOW_PURPLE, 5);
}

void drawLeviActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, LEVI_SURVEY_GREEN, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, LEVI_SILVER_BLADE, 5);
}

void drawSaitamaActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, SAITAMA_HERO_YELLOW, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, SAITAMA_CAPE_RED, 5);
}

void drawDekuActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  drawActivityRing(centerX, centerY, 35, stepProgress, DEKU_HERO_GREEN, 6);
  drawActivityRing(centerX, centerY, 25, 0.7, DEKU_OFA_LIGHTNING, 5);
}

void drawBoboiboyActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / max(1, system_state.step_goal);
  
  // Outer ring - Power Band Orange
  drawActivityRing(centerX, centerY, 35, stepProgress, BBB_BAND_ORANGE, 6);
  
  // Inner ring - Cycles through element colors
  uint16_t elementColors[] = {
    BBB_LIGHTNING_YELLOW, BBB_WIND_BLUE, BBB_EARTH_BROWN,
    BBB_FIRE_RED, BBB_WATER_CYAN, BBB_LEAF_GREEN, BBB_LIGHT_GOLD
  };
  int elemIdx = (millis() / 1500) % 7;
  drawActivityRing(centerX, centerY, 25, 0.8, elementColors[elemIdx], 5);
  
  // Center - element icon (small colored dot)
  gfx->fillCircle(centerX, centerY, 5, elementColors[elemIdx]);
}

// =============================================================================
// UI UTILITIES
// =============================================================================

void drawStepCounter() {
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(10, 10);
  gfx->printf("%d steps", system_state.steps_today);
}

void drawBatteryIndicator() {
  int x = LCD_WIDTH - 50, y = 10;
  gfx->drawRect(x, y, 40, 18, COLOR_WHITE);
  gfx->fillRect(x + 40, y + 4, 4, 10, COLOR_WHITE);
  int fillWidth = (system_state.battery_percentage * 36) / 100;
  uint16_t color = system_state.battery_percentage > 20 ? COLOR_GREEN : COLOR_RED;
  if (system_state.is_charging) color = COLOR_YELLOW;
  gfx->fillRect(x + 2, y + 2, fillWidth, 14, color);
}

void drawThemeButton(int x, int y, int w, int h, const char* text, bool pressed) {
  uint8_t radius = current_theme->corner_radius;
  uint16_t bg = pressed ? current_theme->accent : current_theme->primary;
  gfx->fillRoundRect(x, y, w, h, radius, bg);
  gfx->drawRoundRect(x, y, w, h, radius, current_theme->text);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  int textW = strlen(text) * 12;
  gfx->setCursor(x + (w - textW)/2, y + (h - 16)/2);
  gfx->print(text);
}

void drawGameButton(int x, int y, int w, int h, const char* text, bool pressed) {
  drawThemeButton(x, y, w, h, text, pressed);
}

void drawGlassPanel(int x, int y, int w, int h, uint8_t opacity) {
  uint16_t glassBg = RGB565(30 + opacity, 30 + opacity, 40 + opacity);
  gfx->fillRoundRect(x, y, w, h, current_theme->corner_radius, glassBg);
  gfx->drawRoundRect(x, y, w, h, current_theme->corner_radius, RGB565(80, 80, 90));
}

void drawGlassButton(int x, int y, int w, int h, const char* text, bool pressed) {
  drawGlassPanel(x, y, w, h, pressed ? 50 : 25);
  if (pressed) {
    gfx->drawRoundRect(x - 2, y - 2, w + 4, h + 4, current_theme->corner_radius + 2, current_theme->primary);
  }
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  int textW = strlen(text) * 12;
  gfx->setCursor(x + (w - textW)/2, y + (h - 16)/2);
  gfx->print(text);
}

void drawGlassStatBar(int x, int y, int w, int h, float progress, uint16_t color, const char* label) {
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(40, 40, 50));
  int fillW = (int)(w * constrain(progress, 0.0f, 1.0f));
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, color);
  }
  gfx->drawRoundRect(x, y, w, h, h/2, RGB565(80, 80, 90));
  if (label) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x + 5, y + h/2 - 4);
    gfx->print(label);
  }
}

// =============================================================================
// CHARACTER STATS SCREEN
// =============================================================================

void drawCharacterStatsScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  CharacterProfile* profile = getCurrentCharacterProfile();
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(60, 20);
  gfx->print("CHARACTER STATS");
  
  // Glass card for character info
  drawGlassPanel(50, 55, 270, 95);
  
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(65, 70);
  gfx->print(profile->name);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(65, 95);
  gfx->print(profile->title);
  
  gfx->setTextColor(COLOR_GRAY);
  gfx->setCursor(65, 115);
  gfx->print(profile->series);
  
  gfx->setTextColor(theme->accent);
  gfx->setTextSize(2);
  gfx->setCursor(65, 135);
  gfx->printf("Lv.%d", system_state.player_level);
  
  // Stat bars
  int barY = 165;
  int barW = 270;
  int barH = 22;
  int spacing = 40;
  
  drawGlassStatBar(50, barY, barW, barH, profile->stats.stat1_value / 100.0f, theme->primary, profile->stats.stat1_name);
  drawGlassStatBar(50, barY + spacing, barW, barH, profile->stats.stat2_value / 100.0f, theme->accent, profile->stats.stat2_name);
  drawGlassStatBar(50, barY + spacing * 2, barW, barH, profile->stats.stat3_value / 100.0f, theme->effect1, profile->stats.stat3_name);
  drawGlassStatBar(50, barY + spacing * 3, barW, barH, profile->stats.stat4_value / 100.0f, theme->effect2, profile->stats.stat4_name);
  
  // Catchphrase
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(1);
  gfx->setCursor(50, barY + spacing * 4 + 15);
  gfx->print(profile->catchphrase);
  
  // Back button
  drawGlassButton(140, 410, 80, 35, "Back", false);
}

void drawThemeSelector() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 15);
  gfx->print("SELECT THEME");
  
  // Page indicator
  static int themePage = 0;
  int totalPages = 2;
  
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(LCD_WIDTH - 35, 20);
  gfx->printf("%d/%d", themePage + 1, totalPages);
  
  if (themePage == 0) {
    // Page 1: First 6 themes
    const char* names[] = {"Luffy", "Jin-Woo", "Yugo", "Naruto", "Goku", "Tanjiro"};
    ThemeColors* themes[] = {&luffy_gear5_theme, &sung_jinwoo_theme, &yugo_wakfu_theme,
                             &naruto_sage_theme, &goku_ui_theme, &tanjiro_sun_theme};
    
    for (int i = 0; i < 6; i++) {
      int x = (i % 2) * 175 + 15;
      int y = (i / 2) * 100 + 45;
      
      gfx->fillRoundRect(x, y, 155, 90, 15, themes[i]->primary);
      gfx->drawRoundRect(x, y, 155, 90, 15, RGB565(80, 80, 90));
      
      if ((ThemeType)i == system_state.current_theme) {
        gfx->drawRoundRect(x - 2, y - 2, 159, 94, 17, COLOR_WHITE);
        gfx->drawRoundRect(x - 3, y - 3, 161, 96, 18, getCurrentTheme()->accent);
      }
      
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(2);
      gfx->setCursor(x + 30, y + 32);
      gfx->print(names[i]);
    }
  } else {
    // Page 2: Last 5 themes (Gojo, Levi, Saitama, Deku, BoBoiBoy)
    const char* names[] = {"Gojo", "Levi", "Saitama", "Deku", "BoBoiBoy"};
    ThemeColors* themes[] = {&gojo_infinity_theme, &levi_strongest_theme, 
                             &saitama_opm_theme, &deku_plusultra_theme, &boboiboy_elemental_theme};
    ThemeType types[] = {THEME_GOJO_INFINITY, THEME_LEVI_STRONGEST, 
                         THEME_SAITAMA_OPM, THEME_DEKU_PLUSULTRA, THEME_BOBOIBOY};
    
    for (int i = 0; i < 5; i++) {
      int col = i % 2;
      int row = i / 2;
      int x = col * 175 + 15;
      int y = row * 100 + 45;
      
      // Special layout for 5th item (BoBoiBoy) - centered
      if (i == 4) {
        x = (LCD_WIDTH - 155) / 2;
        y = 2 * 100 + 45;
      }
      
      gfx->fillRoundRect(x, y, 155, 90, 15, themes[i]->primary);
      gfx->drawRoundRect(x, y, 155, 90, 15, RGB565(80, 80, 90));
      
      if (types[i] == system_state.current_theme) {
        gfx->drawRoundRect(x - 2, y - 2, 159, 94, 17, COLOR_WHITE);
        gfx->drawRoundRect(x - 3, y - 3, 161, 96, 18, getCurrentTheme()->accent);
      }
      
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(2);
      int textLen = strlen(names[i]) * 12;
      gfx->setCursor(x + (155 - textLen) / 2, y + 32);
      gfx->print(names[i]);
    }
  }
  
  // Navigation hint
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(100, 105, 120));
  gfx->setCursor(90, 365);
  gfx->print("Swipe up/down for more");
  
  drawGlassButton(140, 400, 80, 30, "Back", false);
}

// Theme selector page state
static int themePageState = 0;

void handleThemeSelectorTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_SWIPE_UP) {
    if (themePageState < 1) themePageState++;
    drawThemeSelector();
    return;
  }
  if (gesture.event == TOUCH_SWIPE_DOWN) {
    if (themePageState > 0) themePageState--;
    drawThemeSelector();
    return;
  }
  
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Back button
  if (y >= 400 && x >= 140 && x < 220) {
    themePageState = 0;
    system_state.current_screen = SCREEN_SETTINGS;
    return;
  }
  
  // Theme selection
  if (themePageState == 0) {
    // Page 1: themes 0-5
    for (int i = 0; i < 6; i++) {
      int tx = (i % 2) * 175 + 15;
      int ty = (i / 2) * 100 + 45;
      if (x >= tx && x < tx + 155 && y >= ty && y < ty + 90) {
        setTheme((ThemeType)i);
        drawThemeSelector();
        return;
      }
    }
  } else {
    // Page 2: themes 6-10
    ThemeType types[] = {THEME_GOJO_INFINITY, THEME_LEVI_STRONGEST, 
                         THEME_SAITAMA_OPM, THEME_DEKU_PLUSULTRA, THEME_BOBOIBOY};
    for (int i = 0; i < 5; i++) {
      int col = i % 2;
      int row = i / 2;
      int tx = col * 175 + 15;
      int ty = row * 100 + 45;
      
      if (i == 4) {
        tx = (LCD_WIDTH - 155) / 2;
        ty = 2 * 100 + 45;
      }
      
      if (x >= tx && x < tx + 155 && y >= ty && y < ty + 90) {
        setTheme(types[i]);
        drawThemeSelector();
        return;
      }
    }
  }
}
