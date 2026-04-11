/*
 * xp_system.h - XP & Leveling System Header
 * FUSION OS RPG Integration
 *
 * UPDATED: Added XP rewards for card selling, battles, quests, evolution
 */

#ifndef XP_SYSTEM_H
#define XP_SYSTEM_H

#include "config.h"    // FIXED: Changed from types.h - ThemeType is defined in config.h

// =============================================================================
// XP CONSTANTS
// =============================================================================

// Base XP values
#define BASE_XP_PER_LEVEL 100
#define XP_SCALING_FACTOR 1.15f

// Daily/Hourly rewards
#define XP_DAILY_LOGIN 50
#define XP_HOURLY_SHOP 10
#define XP_DAILY_GOAL_BONUS 150     // Daily goal completion bonus

// Gacha XP rewards
#define XP_GACHA_PULL 5
#define XP_GACHA_LEGENDARY 20

// =============================================================================
// XP REWARDS - Card Selling
// =============================================================================
#define XP_SELL_MYTHIC      10000   // Selling a Mythic card
#define XP_SELL_LEGENDARY   5000    // Selling a Legendary card
#define XP_SELL_OTHER       500     // Selling Epic/Rare/Common cards

// =============================================================================
// XP REWARDS - Battles
// =============================================================================
#define XP_BATTLE_WIN       100     // Winning a battle
#define XP_BATTLE_LOSE      25      // Losing a battle (participation)
#define XP_BOSS_DEFEAT      500     // Defeating a boss
#define XP_BOSS_RUSH_CLEAR  1000    // Clearing boss rush mode

// Boss tier XP rewards
#define XP_BOSS_TIER_1      100     // Tier 1 boss
#define XP_BOSS_TIER_2      250     // Tier 2 boss
#define XP_BOSS_TIER_3      500     // Tier 3 boss
#define XP_BOSS_TIER_4      750     // Tier 4 boss
#define XP_BOSS_TIER_5      1000    // Tier 5 boss (final)

// =============================================================================
// XP REWARDS - Daily Quests
// =============================================================================
#define XP_QUEST_EASY       50      // Easy quest completion
#define XP_QUEST_MEDIUM     100     // Medium quest completion
#define XP_QUEST_HARD       200     // Hard quest completion
#define XP_QUEST_ALL_DAILY  500     // Completing all daily quests

// =============================================================================
// XP REWARDS - Card Evolution
// =============================================================================
#define XP_EVOLVE_TO_LV1    100     // Evolving to EVOLVED
#define XP_EVOLVE_TO_LV2    250     // Evolving to AWAKENED
#define XP_EVOLVE_TO_LV3    500     // Evolving to TRANSCENDED

// =============================================================================
// XP REWARDS - Miscellaneous
// =============================================================================
#define XP_STEP_GOAL        100     // Reaching daily step goal
#define XP_NEW_CARD         50      // Getting a new unique card
#define XP_COLLECTION_10    200     // Every 10 cards collected milestone
#define XP_DECK_COMPLETE    300     // Completing a full 5-card deck

// =============================================================================
// GEM REWARDS - Card Selling
// =============================================================================
#define GEMS_SELL_MYTHIC    500     // Gems from selling Mythic
#define GEMS_SELL_LEGENDARY 200     // Gems from selling Legendary
#define GEMS_SELL_EPIC      50      // Gems from selling Epic
#define GEMS_SELL_RARE      20      // Gems from selling Rare
#define GEMS_SELL_COMMON    5       // Gems from selling Common

// =============================================================================
// LEVELING SYSTEM
// =============================================================================

#define MAX_TITLES_PER_CHARACTER 22
#define MAX_TITLE_LEVEL 100

// =============================================================================
// DATA STRUCTURES
// =============================================================================

struct CharacterTitle {
    const char* name;
    int level_required;
    bool unlocked;
};

struct CharacterXPData {
    ThemeType theme;              // FIXED: Use ThemeType (defined in config.h)
    int level;
    long xp;
    long xp_to_next_level;
    int equipped_title_index;
    CharacterTitle titles[MAX_TITLES_PER_CHARACTER];
};

struct XPSystemState {
    CharacterXPData* current_character;
    int total_gems;
    int last_login_day;
    int last_hourly_claim_hour;
    bool daily_step_goal_claimed;
};

extern XPSystemState xp_system;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

void initXPSystem();
void saveXPData();
void loadXPData();

// Character management
CharacterXPData* getCurrentCharacterXP();
void switchCharacter(ThemeType theme);    // FIXED: Use ThemeType

// XP operations
void gainExperience(int amount, const char* source);
void handleLevelUp(int new_level);
long calculateXPForLevel(int level);
float getXPProgress();

// Title system
const char* getEquippedTitle();
void equipTitle(int title_index);

// Daily/hourly rewards
void checkDailyLoginBonus();
void checkHourlyShopClaim();

#endif // XP_SYSTEM_H
