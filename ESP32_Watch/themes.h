/*
 * themes.h - Complete Theme System
 * All 10 Premium Anime Characters with full effects
 * Daily rotating characters, glass morphism UI
 */

#ifndef THEMES_H
#define THEMES_H

#include "config.h"

// Theme color schemes for all 10 characters
extern ThemeColors luffy_gear5_theme;
extern ThemeColors sung_jinwoo_theme;
extern ThemeColors yugo_wakfu_theme;
extern ThemeColors naruto_sage_theme;
extern ThemeColors goku_ui_theme;
extern ThemeColors tanjiro_sun_theme;
extern ThemeColors gojo_infinity_theme;
extern ThemeColors levi_strongest_theme;
extern ThemeColors saitama_opm_theme;
extern ThemeColors deku_plusultra_theme;
extern ThemeColors* current_theme;

// Character profiles for all 10 characters
extern CharacterProfile character_profiles[10];

// =============================================================================
// THEME MANAGEMENT
// =============================================================================

void initializeThemes();
void setTheme(ThemeType theme);
ThemeColors* getCurrentTheme();
ThemeColors* getThemeColors(ThemeType theme);
CharacterProfile* getCharacterProfile(ThemeType theme);
const char* getThemeName(ThemeType theme);

// Daily rotation
void updateDailyCharacter();
ThemeType getDailyRotationTheme(int day_of_week);
bool isMainCharacter(ThemeType theme);  // Luffy, Jin-Woo, Yugo always available

// =============================================================================
// WATCH FACES - ALL 10 CHARACTERS
// =============================================================================

void drawWatchFace();
void drawLuffyWatchFace();
void drawJinwooWatchFace();
void drawYugoWatchFace();
void drawNarutoWatchFace();
void drawGokuWatchFace();
void drawTanjiroWatchFace();
void drawGojoWatchFace();
void drawLeviWatchFace();
void drawSaitamaWatchFace();
void drawDekuWatchFace();
void drawSleepWatchFace();

// Custom wallpaper
void drawCustomWallpaperWatchFace();
void drawWallpaperOverlayElements();
void drawMinimalTimeOverlay();

// =============================================================================
// CHARACTER EFFECTS - ALL 10 CHARACTERS
// =============================================================================

// Luffy Gear 5
void drawLuffyGear5Effects();
void drawLuffySunRays();
void drawLuffyCloudWisps();
void drawLuffyRubberStretch();

// Jin-Woo Shadow Monarch
void drawJinwooShadows();
void drawJinwooAriseEffect();
void drawJinwooVoidRifts();
void drawJinwooWeaponTrails();

// Yugo Portal Master
void drawYugoPortals();
void drawYugoWakfuParticles();
void drawYugoDimensionalRifts();
void drawYugoEnergyOrbs();

// Naruto Sage Mode
void drawNarutoSageAura();
void drawNarutoKuramaFlames();
void drawNarutoRasengan();
void drawNarutoShadowClones();

// Goku Ultra Instinct
void drawGokuUIAura();
void drawGokuKiBlasts();
void drawGokuSpeedLines();
void drawGokuDivineGlow();

// Tanjiro Sun Breathing
void drawTanjiroSunFlames();
void drawTanjiroWaterTechniques();
void drawTanjiroHinokamiKagura();
void drawTanjiroNichirinBlade();

// Gojo Infinity
void drawGojoInfinityAura();
void drawGojoSixEyesGlow();
void drawGojoHollowPurple();
void drawGojoDomainExpansion();

// Levi Humanity's Strongest
void drawLeviBladeShine();
void drawLeviODMGearMotion();
void drawLeviSpeedBlur();
void drawLeviCleanAesthetic();

// Saitama One Punch
void drawSaitamaPunchImpact();
void drawSaitamaCapeFlow();
void drawSaitamaDotEyes();
void drawSaitamaPowerEffect();

// Deku Plus Ultra
void drawDekuOFALightning();
void drawDekuFullCowlAura();
void drawDekuAirForce();
void drawDekuPowerStrainMarks();

// =============================================================================
// ACTIVITY RINGS - ALL CHARACTERS
// =============================================================================

void drawLuffyActivityRings(int centerX, int centerY);
void drawJinwooActivityRings(int centerX, int centerY);
void drawYugoActivityRings(int centerX, int centerY);
void drawNarutoActivityRings(int centerX, int centerY);
void drawGokuActivityRings(int centerX, int centerY);
void drawTanjiroActivityRings(int centerX, int centerY);
void drawGojoActivityRings(int centerX, int centerY);
void drawLeviActivityRings(int centerX, int centerY);
void drawSaitamaActivityRings(int centerX, int centerY);
void drawDekuActivityRings(int centerX, int centerY);

// =============================================================================
// ANIMATIONS
// =============================================================================

void playLuffyAnimation();
void playJinwooAnimation();
void playYugoAnimation();
void playNarutoAnimation();
void playGokuAnimation();
void playTanjiroAnimation();
void playGojoAnimation();
void playLeviAnimation();
void playSaitamaAnimation();
void playDekuAnimation();

// =============================================================================
// UI ELEMENTS - GLASS MORPHISM
// =============================================================================

void drawThemeButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawGameButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawThemeProgressBar(int x, int y, int w, int h, float progress, const char* label);
void drawThemeNotification(const char* title, const char* message);

// Glass Morphism Components
void drawGlassPanel(int x, int y, int w, int h, uint8_t opacity = 25);
void drawGlassButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawGlassCard(int x, int y, int w, int h);
void drawGlassStatBar(int x, int y, int w, int h, float progress, uint16_t color, const char* label);

// Character Signature Rings
void drawCharacterRings(int centerX, int centerY);
void drawPulsingGlow(int x, int y, int radius, uint16_t color);

// Watch face utilities
void drawStepCounter();
void drawBatteryIndicator();

// =============================================================================
// CHARACTER STATS DISPLAY
// =============================================================================

void drawCharacterStatsScreen();
void drawStatBar(int x, int y, int w, int h, int value, int maxValue, uint16_t color, const char* label);
void drawAnimatedStatBar(int x, int y, int w, int h, float targetProgress, uint16_t color);

// Theme Selector
void drawThemeSelector();
void handleThemeSelectorTouch(TouchGesture& gesture);

#endif // THEMES_H
