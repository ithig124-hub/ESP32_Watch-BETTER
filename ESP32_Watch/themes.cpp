/*
 * themes.cpp - Complete Theme Implementation
 * All 10 Premium Anime Characters with full visual themes
 * Glass morphism UI, character effects, activity rings
 */

#include "themes.h"
#include "config.h"
#include "display.h"
#include "hardware.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// =============================================================================
// THEME COLOR DEFINITIONS - ALL 10 CHARACTERS
// =============================================================================

// Luffy Gear 5 - Sun God Nika (Very rounded, bouncy, strong sun aura)
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
  .corner_radius = 20,      // Very rounded, cloud-like
  .glow_intensity = 200,    // Strong sun aura
  .animation_speed = 8,     // Fast, energetic
  .border_width = 3         // Bold presence
};

// Jin-Woo - Shadow Monarch (Sharp, angular, MAXIMUM power)
ThemeColors sung_jinwoo_theme = {
  .primary = JINWOO_MONARCH_PURPLE,
  .secondary = JINWOO_VOID_BLACK,
  .accent = JINWOO_POWER_VIOLET,
  .background = JINWOO_ABSOLUTE_DARK,
  .text = COLOR_WHITE,
  .shadow = JINWOO_DEATH_BLACK,
  .effect1 = JINWOO_ARISE_GLOW,
  .effect2 = JINWOO_DAGGER_BLUE,
  .effect3 = JINWOO_MIST_PURPLE,
  .effect4 = JINWOO_SOUL_PURPLE,
  .corner_radius = 5,       // Sharp, angular, precise
  .glow_intensity = 255,    // MAXIMUM power
  .animation_speed = 10,    // Instant, dominant
  .border_width = 2         // Thin, sharp lines
};

// Yugo - Portal Master (Rounded portal-shaped, soft mystical glow)
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
  .corner_radius = 15,      // Rounded, portal-shaped
  .glow_intensity = 150,    // Soft mystical glow
  .animation_speed = 7,     // Smooth, flowing
  .border_width = 3         // Defined portals
};

// Naruto - Sage Mode (Bold, energetic)
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

// Goku - Ultra Instinct (Divine silver, maximum speed)
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

// Tanjiro - Sun Breathing (Warm flames, moderate speed)
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

// Gojo - Infinity (Intense blue, fast animations)
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

// Levi - Humanity's Strongest (Sharp, clean, fast)
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

// Saitama - One Punch (Minimalist, bold, minimal animations)
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
  .corner_radius = 0,       // Minimalist, bold
  .glow_intensity = 100,
  .animation_speed = 1,     // Minimal (he's too powerful to care)
  .border_width = 3
};

// Deku - Plus Ultra (Energetic green, fast lightning)
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
// CHARACTER PROFILES
// =============================================================================

CharacterProfile character_profiles[10] = {
  // Luffy
  {"Monkey D. Luffy", "Sun God Nika", "One Piece", "Bajrang Gun", "I'm gonna be King of the Pirates!", THEME_LUFFY_GEAR5,
   {"Haki Mastery", "Three types mastered", 80, "Gear Progression", "Gear 5 unlocked", 100, "Freedom Level", "Unbound by limits", 100, "Will to Dream", "Pirate King ambition", 85}},
  // Jin-Woo
  {"Sung Jin-Woo", "Shadow Monarch", "Solo Leveling", "Ruler's Authority", "ARISE!", THEME_SUNG_JINWOO,
   {"Shadow Army", "10,000+ soldiers", 85, "Monarch Power", "True awakened", 100, "Hunter Level", "SSS-Rank", 87, "Mana Capacity", "Infinite growth", 70}},
  // Yugo
  {"Yugo", "Portal Master", "Wakfu", "Portal Slash", "Adventure awaits!", THEME_YUGO_WAKFU,
   {"Wakfu Mastery", "Life force energy", 75, "Portal Expertise", "Dimensional gates", 80, "Combat Speed", "Lightning reflexes", 95, "Heart of Adventure", "Never-ending quest", 85}},
  // Naruto
  {"Naruto Uzumaki", "Sage Mode", "Naruto", "Rasengan", "Believe it! Dattebayo!", THEME_NARUTO_SAGE,
   {"Chakra Reserves", "Nine-Tails infinite", 100, "Sage Mode", "Frog Kumite", 80, "Kurama Bond", "Best friends", 95, "Bonds Strength", "Never abandons", 100}},
  // Goku
  {"Son Goku", "Ultra Instinct", "Dragon Ball", "Kamehameha", "I am the Saiyan who came from Earth!", THEME_GOKU_UI,
   {"Power Level", "Breaking limits", 100, "Combat Speed", "Body moves alone", 100, "Ki Control", "Perfect energy", 95, "Transformations", "SSJ to UI", 80}},
  // Tanjiro
  {"Tanjiro Kamado", "Sun Breathing", "Demon Slayer", "Hinokami Kagura", "The bond between us can never be severed!", THEME_TANJIRO_SUN,
   {"Breathing Forms", "13 Sun forms", 80, "Form Count", "All mastered", 95, "Enhanced Smell", "Track demons", 100, "Determination", "Never gives up", 100}},
  // Gojo
  {"Satoru Gojo", "Infinity", "Jujutsu Kaisen", "Hollow Purple", "Throughout Heaven and Earth, I alone am the honored one.", THEME_GOJO_INFINITY,
   {"Infinity", "Perfect Limitless", 100, "Cursed Energy", "Unlimited", 100, "Six Eyes", "See everything", 100, "Domain Expansion", "Unlimited Void", 100}},
  // Levi
  {"Levi Ackerman", "Humanity's Strongest", "Attack on Titan", "Spinning Slash", "Give up on your dreams and die.", THEME_LEVI_STRONGEST,
   {"Titan Kills", "58+ confirmed", 100, "Kill Speed", "Seconds per titan", 100, "ODM Gear", "Unmatched", 100, "Cleanliness", "OBSESSED", 100}},
  // Saitama
  {"Saitama", "One Punch", "One Punch Man", "Serious Punch", "OK.", THEME_SAITAMA_OPM,
   {"Punch Power", "Infinite", 100, "Hero Rank", "C-Class Rank 7", 7, "Boredom Level", "No opponents", 100, "Bargain Hunting", "Expert", 100}},
  // Deku
  {"Izuku Midoriya", "Plus Ultra", "My Hero Academia", "United States of Smash", "A hero is someone who saves people!", THEME_DEKU_PLUSULTRA,
   {"One For All", "45% control", 45, "Quirks Unlocked", "7 of 9", 77, "Smash Power", "Building level", 80, "Hero Spirit", "Saves everyone", 100}}
};

// =============================================================================
// THEME MANAGEMENT
// =============================================================================

void initializeThemes() {
  Serial.println("[Themes] Initializing complete theme system (10 characters)...");
  setTheme(system_state.current_theme);
}

void setTheme(ThemeType theme) {
  system_state.current_theme = theme;
  current_theme = getThemeColors(theme);
  Serial.printf("[Themes] Theme set to: %s\n", getThemeName(theme));
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
    "One Punch", "Plus Ultra", "Custom"
  };
  if (theme <= THEME_CUSTOM) return names[(int)theme];
  return "Unknown";
}

// Main characters always available: Luffy, Jin-Woo, Yugo
bool isMainCharacter(ThemeType theme) {
  return theme == THEME_LUFFY_GEAR5 || theme == THEME_SUNG_JINWOO || theme == THEME_YUGO_WAKFU;
}

// Daily rotation for the other 7 characters
ThemeType getDailyRotationTheme(int day_of_week) {
  // Rotate through: Naruto, Goku, Tanjiro, Gojo, Levi, Saitama, Deku
  ThemeType rotation[] = {THEME_NARUTO_SAGE, THEME_GOKU_UI, THEME_TANJIRO_SUN,
                          THEME_GOJO_INFINITY, THEME_LEVI_STRONGEST, THEME_SAITAMA_OPM, THEME_DEKU_PLUSULTRA};
  return rotation[day_of_week % 7];
}

void updateDailyCharacter() {
  WatchTime time = getCurrentTime();
  // The daily rotation theme changes based on day of week
  Serial.printf("[Themes] Daily character: %s\n", getThemeName(getDailyRotationTheme(time.weekday)));
}

// =============================================================================
// WATCH FACES - ALL 10 CHARACTERS
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
    default: drawLuffyWatchFace(); break;
  }
}

void drawLuffyWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Character signature rings
  drawCharacterRings(LCD_WIDTH/2, LCD_HEIGHT/2 - 50);
  
  // Title with glow effect
  gfx->setTextColor(LUFFY_SUN_GOLD);
  gfx->setTextSize(2);
  gfx->setCursor(85, 30);
  gfx->print("SUN GOD NIKA");
  
  // Time with theme glow
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(LUFFY_SUN_GOLD);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Date
  gfx->setTextSize(1);
  gfx->setTextColor(LUFFY_CLOUD_WHITE);
  gfx->setCursor(120, 235);
  gfx->printf("%02d/%02d/%04d", time.day, time.month, time.year);
  
  // Signature move
  gfx->setTextColor(LUFFY_ENERGY_ORANGE);
  gfx->setCursor(120, 260);
  gfx->print("Bajrang Gun");
  
  // Activity rings
  drawLuffyActivityRings(LCD_WIDTH/2, 340);
  
  // Effects
  drawLuffyGear5Effects();
  
  // Status bar
  drawBatteryIndicator();
  drawStepCounter();
}

void drawJinwooWatchFace() {
  gfx->fillScreen(JINWOO_ABSOLUTE_DARK);
  
  // Shadow particles rising
  drawJinwooShadows();
  
  // Title
  gfx->setTextColor(JINWOO_MONARCH_PURPLE);
  gfx->setTextSize(2);
  gfx->setCursor(60, 30);
  gfx->print("SHADOW MONARCH");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(JINWOO_POWER_VIOLET);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(JINWOO_ARISE_GLOW);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // ARISE text effect
  gfx->setTextColor(JINWOO_ARISE_GLOW);
  gfx->setTextSize(2);
  gfx->setCursor(145, 260);
  gfx->print("ARISE!");
  
  drawJinwooActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawYugoWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Portal effects
  drawYugoPortals();
  
  // Title
  gfx->setTextColor(YUGO_PORTAL_CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(70, 30);
  gfx->print("PORTAL MASTER");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(YUGO_WAKFU_ENERGY);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(YUGO_HAT_GOLD);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Catchphrase
  gfx->setTextColor(YUGO_PORTAL_GLOW);
  gfx->setTextSize(1);
  gfx->setCursor(100, 260);
  gfx->print("Adventure awaits!");
  
  drawYugoActivityRings(LCD_WIDTH/2, 340);
  drawYugoWakfuParticles();
  drawBatteryIndicator();
  drawStepCounter();
}

void drawNarutoWatchFace() {
  gfx->fillScreen(NARUTO_SLATE_GREY);
  
  // Sage mode aura
  drawNarutoSageAura();
  
  // Title
  gfx->setTextColor(NARUTO_CHAKRA_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("SAGE MODE");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(NARUTO_SAGE_GOLD);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(NARUTO_KURAMA_FLAME);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Catchphrase
  gfx->setTextColor(NARUTO_CHAKRA_ORANGE);
  gfx->setTextSize(1);
  gfx->setCursor(110, 260);
  gfx->print("Believe it!");
  
  drawNarutoActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawGokuWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // UI aura
  drawGokuUIAura();
  
  // Title
  gfx->setTextColor(GOKU_UI_SILVER);
  gfx->setTextSize(2);
  gfx->setCursor(60, 30);
  gfx->print("ULTRA INSTINCT");
  
  // Time with silver glow
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(GOKU_DIVINE_SILVER);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(GOKU_AURA_WHITE);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Speed lines
  drawGokuSpeedLines();
  
  drawGokuActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawTanjiroWatchFace() {
  gfx->fillScreen(TANJIRO_DARK_CHARCOAL);
  
  // Sun breathing flames
  drawTanjiroSunFlames();
  
  // Title
  gfx->setTextColor(TANJIRO_FIRE_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 30);
  gfx->print("SUN BREATHING");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(TANJIRO_FLAME_GLOW);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(TANJIRO_SUN_FIRE);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Nichirin blade effect
  gfx->setTextColor(TANJIRO_WATER_BLUE);
  gfx->setTextSize(1);
  gfx->setCursor(95, 260);
  gfx->print("Hinokami Kagura");
  
  drawTanjiroActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawGojoWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Infinity aura
  drawGojoInfinityAura();
  
  // Title
  gfx->setTextColor(GOJO_INFINITY_BLUE);
  gfx->setTextSize(2);
  gfx->setCursor(110, 30);
  gfx->print("INFINITY");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(GOJO_SIX_EYES_BLUE);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Six Eyes glow
  drawGojoSixEyesGlow();
  
  // Catchphrase
  gfx->setTextColor(GOJO_LIGHT_BLUE_GLOW);
  gfx->setTextSize(1);
  gfx->setCursor(50, 260);
  gfx->print("I alone am the honored one");
  
  drawGojoActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawLeviWatchFace() {
  gfx->fillScreen(LEVI_CHARCOAL_DARK);
  
  // Clean minimal design
  
  // Title
  gfx->setTextColor(LEVI_SURVEY_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(35, 30);
  gfx->print("HUMANITY'S STRONGEST");
  
  // Time - clean and precise
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(LEVI_CLEAN_WHITE);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(LEVI_SILVER_BLADE);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Blade effects
  drawLeviBladeShine();
  
  drawLeviActivityRings(LCD_WIDTH/2, 340);
  drawBatteryIndicator();
  drawStepCounter();
}

void drawSaitamaWatchFace() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Minimalist design - Saitama doesn't need fancy effects
  
  // Title
  gfx->setTextColor(SAITAMA_HERO_YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("ONE PUNCH");
  
  // Time - BOLD
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(SAITAMA_BALD_WHITE);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Simple "OK."
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
  
  // OFA Lightning
  drawDekuOFALightning();
  
  // Title
  gfx->setTextColor(DEKU_HERO_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("PLUS ULTRA");
  
  // Time
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  gfx->setTextSize(5);
  gfx->setTextColor(DEKU_FULL_COWL);
  gfx->setCursor(70, 170);
  gfx->print(timeStr);
  
  // Seconds
  gfx->setTextSize(2);
  gfx->setTextColor(DEKU_OFA_LIGHTNING);
  gfx->setCursor(260, 195);
  gfx->printf("%02d", time.second);
  
  // Full Cowl indicator
  gfx->setTextColor(DEKU_ALLMIGHT_GOLD);
  gfx->setTextSize(1);
  gfx->setCursor(115, 260);
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

// Step counter display
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

void drawCustomWallpaperWatchFace() {
  drawMinimalTimeOverlay();
}

void drawWallpaperOverlayElements() {
  drawMinimalTimeOverlay();
  drawBatteryIndicator();
}

void drawMinimalTimeOverlay() {
  WatchTime time = getCurrentTime();
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d", time.hour, time.minute);
  
  // Semi-transparent background
  gfx->fillRoundRect(60, 180, 250, 80, 15, RGB565(0, 0, 0));
  
  gfx->setTextSize(4);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(90, 200);
  gfx->print(timeStr);
}

// =============================================================================
// CHARACTER EFFECTS
// =============================================================================

void drawCharacterRings(int centerX, int centerY) {
  // Draw 12-16 glowing rings around the character area
  uint16_t ringColor = current_theme->effect1;
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30) * PI / 180;
    int rx = centerX + cos(angle) * 80;
    int ry = centerY + sin(angle) * 80;
    int size = random(8, 16);
    gfx->fillCircle(rx, ry, size, ringColor);
  }
}

void drawPulsingGlow(int x, int y, int radius, uint16_t color) {
  static uint8_t pulse = 0;
  pulse = (pulse + 5) % 255;
  uint8_t alpha = 128 + (sin(pulse * PI / 128) * 64);
  gfx->drawCircle(x, y, radius, color);
  gfx->drawCircle(x, y, radius - 2, color);
}

void drawLuffyGear5Effects() {
  // Golden sun particles
  for (int i = 0; i < 8; i++) {
    int x = random(0, LCD_WIDTH);
    int y = random(50, 150);
    gfx->fillCircle(x, y, random(2, 5), LUFFY_SUN_GLOW);
  }
}

void drawLuffySunRays() {
  // Radial sun rays from center
  int cx = LCD_WIDTH / 2;
  int cy = 100;
  for (int i = 0; i < 8; i++) {
    float angle = i * 45 * PI / 180;
    int x2 = cx + cos(angle) * 60;
    int y2 = cy + sin(angle) * 60;
    gfx->drawLine(cx, cy, x2, y2, LUFFY_SUN_GOLD);
  }
}

void drawLuffyCloudWisps() {
  for (int i = 0; i < 5; i++) {
    int x = random(20, LCD_WIDTH - 20);
    int y = random(80, 130);
    gfx->fillEllipse(x, y, random(10, 20), random(5, 10), LUFFY_CLOUD_WHITE);
  }
}

void drawLuffyRubberStretch() {
  // Stretchy effect lines
}

void drawJinwooShadows() {
  // Shadow soldiers rising
  for (int i = 0; i < 5; i++) {
    int x = 40 + i * 70;
    int y = LCD_HEIGHT - 80;
    gfx->fillEllipse(x, y, 15, 25, JINWOO_VOID_BLACK);
    gfx->fillCircle(x, y - 20, 8, JINWOO_MONARCH_PURPLE);
    // Glowing eyes
    gfx->fillCircle(x - 3, y - 22, 2, JINWOO_ARISE_GLOW);
    gfx->fillCircle(x + 3, y - 22, 2, JINWOO_ARISE_GLOW);
  }
}

void drawJinwooAriseEffect() {
  gfx->setTextColor(JINWOO_ARISE_GLOW);
  gfx->setTextSize(2);
  gfx->setCursor(140, LCD_HEIGHT - 100);
  gfx->print("ARISE!");
}

void drawJinwooVoidRifts() {
  // Dimensional tears
  for (int i = 0; i < 3; i++) {
    int x = random(50, LCD_WIDTH - 50);
    int y = random(60, 140);
    gfx->drawLine(x, y, x + random(-10, 10), y + random(10, 30), JINWOO_MIST_PURPLE);
  }
}

void drawJinwooWeaponTrails() {
  // Dagger slash effects
}

void drawYugoPortals() {
  // Left portal
  gfx->drawCircle(60, 100, 30, YUGO_PORTAL_CYAN);
  gfx->drawCircle(60, 100, 25, YUGO_WAKFU_ENERGY);
  gfx->drawCircle(60, 100, 20, YUGO_PORTAL_GLOW);
  
  // Right portal
  gfx->drawCircle(LCD_WIDTH - 60, 100, 30, YUGO_PORTAL_CYAN);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 25, YUGO_WAKFU_ENERGY);
  gfx->drawCircle(LCD_WIDTH - 60, 100, 20, YUGO_PORTAL_GLOW);
}

void drawYugoWakfuParticles() {
  for (int i = 0; i < 6; i++) {
    int x = random(30, LCD_WIDTH - 30);
    int y = random(50, 150);
    gfx->fillCircle(x, y, 3, YUGO_ENERGY_BURST);
  }
}

void drawYugoDimensionalRifts() {
  // Space tears effect
}

void drawYugoEnergyOrbs() {
  // Floating wakfu orbs
}

void drawNarutoSageAura() {
  // Orange chakra aura
  int cx = LCD_WIDTH / 2;
  int cy = LCD_HEIGHT / 2 - 50;
  for (int r = 60; r < 80; r += 5) {
    gfx->drawCircle(cx, cy, r, NARUTO_CHAKRA_ORANGE);
  }
}

void drawNarutoKuramaFlames() {
  // Nine-tails flame effect
  for (int i = 0; i < 9; i++) {
    int x = 30 + i * 35;
    int y = LCD_HEIGHT - 50;
    gfx->fillTriangle(x, y, x - 8, y + 20, x + 8, y + 20, NARUTO_KURAMA_FLAME);
  }
}

void drawNarutoRasengan() {
  // Spinning rasengan effect
  gfx->fillCircle(LCD_WIDTH / 2, LCD_HEIGHT - 100, 20, NARUTO_RASENGAN_BLUE);
  gfx->drawCircle(LCD_WIDTH / 2, LCD_HEIGHT - 100, 25, COLOR_WHITE);
}

void drawNarutoShadowClones() {
  // Multiple faint clone silhouettes
}

void drawGokuUIAura() {
  // Silver UI aura
  int cx = LCD_WIDTH / 2;
  int cy = 130;
  for (int r = 50; r < 70; r += 3) {
    gfx->drawCircle(cx, cy, r, GOKU_SILVER_GLOW);
  }
}

void drawGokuKiBlasts() {
  // Ki blast particles
}

void drawGokuSpeedLines() {
  // Motion blur lines
  for (int i = 0; i < 10; i++) {
    int x = random(0, LCD_WIDTH);
    int y1 = random(50, 150);
    int y2 = y1 + random(20, 40);
    gfx->drawLine(x, y1, x + random(-5, 5), y2, GOKU_SPEED_LINES);
  }
}

void drawGokuDivineGlow() {
  // Divine silver glow effect
}

void drawTanjiroSunFlames() {
  // Sun breathing flames
  for (int i = 0; i < 7; i++) {
    int x = 50 + i * 45;
    int y = 80;
    gfx->fillTriangle(x, y, x - 10, y + 25, x + 10, y + 25, TANJIRO_FLAME_GLOW);
  }
}

void drawTanjiroWaterTechniques() {
  // Water effects (alternate breathing)
}

void drawTanjiroHinokamiKagura() {
  // Golden sun dance
}

void drawTanjiroNichirinBlade() {
  // Blade shine effect
  gfx->drawLine(LCD_WIDTH - 30, 60, LCD_WIDTH - 50, 140, TANJIRO_STEEL_GREY);
  gfx->drawLine(LCD_WIDTH - 29, 60, LCD_WIDTH - 49, 140, TANJIRO_SPIRIT_WHITE);
}

void drawGojoInfinityAura() {
  // Blue infinity aura
  int cx = LCD_WIDTH / 2;
  int cy = 130;
  for (int r = 40; r < 65; r += 4) {
    gfx->drawCircle(cx, cy, r, GOJO_INFINITY_BLUE);
  }
}

void drawGojoSixEyesGlow() {
  // Six eyes blue glow at top
  gfx->fillCircle(LCD_WIDTH / 2 - 20, 55, 8, GOJO_SIX_EYES_BLUE);
  gfx->fillCircle(LCD_WIDTH / 2 + 20, 55, 8, GOJO_SIX_EYES_BLUE);
}

void drawGojoHollowPurple() {
  // Purple/red convergence effect
}

void drawGojoDomainExpansion() {
  // Unlimited void visual
}

void drawLeviBladeShine() {
  // Dual blade reflections
  gfx->drawLine(30, 80, 50, 140, LEVI_SILVER_BLADE);
  gfx->drawLine(LCD_WIDTH - 30, 80, LCD_WIDTH - 50, 140, LEVI_SILVER_BLADE);
}

void drawLeviODMGearMotion() {
  // Wire/gear motion trails
}

void drawLeviSpeedBlur() {
  // Fast movement lines
}

void drawLeviCleanAesthetic() {
  // Everything is spotless
}

void drawSaitamaPunchImpact() {
  // Punch shockwave
  gfx->drawCircle(LCD_WIDTH / 2, 120, 40, SAITAMA_GOLDEN_PUNCH);
  gfx->drawCircle(LCD_WIDTH / 2, 120, 50, SAITAMA_IMPACT_ORANGE);
}

void drawSaitamaCapeFlow() {
  // Flowing red cape
}

void drawSaitamaDotEyes() {
  // Simple dot eyes
  gfx->fillCircle(LCD_WIDTH / 2 - 15, 100, 3, SAITAMA_DOT_EYES);
  gfx->fillCircle(LCD_WIDTH / 2 + 15, 100, 3, SAITAMA_DOT_EYES);
}

void drawSaitamaPowerEffect() {
  // Overwhelming casual power
}

void drawDekuOFALightning() {
  // Green lightning crackles
  for (int i = 0; i < 5; i++) {
    int x1 = random(30, LCD_WIDTH - 30);
    int y1 = random(50, 100);
    int x2 = x1 + random(-20, 20);
    int y2 = y1 + random(20, 40);
    gfx->drawLine(x1, y1, x2, y2, DEKU_OFA_LIGHTNING);
  }
}

void drawDekuFullCowlAura() {
  // Green energy aura
  int cx = LCD_WIDTH / 2;
  int cy = 130;
  for (int r = 45; r < 60; r += 5) {
    gfx->drawCircle(cx, cy, r, DEKU_FULL_COWL);
  }
}

void drawDekuAirForce() {
  // Air pressure lines
}

void drawDekuPowerStrainMarks() {
  // Red strain veins
}

// =============================================================================
// ACTIVITY RINGS
// =============================================================================

void drawLuffyActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, LUFFY_SUN_GOLD, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, LUFFY_ENERGY_ORANGE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, LUFFY_STRAW_RED, 5);
}

void drawJinwooActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, JINWOO_MONARCH_PURPLE, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, JINWOO_POWER_VIOLET, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, JINWOO_ARISE_GLOW, 5);
}

void drawYugoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, YUGO_PORTAL_CYAN, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, YUGO_HAT_GOLD, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, YUGO_WAKFU_ENERGY, 5);
}

void drawNarutoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, NARUTO_CHAKRA_ORANGE, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, NARUTO_KURAMA_RED, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, NARUTO_SAGE_GOLD, 5);
}

void drawGokuActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, GOKU_UI_SILVER, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, GOKU_KI_BLAST_BLUE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, GOKU_AURA_WHITE, 5);
}

void drawTanjiroActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, TANJIRO_FIRE_ORANGE, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, TANJIRO_WATER_BLUE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, TANJIRO_FLAME_GLOW, 5);
}

void drawGojoActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, GOJO_INFINITY_BLUE, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, GOJO_HOLLOW_PURPLE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, GOJO_SIX_EYES_BLUE, 5);
}

void drawLeviActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, LEVI_SURVEY_GREEN, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, LEVI_SILVER_BLADE, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, LEVI_MILITARY_GREY, 5);
}

void drawSaitamaActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, SAITAMA_HERO_YELLOW, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, SAITAMA_CAPE_RED, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, SAITAMA_GOLDEN_PUNCH, 5);
}

void drawDekuActivityRings(int centerX, int centerY) {
  float stepProgress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX, centerY, 50, stepProgress, DEKU_HERO_GREEN, 8);
  drawActivityRing(centerX, centerY, 38, 0.7, DEKU_OFA_LIGHTNING, 6);
  drawActivityRing(centerX, centerY, 28, 0.5, DEKU_ALLMIGHT_GOLD, 5);
}

// =============================================================================
// ANIMATIONS
// =============================================================================

void playLuffyAnimation() {
  for (int i = 0; i < 5; i++) {
    gfx->fillCircle(LCD_WIDTH/2, LCD_HEIGHT/2, 50 + i*20, LUFFY_SUN_GOLD);
    delay(50);
  }
}

void playJinwooAnimation() {
  for (int i = 0; i < 10; i++) {
    int x = random(50, LCD_WIDTH - 50);
    int y = random(100, LCD_HEIGHT - 100);
    gfx->fillCircle(x, y, 20, JINWOO_MONARCH_PURPLE);
    delay(30);
  }
}

void playYugoAnimation() {
  for (int r = 10; r < 80; r += 5) {
    gfx->drawCircle(LCD_WIDTH/2, LCD_HEIGHT/2, r, YUGO_PORTAL_CYAN);
    delay(30);
  }
}

void playNarutoAnimation() {
  for (int i = 0; i < 5; i++) {
    gfx->fillCircle(LCD_WIDTH/2, LCD_HEIGHT/2, 30, NARUTO_RASENGAN_BLUE);
    delay(100);
    gfx->fillCircle(LCD_WIDTH/2, LCD_HEIGHT/2, 30, COLOR_BLACK);
    delay(50);
  }
}

void playGokuAnimation() {
  // UI transformation flash
  gfx->fillScreen(GOKU_AURA_WHITE);
  delay(100);
  gfx->fillScreen(COLOR_BLACK);
}

void playTanjiroAnimation() {
  // Sun breathing flame sweep
  for (int x = 0; x < LCD_WIDTH; x += 30) {
    gfx->fillTriangle(x, 100, x - 15, 150, x + 15, 150, TANJIRO_FLAME_GLOW);
    delay(50);
  }
}

void playGojoAnimation() {
  // Infinity sphere expansion
  for (int r = 10; r < 100; r += 10) {
    gfx->drawCircle(LCD_WIDTH/2, LCD_HEIGHT/2, r, GOJO_INFINITY_BLUE);
    delay(40);
  }
}

void playLeviAnimation() {
  // Fast blade slash
  gfx->drawLine(0, LCD_HEIGHT/2, LCD_WIDTH, LCD_HEIGHT/2, LEVI_SILVER_BLADE);
  delay(50);
}

void playSaitamaAnimation() {
  // One punch - simple
  gfx->fillCircle(LCD_WIDTH/2, LCD_HEIGHT/2, 80, SAITAMA_GOLDEN_PUNCH);
  delay(100);
}

void playDekuAnimation() {
  // Full Cowl lightning
  for (int i = 0; i < 10; i++) {
    int x = random(0, LCD_WIDTH);
    int y = random(0, LCD_HEIGHT);
    gfx->drawLine(x, y, x + random(-30, 30), y + random(-30, 30), DEKU_OFA_LIGHTNING);
  }
}

// =============================================================================
// UI ELEMENTS - GLASS MORPHISM
// =============================================================================

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

void drawThemeProgressBar(int x, int y, int w, int h, float progress, const char* label) {
  progress = constrain(progress, 0.0f, 1.0f);
  
  // Background with glass effect
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(40, 40, 40));
  
  // Progress fill with theme gradient
  int fillW = (int)(w * progress);
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, current_theme->primary);
  }
  
  // Glow effect
  if (current_theme->glow_intensity > 100) {
    gfx->drawRoundRect(x - 1, y - 1, w + 2, h + 2, h/2, current_theme->effect1);
  }
  
  // Label
  if (label && strlen(label) > 0) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x, y - 14);
    gfx->print(label);
  }
}

void drawThemeNotification(const char* title, const char* message) {
  int w = 300, h = 100;
  int x = (LCD_WIDTH - w) / 2;
  int y = 50;
  
  drawGlassPanel(x, y, w, h);
  
  gfx->setTextColor(current_theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(x + 15, y + 15);
  gfx->print(title);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 15, y + 50);
  gfx->print(message);
}

// Glass Morphism Components
void drawGlassPanel(int x, int y, int w, int h, uint8_t opacity) {
  // Background with transparency effect (simulated)
  uint16_t glassBg = RGB565(30 + opacity, 30 + opacity, 40 + opacity);
  gfx->fillRoundRect(x, y, w, h, current_theme->corner_radius, glassBg);
  
  // Border with transparency
  gfx->drawRoundRect(x, y, w, h, current_theme->corner_radius, 
                     RGB565(255, 255, 255));
  
  // Inner glow
  gfx->drawRoundRect(x + 1, y + 1, w - 2, h - 2, current_theme->corner_radius - 1,
                     RGB565(100, 100, 120));
}

void drawGlassButton(int x, int y, int w, int h, const char* text, bool pressed) {
  drawGlassPanel(x, y, w, h, pressed ? 50 : 25);
  
  // Themed glow on hover
  if (pressed) {
    gfx->drawRoundRect(x - 2, y - 2, w + 4, h + 4, current_theme->corner_radius + 2,
                       current_theme->primary);
  }
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  int textW = strlen(text) * 12;
  gfx->setCursor(x + (w - textW)/2, y + (h - 16)/2);
  gfx->print(text);
}

void drawGlassCard(int x, int y, int w, int h) {
  drawGlassPanel(x, y, w, h, 20);
}

void drawGlassStatBar(int x, int y, int w, int h, float progress, uint16_t color, const char* label) {
  // Glass background
  gfx->fillRoundRect(x, y, w, h, h/2, RGB565(40, 40, 50));
  
  // Progress fill
  int fillW = (int)(w * constrain(progress, 0.0f, 1.0f));
  if (fillW > 0) {
    gfx->fillRoundRect(x, y, fillW, h, h/2, color);
  }
  
  // Glow effect
  gfx->drawRoundRect(x, y, w, h, h/2, RGB565(100, 100, 120));
  
  // Label
  if (label) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x + 5, y + h/2 - 4);
    gfx->print(label);
  }
  
  // Percentage
  char pct[8];
  sprintf(pct, "%d%%", (int)(progress * 100));
  gfx->setCursor(x + w - 35, y + h/2 - 4);
  gfx->print(pct);
}

// =============================================================================
// CHARACTER STATS SCREEN
// =============================================================================

void drawCharacterStatsScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  CharacterProfile* profile = getCharacterProfile(system_state.current_theme);
  
  // Header
  gfx->setTextColor(current_theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(50, 20);
  gfx->print("CHARACTER STATS");
  
  // Glass portrait container
  drawGlassPanel(60, 60, 240, 100);
  
  // Character name and title
  gfx->setTextColor(current_theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(75, 75);
  gfx->print(profile->name);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(75, 100);
  gfx->print(profile->title);
  
  gfx->setTextColor(COLOR_GRAY);
  gfx->setCursor(75, 120);
  gfx->print(profile->series);
  
  // Level display with stars
  gfx->setTextColor(current_theme->accent);
  gfx->setTextSize(2);
  gfx->setCursor(75, 140);
  gfx->printf("Lv.%d", system_state.player_level);
  
  // Stat bars
  int barY = 180;
  int barW = 280;
  int barH = 24;
  int spacing = 45;
  
  drawGlassStatBar(40, barY, barW, barH, profile->stats.stat1_value / 100.0f, 
                   current_theme->primary, profile->stats.stat1_name);
  
  drawGlassStatBar(40, barY + spacing, barW, barH, profile->stats.stat2_value / 100.0f,
                   current_theme->accent, profile->stats.stat2_name);
  
  drawGlassStatBar(40, barY + spacing * 2, barW, barH, profile->stats.stat3_value / 100.0f,
                   current_theme->effect1, profile->stats.stat3_name);
  
  drawGlassStatBar(40, barY + spacing * 3, barW, barH, profile->stats.stat4_value / 100.0f,
                   current_theme->effect2, profile->stats.stat4_name);
  
  // Catchphrase
  gfx->setTextColor(current_theme->primary);
  gfx->setTextSize(1);
  int catchphraseY = barY + spacing * 4 + 20;
  gfx->setCursor(40, catchphraseY);
  gfx->print(profile->catchphrase);
  
  // Back button
  drawGlassButton(140, 410, 80, 35, "Back", false);
}

void drawStatBar(int x, int y, int w, int h, int value, int maxValue, uint16_t color, const char* label) {
  float progress = (float)value / maxValue;
  drawGlassStatBar(x, y, w, h, progress, color, label);
}

void drawAnimatedStatBar(int x, int y, int w, int h, float targetProgress, uint16_t color) {
  // Animation happens over time - this is a placeholder for animated fill
  static float currentProgress = 0;
  if (currentProgress < targetProgress) {
    currentProgress += 0.02;
  }
  drawGlassStatBar(x, y, w, h, currentProgress, color, nullptr);
}

// =============================================================================
// THEME SELECTOR
// =============================================================================

void drawThemeSelector() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("SELECT THEME");
  
  // Draw theme cards in a scrollable list
  const char* names[] = {"Luffy", "Jin-Woo", "Yugo", "Naruto", "Goku",
                         "Tanjiro", "Gojo", "Levi", "Saitama", "Deku"};
  ThemeColors* themes[] = {&luffy_gear5_theme, &sung_jinwoo_theme, &yugo_wakfu_theme,
                           &naruto_sage_theme, &goku_ui_theme, &tanjiro_sun_theme,
                           &gojo_infinity_theme, &levi_strongest_theme, &saitama_opm_theme,
                           &deku_plusultra_theme};
  
  for (int i = 0; i < 6; i++) {  // Show 6 at a time
    int x = (i % 2) * 175 + 15;
    int y = (i / 2) * 110 + 60;
    
    gfx->fillRoundRect(x, y, 160, 100, 15, themes[i]->primary);
    gfx->drawRoundRect(x, y, 160, 100, 15, COLOR_WHITE);
    
    // Highlight current theme
    if ((ThemeType)i == system_state.current_theme) {
      gfx->drawRoundRect(x - 2, y - 2, 164, 104, 17, COLOR_WHITE);
      gfx->drawRoundRect(x - 3, y - 3, 166, 106, 18, COLOR_WHITE);
    }
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(x + 20, y + 40);
    gfx->print(names[i]);
  }
  
  // Navigation hint
  gfx->setTextSize(1);
  gfx->setCursor(100, 400);
  gfx->print("Swipe for more themes");
  
  drawGlassButton(140, 420, 80, 30, "Back", false);
}

void handleThemeSelectorTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check theme cards
  for (int i = 0; i < 6; i++) {
    int tx = (i % 2) * 175 + 15;
    int ty = (i / 2) * 110 + 60;
    
    if (x >= tx && x < tx + 160 && y >= ty && y < ty + 100) {
      setTheme((ThemeType)i);
      return;
    }
  }
  
  // Back button
  if (y >= 420 && x >= 140 && x < 220) {
    system_state.current_screen = SCREEN_SETTINGS;
  }
}
