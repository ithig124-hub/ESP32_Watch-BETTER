/*
 * storyline.cpp - Story Mode with NVS + Full Dialogue + Boss Sprites
 * FUSION OS - Complete Story Implementation
 */

#include "storyline.h"
#include "config.h"
#include "themes.h"
#include "xp_system.h"
#include "gacha.h"
#include "display.h"
#include "touch.h"
#include "navigation.h"  

StorySystemState story_system;
CharacterStory stories[THEME_COUNT];

static const int CHAPTER_LEVELS[MAX_CHAPTERS_PER_CHARACTER] = {
 CHAPTER_1_LEVEL, CHAPTER_2_LEVEL, CHAPTER_3_LEVEL, CHAPTER_4_LEVEL, CHAPTER_5_LEVEL,
 CHAPTER_6_LEVEL, CHAPTER_7_LEVEL, CHAPTER_8_LEVEL, CHAPTER_9_LEVEL, CHAPTER_10_LEVEL
};

static const char* STORY_NAMES[THEME_COUNT] = {
 "Road to Pirate King", "Rise of the Shadow Monarch", "Quest for the Eliacube",
 "Legend of the Seventh", "Path to Ultra Instinct", "Sun Breathing Legacy",
 "The Strongest Sorcerer", "Wings of Freedom", "Hero for Fun",
 "Symbol of Peace", "Elemental Hero Rising"
};

static const char* CHAPTER_TITLES[THEME_COUNT][MAX_CHAPTERS_PER_CHARACTER] = {
 {"Romance Dawn", "Orange Town Circus", "Syrup Village", "Baratie Showdown", "Arlong Park", "Alabasta Rising", "Enies Lobby", "Dressrosa Liberation", "Whole Cake Escape", "Wano: Gear 5 Dawn"},
 {"Double Dungeon", "Instance Dungeon", "Penalty Quest", "Red Gate", "Demon Castle", "Return to Jeju", "Architect's Design", "National Level", "Monarchs War", "Final Battle: Antares"},
 {"The Boy with the Hat", "Nox's Shadow", "Brotherhood of the Tofu", "The Dragon's Call", "Nox Awakens", "Qilby's Betrayal", "Realm of Shushu", "Count's Ice", "Ogrest's Tears", "Oropo's Mirror"},
 {"Leaf Village Outcast", "Land of Waves", "Chunin Exams", "Sasuke Retrieval", "Sage Training", "Pain's Assault", "Fourth Great War", "Madara Rises", "Kaguya Awakens", "The Last Battle"},
 {"Arrival of Raditz", "Namek Saga", "Frieza's Wrath", "Cell Games", "Majin Buu Arc", "Battle of Gods", "Universe 6 Tournament", "Tournament of Power", "Broly Encounter", "Ultra Instinct Mastered"},
 {"Final Selection", "First Mission", "Asakusa Arc", "Mt. Natagumo", "Mugen Train", "Entertainment District", "Swordsmith Village", "Hashira Training", "Infinity Castle", "Sunrise Countdown"},
 {"Cursed Womb", "Kyoto Goodwill", "Vs. Mahito", "Shibuya Incident", "Hidden Inventory", "Perfect Preparation", "Culling Game", "Colony Assault", "Bath Ritual", "Shinjuku Showdown"},
 {"104th Cadet Corps", "Trost District", "Female Titan", "Clash of Titans", "Uprising Arc", "Return to Shiganshina", "Marley Arc", "War for Paradis", "Rumbling", "Final Stand"},
 {"Bald Cape", "House of Evolution", "Sea King Crisis", "Boros Ship", "Garou Intro", "Monster Association", "Hero Hunt", "Monster Garou", "Cosmic Fear", "Limiter Theory"},
 {"Entrance Exam", "USJ Incident", "Sports Festival", "Hero Killer", "Training Camp", "Overhaul Raid", "Joint Training", "Paranormal Liberation", "Dark Hero Arc", "Final War"},
 {"Power Sphera Awakening", "Adu Du's Attack", "BoBoiBoy Split", "Ejo Jo Invasion", "Galaxy Mission Start", "Retak'ka Returns", "Power Sphera Hunt", "Elemental Fusion", "Supra Awakening", "Final Elemental Battle"}
};

// Yugo alt path titles
static const char* YUGO_DRAGON_TITLES[MAX_CHAPTERS_PER_CHARACTER] = {
 "Birth of the Dragon", "Dragon's Call", "Wings of Destiny",
 "The Dragon King's Legacy", "Flames of Adamai", "Council of Six",
 "Shinonome Rising", "Eliatrope Dragon Bond", "The Dragon God", "Ruler of Dragons"
};
static const char* YUGO_PORTAL_TITLES[MAX_CHAPTERS_PER_CHARACTER] = {
 "Portal Awakening", "Gateway Training", "Path of Portals",
 "Dimensional Mastery", "Wakfu Convergence", "The Eliacube's Secret",
 "Realm Walker", "Time and Space", "The Krosmoz Revealed", "Master of Dimensions"
};

// =============================================================================
// HELPER: Set a dialogue entry
// =============================================================================
static void setDlg(DialogueScreen* d, const char* spk, const char* txt, uint16_t col,
 bool choice = false, const char* c1 = "", const char* c2 = "",
 int n1 = 0, int n2 = 0) {
 d->speaker = spk; d->text = txt; d->speaker_color = col;
 d->has_choices = choice; d->choice1 = c1; d->choice2 = c2;
 d->choice1_next_index = n1; d->choice2_next_index = n2;
}

// =============================================================================
// BOSS DATA (with visual type + colors for sprites)
// =============================================================================
static const StoryBoss ALL_BOSSES[THEME_COUNT][MAX_CHAPTERS_PER_CHARACTER] = {
 { // Luffy
   {"Alvida", "One Piece", 100,10,5,50,25,-1, BOSS_HUMANOID, RGB565(180,80,120), RGB565(255,200,200)},
   {"Buggy", "One Piece", 150,15,8,75,35,-1, BOSS_HUMANOID, RGB565(200,50,50), RGB565(100,100,255)},
   {"Kuro", "One Piece", 200,18,10,100,50,1, BOSS_HUMANOID, RGB565(30,30,30), RGB565(200,200,200)},
   {"Don Krieg", "One Piece", 250,22,12,125,60,-1, BOSS_GIANT, RGB565(180,160,50), RGB565(100,100,100)},
   {"Arlong", "One Piece", 350,28,15,150,75,2, BOSS_BEAST, RGB565(60,80,180), RGB565(200,200,220)},
   {"Crocodile", "One Piece", 500,35,20,200,100,3, BOSS_ELEMENTAL, RGB565(180,160,100), RGB565(255,220,150)},
   {"Rob Lucci", "One Piece", 650,42,25,250,125,-1, BOSS_BEAST, RGB565(60,60,80), RGB565(255,255,255)},
   {"Doflamingo", "One Piece", 800,50,30,300,150,4, BOSS_HUMANOID, RGB565(255,100,150), RGB565(255,200,200)},
   {"Katakuri", "One Piece", 1000,60,35,400,200,5, BOSS_HUMANOID, RGB565(120,30,60), RGB565(200,150,180)},
   {"Kaido", "One Piece", 1500,80,50,500,300,6, BOSS_BEAST, RGB565(100,60,180), RGB565(255,200,50)}
 },
 { // Jinwoo
   {"Low Rank Orc", "Solo Leveling", 100,10,5,50,25,-1, BOSS_BEAST, RGB565(80,120,50), RGB565(40,60,20)},
   {"Stone Golem", "Solo Leveling", 150,15,10,75,35,-1, BOSS_GIANT, RGB565(120,120,120), RGB565(80,80,80)},
   {"Ice Elf", "Solo Leveling", 200,20,12,100,50,10, BOSS_ELEMENTAL, RGB565(150,200,255), RGB565(200,230,255)},
   {"Cerberus", "Solo Leveling", 300,25,15,150,75,-1, BOSS_BEAST, RGB565(80,20,20), RGB565(255,80,30)},
   {"Igris", "Solo Leveling", 400,35,20,200,100,11, BOSS_HUMANOID, RGB565(200,50,50), RGB565(255,200,50)},
   {"Tusk", "Solo Leveling", 500,40,22,250,125,-1, BOSS_GIANT, RGB565(60,100,60), RGB565(200,200,200)},
   {"Baran", "Solo Leveling", 700,50,30,300,150,12, BOSS_DEMON, RGB565(180,50,50), RGB565(255,150,50)},
   {"Architect", "Solo Leveling", 900,60,35,400,200,-1, BOSS_HUMANOID, RGB565(200,200,220), RGB565(150,150,200)},
   {"Beast Monarch", "Solo Leveling", 1200,75,45,450,250,13, BOSS_BEAST, RGB565(150,80,30), RGB565(255,200,100)},
   {"Antares", "Solo Leveling", 1800,100,60,600,400,14, BOSS_DEMON, RGB565(200,30,30), RGB565(255,100,100)}
 },
 { // Yugo
   {"Rubilax", "Wakfu", 100,10,5,50,25,-1, BOSS_DEMON, RGB565(180,80,30), RGB565(255,150,50)},
   {"Vampyro", "Wakfu", 150,15,8,75,35,-1, BOSS_HUMANOID, RGB565(80,30,80), RGB565(200,100,200)},
   {"Nox's Puppet", "Wakfu", 200,18,10,100,50,20, BOSS_HUMANOID, RGB565(180,160,80), RGB565(100,80,40)},
   {"Razortime", "Wakfu", 280,24,14,140,70,-1, BOSS_HUMANOID, RGB565(160,140,60), RGB565(200,180,80)},
   {"Nox", "Wakfu", 400,35,20,200,100,21, BOSS_HUMANOID, RGB565(180,160,80), RGB565(60,200,200)},
   {"Qilby", "Wakfu", 550,42,25,260,130,22, BOSS_HUMANOID, RGB565(200,200,220), RGB565(100,200,255)},
   {"Rushu", "Wakfu", 700,50,30,320,160,-1, BOSS_DEMON, RGB565(200,30,30), RGB565(255,100,30)},
   {"Count Harebourg", "Wakfu", 850,58,35,380,190,23, BOSS_HUMANOID, RGB565(100,150,220), RGB565(200,220,255)},
   {"Ogrest", "Wakfu", 1100,70,45,460,240,24, BOSS_GIANT, RGB565(60,150,60), RGB565(100,200,100)},
   {"Oropo", "Wakfu", 1600,90,55,550,350,25, BOSS_HUMANOID, RGB565(0,180,220), RGB565(255,200,80)}
 },
 { // Naruto
   {"Mizuki", "Naruto", 100,10,5,50,25,-1, BOSS_HUMANOID, RGB565(150,150,160), RGB565(200,200,210)},
   {"Zabuza", "Naruto", 180,18,10,90,45,-1, BOSS_HUMANOID, RGB565(80,80,100), RGB565(200,200,220)},
   {"Orochimaru", "Naruto", 250,24,14,130,65,30, BOSS_DEMON, RGB565(140,120,180), RGB565(200,200,100)},
   {"Gaara", "Naruto", 320,30,18,170,85,-1, BOSS_ELEMENTAL, RGB565(180,150,100), RGB565(200,50,50)},
   {"Itachi", "Naruto", 420,38,22,220,110,31, BOSS_HUMANOID, RGB565(200,30,30), RGB565(30,30,30)},
   {"Pain", "Naruto", 600,50,30,300,150,32, BOSS_HUMANOID, RGB565(200,100,30), RGB565(150,100,180)},
   {"Obito", "Naruto", 800,62,38,380,190,-1, BOSS_HUMANOID, RGB565(200,100,30), RGB565(30,30,30)},
   {"Madara", "Naruto", 1000,75,45,450,225,33, BOSS_HUMANOID, RGB565(180,30,30), RGB565(40,40,60)},
   {"Kaguya", "Naruto", 1300,90,55,520,280,34, BOSS_DEMON, RGB565(230,230,240), RGB565(200,100,200)},
   {"Isshiki", "Naruto", 1700,110,70,600,380,35, BOSS_HUMANOID, RGB565(80,60,80), RGB565(200,200,200)}
 },
 { // Goku
   {"Raditz", "Dragon Ball", 120,12,6,60,30,-1, BOSS_HUMANOID, RGB565(60,40,30), RGB565(200,200,200)},
   {"Nappa", "Dragon Ball", 180,18,10,90,45,-1, BOSS_GIANT, RGB565(100,80,60), RGB565(180,160,100)},
   {"Vegeta", "Dragon Ball", 280,28,16,140,70,40, BOSS_HUMANOID, RGB565(40,40,120), RGB565(255,200,50)},
   {"Frieza", "Dragon Ball", 380,38,22,200,100,-1, BOSS_HUMANOID, RGB565(200,150,220), RGB565(200,50,80)},
   {"Frieza Final", "Dragon Ball", 500,50,30,280,140,41, BOSS_HUMANOID, RGB565(220,220,230), RGB565(200,50,200)},
   {"Cell", "Dragon Ball", 700,65,38,360,180,42, BOSS_HUMANOID, RGB565(80,140,80), RGB565(30,60,30)},
   {"Majin Buu", "Dragon Ball", 900,80,48,440,220,-1, BOSS_DEMON, RGB565(240,180,200), RGB565(200,100,120)},
   {"Beerus", "Dragon Ball", 1200,100,60,520,280,43, BOSS_HUMANOID, RGB565(140,100,180), RGB565(200,180,220)},
   {"Jiren", "Dragon Ball", 1500,120,75,600,350,44, BOSS_GIANT, RGB565(180,30,30), RGB565(200,200,200)},
   {"Moro", "Dragon Ball", 1900,150,95,700,450,45, BOSS_DEMON, RGB565(60,60,80), RGB565(100,200,100)}
 },
 { // Tanjiro
   {"Hand Demon", "Demon Slayer", 100,10,5,50,25,-1, BOSS_DEMON, RGB565(80,60,60), RGB565(200,100,100)},
   {"Swamp Demon", "Demon Slayer", 160,16,9,80,40,-1, BOSS_DEMON, RGB565(60,80,60), RGB565(100,200,100)},
   {"Susamaru", "Demon Slayer", 220,22,13,120,60,50, BOSS_HUMANOID, RGB565(200,100,50), RGB565(255,200,100)},
   {"Kyogai", "Demon Slayer", 300,30,18,160,80,-1, BOSS_DEMON, RGB565(100,80,60), RGB565(200,180,100)},
   {"Rui", "Demon Slayer", 420,40,24,220,110,51, BOSS_HUMANOID, RGB565(220,220,230), RGB565(200,200,255)},
   {"Enmu", "Demon Slayer", 550,52,32,290,145,52, BOSS_DEMON, RGB565(60,40,100), RGB565(150,100,200)},
   {"Daki & Gyutaro", "Demon Slayer", 720,68,42,370,185,-1, BOSS_HUMANOID, RGB565(200,100,150), RGB565(80,120,80)},
   {"Hantengu", "Demon Slayer", 900,85,52,440,220,53, BOSS_DEMON, RGB565(120,100,80), RGB565(200,180,100)},
   {"Akaza", "Demon Slayer", 1150,105,65,520,280,54, BOSS_HUMANOID, RGB565(200,80,120), RGB565(100,150,200)},
   {"Muzan", "Demon Slayer", 1600,140,85,650,400,55, BOSS_DEMON, RGB565(40,0,0), RGB565(200,30,30)}
 },
 { // Gojo
   {"Cursed Spirit E", "JJK", 100,10,5,50,25,-1, BOSS_DEMON, RGB565(60,80,60), RGB565(100,150,100)},
   {"Cursed Spirit D", "JJK", 150,15,8,75,38,-1, BOSS_DEMON, RGB565(80,60,80), RGB565(150,100,150)},
   {"Finger Bearer", "JJK", 220,22,13,120,60,60, BOSS_DEMON, RGB565(140,60,60), RGB565(200,100,100)},
   {"Hanami", "JJK", 320,32,20,180,90,-1, BOSS_ELEMENTAL, RGB565(60,120,60), RGB565(200,100,50)},
   {"Jogo", "JJK", 450,45,28,250,125,61, BOSS_ELEMENTAL, RGB565(200,80,30), RGB565(255,200,50)},
   {"Dagon", "JJK", 580,55,35,320,160,-1, BOSS_ELEMENTAL, RGB565(40,80,150), RGB565(100,150,200)},
   {"Mahito", "JJK", 750,70,45,400,200,62, BOSS_HUMANOID, RGB565(100,120,140), RGB565(200,200,220)},
   {"Choso", "JJK", 900,85,52,470,235,-1, BOSS_HUMANOID, RGB565(140,60,60), RGB565(200,100,100)},
   {"Toji", "JJK", 1150,110,68,550,300,63, BOSS_HUMANOID, RGB565(40,40,50), RGB565(200,200,200)},
   {"Sukuna", "JJK", 1700,150,90,680,420,64, BOSS_DEMON, RGB565(180,60,80), RGB565(255,200,200)}
 },
 { // Levi
   {"Titan 3m", "AoT", 80,8,4,45,22,-1, BOSS_GIANT, RGB565(180,140,100), RGB565(200,100,80)},
   {"Titan 7m", "AoT", 140,14,8,75,38,-1, BOSS_GIANT, RGB565(160,120,90), RGB565(200,80,60)},
   {"Abnormal", "AoT", 200,20,12,110,55,70, BOSS_GIANT, RGB565(180,150,110), RGB565(200,60,60)},
   {"Female Titan", "AoT", 320,32,20,180,90,-1, BOSS_GIANT, RGB565(200,180,160), RGB565(100,150,200)},
   {"Armored Titan", "AoT", 500,45,35,280,140,71, BOSS_GIANT, RGB565(180,160,120), RGB565(200,200,200)},
   {"Colossal", "AoT", 650,55,40,350,175,72, BOSS_GIANT, RGB565(200,80,60), RGB565(200,200,200)},
   {"Beast Titan", "AoT", 800,70,48,420,210,-1, BOSS_BEAST, RGB565(140,120,80), RGB565(100,80,50)},
   {"War Hammer", "AoT", 950,85,55,490,250,73, BOSS_GIANT, RGB565(220,220,230), RGB565(200,200,200)},
   {"Attack Titan", "AoT", 1150,100,65,560,300,74, BOSS_GIANT, RGB565(100,140,80), RGB565(200,200,200)},
   {"Founding", "AoT", 1600,140,85,680,420,75, BOSS_GIANT, RGB565(220,200,180), RGB565(255,255,255)}
 },
 { // Saitama
   {"Crablante", "OPM", 50,5,2,30,15,-1, BOSS_BEAST, RGB565(180,60,40), RGB565(200,100,80)},
   {"Mosquito Girl", "OPM", 100,10,5,60,30,-1, BOSS_HUMANOID, RGB565(150,80,60), RGB565(200,150,100)},
   {"Deep Sea King", "OPM", 200,20,12,120,60,80, BOSS_BEAST, RGB565(60,100,150), RGB565(200,200,220)},
   {"Genos Spar", "OPM", 280,28,18,160,80,-1, BOSS_HUMANOID, RGB565(255,200,50), RGB565(30,30,30)},
   {"Boros G1", "OPM", 400,40,25,230,115,81, BOSS_HUMANOID, RGB565(100,60,150), RGB565(200,150,255)},
   {"Boros G2", "OPM", 600,55,35,320,160,82, BOSS_HUMANOID, RGB565(200,150,255), RGB565(255,200,255)},
   {"Garou Human", "OPM", 750,70,45,400,200,-1, BOSS_HUMANOID, RGB565(200,200,220), RGB565(200,30,30)},
   {"Garou Monster", "OPM", 1000,95,60,500,280,83, BOSS_BEAST, RGB565(30,30,40), RGB565(200,50,50)},
   {"Cosmic Garou", "OPM", 1400,130,80,620,380,84, BOSS_DEMON, RGB565(30,30,60), RGB565(255,255,255)},
   {"God", "OPM", 2000,180,100,800,500,85, BOSS_DEMON, RGB565(20,20,30), RGB565(255,255,220)}
 },
 { // Deku
   {"Sludge Villain", "MHA", 80,8,4,45,22,-1, BOSS_ELEMENTAL, RGB565(80,100,60), RGB565(100,150,80)},
   {"Robot 0-P", "MHA", 150,15,10,85,42,-1, BOSS_GIANT, RGB565(100,100,110), RGB565(200,50,50)},
   {"Nomu USJ", "MHA", 250,25,18,140,70,90, BOSS_BEAST, RGB565(40,40,50), RGB565(200,200,200)},
   {"Stain", "MHA", 350,35,22,200,100,-1, BOSS_HUMANOID, RGB565(150,30,30), RGB565(200,200,200)},
   {"Muscular", "MHA", 480,48,30,270,135,91, BOSS_HUMANOID, RGB565(180,100,80), RGB565(200,60,60)},
   {"Overhaul", "MHA", 620,60,40,350,175,92, BOSS_HUMANOID, RGB565(80,40,60), RGB565(200,180,100)},
   {"High-End Nomu", "MHA", 800,78,50,440,220,-1, BOSS_BEAST, RGB565(60,60,80), RGB565(200,200,200)},
   {"Re-Destro", "MHA", 950,92,58,520,260,93, BOSS_HUMANOID, RGB565(120,60,60), RGB565(200,200,200)},
   {"Shigaraki", "MHA", 1200,115,72,620,340,94, BOSS_HUMANOID, RGB565(150,150,160), RGB565(200,30,30)},
   {"All For One", "MHA", 1700,160,95,750,480,95, BOSS_DEMON, RGB565(30,30,40), RGB565(200,30,30)}
 },
 { // BoBoiBoy
   {"Adu Du Robot", "BBB", 100,10,5,50,25,-1, BOSS_HUMANOID, RGB565(80,150,80), RGB565(200,200,200)},
   {"Probe Mech", "BBB", 160,16,9,80,40,-1, BOSS_GIANT, RGB565(200,100,200), RGB565(255,200,255)},
   {"Ejo Jo", "BBB", 240,24,15,130,65,100, BOSS_HUMANOID, RGB565(200,50,50), RGB565(200,200,200)},
   {"Captain Kaizo", "BBB", 350,35,22,190,95,-1, BOSS_HUMANOID, RGB565(100,50,150), RGB565(200,150,255)},
   {"Retak'ka", "BBB", 500,50,32,280,140,101, BOSS_ELEMENTAL, RGB565(200,100,30), RGB565(255,200,80)},
   {"Bora Ra", "BBB", 650,62,40,360,180,102, BOSS_HUMANOID, RGB565(200,30,30), RGB565(200,200,200)},
   {"Reramos", "BBB", 820,78,50,450,225,-1, BOSS_GIANT, RGB565(80,80,100), RGB565(200,200,200)},
   {"Dark Klamkabot", "BBB", 1000,95,60,540,280,103, BOSS_DEMON, RGB565(40,30,60), RGB565(200,100,255)},
   {"Cristalbot", "BBB", 1250,115,75,640,350,104, BOSS_ELEMENTAL, RGB565(150,200,255), RGB565(255,255,255)},
   {"Dark Fang", "BBB", 1600,145,92,780,480,105, BOSS_DEMON, RGB565(30,0,30), RGB565(200,50,200)}
 }
};

// =============================================================================
// NVS KEY HELPERS
// =============================================================================
static void getCharKey(ThemeType t, const char* s, char* b, size_t n) { snprintf(b,n,"s%d_%s",(int)t,s); }
static void getChKey(ThemeType t, int c, const char* s, char* b, size_t n) { snprintf(b,n,"s%d_c%d_%s",(int)t,c,s); }

// =============================================================================
// DIALOGUE INITIALIZATION
// =============================================================================
void initStoryDialogues() {
 // --- LUFFY ---
 CharacterStory* s = &stories[THEME_LUFFY_GEAR5];
 setDlg(&s->chapters[0].dialogues[0], "Luffy", "I'm gonna be King of the Pirates!", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[0].dialogues[1], "Shanks", "This straw hat... I'll trust it to you, Luffy.", LUFFY_STRAW_RED);
 setDlg(&s->chapters[0].dialogues[2], "Narrator", "Years later, a rubber boy sets sail alone...", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[3], "Coby", "P-pirates are scary! You can't just--", COLOR_PINK);
 setDlg(&s->chapters[0].dialogues[4], "Luffy", "Shishishi! A pirate's life is all about freedom!", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[0].dialogues[5], "Narrator", "But first... Alvida blocks the way!", COLOR_WHITE);
 s->chapters[0].dialogue_count = 6;

 setDlg(&s->chapters[1].dialogues[0], "Nami", "That clown pirate has terrorized this town...", NARUTO_RASENGAN_BLUE);
 setDlg(&s->chapters[1].dialogues[1], "Buggy", "MY NOSE ISN'T BIG! Flashy punishment time!", COLOR_RED);
 setDlg(&s->chapters[1].dialogues[2], "Luffy", "Your nose IS big. And your jokes are worse!", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[1].dialogues[3], "Zoro", "Captain... this is what I signed up for?", LEVI_SURVEY_GREEN);
 setDlg(&s->chapters[1].dialogues[4], "Luffy", "Gomu Gomu no... PISTOL!", LUFFY_SUN_GOLD);
 s->chapters[1].dialogue_count = 5;

 setDlg(&s->chapters[2].dialogues[0], "Usopp", "I-I am Captain Usopp! With 8000 followers!", COLOR_YELLOW);
 setDlg(&s->chapters[2].dialogues[1], "Luffy", "Cool! You're funny. Join my crew!", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[2].dialogues[2], "Kaya", "Please... save our village from Kuro!", COLOR_WHITE);
 setDlg(&s->chapters[2].dialogues[3], "Usopp", "Even if I'm scared... I'll protect this village!", COLOR_YELLOW);
 setDlg(&s->chapters[2].dialogues[4], "Luffy", "That's the spirit! Let's go kick his butt!", LUFFY_SUN_GOLD);
 s->chapters[2].dialogue_count = 5;

 setDlg(&s->chapters[3].dialogues[0], "Sanji", "A cook fights with his hands, never his feet-- wait, reverse that.", LUFFY_ENERGY_ORANGE);
 setDlg(&s->chapters[3].dialogues[1], "Luffy", "Your food is amazing! Be my cook!", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[3].dialogues[2], "Zeff", "That brat... he's got the same crazy dream as you once did.", COLOR_WHITE);
 setDlg(&s->chapters[3].dialogues[3], "Narrator", "Don Krieg's armada arrives at the Baratie!", COLOR_WHITE);
 setDlg(&s->chapters[3].dialogues[4], "Luffy", "Nobody messes with a place that feeds people!", LUFFY_SUN_GOLD);
 s->chapters[3].dialogue_count = 5;

 setDlg(&s->chapters[4].dialogues[0], "Nami", "I... I'm sorry. I had no choice... Arlong controls everything.", NARUTO_RASENGAN_BLUE);
 setDlg(&s->chapters[4].dialogues[1], "Luffy", "Nami... you're my navigator. That's all I need to know.", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[4].dialogues[2], "Arlong", "Humans are inferior! Your weakness disgusts me!", COLOR_BLUE);
 setDlg(&s->chapters[4].dialogues[3], "Luffy", "I don't know how to use swords or navigate... but I can beat YOU.", LUFFY_SUN_GOLD);
 setDlg(&s->chapters[4].dialogues[4], "Narrator", "The room where Nami suffered... Luffy destroys it all.", COLOR_WHITE);
 setDlg(&s->chapters[4].dialogues[5], "Nami", "Luffy... help me.", NARUTO_RASENGAN_BLUE);
 setDlg(&s->chapters[4].dialogues[6], "Luffy", "OF COURSE I WILL!!!", LUFFY_SUN_GOLD);
 s->chapters[4].dialogue_count = 7;

 // Chapters 5-9 shorter
 for (int ch = 5; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Luffy", "Another adventure! Let's gooo!", LUFFY_SUN_GOLD);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Zoro", "This opponent is strong... but I have a dream too.", LEVI_SURVEY_GREEN);
   setDlg(&s->chapters[ch].dialogues[3], "Luffy", "I'll surpass every obstacle! That's my captain's duty!", LUFFY_SUN_GOLD);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- JINWOO ---
 s = &stories[THEME_SUNG_JINWOO];
 setDlg(&s->chapters[0].dialogues[0], "Jinwoo", "E-Rank Hunter Sung Jin-Woo... the weakest of all.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[0].dialogues[1], "Narrator", "A hidden dungeon within a dungeon appears...", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[2], "Jinwoo", "These statues... they're alive?!", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[0].dialogues[3], "System", "[You have been chosen as a Player.]", JINWOO_ARISE_GLOW);
 setDlg(&s->chapters[0].dialogues[4], "Jinwoo", "A quest window? What is this... a game?", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[0].dialogues[5], "System", "[Daily Quest: Survival. Do you accept?]", JINWOO_ARISE_GLOW);
 s->chapters[0].dialogue_count = 6;

 setDlg(&s->chapters[1].dialogues[0], "System", "[Instance Dungeon Key acquired.]", JINWOO_ARISE_GLOW);
 setDlg(&s->chapters[1].dialogues[1], "Jinwoo", "My stats are rising... I'm getting stronger.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[1].dialogues[2], "Narrator", "The dungeon shifts and transforms around him.", COLOR_WHITE);
 setDlg(&s->chapters[1].dialogues[3], "Jinwoo", "I won't be the weakest anymore. Never again.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[1].dialogues[4], "System", "[Level Up!]", JINWOO_ARISE_GLOW);
 s->chapters[1].dialogue_count = 5;

 setDlg(&s->chapters[2].dialogues[0], "System", "[Penalty Quest initiated. Survive 4 hours.]", JINWOO_ARISE_GLOW);
 setDlg(&s->chapters[2].dialogues[1], "Jinwoo", "Poisonous snakes... in a closed room?!", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[2].dialogues[2], "Narrator", "With no escape, Jin-Woo fights for his life.", COLOR_WHITE);
 setDlg(&s->chapters[2].dialogues[3], "Jinwoo", "If I stop... I die. So I won't stop.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[2].dialogues[4], "System", "[Penalty Quest Complete. Reward: +3 to all stats.]", JINWOO_ARISE_GLOW);
 s->chapters[2].dialogue_count = 5;

 setDlg(&s->chapters[3].dialogues[0], "Narrator", "The Red Gate traps hunters inside a frozen hell.", COLOR_WHITE);
 setDlg(&s->chapters[3].dialogues[1], "Jinwoo", "Everyone is counting on me... I can't fail.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[3].dialogues[2], "Jinwoo", "Shadow extraction... ARISE!", JINWOO_ARISE_GLOW);
 setDlg(&s->chapters[3].dialogues[3], "Narrator", "From the fallen Ice Elf, a shadow rises...", COLOR_WHITE);
 setDlg(&s->chapters[3].dialogues[4], "Jinwoo", "Come forth, my soldiers.", JINWOO_MONARCH_PURPLE);
 s->chapters[3].dialogue_count = 5;

 setDlg(&s->chapters[4].dialogues[0], "Igris", "...", JINWOO_POWER_VIOLET);
 setDlg(&s->chapters[4].dialogues[1], "Jinwoo", "A knight in red armor... protecting a throne.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[4].dialogues[2], "Narrator", "Igris, the Blood-Red Commander, blocks the path.", COLOR_WHITE);
 setDlg(&s->chapters[4].dialogues[3], "Jinwoo", "You're strong. Perfect. You'll be my shadow.", JINWOO_MONARCH_PURPLE);
 setDlg(&s->chapters[4].dialogues[4], "System", "[Shadow Extraction successful: IGRIS]", JINWOO_ARISE_GLOW);
 s->chapters[4].dialogue_count = 5;

 for (int ch = 5; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "System", "[New Quest Available.]", JINWOO_ARISE_GLOW);
   setDlg(&s->chapters[ch].dialogues[1], "Jinwoo", "Every shadow I collect makes me stronger.", JINWOO_MONARCH_PURPLE);
   setDlg(&s->chapters[ch].dialogues[2], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[3], "Jinwoo", "ARISE.", JINWOO_ARISE_GLOW);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- YUGO ---
 s = &stories[THEME_YUGO_WAKFU];
 setDlg(&s->chapters[0].dialogues[0], "Alibert", "Yugo, you're special. These portals... they're a gift.", YUGO_HAT_GOLD);
 setDlg(&s->chapters[0].dialogues[1], "Yugo", "But why can only I make them? Where do I come from?", YUGO_PORTAL_CYAN);
 setDlg(&s->chapters[0].dialogues[2], "Narrator", "A young Eliatrope discovers his portal powers.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[3], "Az", "*chirps excitedly, flapping tiny wings*", YUGO_PORTAL_GLOW);
 setDlg(&s->chapters[0].dialogues[4], "Yugo", "Az! My dragon brother! Let's find our answers together!", YUGO_PORTAL_CYAN);
 s->chapters[0].dialogue_count = 5;

 setDlg(&s->chapters[1].dialogues[0], "Nox", "Time... I need more time... the Eliacube will give me everything.", YUGO_MYSTERY_BLUE);
 setDlg(&s->chapters[1].dialogues[1], "Yugo", "Who is that masked figure? His puppets are everywhere!", YUGO_PORTAL_CYAN);
 setDlg(&s->chapters[1].dialogues[2], "Ruel", "Kid, some things are best left alone. But fine, I'll tag along.", COLOR_GOLD);
 setDlg(&s->chapters[1].dialogues[3], "Amalia", "The Sadida kingdom will not fall to some clockmaker!", LEVI_SURVEY_GREEN);
 setDlg(&s->chapters[1].dialogues[4], "Yugo", "Together, we can stop him. I know it!", YUGO_PORTAL_CYAN);
 s->chapters[1].dialogue_count = 5;

 // Chapter 3 with Yugo choice point
 setDlg(&s->chapters[2].dialogues[0], "Adamai", "Yugo... we are Eliatropes. Dragons and portals are our legacy.", YUGO_ELIATROPE_TEAL);
 setDlg(&s->chapters[2].dialogues[1], "Yugo", "I feel something calling to me... two paths, two destinies.", YUGO_PORTAL_CYAN);
 setDlg(&s->chapters[2].dialogues[2], "Narrator", "The Brotherhood faces a crossroads...", COLOR_WHITE);
 setDlg(&s->chapters[2].dialogues[3], "Grougaloragran", "Choose wisely, young one. This shapes your future.", YUGO_HAT_GOLD);
 setDlg(&s->chapters[2].dialogues[4], "Narrator", "Embrace dragon power, or master the portals?", COLOR_WHITE,
   true, "Dragon Path", "Portal Path", 5, 5);
 s->chapters[2].dialogue_count = 5;

 for (int ch = 3; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Yugo", "Every portal takes me closer to the truth.", YUGO_PORTAL_CYAN);
   setDlg(&s->chapters[ch].dialogues[1], "Az", "*roars protectively*", YUGO_PORTAL_GLOW);
   setDlg(&s->chapters[ch].dialogues[2], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[3], "Yugo", "I won't let anyone destroy the World of Twelve!", YUGO_PORTAL_CYAN);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- NARUTO ---
 s = &stories[THEME_NARUTO_SAGE];
 setDlg(&s->chapters[0].dialogues[0], "Naruto", "One day, I'll be Hokage! Believe it!", NARUTO_CHAKRA_ORANGE);
 setDlg(&s->chapters[0].dialogues[1], "Iruka", "Naruto... you've graduated. This headband is yours.", NARUTO_SAGE_GOLD);
 setDlg(&s->chapters[0].dialogues[2], "Narrator", "The Nine-Tails jinchuriki takes his first step.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[3], "Mizuki", "You're a monster, Naruto! The fox demon lives inside you!", COLOR_GRAY);
 setDlg(&s->chapters[0].dialogues[4], "Iruka", "Naruto is... one of my precious students!", NARUTO_SAGE_GOLD);
 setDlg(&s->chapters[0].dialogues[5], "Naruto", "Shadow Clone Jutsu!", NARUTO_CHAKRA_ORANGE);
 s->chapters[0].dialogue_count = 6;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Naruto", "I never go back on my word. That's my ninja way!", NARUTO_CHAKRA_ORANGE);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Kakashi", "Those who abandon their comrades are worse than scum.", COLOR_GRAY);
   setDlg(&s->chapters[ch].dialogues[3], "Naruto", "Rasengan!", NARUTO_RASENGAN_BLUE);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- GOKU ---
 s = &stories[THEME_GOKU_UI];
 setDlg(&s->chapters[0].dialogues[0], "Goku", "I'm Son Goku! I eat a lot and I love to fight!", GOKU_UI_SILVER);
 setDlg(&s->chapters[0].dialogues[1], "Raditz", "You're a Saiyan, Kakarot. You were sent to destroy this planet.", GOKU_GI_BLUE);
 setDlg(&s->chapters[0].dialogues[2], "Goku", "This is my home! These are my friends!", GOKU_UI_SILVER);
 setDlg(&s->chapters[0].dialogues[3], "Piccolo", "Goku... we'll need to work together. Just this once.", DEKU_HERO_GREEN);
 setDlg(&s->chapters[0].dialogues[4], "Goku", "Alright! Let's do this!", GOKU_UI_SILVER);
 s->chapters[0].dialogue_count = 5;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Goku", "I can feel it... a power beyond my limits!", GOKU_UI_SILVER);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Vegeta", "Kakarot... you always manage to surpass me.", GOKU_GI_BLUE);
   setDlg(&s->chapters[ch].dialogues[3], "Goku", "HAAAAAA! Kamehameha!", GOKU_KI_BLAST_BLUE);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- TANJIRO ---
 s = &stories[THEME_TANJIRO_SUN];
 setDlg(&s->chapters[0].dialogues[0], "Tanjiro", "The smell of blood... no. My family!", TANJIRO_FIRE_ORANGE);
 setDlg(&s->chapters[0].dialogues[1], "Narrator", "Only Nezuko survived. But she has... changed.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[2], "Giyu", "If you want to save your sister, go to Mt. Sagiri.", TANJIRO_WATER_BLUE);
 setDlg(&s->chapters[0].dialogues[3], "Tanjiro", "Nezuko... I'll find a way to turn you back. I promise!", TANJIRO_FIRE_ORANGE);
 setDlg(&s->chapters[0].dialogues[4], "Urokodaki", "The Final Selection awaits. Survive... and become a Slayer.", COLOR_WHITE);
 s->chapters[0].dialogue_count = 5;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Tanjiro", "I can smell it... the demon is close.", TANJIRO_FIRE_ORANGE);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Zenitsu", "I'M GONNA DIE! WE'RE ALL GONNA DIE!", COLOR_YELLOW);
   setDlg(&s->chapters[ch].dialogues[3], "Tanjiro", "Hinokami Kagura... Dance of the Fire God!", TANJIRO_SUN_FIRE);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- GOJO ---
 s = &stories[THEME_GOJO_INFINITY];
 setDlg(&s->chapters[0].dialogues[0], "Gojo", "Throughout Heaven and Earth, I alone am the honored one.", GOJO_INFINITY_BLUE);
 setDlg(&s->chapters[0].dialogues[1], "Yuji", "Sensei... is that cursed spirit really dangerous?", SAITAMA_CAPE_RED);
 setDlg(&s->chapters[0].dialogues[2], "Gojo", "Nah. I'm here. Nothing is dangerous when I'm around~", GOJO_INFINITY_BLUE);
 setDlg(&s->chapters[0].dialogues[3], "Narrator", "The strongest sorcerer guides his students.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[4], "Gojo", "Infinity. Nothing touches me unless I allow it.", GOJO_SIX_EYES_BLUE);
 s->chapters[0].dialogue_count = 5;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Gojo", "Shall I show you what real power looks like?", GOJO_INFINITY_BLUE);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Megumi", "Gojo-sensei is reckless... but undeniably strong.", GOJO_VOID_BLACK);
   setDlg(&s->chapters[ch].dialogues[3], "Gojo", "Domain Expansion: Unlimited Void.", GOJO_DEEP_INFINITY);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- LEVI ---
 s = &stories[THEME_LEVI_STRONGEST];
 setDlg(&s->chapters[0].dialogues[0], "Levi", "Tch. Another batch of brats who'll get themselves killed.", LEVI_SILVER_BLADE);
 setDlg(&s->chapters[0].dialogues[1], "Erwin", "Levi, humanity needs soldiers who can fight.", LEVI_SURVEY_GREEN);
 setDlg(&s->chapters[0].dialogues[2], "Narrator", "Humanity's Strongest Soldier. Captain Levi Ackerman.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[3], "Levi", "Give your hearts. That's all I ask.", LEVI_SILVER_BLADE);
 setDlg(&s->chapters[0].dialogues[4], "Narrator", "Beyond the walls, titans await...", COLOR_WHITE);
 s->chapters[0].dialogue_count = 5;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Levi", "I'll cut them down. Every last one.", LEVI_SILVER_BLADE);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Erwin", "ADVANCE! DEDICATE YOUR HEARTS!", LEVI_SURVEY_GREEN);
   setDlg(&s->chapters[ch].dialogues[3], "Levi", "No regrets.", LEVI_SILVER_BLADE);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- SAITAMA ---
 s = &stories[THEME_SAITAMA_OPM];
 setDlg(&s->chapters[0].dialogues[0], "Saitama", "I became a hero for fun.", SAITAMA_HERO_YELLOW);
 setDlg(&s->chapters[0].dialogues[1], "Narrator", "One Punch Man. The hero nobody believes in.", COLOR_WHITE);
 setDlg(&s->chapters[0].dialogues[2], "Saitama", "100 push-ups, 100 sit-ups, 100 squats, 10km run. Every day.", SAITAMA_HERO_YELLOW);
 setDlg(&s->chapters[0].dialogues[3], "Saitama", "I'm just a guy who's a hero for fun.", SAITAMA_HERO_YELLOW);
 setDlg(&s->chapters[0].dialogues[4], "Narrator", "But the cost... was all his hair.", COLOR_WHITE);
 s->chapters[0].dialogue_count = 5;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Saitama", "OK.", SAITAMA_HERO_YELLOW);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Genos", "Sensei! Let me record your training methods!", SAITAMA_GOLDEN_PUNCH);
   setDlg(&s->chapters[ch].dialogues[3], "Saitama", "One punch is all I need.", SAITAMA_HERO_YELLOW);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- DEKU ---
 s = &stories[THEME_DEKU_PLUSULTRA];
 setDlg(&s->chapters[0].dialogues[0], "Deku", "I was born without a Quirk. In a world where 80% have powers...", DEKU_HERO_GREEN);
 setDlg(&s->chapters[0].dialogues[1], "All Might", "You CAN become a hero! I deem you worthy!", DEKU_ALLMIGHT_GOLD);
 setDlg(&s->chapters[0].dialogues[2], "Deku", "Eat... this hair?!", DEKU_HERO_GREEN);
 setDlg(&s->chapters[0].dialogues[3], "All Might", "One For All! The power passed from hero to hero!", DEKU_ALLMIGHT_GOLD);
 setDlg(&s->chapters[0].dialogues[4], "Deku", "SMAAASH! ...ow, my arm.", DEKU_OFA_LIGHTNING);
 setDlg(&s->chapters[0].dialogues[5], "Narrator", "The quirkless boy begins his path to become the greatest hero.", COLOR_WHITE);
 s->chapters[0].dialogue_count = 6;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "Deku", "I have to give it my all... PLUS ULTRA!", DEKU_HERO_GREEN);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Bakugo", "GET OUT OF MY WAY, DEKU!", COLOR_ORANGE);
   setDlg(&s->chapters[ch].dialogues[3], "Deku", "Full Cowl: 100%! Delaware Detroit SMASH!", DEKU_FULL_COWL);
   s->chapters[ch].dialogue_count = 4;
 }

 // --- BOBOIBOY ---
 s = &stories[THEME_BOBOIBOY];
 setDlg(&s->chapters[0].dialogues[0], "Ochobot", "BoBoiBoy! I'm giving you elemental powers!", BBB_OCHOBOT_WHITE);
 setDlg(&s->chapters[0].dialogues[1], "BoBoiBoy", "Woah! Lightning, Wind AND Earth?! Awesome!", BBB_BAND_ORANGE);
 setDlg(&s->chapters[0].dialogues[2], "Adu Du", "That Power Sphera belongs to ME! Probe, attack!", BBB_LEAF_GREEN);
 setDlg(&s->chapters[0].dialogues[3], "BoBoiBoy", "BoBoiBoy Lightning! Hiyaaah!", BBB_LIGHTNING_YELLOW);
 setDlg(&s->chapters[0].dialogues[4], "Gopal", "Dey! Can you teach me too?!", COLOR_YELLOW);
 setDlg(&s->chapters[0].dialogues[5], "BoBoiBoy", "Let's protect our friends! Terbaik!", BBB_BAND_ORANGE);
 s->chapters[0].dialogue_count = 6;

 for (int ch = 1; ch < 10; ch++) {
   setDlg(&s->chapters[ch].dialogues[0], "BoBoiBoy", "Elemental Power! BoBoiBoy... SPLIT!", BBB_BAND_ORANGE);
   setDlg(&s->chapters[ch].dialogues[1], "Narrator", s->chapters[ch].title, COLOR_WHITE);
   setDlg(&s->chapters[ch].dialogues[2], "Ochobot", "Be careful, BoBoiBoy! This enemy is strong!", BBB_OCHOBOT_WHITE);
   setDlg(&s->chapters[ch].dialogues[3], "BoBoiBoy", "BoBoiBoy Galaxy! TERBAIK!", BBB_SUPRA_GOLD);
   s->chapters[ch].dialogue_count = 4;
 }
}

// =============================================================================
// BOSS SPRITE DRAWING
// =============================================================================
void drawBossSprite(int x, int y, StoryBoss* boss, int size, int anim_frame) {
 int bob = (anim_frame % 2) * 2 - 1;
 y += bob;

 switch (boss->visual_type) {
   case BOSS_HUMANOID: {
     // Head
     gfx->fillCircle(x, y - size/2, size/4, boss->boss_color);
     // Body
     gfx->fillRect(x - size/4, y - size/4, size/2, size/2, boss->boss_color);
     // Arms
     gfx->fillRect(x - size/2, y - size/6, size/5, size/8, boss->boss_accent);
     gfx->fillRect(x + size/3, y - size/6, size/5, size/8, boss->boss_accent);
     // Legs
     gfx->fillRect(x - size/5, y + size/4, size/7, size/4, boss->boss_color);
     gfx->fillRect(x + size/10, y + size/4, size/7, size/4, boss->boss_color);
     // Eyes (menacing)
     gfx->fillRect(x - size/8, y - size/2 - 2, size/10, size/12, COLOR_RED);
     gfx->fillRect(x + size/12, y - size/2 - 2, size/10, size/12, COLOR_RED);
     // Aura glow
     gfx->drawCircle(x, y, size/2 + 5, boss->boss_accent);
     break;
   }
   case BOSS_BEAST: {
     // Main body (hunched)
     gfx->fillCircle(x, y - size/6, size/3, boss->boss_color);
     gfx->fillCircle(x, y + size/6, size/3 + 4, boss->boss_color);
     // Head with snout
     gfx->fillCircle(x, y - size/2 + 5, size/4, boss->boss_color);
     gfx->fillTriangle(x, y - size/2 - size/4, x - size/6, y - size/3, x + size/6, y - size/3, boss->boss_color);
     // Eyes
     gfx->fillCircle(x - size/8, y - size/2, 3, COLOR_RED);
     gfx->fillCircle(x + size/8, y - size/2, 3, COLOR_RED);
     // Teeth
     for (int t = -2; t <= 2; t++) {
       gfx->fillTriangle(x + t*5, y - size/3, x + t*5 - 2, y - size/3 + 5, x + t*5 + 2, y - size/3 + 5, COLOR_WHITE);
     }
     // Claws
     gfx->fillTriangle(x - size/3, y + size/4, x - size/3 - 8, y + size/2, x - size/3 + 4, y + size/2, boss->boss_accent);
     gfx->fillTriangle(x + size/3, y + size/4, x + size/3 - 4, y + size/2, x + size/3 + 8, y + size/2, boss->boss_accent);
     break;
   }
   case BOSS_GIANT: {
     // Massive body
     gfx->fillRect(x - size/3, y - size/3, size*2/3, size*2/3, boss->boss_color);
     // Small head
     gfx->fillCircle(x, y - size/2 + 5, size/5, boss->boss_color);
     // Eyes
     gfx->fillRect(x - size/8, y - size/2 + 2, size/6, size/10, boss->boss_accent);
     // Thick arms
     gfx->fillRect(x - size/2 - size/6, y - size/4, size/5, size/2, boss->boss_color);
     gfx->fillRect(x + size/3, y - size/4, size/5, size/2, boss->boss_color);
     // Fists
     gfx->fillCircle(x - size/2 - size/10, y + size/4, size/8, boss->boss_accent);
     gfx->fillCircle(x + size/3 + size/10, y + size/4, size/8, boss->boss_accent);
     // Ground crack
     gfx->drawLine(x - size/2, y + size/3 + 5, x + size/2, y + size/3 + 5, boss->boss_accent);
     gfx->drawLine(x - size/3, y + size/3 + 8, x + size/3, y + size/3 + 8, boss->boss_accent);
     break;
   }
   case BOSS_ELEMENTAL: {
     // Swirling elemental core
     for (int r = size/2; r > 5; r -= 4) {
       uint16_t col = (r % 8 < 4) ? boss->boss_color : boss->boss_accent;
       gfx->drawCircle(x, y, r, col);
     }
     gfx->fillCircle(x, y, size/6, COLOR_WHITE);
     // Elemental tendrils
     for (int i = 0; i < 6; i++) {
       float angle = (i * 60 + anim_frame * 30) * PI / 180.0;
       int tx = x + cos(angle) * size/2;
       int ty = y + sin(angle) * size/2;
       gfx->drawLine(x, y, tx, ty, boss->boss_accent);
       gfx->fillCircle(tx, ty, 4, boss->boss_accent);
     }
     // Eyes in core
     gfx->fillCircle(x - 5, y - 3, 3, boss->boss_color);
     gfx->fillCircle(x + 5, y - 3, 3, boss->boss_color);
     break;
   }
   case BOSS_DEMON: {
     // Dark body
     gfx->fillCircle(x, y, size/3, boss->boss_color);
     // Horns
     gfx->fillTriangle(x - size/4, y - size/3, x - size/6, y - size/2 - size/4, x - size/8, y - size/3, boss->boss_accent);
     gfx->fillTriangle(x + size/4, y - size/3, x + size/6, y - size/2 - size/4, x + size/8, y - size/3, boss->boss_accent);
     // Glowing eyes
     gfx->fillCircle(x - size/8, y - size/8, 5, COLOR_RED);
     gfx->fillCircle(x + size/8, y - size/8, 5, COLOR_RED);
     gfx->fillCircle(x - size/8, y - size/8, 2, COLOR_WHITE);
     gfx->fillCircle(x + size/8, y - size/8, 2, COLOR_WHITE);
     // Dark wings
     gfx->fillTriangle(x - size/3, y, x - size/2 - size/4, y - size/4, x - size/3, y + size/4, boss->boss_color);
     gfx->fillTriangle(x + size/3, y, x + size/2 + size/4, y - size/4, x + size/3, y + size/4, boss->boss_color);
     // Mouth
     gfx->drawLine(x - size/6, y + size/8, x + size/6, y + size/8, COLOR_RED);
     // Dark aura
     gfx->drawCircle(x, y, size/2 + 3, boss->boss_accent);
     gfx->drawCircle(x, y, size/2 + 6, boss->boss_color);
     break;
   }
 }
}

// =============================================================================
// INITIALIZATION
// =============================================================================
void initStorySystem() {
 story_system.current_story = nullptr;
 story_system.current_dialogue_index = 0;
 story_system.in_story_mode = false;
 story_system.in_boss_battle = false;
 story_system.showing_rewards = false;
 story_system.player_hp = 100;
 story_system.player_max_hp = 100;
 story_system.boss_hp = 0;
 story_system.boss_max_hp = 0;
 story_system.last_event_check_day = -1;
 story_system.nvs_initialized = false;

 for (int i = 0; i < THEME_COUNT; i++) {
   stories[i].character = (ThemeType)i;
   stories[i].story_name = STORY_NAMES[i];
   stories[i].current_chapter = 0;
   stories[i].chapters_completed = 0;
   stories[i].story_completed = false;
   stories[i].yugo_path = YUGO_PATH_UNDECIDED;

   for (int j = 0; j < MAX_CHAPTERS_PER_CHARACTER; j++) {
     stories[i].chapters[j].chapter_number = j + 1;
     stories[i].chapters[j].title = CHAPTER_TITLES[i][j];
     stories[i].chapters[j].level_required = CHAPTER_LEVELS[j];
     stories[i].chapters[j].dialogue_count = 4;
     stories[i].chapters[j].boss = ALL_BOSSES[i][j];
     stories[i].chapters[j].completed = false;
     stories[i].chapters[j].boss_defeated = false;
     stories[i].chapters[j].rewards_claimed = false;

     // Default dialogues
     for (int k = 0; k < MAX_DIALOGUE_SCREENS; k++) {
       stories[i].chapters[j].dialogues[k] = {"Narrator", "The adventure continues...", COLOR_WHITE, false, "", "", 0, 0};
     }
   }
 }

 // Load actual dialogue content
 initStoryDialogues();

 // Daily events
 story_system.daily_events[0] = {"Morning Training", "Complete exercises", 50, 6, 10, false, THEME_COUNT};
 story_system.daily_events[1] = {"Afternoon Quest", "Help the villagers", 75, 12, 16, false, THEME_COUNT};
 story_system.daily_events[2] = {"Evening Battle", "Face the darkness", 100, 18, 22, false, THEME_COUNT};
 story_system.daily_events[3] = {"Midnight Mystery", "Uncover secrets", 150, 0, 4, false, THEME_COUNT};

 if (story_system.prefs.begin(STORY_NVS_NAMESPACE, false)) {
   story_system.nvs_initialized = true;
   loadStoryProgress();
 }
}

// =============================================================================
// NVS SAVE/LOAD (same as before)
// =============================================================================
void saveStoryProgress() {
 if (!story_system.nvs_initialized) return;
 for (int i = 0; i < THEME_COUNT; i++) saveStoryProgressForCharacter((ThemeType)i);
 story_system.prefs.putInt("last_event_day", story_system.last_event_check_day);
 for (int i = 0; i < 4; i++) {
   char key[20]; snprintf(key, sizeof(key), "event_%d_done", i);
   story_system.prefs.putBool(key, story_system.daily_events[i].completed_today);
 }
 story_system.prefs.putBool("initialized", true);
}

void saveStoryProgressForCharacter(ThemeType character) {
 if (!story_system.nvs_initialized || character >= THEME_COUNT) return;
 CharacterStory* st = &stories[character]; char key[32];
 getCharKey(character, "chapter", key, sizeof(key)); story_system.prefs.putInt(key, st->current_chapter);
 getCharKey(character, "completed", key, sizeof(key)); story_system.prefs.putInt(key, st->chapters_completed);
 getCharKey(character, "done", key, sizeof(key)); story_system.prefs.putBool(key, st->story_completed);
 if (character == THEME_YUGO_WAKFU) story_system.prefs.putInt("yugo_path", (int)st->yugo_path);
 for (int j = 0; j < MAX_CHAPTERS_PER_CHARACTER; j++) {
   getChKey(character, j, "done", key, sizeof(key)); story_system.prefs.putBool(key, st->chapters[j].completed);
   getChKey(character, j, "boss", key, sizeof(key)); story_system.prefs.putBool(key, st->chapters[j].boss_defeated);
   getChKey(character, j, "reward", key, sizeof(key)); story_system.prefs.putBool(key, st->chapters[j].rewards_claimed);
 }
}

void loadStoryProgress() {
 if (!story_system.nvs_initialized || !story_system.prefs.getBool("initialized", false)) return;
 for (int i = 0; i < THEME_COUNT; i++) loadStoryProgressForCharacter((ThemeType)i);
 story_system.last_event_check_day = story_system.prefs.getInt("last_event_day", -1);
 for (int i = 0; i < 4; i++) {
   char key[20]; snprintf(key, sizeof(key), "event_%d_done", i);
   story_system.daily_events[i].completed_today = story_system.prefs.getBool(key, false);
 }
}

void loadStoryProgressForCharacter(ThemeType character) {
 if (!story_system.nvs_initialized || character >= THEME_COUNT) return;
 CharacterStory* st = &stories[character]; char key[32];
 getCharKey(character, "chapter", key, sizeof(key)); st->current_chapter = story_system.prefs.getInt(key, 0);
 getCharKey(character, "completed", key, sizeof(key)); st->chapters_completed = story_system.prefs.getInt(key, 0);
 getCharKey(character, "done", key, sizeof(key)); st->story_completed = story_system.prefs.getBool(key, false);
 if (character == THEME_YUGO_WAKFU) {
   YugoStoryPath p = (YugoStoryPath)story_system.prefs.getInt("yugo_path", YUGO_PATH_UNDECIDED);
   st->yugo_path = p; if (p != YUGO_PATH_UNDECIDED) loadYugoPathStory(p);
 }
 for (int j = 0; j < MAX_CHAPTERS_PER_CHARACTER; j++) {
   getChKey(character, j, "done", key, sizeof(key)); st->chapters[j].completed = story_system.prefs.getBool(key, false);
   getChKey(character, j, "boss", key, sizeof(key)); st->chapters[j].boss_defeated = story_system.prefs.getBool(key, false);
   getChKey(character, j, "reward", key, sizeof(key)); st->chapters[j].rewards_claimed = story_system.prefs.getBool(key, false);
 }
}

void clearAllStoryProgress() {
 if (!story_system.nvs_initialized) return;
 story_system.prefs.clear();
 for (int i = 0; i < THEME_COUNT; i++) {
   stories[i].current_chapter = 0; stories[i].chapters_completed = 0;
   stories[i].story_completed = false; stories[i].yugo_path = YUGO_PATH_UNDECIDED;
   for (int j = 0; j < MAX_CHAPTERS_PER_CHARACTER; j++) {
     stories[i].chapters[j].completed = false; stories[i].chapters[j].boss_defeated = false;
     stories[i].chapters[j].rewards_claimed = false;
   }
 }
}

// =============================================================================
// STORY MANAGEMENT
// =============================================================================
CharacterStory* getCharacterStory(ThemeType t) { return (t < THEME_COUNT) ? &stories[t] : nullptr; }
void setCurrentStory(ThemeType t) { if (t < THEME_COUNT) { story_system.current_story = &stories[t]; story_system.in_story_mode = true; } }
bool isChapterUnlocked(int ch) {
 if (ch < 1 || ch > MAX_CHAPTERS_PER_CHARACTER) return false;
 CharacterXPData* xp = getCurrentCharacterXP();
 return xp ? xp->level >= CHAPTER_LEVELS[ch-1] : ch == 1;
}
int getUnlockedChapterCount() { int c = 0; for (int i = 0; i < MAX_CHAPTERS_PER_CHARACTER; i++) if (isChapterUnlocked(i+1)) c++; return c; }

void startChapter(int ch) {
 if (!story_system.current_story || ch < 1 || ch > MAX_CHAPTERS_PER_CHARACTER || !isChapterUnlocked(ch)) return;
 story_system.current_story->current_chapter = ch;
 story_system.current_dialogue_index = 0;
 story_system.in_story_mode = true;
 story_system.in_boss_battle = false;
 story_system.showing_rewards = false;
 saveStoryProgressForCharacter(story_system.current_story->character);
}

void advanceDialogue() {
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter;
 if (ch < 1) return;
 StoryChapter* chapter = &story_system.current_story->chapters[ch-1];
 story_system.current_dialogue_index++;
 if (story_system.current_dialogue_index >= chapter->dialogue_count) {
   if (!chapter->boss_defeated) startStoryBoss();
   else story_system.showing_rewards = true;
 }
}

void handleDialogueChoice(int choice) {
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter;
 DialogueScreen* d = &story_system.current_story->chapters[ch-1].dialogues[story_system.current_dialogue_index];
 if (!d->has_choices) { advanceDialogue(); return; }
 if (story_system.current_story->character == THEME_YUGO_WAKFU && isYugoDecisionPoint()) {
   setYugoPath(choice == 1 ? YUGO_PATH_DRAGONS : YUGO_PATH_PORTALS);
 }
 story_system.current_dialogue_index = (choice == 1) ? d->choice1_next_index : d->choice2_next_index;
}

void completeCurrentChapter() {
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter;
 if (ch < 1) return;
 StoryChapter* chapter = &story_system.current_story->chapters[ch-1];
 if (!chapter->completed) {
   chapter->completed = true;
   story_system.current_story->chapters_completed++;
   if (story_system.current_story->chapters_completed >= MAX_CHAPTERS_PER_CHARACTER) story_system.current_story->story_completed = true;
   gainExperience(XP_CHAPTER_COMPLETE, "Chapter Complete");
   system_state.player_gems += GEMS_CHAPTER_COMPLETE;
   saveStoryProgressForCharacter(story_system.current_story->character);
 }
}

// =============================================================================
// BOSS BATTLES
// =============================================================================
void startStoryBoss() {
 if (!story_system.current_story) return;
 story_system.in_boss_battle = true;
 CharacterXPData* xp = getCurrentCharacterXP();
 int lv = xp ? xp->level : 1;
 story_system.player_max_hp = 100 + lv * 10;
 story_system.player_hp = story_system.player_max_hp;
 StoryChapter* ch = &story_system.current_story->chapters[story_system.current_story->current_chapter - 1];
 story_system.boss_max_hp = ch->boss.hp;
 story_system.boss_hp = story_system.boss_max_hp;
}

void handleStoryBossAttack() {
 if (!story_system.in_boss_battle || !story_system.current_story) return;
 CharacterXPData* xp = getCurrentCharacterXP();
 int atk = 20 + (xp ? xp->level * 5 : 0);
 story_system.boss_hp -= atk;
 if (story_system.boss_hp > 0) {
   StoryChapter* ch = &story_system.current_story->chapters[story_system.current_story->current_chapter - 1];
   int dmg = max(1, ch->boss.attack - (xp ? xp->level : 0));
   story_system.player_hp -= dmg;
   if (story_system.player_hp <= 0) { story_system.player_hp = 0; story_system.in_boss_battle = false; story_system.in_story_mode = false; }
 } else {
   story_system.boss_hp = 0;
   story_system.current_story->chapters[story_system.current_story->current_chapter - 1].boss_defeated = true;
   story_system.in_boss_battle = false; story_system.showing_rewards = true;
   saveStoryProgressForCharacter(story_system.current_story->character);
 }
}

void handleStoryBossSpecial() {
 if (!story_system.in_boss_battle) return;
 CharacterXPData* xp = getCurrentCharacterXP();
 story_system.boss_hp -= 50 + (xp ? xp->level * 8 : 0);
 if (story_system.boss_hp <= 0) {
   story_system.boss_hp = 0;
   if (story_system.current_story) {
     story_system.current_story->chapters[story_system.current_story->current_chapter - 1].boss_defeated = true;
     saveStoryProgressForCharacter(story_system.current_story->character);
   }
   story_system.in_boss_battle = false; story_system.showing_rewards = true;
 }
}

bool isStoryBossDefeated() { return story_system.boss_hp <= 0; }

void claimBossRewards() {
 if (!story_system.current_story) return;
 StoryChapter* ch = &story_system.current_story->chapters[story_system.current_story->current_chapter - 1];
 if (!ch->rewards_claimed && ch->boss_defeated) {
   ch->rewards_claimed = true;
   gainExperience(ch->boss.xp_reward, "Boss Defeated");
   system_state.player_gems += ch->boss.gem_reward;
   completeCurrentChapter();
   story_system.showing_rewards = false; story_system.in_story_mode = false;
   saveStoryProgressForCharacter(story_system.current_story->character);
 }
}

// =============================================================================
// YUGO PATH
// =============================================================================
void setYugoPath(YugoStoryPath p) {
 if (stories[THEME_YUGO_WAKFU].yugo_path == YUGO_PATH_UNDECIDED) {
   stories[THEME_YUGO_WAKFU].yugo_path = p; loadYugoPathStory(p);
   if (story_system.nvs_initialized) story_system.prefs.putInt("yugo_path", (int)p);
 }
}
YugoStoryPath getYugoPath() { return stories[THEME_YUGO_WAKFU].yugo_path; }
bool isYugoDecisionPoint() {
 if (!story_system.current_story || story_system.current_story->character != THEME_YUGO_WAKFU) return false;
 return story_system.current_story->current_chapter == 3 && stories[THEME_YUGO_WAKFU].yugo_path == YUGO_PATH_UNDECIDED
   && story_system.current_dialogue_index >= story_system.current_story->chapters[2].dialogue_count - 1;
}
void loadYugoPathStory(YugoStoryPath p) {
 const char** t = (p == YUGO_PATH_DRAGONS) ? YUGO_DRAGON_TITLES : YUGO_PORTAL_TITLES;
 for (int i = 0; i < MAX_CHAPTERS_PER_CHARACTER; i++) stories[THEME_YUGO_WAKFU].chapters[i].title = t[i];
}

// =============================================================================
// EVENTS
// =============================================================================
void checkDailyStoryEvents() {
 WatchTime c = getCurrentTime();
 if (c.day != story_system.last_event_check_day) {
   for (int i = 0; i < 4; i++) story_system.daily_events[i].completed_today = false;
   story_system.last_event_check_day = c.day; saveStoryProgress();
 }
}
StoryEvent* getCurrentActiveEvent() {
 WatchTime c = getCurrentTime();
 for (int i = 0; i < 4; i++) {
   StoryEvent* e = &story_system.daily_events[i];
   bool in_w = (e->start_hour <= e->end_hour) ? (c.hour >= e->start_hour && c.hour < e->end_hour) : (c.hour >= e->start_hour || c.hour < e->end_hour);
   if (in_w && !e->completed_today) return e;
 }
 return nullptr;
}
void claimEventReward(int idx) {
 if (idx < 0 || idx >= 4) return;
 StoryEvent* e = &story_system.daily_events[idx];
 if (!e->completed_today) { e->completed_today = true; gainExperience(e->xp_reward, e->title); saveStoryProgress(); }
}
bool hasActiveEvent() { return getCurrentActiveEvent() != nullptr; }

// =============================================================================
// DRAWING FUNCTIONS (same structure as before, now with boss sprites)
// =============================================================================
void drawStoryMenu() {
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRect(0, 0, LCD_WIDTH, 55, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(3); gfx->setCursor(20, 15); gfx->print("Story Mode");
 if (story_system.current_story) {
   gfx->setTextSize(2); gfx->setTextColor(colors.accent); gfx->setCursor(20, 70);
   gfx->print(story_system.current_story->story_name);
   int p = (story_system.current_story->chapters_completed * 100) / MAX_CHAPTERS_PER_CHARACTER;
   gfx->fillRect(20, 100, LCD_WIDTH-40, 20, COLOR_GRAY);
   gfx->fillRect(20, 100, ((LCD_WIDTH-40)*p)/100, 20, colors.primary);
   gfx->setTextSize(1); gfx->setTextColor(COLOR_WHITE); gfx->setCursor(LCD_WIDTH/2-15, 105);
   char pt[10]; sprintf(pt, "%d%%", p); gfx->print(pt);
 }
 gfx->fillRoundRect(30, 150, LCD_WIDTH-60, 50, 10, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-50, 165); gfx->print("Chapters");
 gfx->fillRoundRect(30, 220, LCD_WIDTH-60, 50, 10, colors.secondary);
 gfx->setCursor(LCD_WIDTH/2-40, 235); gfx->print("Events");
 gfx->fillRoundRect(30, LCD_HEIGHT-70, 80, 40, 8, COLOR_GRAY);
 gfx->setTextSize(2); gfx->setCursor(45, LCD_HEIGHT-58); gfx->print("Back");
}

void drawChapterSelect() {
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRect(0, 0, LCD_WIDTH, 50, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(20, 15); gfx->print("Select Chapter");
 if (!story_system.current_story) return;
 int y_off = 60;
 for (int i = 0; i < 5 && i < MAX_CHAPTERS_PER_CHARACTER; i++) {
   StoryChapter* ch = &story_system.current_story->chapters[i];
   bool unlocked = isChapterUnlocked(i+1);
   uint16_t bg = ch->completed ? colors.primary : (unlocked ? colors.secondary : RGB565(50,50,50));
   uint16_t tc = unlocked ? COLOR_WHITE : COLOR_GRAY;
   gfx->fillRoundRect(20, y_off + i*75, LCD_WIDTH-40, 65, 8, bg);
   gfx->setTextColor(tc); gfx->setTextSize(2); gfx->setCursor(30, y_off + i*75 + 10);
   gfx->print("Ch."); gfx->print(i+1); gfx->print(": ");
   gfx->setTextSize(1); gfx->setCursor(30, y_off + i*75 + 35); gfx->print(ch->title);
   if (ch->completed) { gfx->setTextColor(COLOR_GREEN); gfx->setCursor(LCD_WIDTH-80, y_off+i*75+20); gfx->print("DONE"); }
   else if (!unlocked) { gfx->setTextColor(COLOR_RED); gfx->setCursor(LCD_WIDTH-90, y_off+i*75+20); gfx->print("Lv."); gfx->print(CHAPTER_LEVELS[i]); }
 }
 gfx->fillRoundRect(30, LCD_HEIGHT-60, 80, 40, 8, COLOR_GRAY);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(45, LCD_HEIGHT-48); gfx->print("Back");
}

void drawDialogueScreen() {
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter; if (ch < 1) return;
 StoryChapter* chapter = &story_system.current_story->chapters[ch-1];
 DialogueScreen* d = &chapter->dialogues[story_system.current_dialogue_index];
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT/2, colors.background);
 gfx->setTextColor(colors.accent); gfx->setTextSize(1); gfx->setCursor(10, 10);
 gfx->print("Chapter "); gfx->print(ch); gfx->print(": "); gfx->print(chapter->title);
 gfx->fillRoundRect(10, LCD_HEIGHT/2, LCD_WIDTH-20, LCD_HEIGHT/2-10, 10, RGB565(30,30,40));
 gfx->drawRoundRect(10, LCD_HEIGHT/2, LCD_WIDTH-20, LCD_HEIGHT/2-10, 10, colors.primary);
 gfx->setTextColor(d->speaker_color); gfx->setTextSize(2); gfx->setCursor(25, LCD_HEIGHT/2+15); gfx->print(d->speaker);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1);
 // Word wrap
 const char* txt = d->text; int x = 25, y = LCD_HEIGHT/2+50;
 while (*txt) {
   char w[50]; int wl = 0;
   while (*txt && *txt != ' ' && wl < 49) w[wl++] = *txt++;
   w[wl] = 0;
   if (x + wl*6 > LCD_WIDTH-50) { x = 25; y += 15; }
   gfx->setCursor(x, y); gfx->print(w); x += wl*6 + 6;
   if (*txt == ' ') txt++;
 }
 if (d->has_choices) {
   drawChoiceScreen(d->choice1, d->choice2);
 } else {
   gfx->setTextColor(COLOR_GRAY); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH/2-50, LCD_HEIGHT-25); gfx->print("Tap to continue...");
 }
}

void drawChoiceScreen(const char* c1, const char* c2) {
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRoundRect(30, LCD_HEIGHT-180, LCD_WIDTH-60, 60, 10, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(50, LCD_HEIGHT-160); gfx->print("1: ");
 gfx->setTextSize(1); gfx->print(c1);
 gfx->fillRoundRect(30, LCD_HEIGHT-100, LCD_WIDTH-60, 60, 10, colors.secondary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(50, LCD_HEIGHT-80); gfx->print("2: ");
 gfx->setTextSize(1); gfx->print(c2);
}

void drawStoryBossScreen() {
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter;
 StoryChapter* chapter = &story_system.current_story->chapters[ch-1];
 StoryBoss* boss = &chapter->boss;
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer

 // Boss name
 gfx->setTextColor(COLOR_RED); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-60, 15); gfx->print(boss->name);
 gfx->setTextColor(COLOR_GRAY); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH/2-40, 40); gfx->print(boss->series);

 // Boss sprite
 static int boss_anim = 0; boss_anim++;
 drawBossSprite(LCD_WIDTH/2, 130, boss, 50, boss_anim);

 // Boss HP
 int bhpw = (story_system.boss_hp * (LCD_WIDTH-80)) / story_system.boss_max_hp;
 gfx->fillRect(40, 200, LCD_WIDTH-80, 20, COLOR_GRAY);
 gfx->fillRect(40, 200, max(0, bhpw), 20, COLOR_RED);
 gfx->drawRect(40, 200, LCD_WIDTH-80, 20, COLOR_WHITE);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH/2-30, 204);
 char hp[20]; sprintf(hp, "%d/%d", story_system.boss_hp, story_system.boss_max_hp); gfx->print(hp);

 // Player HP
 int phpw = (story_system.player_hp * (LCD_WIDTH-80)) / story_system.player_max_hp;
 gfx->fillRect(40, LCD_HEIGHT-150, LCD_WIDTH-80, 20, COLOR_GRAY);
 gfx->fillRect(40, LCD_HEIGHT-150, max(0, phpw), 20, COLOR_GREEN);
 gfx->drawRect(40, LCD_HEIGHT-150, LCD_WIDTH-80, 20, COLOR_WHITE);
 gfx->setCursor(LCD_WIDTH/2-30, LCD_HEIGHT-146);
 sprintf(hp, "%d/%d", story_system.player_hp, story_system.player_max_hp); gfx->print(hp);

 // Buttons
 gfx->fillRoundRect(30, LCD_HEIGHT-110, 150, 50, 10, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(60, LCD_HEIGHT-95); gfx->print("Attack");
 gfx->fillRoundRect(LCD_WIDTH-180, LCD_HEIGHT-110, 150, 50, 10, colors.accent);
 gfx->setCursor(LCD_WIDTH-150, LCD_HEIGHT-95); gfx->print("Special");
 gfx->fillRoundRect(LCD_WIDTH/2-50, LCD_HEIGHT-50, 100, 40, 8, COLOR_GRAY);
 gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH/2-20, LCD_HEIGHT-40); gfx->print("Flee");
}

void drawChapterRewards() {
 if (!story_system.current_story) return;
 StoryChapter* ch = &story_system.current_story->chapters[story_system.current_story->current_chapter - 1];
 gfx->fillScreen(COLOR_BLACK);
 ThemeColors colors = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->setTextColor(COLOR_GOLD); gfx->setTextSize(3); gfx->setCursor(LCD_WIDTH/2-70, 50); gfx->print("Victory!");
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(50, 120); gfx->print("Rewards:");
 gfx->setTextColor(COLOR_CYAN); gfx->setCursor(50, 160); gfx->print("XP: +"); gfx->print(ch->boss.xp_reward + XP_CHAPTER_COMPLETE);
 gfx->setTextColor(COLOR_YELLOW); gfx->setCursor(50, 200); gfx->print("Gems: +"); gfx->print(ch->boss.gem_reward + GEMS_CHAPTER_COMPLETE);
 if (ch->boss.exclusive_card_id >= 0) { gfx->setTextColor(COLOR_PURPLE); gfx->setCursor(50, 240); gfx->print("Exclusive Card!"); }
 gfx->fillRoundRect(LCD_WIDTH/2-80, LCD_HEIGHT-100, 160, 60, 10, colors.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-40, LCD_HEIGHT-80); gfx->print("Claim");
}

void drawStoryEventPopup() {
 StoryEvent* e = getCurrentActiveEvent(); if (!e) return;
 ThemeColors c = *getThemeColors(system_state.current_theme);  // FIX: Added * to dereference pointer
 gfx->fillRoundRect(40, 100, LCD_WIDTH-80, 200, 15, RGB565(30,30,40));
 gfx->drawRoundRect(40, 100, LCD_WIDTH-80, 200, 15, c.accent);
 gfx->setTextColor(c.accent); gfx->setTextSize(2); gfx->setCursor(60, 120); gfx->print(e->title);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1); gfx->setCursor(60, 160); gfx->print(e->description);
 gfx->setTextColor(COLOR_CYAN); gfx->setCursor(60, 200); gfx->print("Reward: +"); gfx->print(e->xp_reward); gfx->print(" XP");
 gfx->fillRoundRect(70, 240, LCD_WIDTH-140, 45, 10, c.primary);
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-40, 252); gfx->print("Claim!");
}

void drawYugoPathChoice() {
 gfx->fillScreen(COLOR_BLACK);
 gfx->setTextColor(YUGO_PORTAL_CYAN); gfx->setTextSize(2); gfx->setCursor(LCD_WIDTH/2-100, 50); gfx->print("Choose Your Path");
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(1); gfx->setCursor(LCD_WIDTH/2-120, 90); gfx->print("This choice will affect your story...");
 gfx->fillRoundRect(30, 140, LCD_WIDTH-60, 100, 15, RGB565(180,80,30));
 gfx->setTextColor(COLOR_WHITE); gfx->setTextSize(2); gfx->setCursor(50, 160); gfx->print("Dragon Path");
 gfx->setTextSize(1); gfx->setCursor(50, 200); gfx->print("Embrace dragon power - combat focused");
 gfx->fillRoundRect(30, 260, LCD_WIDTH-60, 100, 15, YUGO_PORTAL_CYAN);
 gfx->setTextColor(COLOR_BLACK); gfx->setTextSize(2); gfx->setCursor(50, 280); gfx->print("Portal Path");
 gfx->setTextSize(1); gfx->setCursor(50, 320); gfx->print("Master dimensions - exploration focused");
}

// =============================================================================
// TOUCH HANDLING
// =============================================================================
void handleStoryMenuTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 if (g.y >= 150 && g.y <= 200) { system_state.current_screen = SCREEN_CHAPTER_SELECT; return; }
 if (g.y >= 220 && g.y <= 270 && hasActiveEvent()) { drawStoryEventPopup(); return; }
 if (g.y >= LCD_HEIGHT-70 && g.x <= 110) { story_system.in_story_mode = false; returnToAppGrid(); }
}

void handleChapterSelectTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 for (int i = 0; i < 5; i++) {
   int by = 60 + i*75;
   if (g.y >= by && g.y <= by+65 && g.x >= 20 && g.x <= LCD_WIDTH-20 && isChapterUnlocked(i+1)) {
     startChapter(i+1); system_state.current_screen = SCREEN_STORY_DIALOGUE; return;
   }
 }
 if (g.y >= LCD_HEIGHT-60 && g.x <= 110) system_state.current_screen = SCREEN_STORY_MENU;
}

void handleDialogueTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 if (isYugoDecisionPoint()) {
   if (g.y >= 140 && g.y <= 240) { setYugoPath(YUGO_PATH_DRAGONS); advanceDialogue(); return; }
   if (g.y >= 260 && g.y <= 360) { setYugoPath(YUGO_PATH_PORTALS); advanceDialogue(); return; }
   return;
 }
 if (!story_system.current_story) return;
 int ch = story_system.current_story->current_chapter; if (ch < 1) return;
 DialogueScreen* d = &story_system.current_story->chapters[ch-1].dialogues[story_system.current_dialogue_index];
 if (d->has_choices) {
   if (g.y >= LCD_HEIGHT-180 && g.y <= LCD_HEIGHT-120) handleDialogueChoice(1);
   else if (g.y >= LCD_HEIGHT-100 && g.y <= LCD_HEIGHT-40) handleDialogueChoice(2);
 } else {
   advanceDialogue();
   if (story_system.in_boss_battle) system_state.current_screen = SCREEN_STORY_BOSS;
 }
}

void handleStoryBossTouch(TouchGesture& g) {
 if (g.event != TOUCH_TAP) return;
 if (g.y >= LCD_HEIGHT-110 && g.y <= LCD_HEIGHT-60) {
   if (g.x <= 180) { handleStoryBossAttack(); if (!story_system.in_boss_battle && story_system.showing_rewards) drawChapterRewards(); return; }
   if (g.x >= LCD_WIDTH-180) { handleStoryBossSpecial(); if (!story_system.in_boss_battle && story_system.showing_rewards) drawChapterRewards(); return; }
 }
 if (g.y >= LCD_HEIGHT-50 && g.x >= LCD_WIDTH/2-50 && g.x <= LCD_WIDTH/2+50) {
   story_system.in_boss_battle = false; story_system.in_story_mode = false; system_state.current_screen = SCREEN_STORY_MENU; return;
 }
 if (story_system.showing_rewards && g.y >= LCD_HEIGHT-100) { claimBossRewards(); system_state.current_screen = SCREEN_STORY_MENU; }
}

// =============================================================================
// HELPERS
// =============================================================================
uint16_t getStoryThemeColor(ThemeType t) { return getThemeColors(t)->primary; }  // This one uses -> correctly since it returns a pointer
const char* getChapterStatusText(int ch) {
 if (!story_system.current_story || ch < 1 || ch > MAX_CHAPTERS_PER_CHARACTER) return "Unknown";
 StoryChapter* c = &story_system.current_story->chapters[ch-1];
 if (c->completed) return "Complete"; if (!isChapterUnlocked(ch)) return "Locked";
 if (c->boss_defeated) return "Boss Cleared"; return "Available";
}
float getChapterProgress() {
 if (!story_system.current_story) return 0.0f;
 return (float)story_system.current_story->chapters_completed / (float)MAX_CHAPTERS_PER_CHARACTER;
}
