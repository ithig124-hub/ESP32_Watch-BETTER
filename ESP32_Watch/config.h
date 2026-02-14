/*
 * config.h - Unified Configuration
 * Consolidates: config.h, pin_config.h, compatibility.h, memory_optimization.h
 * 
 * ESP32-S3-Touch-AMOLED-1.8 (Waveshare)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

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
// ENUMS
// =============================================================================

enum ScreenType {
  SCREEN_SPLASH, SCREEN_WATCHFACE, SCREEN_APP_GRID, SCREEN_MUSIC, SCREEN_NOTES,
  SCREEN_QUESTS, SCREEN_SETTINGS, SCREEN_PDF_READER, SCREEN_FILE_BROWSER,
  SCREEN_GAMES, SCREEN_WIFI_SETUP, SCREEN_NETWORK_STATUS, SCREEN_WEATHER_APP,
  SCREEN_NEWS_APP, SCREEN_WALLPAPER_SELECTOR, SCREEN_CALCULATOR, SCREEN_FLASHLIGHT,
  SCREEN_SLEEP, SCREEN_CHARGING
};

enum ThemeType {
  THEME_LUFFY_GEAR5, THEME_SUNG_JINWOO, THEME_YUGO_WAKFU, THEME_CUSTOM
};

enum AppType {
  APP_WATCHFACE, APP_QUESTS, APP_MUSIC, APP_NOTES, APP_FILES, APP_SETTINGS,
  APP_PDF_READER, APP_WEATHER, APP_GAMES, APP_WIFI_MANAGER, APP_NEWS_READER,
  APP_WALLPAPER_SELECTOR, APP_CALCULATOR, APP_FLASHLIGHT
};

enum GameType {
  GAME_BATTLE_ARENA, GAME_SHADOW_DUNGEON, GAME_PIRATE_ADVENTURE,
  GAME_WAKFU_QUEST, GAME_MINI_SNAKE, GAME_MEMORY_MATCH
};

enum GameState {
  GAME_MENU, GAME_PLAYING, GAME_PAUSED, GAME_OVER, GAME_WIN,
  GAME_BATTLE_SELECT, GAME_BATTLE_FIGHT, GAME_BATTLE_RESULT
};

enum QuestType {
  QUEST_STEPS, QUEST_DISTANCE, QUEST_ACTIVE_TIME, QUEST_COMBO
};

enum QuestDifficulty {
  QUEST_EASY, QUEST_MEDIUM, QUEST_HARD, QUEST_LEGENDARY
};

enum QuestCharacter {
  QUEST_LUFFY, QUEST_JINWOO, QUEST_YUGO
};

enum RPGCharacterType {
  RPG_LUFFY = 0, RPG_YUGO = 1, RPG_JINWOO = 2
};

enum YugoEndgamePath {
  YUGO_UNDECIDED = 0, YUGO_DRAGON_KING = 1, YUGO_PORTAL_MASTER = 2
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
// COLOR DEFINITIONS
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

#define RGB565(r,g,b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Theme colors
#define LUFFY_GOLD      COLOR_GOLD
#define LUFFY_PURPLE    COLOR_PURPLE
#define LUFFY_WHITE     COLOR_WHITE
#define LUFFY_CREAM     RGB565(255, 245, 225)
#define JINWOO_PURPLE   RGB565(154, 14, 234)
#define JINWOO_DARK     RGB565(75, 0, 130)
#define JINWOO_VIOLET   RGB565(178, 102, 255)
#define YUGO_TEAL       RGB565(0, 255, 255)
#define YUGO_BLUE       COLOR_BLUE
#define YUGO_ENERGY     COLOR_TEAL

// =============================================================================
// STRUCTURES
// =============================================================================

struct ThemeColors {
  uint16_t primary, secondary, accent, background, text, shadow;
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
// UTILITY MACROS
// =============================================================================

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif // CONFIG_H
