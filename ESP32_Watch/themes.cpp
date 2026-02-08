/**
 * Theme System Implementation
 */

#include "themes.h"
#include "ui_manager.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════════════
static const ThemeColors luffyColors = {
  .primary    = 0xFFA500,  // Orange
  .secondary  = 0xFFD700,  // Gold
  .accent     = 0xFF4500,  // Red-Orange
  .background = 0x1A0A00,  // Dark warm
  .text       = 0xFFFAF0   // Cream
};

static const ThemeColors jinwooColors = {
  .primary    = 0x9A0EEA,  // Purple
  .secondary  = 0x4B0082,  // Indigo
  .accent     = 0xB266FF,  // Light purple
  .background = 0x0A0014,  // Dark purple
  .text       = 0xE8E8E8   // Silver
};

static const ThemeColors yugoColors = {
  .primary    = 0x00CED1,  // Dark cyan
  .secondary  = 0x0077BE,  // Blue
  .accent     = 0x40E0D0,  // Turquoise
  .background = 0x001419,  // Dark teal
  .text       = 0xE0FFFF   // Light cyan
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THEME FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void initThemes() {
  applyTheme(watch.theme);
}

ThemeColors getThemeColors(ThemeType theme) {
  switch(theme) {
    case THEME_LUFFY:  return luffyColors;
    case THEME_JINWOO: return jinwooColors;
    case THEME_YUGO:   return yugoColors;
    default:           return luffyColors;
  }
}

const char* getThemeName(ThemeType theme) {
  switch(theme) {
    case THEME_LUFFY:  return "Luffy - Sun God";
    case THEME_JINWOO: return "Jin-Woo - Shadow";
    case THEME_YUGO:   return "Yugo - Portal";
    default:           return "Unknown";
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WALLPAPER/THEME SELECTOR SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createWallpaperScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  // Title bar
  createTitleBar(scr, "Themes");
  
  // Theme buttons
  const char* names[] = {"Luffy\nSun God", "Jin-Woo\nShadow", "Yugo\nPortal"};
  const uint32_t btnColors[] = {0xFFA500, 0x9A0EEA, 0x00CED1};
  
  for (int i = 0; i < 3; i++) {
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, LCD_WIDTH - 40, 100);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 70 + i * 110);
    lv_obj_set_style_bg_color(btn, lv_color_hex(btnColors[i]), 0);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
      lv_obj_t* btn = lv_event_get_target(e);
      ThemeType theme = (ThemeType)(intptr_t)lv_obj_get_user_data(btn);
      applyTheme(theme);
      
      // Refresh all screens with new theme
      for (int i = 0; i < 11; i++) {
        if (screens[i]) {
          lv_obj_del(screens[i]);
          screens[i] = nullptr;
        }
      }
      showScreen(SCREEN_CLOCK);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lbl, names[i]);
    lv_obj_center(lbl);
    
    // Current indicator
    if (i == (int)watch.theme) {
      lv_obj_t* check = lv_label_create(btn);
      lv_obj_set_style_text_color(check, lv_color_hex(0xFFFFFF), 0);
      lv_label_set_text(check, LV_SYMBOL_OK);
      lv_obj_align(check, LV_ALIGN_RIGHT_MID, -10, 0);
    }
  }
  
  return scr;
}
