/*
 * xp_system.cpp - XP & Leveling System Implementation
 * FUSION OS RPG Integration
 */

#include "xp_system.h"
#include "display.h"
#include <Preferences.h>

extern Arduino_CO5300 *gfx;
extern SystemState system_state;

Preferences xp_prefs;

XPSystemState xp_system = {
  .current_character = nullptr,
  .total_gems = 0,
  .last_login_day = -1,
  .last_hourly_claim_hour = -1,
  .daily_step_goal_claimed = false
};

// =============================================================================
// CHARACTER TITLE DEFINITIONS
// NOTE: Luffy, Yugo, and Jinwoo already have titles defined in themes.cpp
// We'll add titles for the remaining characters here
// =============================================================================

// Luffy Gear 5 (22 titles) - EXISTING in themes.cpp
const char* luffy_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Rubber Rookie",              // 1
  "Straw Hat Fighter",          // 5
  "Gum-Gum Kid",                // 10
  "East Blue Champion",         // 15
  "Captain of Freedom",         // 16
  "Sky Island Drifter",         // 20
  "Gear Second Surger",         // 25
  "Grand Line Veteran",         // 30
  "Gear Third Brawler",         // 31
  "Gear Fourth Juggernaut",     // 40
  "Haki Awakener",              // 45
  "Conqueror's Vessel",         // 50
  "Emissary of Laughter",       // 51
  "Toon World Trickster",       // 60
  "Nika's Wrath",               // 65
  "Joy Boy's Heir",             // 70
  "Toon God Warrior",           // 71
  "Reality Cartoonist",         // 80
  "Haki Harmonizer",            // 85
  "Boundless Luffy",            // 90
  "Liberation Deity",           // 95
  "Sun God Nika"                // 100
};

// Sung Jin-Woo (22 titles) - EXISTING in themes.cpp
const char* jinwoo_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Weakest Hunter",             // 1
  "E-Rank Grinder",             // 5
  "Dungeon Survivor",           // 10
  "System User",                // 15
  "Rank D Ascender",            // 16
  "C-Rank Climber",             // 25
  "Solo Dungeon Clearer",       // 30
  "Grave Raiser",               // 31
  "Shadow Initiate",            // 35
  "Shadow Commander",           // 40
  "B-Rank Hunter",              // 45
  "Red Gate Conqueror",         // 50
  "Rank B Executioner",         // 51
  "Dungeon Monarch",            // 60
  "Rank A Terminator",          // 65
  "Jeju Island Savior",         // 70
  "Shadow Monarch's Vessel",    // 71
  "Monarch of Shadows",         // 80
  "King of Death",              // 85
  "Ashborn's Successor",        // 90
  "Darkness Embodied",          // 95
  "Shadow Monarch"              // 100
};

// Yugo - Portal Master (22 titles) - EXISTING in themes.cpp
const char* yugo_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Young Eliatrope",            // 1
  "Curious Explorer",           // 5
  "Brotherhood Member",         // 10
  "Wakfu Student",              // 15
  "Portal Initiate",            // 16
  "Warp Jumper",                // 20
  "Sadida Ally",                // 25
  "Dimensional Walker",         // 30
  "Shushu Whisperer",           // 31
  "Wakfu Manipulator",          // 40
  "Rubilax Wielder",            // 45
  "Adamai's Brother",           // 50
  "Portal Architect",           // 51
  "Temporal Traveler",          // 60
  "Prince of Eliatropes",       // 65
  "Inglorium Navigator",        // 70
  "Dimensional Weaver",         // 71
  "Keeper of Realities",        // 80
  "Cosmic Warper",              // 85
  "Eliatrope King",             // 90
  "King of Dragons",            // 95
  "Master of Portals"           // 100
};

// BoBoiBoy (22 titles) - NEW
const char* boboiboy_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Element Rookie",             // 1
  "Power Sphere Bearer",        // 5
  "Lightning Wielder",          // 10
  "Dual Element User",          // 15
  "Triple Split Master",        // 20
  "Earth Protector",            // 25
  "Wind Rider",                 // 30
  "Fire Blazer",                // 35
  "Water Bender",               // 40
  "Leaf Guardian",              // 45
  "Light Emitter",              // 50
  "Tier 2 Awakened",            // 55
  "Thunderstorm Rager",         // 60
  "Cyclone Tornado",            // 65
  "Quake Shaker",               // 70
  "Blaze Inferno",              // 75
  "Ice Freezer",                // 80
  "Thorn Piercer",              // 85
  "Solar Eclipse",              // 90
  "Fusion Master",              // 95
  "Elemental God",              // 100
  "Kuasa Tujuh!"                // 100
};

// Gojo Satoru (22 titles)
const char* gojo_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Six Eyes Prodigy",           // 1
  "Limitless User",             // 2
  "Infinity's Wall",            // 3
  "Blue Energy",                // 4
  "Red Reversal",               // 5
  "Star Vessel Guard",          // 6
  "High School Strongest",      // 7
  "Toji's Nightmare",           // 8
  "Awakened Sorcerer",          // 9
  "Hollow Purple",              // 10
  "Infinite Void Master",       // 11
  "Special Grade Anomaly",      // 12
  "Untouchable God",            // 13
  "Shibuya's Ace",              // 14
  "Prison Realm Prisoner",      // 15
  "The Returning Legend",       // 16
  "Shinjuku Challenger",        // 17
  "Sukuna's Rival",             // 18
  "Absolute Zenith",            // 19
  "Throughout Heaven and Earth...", // 20
  "The Honored One",            // 21
  "The Strongest Sorcerer"      // 22
};

// Naruto Uzumaki (22 titles)
const char* naruto_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Nine-Tails Jinchuriki",      // 1
  "Prankster of the Leaf",      // 2
  "Team 7 Genin",               // 3
  "Worst Ninja",                // 4
  "Toad Sage Apprentice",       // 5
  "Rasenshuriken Innovator",    // 6
  "Sage of Mount Myoboku",      // 7
  "Hero of the Leaf",           // 8
  "Kage-Level Contender",       // 9
  "Kyubi Chakra Mode",          // 10
  "Kurama Mode Link",           // 11
  "Ninja Way",                  // 12
  "Allied Force Leader",        // 13
  "Six Paths Sage",             // 14
  "Asura's Reincarnation",      // 15
  "Child of Prophecy",          // 16
  "Saviour of the World",       // 17
  "Orange Hokage",              // 18
  "Baryon Mode",                // 19
  "Will Of Fire",               // 20
  "The 7th Hokage",             // 21
  "The Strongest Shinobi"       // 22
};

// Son Goku (22 titles)
const char* goku_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Low-Class Warrior",          // 1
  "Turtle School Student",      // 2
  "World Martial Artist",       // 3
  "Kaio-ken Master",            // 4
  "Legendary Super Saiyan",     // 5
  "Ascended Saiyan",            // 6
  "Super Saiyan 2",             // 7
  "Super Saiyan 3",             // 8
  "Fusion Component",           // 9
  "Super Saiyan God",           // 10
  "Super Saiyan Blue",          // 11
  "Blue Kaio-ken x20",          // 12
  "Universe 7 Ace",             // 13
  "Ultra Instinct Sign",        // 14
  "Silver-Eyed Master",         // 15
  "Mastered Ultra Instinct",    // 16
  "Giant Ki Avatar",            // 17
  "True Ultra Instinct",        // 18
  "Beyond the Gods",            // 19
  "Multiverse Apex",            // 20
  "Kakarot",                    // 21
  "Mightiest in the Heavens"    // 22
};

// Saitama (22 titles)
const char* saitama_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Salaryman",                  // 1
  "100 Pushups Daily",          // 2
  "Bald Aspirant",              // 3
  "The Bald Cape",              // 4
  "Class C Rookie",             // 5
  "Ghost of City Z",            // 6
  "Class B Leader",             // 7
  "Normal Puncher",             // 8
  "Deep Sea King Slayer",       // 9
  "Class A Hero",               // 10
  "Serious Punch",              // 11
  "Serious Table Flip",         // 12
  "Meteor Shatterer",           // 13
  "Monster Nightmare",          // 14
  "Serious Sneeze",             // 15
  "Zero Punch Paradox",         // 16
  "Limiter Breaker",            // 17
  "Absolute Strength",          // 18
  "The Bored God",              // 19
  "S-Class Secret",             // 20
  "One Punch Man",              // 21
  "Strongest in Existence"      // 22
};

// Tanjiro Kamado (22 titles)
const char* tanjiro_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Water Breathing Learner",    // 1
  "Final Selection Survivor",   // 2
  "Black Blade Wielder",        // 3
  "Drum House Victor",          // 4
  "Lower Moon Assassin",        // 5
  "Hinokami Kagura User",       // 6
  "Sun Breath Successor",       // 7
  "Constant Concentration",     // 8
  "Upper Moon Executioner",     // 9
  "Slayer Mark Awakened",       // 10
  "Sun Halo Dragon",            // 11
  "Hashira Apprentice",         // 12
  "See-Through World",          // 13
  "Selfless State Master",      // 14
  "Hanafuda Inheritor",         // 15
  "The 13th Form",              // 16
  "Muzan's Nemesis",            // 17
  "Demon King (Shadow)",        // 18
  "Dawn Bringer",               // 19
  "Last Breath",                // 20
  "Sun God's Avatar",           // 21
  "Demon Slayer"                // 22
};

// Levi Ackerman (22 titles)
const char* levi_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Underground Legend",         // 1
  "Thieving Acrobat",           // 2
  "Scout Recruit",              // 3
  "ODM Prodigy",                // 4
  "Blade Master",               // 5
  "Ackerman Instinct",          // 6
  "Spinning Whirlwind",         // 7
  "Squad Leader",               // 8
  "Female Titan's Bane",        // 9
  "Beast Titan Shredder",       // 10
  "Wall Maria Hero",            // 11
  "Zeke's Nightmare",           // 12
  "Thunder Spear Expert",       // 13
  "Unkillable Captain",         // 14
  "Last Ackerman",              // 15
  "Crimson Blade",              // 16
  "Humanity's Vengeance",       // 17
  "Fulfilled Promise",          // 18
  "Fallen Wings Legacy",        // 19
  "Last Sentinel",              // 20
  "Humanity's Strongest",       // 21
  "Greatest Soldier in History" // 22
};

// Deku (22 titles)
const char* deku_title_names[MAX_TITLES_PER_CHARACTER] = {
  "Quirkless Kid",              // 1
  "All Might's Successor",      // 2
  "One For All 5%",             // 3
  "Full Cowl Initiate",         // 4
  "Shoot Style User",           // 5
  "Stain's Equal",              // 6
  "One For All 20%",            // 7
  "Blackwhip Awakened",         // 8
  "Float Master",               // 9
  "Danger Sense Instinct",      // 10
  "Smokescreen Strategist",     // 11
  "Fa Jin Kineticist",          // 12
  "One For All 45%",            // 13
  "Dark Deku (Vigilante)",      // 14
  "Gearshift Overdrive",        // 15
  "Transmission Master",        // 16
  "Symbol of Peace",            // 17
  "One For All 100%",           // 18
  "Bane of All For One",        // 19
  "The World's Hero",           // 20
  "The Symbol of Hope",         // 21
  "The Greatest Hero"           // 22
};

// Character XP data structures
CharacterXPData luffy_xp = {THEME_LUFFY_GEAR5, 1, 0, 100, 0};
CharacterXPData jinwoo_xp = {THEME_SUNG_JINWOO, 1, 0, 100, 0};
CharacterXPData yugo_xp = {THEME_YUGO_WAKFU, 1, 0, 100, 0};
CharacterXPData boboiboy_xp = {THEME_BOBOIBOY, 1, 0, 100, 0};
CharacterXPData gojo_xp = {THEME_GOJO_INFINITY, 1, 0, 100, 0};
CharacterXPData naruto_xp = {THEME_NARUTO_SAGE, 1, 0, 100, 0};
CharacterXPData goku_xp = {THEME_GOKU_UI, 1, 0, 100, 0};
CharacterXPData saitama_xp = {THEME_SAITAMA_OPM, 1, 0, 100, 0};
CharacterXPData tanjiro_xp = {THEME_TANJIRO_SUN, 1, 0, 100, 0};
CharacterXPData levi_xp = {THEME_LEVI_STRONGEST, 1, 0, 100, 0};
CharacterXPData deku_xp = {THEME_DEKU_PLUSULTRA, 1, 0, 100, 0};

// =============================================================================
// INITIALIZE XP SYSTEM
// =============================================================================
void initXPSystem() {
  // Initialize title arrays with level requirements
  const char** title_arrays[11] = {
    luffy_title_names, jinwoo_title_names, yugo_title_names, boboiboy_title_names,
    gojo_title_names, naruto_title_names, goku_title_names,
    saitama_title_names, tanjiro_title_names, levi_title_names, deku_title_names
  };
  
  CharacterXPData* characters[11] = {
    &luffy_xp, &jinwoo_xp, &yugo_xp, &boboiboy_xp,
    &gojo_xp, &naruto_xp, &goku_xp, &saitama_xp, &tanjiro_xp, &levi_xp, &deku_xp
  };
  
  // Title level requirements (based on existing themes.cpp levels)
  int level_reqs[MAX_TITLES_PER_CHARACTER] = {
    1, 5, 10, 15, 16, 20, 25, 30, 31, 35, 40, 45, 50, 51, 
    60, 65, 70, 71, 80, 85, 90, 95
  };
  
  // Initialize all character titles
  for (int char_idx = 0; char_idx < 11; char_idx++) {
    CharacterXPData* char_data = characters[char_idx];
    for (int i = 0; i < MAX_TITLES_PER_CHARACTER; i++) {
      char_data->titles[i].name = title_arrays[char_idx][i];
      char_data->titles[i].level_required = level_reqs[i];
      char_data->titles[i].unlocked = false;
    }
  }
  
  // Load saved data
  loadXPData();
  
  // Set current character based on theme
  switchCharacter(system_state.current_theme);
  
  Serial.println("[XP] XP System initialized");
  Serial.printf("     Current: Lv%d, %ld/%ld XP, Title: %s\n",
                xp_system.current_character->level,
                xp_system.current_character->xp,
                xp_system.current_character->xp_to_next_level,
                getEquippedTitle());
}

// =============================================================================
// CALCULATE XP FOR LEVEL
// =============================================================================
long calculateXPForLevel(int level) {
  if (level <= 1) return 0;
  return (long)(BASE_XP_PER_LEVEL * pow(XP_SCALING_FACTOR, level - 2));
}

// =============================================================================
// GET CURRENT CHARACTER XP
// =============================================================================
CharacterXPData* getCurrentCharacterXP() {
  return xp_system.current_character;
}

// =============================================================================
// SWITCH CHARACTER
// =============================================================================
void switchCharacter(ThemeType theme) {
  switch (theme) {
    case THEME_LUFFY_GEAR5:       xp_system.current_character = &luffy_xp; break;
    case THEME_SUNG_JINWOO:       xp_system.current_character = &jinwoo_xp; break;
    case THEME_YUGO_WAKFU:        xp_system.current_character = &yugo_xp; break;
    case THEME_BOBOIBOY:          xp_system.current_character = &boboiboy_xp; break;
    case THEME_GOJO_INFINITY:     xp_system.current_character = &gojo_xp; break;
    case THEME_NARUTO_SAGE:       xp_system.current_character = &naruto_xp; break;
    case THEME_GOKU_UI:           xp_system.current_character = &goku_xp; break;
    case THEME_SAITAMA_OPM:       xp_system.current_character = &saitama_xp; break;
    case THEME_TANJIRO_SUN:       xp_system.current_character = &tanjiro_xp; break;
    case THEME_LEVI_STRONGEST:    xp_system.current_character = &levi_xp; break;
    case THEME_DEKU_PLUSULTRA:    xp_system.current_character = &deku_xp; break;
    default:                      xp_system.current_character = &luffy_xp; break;
  }
  
  Serial.printf("[XP] Switched to character (theme %d)\n", theme);
}

// =============================================================================
// GAIN EXPERIENCE
// =============================================================================
void gainExperience(int amount, const char* source) {
  if (!xp_system.current_character) return;
  
  CharacterXPData* char_data = xp_system.current_character;
  
  char_data->xp += amount;
  Serial.printf("[XP] +%d XP from %s (Total: %ld/%ld)\n", 
                amount, source, char_data->xp, char_data->xp_to_next_level);
  
  // Check for level up (NO LEVEL CAP - can level infinitely!)
  while (char_data->xp >= char_data->xp_to_next_level) {
    char_data->xp -= char_data->xp_to_next_level;
    char_data->level++;
    char_data->xp_to_next_level = calculateXPForLevel(char_data->level + 1);
    
    handleLevelUp(char_data->level);
  }
  
  // Update system_state for compatibility
  system_state.player_level = char_data->level;
  system_state.player_xp = char_data->xp;
  
  // Save progress
  saveXPData();
  
  // If on Character Stats screen, refresh it to show updated XP bar
  extern SystemState system_state;
  if (system_state.current_screen == SCREEN_CHARACTER_STATS) {
    extern void drawCharacterStatsScreen();
    drawCharacterStatsScreen();
  }
}

// =============================================================================
// HANDLE LEVEL UP
// =============================================================================
void handleLevelUp(int new_level) {
  Serial.printf("[XP] ★★★ LEVEL UP! → Level %d ★★★\n", new_level);
  
  CharacterXPData* char_data = xp_system.current_character;
  
  // Unlock titles based on level (only up to MAX_TITLE_LEVEL)
  if (new_level <= MAX_TITLE_LEVEL) {
    for (int i = 0; i < MAX_TITLES_PER_CHARACTER; i++) {
      if (!char_data->titles[i].unlocked && 
          new_level >= char_data->titles[i].level_required) {
        char_data->titles[i].unlocked = true;
        Serial.printf("[XP] Title unlocked: \"%s\" (Lv%d)\n",
                      char_data->titles[i].name,
                      char_data->titles[i].level_required);
      }
    }
  } else {
    // Past max title level - no new titles, but can still level up!
    Serial.printf("[XP] Level %d reached! (No new titles past Lv%d)\n", 
                  new_level, MAX_TITLE_LEVEL);
  }
  
  // TODO: Show level-up animation on screen
  // triggerAnimationBurst(1000);  // 1 second celebration burst
}

// =============================================================================
// GET EQUIPPED TITLE
// =============================================================================
const char* getEquippedTitle() {
  if (!xp_system.current_character) return "None";
  
  CharacterXPData* char_data = xp_system.current_character;
  int title_idx = char_data->equipped_title_index;
  
  if (title_idx >= 0 && title_idx < MAX_TITLES_PER_CHARACTER &&
      char_data->titles[title_idx].unlocked) {
    return char_data->titles[title_idx].name;
  }
  
  return "No Title";
}

// =============================================================================
// EQUIP TITLE
// =============================================================================
void equipTitle(int title_index) {
  if (!xp_system.current_character) return;
  
  CharacterXPData* char_data = xp_system.current_character;
  
  if (title_index >= 0 && title_index < MAX_TITLES_PER_CHARACTER &&
      char_data->titles[title_index].unlocked) {
    char_data->equipped_title_index = title_index;
    Serial.printf("[XP] Equipped title: \"%s\"\n", char_data->titles[title_index].name);
    saveXPData();
  }
}

// =============================================================================
// CHECK DAILY LOGIN BONUS
// =============================================================================
void checkDailyLoginBonus() {
  WatchTime current = getCurrentTime();
  int current_day = current.day;
  
  if (current_day != xp_system.last_login_day) {
    xp_system.last_login_day = current_day;
    gainExperience(XP_DAILY_LOGIN, "Daily Login");
    system_state.daily_login_count++;
  }
}

// =============================================================================
// CHECK HOURLY SHOP CLAIM
// =============================================================================
void checkHourlyShopClaim() {
  WatchTime current = getCurrentTime();
  int current_hour = current.hour;
  
  if (current_hour != xp_system.last_hourly_claim_hour) {
    xp_system.last_hourly_claim_hour = current_hour;
    gainExperience(XP_HOURLY_SHOP, "Hourly Shop");
  }
}

// =============================================================================
// GET XP PROGRESS (0.0 to 1.0)
// =============================================================================
float getXPProgress() {
  if (!xp_system.current_character) return 0.0f;
  
  CharacterXPData* char_data = xp_system.current_character;
  if (char_data->xp_to_next_level == 0) return 1.0f;
  
  return (float)char_data->xp / (float)char_data->xp_to_next_level;
}

// =============================================================================
// SAVE XP DATA
// =============================================================================
void saveXPData() {
  xp_prefs.begin("xp", false);
  
  // Save each character's data (11 characters now)
  CharacterXPData* chars[11] = {&luffy_xp, &jinwoo_xp, &yugo_xp, &boboiboy_xp,
                                &gojo_xp, &naruto_xp, &goku_xp, &saitama_xp, 
                                &tanjiro_xp, &levi_xp, &deku_xp};
  const char* prefixes[11] = {"lf", "jw", "yg", "bb", "gj", "nr", "gk", "st", "tj", "lv", "dk"};
  
  for (int i = 0; i < 11; i++) {
    char key[16];
    sprintf(key, "%s_lv", prefixes[i]);
    xp_prefs.putInt(key, chars[i]->level);
    
    sprintf(key, "%s_xp", prefixes[i]);
    xp_prefs.putLong(key, chars[i]->xp);
    
    sprintf(key, "%s_ttl", prefixes[i]);
    xp_prefs.putInt(key, chars[i]->equipped_title_index);
  }
  
  // Save persistent gems
  xp_prefs.putInt("gems", xp_system.total_gems);
  xp_prefs.putInt("last_day", xp_system.last_login_day);
  xp_prefs.putInt("last_hr", xp_system.last_hourly_claim_hour);
  
  xp_prefs.end();
}

// =============================================================================
// LOAD XP DATA
// =============================================================================
void loadXPData() {
  xp_prefs.begin("xp", true);
  
  // Load each character's data (11 characters now)
  CharacterXPData* chars[11] = {&luffy_xp, &jinwoo_xp, &yugo_xp, &boboiboy_xp,
                                &gojo_xp, &naruto_xp, &goku_xp, &saitama_xp,
                                &tanjiro_xp, &levi_xp, &deku_xp};
  const char* prefixes[11] = {"lf", "jw", "yg", "bb", "gj", "nr", "gk", "st", "tj", "lv", "dk"};
  
  for (int i = 0; i < 11; i++) {
    char key[16];
    sprintf(key, "%s_lv", prefixes[i]);
    chars[i]->level = xp_prefs.getInt(key, 1);
    
    sprintf(key, "%s_xp", prefixes[i]);
    chars[i]->xp = xp_prefs.getLong(key, 0);
    
    sprintf(key, "%s_ttl", prefixes[i]);
    chars[i]->equipped_title_index = xp_prefs.getInt(key, 0);  // Default to 0 (Level 1 title)
    
    // FUSION OS: Auto-equip Level 1 title if this is first boot (title still -1 or invalid)
    if (chars[i]->equipped_title_index < 0 || chars[i]->equipped_title_index >= MAX_TITLES_PER_CHARACTER) {
      chars[i]->equipped_title_index = 0;  // Force Level 1 title
      Serial.printf("[XP] Auto-equipped Level 1 title for character %d\n", i);
    }
    
    // Calculate XP to next level
    chars[i]->xp_to_next_level = calculateXPForLevel(chars[i]->level + 1);
    
    // Unlock titles based on level
    for (int j = 0; j < MAX_TITLES_PER_CHARACTER; j++) {
      if (chars[i]->level >= chars[i]->titles[j].level_required) {
        chars[i]->titles[j].unlocked = true;
      }
    }
  }
  
  // Load persistent data
  xp_system.total_gems = xp_prefs.getInt("gems", system_state.player_gems);
  xp_system.last_login_day = xp_prefs.getInt("last_day", -1);
  xp_system.last_hourly_claim_hour = xp_prefs.getInt("last_hr", -1);
  
  xp_prefs.end();
  
  // Sync with system_state
  system_state.player_gems = xp_system.total_gems;
}
