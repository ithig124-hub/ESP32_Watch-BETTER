/*
 * boboiboy_elements.cpp - BoBoiBoy Element Tree Implementation
 * Visual element tree showing all forms, evolutions, and fusions
 * Only accessible when BoBoiBoy theme is active
 */

#include "boboiboy_elements.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Current state
int selected_element = 0;
int element_tree_page = 0;  // 0 = Base, 1 = Evolved, 2 = Fusions

// Element unlock status (for game progression)
static bool elements_unlocked[FORM_COUNT] = {
  true, true, true, true, true, true, true,  // Base elements always unlocked
  false, false, false, false, false, false, false,  // Tier 2 locked initially
  false, false, false, false, false, false   // Fusions locked initially
};

// =============================================================================
// ELEMENT DATA - All 20 BoBoiBoy Forms
// =============================================================================

ElementData boboiboy_elements[FORM_COUNT] = {
  // Tier 1 - Base Elements (7)
  {"Halilintar", "Lightning", BBB_LIGHTNING_YELLOW, BBB_LIGHTNING_BLACK, 
   TIER_BASE, "Lightning Speed", "First elemental power. Super speed and lightning attacks.", -1, -1},
  
  {"Taufan", "Wind", BBB_WIND_BLUE, BBB_WIND_CYAN,
   TIER_BASE, "Wind Control", "Controls wind and air currents. Can fly with hover board.", -1, -1},
  
  {"Gempa", "Earth", BBB_EARTH_BROWN, BBB_EARTH_ORANGE,
   TIER_BASE, "Earth Power", "Controls earth and rock. Creates golems and barriers.", -1, -1},
  
  {"Blaze", "Fire", BBB_FIRE_RED, BBB_FIRE_ORANGE,
   TIER_BASE, "Fire Control", "Controls fire with intense heat. Aggressive fighter.", -1, -1},
  
  {"Ice", "Ice", BBB_WATER_CYAN, BBB_WATER_WHITE,
   TIER_BASE, "Ice Creation", "Creates and controls ice. Calm and strategic.", -1, -1},
  
  {"Thorn", "Leaf", BBB_LEAF_GREEN, BBB_LEAF_DARK,
   TIER_BASE, "Plant Control", "Controls plants and vines. Nature powers.", -1, -1},
  
  {"Solar", "Light", BBB_LIGHT_GOLD, BBB_LIGHT_WHITE,
   TIER_BASE, "Light Energy", "Harnesses solar energy. Powerful light attacks.", -1, -1},
  
  // Tier 2 - Evolved Forms (7)
  {"Thunderstorm", "Guruh", BBB_THUNDERSTORM_BLACK, BBB_THUNDERSTORM_RED,
   TIER_EVOLVED, "Thunder Fury", "Evolved Halilintar. Dark lightning with red eyes.", FORM_HALILINTAR, -1},
  
  {"Cyclone", "Taufan+", BBB_CYCLONE_DARK_BLUE, BBB_WIND_CYAN,
   TIER_EVOLVED, "Storm Master", "Evolved Taufan. Creates powerful cyclones.", FORM_TAUFAN, -1},
  
  {"Quake", "Gempa+", BBB_QUAKE_DARK, BBB_EARTH_ORANGE,
   TIER_EVOLVED, "Earthquake", "Evolved Gempa. Massive earth manipulation.", FORM_GEMPA, -1},
  
  {"Inferno", "Blaze+", BBB_BLAZE_CRIMSON, BBB_FIRE_ORANGE,
   TIER_EVOLVED, "Hellfire", "Evolved Blaze. Blue-core flames.", FORM_BLAZE, -1},
  
  {"Glacier", "Ice+", BBB_ICE_LIGHT, BBB_WATER_CYAN,
   TIER_EVOLVED, "Absolute Zero", "Evolved Ice. Flash freeze abilities.", FORM_ICE, -1},
  
  {"Darkwood", "Thorn+", BBB_THORN_DARK, BBB_LEAF_GREEN,
   TIER_EVOLVED, "Forest Wrath", "Evolved Thorn. Dark nature powers.", FORM_THORN, -1},
  
  {"Supernova", "Solar+", BBB_SOLAR_ORANGE, BBB_LIGHT_GOLD,
   TIER_EVOLVED, "Star Power", "Evolved Solar. Stellar energy control.", FORM_SOLAR, -1},
  
  // Fusions (6) - OFFICIAL BOBOIBOY FUSIONS
  {"FrostFire", "Api Ais", BBB_FROSTFIRE_PINK, BBB_FIRE_RED,
   TIER_FUSION, "Hot & Cold", "Blaze + Ice fusion. Fire and ice combined.", FORM_BLAZE, FORM_ICE},
  
  {"Glacier", "Glasier", BBB_GLACIER_BLUE, BBB_EARTH_BROWN,
   TIER_FUSION, "Frozen Earth", "Quake + Ice fusion. Icy rock powers.", FORM_QUAKE, FORM_ICE},
  
  {"Supra", "Supra", BBB_SUPRA_GOLD, BBB_THUNDERSTORM_BLACK,
   TIER_FUSION, "Ultimate Power", "Thunderstorm + Solar. Strongest fusion.", FORM_THUNDERSTORM, FORM_SUPERNOVA},
  
  {"Sori", "Sori", BBB_LIGHT_GOLD, BBB_LEAF_GREEN,
   TIER_FUSION, "Light Thorns", "Thorn + Solar fusion. Nature and light.", FORM_DARKWOOD, FORM_SUPERNOVA},
  
  {"Rumble", "Rumble", BBB_EARTH_ORANGE, BBB_THUNDERSTORM_RED,
   TIER_FUSION, "Thunder Quake", "Quake + Thunderstorm. Seismic thunder.", FORM_QUAKE, FORM_THUNDERSTORM},
  
  {"Sopan", "Sopan", BBB_LIGHT_GOLD, BBB_WIND_CYAN,
   TIER_FUSION, "Solar Wind", "Solar + Cyclone. Light and storm.", FORM_SUPERNOVA, FORM_CYCLONE}
};

// =============================================================================
// INITIALIZATION
// =============================================================================

void initBoboiboyElements() {
  Serial.println("[BBB] Initializing element tree system...");
  selected_element = 0;
  element_tree_page = 0;
  
  // Base elements always unlocked
  for (int i = 0; i < 7; i++) {
    elements_unlocked[i] = true;
  }
}

// =============================================================================
// ELEMENT TREE DRAWING - MAIN SCREEN
// =============================================================================

void drawElementTree() {
  gfx->fillScreen(RGB565(8, 10, 15));
  
  int centerX = LCD_WIDTH / 2;
  
  // Header with gradient
  for (int y = 0; y < 50; y++) {
    uint8_t alpha = 30 - (y * 25 / 50);
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(alpha, alpha/2, 0));
  }
  
  // Title
  gfx->setTextColor(BBB_BAND_ORANGE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 12);
  gfx->print("ELEMENT TREE");
  
  // Page tabs
  const char* tabs[] = {"BASE", "EVOLVED", "FUSION"};
  uint16_t tabColors[] = {BBB_LIGHTNING_YELLOW, BBB_THUNDERSTORM_RED, BBB_FROSTFIRE_PINK};
  
  for (int i = 0; i < 3; i++) {
    int tabX = 20 + i * 115;
    int tabY = 50;
    int tabW = 105;
    int tabH = 28;
    
    if (i == element_tree_page) {
      // Active tab
      gfx->fillRoundRect(tabX, tabY, tabW, tabH, 8, RGB565(30, 32, 40));
      gfx->drawRoundRect(tabX, tabY, tabW, tabH, 8, tabColors[i]);
      gfx->setTextColor(tabColors[i]);
    } else {
      // Inactive tab
      gfx->fillRoundRect(tabX, tabY, tabW, tabH, 8, RGB565(20, 22, 28));
      gfx->drawRoundRect(tabX, tabY, tabW, tabH, 8, RGB565(60, 60, 70));
      gfx->setTextColor(RGB565(100, 100, 110));
    }
    
    gfx->setTextSize(1);
    int textW = strlen(tabs[i]) * 6;
    gfx->setCursor(tabX + (tabW - textW) / 2, tabY + 10);
    gfx->print(tabs[i]);
  }
  
  // Draw elements based on current page
  if (element_tree_page == 0) {
    drawBaseElements();
  } else if (element_tree_page == 1) {
    drawEvolvedElements();
  } else {
    drawFusionElements();
  }
  
  // Bottom instruction
  gfx->setTextColor(RGB565(80, 85, 95));
  gfx->setTextSize(1);
  gfx->setCursor(centerX - 55, 415);
  gfx->print("Tap element for details");
  
  // Back button
  drawBackButtonElement(10, 405);
}

void drawBackButtonElement(int x, int y) {
  gfx->fillRoundRect(x, y, 70, 30, 10, RGB565(30, 32, 40));
  gfx->drawRoundRect(x, y, 70, 30, 10, BBB_BAND_ORANGE);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(x + 18, y + 10);
  gfx->print("Back");
}

// =============================================================================
// BASE ELEMENTS (7) - Circular Layout
// =============================================================================

void drawBaseElements() {
  int centerX = LCD_WIDTH / 2;
  int centerY = 230;
  int radius = 100;
  
  // Center - BoBoiBoy icon
  gfx->fillCircle(centerX, centerY, 35, RGB565(30, 32, 40));
  gfx->drawCircle(centerX, centerY, 35, BBB_BAND_ORANGE);
  gfx->drawCircle(centerX, centerY, 33, BBB_BAND_GLOW);
  
  gfx->setTextColor(BBB_BAND_ORANGE);
  gfx->setTextSize(1);
  gfx->setCursor(centerX - 24, centerY - 10);
  gfx->print("BOBOIBOY");
  gfx->setCursor(centerX - 18, centerY + 2);
  gfx->print("KUASA 7");
  
  // Draw 7 elements in a circle
  for (int i = 0; i < 7; i++) {
    float angle = (i * 51.43 - 90) * PI / 180.0;  // 360/7 = 51.43
    int nodeX = centerX + cos(angle) * radius;
    int nodeY = centerY + sin(angle) * radius;
    
    // Connection line to center
    gfx->drawLine(centerX, centerY, nodeX, nodeY, RGB565(40, 42, 50));
    
    // Draw element node
    bool isSelected = (selected_element == i);
    drawElementNode(nodeX, nodeY, (BoBoiBoyForm)i, isSelected, true);
  }
  
  // Element info card at bottom
  if (selected_element >= 0 && selected_element < 7) {
    drawElementInfoCard(boboiboy_elements[selected_element], 330);
  }
}

// =============================================================================
// EVOLVED ELEMENTS (7) - With Evolution Arrows
// =============================================================================

void drawEvolvedElements() {
  int startY = 95;
  int spacing = 85;
  
  // Draw 7 base -> evolved pairs
  for (int i = 0; i < 7; i++) {
    int row = i / 2;
    int col = i % 2;
    int baseX = 50 + col * 180;
    int evolvedX = baseX + 90;
    int y = startY + row * spacing;
    
    // Special case for 7th element (centered)
    if (i == 6) {
      baseX = LCD_WIDTH / 2 - 45;
      evolvedX = baseX + 90;
      y = startY + 3 * spacing;
    }
    
    // Base element (small)
    uint16_t baseColor = boboiboy_elements[i].primary_color;
    gfx->fillCircle(baseX, y, 18, baseColor);
    gfx->drawCircle(baseX, y, 18, COLOR_WHITE);
    
    // Arrow
    gfx->drawLine(baseX + 22, y, evolvedX - 22, y, RGB565(100, 100, 110));
    gfx->fillTriangle(evolvedX - 22, y - 5, evolvedX - 22, y + 5, evolvedX - 12, y, RGB565(100, 100, 110));
    
    // Evolved element
    int evolvedIdx = i + 7;
    bool unlocked = elements_unlocked[evolvedIdx];
    bool isSelected = (selected_element == evolvedIdx);
    
    if (unlocked) {
      uint16_t evolvedColor = boboiboy_elements[evolvedIdx].primary_color;
      gfx->fillCircle(evolvedX, y, 25, evolvedColor);
      if (isSelected) {
        gfx->drawCircle(evolvedX, y, 27, COLOR_WHITE);
        gfx->drawCircle(evolvedX, y, 29, boboiboy_elements[evolvedIdx].secondary_color);
      } else {
        gfx->drawCircle(evolvedX, y, 25, RGB565(80, 80, 90));
      }
    } else {
      gfx->fillCircle(evolvedX, y, 25, RGB565(30, 30, 35));
      gfx->drawCircle(evolvedX, y, 25, RGB565(50, 50, 55));
      gfx->setTextColor(RGB565(60, 60, 65));
      gfx->setTextSize(2);
      gfx->setCursor(evolvedX - 6, y - 8);
      gfx->print("?");
    }
    
    // Labels
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(150, 150, 160));
    
    // Base name
    const char* baseName = boboiboy_elements[i].name;
    int baseNameW = strlen(baseName) * 6;
    gfx->setCursor(baseX - baseNameW/2, y + 25);
    gfx->print(baseName);
    
    // Evolved name
    if (unlocked) {
      gfx->setTextColor(boboiboy_elements[evolvedIdx].primary_color);
      const char* evolvedName = boboiboy_elements[evolvedIdx].name;
      int evolvedNameW = strlen(evolvedName) * 6;
      gfx->setCursor(evolvedX - evolvedNameW/2, y + 32);
      gfx->print(evolvedName);
    }
  }
}

// =============================================================================
// FUSION ELEMENTS (6) - With Combination Diagram
// =============================================================================

void drawFusionElements() {
  int startY = 105;
  int spacing = 95;
  
  // Draw 6 fusions with their parent elements
  for (int i = 0; i < 6; i++) {
    int fusionIdx = 14 + i;  // Fusions start at index 14
    int row = i / 2;
    int col = i % 2;
    
    int x = 90 + col * 190;
    int y = startY + row * spacing;
    
    ElementData& fusion = boboiboy_elements[fusionIdx];
    bool unlocked = elements_unlocked[fusionIdx];
    bool isSelected = (selected_element == fusionIdx);
    
    // Parent elements (small circles)
    int parent1 = fusion.parent1;
    int parent2 = fusion.parent2;
    
    // Left parent
    gfx->fillCircle(x - 40, y - 15, 12, boboiboy_elements[parent1].primary_color);
    gfx->drawCircle(x - 40, y - 15, 12, RGB565(60, 60, 70));
    
    // Right parent
    gfx->fillCircle(x + 40, y - 15, 12, boboiboy_elements[parent2].primary_color);
    gfx->drawCircle(x + 40, y - 15, 12, RGB565(60, 60, 70));
    
    // Plus sign
    gfx->setTextColor(RGB565(100, 100, 110));
    gfx->setTextSize(2);
    gfx->setCursor(x - 6, y - 22);
    gfx->print("+");
    
    // Arrow down
    gfx->drawLine(x, y - 5, x, y + 10, RGB565(100, 100, 110));
    gfx->fillTriangle(x - 5, y + 10, x + 5, y + 10, x, y + 18, RGB565(100, 100, 110));
    
    // Fusion result
    if (unlocked) {
      gfx->fillCircle(x, y + 35, 28, fusion.primary_color);
      if (isSelected) {
        gfx->drawCircle(x, y + 35, 30, COLOR_WHITE);
        gfx->drawCircle(x, y + 35, 32, fusion.secondary_color);
      } else {
        gfx->drawCircle(x, y + 35, 28, RGB565(80, 80, 90));
      }
      
      // Fusion name
      gfx->setTextColor(fusion.primary_color);
      gfx->setTextSize(1);
      int nameW = strlen(fusion.name) * 6;
      gfx->setCursor(x - nameW/2, y + 70);
      gfx->print(fusion.name);
    } else {
      gfx->fillCircle(x, y + 35, 28, RGB565(25, 25, 30));
      gfx->drawCircle(x, y + 35, 28, RGB565(50, 50, 55));
      gfx->setTextColor(RGB565(60, 60, 65));
      gfx->setTextSize(2);
      gfx->setCursor(x - 6, y + 27);
      gfx->print("?");
      
      gfx->setTextColor(RGB565(80, 80, 85));
      gfx->setTextSize(1);
      gfx->setCursor(x - 18, y + 70);
      gfx->print("LOCKED");
    }
  }
}

// =============================================================================
// ELEMENT NODE DRAWING
// =============================================================================

void drawElementNode(int x, int y, BoBoiBoyForm form, bool selected, bool unlocked) {
  ElementData& elem = boboiboy_elements[(int)form];
  int radius = 28;
  
  if (unlocked) {
    // Filled circle with element color
    gfx->fillCircle(x, y, radius, elem.primary_color);
    
    // Selection highlight
    if (selected) {
      gfx->drawCircle(x, y, radius + 2, COLOR_WHITE);
      gfx->drawCircle(x, y, radius + 4, elem.secondary_color);
    } else {
      gfx->drawCircle(x, y, radius, RGB565(80, 80, 90));
    }
    
    // Element initial
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    char initial[2] = {elem.name[0], '\0'};
    gfx->setCursor(x - 6, y - 8);
    gfx->print(initial);
    
  } else {
    // Locked - gray circle with question mark
    gfx->fillCircle(x, y, radius, RGB565(30, 30, 35));
    gfx->drawCircle(x, y, radius, RGB565(50, 50, 55));
    gfx->setTextColor(RGB565(60, 60, 65));
    gfx->setTextSize(2);
    gfx->setCursor(x - 6, y - 8);
    gfx->print("?");
  }
}

// =============================================================================
// ELEMENT INFO CARD
// =============================================================================

void drawElementInfoCard(ElementData& elem, int y) {
  int cardX = 20;
  int cardW = LCD_WIDTH - 40;
  int cardH = 70;
  
  // Card background
  gfx->fillRoundRect(cardX, y, cardW, cardH, 12, RGB565(25, 27, 35));
  gfx->drawRoundRect(cardX, y, cardW, cardH, 12, elem.primary_color);
  
  // Color stripe
  gfx->fillRoundRect(cardX, y, 8, cardH, 12, elem.primary_color);
  gfx->fillRect(cardX + 4, y, 4, cardH, elem.primary_color);
  
  // Element name
  gfx->setTextColor(elem.primary_color);
  gfx->setTextSize(2);
  gfx->setCursor(cardX + 20, y + 8);
  gfx->print(elem.name);
  
  // Malay name
  gfx->setTextColor(RGB565(120, 120, 130));
  gfx->setTextSize(1);
  gfx->setCursor(cardX + 20 + strlen(elem.name) * 12 + 10, y + 12);
  gfx->print(elem.malay_name);
  
  // Power
  gfx->setTextColor(elem.secondary_color);
  gfx->setCursor(cardX + 20, y + 32);
  gfx->print(elem.power);
  
  // Description (truncated)
  gfx->setTextColor(RGB565(150, 150, 160));
  gfx->setCursor(cardX + 20, y + 48);
  char desc[45];
  strncpy(desc, elem.description, 44);
  desc[44] = '\0';
  gfx->print(desc);
}

// =============================================================================
// TOUCH HANDLING
// =============================================================================

void handleElementTreeTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_SWIPE_LEFT || gesture.event == TOUCH_SWIPE_RIGHT) {
    // Change page
    if (gesture.event == TOUCH_SWIPE_LEFT && element_tree_page < 2) {
      element_tree_page++;
      selected_element = -1;
      drawElementTree();
    } else if (gesture.event == TOUCH_SWIPE_RIGHT && element_tree_page > 0) {
      element_tree_page--;
      selected_element = -1;
      drawElementTree();
    }
    return;
  }
  
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x;
  int y = gesture.y;
  
  // Back button
  if (x >= 10 && x < 80 && y >= 405 && y < 435) {
    returnToAppGrid();
    return;
  }
  
  // Tab selection
  if (y >= 50 && y < 78) {
    for (int i = 0; i < 3; i++) {
      int tabX = 20 + i * 115;
      if (x >= tabX && x < tabX + 105) {
        element_tree_page = i;
        selected_element = -1;
        drawElementTree();
        return;
      }
    }
  }
  
  // Element selection based on page
  if (element_tree_page == 0) {
    // Base elements - circular layout
    int centerX = LCD_WIDTH / 2;
    int centerY = 230;
    int radius = 100;
    
    for (int i = 0; i < 7; i++) {
      float angle = (i * 51.43 - 90) * PI / 180.0;
      int nodeX = centerX + cos(angle) * radius;
      int nodeY = centerY + sin(angle) * radius;
      
      int dist = sqrt((x - nodeX) * (x - nodeX) + (y - nodeY) * (y - nodeY));
      if (dist < 35) {
        selected_element = i;
        drawElementTree();
        return;
      }
    }
  }
  else if (element_tree_page == 1) {
    // Evolved elements
    int startY = 95;
    int spacing = 85;
    
    for (int i = 0; i < 7; i++) {
      int row = i / 2;
      int col = i % 2;
      int evolvedX = 50 + col * 180 + 90;
      int elemY = startY + row * spacing;
      
      if (i == 6) {
        evolvedX = LCD_WIDTH / 2 + 45;
        elemY = startY + 3 * spacing;
      }
      
      int dist = sqrt((x - evolvedX) * (x - evolvedX) + (y - elemY) * (y - elemY));
      if (dist < 30) {
        selected_element = 7 + i;
        drawElementTree();
        return;
      }
    }
  }
  else {
    // Fusion elements
    int startY = 105;
    int spacing = 95;
    
    for (int i = 0; i < 6; i++) {
      int row = i / 2;
      int col = i % 2;
      int fusionX = 90 + col * 190;
      int fusionY = startY + row * spacing + 35;
      
      int dist = sqrt((x - fusionX) * (x - fusionX) + (y - fusionY) * (y - fusionY));
      if (dist < 35) {
        selected_element = 14 + i;
        drawElementTree();
        return;
      }
    }
  }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

bool isElementUnlocked(BoBoiBoyForm form) {
  return elements_unlocked[(int)form];
}

void unlockElement(BoBoiBoyForm form) {
  elements_unlocked[(int)form] = true;
  Serial.printf("[BBB] Unlocked element: %s\n", boboiboy_elements[(int)form].name);
}

uint16_t getElementColor(BoBoiBoyForm form) {
  return boboiboy_elements[(int)form].primary_color;
}

const char* getElementName(BoBoiBoyForm form) {
  return boboiboy_elements[(int)form].name;
}
