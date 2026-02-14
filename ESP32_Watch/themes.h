/*
 * themes.h - Theme System
 * Luffy Gear 5, Sung Jin-Woo, Yugo Wakfu themes
 */

#ifndef THEMES_H
#define THEMES_H

#include "config.h"

// Theme color schemes
extern ThemeColors luffy_gear5_theme;
extern ThemeColors sung_jinwoo_theme;
extern ThemeColors yugo_wakfu_theme;
extern ThemeColors* current_theme;

// =============================================================================
// THEME MANAGEMENT
// =============================================================================

void initializeThemes();
void setTheme(ThemeType theme);
ThemeColors* getCurrentTheme();

// Watch faces
void drawLuffyWatchFace();
void drawJinwooWatchFace();
void drawYugoWatchFace();
void drawSleepWatchFace();
void drawWatchFace();

// Custom wallpaper
void drawCustomWallpaperWatchFace();
void drawWallpaperOverlayElements();
void drawMinimalTimeOverlay();

// Theme animations
void playLuffyAnimation();
void playJinwooAnimation();
void playYugoAnimation();

// UI elements
void drawThemeButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawGameButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawThemeProgressBar(int x, int y, int w, int h, float progress, const char* label);
void drawThemeNotification(const char* title, const char* message);

// Character effects
void drawLuffyGear5Effects();
void drawLuffyCartoonEffects();
void drawJinwooShadows();
void drawYugoPortals();

// Activity rings
void drawLuffyActivityRings(int centerX, int centerY);
void drawJinwooActivityRings(int centerX, int centerY);
void drawYugoActivityRings(int centerX, int centerY);

#endif // THEMES_H
