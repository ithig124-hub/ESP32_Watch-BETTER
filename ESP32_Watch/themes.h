/**
 * Premium Theme System - Apple Watch Inspired
 * Enhanced Anime Themes with Luxury Aesthetics
 */

#ifndef THEMES_H
#define THEMES_H

#include "config.h"
#include <lvgl.h>

// Initialize theme system
void initThemes();

// Theme management
void applyTheme(ThemeType theme);
ThemeColors getThemeColors(ThemeType theme);
const char* getThemeName(ThemeType theme);

// Random daily character system
void updateDailyCharacter();
RandomCharacter calculateDailyCharacter();
const char* getRandomCharacterName(RandomCharacter character);
const char* getRandomCharacterTitle(RandomCharacter character);
const char* getRandomCharacterSeries(RandomCharacter character);

// Premium UI elements (Apple Watch style)
void drawGlassmorphismCard(lv_obj_t* parent, int x, int y, int w, int h, uint32_t tint);
void drawPremiumGlow(lv_obj_t* parent, uint32_t color, int intensity);
void drawSubtleGradient(lv_obj_t* parent, uint32_t color1, uint32_t color2);
void drawAnimePowerAura(lv_obj_t* parent, uint32_t color, int intensity);

// Theme selector screen
lv_obj_t* createWallpaperScreen();

#endif
