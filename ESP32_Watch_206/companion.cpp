/*
 * companion.cpp - Virtual Pet System with NVS + Detailed Sprites
 * FUSION OS - 11 unique companions with evolution sprites
 */

#include "companion.h"
#include "config.h"
#include "themes.h"
#include "xp_system.h"
#include "display.h"
#include "touch.h"
#include "navigation.h"  

CompanionSystemState companion_system;

CompanionProfile COMPANION_PROFILES[COMPANION_COUNT] = {
 {"Sunny", "One Piece", LUFFY_SUN_GOLD, LUFFY_NIKA_WHITE, "Meat", "Sailing", "Set sail!"},
 {"Igris", "Solo Leveling", RGB565(200, 50, 50), JINWOO_ARISE_GLOW, "Mana Crystals", "Training", "..."},
 {"Az", "Wakfu", YUGO_PORTAL_CYAN, YUGO_HAT_GOLD, "Dragon Treats", "Portal Chase", "Let's explore!"},
 {"Kurama", "Naruto", NARUTO_KURAMA_FLAME, NARUTO_CHAKRA_ORANGE, "Chakra Pills", "Rasengan", "Tch, brat..."},
 {"Puar", "Dragon Ball", GOKU_UI_SILVER, GOKU_AURA_WHITE, "Fish", "Shape Shift", "Yamcha!"},
 {"Nezuko", "Demon Slayer", TANJIRO_FIRE_ORANGE, TANJIRO_CHECK_GREEN, "Sleep", "Box Time", "Mmmph!"},
 {"Infinity", "Jujutsu Kaisen", GOJO_INFINITY_BLUE, GOJO_SNOW_WHITE, "Cursed Energy", "Barriers", "Honored one!"},
 {"Wings", "Attack on Titan", LEVI_SURVEY_GREEN, LEVI_SILVER_BLADE, "Tea", "Cleaning", "Keep it clean."},
 {"Genos Jr", "One Punch Man", SAITAMA_HERO_YELLOW, SAITAMA_CAPE_RED, "Parts", "Training", "Sensei!"},
 {"Might Jr", "My Hero Academia", DEKU_HERO_GREEN, DEKU_ALLMIGHT_GOLD, "Justice", "SMASH", "I AM HERE!"},
 {"Ochobot", "BoBoiBoy", BBB_BAND_ORANGE, BBB_OCHOBOT_WHITE, "Power Sphera", "Elements", "Let's go!"}
};

static void getCompKey(int i, const char* s, char* b, size_t n) { snprintf(b,n,"c%d_%s",i,s); }

// =============================================================================
// DETAILED COMPANION SPRITES
// =============================================================================

void drawSunnySprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2;
 y += bob;
 // --- Sunny Lion Mascot (Thousand Sunny figurehead) ---
 // Sun-petal mane (7 orange petals radiating from head)
 uint16_t maneOrange = RGB565(255, 160, 30);
 uint16_t maneDark = RGB565(220, 120, 20);
 int maneR = size/2 + size/6;
 for (int p = 0; p < 7; p++) {
   float angle = (p * 51 - 153) * PI / 180.0; // spread behind and above head
   int px = x + cos(angle) * maneR;
   int py = y - size/4 + sin(angle) * maneR;
   // Each petal is a fat triangle
   int bx1 = x + cos(angle - 0.3) * (size/3);
   int by1 = y - size/4 + sin(angle - 0.3) * (size/3);
   int bx2 = x + cos(angle + 0.3) * (size/3);
   int by2 = y - size/4 + sin(angle + 0.3) * (size/3);
   gfx->fillTriangle(bx1, by1, bx2, by2, px, py, maneOrange);
   // Inner line for depth
   gfx->drawLine((bx1+bx2)/2, (by1+by2)/2, px, py, maneDark);
 }
 // Round yellow head
 uint16_t bodyYellow = RGB565(255, 235, 100);
 gfx->fillCircle(x, y - size/4, size/3, bodyYellow);
 // Plump yellow torso
 gfx->fillCircle(x, y + size/6, size/4, bodyYellow);
 gfx->fillRect(x - size/4, y - size/8, size/2, size/3, bodyYellow);
 // Big round eyes
 gfx->fillCircle(x - size/7, y - size/4 - 2, size/8, COLOR_WHITE);
 gfx->fillCircle(x + size/7, y - size/4 - 2, size/8, COLOR_WHITE);
 gfx->fillCircle(x - size/7, y - size/4 - 2, size/14, COLOR_BLACK);
 gfx->fillCircle(x + size/7, y - size/4 - 2, size/14, COLOR_BLACK);
 // Eye shine
 gfx->fillCircle(x - size/7 + 2, y - size/4 - 4, 2, COLOR_WHITE);
 gfx->fillCircle(x + size/7 + 2, y - size/4 - 4, 2, COLOR_WHITE);
 // Small brown nose
 gfx->fillCircle(x, y - size/6, size/12, RGB565(160, 120, 80));
 // Open happy mouth
 gfx->fillCircle(x, y - size/8, size/7, RGB565(220, 100, 120));
 gfx->fillRect(x - size/7, y - size/6, size*2/7, size/10, bodyYellow); // upper lip mask
 // White belt around waist
 gfx->fillRect(x - size/4, y + size/8, size/2, size/8, COLOR_WHITE);
 // Belt buckle (circle with "1")
 gfx->fillCircle(x, y + size/8 + size/16, size/10, RGB565(80, 40, 30));
 gfx->fillCircle(x, y + size/8 + size/16, size/14, RGB565(255, 200, 50));
 gfx->setTextColor(RGB565(150, 40, 30)); gfx->setTextSize(1);
 gfx->setCursor(x - 3, y + size/8 + size/16 - 3); gfx->print("1");
 // Dark red lower body / legs
 uint16_t legColor = RGB565(120, 30, 30);
 gfx->fillRect(x - size/5, y + size/4, size/6, size/4, legColor);
 gfx->fillRect(x + size/15, y + size/4, size/6, size/4, legColor);
 gfx->fillCircle(x - size/5 + size/12, y + size/2, size/10, legColor);
 gfx->fillCircle(x + size/15 + size/12, y + size/2, size/10, legColor);
 // Stubby yellow arms
 gfx->fillCircle(x - size/3, y + size/12, size/10, bodyYellow);
 gfx->fillCircle(x + size/3, y + size/12, size/10, bodyYellow);
 // Big fluffy brown tail (right side)
 uint16_t tailBrown = RGB565(180, 110, 50);
 gfx->fillCircle(x + size/3, y + size/6, size/4, tailBrown);
 gfx->fillCircle(x + size/3 + size/6, y + size/8, size/5, tailBrown);
 gfx->fillCircle(x + size/3 + size/4, y + size/5, size/6, RGB565(160, 90, 40));
 // Tail texture lines
 gfx->drawLine(x + size/4, y + size/8, x + size/2, y + size/5, RGB565(140, 80, 35));
 gfx->drawLine(x + size/4, y + size/4, x + size/2, y + size/3, RGB565(140, 80, 35));
}

void drawShadowSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2 - 1;
 y += bob;
 // --- IGRIS: Blood-Red Commander Knight ---
 uint16_t armorRed = RGB565(180, 30, 30);
 uint16_t armorDark = RGB565(100, 15, 15);
 uint16_t armorHighlight = RGB565(220, 60, 60);
 uint16_t capeColor = RGB565(140, 20, 20);
 uint16_t goldTrim = RGB565(200, 160, 50);
 uint16_t visorGlow = JINWOO_ARISE_GLOW;

 // Cape flowing behind
 gfx->fillTriangle(x - size/4, y - size/6, x - size/3, y + size/2 + size/6, x + size/6, y + size/2, capeColor);
 gfx->fillTriangle(x + size/4, y - size/6, x + size/3, y + size/2 + size/6, x - size/6, y + size/2, capeColor);
 gfx->drawLine(x - size/4, y + size/4, x - size/3, y + size/2 + size/6, armorDark);

 // Armored body (chest plate)
 gfx->fillRect(x - size/4, y - size/6, size/2, size/2, armorRed);
 gfx->drawRect(x - size/4, y - size/6, size/2, size/2, armorDark);
 gfx->drawLine(x, y - size/8, x - size/6, y + size/8, goldTrim);
 gfx->drawLine(x, y - size/8, x + size/6, y + size/8, goldTrim);
 gfx->drawFastVLine(x, y - size/6, size/2, armorDark);

 // Shoulder pauldrons
 gfx->fillCircle(x - size/3, y - size/8, size/6, armorRed);
 gfx->drawCircle(x - size/3, y - size/8, size/6, goldTrim);
 gfx->fillCircle(x + size/3, y - size/8, size/6, armorRed);
 gfx->drawCircle(x + size/3, y - size/8, size/6, goldTrim);
 gfx->fillTriangle(x - size/3, y - size/8 - size/6, x - size/3 - 5, y - size/8 - size/4, x - size/3 + 5, y - size/8 - size/4, armorHighlight);
 gfx->fillTriangle(x + size/3, y - size/8 - size/6, x + size/3 - 5, y - size/8 - size/4, x + size/3 + 5, y - size/8 - size/4, armorHighlight);

 // Helmet
 gfx->fillCircle(x, y - size/3, size/4, armorRed);
 gfx->drawCircle(x, y - size/3, size/4, armorDark);
 gfx->fillTriangle(x, y - size/3 - size/4, x - size/10, y - size/3 - size/10, x + size/10, y - size/3 - size/10, armorHighlight);
 gfx->fillRect(x - 2, y - size/3 - size/6, 4, size/8, armorHighlight);
 // Visor with glowing eyes
 gfx->fillRect(x - size/5, y - size/3 - 2, size*2/5, size/10, COLOR_BLACK);
 gfx->fillRect(x - size/6, y - size/3 - 1, size/6, size/12, visorGlow);
 gfx->fillRect(x + size/20, y - size/3 - 1, size/6, size/12, visorGlow);
 if (frame % 3 != 2) gfx->drawRect(x - size/5 - 1, y - size/3 - 3, size*2/5 + 2, size/10 + 2, visorGlow);
 gfx->fillTriangle(x - size/6, y - size/5, x + size/6, y - size/5, x, y - size/8, armorDark);

 // Arms + gauntlets
 gfx->fillRect(x - size/3 - size/8, y - size/12, size/6, size/3, armorRed);
 gfx->fillRect(x + size/4, y - size/12, size/6, size/3, armorRed);
 gfx->fillCircle(x - size/3 - size/16, y + size/4, size/10, armorDark);
 gfx->fillCircle(x + size/4 + size/10, y + size/4, size/10, armorDark);

 // Sword
 int sx = x + size/4 + size/10;
 gfx->fillRect(sx - 1, y - size/3, 3, size*2/3, RGB565(200, 200, 210));
 gfx->fillRect(sx - 1, y - size/3 - 3, 3, 5, COLOR_WHITE);
 gfx->fillRect(sx - size/10, y + size/6 - 2, size/5, 4, goldTrim);
 gfx->fillRect(sx - 1, y + size/6, 3, size/8, RGB565(80, 40, 20));
 if (frame % 4 == 0) gfx->drawFastVLine(sx + 2, y - size/4, size/3, COLOR_WHITE);

 // Armored legs + boots
 gfx->fillRect(x - size/5, y + size/4, size/7, size/4, armorRed);
 gfx->fillRect(x + size/10, y + size/4, size/7, size/4, armorRed);
 gfx->fillRect(x - size/5 - 2, y + size/2 - 4, size/7 + 4, 6, armorDark);
 gfx->fillRect(x + size/10 - 2, y + size/2 - 4, size/7 + 4, 6, armorDark);

 // Shadow aura at feet
 for (int w = -2; w <= 2; w++) {
   int wx = x + w * size/5;
   gfx->drawLine(wx, y + size/2 + 3, wx + (frame%2?3:-3), y + size/2 + 8, JINWOO_MIST_PURPLE);
 }
}

void drawAzSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 3 - 1;
 y += bob;
 // Dragon body (round, cute)
 gfx->fillCircle(x, y, size/3, YUGO_ELIATROPE_TEAL);
 gfx->fillCircle(x, y + size/6, size/4, YUGO_ELIATROPE_TEAL);
 // Belly
 gfx->fillCircle(x, y + size/8, size/5, YUGO_HAT_GOLD);
 // Head
 gfx->fillCircle(x, y - size/3, size/4, YUGO_PORTAL_CYAN);
 // Horns
 gfx->fillTriangle(x - size/5, y - size/3 - size/6, x - size/5 - 5, y - size/3 - size/3, x - size/5 + 5, y - size/3 - size/4, YUGO_HAT_GOLD);
 gfx->fillTriangle(x + size/5, y - size/3 - size/6, x + size/5 - 5, y - size/3 - size/3, x + size/5 + 5, y - size/3 - size/4, YUGO_HAT_GOLD);
 // Big cute eyes
 gfx->fillCircle(x - size/8, y - size/3, 6, COLOR_WHITE);
 gfx->fillCircle(x + size/8, y - size/3, 6, COLOR_WHITE);
 gfx->fillCircle(x - size/8, y - size/3, 3, YUGO_PORTAL_CYAN);
 gfx->fillCircle(x + size/8, y - size/3, 3, YUGO_PORTAL_CYAN);
 gfx->fillCircle(x - size/8 + 1, y - size/3 - 1, 1, COLOR_WHITE);
 gfx->fillCircle(x + size/8 + 1, y - size/3 - 1, 1, COLOR_WHITE);
 // Wings (flapping based on frame)
 int wingAngle = (frame % 2) * 10;
 gfx->fillTriangle(x - size/3, y - size/6, x - size/2 - size/4, y - size/3 - wingAngle, x - size/4, y, YUGO_PORTAL_GLOW);
 gfx->fillTriangle(x + size/3, y - size/6, x + size/2 + size/4, y - size/3 - wingAngle, x + size/4, y, YUGO_PORTAL_GLOW);
 // Tiny tail
 gfx->fillTriangle(x + size/4, y + size/4, x + size/3 + 8, y + size/3, x + size/4, y + size/3, YUGO_ELIATROPE_TEAL);
 // Nose
 gfx->fillCircle(x, y - size/4, 2, YUGO_HAT_GOLD);
}

void drawKuramaSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2;
 y += bob;
 // Fox body
 gfx->fillCircle(x, y, size/3, NARUTO_KURAMA_FLAME);
 // Head
 gfx->fillCircle(x, y - size/4, size/4, NARUTO_CHAKRA_ORANGE);
 // Pointed ears
 gfx->fillTriangle(x - size/4, y - size/3, x - size/5, y - size/2 - size/5, x - size/8, y - size/3, NARUTO_KURAMA_FLAME);
 gfx->fillTriangle(x + size/4, y - size/3, x + size/5, y - size/2 - size/5, x + size/8, y - size/3, NARUTO_KURAMA_FLAME);
 // Inner ears
 gfx->fillTriangle(x - size/5, y - size/3, x - size/6, y - size/2 - size/8, x - size/9, y - size/3, NARUTO_CHAKRA_ORANGE);
 gfx->fillTriangle(x + size/5, y - size/3, x + size/6, y - size/2 - size/8, x + size/9, y - size/3, NARUTO_CHAKRA_ORANGE);
 // Red slitted eyes
 gfx->fillCircle(x - size/8, y - size/4, 4, COLOR_RED);
 gfx->fillCircle(x + size/8, y - size/4, 4, COLOR_RED);
 gfx->fillRect(x - size/8 - 1, y - size/4 - 1, 3, 3, COLOR_BLACK);
 gfx->fillRect(x + size/8 - 1, y - size/4 - 1, 3, 3, COLOR_BLACK);
 // Snout
 gfx->fillCircle(x, y - size/6, size/8, NARUTO_SAGE_GOLD);
 gfx->fillCircle(x, y - size/6 - 2, 2, COLOR_BLACK);
 // Tails! (9 of them, fanned out)
 for (int t = 0; t < 9; t++) {
   float angle = (t * 20 - 80 + (frame % 2) * 5) * PI / 180.0;
   int tx = x + cos(angle) * (size/2 + t * 2);
   int ty = y + size/3 + sin(angle) * size/4 + 5;
   gfx->drawLine(x, y + size/4, tx, ty, NARUTO_KURAMA_FLAME);
   gfx->fillCircle(tx, ty, 3, NARUTO_WILL_FIRE);
 }
 // Whisker marks
 gfx->drawLine(x - size/3, y - size/5, x - size/6, y - size/5, NARUTO_CHAKRA_ORANGE);
 gfx->drawLine(x + size/6, y - size/5, x + size/3, y - size/5, NARUTO_CHAKRA_ORANGE);
}

void drawPuarSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 3 - 1;
 y += bob;
 // Body
 gfx->fillCircle(x, y, size/3, GOKU_GI_BLUE);
 gfx->fillCircle(x, y + size/6, size/4, GOKU_GI_BLUE);
 // Head (larger, cat-like)
 gfx->fillCircle(x, y - size/4, size/3, GOKU_GI_BLUE);
 // Pointy ears
 gfx->fillTriangle(x - size/3, y - size/3, x - size/4, y - size/2 - size/4, x - size/6, y - size/3, GOKU_GI_BLUE);
 gfx->fillTriangle(x + size/3, y - size/3, x + size/4, y - size/2 - size/4, x + size/6, y - size/3, GOKU_GI_BLUE);
 // Inner ears
 gfx->fillTriangle(x - size/4, y - size/3, x - size/5, y - size/2 - size/6, x - size/6, y - size/3, RGB565(255, 180, 180));
 gfx->fillTriangle(x + size/4, y - size/3, x + size/5, y - size/2 - size/6, x + size/6, y - size/3, RGB565(255, 180, 180));
 // Big round eyes
 gfx->fillCircle(x - size/7, y - size/4, 6, COLOR_WHITE);
 gfx->fillCircle(x + size/7, y - size/4, 6, COLOR_WHITE);
 gfx->fillCircle(x - size/7, y - size/4, 3, COLOR_BLACK);
 gfx->fillCircle(x + size/7, y - size/4, 3, COLOR_BLACK);
 gfx->fillCircle(x - size/7 + 1, y - size/4 - 1, 1, COLOR_WHITE);
 gfx->fillCircle(x + size/7 + 1, y - size/4 - 1, 1, COLOR_WHITE);
 // Small nose
 gfx->fillCircle(x, y - size/6, 2, RGB565(255, 150, 150));
 // Tiny wings (shape-shifter!)
 gfx->fillTriangle(x - size/3, y - size/8, x - size/2, y - size/4, x - size/4, y + size/8, GOKU_AURA_WHITE);
 gfx->fillTriangle(x + size/3, y - size/8, x + size/2, y - size/4, x + size/4, y + size/8, GOKU_AURA_WHITE);
 // Tail
 gfx->drawLine(x + size/4, y + size/4, x + size/3, y + size/3, GOKU_GI_BLUE);
 gfx->drawLine(x + size/3, y + size/3, x + size/3 + 5, y + size/4, GOKU_GI_BLUE);
}

void drawNezukoSprite(int x, int y, int size, int frame) {
 int bob = (frame % 3 == 0) ? 2 : 0;
 y += bob;
 // Box body (she's in the box!)
 int bw = size*2/3, bh = size;
 gfx->fillRect(x - bw/2, y - bh/3, bw, bh*2/3, RGB565(139, 90, 43));
 gfx->drawRect(x - bw/2, y - bh/3, bw, bh*2/3, RGB565(100, 60, 30));
 // Wood grain lines
 gfx->drawFastHLine(x - bw/2 + 3, y - bh/6, bw - 6, RGB565(120, 75, 35));
 gfx->drawFastHLine(x - bw/2 + 3, y + bh/6, bw - 6, RGB565(120, 75, 35));
 // Nezuko peeking out! Hair on top
 gfx->fillCircle(x, y - bh/3 - size/6, size/4, RGB565(30, 30, 30));
 // Pink eyes peeking over edge
 gfx->fillCircle(x - size/8, y - bh/3 + 3, 5, RGB565(255, 150, 180));
 gfx->fillCircle(x + size/8, y - bh/3 + 3, 5, RGB565(255, 150, 180));
 gfx->fillCircle(x - size/8, y - bh/3 + 3, 2, COLOR_BLACK);
 gfx->fillCircle(x + size/8, y - bh/3 + 3, 2, COLOR_BLACK);
 // Bamboo muzzle
 gfx->fillRect(x - size/4, y - bh/3 + 8, size/2, 5, RGB565(100, 180, 80));
 gfx->drawRect(x - size/4, y - bh/3 + 8, size/2, 5, RGB565(60, 120, 40));
 // Hair ribbon
 gfx->fillRect(x + size/6, y - bh/3 - size/4, 6, 8, RGB565(255, 100, 150));
 // Blush marks
 gfx->fillCircle(x - size/5, y - bh/3 + 5, 3, RGB565(255, 180, 180));
 gfx->fillCircle(x + size/5, y - bh/3 + 5, 3, RGB565(255, 180, 180));
}

void drawSpiritSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2 - 1;
 y += bob;
 // Infinity sphere with rotating rings
 gfx->fillCircle(x, y, size/3, GOJO_DEEP_INFINITY);
 gfx->drawCircle(x, y, size/3, GOJO_INFINITY_BLUE);
 // Rotating ring effect
 int ringR = size/3 + 5 + (frame % 3) * 2;
 gfx->drawCircle(x, y, ringR, GOJO_LIGHT_BLUE_GLOW);
 // Infinity symbol in center
 gfx->drawCircle(x - size/8, y, size/6, GOJO_SIX_EYES_BLUE);
 gfx->drawCircle(x + size/8, y, size/6, GOJO_SIX_EYES_BLUE);
 // Central eye (Six Eyes reference)
 gfx->fillCircle(x, y, size/6, COLOR_WHITE);
 gfx->fillCircle(x, y, size/8, GOJO_INFINITY_BLUE);
 gfx->fillCircle(x, y, size/12, COLOR_BLACK);
 gfx->fillCircle(x + 1, y - 1, 2, COLOR_WHITE);
 // Floating particles
 for (int p = 0; p < 6; p++) {
   float a = (p * 60 + frame * 15) * PI / 180.0;
   int px = x + cos(a) * (size/2);
   int py = y + sin(a) * (size/2);
   gfx->fillCircle(px, py, 2, GOJO_LIGHT_BLUE_GLOW);
 }
}

void drawBladesSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2;
 y += bob;
 // Wings of Freedom emblem
 // Left wing (blue)
 gfx->fillTriangle(x - 5, y - size/3, x - size/2, y - size/6, x - 5, y + size/4, GOJO_INFINITY_BLUE);
 gfx->fillTriangle(x - 5, y - size/3, x - size/2 - size/6, y, x - size/2, y - size/6, GOJO_INFINITY_BLUE);
 // Right wing (white)
 gfx->fillTriangle(x + 5, y - size/3, x + size/2, y - size/6, x + 5, y + size/4, COLOR_WHITE);
 gfx->fillTriangle(x + 5, y - size/3, x + size/2 + size/6, y, x + size/2, y - size/6, COLOR_WHITE);
 // Center shield
 gfx->fillRect(x - 5, y - size/4, 10, size/2, LEVI_DARK_UNIFORM);
 gfx->drawRect(x - 5, y - size/4, 10, size/2, LEVI_SILVER_BLADE);
 // Crossed blades
 gfx->drawLine(x - size/3, y - size/2, x + size/3, y + size/4, LEVI_SILVER_BLADE);
 gfx->drawLine(x + size/3, y - size/2, x - size/3, y + size/4, LEVI_SILVER_BLADE);
 // Blade shine
 if (frame % 3 == 0) {
   gfx->fillCircle(x - size/6, y - size/4, 2, COLOR_WHITE);
   gfx->fillCircle(x + size/6, y - size/8, 2, COLOR_WHITE);
 }
 // Green glow (Survey Corps)
 gfx->drawCircle(x, y, size/2 + 3, LEVI_SURVEY_GREEN);
}

void drawGenosSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2 - 1;
 y += bob;
 // Robot body
 gfx->fillRect(x - size/4, y - size/6, size/2, size/2, RGB565(40, 40, 50));
 // Head (round, mechanical)
 gfx->fillCircle(x, y - size/3, size/4, SAITAMA_HERO_YELLOW);
 gfx->drawCircle(x, y - size/3, size/4, RGB565(200, 180, 40));
 // Mechanical eyes
 gfx->fillRect(x - size/6, y - size/3 - 3, size/8, 6, COLOR_BLACK);
 gfx->fillRect(x + size/12, y - size/3 - 3, size/8, 6, COLOR_BLACK);
 gfx->fillCircle(x - size/8, y - size/3, 3, COLOR_YELLOW);
 gfx->fillCircle(x + size/8, y - size/3, 3, COLOR_YELLOW);
 // Glowing core
 uint16_t coreColor = (frame % 2 == 0) ? RGB565(255, 150, 50) : RGB565(255, 200, 100);
 gfx->fillCircle(x, y + size/12, size/8, coreColor);
 gfx->drawCircle(x, y + size/12, size/8, SAITAMA_HERO_YELLOW);
 // Mechanical arms
 gfx->fillRect(x - size/3 - size/6, y - size/8, size/6, size/3, RGB565(60, 60, 70));
 gfx->fillRect(x + size/4, y - size/8, size/6, size/3, RGB565(60, 60, 70));
 // Arm cannons
 gfx->fillCircle(x - size/3 - size/10, y + size/6, size/10, SAITAMA_GOLDEN_PUNCH);
 gfx->fillCircle(x + size/4 + size/10, y + size/6, size/10, SAITAMA_GOLDEN_PUNCH);
 // Legs
 gfx->fillRect(x - size/6, y + size/4, size/8, size/4, RGB565(50, 50, 60));
 gfx->fillRect(x + size/12, y + size/4, size/8, size/4, RGB565(50, 50, 60));
 // Vent lines on body
 gfx->drawFastHLine(x - size/6, y - size/8, size/3, RGB565(80, 80, 90));
 gfx->drawFastHLine(x - size/6, y, size/3, RGB565(80, 80, 90));
}

void drawAllMightSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 2;
 y += bob;
 // Muscular body
 gfx->fillRect(x - size/3, y - size/6, size*2/3, size/2, GOKU_GI_BLUE);
 // Broad shoulders
 gfx->fillCircle(x - size/3, y - size/8, size/6, GOKU_GI_BLUE);
 gfx->fillCircle(x + size/3, y - size/8, size/6, GOKU_GI_BLUE);
 // Head
 gfx->fillCircle(x, y - size/3, size/4, RGB565(220, 180, 140));
 // Golden hair (iconic)
 gfx->fillTriangle(x - size/5, y - size/3 - size/5, x, y - size/2 - size/4, x + size/5, y - size/3 - size/5, DEKU_ALLMIGHT_GOLD);
 gfx->fillTriangle(x - size/8, y - size/2, x - size/4, y - size/2 - size/6, x, y - size/2, DEKU_ALLMIGHT_GOLD);
 gfx->fillTriangle(x, y - size/2, x + size/4, y - size/2 - size/6, x + size/8, y - size/2, DEKU_ALLMIGHT_GOLD);
 // Eyes (shadowed, heroic)
 gfx->fillRect(x - size/6, y - size/3, size/3, size/10, RGB565(50, 50, 80));
 gfx->fillCircle(x - size/8, y - size/3 + 2, 3, GOKU_KI_BLAST_BLUE);
 gfx->fillCircle(x + size/8, y - size/3 + 2, 3, GOKU_KI_BLAST_BLUE);
 // Big smile
 gfx->drawLine(x - size/6, y - size/5, x + size/6, y - size/5, COLOR_WHITE);
 gfx->drawLine(x - size/6, y - size/5, x - size/8, y - size/5 + 3, COLOR_WHITE);
 gfx->drawLine(x + size/6, y - size/5, x + size/8, y - size/5 + 3, COLOR_WHITE);
 // Cape (red)
 gfx->fillTriangle(x - size/3, y - size/8, x - size/2, y + size/3, x - size/6, y + size/4, SAITAMA_CAPE_RED);
 gfx->fillTriangle(x + size/3, y - size/8, x + size/2, y + size/3, x + size/6, y + size/4, SAITAMA_CAPE_RED);
 // Fists
 gfx->fillCircle(x - size/2, y + size/8, size/8, RGB565(220, 180, 140));
 gfx->fillCircle(x + size/2, y + size/8, size/8, RGB565(220, 180, 140));
}

void drawOchobotSprite(int x, int y, int size, int frame) {
 int bob = (frame % 2) * 3 - 1;
 y += bob;
 // Main sphere body
 gfx->fillCircle(x, y, size/3, BBB_OCHOBOT_WHITE);
 gfx->drawCircle(x, y, size/3, RGB565(200, 200, 210));
 // Orange band around middle
 gfx->fillRect(x - size/3, y - size/12, size*2/3, size/6, BBB_BAND_ORANGE);
 gfx->drawRect(x - size/3, y - size/12, size*2/3, size/6, RGB565(200, 100, 20));
 // Eyes (big, round, friendly)
 gfx->fillCircle(x - size/7, y - size/8, 7, COLOR_WHITE);
 gfx->fillCircle(x + size/7, y - size/8, 7, COLOR_WHITE);
 gfx->fillCircle(x - size/7, y - size/8, 4, GOJO_INFINITY_BLUE);
 gfx->fillCircle(x + size/7, y - size/8, 4, GOJO_INFINITY_BLUE);
 gfx->fillCircle(x - size/7, y - size/8, 2, COLOR_BLACK);
 gfx->fillCircle(x + size/7, y - size/8, 2, COLOR_BLACK);
 gfx->fillCircle(x - size/7 + 1, y - size/8 - 1, 1, COLOR_WHITE);
 gfx->fillCircle(x + size/7 + 1, y - size/8 - 1, 1, COLOR_WHITE);
 // Antenna
 gfx->fillRect(x - 2, y - size/3 - size/6, 4, size/6, RGB565(200, 200, 210));
 gfx->fillCircle(x, y - size/3 - size/6, 5, BBB_BAND_ORANGE);
 // Glow on antenna (signal)
 if (frame % 3 == 0) {
   gfx->drawCircle(x, y - size/3 - size/6, 8, BBB_BAND_GLOW);
 }
 // Small arms
 gfx->fillRect(x - size/3 - size/6, y - size/12, size/6, size/10, RGB565(200, 200, 210));
 gfx->fillRect(x + size/3, y - size/12, size/6, size/10, RGB565(200, 200, 210));
 // Hover glow at bottom
 gfx->drawCircle(x, y + size/3 + 5, size/5, BBB_BAND_GLOW);
 gfx->drawCircle(x, y + size/3 + 5, size/6, BBB_BAND_ORANGE);
}

// =============================================================================
// MAIN SPRITE DISPATCHER (with evolution scaling)
// =============================================================================
void drawCompanionSprite(int x, int y, CompanionType type, CompanionEvolution evo) {
 int size = 20 + (int)evo * 12; // Baby=20, Child=32, Adult=44, Awakened=56
 int frame = companion_system.animation_frame;

 // Awakened glow aura
 if (evo == EVO_AWAKENED) {
   uint16_t glowCol = COMPANION_PROFILES[type].primary_color;
   for (int r = size/2 + 15; r > size/2 + 5; r -= 2) {
     gfx->drawCircle(x, y, r, glowCol);
   }
 }

 switch (type) {
   case COMP_SUNNY: drawSunnySprite(x, y, size, frame); break;
   case COMP_IGRIS: drawShadowSprite(x, y, size, frame); break;
   case COMP_AZ: drawAzSprite(x, y, size, frame); break;
   case COMP_KURAMA: drawKuramaSprite(x, y, size, frame); break;
   case COMP_PUAR: drawPuarSprite(x, y, size, frame); break;
   case COMP_NEZUKO: drawNezukoSprite(x, y, size, frame); break;
   case COMP_SPIRIT: drawSpiritSprite(x, y, size, frame); break;
   case COMP_BLADES: drawBladesSprite(x, y, size, frame); break;
   case COMP_GENOS: drawGenosSprite(x, y, size, frame); break;
   case COMP_ALLMIGHT: drawAllMightSprite(x, y, size, frame); break;
   case COMP_OCHOBOT: drawOchobotSprite(x, y, size, frame); break;
 }

 // Evolution label
 if (evo >= EVO_CHILD) {
   gfx->setTextColor(COMPANION_PROFILES[type].primary_color);
   gfx->setTextSize(1);
   gfx->setCursor(x - 15, y + size/2 + 10);
   gfx->print(getEvolutionText(evo));
 }

 // Sleeping indicator
 CompanionData* comp = companion_system.current_companion;
 if (comp && comp->care.is_sleeping) {
   gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2);
   gfx->setCursor(x + size/3, y - size/2);
   gfx->print("Zzz");
 }
}

// =============================================================================
// INIT, NVS, CARE, BOND, GAME, DRAW, TOUCH (same core logic as before)
// =============================================================================
void initCompanionSystem() {
 companion_system.current_companion = nullptr;
 companion_system.current_companion_index = -1;
 companion_system.in_care_mode = false;
 companion_system.in_mini_game = false;
 companion_system.care_menu_selection = 0;
 companion_system.animation_frame = 0;
 companion_system.last_animation_time = 0;
 companion_system.nvs_initialized = false;
 companion_system.current_game = {"", 0, 0, false, 0, 0, 0};

 for (int i = 0; i < COMPANION_COUNT; i++) {
   companion_system.companions[i].type = (CompanionType)i;
   companion_system.companions[i].profile = &COMPANION_PROFILES[i];
   companion_system.companions[i].stats = {80, 80, 80, 0, 1, MOOD_CONTENT, EVO_BABY, 0, 0};
   companion_system.companions[i].care = {0, 0, 0, millis(), 0, 0, 0, false, 0};
 }

 if (companion_system.prefs.begin(COMPANION_NVS_NAMESPACE, false)) {
   companion_system.nvs_initialized = true;
   loadCompanionData();
 }
}

void saveCompanionData() {
 if (!companion_system.nvs_initialized) return;
 for (int i = 0; i < COMPANION_COUNT; i++) saveCompanionDataForIndex(i);
 companion_system.prefs.putInt("current_idx", companion_system.current_companion_index);
 companion_system.prefs.putBool("initialized", true);
}

void saveCompanionDataForIndex(int idx) {
 if (!companion_system.nvs_initialized || idx < 0 || idx >= COMPANION_COUNT) return;
 CompanionData* c = &companion_system.companions[idx]; char k[32];
 getCompKey(idx,"hunger",k,32); companion_system.prefs.putInt(k,c->stats.hunger);
 getCompKey(idx,"happy",k,32); companion_system.prefs.putInt(k,c->stats.happiness);
 getCompKey(idx,"energy",k,32); companion_system.prefs.putInt(k,c->stats.energy);
 getCompKey(idx,"bond",k,32); companion_system.prefs.putInt(k,c->stats.bond_level);
 getCompKey(idx,"rank",k,32); companion_system.prefs.putInt(k,c->stats.bond_rank);
 getCompKey(idx,"mood",k,32); companion_system.prefs.putInt(k,(int)c->stats.mood);
 getCompKey(idx,"evo",k,32); companion_system.prefs.putInt(k,(int)c->stats.evolution);
 getCompKey(idx,"interact",k,32); companion_system.prefs.putInt(k,c->stats.total_interactions);
 getCompKey(idx,"days",k,32); companion_system.prefs.putInt(k,c->stats.days_together);
 getCompKey(idx,"sleep",k,32); companion_system.prefs.putBool(k,c->care.is_sleeping);
 getCompKey(idx,"d_feed",k,32); companion_system.prefs.putInt(k,c->care.daily_feed_count);
 getCompKey(idx,"d_play",k,32); companion_system.prefs.putInt(k,c->care.daily_play_count);
 getCompKey(idx,"d_train",k,32); companion_system.prefs.putInt(k,c->care.daily_train_count);
}

void loadCompanionData() {
 if (!companion_system.nvs_initialized || !companion_system.prefs.getBool("initialized", false)) return;
 for (int i = 0; i < COMPANION_COUNT; i++) loadCompanionDataForIndex(i);
 int si = companion_system.prefs.getInt("current_idx", -1);
 if (si >= 0 && si < COMPANION_COUNT) { companion_system.current_companion_index = si; companion_system.current_companion = &companion_system.companions[si]; }
}

void loadCompanionDataForIndex(int idx) {
 if (!companion_system.nvs_initialized || idx < 0 || idx >= COMPANION_COUNT) return;
 CompanionData* c = &companion_system.companions[idx]; char k[32];
 getCompKey(idx,"hunger",k,32); c->stats.hunger = companion_system.prefs.getInt(k,80);
 getCompKey(idx,"happy",k,32); c->stats.happiness = companion_system.prefs.getInt(k,80);
 getCompKey(idx,"energy",k,32); c->stats.energy = companion_system.prefs.getInt(k,80);
 getCompKey(idx,"bond",k,32); c->stats.bond_level = companion_system.prefs.getInt(k,0);
 getCompKey(idx,"rank",k,32); c->stats.bond_rank = companion_system.prefs.getInt(k,1);
 getCompKey(idx,"mood",k,32); c->stats.mood = (CompanionMood)companion_system.prefs.getInt(k,MOOD_CONTENT);
 getCompKey(idx,"evo",k,32); c->stats.evolution = (CompanionEvolution)companion_system.prefs.getInt(k,EVO_BABY);
 getCompKey(idx,"interact",k,32); c->stats.total_interactions = companion_system.prefs.getInt(k,0);
 getCompKey(idx,"days",k,32); c->stats.days_together = companion_system.prefs.getInt(k,0);
 getCompKey(idx,"sleep",k,32); c->care.is_sleeping = companion_system.prefs.getBool(k,false);
 getCompKey(idx,"d_feed",k,32); c->care.daily_feed_count = companion_system.prefs.getInt(k,0);
 getCompKey(idx,"d_play",k,32); c->care.daily_play_count = companion_system.prefs.getInt(k,0);
 getCompKey(idx,"d_train",k,32); c->care.daily_train_count = companion_system.prefs.getInt(k,0);
 c->care.last_stat_update = millis();
}

void clearAllCompanionData() {
 if (!companion_system.nvs_initialized) return;
 companion_system.prefs.clear();
 for (int i = 0; i < COMPANION_COUNT; i++) {
   companion_system.companions[i].stats = {80,80,80,0,1,MOOD_CONTENT,EVO_BABY,0,0};
   companion_system.companions[i].care = {0,0,0,millis(),0,0,0,false,0};
 }
}

void setCurrentCompanion(ThemeType t) {
 if (t < THEME_COUNT) { companion_system.current_companion_index = (int)t; companion_system.current_companion = &companion_system.companions[t];
 if (companion_system.nvs_initialized) companion_system.prefs.putInt("current_idx", (int)t); }
}
CompanionData* getCurrentCompanion() { return companion_system.current_companion; }
CompanionData* getCompanionByType(CompanionType t) { return (t < COMPANION_COUNT) ? &companion_system.companions[t] : nullptr; }

bool feedCompanion() {
 CompanionData* c = companion_system.current_companion; if (!c || c->care.is_sleeping || system_state.player_gems < FEED_COST_GEMS) return false;
 system_state.player_gems -= FEED_COST_GEMS; c->stats.hunger = min(STAT_MAX, c->stats.hunger+FEED_HUNGER_GAIN);
 c->stats.happiness = min(STAT_MAX, c->stats.happiness+FEED_HAPPINESS_GAIN); c->stats.total_interactions++;
 c->care.last_feed_time = millis(); c->care.daily_feed_count++; updateCompanionMood(); addBondPoints(2);
 saveCompanionDataForIndex(companion_system.current_companion_index); return true;
}

bool playWithCompanion() {
 CompanionData* c = companion_system.current_companion; if (!c || c->care.is_sleeping || c->stats.energy < PLAY_ENERGY_COST) return false;
 c->stats.energy -= PLAY_ENERGY_COST; c->stats.happiness = min(STAT_MAX, c->stats.happiness+PLAY_HAPPINESS_GAIN);
 c->stats.total_interactions++; c->care.last_play_time = millis(); c->care.daily_play_count++;
 addBondPoints(PLAY_BOND_GAIN); updateCompanionMood(); saveCompanionDataForIndex(companion_system.current_companion_index);
 startCompanionGame(); return true;
}

bool trainCompanion() {
 CompanionData* c = companion_system.current_companion; if (!c || c->care.is_sleeping || c->stats.energy < TRAIN_ENERGY_COST) return false;
 c->stats.energy -= TRAIN_ENERGY_COST; c->stats.happiness = max(STAT_MIN, c->stats.happiness-5);
 c->stats.total_interactions++; c->care.last_train_time = millis(); c->care.daily_train_count++;
 gainExperience(TRAIN_XP_GAIN, "Companion Training"); addBondPoints(TRAIN_BOND_GAIN); updateCompanionMood();
 saveCompanionDataForIndex(companion_system.current_companion_index); return true;
}

void toggleCompanionSleep() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 if (c->care.is_sleeping) wakeCompanion(); else { c->care.is_sleeping = true; c->care.sleep_start_time = millis(); }
 saveCompanionDataForIndex(companion_system.current_companion_index);
}

void wakeCompanion() {
 CompanionData* c = companion_system.current_companion; if (!c || !c->care.is_sleeping) return;
 unsigned long dur = millis() - c->care.sleep_start_time;
 int gain = (dur >= REST_DURATION_MS) ? REST_ENERGY_GAIN : (int)((dur * REST_ENERGY_GAIN) / REST_DURATION_MS);
 c->stats.energy = min(STAT_MAX, c->stats.energy + gain); c->care.is_sleeping = false;
 updateCompanionMood(); saveCompanionDataForIndex(companion_system.current_companion_index);
}

void updateCompanionStats() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 unsigned long elapsed = millis() - c->care.last_stat_update;
 if (elapsed < 60000) return;
 float hrs = elapsed / 60000.0f;
 if (!c->care.is_sleeping) {
   c->stats.hunger = max(STAT_MIN, c->stats.hunger-(int)(HUNGER_DECAY_RATE*hrs));
   c->stats.happiness = max(STAT_MIN, c->stats.happiness-(int)(HAPPINESS_DECAY_RATE*hrs));
   c->stats.energy = max(STAT_MIN, c->stats.energy-(int)(ENERGY_DECAY_RATE*hrs));
 } else { c->stats.energy = min(STAT_MAX, c->stats.energy+(int)(5*hrs)); }
 c->care.last_stat_update = millis(); updateCompanionMood(); updateCompanionEvolution();
 saveCompanionDataForIndex(companion_system.current_companion_index);
}

void updateCompanionMood() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 int avg = (c->stats.hunger + c->stats.happiness + c->stats.energy) / 3;
 if (avg >= 80) c->stats.mood = MOOD_ECSTATIC; else if (avg >= 60) c->stats.mood = MOOD_HAPPY;
 else if (avg >= 40) c->stats.mood = MOOD_CONTENT; else if (avg >= 20) c->stats.mood = MOOD_SAD;
 else c->stats.mood = MOOD_MISERABLE;
}

void updateCompanionEvolution() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 if (c->stats.bond_level >= 80 && c->stats.total_interactions >= 200 && c->stats.evolution < EVO_AWAKENED) c->stats.evolution = EVO_AWAKENED;
 else if (c->stats.bond_level >= 50 && c->stats.total_interactions >= 100 && c->stats.evolution < EVO_ADULT) c->stats.evolution = EVO_ADULT;
 else if (c->stats.bond_level >= 20 && c->stats.total_interactions >= 30 && c->stats.evolution < EVO_CHILD) c->stats.evolution = EVO_CHILD;
}

void updateCompanionAnimation() {
 if (millis() - companion_system.last_animation_time >= 500) {
   companion_system.animation_frame = (companion_system.animation_frame + 1) % 4;
   companion_system.last_animation_time = millis();
 }
}

void checkCompanionDailyReset() {
 for (int i = 0; i < COMPANION_COUNT; i++) {
   companion_system.companions[i].care.daily_feed_count = 0;
   companion_system.companions[i].care.daily_play_count = 0;
   companion_system.companions[i].care.daily_train_count = 0;
   companion_system.companions[i].stats.days_together++;
 }
 saveCompanionData();
}

void addBondPoints(int pts) {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 c->stats.bond_level = min(BOND_MAX, c->stats.bond_level + pts);
 if (c->stats.bond_level >= BOND_RANK_5) c->stats.bond_rank = 5;
 else if (c->stats.bond_level >= BOND_RANK_4) c->stats.bond_rank = 4;
 else if (c->stats.bond_level >= BOND_RANK_3) c->stats.bond_rank = 3;
 else if (c->stats.bond_level >= BOND_RANK_2) c->stats.bond_rank = 2;
 else c->stats.bond_rank = 1;
}
int getBondRank() { return companion_system.current_companion ? companion_system.current_companion->stats.bond_rank : 1; }
const char* getBondRankName() { switch(getBondRank()) { case 1: return "Friend"; case 2: return "Good Friend"; case 3: return "Best Friend"; case 4: return "Soul Bond"; case 5: return "Eternal Bond"; default: return "Unknown"; } }
float getXPBonus() { return 1.0f + getBondRank() * 0.05f; }

void startCompanionGame() { companion_system.in_mini_game = true; companion_system.current_game = {"Catch!", 0, 0, true, millis(), (int)random(50,LCD_WIDTH-50), (int)random(100,LCD_HEIGHT-100)}; }
void updateCompanionGame() { if (!companion_system.current_game.active) return; if (millis()-companion_system.current_game.start_time >= 30000) { endCompanionGame(); return; }
 if ((millis()-companion_system.current_game.start_time) % 2000 < 100) { companion_system.current_game.target_x = random(50,LCD_WIDTH-50); companion_system.current_game.target_y = random(100,LCD_HEIGHT-100); } }
void endCompanionGame() { companion_system.current_game.active = false; companion_system.in_mini_game = false;
 if (companion_system.current_game.score > companion_system.current_game.high_score) companion_system.current_game.high_score = companion_system.current_game.score;
 CompanionData* c = companion_system.current_companion; if (c) { int b = companion_system.current_game.score/10; c->stats.happiness = min(STAT_MAX, c->stats.happiness+b); addBondPoints(b/2); saveCompanionDataForIndex(companion_system.current_companion_index); } }
void handleCompanionGameTouch(TouchGesture& g) { if (!companion_system.current_game.active || g.event != TOUCH_TAP) return;
 int dx = g.x - companion_system.current_game.target_x, dy = g.y - companion_system.current_game.target_y;
 if (dx*dx+dy*dy <= 1600) { companion_system.current_game.score += 10; companion_system.current_game.target_x = random(50,LCD_WIDTH-50); companion_system.current_game.target_y = random(100,LCD_HEIGHT-100); } }

// =============================================================================
// DRAWING
// =============================================================================
void drawCompanionScreen() {
 CompanionData* c = companion_system.current_companion;
 if (!c) { gfx->fillScreen(COLOR_BLACK); gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-80,LCD_HEIGHT/2); gfx->print("No companion"); return; }
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRect(0,0,LCD_WIDTH,50,c->profile->primary_color);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(20,15); gfx->print(c->profile->name);
 gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH-80,20); gfx->print(getEvolutionText(c->stats.evolution));
 // Sprite area
 gfx->fillRect(LCD_WIDTH/2-70, 55, 140, 140, RGB565(15,18,25));
 gfx->drawRect(LCD_WIDTH/2-70, 55, 140, 140, RGB565(40,45,60));
 drawCompanionSprite(LCD_WIDTH/2, 125, c->type, c->stats.evolution);
 drawCompanionMood(); drawCompanionStats(); drawBondLevel();
 // Catchphrase
 gfx->setTextColor(c->profile->primary_color); gfx->setTextSize(1); gfx->setCursor(20, 380);
 gfx->print("\""); gfx->print(c->profile->catchphrase); gfx->print("\"");
 // Care button
 gfx->fillRoundRect(LCD_WIDTH/2-60,LCD_HEIGHT-80,120,50,10,colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-30,LCD_HEIGHT-65); gfx->print("Care");
 gfx->fillRoundRect(30,LCD_HEIGHT-80,80,50,10,COLOR_GRAY);
 gfx->setTextSize(2); gfx->setCursor(45,LCD_HEIGHT-65); gfx->print("Back");
}

void drawCompanionStats() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 int bw = LCD_WIDTH-100, bh = 15, by = 210;
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1);
 gfx->setCursor(20,by); gfx->print("Hunger:");
 gfx->fillRect(80,by,bw,bh,COLOR_GRAY); gfx->fillRect(80,by,(bw*c->stats.hunger)/100,bh, isStatCritical(c->stats.hunger)?COLOR_RED:COLOR_GREEN);
 by += 30; gfx->setCursor(20,by); gfx->print("Happy:");
 gfx->fillRect(80,by,bw,bh,COLOR_GRAY); gfx->fillRect(80,by,(bw*c->stats.happiness)/100,bh, isStatCritical(c->stats.happiness)?COLOR_RED:COLOR_YELLOW);
 by += 30; gfx->setCursor(20,by); gfx->print("Energy:");
 gfx->fillRect(80,by,bw,bh,COLOR_GRAY); gfx->fillRect(80,by,(bw*c->stats.energy)/100,bh, isStatCritical(c->stats.energy)?COLOR_RED:COLOR_CYAN);
}

void drawCareMenu() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRect(0,0,LCD_WIDTH,50,c->profile->primary_color);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(20,15); gfx->print("Care Menu");
 int by = 70, bh = 60, bg = 15;
 gfx->fillRoundRect(30,by,LCD_WIDTH-60,bh,10,system_state.player_gems>=FEED_COST_GEMS?colors.primary:COLOR_GRAY);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(50,by+10); gfx->print("Feed");
 gfx->setTextSize(1); gfx->setCursor(50,by+35); gfx->print("Cost: "); gfx->print(FEED_COST_GEMS); gfx->print(" Gems");
 by += bh+bg;
 gfx->fillRoundRect(30,by,LCD_WIDTH-60,bh,10,c->stats.energy>=PLAY_ENERGY_COST?colors.secondary:COLOR_GRAY);
 gfx->setTextSize(2); gfx->setCursor(50,by+10); gfx->print("Play");
 gfx->setTextSize(1); gfx->setCursor(50,by+35); gfx->print("Energy: -"); gfx->print(PLAY_ENERGY_COST);
 by += bh+bg;
 gfx->fillRoundRect(30,by,LCD_WIDTH-60,bh,10,c->stats.energy>=TRAIN_ENERGY_COST?colors.accent:COLOR_GRAY);
 gfx->setTextSize(2); gfx->setCursor(50,by+10); gfx->print("Train");
 gfx->setTextSize(1); gfx->setCursor(50,by+35); gfx->print("XP: +"); gfx->print(TRAIN_XP_GAIN);
 by += bh+bg;
 gfx->fillRoundRect(30,by,LCD_WIDTH-60,bh,10,RGB565(80,80,120));
 gfx->setTextSize(2); gfx->setCursor(50,by+10); gfx->print(c->care.is_sleeping?"Wake Up":"Rest");
 gfx->setTextSize(1); gfx->setCursor(50,by+35); gfx->print(c->care.is_sleeping?"Energy restoring...":"Restores energy");
 gfx->fillRoundRect(30,LCD_HEIGHT-60,80,45,10,COLOR_GRAY);
 gfx->setTextSize(2); gfx->setCursor(45,LCD_HEIGHT-48); gfx->print("Back");
 gfx->setTextColor(COLOR_YELLOW); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH-100,LCD_HEIGHT-50); gfx->print("Gems: "); gfx->print(system_state.player_gems);
}

void drawCompanionMood() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 gfx->fillRoundRect(LCD_WIDTH-100,60,90,30,8,getMoodColor(c->stats.mood));
 gfx->setTextColor(COLOR_BLACK); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH-95,70); gfx->print(getMoodText(c->stats.mood));
}

void drawBondLevel() {
 CompanionData* c = companion_system.current_companion; if (!c) return;
 int y = 320;
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1); gfx->setCursor(20,y); gfx->print("Bond: "); gfx->print(getBondRankName());
 int sx = 20; y += 20;
 for (int i = 0; i < 5; i++) {
   uint16_t sc = (i < c->stats.bond_rank) ? COLOR_GOLD : COLOR_GRAY;
   gfx->fillTriangle(sx,y+10,sx+10,y,sx+20,y+10,sc);
   gfx->fillTriangle(sx,y+10,sx+20,y+10,sx+10,y+20,sc);
   sx += 25;
 }
 y += 30; int bw = LCD_WIDTH-100;
 gfx->fillRect(20,y,bw,8,COLOR_GRAY); gfx->fillRect(20,y,(bw*c->stats.bond_level)/BOND_MAX,8,COLOR_GOLD);
 gfx->setTextColor(COLOR_CYAN); gfx->setCursor(LCD_WIDTH-70,y-5); gfx->print("+"); gfx->print((int)((getXPBonus()-1.0f)*100)); gfx->print("% XP");
}

void drawCompanionGame() {
 if (!companion_system.current_game.active) return;
 gfx->fillScreen(COLOR_BLACK);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-40,20); gfx->print(companion_system.current_game.name);
 gfx->setTextSize(1); gfx->setCursor(20,50); gfx->print("Score: "); gfx->print(companion_system.current_game.score);
 int tl = max(0, 30-(int)((millis()-companion_system.current_game.start_time)/1000));
 gfx->setCursor(LCD_WIDTH-60,50); gfx->print("Time: "); gfx->print(tl);
 CompanionData* c = companion_system.current_companion;
 if (c) {
   drawCompanionSprite(companion_system.current_game.target_x, companion_system.current_game.target_y, c->type, c->stats.evolution);
   gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2);
   gfx->setCursor(companion_system.current_game.target_x-15, companion_system.current_game.target_y-40);
   gfx->print("TAP!");
 }
}

void drawCompanionAnimation() { updateCompanionAnimation(); CompanionData* c = companion_system.current_companion; if (c) drawCompanionSprite(LCD_WIDTH/2,125,c->type,c->stats.evolution); }

// Touch
void handleCompanionScreenTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 if (g.y >= LCD_HEIGHT-80 && g.x >= LCD_WIDTH/2-60 && g.x <= LCD_WIDTH/2+60) { companion_system.in_care_mode = true; return; }
 if (g.y >= LCD_HEIGHT-80 && g.x <= 110) { returnToAppGrid(); }
}

void handleCareMenuTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 int by = 70, bh = 60, bg = 15;
 if (g.y >= by && g.y <= by+bh) { feedCompanion(); return; }
 by += bh+bg; if (g.y >= by && g.y <= by+bh) { playWithCompanion(); return; }
 by += bh+bg; if (g.y >= by && g.y <= by+bh) { trainCompanion(); return; }
 by += bh+bg; if (g.y >= by && g.y <= by+bh) { toggleCompanionSleep(); return; }
 if (g.y >= LCD_HEIGHT-60 && g.x <= 110) { companion_system.in_care_mode = false; }
}

// Helpers
const char* getMoodText(CompanionMood m) { switch(m) { case MOOD_ECSTATIC: return "Ecstatic!"; case MOOD_HAPPY: return "Happy"; case MOOD_CONTENT: return "Content"; case MOOD_SAD: return "Sad"; case MOOD_MISERABLE: return "Miserable"; default: return "?"; } }
uint16_t getMoodColor(CompanionMood m) { switch(m) { case MOOD_ECSTATIC: return COLOR_GOLD; case MOOD_HAPPY: return COLOR_GREEN; case MOOD_CONTENT: return COLOR_CYAN; case MOOD_SAD: return COLOR_ORANGE; case MOOD_MISERABLE: return COLOR_RED; default: return COLOR_GRAY; } }
const char* getEvolutionText(CompanionEvolution e) { switch(e) { case EVO_BABY: return "Baby"; case EVO_CHILD: return "Child"; case EVO_ADULT: return "Adult"; case EVO_AWAKENED: return "Awakened"; default: return "?"; } }
int getStatPercentage(int s) { return constrain(s,STAT_MIN,STAT_MAX); }
bool isStatCritical(int s) { return s <= STAT_CRITICAL; }
bool isStatLow(int s) { return s <= STAT_LOW; }
