/*
 * xp_system.h - XP & Leveling System with Character Titles
 * FUSION OS RPG Integration
 * 
 * Features:
 * - XP gain from boss rush, steps, gacha, daily login, hourly shop
 * - Character-specific XP and levels
 * - 22 titles per character
 * - Level-based title unlocking
 * - Persistent storage (per-character XP, persistent gems)
 */

#ifndef XP_SYSTEM_H
#define XP_SYSTEM_H

#include <Arduino.h>
#include "config.h"

// =============================================================================
// XP CONFIGURATION
// =============================================================================
// Players can level up infinitely, but titles only unlock up to level 110
// Title unlocks: Level 1, 5, 10, 15, 16, 20, 25, 30, 31, 35, 40, 45, 50, 51, 
//                60, 65, 70, 71, 80, 85, 90, 95 (22 titles total)
// After level 95, players can continue leveling but no new titles unlock

#define MAX_TITLE_LEVEL          95     // Last title unlocks at level 95
#define BASE_XP_PER_LEVEL        100    // XP required for level 2
#define XP_SCALING_FACTOR        1.15   // Exponential growth per level

// XP Formula: XP_needed = BASE_XP_PER_LEVEL * (SCALING_FACTOR ^ (level - 2))
// Examples:
//   Level 1→2:   100 XP
//   Level 2→3:   115 XP
//   Level 5→6:   152 XP
//   Level 10→11: 303 XP
//   Level 20→21: 918 XP
//   Level 50→51: 10,834 XP
//   Level 95→96: 229,686 XP
//   Level 100→101: 369,830 XP

// XP Rewards
#define XP_BOSS_TIER_1           10
#define XP_BOSS_TIER_2           25
#define XP_BOSS_TIER_3           50
#define XP_BOSS_TIER_4           100
#define XP_STEPS_PER_100         1     // 1 XP per 100 steps
#define XP_DAILY_GOAL_BONUS      50    // Big bonus for hitting step goal
#define XP_GACHA_PULL            5     // XP per gacha pull
#define XP_GACHA_LEGENDARY       20    // Bonus for legendary pull
#define XP_DAILY_LOGIN           25    // Daily login bonus
#define XP_HOURLY_SHOP           10    // Hourly shop claim

// =============================================================================
// CHARACTER TITLES
// =============================================================================
#define MAX_TITLES_PER_CHARACTER 22

struct CharacterTitle {
  const char* name;
  int level_required;
  bool unlocked;
};

// =============================================================================
// CHARACTER XP DATA (PER CHARACTER)
// =============================================================================
struct CharacterXPData {
  ThemeType character;
  int level;
  long xp;
  long xp_to_next_level;
  int equipped_title_index;
  CharacterTitle titles[MAX_TITLES_PER_CHARACTER];
};

// =============================================================================
// XP SYSTEM STATE
// =============================================================================
struct XPSystemState {
  CharacterXPData* current_character;
  int total_gems;  // Persistent across all characters
  int last_login_day;
  int last_hourly_claim_hour;
  bool daily_step_goal_claimed;
};

extern XPSystemState xp_system;

// Character XP data for all characters
extern CharacterXPData luffy_xp;
extern CharacterXPData jinwoo_xp;
extern CharacterXPData yugo_xp;
extern CharacterXPData boboiboy_xp;
extern CharacterXPData gojo_xp;
extern CharacterXPData naruto_xp;
extern CharacterXPData goku_xp;
extern CharacterXPData saitama_xp;
extern CharacterXPData tanjiro_xp;
extern CharacterXPData levi_xp;
extern CharacterXPData deku_xp;

// =============================================================================
// FUNCTIONS
// =============================================================================

// Initialize XP system
void initXPSystem();

// Gain experience points
void gainExperience(int amount, const char* source);

// Calculate XP required for a level
long calculateXPForLevel(int level);

// Get current character's XP data
CharacterXPData* getCurrentCharacterXP();

// Switch character (when theme changes)
void switchCharacter(ThemeType theme);

// Get equipped title name
const char* getEquippedTitle();

// Unlock title by index
void unlockTitle(int title_index);

// Equip title
void equipTitle(int title_index);

// Check for daily login bonus
void checkDailyLoginBonus();

// Check for hourly shop claim
void checkHourlyShopClaim();

// Level up handler (called when level increases)
void handleLevelUp(int new_level);

// Save XP data to NVS
void saveXPData();

// Load XP data from NVS
void loadXPData();

// Get level-up XP bar progress (0.0 to 1.0)
float getXPProgress();

// =============================================================================
// TITLE DEFINITIONS
// =============================================================================

// Luffy Gear 5 Titles (from themes.cpp)
extern const char* luffy_title_names[MAX_TITLES_PER_CHARACTER];

// Sung Jin-Woo Titles (from themes.cpp)
extern const char* jinwoo_title_names[MAX_TITLES_PER_CHARACTER];

// Yugo - Portal Master Titles (from themes.cpp)
extern const char* yugo_title_names[MAX_TITLES_PER_CHARACTER];

// BoBoiBoy Titles
extern const char* boboiboy_title_names[MAX_TITLES_PER_CHARACTER];

// Gojo Satoru Titles
extern const char* gojo_title_names[MAX_TITLES_PER_CHARACTER];

// Naruto Uzumaki Titles
extern const char* naruto_title_names[MAX_TITLES_PER_CHARACTER];

// Son Goku Titles
extern const char* goku_title_names[MAX_TITLES_PER_CHARACTER];

// Saitama Titles
extern const char* saitama_title_names[MAX_TITLES_PER_CHARACTER];

// Tanjiro Kamado Titles
extern const char* tanjiro_title_names[MAX_TITLES_PER_CHARACTER];

// Levi Ackerman Titles
extern const char* levi_title_names[MAX_TITLES_PER_CHARACTER];

// Deku Titles
extern const char* deku_title_names[MAX_TITLES_PER_CHARACTER];

#endif // XP_SYSTEM_H
