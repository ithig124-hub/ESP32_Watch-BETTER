/**
 * Theme System - Luffy, JinWoo, Yugo
 */

#ifndef THEMES_H
#define THEMES_H

#include <lvgl.h>
#include "config.h"

// Initialize themes
void initThemes();

// Get colors for theme
ThemeColors getThemeColors(ThemeType theme);

// Apply theme to UI
void applyTheme(ThemeType theme);

// Get theme name
const char* getThemeName(ThemeType theme);

// Theme-specific watch faces
void drawLuffyWatchFace(lv_obj_t* parent);
void drawJinWooWatchFace(lv_obj_t* parent);
void drawYugoWatchFace(lv_obj_t* parent);

// Theme-specific effects
void drawThemeEffects(lv_obj_t* parent, ThemeType theme);

// Wallpaper screen
lv_obj_t* createWallpaperScreen();

#endif