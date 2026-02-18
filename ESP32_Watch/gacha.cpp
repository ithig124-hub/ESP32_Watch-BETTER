/*
 * gacha.cpp - Gacha Collection System Implementation
 * Complete collectible card game with gem economy
 */

#include "gacha.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include <SD_MMC.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Card database - 100 cards (10 series x 10 cards each)
GachaCard gacha_cards[GACHA_TOTAL_CARDS];
bool cards_owned[GACHA_TOTAL_CARDS];
int cards_duplicates[GACHA_TOTAL_CARDS];

// Series names
const char* GACHA_SERIES_NAMES[10] = {
  "One Piece", "Solo Leveling", "Wakfu", "Naruto", "Dragon Ball",
  "Demon Slayer", "Jujutsu Kaisen", "Attack on Titan", "One Punch Man", "My Hero Academia"
};

// Character names per series (10 per series)
const char* SERIES_CHARACTERS[10][10] = {
  // One Piece
  {"Luffy", "Zoro", "Sanji", "Nami", "Robin", "Chopper", "Franky", "Brook", "Jinbe", "Shanks"},
  // Solo Leveling
  {"Jin-Woo", "Igris", "Beru", "Tusk", "Iron", "Tank", "Kaisel", "Bellion", "Go Gun-Hee", "Cha Hae-In"},
  // Wakfu
  {"Yugo", "Adamai", "Ruel", "Amalia", "Evangelyne", "Percedal", "Grougaloragran", "Qilby", "Nox", "Ogrest"},
  // Naruto
  {"Naruto", "Sasuke", "Sakura", "Kakashi", "Itachi", "Madara", "Minato", "Jiraiya", "Tsunade", "Gaara"},
  // Dragon Ball
  {"Goku", "Vegeta", "Gohan", "Piccolo", "Frieza", "Cell", "Buu", "Broly", "Beerus", "Whis"},
  // Demon Slayer
  {"Tanjiro", "Nezuko", "Zenitsu", "Inosuke", "Giyu", "Rengoku", "Shinobu", "Muzan", "Akaza", "Kokushibo"},
  // Jujutsu Kaisen
  {"Gojo", "Yuji", "Megumi", "Nobara", "Sukuna", "Todo", "Maki", "Toge", "Nanami", "Yuta"},
  // Attack on Titan
  {"Levi", "Eren", "Mikasa", "Armin", "Erwin", "Hange", "Annie", "Reiner", "Bertholdt", "Zeke"},
  // One Punch Man
  {"Saitama", "Genos", "Tatsumaki", "Fubuki", "King", "Bang", "Garou", "Boros", "Sonic", "Atomic"},
  // My Hero Academia
  {"Deku", "Bakugo", "Todoroki", "All Might", "Endeavor", "Hawks", "Aizawa", "Shigaraki", "Dabi", "Toga"}
};

// Character catchphrases
const char* SERIES_CATCHPHRASES[10][10] = {
  // One Piece
  {"I'll be King of Pirates!", "Nothing happened.", "I found it... All Blue!", "Money!", "I want to live!", 
   "DOCTOR!!", "SUPER!", "Yohohoho!", "Leave it to me!", "Let's drink!"},
  // Solo Leveling
  {"ARISE!", "My liege...", "SCREECH!", "...", "Ready to serve!", 
   "Protecting master!", "Fly!", "At your command!", "Fight well!", "You're strong..."},
  // Wakfu
  {"Adventure!", "Brother...", "Gold!", "Nature's call!", "On target!", 
   "IIIIIOP!", "Wisdom of ages", "Eliatrope secrets", "Time is Wakfu", "RAAAAWR!"},
  // Naruto
  {"Believe it!", "I'll kill Itachi!", "CHA!", "Yo.", "Forgive me, Sasuke.", 
   "I am Madara Uchiha!", "Yellow Flash!", "Research time!", "GAMBLING!", "Sand coffin!"},
  // Dragon Ball
  {"Kamehameha!", "Prince of Saiyans!", "I'm the Great Saiyaman!", "DODGE!", "I'll destroy you!", 
   "Perfect!", "BUU!", "KAKAROT!", "Before creation...", "That was fun!"},
  // Demon Slayer
  {"Total Concentration!", "Brother...", "ZENITSU!", "PIG ASSAULT!", "The water is calm.", 
   "SET YOUR HEART ABLAZE!", "Ara ara...", "I am... inevitable.", "Die weak!", "Moon Breathing..."},
  // Jujutsu Kaisen
  {"I'm the strongest.", "Give me a sec!", "Divine Dogs!", "Nails!", "Know your place.", 
   "My best friend!", "Physical strength!", "Salmon!", "Overtime again...", "Pure love!"},
  // Attack on Titan
  {"Tch.", "I'll destroy them all!", "Eren!", "Let's think...", "ADVANCE!", 
   "Science!", "Female Titan!", "Armored Titan!", "Colossal kick!", "No regrets!"},
  // One Punch Man
  {"OK.", "Incinerate!", "Don't underestimate me!", "Blizzard of Hell!", "King Engine!", 
   "Water Stream!", "Hunt!", "Meteoric Burst!", "Too slow!", "Atomic Slash!"},
  // My Hero Academia
  {"SMASH!", "DIE!", "Half-cold Half-hot!", "PLUS ULTRA!", "PROMINENCE BURN!", 
   "Hawks!", "Don't make me use it.", "Decay!", "Cremation!", "I love you!"}
};

// =============================================================================
// GACHA SYSTEM
// =============================================================================

void initGachaSystem() {
  Serial.println("[Gacha] Initializing gacha system...");
  initCardDatabase();
  
  if (!loadGachaProgress()) {
    // Initialize fresh
    system_state.player_gems = 500;  // Starting gems
    system_state.gacha_cards_collected = 0;
    
    for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
      cards_owned[i] = false;
      cards_duplicates[i] = 0;
    }
  }
}

void initCardDatabase() {
  int card_id = 0;
  
  for (int series = 0; series < 10; series++) {
    for (int c = 0; c < 10; c++) {
      // Assign rarity based on character importance (first few are rarer)
      GachaRarity rarity;
      if (c == 0) rarity = RARITY_LEGENDARY;      // Main character
      else if (c < 3) rarity = RARITY_EPIC;       // Major characters
      else if (c < 6) rarity = RARITY_RARE;       // Supporting
      else rarity = RARITY_COMMON;                // Others
      
      // Some mythic cards for special characters
      if ((series == 0 && c == 0) ||   // Luffy
          (series == 1 && c == 0) ||   // Jin-Woo
          (series == 4 && c == 0) ||   // Goku
          (series == 6 && c == 0)) {   // Gojo
        // These have a mythic variant too
      }
      
      gacha_cards[card_id] = generateCard(series, c, rarity);
      card_id++;
    }
  }
  
  Serial.printf("[Gacha] Initialized %d cards\n", card_id);
}

GachaCard generateCard(int series_index, int card_index, GachaRarity rarity) {
  GachaCard card;
  card.character_name = String(SERIES_CHARACTERS[series_index][card_index]);
  card.series = String(GACHA_SERIES_NAMES[series_index]);
  card.catchphrase = String(SERIES_CATCHPHRASES[series_index][card_index]);
  card.rarity = rarity;
  
  // Power based on rarity
  int min_power = getPowerRange(rarity, false);
  int max_power = getPowerRange(rarity, true);
  card.power_rating = random(min_power, max_power + 1);
  
  // Color based on series
  uint16_t series_colors[] = {
    LUFFY_SUN_GOLD, JINWOO_MONARCH_PURPLE, YUGO_PORTAL_CYAN, NARUTO_CHAKRA_ORANGE, GOKU_UI_SILVER,
    TANJIRO_FIRE_ORANGE, GOJO_INFINITY_BLUE, LEVI_SURVEY_GREEN, SAITAMA_HERO_YELLOW, DEKU_HERO_GREEN
  };
  card.card_color = series_colors[series_index];
  
  card.owned = false;
  card.duplicate_count = 0;
  
  return card;
}

void saveGachaProgress() {
  extern bool sdCardInitialized;
  if (!sdCardInitialized) {
    Serial.println("[Gacha] SD Card not available");
    return;
  }
  
  File dataFile = SD_MMC.open("/WATCH/gacha/cards.dat", FILE_WRITE);
  if (!dataFile) {
    Serial.println("[Gacha] Cannot save progress");
    return;
  }
  
  dataFile.printf("VERSION=1\n");
  dataFile.printf("TOTAL_COLLECTED=%d\n", system_state.gacha_cards_collected);
  
  // Save each card's owned status and duplicate count
  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    if (cards_owned[i]) {
      dataFile.printf("CARD_%d=%d\n", i, cards_duplicates[i]);
    }
  }
  
  dataFile.close();
  Serial.println("[Gacha] Progress saved to SD card");
}

bool loadGachaProgress() {
  extern bool sdCardInitialized;
  if (!sdCardInitialized) {
    return false;
  }
  
  File dataFile = SD_MMC.open("/WATCH/gacha/cards.dat", FILE_READ);
  if (!dataFile) {
    Serial.println("[Gacha] No saved progress found");
    return false;
  }
  
  // Reset all cards first
  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    cards_owned[i] = false;
    cards_duplicates[i] = 0;
  }
  
  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    line.trim();
    
    int eqPos = line.indexOf('=');
    if (eqPos < 0) continue;
    
    String key = line.substring(0, eqPos);
    int value = line.substring(eqPos + 1).toInt();
    
    if (key == "TOTAL_COLLECTED") {
      system_state.gacha_cards_collected = value;
    } else if (key.startsWith("CARD_")) {
      int cardIndex = key.substring(5).toInt();
      if (cardIndex >= 0 && cardIndex < GACHA_TOTAL_CARDS) {
        cards_owned[cardIndex] = true;
        cards_duplicates[cardIndex] = value;
      }
    }
  }
  
  dataFile.close();
  Serial.printf("[Gacha] Progress loaded: %d cards collected\n", system_state.gacha_cards_collected);
  return true;
}

// =============================================================================
// GEM MANAGEMENT
// =============================================================================

int getPlayerGems() {
  return system_state.player_gems;
}

void addGems(int amount, const char* source) {
  system_state.player_gems += amount;
  Serial.printf("[Gacha] +%d gems from %s (Total: %d)\n", amount, source, system_state.player_gems);
}

bool spendGems(int amount) {
  if (system_state.player_gems >= amount) {
    system_state.player_gems -= amount;
    return true;
  }
  return false;
}

// =============================================================================
// PULLING
// =============================================================================

bool canPullSingle() {
  return system_state.player_gems >= GACHA_SINGLE_PULL_COST;
}

bool canPullTen() {
  return system_state.player_gems >= GACHA_TEN_PULL_COST;
}

GachaRarity rollRarity() {
  int roll = random(0, 100);
  
  if (roll < 1) return RARITY_MYTHIC;          // 1%
  else if (roll < 5) return RARITY_LEGENDARY;  // 4%
  else if (roll < 20) return RARITY_EPIC;      // 15%
  else if (roll < 50) return RARITY_RARE;      // 30%
  else return RARITY_COMMON;                    // 50%
}

GachaCard performSinglePull() {
  if (!spendGems(GACHA_SINGLE_PULL_COST)) {
    GachaCard empty;
    empty.character_name = "ERROR";
    return empty;
  }
  
  GachaRarity rarity = rollRarity();
  
  // Find a card of this rarity
  int attempts = 0;
  while (attempts < 100) {
    int card_id = random(0, GACHA_TOTAL_CARDS);
    if (gacha_cards[card_id].rarity == rarity || 
        (rarity == RARITY_MYTHIC && gacha_cards[card_id].rarity == RARITY_LEGENDARY)) {
      
      GachaCard pulled = gacha_cards[card_id];
      pulled.rarity = rarity;  // Override with rolled rarity for mythic upgrade
      
      addCardToCollection(pulled);
      return pulled;
    }
    attempts++;
  }
  
  // Fallback - return random card with rolled rarity
  int card_id = random(0, GACHA_TOTAL_CARDS);
  GachaCard pulled = gacha_cards[card_id];
  pulled.rarity = rarity;
  addCardToCollection(pulled);
  return pulled;
}

void performTenPull(GachaCard* results) {
  if (!spendGems(GACHA_TEN_PULL_COST)) return;
  
  // Guaranteed at least one Epic or higher in 10-pull
  bool hasHighRarity = false;
  
  for (int i = 0; i < 10; i++) {
    GachaRarity rarity = rollRarity();
    
    // On last card, guarantee Epic+ if none yet
    if (i == 9 && !hasHighRarity && rarity < RARITY_EPIC) {
      rarity = RARITY_EPIC;
    }
    
    if (rarity >= RARITY_EPIC) hasHighRarity = true;
    
    // Find card of this rarity
    int card_id = random(0, GACHA_TOTAL_CARDS);
    results[i] = gacha_cards[card_id];
    results[i].rarity = rarity;
    
    addCardToCollection(results[i]);
  }
}

// =============================================================================
// COLLECTION
// =============================================================================

int getCardsOwned() {
  int count = 0;
  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    if (cards_owned[i]) count++;
  }
  return count;
}

int getTotalCards() {
  return GACHA_TOTAL_CARDS;
}

bool ownsCard(int card_id) {
  if (card_id < 0 || card_id >= GACHA_TOTAL_CARDS) return false;
  return cards_owned[card_id];
}

void addCardToCollection(GachaCard& card) {
  // Find this card in the database
  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    if (gacha_cards[i].character_name == card.character_name &&
        gacha_cards[i].series == card.series) {
      
      if (!cards_owned[i]) {
        cards_owned[i] = true;
        system_state.gacha_cards_collected++;
        Serial.printf("[Gacha] NEW CARD: %s (%s)\n", card.character_name.c_str(), getRarityName(card.rarity));
      } else {
        cards_duplicates[i]++;
        Serial.printf("[Gacha] Duplicate: %s (x%d)\n", card.character_name.c_str(), cards_duplicates[i]);
      }
      
      checkCollectionRewards();
      return;
    }
  }
}

GachaCard* getCard(int card_id) {
  if (card_id < 0 || card_id >= GACHA_TOTAL_CARDS) return nullptr;
  return &gacha_cards[card_id];
}

int getDuplicateCount(int card_id) {
  if (card_id < 0 || card_id >= GACHA_TOTAL_CARDS) return 0;
  return cards_duplicates[card_id];
}

void checkCollectionRewards() {
  int owned = getCardsOwned();
  
  if (owned == 25) giveCollectionReward(25);
  else if (owned == 50) giveCollectionReward(50);
  else if (owned == 75) giveCollectionReward(75);
  else if (owned == 100) giveCollectionReward(100);
}

void giveCollectionReward(int milestone) {
  switch(milestone) {
    case 25: addGems(500, "25 Card Milestone"); break;
    case 50: addGems(1000, "50 Card Milestone"); break;
    case 75: addGems(2000, "75 Card Milestone"); break;
    case 100: addGems(5000, "100 Card Completion!"); break;
  }
}

// =============================================================================
// RARITY HELPERS
// =============================================================================

uint16_t getRarityBorderColor(GachaRarity rarity) {
  switch(rarity) {
    case RARITY_COMMON:    return COLOR_WHITE;
    case RARITY_RARE:      return COLOR_BLUE;
    case RARITY_EPIC:      return COLOR_PURPLE;
    case RARITY_LEGENDARY: return COLOR_GOLD;
    case RARITY_MYTHIC:    return COLOR_PINK;  // Will animate rainbow
    default: return COLOR_WHITE;
  }
}

uint16_t getRarityGlowColor(GachaRarity rarity) {
  switch(rarity) {
    case RARITY_COMMON:    return RGB565(50, 50, 50);
    case RARITY_RARE:      return RGB565(0, 100, 200);
    case RARITY_EPIC:      return RGB565(150, 50, 200);
    case RARITY_LEGENDARY: return RGB565(255, 200, 50);
    case RARITY_MYTHIC:    return RGB565(255, 100, 200);
    default: return COLOR_GRAY;
  }
}

const char* getRarityName(GachaRarity rarity) {
  const char* names[] = {"Common", "Rare", "Epic", "Legendary", "MYTHIC"};
  return names[(int)rarity];
}

int getRarityStars(GachaRarity rarity) {
  return (int)rarity + 1;  // 1-5 stars
}

int getPowerRange(GachaRarity rarity, bool max) {
  int ranges[][2] = {
    {100, 500},     // Common
    {501, 1500},    // Rare
    {1501, 3000},   // Epic
    {3001, 7000},   // Legendary
    {7001, 9999}    // Mythic
  };
  return ranges[(int)rarity][max ? 1 : 0];
}

// =============================================================================
// GACHA UI
// =============================================================================

void drawGachaScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  // Header
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(140, 15);
  gfx->print("GACHA");
  
  // Gems display
  gfx->setTextColor(COLOR_GOLD);
  gfx->setTextSize(1);
  gfx->setCursor(20, 50);
  gfx->printf("Gems: %d", system_state.player_gems);
  
  // Collection progress
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(200, 50);
  gfx->printf("Cards: %d/%d", getCardsOwned(), getTotalCards());
  
  // Pull buttons with glass effect
  drawGlassPanel(40, 100, 280, 120);
  
  // Single Pull button
  bool canSingle = canPullSingle();
  gfx->fillRoundRect(60, 120, 240, 35, 10, canSingle ? getCurrentTheme()->primary : COLOR_GRAY);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(90, 128);
  gfx->printf("1x Pull - %d", GACHA_SINGLE_PULL_COST);
  
  // 10x Pull button
  bool canTen = canPullTen();
  gfx->fillRoundRect(60, 165, 240, 35, 10, canTen ? getCurrentTheme()->accent : COLOR_GRAY);
  gfx->setCursor(80, 173);
  gfx->printf("10x Pull - %d", GACHA_TEN_PULL_COST);
  
  // Recent pull display area
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(40, 240);
  gfx->print("Recent Pulls:");
  
  drawGlassPanel(40, 255, 280, 100);
  
  // Collection button
  drawGlassButton(40, 380, 130, 40, "Collection", false);
  
  // Back button
  drawGlassButton(190, 380, 130, 40, "Back", false);
}

void drawGachaPullAnimation(GachaCard& card) {
  // Spinning portal animation
  gfx->fillScreen(COLOR_BLACK);
  
  for (int r = 10; r < 100; r += 10) {
    gfx->drawCircle(LCD_WIDTH/2, LCD_HEIGHT/2, r, getRarityGlowColor(card.rarity));
    delay(50);
  }
  
  // Flash based on rarity
  for (int i = 0; i < (int)card.rarity + 1; i++) {
    gfx->fillScreen(getRarityGlowColor(card.rarity));
    delay(50);
    gfx->fillScreen(COLOR_BLACK);
    delay(50);
  }
}

void drawGachaReveal(GachaCard& card) {
  gfx->fillScreen(COLOR_BLACK);
  
  // Draw large card in center
  int cardW = 200, cardH = 280;
  int cardX = (LCD_WIDTH - cardW) / 2;
  int cardY = 60;
  
  drawGachaCard(cardX, cardY, cardW, cardH, card);
  
  // Tap to continue
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(120, 400);
  gfx->print("Tap to continue");
}

void drawGachaCard(int x, int y, int w, int h, GachaCard& card) {
  // Background
  gfx->fillRoundRect(x, y, w, h, 15, card.card_color);
  
  // Rarity border
  drawCardRarityBorder(x, y, w, h, card.rarity);
  
  // Character name
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 10, y + 20);
  gfx->print(card.character_name);
  
  // Series
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_BLACK);
  gfx->setCursor(x + 10, y + 50);
  gfx->print(card.series);
  
  // Power rating
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 10, y + h - 60);
  gfx->printf("PWR: %d", card.power_rating);
  
  // Stars for rarity
  gfx->setTextSize(1);
  gfx->setCursor(x + 10, y + h - 30);
  for (int i = 0; i < getRarityStars(card.rarity); i++) {
    gfx->print("*");
  }
  
  // Rarity name
  gfx->setTextColor(getRarityBorderColor(card.rarity));
  gfx->setCursor(x + w - 60, y + h - 30);
  gfx->print(getRarityName(card.rarity));
  
  // Catchphrase (if fits)
  if (h > 200) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(x + 10, y + 80);
    // Truncate catchphrase if too long
    String phrase = card.catchphrase;
    if (phrase.length() > 20) phrase = phrase.substring(0, 17) + "...";
    gfx->print(phrase);
  }
}

void drawCardRarityBorder(int x, int y, int w, int h, GachaRarity rarity) {
  uint16_t color = getRarityBorderColor(rarity);
  int thickness = (rarity >= RARITY_EPIC) ? 4 : 2;
  
  for (int i = 0; i < thickness; i++) {
    gfx->drawRoundRect(x - i, y - i, w + i*2, h + i*2, 15 + i, color);
  }
  
  // Mythic rainbow effect (simplified)
  if (rarity == RARITY_MYTHIC) {
    static uint8_t hue = 0;
    hue += 20;
    // In real implementation, would cycle through rainbow colors
    gfx->drawRoundRect(x - 4, y - 4, w + 8, h + 8, 19, COLOR_PINK);
    gfx->drawRoundRect(x - 5, y - 5, w + 10, h + 10, 20, COLOR_CYAN);
  }
}

void drawCardGlow(int x, int y, int w, int h, GachaRarity rarity) {
  if (rarity < RARITY_RARE) return;
  
  uint16_t glowColor = getRarityGlowColor(rarity);
  
  // Outer glow effect
  for (int i = 0; i < 3; i++) {
    gfx->drawRoundRect(x - 6 - i, y - 6 - i, w + 12 + i*2, h + 12 + i*2, 18 + i, glowColor);
  }
}

void drawGachaCollection() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(90, 15);
  gfx->print("COLLECTION");
  
  // Progress
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(120, 45);
  gfx->printf("%d / %d cards", getCardsOwned(), getTotalCards());
  
  // Card grid - 3x4 small cards
  int cardW = 100, cardH = 80;
  int startX = 25, startY = 70;
  int spacing = 10;
  
  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = startX + col * (cardW + spacing);
    int y = startY + row * (cardH + spacing);
    
    if (i < GACHA_TOTAL_CARDS && cards_owned[i]) {
      // Draw mini card
      gfx->fillRoundRect(x, y, cardW, cardH, 8, gacha_cards[i].card_color);
      gfx->drawRoundRect(x, y, cardW, cardH, 8, getRarityBorderColor(gacha_cards[i].rarity));
      
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(1);
      gfx->setCursor(x + 5, y + 30);
      String name = gacha_cards[i].character_name;
      if (name.length() > 10) name = name.substring(0, 8) + "..";
      gfx->print(name);
    } else {
      // Empty slot
      gfx->fillRoundRect(x, y, cardW, cardH, 8, RGB565(30, 30, 30));
      gfx->drawRoundRect(x, y, cardW, cardH, 8, COLOR_GRAY);
      gfx->setTextColor(COLOR_GRAY);
      gfx->setTextSize(2);
      gfx->setCursor(x + 40, y + 30);
      gfx->print("?");
    }
  }
  
  // Navigation
  gfx->setTextSize(1);
  gfx->setCursor(130, 400);
  gfx->print("Swipe for more");
  
  drawGlassButton(140, 420, 80, 30, "Back", false);
}

void handleGachaTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Single pull button
  if (y >= 120 && y < 155 && x >= 60 && x < 300) {
    if (canPullSingle()) {
      GachaCard card = performSinglePull();
      drawGachaPullAnimation(card);
      drawGachaReveal(card);
      delay(2000);  // Show reveal
    }
    return;
  }
  
  // 10x pull button
  if (y >= 165 && y < 200 && x >= 60 && x < 300) {
    if (canPullTen()) {
      GachaCard results[10];
      performTenPull(results);
      // Show each card briefly
      for (int i = 0; i < 10; i++) {
        drawGachaReveal(results[i]);
        delay(500);
      }
    }
    return;
  }
  
  // Collection button
  if (y >= 380 && y < 420 && x >= 40 && x < 170) {
    system_state.current_screen = SCREEN_COLLECTION;
    return;
  }
  
  // Back button
  if (y >= 380 && y < 420 && x >= 190 && x < 320) {
    system_state.current_screen = SCREEN_GAMES;
    return;
  }
}

void handleCollectionTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
    system_state.current_screen = SCREEN_GACHA;
  }
}
