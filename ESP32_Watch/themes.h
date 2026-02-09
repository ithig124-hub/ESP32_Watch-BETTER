/**
 * Theme System - Luffy, JinWoo, Yugo + Random Daily
 * No bouncing, no rubber physics - clean controlled animations
 */

#ifndef THEMES_H
#define THEMES_H

#include <lvgl.h>
#include "config.h"

// Initialize themes
void initThemes();

// Get colors for theme (handles Random theme automatically)
ThemeColors getThemeColors(ThemeType theme);

// Get colors for specific random character
ThemeColors getRandomCharacterColors(RandomCharacter character);

// Apply theme to UI
void applyTheme(ThemeType theme);

// Get theme name
const char* getThemeName(ThemeType theme);
const char* getRandomCharacterName(RandomCharacter character);
const char* getRandomCharacterTitle(RandomCharacter character);
const char* getRandomCharacterSeries(RandomCharacter character);

// Wallpaper/Theme selector screen
lv_obj_t* createWallpaperScreen();

// Daily character rotation
RandomCharacter calculateDailyCharacter();
void updateDailyCharacter();

// Anime-style UI elements (speech bubbles, action lines - static)
void drawSpeechBubble(lv_obj_t* parent, const char* text, int x, int y);
void drawActionLines(lv_obj_t* parent, uint32_t color);
void drawPowerAura(lv_obj_t* parent, uint32_t color, int intensity);

// Theme-specific effects (static visual elements, no motion)
void drawThemeEffects(lv_obj_t* parent, ThemeType theme);
void drawRandomCharacterEffects(lv_obj_t* parent, RandomCharacter character);

#endif
