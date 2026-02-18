/*
 * config.h - Unified Configuration
 * ESP32 Anime Smartwatch - COMPLETE FEATURES
 * 
 * ESP32-S3-Touch-AMOLED-1.8 (Waveshare)
 * All 10 anime characters, Gacha, Training, Boss Rush
 */

#ifndef CONFIG_H
#define CONFIG_H

#include &lt;Arduino.h&gt;

// =============================================================================
// PIN CONFIGURATION (from pin_config.h)
// =============================================================================

// Display (QSPI)
#define LCD_SDIO0    4
#define LCD_SDIO1    5
#define LCD_SDIO2    6
#define LCD_SDIO3    7
#define LCD_SCLK     11
#define LCD_CS       12
#define LCD_RESET    8
#define LCD_WIDTH    368
#define LCD_HEIGHT   448

// I2C Bus
#define IIC_SDA      15
#define IIC_SCL      14

// Touch Controller
#define TP_INT       38
#define TP_RESET     9
#define FT3168_ADDR  0x38

// I/O Expander
#define EXPANDER_ADDR 0x20

// Power Management
#define AXP2101_ADDR  0x34

// Sensors
#define QMI8658_ADDR  0x6B
#define RTC_ADDR      0x51

// Audio
#define I2S_MCK_IO    16
#define I2S_BCK_IO    9
#define I2S_WS_IO     45
#define I2S_DO_IO     10
#define I2S_DI_IO     8
#define PA_PIN        46

// SD Card
#define SDMMC_CLK     2
#define SDMMC_CMD     1
#define SDMMC_DATA    3

// Buttons
#define BTN_BOOT      0

// =============================================================================
// DISPLAY CONFIGURATION
// =============================================================================

#define DISPLAY_WIDTH   LCD_WIDTH
#define DISPLAY_HEIGHT  LCD_HEIGHT
#define DISPLAY_ROTATION 0

// =============================================================================
// SYSTEM CONSTANTS
// =============================================================================

#define SLEEP_TIMEOUT           30000
#define DEEP_SLEEP_TIMEOUT      300000
#define SENSOR_UPDATE_INTERVAL  100
#define UI_UPDATE_INTERVAL      16
#define BATTERY_LOW_THRESHOLD   15
#define BATTERY_CRITICAL        5
#define MIN_STEP_THRESHOLD      1.2
#define STEP_TIME_WINDOW        500

// =============================================================================
// ENUMS - ENHANCED WITH ALL FEATURES
// =============================================================================

enum ScreenType {
  SCREEN_SPLASH, SCREEN_WATCHFACE, SCREEN_APP_GRID, SCREEN_MUSIC, SCREEN_NOTES,
  SCREEN_QUESTS, SCREEN_SETTINGS, SCREEN_PDF_READER, SCREEN_FILE_BROWSER,
  SCREEN_GAMES, SCREEN_WIFI_SETUP, SCREEN_NETWORK_STATUS, SCREEN_WEATHER_APP,
  SCREEN_NEWS_APP, SCREEN_WALLPAPER_SELECTOR, SCREEN_CALCULATOR, SCREEN_FLASHLIGHT,
  SCREEN_SLEEP, SCREEN_CHARGING, SCREEN_CHARACTER_STATS, SCREEN_GACHA,
  SCREEN_TRAINING, SCREEN_BOSS_RUSH, SCREEN_COLLECTION, SCREEN_THEME_SELECTOR,
  SCREEN_OTA_UPDATE, SCREEN_FUSION_LABS, SCREEN_WIFI_MANAGER, SCREEN_BACKUP
};

// 10 Premium Anime Character Themes
enum ThemeType {
  THEME_LUFFY_GEAR5,      // Gear 5 Luffy - Sun God Nika
  THEME_SUNG_JINWOO,      // Jin-Woo - Shadow Monarch
  THEME_YUGO_WAKFU,       // Yugo - Portal Master
  THEME_NARUTO_SAGE,      // Naruto - Sage Mode
  THEME_GOKU_UI,          // Goku - Ultra Instinct
  THEME_TANJIRO_SUN,      // Tanjiro - Sun Breathing
  THEME_GOJO_INFINITY,    // Gojo - Infinity
  THEME_LEVI_STRONGEST,   // Levi - Humanity's Strongest
  THEME_SAITAMA_OPM,      // Saitama - One Punch
  THEME_DEKU_PLUSULTRA,   // Deku - Plus Ultra
  THEME_CUSTOM,
  THEME_COUNT = 10
};

enum AppType {
  APP_WATCHFACE, APP_QUESTS, APP_MUSIC, APP_NOTES, APP_FILES, APP_SETTINGS,
  APP_PDF_READER, APP_WEATHER, APP_GAMES, APP_WIFI_MANAGER, APP_NEWS_READER,
  APP_WALLPAPER_SELECTOR, APP_CALCULATOR, APP_FLASHLIGHT, APP_STATS,
  APP_GACHA, APP_TRAINING, APP_BOSS_RUSH, APP_COLLECTION, APP_THEME_CHANGER
};

enum GameType {
  GAME_BATTLE_ARENA, GAME_SHADOW_DUNGEON, GAME_PIRATE_ADVENTURE,
  GAME_WAKFU_QUEST, GAME_MINI_SNAKE, GAME_MEMORY_MATCH,
  GAME_GACHA, GAME_TRAINING_REFLEX, GAME_TRAINING_TARGET, 
  GAME_TRAINING_SPEED, GAME_TRAINING_MEMORY, GAME_BOSS_RUSH
};

enum GameState {
  GAME_MENU, GAME_PLAYING, GAME_PAUSED, GAME_OVER, GAME_WIN,
  GAME_BATTLE_SELECT, GAME_BATTLE_FIGHT, GAME_BATTLE_RESULT,
  GAME_GACHA_PULLING, GAME_GACHA_REVEAL, GAME_BOSS_ATTACK,
  GAME_BOSS_DEFEND, GAME_BOSS_SPECIAL, GAME_TRAINING_READY
};

enum QuestType {
  QUEST_STEPS, QUEST_DISTANCE, QUEST_ACTIVE_TIME, QUEST_COMBO,
  QUEST_GAME_WIN, QUEST_BOSS_DEFEAT, QUEST_GACHA_PULL
};

enum QuestDifficulty {
  QUEST_EASY, QUEST_MEDIUM, QUEST_HARD, QUEST_LEGENDARY
};

enum QuestCharacter {
  QUEST_LUFFY, QUEST_JINWOO, QUEST_YUGO, QUEST_NARUTO, QUEST_GOKU,
  QUEST_TANJIRO, QUEST_GOJO, QUEST_LEVI, QUEST_SAITAMA, QUEST_DEKU
};

enum RPGCharacterType {
  RPG_LUFFY = 0, RPG_YUGO = 1, RPG_JINWOO = 2, RPG_NARUTO = 3, RPG_GOKU = 4,
  RPG_TANJIRO = 5, RPG_GOJO = 6, RPG_LEVI = 7, RPG_SAITAMA = 8, RPG_DEKU = 9
};

enum YugoEndgamePath {
  YUGO_UNDECIDED = 0, YUGO_DRAGON_KING = 1, YUGO_PORTAL_MASTER = 2
};

// Gacha Rarity Levels
enum GachaRarity {
  RARITY_COMMON = 0,     // 50% - White border
  RARITY_RARE = 1,       // 30% - Blue border
  RARITY_EPIC = 2,       // 15% - Purple border
  RARITY_LEGENDARY = 3,  // 4% - Gold border
  RARITY_MYTHIC = 4      // 1% - Rainbow border
};

// Boss Tiers
enum BossTier {
  BOSS_TIER_1 = 1,  // HP: 5000-7000
  BOSS_TIER_2 = 2,  // HP: 15000-20000
  BOSS_TIER_3 = 3,  // HP: 40000-50000
  BOSS_TIER_4 = 4   // HP: 100000-150000
};

// Training Game Types
enum TrainingType {
  TRAINING_REFLEX,   // Tap flashing buttons
  TRAINING_TARGET,   // Hit targets
  TRAINING_SPEED,    // Speed tap challenge
  TRAINING_MEMORY    // Memory pattern
};

enum FileType {
  FILE_UNKNOWN, FILE_MP3, FILE_PDF, FILE_TXT, FILE_JPG, FILE_PNG, FILE_BMP, FILE_CONFIG
};

enum WiFiState {
  WIFI_DISCONNECTED, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_FAILED, WIFI_SCANNING
};

enum PowerState {
  POWER_ACTIVE, POWER_IDLE, POWER_SLEEP, POWER_DEEP_SLEEP, POWER_CHARGING
};

enum TouchEvent {
  TOUCH_NONE, TOUCH_PRESS, TOUCH_RELEASE, TOUCH_MOVE, TOUCH_DRAG,
  TOUCH_SWIPE_UP, TOUCH_SWIPE_DOWN, TOUCH_SWIPE_LEFT, TOUCH_SWIPE_RIGHT,
  TOUCH_TAP, TOUCH_DOUBLE_TAP, TOUCH_LONG_PRESS
};

// =============================================================================
// COLOR DEFINITIONS - ALL 10 CHARACTERS
// =============================================================================

#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_ORANGE    0xFD20
#define COLOR_GOLD      0xFFC0
#define COLOR_TEAL      0x07FF
#define COLOR_GRAY      0x8410
#define COLOR_PURPLE    0x8010
#define COLOR_CYAN      0x07FF
#define COLOR_PINK      0xF81F
#define COLOR_SILVER    0xC618

#define RGB565(r,g,b) (((r &amp; 0xF8) &lt;&lt; 8) | ((g &amp; 0xFC) &lt;&lt; 3) | (b &gt;&gt; 3))

// === LUFFY GEAR 5 - Sun God Nika ===
#define LUFFY_NIKA_WHITE    0xFFFF
#define LUFFY_SUN_GOLD      0xFE60
#define LUFFY_STRAW_RED     0xF800
#define LUFFY_DEEP_BLACK    0x0000
#define LUFFY_SUN_GLOW      0xFFE0
#define LUFFY_ENERGY_ORANGE 0xFD20
#define LUFFY_CLOUD_WHITE   0xF7BE
#define LUFFY_JOY_YELLOW    0xFFE0
#define LUFFY_FREEDOM_SKY   0x5DFF
#define LUFFY_RUBBER_PINK   0xF81F

// === JIN-WOO - Shadow Monarch ===
#define JINWOO_MONARCH_PURPLE 0x780F
#define JINWOO_VOID_BLACK     0x0010
#define JINWOO_ICE_BLUE       0x001F
#define JINWOO_ABSOLUTE_DARK  0x0000
#define JINWOO_ARISE_GLOW     0x881F
#define JINWOO_DAGGER_BLUE    0x4A5F
#define JINWOO_MIST_PURPLE    0x4008
#define JINWOO_POWER_VIOLET   0xA01F
#define JINWOO_DEATH_BLACK    0x0821
#define JINWOO_SOUL_PURPLE    0x6811
#define JINWOO_PURPLE         JINWOO_MONARCH_PURPLE  // Alias for compatibility

// === YUGO - Portal Master ===
#define YUGO_PORTAL_CYAN      0x07FF
#define YUGO_ELIATROPE_TEAL   0x0410
#define YUGO_HAT_GOLD         0xFE00
#define YUGO_SKY_BLUE_GREY    0x1082
#define YUGO_PORTAL_GLOW      0x5FFF
#define YUGO_WAKFU_ENERGY     0x07F0
#define YUGO_PORTAL_RING      0x2E7F
#define YUGO_ENERGY_BURST     0x97FF
#define YUGO_MYSTERY_BLUE     0x0455
#define YUGO_ADVENTURE_TEAL   0x0670
#define YUGO_TEAL             YUGO_PORTAL_CYAN       // Alias for compatibility
#define YUGO_ENERGY           YUGO_WAKFU_ENERGY      // Alias for compatibility

// === NARUTO - Sage Mode ===
#define NARUTO_CHAKRA_ORANGE  0xFD20
#define NARUTO_OUTFIT_BLACK   0x0000
#define NARUTO_KURAMA_RED     0xF800
#define NARUTO_SLATE_GREY     0x10A2
#define NARUTO_SAGE_GOLD      0xFE60
#define NARUTO_SAGE_EYES      0xF780
#define NARUTO_KURAMA_FLAME   0xFA00
#define NARUTO_RASENGAN_BLUE  0x5DDF
#define NARUTO_WILL_FIRE      0xFD00
#define NARUTO_SHADOW_ORANGE  0xC400

// === GOKU - Ultra Instinct ===
#define GOKU_UI_SILVER        0xC618
#define GOKU_GI_BLUE          0x001F
#define GOKU_AURA_WHITE       0xFFFF
#define GOKU_VOID_BLACK       0x0000
#define GOKU_SILVER_GLOW      0xBDF7
#define GOKU_LIGHT_SILVER     0x9CF3
#define GOKU_KI_BLAST_BLUE    0x5DFF
#define GOKU_DIVINE_SILVER    0xEF7D
#define GOKU_SPEED_LINES      0x7BEF
#define GOKU_POWER_WHITE      0xFFFF

// === TANJIRO - Sun Breathing ===
#define TANJIRO_HANAFUDA_RED  0xB000
#define TANJIRO_CHECK_GREEN   0x0208
#define TANJIRO_FIRE_ORANGE   0xFD60
#define TANJIRO_DARK_CHARCOAL 0x0841
#define TANJIRO_FLAME_GLOW    0xFC00
#define TANJIRO_WATER_BLUE    0x3DDF
#define TANJIRO_SUN_FIRE      0xF800
#define TANJIRO_SCAR_BURGUNDY 0xA000
#define TANJIRO_STEEL_GREY    0x7BEF
#define TANJIRO_SPIRIT_WHITE  0xFFDF

// === GOJO - Infinity ===
#define GOJO_INFINITY_BLUE    0x5DDF
#define GOJO_SNOW_WHITE       0xFFFF
#define GOJO_CURSED_PURPLE    0x781F
#define GOJO_VOID_BLACK       0x0000
#define GOJO_LIGHT_BLUE_GLOW  0x6F3F
#define GOJO_DEEP_INFINITY    0x4A9F
#define GOJO_DOMAIN_PURPLE    0x280F
#define GOJO_SIX_EYES_BLUE    0x5DFF
#define GOJO_HOLLOW_PURPLE    0xA01F
#define GOJO_REVERSE_RED      0xF800

// === LEVI - Humanity's Strongest ===
#define LEVI_MILITARY_GREY    0x3186
#define LEVI_DARK_UNIFORM     0x2124
#define LEVI_SURVEY_GREEN     0x07E0
#define LEVI_CHARCOAL_DARK    0x0841
#define LEVI_SILVER_BLADE     0xC618
#define LEVI_CAPE_GREEN       0x0540
#define LEVI_STEEL_METAL      0xE71C
#define LEVI_TITAN_BLOOD      0x8000
#define LEVI_SPEED_BLUR       0x7BEF
#define LEVI_CLEAN_WHITE      0xFFFF

// === SAITAMA - One Punch ===
#define SAITAMA_HERO_YELLOW   0xFFE0
#define SAITAMA_CAPE_RED      0xF800
#define SAITAMA_BALD_WHITE    0xFFFF
#define SAITAMA_SIMPLE_BLACK  0x0000
#define SAITAMA_GOLDEN_PUNCH  0xFEE0
#define SAITAMA_DEEP_RED_CAPE 0xC000
#define SAITAMA_IMPACT_ORANGE 0xFD20
#define SAITAMA_DOT_EYES      0x0000
#define SAITAMA_GLOVE_RED     0xF000
#define SAITAMA_BOOT_YELLOW   0xFFC0

// === DEKU - Plus Ultra ===
#define DEKU_HERO_GREEN       0x07E0
#define DEKU_COSTUME_BLACK    0x0000
#define DEKU_ALLMIGHT_GOLD    0xFFE0
#define DEKU_DARK_HERO        0x0841
#define DEKU_OFA_LIGHTNING    0x5FE0
#define DEKU_FULL_COWL        0xAFF5
#define DEKU_AIR_FORCE        0x37E6
#define DEKU_POWER_VEINS      0xF800
#define DEKU_SHOOT_STYLE      0x07C0
#define DEKU_BLACKWHIP_DARK   0x2020

// Gacha Rarity Colors
#define GACHA_COMMON_BORDER    COLOR_WHITE
#define GACHA_RARE_BORDER      COLOR_BLUE
#define GACHA_EPIC_BORDER      COLOR_PURPLE
#define GACHA_LEGENDARY_BORDER COLOR_GOLD
#define GACHA_MYTHIC_BORDER    COLOR_PINK  // Rainbow animated

// =============================================================================
// STRUCTURES - ENHANCED
// =============================================================================

struct ThemeColors {
  uint16_t primary, secondary, accent, background, text, shadow;
  uint16_t effect1, effect2, effect3, effect4;  // Additional effect colors
  uint8_t corner_radius;     // UI corner radius (px)
  uint8_t glow_intensity;    // 0-255
  uint8_t animation_speed;   // 1-10
  uint8_t border_width;      // px
};

struct CharacterStats {
  const char* stat1_name;
  const char* stat1_desc;
  uint8_t stat1_value;
  const char* stat2_name;
  const char* stat2_desc;
  uint8_t stat2_value;
  const char* stat3_name;
  const char* stat3_desc;
  uint8_t stat3_value;
  const char* stat4_name;
  const char* stat4_desc;
  uint8_t stat4_value;
};

struct CharacterProfile {
  const char* name;
  const char* title;
  const char* series;
  const char* signature_move;
  const char* catchphrase;
  ThemeType theme;
  CharacterStats stats;
};

struct SystemState {
  ScreenType current_screen;
  ThemeType current_theme;
  AppType current_app;
  int battery_percentage;
  bool is_charging, low_battery_warning, low_power_mode;
  int brightness;
  bool wifi_connected;
  String wifi_ssid;
  int wifi_signal_strength;
  bool internet_available;
  unsigned long sleep_timer;
  int steps_today, step_goal;
  float heart_rate;
  bool touch_active;
  int touch_x, touch_y;
  bool button_pressed;
  int current_quest;
  bool music_playing;
  int current_song;
  String current_wallpaper_path;
  bool wallpaper_enabled;
  int total_mp3_files, total_pdf_files;
  bool power_available, display_available, touch_available;
  bool sensors_available, rtc_available, filesystem_available, lvgl_available;
  // New for complete features
  int player_gems;
  int player_level;
  int player_xp;
  int gacha_cards_collected;
  int bosses_defeated;
  int training_streak;
  int daily_login_count;
};

struct TouchGesture {
  TouchEvent event;
  int x, y, start_x, start_y, end_x, end_y, dx, dy, pressure;
  unsigned long timestamp, duration;
  bool is_valid;
};

struct Quest {
  String title, description;
  int target_steps, reward_points, progress;
  bool completed;
};

struct QuestData {
  String title, description;
  QuestType type;
  QuestCharacter character;
  QuestDifficulty difficulty;
  int target_value, current_progress, reward_points;
  unsigned long time_limit, start_time;
  bool completed, urgent, daily;
};

struct BattleCreature {
  String name, type;
  int level, hp, max_hp, attack, defense, speed;
  String moves[4];
  int move_power[4];
  bool is_player;
  uint16_t color;
};

// Gacha Card Structure
struct GachaCard {
  String character_name;
  String series;
  String catchphrase;
  GachaRarity rarity;
  int power_rating;   // 100-9999
  uint16_t card_color;
  bool owned;
  int duplicate_count;
};

// Boss Data Structure
struct BossData {
  String name;
  String series;
  BossTier tier;
  int hp;
  int max_hp;      // Store max HP for reset
  int attack;
  int defense;
  int level;
  bool defeated;
  int gem_reward;
};

// Training Score
struct TrainingScore {
  TrainingType type;
  int score;
  int xp_earned;
  int best_time_ms;
  int combo_count;
};

struct GameSession {
  GameType current_game;
  GameState state;
  int score, level, lives;
  unsigned long start_time;
  bool sound_enabled;
  BattleCreature player_creature, enemy_creature;
  int selected_move;
  String battle_log[5];
  int log_count;
  int player_x, player_y, map_level, items_collected;
  int snake_x[100], snake_y[100];
  int snake_length, food_x, food_y, direction;
  // Enhanced Snake
  int snake_speed_level;  // 0-3
  int snake_extra_lives;
  // Boss Rush
  int current_boss_index;
  int player_energy;      // For special attacks (0-100)
  int combo_count;
  int potions_remaining;
  // Training
  TrainingType training_type;
  int training_round;
  int training_targets_hit;
  unsigned long training_start_ms;
  int reaction_times[10];
  // Gacha
  int gacha_pull_count;
  GachaCard last_pulled_card;
};

struct RPGStats {
  int strength, speed, intelligence, endurance, magic, special_power, total_power;
};

struct RPGCharacterData {
  RPGCharacterType character_type;
  int level;
  long long experience, experience_to_next;
  RPGStats base_stats, current_stats;
  YugoEndgamePath yugo_path;
  int shadow_army_size;
  bool awakened_form;
  int quests_completed, battles_won, days_active;
  long total_steps;
};

struct WatchTime {
  int hour, minute, second, day, month, year, weekday;
};

struct FileInfo {
  String filename, filepath;
  FileType type;
  size_t size;
  time_t modified;
  bool is_directory;
};

struct MusicFile {
  String filename, filepath, title, artist, album;
  int duration;
  bool has_metadata;
};

struct WallpaperFile {
  String filename, filepath;
  ThemeType theme;
  FileType format;
  size_t file_size;
  bool is_valid;
};

struct WeatherData {
  String location, description, icon, last_update;
  float temperature, humidity, wind_speed;
  int pressure;
  bool valid;
};

struct NewsArticle {
  String title, summary, source, url, publish_time;
};

struct BatteryInfo {
  int percentage, voltage_mv, current_ma, temperature;
  bool is_charging, is_plugged;
  unsigned long charge_time_remaining, estimated_runtime;
};

struct IMUData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float temperature;
  unsigned long timestamp;
};

struct StepData {
  int daily_steps, calories_burned, active_minutes;
  float distance_km;
  unsigned long last_step_time;
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

extern SystemState system_state;

// =============================================================================
// GACHA CONSTANTS
// =============================================================================

#define GACHA_SINGLE_PULL_COST   100
#define GACHA_TEN_PULL_COST      900
#define GACHA_TOTAL_CARDS        100
#define GACHA_CARDS_PER_SERIES   10

// Gem earning rates
#define GEMS_DAILY_LOGIN         50
#define GEMS_QUEST_COMPLETE_MIN  20
#define GEMS_QUEST_COMPLETE_MAX  100
#define GEMS_LEVEL_UP            100
#define GEMS_BOSS_DEFEAT_MIN     50
#define GEMS_BOSS_DEFEAT_MAX     1000
#define GEMS_GAME_WIN_MIN        10
#define GEMS_GAME_WIN_MAX        50
#define GEMS_STEP_GOAL           30

// =============================================================================
// BOSS RUSH CONSTANTS
// =============================================================================

#define TOTAL_BOSSES             20
#define MAX_POTIONS              3
#define POTION_HEAL_PERCENT      50
#define SPECIAL_ATTACK_COST      100
#define DEFEND_DAMAGE_REDUCTION  50
#define CRITICAL_CHANCE          20
#define CRITICAL_MULTIPLIER      2

// =============================================================================
// TRAINING CONSTANTS
// =============================================================================

#define TRAINING_XP_PER_GAME_MIN 50
#define TRAINING_XP_PER_GAME_MAX 150
#define TRAINING_DAILY_BONUS     50
#define TRAINING_PERFECT_BONUS   100
#define REFLEX_PERFECT_MS        150
#define REFLEX_GREAT_MS          250
#define REFLEX_GOOD_MS           400

// =============================================================================
// UTILITY MACROS
// =============================================================================

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif // CONFIG_H
