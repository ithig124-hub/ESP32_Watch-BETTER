/*
 * gacha.cpp - Gacha Collection System Implementation
 * Complete collectible card game with gem economy
 *
 * FUSION OS:
 * - XP rewards for pulls (5 XP per pull, 20 XP for legendary)
 */

#include "gacha.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"
#include "games.h"
#include "xp_system.h" // FUSION OS: XP rewards
#include <SD_MMC.h>
#include <Arduino.h>

extern Arduino_CO5300 *gfx;
extern SystemState system_state;

// Card database - 100 cards (10 series x 10 cards each)
GachaCard gacha_cards[GACHA_TOTAL_CARDS];
bool cards_owned[GACHA_TOTAL_CARDS];
int cards_duplicates[GACHA_TOTAL_CARDS];

// =============================================================================
// COLLECTION BROWSER STATE - Swipe-based card viewing
// =============================================================================
static int collection_view_index = 0; // Current card being viewed
static int collection_filter = 0; // 0=ALL, 1-5=rarity filter (by star count)
static int collection_sort = 0; // 0=Rarity, 1=Power, 2=Name, 3=Series
static int filtered_cards[GACHA_TOTAL_CARDS]; // Filtered card indices
static int filtered_count = 0; // Number of cards after filter
static bool collection_needs_rebuild = true; // Flag to rebuild filtered list

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
    system_state.player_gems = 500; // Starting gems
    system_state.gacha_cards_collected = 0;
    system_state.pity_counter = 0;
    system_state.pity_legendary_counter = 0;
    system_state.deck_size = 0;
    for (int i = 0; i < MAX_DECK_SIZE; i++) system_state.battle_deck[i] = -1;

    for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
      cards_owned[i] = false;
      cards_duplicates[i] = 0;
      gacha_cards[i].evolution_level = 0;
    }
  }
}

void initCardDatabase() {
  int card_id = 0;

  for (int series = 0; series < 10; series++) {
    for (int c = 0; c < 10; c++) {
      // Assign rarity based on character importance (first few are rarer)
      GachaRarity rarity;
      if (c == 0) rarity = RARITY_LEGENDARY; // Main character
      else if (c < 3) rarity = RARITY_EPIC; // Major characters
      else if (c < 6) rarity = RARITY_RARE; // Supporting
      else rarity = RARITY_COMMON; // Others

      // Some mythic cards for special characters
      if ((series == 0 && c == 0) || // Luffy
          (series == 1 && c == 0) || // Jin-Woo
          (series == 4 && c == 0) || // Goku
          (series == 6 && c == 0)) { // Gojo
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
  card.duplicates = 0;
  card.evolution_level = 0;
  card.power = card.power_rating; // Store base power

  // Stats based on rarity + power
  card.hp = 50 + card.power_rating / 2;
  card.attack = 10 + card.power_rating / 5;
  card.defense = 5 + card.power_rating / 8;

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

  dataFile.printf("VERSION=2\n");
  dataFile.printf("TOTAL_COLLECTED=%d\n", system_state.gacha_cards_collected);
  dataFile.printf("PITY_EPIC=%d\n", system_state.pity_counter);
  dataFile.printf("PITY_LEGEND=%d\n", system_state.pity_legendary_counter);
  dataFile.printf("DECK_SIZE=%d\n", system_state.deck_size);

  // Save deck
  for (int i = 0; i < system_state.deck_size; i++) {
    dataFile.printf("DECK_%d=%d\n", i, system_state.battle_deck[i]);
  }

  // Save each card's owned status, duplicate count, and evolution level
  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    if (cards_owned[i]) {
      dataFile.printf("CARD_%d=%d\n", i, cards_duplicates[i]);
      if (gacha_cards[i].evolution_level > 0) {
        dataFile.printf("EVO_%d=%d\n", i, gacha_cards[i].evolution_level);
      }
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
    gacha_cards[i].evolution_level = 0;
  }
  system_state.pity_counter = 0;
  system_state.pity_legendary_counter = 0;
  system_state.deck_size = 0;
  for (int i = 0; i < MAX_DECK_SIZE; i++) system_state.battle_deck[i] = -1;

  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    line.trim();

    int eqPos = line.indexOf('=');
    if (eqPos < 0) continue;

    String key = line.substring(0, eqPos);
    int value = line.substring(eqPos + 1).toInt();

    if (key == "TOTAL_COLLECTED") {
      system_state.gacha_cards_collected = value;
    } else if (key == "PITY_EPIC") {
      system_state.pity_counter = value;
    } else if (key == "PITY_LEGEND") {
      system_state.pity_legendary_counter = value;
    } else if (key == "DECK_SIZE") {
      system_state.deck_size = min(value, MAX_DECK_SIZE);
    } else if (key.startsWith("DECK_")) {
      int slot = key.substring(5).toInt();
      if (slot >= 0 && slot < MAX_DECK_SIZE) {
        system_state.battle_deck[slot] = value;
      }
    } else if (key.startsWith("EVO_")) {
      int cardIndex = key.substring(4).toInt();
      if (cardIndex >= 0 && cardIndex < GACHA_TOTAL_CARDS) {
        gacha_cards[cardIndex].evolution_level = value;
        // Recalculate power with evolution multiplier
        float mult = getEvolvePowerMult(value);
        gacha_cards[cardIndex].power_rating = (int)(gacha_cards[cardIndex].power * mult);
      }
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

// Save a pulled card to collection
void saveGachaCard(GachaCard& card) {
  int cardId = card.id;

  if (!cards_owned[cardId]) {
    // New card!
    cards_owned[cardId] = true;
    cards_duplicates[cardId] = 1;
    system_state.gacha_cards_collected++;
    Serial.printf("[Gacha] NEW CARD: %s (%s) - %d/%d collected\n",
                  card.character_name, card.series,
                  system_state.gacha_cards_collected, GACHA_TOTAL_CARDS);
  } else {
    // Duplicate
    cards_duplicates[cardId]++;
    Serial.printf("[Gacha] Duplicate: %s (x%d)\n", card.character_name, cards_duplicates[cardId]);
  }

  // Auto-save to SD card
  saveGachaProgress();
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

// Draw a 5-pointed star
void drawStar(int x, int y, int size, uint16_t color) {
  int points = 5;
  float angleStep = 360.0 / points;

  for (int i = 0; i < points; i++) {
    float angle1 = (i * angleStep - 90) * PI / 180.0;
    float angle2 = ((i + 0.5) * angleStep - 90) * PI / 180.0;
    float angle3 = ((i + 1) * angleStep - 90) * PI / 180.0;

    int x1 = x + cos(angle1) * size;
    int y1 = y + sin(angle1) * size;
    int x2 = x + cos(angle2) * (size * 0.4);
    int y2 = y + sin(angle2) * (size * 0.4);
    int x3 = x + cos(angle3) * size;
    int y3 = y + sin(angle3) * size;

    gfx->fillTriangle(x, y, x1, y1, x2, y2, color);
    gfx->fillTriangle(x, y, x2, y2, x3, y3, color);
  }
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

  GachaRarity result;
  if (roll < 1) result = RARITY_MYTHIC; // 1%
  else if (roll < 5) result = RARITY_LEGENDARY; // 4%
  else if (roll < 20) result = RARITY_EPIC; // 15%
  else if (roll < 50) result = RARITY_RARE; // 30%
  else result = RARITY_COMMON; // 50%

  // Apply pity system
  applyPitySystem(result);
  return result;
}

void applyPitySystem(GachaRarity& rarity) {
  system_state.pity_counter++;
  system_state.pity_legendary_counter++;

  // Hard pity: guaranteed Legendary at 90 pulls
  if (system_state.pity_legendary_counter >= PITY_LEGENDARY_GUARANTEE) {
    if (rarity < RARITY_LEGENDARY) {
      rarity = RARITY_LEGENDARY;
      Serial.println("[Pity] LEGENDARY PITY triggered!");
    }
  }
  // Soft pity: guaranteed Epic at 30 pulls
  else if (system_state.pity_counter >= PITY_EPIC_GUARANTEE) {
    if (rarity < RARITY_EPIC) {
      rarity = RARITY_EPIC;
      Serial.println("[Pity] EPIC PITY triggered!");
    }
  }

  // Reset counters on high rarity pull
  if (rarity >= RARITY_EPIC) {
    system_state.pity_counter = 0;
  }
  if (rarity >= RARITY_LEGENDARY) {
    system_state.pity_legendary_counter = 0;
  }
}

GachaCard performSinglePull() {
  if (!spendGems(GACHA_SINGLE_PULL_COST)) {
    GachaCard empty;
    empty.character_name = "ERROR";
    return empty;
  }

  // FUSION OS: Award XP for gacha pull
  gainExperience(XP_GACHA_PULL, "Gacha Pull");

  GachaRarity rarity = rollRarity();

  // Find a card of this rarity
  int attempts = 0;
  while (attempts < 100) {
    int card_id = random(0, GACHA_TOTAL_CARDS);
    if (gacha_cards[card_id].rarity == rarity ||
        (rarity == RARITY_MYTHIC && gacha_cards[card_id].rarity == RARITY_LEGENDARY)) {

      GachaCard pulled = gacha_cards[card_id];
      pulled.rarity = rarity; // Override with rolled rarity for mythic upgrade

      // FUSION OS: Extra XP for legendary pulls
      if (rarity >= RARITY_LEGENDARY) {
        gainExperience(XP_GACHA_LEGENDARY, "Legendary Pull!");
      }

      addCardToCollection(pulled);
      return pulled;
    }
    attempts++;
  }

  // Fallback - return random card with rolled rarity
  int card_id = random(0, GACHA_TOTAL_CARDS);
  GachaCard pulled = gacha_cards[card_id];
  pulled.rarity = rarity;

  // FUSION OS: Extra XP for legendary pulls
  if (rarity >= RARITY_LEGENDARY) {
    gainExperience(XP_GACHA_LEGENDARY, "Legendary Pull!");
  }

  addCardToCollection(pulled);
  return pulled;
}

void performTenPull(GachaCard* results) {
  if (!spendGems(GACHA_TEN_PULL_COST)) return;

  // FUSION OS: Award XP for 10-pull (10 pulls = 50 XP base)
  gainExperience(XP_GACHA_PULL * 10, "10x Gacha Pull");

  // Guaranteed at least one Epic or higher in 10-pull
  bool hasHighRarity = false;
  int legendary_count = 0;

  for (int i = 0; i < 10; i++) {
    GachaRarity rarity = rollRarity();

    // On last card, guarantee Epic+ if none yet
    if (i == 9 && !hasHighRarity && rarity < RARITY_EPIC) {
      rarity = RARITY_EPIC;
    }

    if (rarity >= RARITY_EPIC) hasHighRarity = true;
    if (rarity >= RARITY_LEGENDARY) legendary_count++;

    // Find card of this rarity
    int card_id = random(0, GACHA_TOTAL_CARDS);
    results[i] = gacha_cards[card_id];
    results[i].rarity = rarity;

    addCardToCollection(results[i]);
  }

  // FUSION OS: Bonus XP for legendary pulls in 10-pull
  if (legendary_count > 0) {
    gainExperience(XP_GACHA_LEGENDARY * legendary_count, "Legendary in 10-Pull!");
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

      collection_needs_rebuild = true; // Force collection to refresh with new card

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
    case RARITY_COMMON: return COLOR_WHITE;
    case RARITY_RARE: return COLOR_BLUE;
    case RARITY_EPIC: return COLOR_PURPLE;
    case RARITY_LEGENDARY: return COLOR_GOLD;
    case RARITY_MYTHIC: return COLOR_PINK; // Will animate rainbow
    default: return COLOR_WHITE;
  }
}

uint16_t getRarityGlowColor(GachaRarity rarity) {
  switch(rarity) {
    case RARITY_COMMON: return RGB565(50, 50, 50);
    case RARITY_RARE: return RGB565(0, 100, 200);
    case RARITY_EPIC: return RGB565(150, 50, 200);
    case RARITY_LEGENDARY: return RGB565(255, 200, 50);
    case RARITY_MYTHIC: return RGB565(255, 100, 200);
    default: return COLOR_GRAY;
  }
}

const char* getRarityName(GachaRarity rarity) {
  const char* names[] = {"Common", "Rare", "Epic", "Legendary", "MYTHIC"};
  return names[(int)rarity];
}

int getRarityStars(GachaRarity rarity) {
  return (int)rarity + 1; // 1-5 stars
}

int getPowerRange(GachaRarity rarity, bool max) {
  int ranges[][2] = {
    {100, 500},   // Common
    {501, 1500},  // Rare
    {1501, 3000}, // Epic
    {3001, 7000}, // Legendary
    {7001, 9999}  // Mythic
  };
  return ranges[(int)rarity][max ? 1 : 0];
}

// =============================================================================
// COLLECTION BROWSER - Build filtered card list for swipe navigation
// =============================================================================

void buildFilteredCardList() {
  filtered_count = 0;

  for (int i = 0; i < GACHA_TOTAL_CARDS; i++) {
    // Only show owned cards
    if (!cards_owned[i]) continue;

    // Apply rarity filter (0 = ALL, 1-5 = specific star count)
    if (collection_filter > 0) {
      int card_stars = getRarityStars(gacha_cards[i].rarity);
      if (card_stars != collection_filter) continue;
    }

    filtered_cards[filtered_count] = i;
    filtered_count++;
  }

  // Sort the filtered list
  for (int i = 0; i < filtered_count - 1; i++) {
    for (int j = i + 1; j < filtered_count; j++) {
      bool swap = false;
      int a = filtered_cards[i];
      int b = filtered_cards[j];

      switch (collection_sort) {
        case 0: // Sort by Rarity (highest first)
          swap = gacha_cards[a].rarity < gacha_cards[b].rarity;
          break;
        case 1: // Sort by Power (highest first)
          swap = gacha_cards[a].power_rating < gacha_cards[b].power_rating;
          break;
        case 2: // Sort by Name (A-Z)
          swap = gacha_cards[a].character_name > gacha_cards[b].character_name;
          break;
        case 3: // Sort by Series (A-Z)
          swap = gacha_cards[a].series > gacha_cards[b].series;
          break;
      }

      if (swap) {
        int temp = filtered_cards[i];
        filtered_cards[i] = filtered_cards[j];
        filtered_cards[j] = temp;
      }
    }
  }

  // Reset view index if out of bounds
  if (collection_view_index >= filtered_count) {
    collection_view_index = max(0, filtered_count - 1);
  }

  collection_needs_rebuild = false;
}

// =============================================================================
// GACHA UI
// =============================================================================

void drawGachaScreen() {
  // ========================================
  // RETRO ANIME GACHA SCREEN - CRT Style
  // ANTI-FLICKER: Only redraw when gems or collection changes
  // ========================================

  // ANTI-FLICKER: Track changes
  static bool needs_redraw = true;
  static int last_gems = -1;
  static int last_cards_owned = -1;

  // Check if data changed
  int current_owned = getCardsOwned();
  bool data_changed = (system_state.player_gems != last_gems) ||
                      (current_owned != last_cards_owned);

  if (!needs_redraw && !data_changed) {
    return; // Skip redraw - NO FLICKER!
  }

  // Update tracking
  needs_redraw = false;
  last_gems = system_state.player_gems;
  last_cards_owned = current_owned;

  gfx->fillScreen(RGB565(2, 2, 5));

  // CRT scan lines
  for (int y = 0; y < LCD_HEIGHT; y += 4) {
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
  }

  ThemeColors* theme = getCurrentTheme();

  // Retro header
  gfx->fillRect(0, 0, LCD_WIDTH, 48, RGB565(10, 12, 18));
  for (int x = 0; x < LCD_WIDTH; x += 8) {
    gfx->fillRect(x, 46, 6, 3, theme->primary);
  }

  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(30, 35, 50));
  gfx->setCursor(LCD_WIDTH/2 - 30 + 1, 14 + 1);
  gfx->print("GACHA");
  gfx->setTextColor(theme->primary);
  gfx->setCursor(LCD_WIDTH/2 - 30, 14);
  gfx->print("GACHA");

  // Gems display - retro card
  gfx->fillRect(20, 56, 150, 28, RGB565(12, 14, 20));
  gfx->fillRect(20, 56, 4, 4, COLOR_GOLD);
  gfx->fillRect(166, 56, 4, 4, COLOR_GOLD);
  gfx->drawRect(20, 56, 150, 28, RGB565(40, 45, 60));
  gfx->setTextColor(COLOR_GOLD);
  gfx->setTextSize(1);
  gfx->setCursor(30, 65);
  gfx->printf("GEMS: %d", system_state.player_gems);

  // Collection progress - retro card
  gfx->fillRect(190, 56, 160, 28, RGB565(12, 14, 20));
  gfx->fillRect(190, 56, 4, 4, theme->accent);
  gfx->fillRect(346, 56, 4, 4, theme->accent);
  gfx->drawRect(190, 56, 160, 28, RGB565(40, 45, 60));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(200, 65);
  gfx->printf("CARDS: %d/%d", getCardsOwned(), getTotalCards());

  // Pull panel - retro framed
  gfx->fillRect(30, 95, LCD_WIDTH - 60, 120, RGB565(12, 14, 20));
  gfx->drawRect(30, 95, LCD_WIDTH - 60, 120, RGB565(40, 45, 60));
  // Pixel corner accents
  gfx->fillRect(30, 95, 5, 5, theme->primary);
  gfx->fillRect(LCD_WIDTH - 35, 95, 5, 5, theme->primary);
  gfx->fillRect(30, 210, 5, 5, theme->primary);
  gfx->fillRect(LCD_WIDTH - 35, 210, 5, 5, theme->primary);

  // Single Pull button
  bool canSingle = canPullSingle();
  uint16_t pullColor = canSingle ? theme->primary : RGB565(40, 42, 55);
  gfx->fillRect(50, 115, LCD_WIDTH - 100, 35, pullColor);
  gfx->drawRect(50, 115, LCD_WIDTH - 100, 35, RGB565(60, 65, 80));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(80, 122);
  gfx->printf("1x Pull - %d", GACHA_SINGLE_PULL_COST);

  // 10x Pull button
  bool canTen = canPullTen();
  uint16_t pull10Color = canTen ? theme->accent : RGB565(40, 42, 55);
  gfx->fillRect(50, 165, LCD_WIDTH - 100, 35, pull10Color);
  gfx->drawRect(50, 165, LCD_WIDTH - 100, 35, RGB565(60, 65, 80));
  gfx->setCursor(70, 172);
  gfx->printf("10x Pull - %d", GACHA_TEN_PULL_COST);

  // Recent pull display
  gfx->setTextColor(RGB565(100, 110, 130));
  gfx->setTextSize(1);
  gfx->setCursor(40, 230);
  gfx->print("RECENT PULLS:");

  gfx->fillRect(30, 245, LCD_WIDTH - 60, 100, RGB565(10, 12, 18));
  gfx->drawRect(30, 245, LCD_WIDTH - 60, 100, RGB565(35, 40, 55));

  // Scan lines inside pull area
  for (int sy = 248; sy < 342; sy += 4) {
    gfx->drawFastHLine(32, sy, LCD_WIDTH - 64, RGB565(6, 6, 10));
  }

  // Collection button - retro
  gfx->fillRect(10, 365, 95, 40, RGB565(15, 18, 25));
  gfx->fillRect(10, 365, 5, 5, theme->primary);
  gfx->drawRect(10, 365, 95, 40, RGB565(40, 45, 60));
  gfx->setTextColor(RGB565(180, 185, 200));
  gfx->setTextSize(1);
  gfx->setCursor(20, 381);
  gfx->print("Collection");

  // Evolve button - retro gold
  gfx->fillRect(115, 365, 75, 40, RGB565(20, 18, 10));
  gfx->drawRect(115, 365, 75, 40, COLOR_GOLD);
  gfx->fillRect(115, 365, 4, 4, COLOR_GOLD);
  gfx->fillRect(186, 365, 4, 4, COLOR_GOLD);
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(127, 381);
  gfx->print("Evolve");

  // Deck button - retro cyan
  gfx->fillRect(200, 365, 65, 40, RGB565(10, 18, 20));
  gfx->drawRect(200, 365, 65, 40, COLOR_CYAN);
  gfx->fillRect(200, 365, 4, 4, COLOR_CYAN);
  gfx->fillRect(261, 365, 4, 4, COLOR_CYAN);
  gfx->setTextColor(COLOR_CYAN);
  gfx->setCursor(215, 381);
  gfx->print("Deck");

  // Back button - retro
  gfx->fillRect(275, 365, 75, 40, RGB565(15, 18, 25));
  gfx->drawRect(275, 365, 75, 40, RGB565(40, 45, 60));
  gfx->setTextColor(RGB565(180, 185, 200));
  gfx->setCursor(295, 381);
  gfx->print("Back");

  // Pity counter at bottom
  drawPityCounter(20, 415);

  drawSwipeIndicator();
}

// =============================================================================
// WATCHDOG-SAFE ANIMATION FUNCTIONS
// These use shorter delays and fewer frames to prevent watchdog timeouts
// =============================================================================

void drawGachaPullAnimation(GachaCard& card) {
  // Retro CRT-style pull animation - WATCHDOG SAFE
  gfx->fillScreen(RGB565(2, 2, 5));

  uint16_t glowColor = getRarityGlowColor(card.rarity);

  // Expanding pixel squares instead of circles - fewer iterations
  for (int r = 10; r < 100; r += 10) {
    gfx->drawRect(LCD_WIDTH/2 - r, LCD_HEIGHT/2 - r, r*2, r*2, glowColor);
    delay(50);
  }

  // Flash based on rarity - CRT flicker - shorter delays
  for (int i = 0; i < (int)card.rarity + 1; i++) {
    gfx->fillScreen(glowColor);
    delay(50);
    gfx->fillScreen(RGB565(2, 2, 5));
    delay(50);
  }
}

void drawGachaReveal(GachaCard& card) {
  // Retro CRT reveal
  gfx->fillScreen(RGB565(2, 2, 5));
  for (int y = 0; y < LCD_HEIGHT; y += 4) {
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
  }

  // Draw large card in center
  int cardW = 200, cardH = 280;
  int cardX = (LCD_WIDTH - cardW) / 2;
  int cardY = 60;

  drawGachaCard(cardX, cardY, cardW, cardH, card);

  // Retro tap prompt
  gfx->setTextColor(RGB565(80, 85, 100));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 50, 400);
  gfx->print("> TAP TO CONTINUE <");
}

// IMPROVED GACHA REVEAL with retro CRT animations - WATCHDOG SAFE
void drawGachaRevealImproved(GachaCard& card) {
  // Animated pixel reveal - reduced frames to prevent watchdog
  for (int frame = 0; frame < 15; frame++) {
    gfx->fillScreen(RGB565(2, 2, 5));

    // CRT scan lines
    for (int y = 0; y < LCD_HEIGHT; y += 4) {
      gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
    }

    // Particle effects - pixel style based on rarity
    for (int i = 0; i < 25; i++) {
      int px = random(0, LCD_WIDTH);
      int py = random(0, LCD_HEIGHT);
      uint16_t pcolor;

      switch(card.rarity) {
        case RARITY_MYTHIC: pcolor = (random(0, 3) == 0) ? COLOR_GOLD : COLOR_PINK; break;
        case RARITY_LEGENDARY: pcolor = COLOR_GOLD; break;
        case RARITY_EPIC: pcolor = COLOR_PURPLE; break;
        case RARITY_RARE: pcolor = COLOR_BLUE; break;
        default: pcolor = RGB565(100, 105, 120); break;
      }
      gfx->fillRect(px, py, 3, 3, pcolor);
    }

    // Card with retro pixel glow
    int cardW = 240, cardH = 320;
    int cardX = (LCD_WIDTH - cardW) / 2;
    int cardY = 80;

    // Animated glow - pixel rectangles
    uint16_t glowColor = getRarityBorderColor(card.rarity);
    for (int r = 0; r < 4; r++) {
      int offset = r + (frame % 3);
      gfx->drawRect(cardX - offset, cardY - offset,
                    cardW + offset*2, cardH + offset*2, glowColor);
    }

    // Card background - retro dark
    gfx->fillRect(cardX, cardY, cardW, cardH, RGB565(12, 14, 20));
    gfx->drawRect(cardX, cardY, cardW, cardH, glowColor);
    // Pixel corners
    gfx->fillRect(cardX, cardY, 8, 8, glowColor);
    gfx->fillRect(cardX + cardW - 8, cardY, 8, 8, glowColor);
    gfx->fillRect(cardX, cardY + cardH - 8, 8, 8, glowColor);
    gfx->fillRect(cardX + cardW - 8, cardY + cardH - 8, 8, 8, glowColor);

    // CRT scan lines on card
    for (int sy = cardY + 2; sy < cardY + cardH - 2; sy += 4) {
      gfx->drawFastHLine(cardX + 2, sy, cardW - 4, RGB565(8, 10, 16));
    }

    // Character name with glow effect
    gfx->setTextSize(2);
    gfx->setTextColor(glowColor);
    int nameLen = strlen(card.character_name.c_str()) * 12;
    gfx->setCursor(cardX + (cardW - nameLen)/2, cardY + 30);
    gfx->print(card.character_name);

    // Series
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(120, 125, 140));
    int seriesLen = strlen(card.series.c_str()) * 6;
    gfx->setCursor(cardX + (cardW - seriesLen)/2, cardY + 60);
    gfx->print(card.series);

    // Rarity stars (animated - pixel style)
    int starY = cardY + 100;
    int starCount = getRarityStars(card.rarity);
    int starSpacing = 30;
    int startX = cardX + (cardW - (starCount * starSpacing))/2;

    for (int s = 0; s < starCount; s++) {
      if (frame > s * 2) {
        drawStar(startX + s * starSpacing, starY, 12, COLOR_GOLD);
      }
    }

    // Power level
    gfx->setTextSize(4);
    gfx->setTextColor(glowColor);
    char powerStr[10];
    sprintf(powerStr, "%d", card.power_rating);
    int powerLen = strlen(powerStr) * 24;
    gfx->setCursor(cardX + (cardW - powerLen)/2, cardY + 160);
    gfx->print(powerStr);

    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(80, 85, 100));
    gfx->setCursor(cardX + cardW/2 - 18, cardY + 200);
    gfx->print("POWER");

    // Rarity name
    const char* rarityName = getRarityName(card.rarity);
    gfx->setTextSize(2);
    gfx->setTextColor(glowColor);
    int rarityLen = strlen(rarityName) * 12;
    gfx->setCursor(cardX + (cardW - rarityLen)/2, cardY + 230);
    gfx->print(rarityName);

    // Catchphrase
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(150, 155, 170));
    String phrase = card.catchphrase;
    if (phrase.length() > 30) phrase = phrase.substring(0, 27) + "...";
    int phraseLen = phrase.length() * 6;
    gfx->setCursor(cardX + (cardW - phraseLen)/2, cardY + 270);
    gfx->print(phrase);

    delay(40);
  }

  // Final state - retro blinking prompt
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(LCD_WIDTH/2 - 96, 430);
  gfx->print("> CONTINUE <");

  for (int i = 0; i < 3; i++) {
    delay(300);
    gfx->fillRect(0, 425, LCD_WIDTH, 30, RGB565(2, 2, 5));
    delay(300);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_GOLD);
    gfx->setCursor(LCD_WIDTH/2 - 96, 430);
    gfx->print("> CONTINUE <");
  }
}

void drawGachaCard(int x, int y, int w, int h, GachaCard& card) {
  // FUSION OS: FANCY GACHA CARD VISUALS

  // Rarity-based gradient background
  uint16_t rarity_gradients[5][3] = {
    // Common: Gray gradient
    {RGB565(60, 60, 70), RGB565(80, 80, 90), RGB565(100, 100, 110)},
    // Rare: Blue gradient
    {RGB565(30, 80, 150), RGB565(50, 100, 180), RGB565(70, 120, 210)},
    // Epic: Purple gradient
    {RGB565(120, 40, 180), RGB565(150, 60, 210), RGB565(180, 80, 240)},
    // Legendary: Gold gradient
    {RGB565(180, 120, 0), RGB565(220, 160, 30), RGB565(255, 200, 60)},
    // Mythic: Rainbow/Prismatic
    {RGB565(255, 50, 150), RGB565(100, 200, 255), RGB565(255, 255, 100)}
  };

  int rarity_idx = min((int)card.rarity, 4);

  // Draw gradient background
  for (int i = 0; i < h; i++) {
    float progress = (float)i / h;
    uint16_t color;
    if (progress < 0.33) {
      color = rarity_gradients[rarity_idx][0];
    } else if (progress < 0.66) {
      color = rarity_gradients[rarity_idx][1];
    } else {
      color = rarity_gradients[rarity_idx][2];
    }
    gfx->drawFastHLine(x, y + i, w, color);
  }

  // Shine effect (diagonal lines)
  for (int i = 0; i < 8; i++) {
    int shine_x = x + (i * 20) - 20;
    gfx->drawLine(shine_x, y, shine_x + 30, y + h, RGB565(255, 255, 255));
  }

  // Fancy border with glow
  uint16_t borderColor = getRarityBorderColor(card.rarity);
  gfx->drawRoundRect(x - 2, y - 2, w + 4, h + 4, 8, rarity_gradients[rarity_idx][2]);
  gfx->drawRoundRect(x, y, w, h, 8, COLOR_WHITE);
  gfx->drawRoundRect(x + 1, y + 1, w - 2, h - 2, 8, borderColor);

  // Pixel corner accents (larger)
  gfx->fillRect(x, y, 10, 10, borderColor);
  gfx->fillRect(x + w - 10, y, 10, 10, borderColor);
  gfx->fillRect(x, y + h - 10, 10, 10, borderColor);
  gfx->fillRect(x + w - 10, y + h - 10, 10, 10, borderColor);

  // Character portrait (circular area)
  int portrait_cx = x + w/2;
  int portrait_cy = y + h/3;
  int portrait_r = min(w, h) / 5;
  gfx->fillCircle(portrait_cx, portrait_cy, portrait_r, RGB565(20, 20, 30));
  gfx->drawCircle(portrait_cx, portrait_cy, portrait_r, COLOR_WHITE);
  gfx->drawCircle(portrait_cx, portrait_cy, portrait_r - 2, borderColor);

  // Character name with shadow (fancy)
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(0, 0, 0));
  gfx->setCursor(x + 11, y + h/2 + 11);
  gfx->print(card.character_name);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(x + 10, y + h/2 + 10);
  gfx->print(card.character_name);

  // Series name
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 155, 170));
  gfx->setCursor(x + 10, y + h/2 + 35);
  gfx->print(card.series);

  // Rarity stars (golden)
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(255, 220, 0));
  gfx->setCursor(x + 10, y + h/2 + 55);
  for (int i = 0; i < getRarityStars(card.rarity); i++) {
    gfx->print("★");
  }

  // Stat boxes (fancy mini boxes)
  int stat_y = y + h - 50;
  int box_w = (w - 40) / 3;
  int box_h = 35;

  // Power stat box
  gfx->fillRect(x + 10, stat_y, box_w, box_h, RGB565(20, 20, 30));
  gfx->drawRect(x + 10, stat_y, box_w, box_h, RGB565(100, 100, 120));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 170));
  gfx->setCursor(x + 12, stat_y + 5);
  gfx->print("PWR");
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 15, stat_y + 18);
  gfx->printf("%d", card.power);

  // HP stat box
  gfx->fillRect(x + 15 + box_w, stat_y, box_w, box_h, RGB565(20, 20, 30));
  gfx->drawRect(x + 15 + box_w, stat_y, box_w, box_h, RGB565(100, 100, 120));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 170));
  gfx->setCursor(x + 17 + box_w, stat_y + 5);
  gfx->print("HP");
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 17 + box_w, stat_y + 18);
  gfx->printf("%d", card.hp);

  // ATK stat box
  gfx->fillRect(x + 20 + box_w * 2, stat_y, box_w, box_h, RGB565(20, 20, 30));
  gfx->drawRect(x + 20 + box_w * 2, stat_y, box_w, box_h, RGB565(100, 100, 120));
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 150, 170));
  gfx->setCursor(x + 22 + box_w * 2, stat_y + 5);
  gfx->print("ATK");
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 22 + box_w * 2, stat_y + 18);
  gfx->printf("%d", card.attack);

  // Rarity name badge at bottom
  gfx->fillRoundRect(x + w/2 - 30, y + h - 15, 60, 12, 3, borderColor);
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(0, 0, 0));
  gfx->setCursor(x + w/2 - 20, y + h - 12);
  gfx->print(getRarityName(card.rarity));
}

void drawCardRarityBorder(int x, int y, int w, int h, GachaRarity rarity) {
  uint16_t color = getRarityBorderColor(rarity);
  int thickness = (rarity >= RARITY_EPIC) ? 4 : 2;

  // Retro pixel borders
  for (int i = 0; i < thickness; i++) {
    gfx->drawRect(x - i, y - i, w + i*2, h + i*2, color);
  }

  // Mythic pixel effect
  if (rarity == RARITY_MYTHIC) {
    gfx->drawRect(x - 4, y - 4, w + 8, h + 8, COLOR_PINK);
    gfx->drawRect(x - 5, y - 5, w + 10, h + 10, COLOR_CYAN);
    // Pixel corners
    gfx->fillRect(x - 5, y - 5, 8, 8, COLOR_GOLD);
    gfx->fillRect(x + w - 3, y - 5, 8, 8, COLOR_GOLD);
    gfx->fillRect(x - 5, y + h - 3, 8, 8, COLOR_GOLD);
    gfx->fillRect(x + w - 3, y + h - 3, 8, 8, COLOR_GOLD);
  }
}

void drawCardGlow(int x, int y, int w, int h, GachaRarity rarity) {
  if (rarity < RARITY_RARE) return;

  uint16_t glowColor = getRarityGlowColor(rarity);

  // Retro pixel glow
  for (int i = 0; i < 3; i++) {
    gfx->drawRect(x - 6 - i, y - 6 - i, w + 12 + i*2, h + 12 + i*2, glowColor);
  }
}

void drawGachaCollection() {
  // ========================================
  // IMPROVED SWIPE-BASED CARD COLLECTION UI
  // Features: Large card view, swipe navigation, filter by rarity, sort options
  // ========================================

  // Rebuild filtered list if needed
  if (collection_needs_rebuild) {
    buildFilteredCardList();
  }

  gfx->fillScreen(RGB565(2, 2, 5));

  // CRT scan lines
  for (int y = 0; y < LCD_HEIGHT; y += 4) {
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
  }

  ThemeColors* theme = getCurrentTheme();

  // === HEADER ===
  gfx->fillRect(0, 0, LCD_WIDTH, 50, RGB565(10, 12, 18));
  for (int x = 0; x < LCD_WIDTH; x += 8) {
    gfx->fillRect(x, 48, 6, 3, theme->accent);
  }

  // Title with swipe indicators
  gfx->setTextSize(2);
  gfx->setTextColor(theme->primary);
  gfx->setCursor(LCD_WIDTH/2 - 60, 8);
  gfx->print("COLLECTION");

  // Progress counter
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 155, 170));
  gfx->setCursor(LCD_WIDTH/2 - 45, 32);
  gfx->printf("%d / %d cards owned", getCardsOwned(), getTotalCards());

  // === FILTER TABS ===
  int filterY = 55;
  int tabW = 58;
  int tabSpacing = 5;
  int filterStartX = (LCD_WIDTH - (6 * tabW + 5 * tabSpacing)) / 2;

  const char* filterLabels[] = {"ALL", "*", "**", "***", "****", "*****"};
  uint16_t filterColors[] = {
    RGB565(100, 105, 120), // ALL - Gray
    RGB565(150, 150, 160), // Common - Light gray
    RGB565(30, 144, 255),  // Rare - Blue
    RGB565(160, 32, 240),  // Epic - Purple
    RGB565(255, 215, 0),   // Legendary - Gold
    RGB565(255, 100, 200)  // Mythic - Pink
  };

  for (int f = 0; f < 6; f++) {
    int fx = filterStartX + f * (tabW + tabSpacing);
    bool selected = (collection_filter == f);

    if (selected) {
      gfx->fillRect(fx, filterY, tabW, 22, RGB565(25, 28, 35));
      gfx->drawRect(fx, filterY, tabW, 22, filterColors[f]);
      gfx->fillRect(fx, filterY, 4, 4, filterColors[f]);
      gfx->fillRect(fx + tabW - 4, filterY, 4, 4, filterColors[f]);
    } else {
      gfx->fillRect(fx, filterY, tabW, 22, RGB565(12, 14, 20));
      gfx->drawRect(fx, filterY, tabW, 22, RGB565(40, 45, 60));
    }

    gfx->setTextSize(1);
    gfx->setTextColor(selected ? filterColors[f] : RGB565(80, 85, 100));
    int labelLen = strlen(filterLabels[f]) * 6;
    gfx->setCursor(fx + (tabW - labelLen) / 2, filterY + 7);
    gfx->print(filterLabels[f]);
  }

  // === MAIN CARD DISPLAY AREA ===
  if (filtered_count == 0) {
    // No cards message
    gfx->setTextSize(2);
    gfx->setTextColor(RGB565(80, 85, 100));
    gfx->setCursor(LCD_WIDTH/2 - 55, 200);
    gfx->print("NO CARDS");

    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH/2 - 70, 230);
    if (collection_filter == 0) {
      gfx->print("Pull cards from Gacha!");
    } else {
      gfx->print("No cards of this rarity");
    }
  } else {
    // Get current card
    int cardIdx = filtered_cards[collection_view_index];
    GachaCard& card = gacha_cards[cardIdx];

    // === SWIPE INDICATORS ===
    // Left arrow (if not first card)
    if (collection_view_index > 0) {
      gfx->setTextSize(3);
      gfx->setTextColor(RGB565(80, 85, 100));
      gfx->setCursor(15, 220);
      gfx->print("<");
    }

    // Right arrow (if not last card)
    if (collection_view_index < filtered_count - 1) {
      gfx->setTextSize(3);
      gfx->setTextColor(RGB565(80, 85, 100));
      gfx->setCursor(LCD_WIDTH - 35, 220);
      gfx->print(">");
    }

    // === LARGE CARD DISPLAY ===
    int cardW = 260;
    int cardH = 280;
    int cardX = (LCD_WIDTH - cardW) / 2;
    int cardY = 90;

    // Card glow based on rarity
    uint16_t glowColor = getRarityGlowColor(card.rarity);
    uint16_t borderColor = getRarityBorderColor(card.rarity);

    // Outer glow
    for (int g = 5; g > 0; g--) {
      gfx->drawRoundRect(cardX - g, cardY - g, cardW + g*2, cardH + g*2, 12, glowColor);
    }

    // Card background
    gfx->fillRoundRect(cardX, cardY, cardW, cardH, 10, RGB565(15, 18, 25));
    gfx->drawRoundRect(cardX, cardY, cardW, cardH, 10, borderColor);

    // CRT scan lines on card
    for (int sy = cardY + 2; sy < cardY + cardH - 2; sy += 4) {
      gfx->drawFastHLine(cardX + 2, sy, cardW - 4, RGB565(10, 12, 18));
    }

    // Pixel corner accents
    gfx->fillRect(cardX, cardY, 12, 12, borderColor);
    gfx->fillRect(cardX + cardW - 12, cardY, 12, 12, borderColor);
    gfx->fillRect(cardX, cardY + cardH - 12, 12, 12, borderColor);
    gfx->fillRect(cardX + cardW - 12, cardY + cardH - 12, 12, 12, borderColor);

    // === RARITY BANNER ===
    const char* rarityName = getRarityName(card.rarity);
    int bannerW = strlen(rarityName) * 12 + 20;
    int bannerX = cardX + (cardW - bannerW) / 2;
    gfx->fillRoundRect(bannerX, cardY + 10, bannerW, 24, 5, borderColor);
    gfx->setTextSize(2);
    gfx->setTextColor(RGB565(0, 0, 0));
    gfx->setCursor(bannerX + 10, cardY + 14);
    gfx->print(rarityName);

    // === CHARACTER PORTRAIT AREA ===
    int portraitY = cardY + 45;
    int portraitH = 80;
    gfx->fillRect(cardX + 20, portraitY, cardW - 40, portraitH, card.card_color);
    gfx->drawRect(cardX + 20, portraitY, cardW - 40, portraitH, RGB565(50, 55, 70));

    // CRT effect on portrait
    for (int py = portraitY + 2; py < portraitY + portraitH - 2; py += 3) {
      gfx->drawFastHLine(cardX + 22, py, cardW - 44, RGB565(0, 0, 0));
    }

    // Character initial
    gfx->setTextSize(5);
    gfx->setTextColor(COLOR_WHITE);
    char initial[2] = {card.character_name[0], 0};
    gfx->setCursor(cardX + cardW/2 - 15, portraitY + 20);
    gfx->print(initial);

    // === CHARACTER NAME ===
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE);
    int nameLen = card.character_name.length() * 12;
    gfx->setCursor(cardX + (cardW - nameLen) / 2, cardY + 140);
    gfx->print(card.character_name);

    // === SERIES NAME ===
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(120, 125, 140));
    int seriesLen = card.series.length() * 6;
    gfx->setCursor(cardX + (cardW - seriesLen) / 2, cardY + 165);
    gfx->print(card.series);

    // === RARITY STARS ===
    int starCount = getRarityStars(card.rarity);
    int starSpacing = 25;
    int starStartX = cardX + (cardW - (starCount * starSpacing)) / 2;
    for (int s = 0; s < starCount; s++) {
      drawStar(starStartX + s * starSpacing + 10, cardY + 190, 10, COLOR_GOLD);
    }

    // === POWER RATING ===
    gfx->setTextSize(4);
    gfx->setTextColor(borderColor);
    char powerStr[10];
    sprintf(powerStr, "%d", card.power_rating);
    int powerLen = strlen(powerStr) * 24;
    gfx->setCursor(cardX + (cardW - powerLen) / 2, cardY + 210);
    gfx->print(powerStr);

    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(100, 105, 120));
    gfx->setCursor(cardX + cardW/2 - 18, cardY + 250);
    gfx->print("POWER");

    // === OWNED COUNT BADGE ===
    int ownedCount = cards_duplicates[cardIdx] + 1; // +1 for original
    char ownedStr[15];
    sprintf(ownedStr, "Owned: %dx", ownedCount);
    int ownedBadgeW = strlen(ownedStr) * 6 + 16;
    gfx->fillRoundRect(cardX + cardW/2 - ownedBadgeW/2, cardY + cardH - 28, ownedBadgeW, 20, 5, RGB565(20, 25, 35));
    gfx->drawRoundRect(cardX + cardW/2 - ownedBadgeW/2, cardY + cardH - 28, ownedBadgeW, 20, 5, RGB565(60, 65, 80));
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_CYAN);
    gfx->setCursor(cardX + cardW/2 - (strlen(ownedStr) * 6) / 2, cardY + cardH - 23);
    gfx->print(ownedStr);

    // === EVOLUTION BADGE (if evolved) ===
    if (card.evolution_level > 0) {
      const char* evoName = getEvolutionName(card.evolution_level);
      uint16_t evoColor = getEvolutionColor(card.evolution_level);
      int evoBadgeW = strlen(evoName) * 6 + 12;
      gfx->fillRoundRect(cardX + 10, cardY + 40, evoBadgeW, 16, 3, evoColor);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565(0, 0, 0));
      gfx->setCursor(cardX + 16, cardY + 44);
      gfx->print(evoName);
    }
  }

  // === CARD POSITION INDICATOR ===
  if (filtered_count > 0) {
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(100, 105, 120));
    char posStr[15];
    sprintf(posStr, "< %d / %d >", collection_view_index + 1, filtered_count);
    int posLen = strlen(posStr) * 6;
    gfx->setCursor(LCD_WIDTH/2 - posLen/2, 380);
    gfx->print(posStr);
  }

  // === SORT BUTTON ===
  const char* sortLabels[] = {"RARITY", "POWER", "NAME", "SERIES"};
  char sortStr[20];
  sprintf(sortStr, "Sort: %s", sortLabels[collection_sort]);
  int sortBtnW = strlen(sortStr) * 6 + 16;

  gfx->fillRect(LCD_WIDTH/2 - sortBtnW/2, 398, sortBtnW, 24, RGB565(15, 18, 25));
  gfx->drawRect(LCD_WIDTH/2 - sortBtnW/2, 398, sortBtnW, 24, RGB565(50, 55, 70));
  gfx->fillRect(LCD_WIDTH/2 - sortBtnW/2, 398, 4, 4, theme->accent);
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(150, 155, 170));
  gfx->setCursor(LCD_WIDTH/2 - (strlen(sortStr) * 6) / 2, 406);
  gfx->print(sortStr);

  // === STATS BAR ===
  int statsY = 430;
  gfx->fillRect(10, statsY, LCD_WIDTH - 20, 30, RGB565(10, 12, 18));
  gfx->drawRect(10, statsY, LCD_WIDTH - 20, 30, RGB565(30, 35, 50));

  if (filtered_count > 0) {
    int cardIdx = filtered_cards[collection_view_index];
    GachaCard& card = gacha_cards[cardIdx];

    gfx->setTextSize(1);
    // HP
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(20, statsY + 11);
    gfx->printf("HP:%d", card.hp);

    // ATK
    gfx->setTextColor(COLOR_RED);
    gfx->setCursor(100, statsY + 11);
    gfx->printf("ATK:%d", card.attack);

    // DEF
    gfx->setTextColor(COLOR_BLUE);
    gfx->setCursor(180, statsY + 11);
    gfx->printf("DEF:%d", card.defense);

    // Evolution level
    gfx->setTextColor(getEvolutionColor(card.evolution_level));
    gfx->setCursor(260, statsY + 11);
    gfx->printf("EVO:%d", card.evolution_level);
  }

  // === BACK BUTTON ===
  gfx->fillRect(LCD_WIDTH/2 - 40, 468, 80, 28, RGB565(15, 18, 25));
  gfx->drawRect(LCD_WIDTH/2 - 40, 468, 80, 28, RGB565(40, 45, 60));
  gfx->setTextColor(RGB565(180, 185, 200));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 12, 478);
  gfx->print("Back");

  // === SWIPE HINT ===
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(50, 55, 70));
  gfx->setCursor(LCD_WIDTH/2 - 55, LCD_HEIGHT - 8);
  gfx->print("< SWIPE TO BROWSE >");
}

// Global state for gacha reveal
static bool showing_gacha_result = false;
static GachaCard last_revealed_card;

void handleGachaTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;

  int x = gesture.x, y = gesture.y;

  // TAP TO CONTINUE - Handle result screen
  if (showing_gacha_result) {
    // Save the card to collection
    saveGachaCard(last_revealed_card);
    showing_gacha_result = false;
    drawGachaScreen();
    return;
  }

  // Single pull button
  if (y >= 120 && y < 155 && x >= 60 && x < 300) {
    if (canPullSingle()) {
      GachaCard card = performSinglePull();
      last_revealed_card = card;
      drawGachaPullAnimation(card);
      drawGachaRevealImproved(card);
      showing_gacha_result = true;
    }
    return;
  }

  // 10x pull button
  if (y >= 165 && y < 200 && x >= 60 && x < 300) {
    if (canPullTen()) {
      GachaCard results[10];
      performTenPull(results);
      // Show each card with save
      for (int i = 0; i < 10; i++) {
        saveGachaCard(results[i]);
      }
      // Show last card
      last_revealed_card = results[9];
      drawGachaRevealImproved(results[9]);
      showing_gacha_result = true;
    }
    return;
  }

  // Collection button
  if (y >= 365 && y < 405 && x >= 10 && x < 105) {
    system_state.current_screen = SCREEN_COLLECTION;
    // Reset collection browser state
    collection_view_index = 0;
    collection_filter = 0;
    collection_sort = 0;
    collection_needs_rebuild = true;
    drawGachaCollection();
    return;
  }

  // Evolve button
  if (y >= 365 && y < 405 && x >= 115 && x < 190) {
    system_state.current_screen = SCREEN_CARD_EVOLUTION;
    drawCardEvolutionScreen();
    return;
  }

  // Deck button
  if (y >= 365 && y < 405 && x >= 200 && x < 265) {
    system_state.current_screen = SCREEN_DECK_BUILDER;
    drawDeckBuilderScreen();
    return;
  }

  // Back button
  if (y >= 365 && y < 405 && x >= 275 && x < 350) {
    system_state.current_screen = SCREEN_GAMES;
    drawGameMenu();
    return;
  }
}

void handleCollectionTouch(TouchGesture& gesture) {
  int x = gesture.x;
  int y = gesture.y;

  // === HANDLE SWIPE GESTURES ===
  if (gesture.event == TOUCH_SWIPE_LEFT) {
    // Next card
    if (filtered_count > 0 && collection_view_index < filtered_count - 1) {
      collection_view_index++;
      drawGachaCollection();
    }
    return;
  }

  if (gesture.event == TOUCH_SWIPE_RIGHT) {
    // Previous card
    if (filtered_count > 0 && collection_view_index > 0) {
      collection_view_index--;
      drawGachaCollection();
    }
    return;
  }

  // === HANDLE TAP GESTURES ===
  if (gesture.event != TOUCH_TAP) return;

  // Filter tabs (y: 55-77)
  if (y >= 55 && y < 77) {
    int tabW = 58;
    int tabSpacing = 5;
    int filterStartX = (LCD_WIDTH - (6 * tabW + 5 * tabSpacing)) / 2;

    for (int f = 0; f < 6; f++) {
      int fx = filterStartX + f * (tabW + tabSpacing);
      if (x >= fx && x < fx + tabW) {
        if (collection_filter != f) {
          collection_filter = f;
          collection_view_index = 0;
          collection_needs_rebuild = true;
          drawGachaCollection();
        }
        return;
      }
    }
  }

  // Sort button (y: 398-422)
  int sortBtnW = 100;
  if (y >= 398 && y < 422 && x >= LCD_WIDTH/2 - sortBtnW/2 && x < LCD_WIDTH/2 + sortBtnW/2) {
    collection_sort = (collection_sort + 1) % 4; // Cycle through sort options
    collection_needs_rebuild = true;
    drawGachaCollection();
    return;
  }

  // Left arrow tap (quick navigation)
  if (x < 60 && y >= 180 && y < 280) {
    if (filtered_count > 0 && collection_view_index > 0) {
      collection_view_index--;
      drawGachaCollection();
    }
    return;
  }

  // Right arrow tap (quick navigation)
  if (x > LCD_WIDTH - 60 && y >= 180 && y < 280) {
    if (filtered_count > 0 && collection_view_index < filtered_count - 1) {
      collection_view_index++;
      drawGachaCollection();
    }
    return;
  }

  // Card tap - visual feedback flash
  int cardX = (LCD_WIDTH - 260) / 2;
  int cardY = 90;
  if (x >= cardX && x < cardX + 260 && y >= cardY && y < cardY + 280) {
    if (filtered_count > 0) {
      int cardIdx = filtered_cards[collection_view_index];
      // Flash the card
      gfx->fillRoundRect(cardX, cardY, 260, 280, 10, getRarityBorderColor(gacha_cards[cardIdx].rarity));
      delay(50);
      drawGachaCollection();
    }
    return;
  }

  // Back button (y: 468-496)
  if (y >= 468 && y < 496 && x >= LCD_WIDTH/2 - 40 && x < LCD_WIDTH/2 + 40) {
    // Reset view state before leaving
    collection_view_index = 0;
    collection_filter = 0;
    collection_sort = 0;
    collection_needs_rebuild = true;

    system_state.current_screen = SCREEN_GACHA;
    drawGachaScreen();
    return;
  }
}

// =============================================================================
// PITY SYSTEM
// =============================================================================

void drawPityCounter(int x, int y) {
  ThemeColors* theme = getCurrentTheme();

  // Pity counter panel - retro terminal style
  gfx->fillRect(x, y, LCD_WIDTH - 40, 50, RGB565(8, 10, 14));
  gfx->drawRect(x, y, LCD_WIDTH - 40, 50, RGB565(30, 35, 50));
  gfx->fillRect(x, y, 4, 4, theme->accent);

  gfx->setTextSize(1);

  // Epic pity bar
  int epicPulls = system_state.pity_counter;
  float epicProgress = (float)epicPulls / PITY_EPIC_GUARANTEE;
  if (epicProgress > 1.0f) epicProgress = 1.0f;

  gfx->setTextColor(COLOR_PURPLE);
  gfx->setCursor(x + 6, y + 6);
  gfx->printf("EPIC PITY: %d/%d", epicPulls, PITY_EPIC_GUARANTEE);

  // Epic bar
  int barW = LCD_WIDTH - 60;
  gfx->fillRect(x + 6, y + 18, barW, 6, RGB565(15, 12, 22));
  gfx->drawRect(x + 6, y + 18, barW, 6, RGB565(30, 25, 45));
  int epicFill = (int)(barW * epicProgress);
  if (epicFill > 0) gfx->fillRect(x + 7, y + 19, epicFill - 2, 4, COLOR_PURPLE);

  // Legendary pity bar
  int legPulls = system_state.pity_legendary_counter;
  float legProgress = (float)legPulls / PITY_LEGENDARY_GUARANTEE;
  if (legProgress > 1.0f) legProgress = 1.0f;

  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(x + 6, y + 28);
  gfx->printf("LEGEND PITY: %d/%d", legPulls, PITY_LEGENDARY_GUARANTEE);

  // Legendary bar
  gfx->fillRect(x + 6, y + 40, barW, 6, RGB565(20, 18, 8));
  gfx->drawRect(x + 6, y + 40, barW, 6, RGB565(40, 35, 15));
  int legFill = (int)(barW * legProgress);
  if (legFill > 0) gfx->fillRect(x + 7, y + 41, legFill - 2, 4, COLOR_GOLD);
}

// =============================================================================
// CARD EVOLUTION
// =============================================================================

const char* getEvolutionName(int level) {
  switch (level) {
    case 0: return "BASE";
    case 1: return "EVOLVED";
    case 2: return "AWAKENED";
    case 3: return "TRANSCENDED";
    default: return "BASE";
  }
}

uint16_t getEvolutionColor(int level) {
  switch (level) {
    case 0: return RGB565(130, 135, 150);
    case 1: return RGB565(80, 200, 255);
    case 2: return COLOR_GOLD;
    case 3: return COLOR_PINK;
    default: return RGB565(130, 135, 150);
  }
}

int getEvolveCost(int currentLevel) {
  switch (currentLevel) {
    case 0: return EVOLVE_COST_LV1;
    case 1: return EVOLVE_COST_LV2;
    case 2: return EVOLVE_COST_LV3;
    default: return 999;
  }
}

float getEvolvePowerMult(int level) {
  switch (level) {
    case 1: return EVOLVE_POWER_MULT_LV1;
    case 2: return EVOLVE_POWER_MULT_LV2;
    case 3: return EVOLVE_POWER_MULT_LV3;
    default: return 1.0f;
  }
}

bool evolveCard(int cardIndex) {
  if (cardIndex < 0 || cardIndex >= GACHA_TOTAL_CARDS) return false;
  if (!cards_owned[cardIndex]) return false;
  if (gacha_cards[cardIndex].evolution_level >= 3) return false;

  int cost = getEvolveCost(gacha_cards[cardIndex].evolution_level);
  if (cards_duplicates[cardIndex] < cost) return false;

  // Consume duplicates
  cards_duplicates[cardIndex] -= cost;
  gacha_cards[cardIndex].evolution_level++;

  // Apply power boost
  float mult = getEvolvePowerMult(gacha_cards[cardIndex].evolution_level);
  gacha_cards[cardIndex].power_rating = (int)(gacha_cards[cardIndex].power * mult);
  gacha_cards[cardIndex].hp = (int)(gacha_cards[cardIndex].hp * 1.2f);
  gacha_cards[cardIndex].attack = (int)(gacha_cards[cardIndex].attack * 1.2f);
  gacha_cards[cardIndex].defense = (int)(gacha_cards[cardIndex].defense * 1.2f);

  Serial.printf("[Evolve] %s -> %s (PWR: %d)\n",
                gacha_cards[cardIndex].character_name.c_str(),
                getEvolutionName(gacha_cards[cardIndex].evolution_level),
                gacha_cards[cardIndex].power_rating);

  saveGachaProgress();
  return true;
}

// Evolution selection state
static int evo_scroll_offset = 0;
static int evo_selected_card = -1;

void drawCardEvolutionScreen() {
  gfx->fillScreen(RGB565(2, 2, 5));
  for (int y = 0; y < LCD_HEIGHT; y += 4) {
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
  }

  ThemeColors* theme = getCurrentTheme();

  // Retro header
  gfx->fillRect(0, 0, LCD_WIDTH, 48, RGB565(10, 12, 18));
  for (int x = 0; x < LCD_WIDTH; x += 8) {
    gfx->fillRect(x, 46, 6, 3, COLOR_GOLD);
  }
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(30, 35, 50));
  gfx->setCursor(LCD_WIDTH/2 - 54 + 1, 14 + 1);
  gfx->print("EVOLUTION");
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(LCD_WIDTH/2 - 54, 14);
  gfx->print("EVOLUTION");

  // Card list - show owned cards that can be evolved
  int cardY = 58;
  int shown = 0;

  for (int i = 0; i < GACHA_TOTAL_CARDS && shown < 4; i++) {
    if (!cards_owned[i]) continue;
    if (shown < evo_scroll_offset) { shown++; continue; }

    int cy = cardY + (shown - evo_scroll_offset) * 82;
    bool selected = (i == evo_selected_card);
    int evoLvl = gacha_cards[i].evolution_level;
    int dupes = cards_duplicates[i];
    int cost = getEvolveCost(evoLvl);
    bool canEvolve = (evoLvl < 3 && dupes >= cost);

    // Card row - retro
    uint16_t bg = selected ? RGB565(18, 20, 30) : RGB565(12, 14, 20);
    uint16_t border = selected ? getEvolutionColor(evoLvl) : RGB565(40, 45, 60);
    gfx->fillRect(10, cy, LCD_WIDTH - 20, 72, bg);
    gfx->drawRect(10, cy, LCD_WIDTH - 20, 72, border);
    gfx->fillRect(10, cy, 5, 5, getEvolutionColor(evoLvl));
    gfx->fillRect(LCD_WIDTH - 15, cy, 5, 5, getEvolutionColor(evoLvl));

    // Card color swatch
    gfx->fillRect(18, cy + 8, 40, 56, gacha_cards[i].card_color);
    gfx->drawRect(18, cy + 8, 40, 56, RGB565(50, 55, 70));
    // CRT on swatch
    for (int sy = cy + 9; sy < cy + 63; sy += 3) {
      gfx->drawFastHLine(19, sy, 38, RGB565(0, 0, 0));
    }
    // Initial
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    char init[2] = {gacha_cards[i].character_name[0], 0};
    gfx->setCursor(28, cy + 28);
    gfx->print(init);

    // Name + evolution level
    gfx->setTextColor(RGB565(200, 205, 220));
    gfx->setTextSize(1);
    gfx->setCursor(68, cy + 10);
    gfx->print(gacha_cards[i].character_name);

    gfx->setTextColor(getEvolutionColor(evoLvl));
    gfx->setCursor(68, cy + 24);
    gfx->printf("[%s]", getEvolutionName(evoLvl));

    // Power
    gfx->setTextColor(theme->accent);
    gfx->setCursor(68, cy + 38);
    gfx->printf("PWR: %d", gacha_cards[i].power_rating);

    // Dupes and cost
    gfx->setTextColor(RGB565(100, 105, 120));
    gfx->setCursor(68, cy + 52);
    if (evoLvl < 3) {
      gfx->printf("Dupes: %d/%d", dupes, cost);
    } else {
      gfx->print("MAX LEVEL");
    }

    // Evolve button (if possible)
    if (canEvolve) {
      gfx->fillRect(LCD_WIDTH - 85, cy + 20, 65, 30, RGB565(20, 40, 15));
      gfx->drawRect(LCD_WIDTH - 85, cy + 20, 65, 30, RGB565(0, 200, 80));
      gfx->fillRect(LCD_WIDTH - 85, cy + 20, 4, 4, RGB565(0, 200, 80));
      gfx->setTextColor(RGB565(0, 200, 80));
      gfx->setTextSize(1);
      gfx->setCursor(LCD_WIDTH - 78, cy + 31);
      gfx->print("EVOLVE");
    } else if (evoLvl >= 3) {
      gfx->fillRect(LCD_WIDTH - 85, cy + 20, 65, 30, RGB565(25, 20, 30));
      gfx->drawRect(LCD_WIDTH - 85, cy + 20, 65, 30, COLOR_PINK);
      gfx->setTextColor(COLOR_PINK);
      gfx->setTextSize(1);
      gfx->setCursor(LCD_WIDTH - 75, cy + 31);
      gfx->print("MAX");
    }

    shown++;
  }

  if (shown == 0) {
    gfx->setTextColor(RGB565(80, 85, 100));
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH/2 - 50, 200);
    gfx->print("No cards to evolve");
    gfx->setCursor(LCD_WIDTH/2 - 60, 220);
    gfx->print("Pull more from Gacha!");
  }

  // Scroll hint
  gfx->setTextColor(RGB565(50, 55, 70));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 40, 395);
  gfx->print("> SWIPE <");

  // Back button
  gfx->fillRect(LCD_WIDTH/2 - 35, 415, 70, 28, RGB565(15, 18, 25));
  gfx->drawRect(LCD_WIDTH/2 - 35, 415, 70, 28, RGB565(40, 45, 60));
  gfx->setTextColor(RGB565(180, 185, 200));
  gfx->setCursor(LCD_WIDTH/2 - 12, 424);
  gfx->print("Back");
}

void handleCardEvolutionTap(int x, int y) {
  // Check evolve buttons
  int cardY = 58;
  int shown = 0;

  for (int i = 0; i < GACHA_TOTAL_CARDS && shown < 4; i++) {
    if (!cards_owned[i]) continue;
    if (shown < evo_scroll_offset) { shown++; continue; }

    int cy = cardY + (shown - evo_scroll_offset) * 82;

    // Evolve button hit
    if (x >= LCD_WIDTH - 85 && x < LCD_WIDTH - 20 && y >= cy + 20 && y < cy + 50) {
      if (gacha_cards[i].evolution_level < 3) {
        if (evolveCard(i)) {
          // Flash animation
          gfx->fillScreen(getEvolutionColor(gacha_cards[i].evolution_level));
          delay(150);
          gfx->fillScreen(RGB565(2, 2, 5));
          delay(100);
        }
      }
      drawCardEvolutionScreen();
      return;
    }

    shown++;
  }

  // Back button
  if (y >= 415 && y < 443 && x >= LCD_WIDTH/2 - 35 && x < LCD_WIDTH/2 + 35) {
    system_state.current_screen = SCREEN_GACHA;
    drawGachaScreen();
    return;
  }
}

// =============================================================================
// BATTLE DECK BUILDER
// =============================================================================

static int deck_scroll_offset = 0;

bool addCardToDeck(int cardIndex) {
  if (cardIndex < 0 || cardIndex >= GACHA_TOTAL_CARDS) return false;
  if (!cards_owned[cardIndex]) return false;
  if (system_state.deck_size >= MAX_DECK_SIZE) return false;

  // Check if already in deck
  for (int i = 0; i < system_state.deck_size; i++) {
    if (system_state.battle_deck[i] == cardIndex) return false;
  }

  system_state.battle_deck[system_state.deck_size] = cardIndex;
  system_state.deck_size++;
  Serial.printf("[Deck] Added %s to deck (slot %d)\n",
                gacha_cards[cardIndex].character_name.c_str(), system_state.deck_size);
  return true;
}

bool removeCardFromDeck(int slotIndex) {
  if (slotIndex < 0 || slotIndex >= system_state.deck_size) return false;

  // Shift cards down
  for (int i = slotIndex; i < system_state.deck_size - 1; i++) {
    system_state.battle_deck[i] = system_state.battle_deck[i + 1];
  }
  system_state.deck_size--;
  system_state.battle_deck[system_state.deck_size] = -1;
  return true;
}

int getDeckTotalPower() {
  int total = 0;
  for (int i = 0; i < system_state.deck_size; i++) {
    int idx = system_state.battle_deck[i];
    if (idx >= 0 && idx < GACHA_TOTAL_CARDS && cards_owned[idx]) {
      total += gacha_cards[idx].power_rating;
    }
  }
  return total;
}

int getDeckBonusATK() {
  int bonus = 0;
  for (int i = 0; i < system_state.deck_size; i++) {
    int idx = system_state.battle_deck[i];
    if (idx >= 0 && idx < GACHA_TOTAL_CARDS && cards_owned[idx]) {
      bonus += DECK_ATK_BONUS_PER_CARD + gacha_cards[idx].attack / 10;
    }
  }
  return bonus;
}

int getDeckBonusHP() {
  int bonus = 0;
  for (int i = 0; i < system_state.deck_size; i++) {
    int idx = system_state.battle_deck[i];
    if (idx >= 0 && idx < GACHA_TOTAL_CARDS && cards_owned[idx]) {
      bonus += DECK_HP_BONUS_PER_CARD + gacha_cards[idx].hp / 10;
    }
  }
  return bonus;
}

int getDeckBonusDEF() {
  int bonus = 0;
  for (int i = 0; i < system_state.deck_size; i++) {
    int idx = system_state.battle_deck[i];
    if (idx >= 0 && idx < GACHA_TOTAL_CARDS && cards_owned[idx]) {
      bonus += DECK_DEF_BONUS_PER_CARD + gacha_cards[idx].defense / 10;
    }
  }
  return bonus;
}

void drawDeckBuilderScreen() {
  gfx->fillScreen(RGB565(2, 2, 5));
  for (int y = 0; y < LCD_HEIGHT; y += 4) {
    gfx->drawFastHLine(0, y, LCD_WIDTH, RGB565(4, 4, 7));
  }

  ThemeColors* theme = getCurrentTheme();

  // Retro header
  gfx->fillRect(0, 0, LCD_WIDTH, 48, RGB565(10, 12, 18));
  for (int x = 0; x < LCD_WIDTH; x += 8) {
    gfx->fillRect(x, 46, 6, 3, COLOR_CYAN);
  }
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565(30, 35, 50));
  gfx->setCursor(LCD_WIDTH/2 - 66 + 1, 14 + 1);
  gfx->print("BATTLE DECK");
  gfx->setTextColor(COLOR_CYAN);
  gfx->setCursor(LCD_WIDTH/2 - 66, 14);
  gfx->print("BATTLE DECK");

  // === CURRENT DECK (top section) ===
  gfx->setTextColor(RGB565(100, 105, 120));
  gfx->setTextSize(1);
  gfx->setCursor(15, 56);
  gfx->printf("YOUR DECK (%d/%d)", system_state.deck_size, MAX_DECK_SIZE);

  // Deck slots - 5 pixel card slots
  int slotW = 60, slotH = 80;
  int slotSpacing = 5;
  int slotStartX = (LCD_WIDTH - (MAX_DECK_SIZE * slotW + (MAX_DECK_SIZE - 1) * slotSpacing)) / 2;
  int slotY = 70;

  for (int i = 0; i < MAX_DECK_SIZE; i++) {
    int sx = slotStartX + i * (slotW + slotSpacing);

    if (i < system_state.deck_size && system_state.battle_deck[i] >= 0) {
      int cardIdx = system_state.battle_deck[i];
      // Filled slot - retro card
      gfx->fillRect(sx, slotY, slotW, slotH, gacha_cards[cardIdx].card_color);
      gfx->drawRect(sx, slotY, slotW, slotH, getEvolutionColor(gacha_cards[cardIdx].evolution_level));
      // CRT lines
      for (int sy = slotY + 1; sy < slotY + slotH - 1; sy += 3) {
        gfx->drawFastHLine(sx + 1, sy, slotW - 2, RGB565(0, 0, 0));
      }
      // Initial
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(2);
      char init[2] = {gacha_cards[cardIdx].character_name[0], 0};
      gfx->setCursor(sx + slotW/2 - 6, slotY + 10);
      gfx->print(init);
      // Power
      gfx->setTextSize(1);
      gfx->setTextColor(COLOR_GOLD);
      gfx->setCursor(sx + 5, slotY + 40);
      gfx->printf("%d", gacha_cards[cardIdx].power_rating);
      // Evolution badge
      gfx->setTextColor(getEvolutionColor(gacha_cards[cardIdx].evolution_level));
      gfx->setCursor(sx + 5, slotY + 55);
      gfx->print(getEvolutionName(gacha_cards[cardIdx].evolution_level));
      // Remove X
      gfx->fillRect(sx + slotW - 14, slotY + 2, 12, 12, RGB565(60, 15, 15));
      gfx->drawRect(sx + slotW - 14, slotY + 2, 12, 12, COLOR_RED);
      gfx->setTextColor(COLOR_RED);
      gfx->setCursor(sx + slotW - 12, slotY + 4);
      gfx->print("X");
    } else {
      // Empty slot
      gfx->fillRect(sx, slotY, slotW, slotH, RGB565(8, 10, 14));
      gfx->drawRect(sx, slotY, slotW, slotH, RGB565(30, 35, 50));
      gfx->setTextColor(RGB565(40, 45, 60));
      gfx->setTextSize(2);
      gfx->setCursor(sx + slotW/2 - 6, slotY + slotH/2 - 8);
      gfx->print("+");
    }
  }

  // === DECK STATS ===
  int statsY = slotY + slotH + 8;
  gfx->fillRect(15, statsY, LCD_WIDTH - 30, 40, RGB565(10, 12, 18));
  gfx->drawRect(15, statsY, LCD_WIDTH - 30, 40, RGB565(30, 35, 50));
  gfx->fillRect(15, statsY, 4, 4, COLOR_CYAN);

  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(25, statsY + 6);
  gfx->printf("TOTAL PWR: %d", getDeckTotalPower());

  gfx->setTextColor(COLOR_RED);
  gfx->setCursor(25, statsY + 22);
  gfx->printf("+%d ATK", getDeckBonusATK());
  gfx->setTextColor(COLOR_GREEN);
  gfx->setCursor(120, statsY + 22);
  gfx->printf("+%d HP", getDeckBonusHP());
  gfx->setTextColor(COLOR_BLUE);
  gfx->setCursor(210, statsY + 22);
  gfx->printf("+%d DEF", getDeckBonusDEF());

  // === AVAILABLE CARDS (bottom section) ===
  int listY = statsY + 50;
  gfx->setTextColor(RGB565(100, 105, 120));
  gfx->setCursor(15, listY);
  gfx->print("AVAILABLE CARDS:");
  listY += 14;

  int shown = 0;
  for (int i = 0; i < GACHA_TOTAL_CARDS && shown < 3; i++) {
    if (!cards_owned[i]) continue;

    // Skip if already in deck
    bool inDeck = false;
    for (int d = 0; d < system_state.deck_size; d++) {
      if (system_state.battle_deck[d] == i) { inDeck = true; break; }
    }
    if (inDeck) continue;

    if (shown < deck_scroll_offset) { shown++; continue; }

    int cy = listY + (shown - deck_scroll_offset) * 50;

    // Card row
    gfx->fillRect(15, cy, LCD_WIDTH - 30, 42, RGB565(12, 14, 20));
    gfx->drawRect(15, cy, LCD_WIDTH - 30, 42, RGB565(40, 45, 60));
    gfx->fillRect(15, cy, 4, 4, gacha_cards[i].card_color);

    // Mini color swatch
    gfx->fillRect(22, cy + 5, 30, 32, gacha_cards[i].card_color);
    for (int sy = cy + 6; sy < cy + 36; sy += 3) {
      gfx->drawFastHLine(23, sy, 28, RGB565(0, 0, 0));
    }

    // Info
    gfx->setTextColor(RGB565(200, 205, 220));
    gfx->setTextSize(1);
    gfx->setCursor(60, cy + 8);
    gfx->print(gacha_cards[i].character_name);

    gfx->setTextColor(getEvolutionColor(gacha_cards[i].evolution_level));
    gfx->setCursor(60, cy + 22);
    gfx->printf("[%s] PWR:%d", getEvolutionName(gacha_cards[i].evolution_level),
                gacha_cards[i].power_rating);

    // Add button
    if (system_state.deck_size < MAX_DECK_SIZE) {
      gfx->fillRect(LCD_WIDTH - 70, cy + 8, 45, 26, RGB565(10, 25, 20));
      gfx->drawRect(LCD_WIDTH - 70, cy + 8, 45, 26, COLOR_CYAN);
      gfx->fillRect(LCD_WIDTH - 70, cy + 8, 3, 3, COLOR_CYAN);
      gfx->setTextColor(COLOR_CYAN);
      gfx->setCursor(LCD_WIDTH - 62, cy + 17);
      gfx->print("ADD");
    }

    shown++;
  }

  if (shown == 0) {
    gfx->setTextColor(RGB565(80, 85, 100));
    gfx->setCursor(LCD_WIDTH/2 - 50, listY + 30);
    gfx->print("No more cards available");
  }

  // Back button
  gfx->fillRect(LCD_WIDTH/2 - 35, 430, 70, 28, RGB565(15, 18, 25));
  gfx->drawRect(LCD_WIDTH/2 - 35, 430, 70, 28, RGB565(40, 45, 60));
  gfx->setTextColor(RGB565(180, 185, 200));
  gfx->setTextSize(1);
  gfx->setCursor(LCD_WIDTH/2 - 12, 439);
  gfx->print("Back");
}

void handleDeckBuilderTap(int x, int y) {
  // Check deck slot remove buttons (X buttons)
  int slotW = 60, slotH = 80;
  int slotSpacing = 5;
  int slotStartX = (LCD_WIDTH - (MAX_DECK_SIZE * slotW + (MAX_DECK_SIZE - 1) * slotSpacing)) / 2;
  int slotY = 70;

  for (int i = 0; i < system_state.deck_size; i++) {
    int sx = slotStartX + i * (slotW + slotSpacing);
    // X button: top-right of slot
    if (x >= sx + slotW - 14 && x < sx + slotW && y >= slotY && y < slotY + 16) {
      removeCardFromDeck(i);
      drawDeckBuilderScreen();
      return;
    }
  }

  // Check add buttons in available cards list
  int statsY = slotY + slotH + 8 + 40 + 50;
  int listY = statsY + 14;

  int shown = 0;
  for (int i = 0; i < GACHA_TOTAL_CARDS && shown < 3; i++) {
    if (!cards_owned[i]) continue;

    bool inDeck = false;
    for (int d = 0; d < system_state.deck_size; d++) {
      if (system_state.battle_deck[d] == i) { inDeck = true; break; }
    }
    if (inDeck) continue;

    if (shown < deck_scroll_offset) { shown++; continue; }

    int cy = listY + (shown - deck_scroll_offset) * 50;

    // Add button hit
    if (x >= LCD_WIDTH - 70 && x < LCD_WIDTH - 25 && y >= cy + 8 && y < cy + 34) {
      if (addCardToDeck(i)) {
        drawDeckBuilderScreen();
        return;
      }
    }

    shown++;
  }

  // Back button
  if (y >= 430 && y < 458 && x >= LCD_WIDTH/2 - 35 && x < LCD_WIDTH/2 + 35) {
    system_state.current_screen = SCREEN_GACHA;
    drawGachaScreen();
    return;
  }
}
