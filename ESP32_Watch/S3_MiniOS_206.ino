/**
 *  S3 MiniOS v7.4 - MERGED LOW POWER + NIKE ENHANCED EDITION
 *  ESP32-S3-Touch-AMOLED-2.06" Smartwatch Firmware
 *
 *  ═══════════════════════════════════════════════════════════════════════════
 *  MERGED FEATURES: v7.2 (Nike Enhanced) + v7.3 (Low Power)
 *  ═══════════════════════════════════════════════════════════════════════════
 *
 *  LOW POWER ENHANCEMENT (EXTREME MODE + BATTERY <20%):
 *  When battery drops below 20% AND Extreme Power Saver is active:
 *  - Light sleep with proper I2C recovery
 *  - IMU pedometer mode (step counting during sleep)
 *  - Touch/Button interrupt wake sources
 *  - CPU frequency scaling
 *  - ~2-8mA during sleep vs ~80-150mA active
 *
 *  POWER STATE MACHINE (EXTREME + LOW BATTERY):
 *  ┌─────────────────┐
 *  │   ACTIVE MODE   │ ← Screen ON, Full CPU (240MHz), LVGL running
 *  │   ~80-150mA     │
 *  └────────┬────────┘
 *           │ (1 second timeout in EXTREME mode)
 *           ▼
 *  ┌─────────────────┐
 *  │  SCREEN OFF     │ ← Screen OFF, CPU throttled
 *  │   ~15-40mA      │
 *  └────────┬────────┘
 *           │ (If battery <20% in EXTREME mode)
 *           ▼
 *  ┌─────────────────┐
 *  │  LIGHT SLEEP    │ ← CPU paused, RAM retained, RTC + Steps only
 *  │   ~2-8mA        │   Wake on: Touch INT, IMU motion, Button
 *  └────────┬────────┘
 *           │ (Touch tap / Button / Motion)
 *           ▼
 *  ┌─────────────────┐
 *  │   WAKE EVENT    │ ← Fast wake (<200ms), I2C recovery, restore UI
 *  └─────────────────┘
 *
 *  BATTERY SAVER MODES:
 *  | Mode     | Brightness | Screen Timeout | WiFi | Light Sleep |
 *  |----------|------------|----------------|------|-------------|
 *  | Off      | User set   | 3 seconds      | On   | Never       |
 *  | Light    | 50%        | 2 seconds      | On   | Never       |
 *  | Medium   | 30%        | 1.5 seconds    | On   | Never       |
 *  | Extreme  | 10%        | 1 second       | Off  | If <20% batt|
 *
 *  ALL v7.2 FEATURES PRESERVED:
 *  - 6 Premium Watch Faces (Digital, Word Clock, Analog, Nike Sport, Minimal, Fitness)
 *  - 10 Nike Color Variants with Blue Dial
 *  - Gradient Themes & Premium Wallpapers
 *  - Full UI with all categories (Clock, Compass, Activity, Games, Weather, etc.)
 *  - Blackjack Game, Calculator, Reaction Test, Daily Challenge
 *  - WiFi Manager, TXT File Reader, SD Card System
 *  - Battery Intelligence with 3 estimation algorithms
 */

#include <lvgl.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <time.h>
#include <Arduino.h>
#include "pin_config.h"
#include <esp_task_wdt.h>
#include <esp_heap_caps.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include "ESP_I2S.h"  // For ES8311 audio codec

// ═══════════════════════════════════════════════════════════════════════════
// GADGETBRIDGE BLE SUPPORT - Time Sync via Bluetooth
// ═══════════════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#ifdef PCF85063_SLAVE_ADDRESS
#undef PCF85063_SLAVE_ADDRESS
#endif

#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "SensorQMI8658.hpp"
#include "SensorPCF85063.hpp"
#include "XPowersLib.h"
#include <FS.h>
#include <SD_MMC.h>
#include "HWCDC.h"
#include <math.h>
#include <Preferences.h>

// Wallpaper images (from 206q)
#include "AdobeStock_17557.c"
#include "AdobeStock_2026.c"
#include "AdobeStock_184869446.c"
#include "AdobeStock_174564.c"

// NIKE CUSTOM FONT - Futura Condensed Extra Bold 60px
#include "NIKE_FONT.c"
LV_FONT_DECLARE(NIKE_FONT);

#if ARDUINO_USB_CDC_ON_BOOT
#define USBSerial Serial
#else
#if !defined(USBSerial)
HWCDC USBSerial;
#endif
#endif

#define WIDGET_OS_NAME      "Widget OS"

// ═══════════════════════════════════════════════════════════════════════════
// GADGETBRIDGE BLE CONFIGURATION
// Uses Nordic UART Service (NUS) for Bangle.js protocol compatibility
// ═══════════════════════════════════════════════════════════════════════════
#define BLE_DEVICE_NAME "Bangle.js 7C3E"  // Gadgetbridge expects "Bangle.js" prefix

// Nordic UART Service UUIDs (Bangle.js compatible)
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // Write
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // Notify

// BLE Objects
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
BLECharacteristic *pRxCharacteristic = NULL;

// BLE State
bool bleDeviceConnected = false;
bool blePreviouslyConnected = false;
bool bleTimeSynced = false;
unsigned long bleLastActivity = 0;
String bleIncomingBuffer = "";

// BLE Auto-Off Timer (1 minute timeout to save battery)
bool bleEnabled = false;                    // BLE on/off state
bool bleAutoOffEnabled = true;              // Auto-off after 3 min of no activity
unsigned long bleStartTime = 0;             // When BLE was enabled
unsigned long bleLastActivityTime = 0;      // Last notification/sync activity
#define BLE_AUTO_OFF_MS 180000              // 3 minutes auto-off
String bleConnectedDeviceName = "";         // Name of connected device

// ═══════════════════════════════════════════════════════════════════════════
// NOTIFICATION SYSTEM
// ═══════════════════════════════════════════════════════════════════════════
#define MAX_NOTIFICATIONS 10
struct Notification {
    String title;
    String body;
    String app;
    unsigned long timestamp;
    bool read;
};
Notification notifications[MAX_NOTIFICATIONS];
int notificationCount = 0;
int notificationFilter = 0;  // 0=All, then cycles through unique apps
String notifAppFilters[MAX_NOTIFICATIONS + 1] = {"All"};  // "All" + unique app names
int notifAppFilterCount = 1;  // Start with "All"
int selectedNotifIndex = -1;  // -1 = list view, >=0 = viewing full notification

// Step sync from phone
int syncedSteps = 0;
int syncedCalories = 0;
int syncedDistance = 0;  // in meters
bool stepsSynced = false;

// ═══════════════════════════════════════════════════════════════════════════
// MOON PHASE & SUNRISE/SUNSET DATA
// ═══════════════════════════════════════════════════════════════════════════
float moonPhase = 0.0;          // 0.0 = new moon, 0.5 = full moon, 1.0 = new moon
String moonPhaseName = "---";
String sunriseTime = "--:--";
String sunsetTime = "--:--";
bool astroDataSynced = false;

// ═══════════════════════════════════════════════════════════════════════════
// TALLY COUNTER
// ═══════════════════════════════════════════════════════════════════════════
int tallyCount = 0;

// ═══════════════════════════════════════════════════════════════════════════
// VOICE MEMO - ES8311 Audio Recording
// ═══════════════════════════════════════════════════════════════════════════
#define VOICE_MEMO_FOLDER "/WATCH/MEMOS"
#define VOICE_SAMPLE_RATE 16000
#define VOICE_BUF_SIZE 8000
int voiceMemoCount = 0;
bool voiceMemoRecording = false;
unsigned long voiceRecordStart = 0;
int voiceRecordDuration = 0;  // seconds
I2SClass voiceI2S;
bool voiceI2SInitialized = false;
File voiceMemoFile;  // Current recording file

// ES8311 I2S Pins (from Waveshare example)
#define I2S_BCK_PIN  41
#define I2S_WS_PIN   45
#define I2S_DOUT_PIN 40
#define I2S_DIN_PIN  42
#define I2S_MCLK_PIN 16
#define ES8311_PA_PIN 46  // Power amplifier enable

// Include ES8311 codec driver
extern "C" {
    #include "es8311.h"
}

#define ES8311_MIC_GAIN_SETTING (es8311_mic_gain_t)(3)  // 0-7, 3 = 18dB

// ═══════════════════════════════════════════════════════════════════════════
// RUNNING PACE (Stopwatch + Activity)
// ═══════════════════════════════════════════════════════════════════════════
bool runningModeActive = false;
unsigned long runningStartTime = 0;
int runningStartSteps = 0;
float runningPace = 0.0;  // steps per minute
float runningDistance = 0.0;  // km

// ═══════════════════════════════════════════════════════════════════════════
// DICE ROLLER & MAGIC 8 BALL
// ═══════════════════════════════════════════════════════════════════════════
int diceValue1 = 1;
int diceValue2 = 1;
bool diceRolling = false;
unsigned long diceRollStart = 0;

String magic8BallAnswer = "Shake to ask";
bool magic8Shaking = false;
const char* magic8Answers[] = {
    "Yes", "No", "Maybe", "Definitely", "Ask again",
    "Absolutely", "No way", "Probably", "Unlikely", "100%",
    "Not now", "Soon", "Never", "Of course", "Doubtful"
};
#define NUM_8BALL_ANSWERS 15

// Forward declarations for BLE
void initGadgetbridgeBLE();
void handleGadgetbridgeMessage(String message);
void sendBLEResponse(String response);
void stopBLE();
void createBluetoothCard();
void createNotificationsCard();
void addNotification(String app, String title, String body);
void showNotificationPopup(int index);
void updateNotifAppFilters();
void createDiceRollerCard();
void createMagic8BallCard();
void createTallyCounterCard();
void createVoiceMemoCard();
void createRunningCard();

// ═══════════════════════════════════════════════════════════════════════════
// MISSING DECLARATIONS - COMPILATION FIXES
// ═══════════════════════════════════════════════════════════════════════════
#define FUSION_PROTOCOL_VERSION "1.0"
#define WEB_SERIAL_BUFFER_SIZE 256
char webSerialBuffer[WEB_SERIAL_BUFFER_SIZE];
int webSerialBufferIndex = 0;

#define WIDGET_OS_VERSION   "7.4.1-BLE"

// Weather icon color helper function
uint32_t getWeatherIconColor(const char* description) {
    String desc = String(description);
    desc.toLowerCase();
    if (desc.indexOf("clear") >= 0 || desc.indexOf("sunny") >= 0) return 0xFFD60A;
    if (desc.indexOf("cloud") >= 0 || desc.indexOf("overcast") >= 0) return 0xAEAEB2;
    if (desc.indexOf("rain") >= 0 || desc.indexOf("drizzle") >= 0) return 0x0A84FF;
    if (desc.indexOf("snow") >= 0 || desc.indexOf("sleet") >= 0) return 0xE0F7FA;
    if (desc.indexOf("thunder") >= 0 || desc.indexOf("storm") >= 0) return 0xFFD60A;
    if (desc.indexOf("mist") >= 0 || desc.indexOf("fog") >= 0 || desc.indexOf("haze") >= 0) return 0x9E9E9E;
    return 0x8E8E93;
}

#define WIDGET_OS_BUILD     "merged-lowpower-nike"
#define DEVICE_ID           "WOS-208A"

// Forward declaration for navigation
void navigateTo(int category, int subCard);

#define DEVICE_SCREEN       "2.06"
#define DEVICE_HW_REV       "A"

// ═══════════════════════════════════════════════════════════════════════════
// LOW POWER THRESHOLD - Light sleep activates when battery below this + EXTREME mode
// ═══════════════════════════════════════════════════════════════════════════
#define LOW_POWER_BATTERY_THRESHOLD 20  // Enable light sleep when battery < 20%

// ═══════════════════════════════════════════════════════════════════════════
// LOW POWER STATE MACHINE - FROM v7.3
// ═══════════════════════════════════════════════════════════════════════════
enum PowerState {
    POWER_STATE_ACTIVE = 0,      // Full power, screen on, all systems running
    POWER_STATE_DIMMED,          // Reduced brightness, slower refresh
    POWER_STATE_SCREEN_OFF,      // Screen off but CPU active (transition state)
    POWER_STATE_LIGHT_SLEEP,     // CPU paused, RAM retained, wake sources active
    POWER_STATE_WAKING           // Transitioning from sleep to active
};

volatile PowerState currentPowerState = POWER_STATE_ACTIVE;
volatile PowerState previousPowerState = POWER_STATE_ACTIVE;

// Interrupt flags for wake sources
volatile bool touchWakeFlag = false;
volatile bool imuWakeFlag = false;
volatile bool buttonWakeFlag = false;
volatile bool rtcAlarmFlag = false;
volatile bool lightSleepEnabled = false;  // Dynamic based on battery + mode

// Step counting via IMU interrupt
volatile uint32_t stepCounterISR = 0;
volatile bool stepDetectedFlag = false;

// LVGL task control for sleep
volatile bool lvglTaskRunning = true;

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE DEFINITIONS - 6 PREMIUM STYLES
// ═══════════════════════════════════════════════════════════════════════════
#define NUM_WATCH_FACES 6

typedef struct {
  const char* name;
  const char* description;
} WatchFaceInfo;

WatchFaceInfo watchFaces[NUM_WATCH_FACES] = {
  {"Digital", "Big time with stats"},
  {"Word Clock", "Time in words"},
  {"Analog Rings", "Classic with activity"},
  {"Nike Sport", "Tap dial to change color"},
  {"Minimal", "Clean dark"},
  {"Fitness", "Activity rings focus"}
};

// Nike Watch Face Color Variants - ENHANCED with all Nike colorways
#define NUM_NIKE_COLORS 10
uint8_t currentNikeColor = 0;  // Start with Nike OG

struct NikeColorScheme {
    uint32_t primary;      // Main text color
    uint32_t secondary;    // Shadow/outline color  
    uint32_t background;   // Background color (0=black for AMOLED)
    uint32_t swoosh;       // Nike swoosh/tick color
    const char* name;
    bool fullColor;        // true = solid bg, false = black AMOLED
    bool hasDial;          // true = circular dial with tick marks
};

// ENHANCED: 10 Nike color variants - ALL CLASSIC COLORWAYS
NikeColorScheme nikeColors[NUM_NIKE_COLORS] = {
    // 0. Nike OG - Classic Black/Red (THE ORIGINAL!)
    {0xE53935, 0x1A0000, 0x000000, 0xE53935, "Nike OG", false, false},
    // 1. Nike Volt - Neon Yellow/Black
    {0x000000, 0x1A1A00, 0xCCFF00, 0xFF4500, "Volt", true, false},
    // 2. Nike Orange - Orange/Black
    {0x000000, 0x331100, 0xFF6D00, 0xFFFFFF, "Orange", true, false},
    // 3. Nike Blue - Blue/White
    {0xFFFFFF, 0x0D47A1, 0x0D47A1, 0x90CAF9, "Blue", true, false},
    // 4. Nike Infrared - Hot Pink/Black (CLASSIC!)
    {0xFF1744, 0x1A0008, 0x000000, 0xFF1744, "Infrared", false, false},
    // 5. Nike Aqua - Teal/Black
    {0x00E5FF, 0x001A1F, 0x000000, 0x00E5FF, "Aqua", false, false},
    // 6. Blue Dial - White text on black, has circular dial
    {0x007AFF, 0x003366, 0x000000, 0xFFFFFF, "Blue Dial", false, true},
    // 7. Purple - White text on deep purple
    {0xFFFFFF, 0x4A0066, 0x9932CC, 0xFFFFFF, "Purple", true, false},
    // 8. Pink - White text on hot pink
    {0xFFFFFF, 0x330022, 0xFF2D92, 0xFFFFFF, "Pink", true, false},
    // 9. White - Black text on clean white (classic Nike)
    {0x000000, 0xCCCCCC, 0xFFFFFF, 0x000000, "White", true, false}
};

// ═══════════════════════════════════════════════════════════════════════════
// 5-DAY FORECAST DATA STRUCTURE
// ═══════════════════════════════════════════════════════════════════════════
struct ForecastDay {
  char dayName[4];
  float tempHigh;
  float tempLow;
  char icon[16];
  char condition[16];
};

ForecastDay forecast5Day[5] = {
  {"Mon", 28, 18, "sun", "Sunny"},
  {"Tue", 26, 17, "cloud", "Cloudy"},
  {"Wed", 24, 16, "rain", "Rain"},
  {"Thu", 27, 19, "sun", "Sunny"},
  {"Fri", 29, 20, "sun", "Clear"}
};
bool forecastLoaded = false;

// ═══════════════════════════════════════════════════════════════════════════
// GRADIENT THEME STRUCT
// ═══════════════════════════════════════════════════════════════════════════
typedef struct {
  const char* name;
  lv_color_t color1;
  lv_color_t color2;
  lv_color_t text;
  lv_color_t accent;
  lv_color_t secondary;
} GradientTheme;

#define NUM_THEMES 8

// ═══════════════════════════════════════════════════════════════════════════
// WALLPAPER THEME STRUCT - Enhanced gradient wallpapers
// ═══════════════════════════════════════════════════════════════════════════
typedef struct {
  const char* name;
  lv_color_t top;
  lv_color_t mid1;
  lv_color_t mid2;
  lv_color_t bottom;
} WallpaperTheme;

// Wallpaper themes - PREMIUM custom scenes BETTER THAN APPLE!
WallpaperTheme gradientWallpapers[] = {
  // None - solid theme color
  {"None", lv_color_hex(0x1C1C1E), lv_color_hex(0x1C1C1E), lv_color_hex(0x1C1C1E), lv_color_hex(0x1C1C1E)},

  // 1. Mountain Sunset - Epic layered mountains with sun
  {"Mountain Sunset", lv_color_hex(0x4A90D9), lv_color_hex(0xFF7F50), lv_color_hex(0xE07020), lv_color_hex(0x5D4037)},

  // 2. Golden Peaks - Sunrise with golden rays
  {"Golden Peaks", lv_color_hex(0xFF6B35), lv_color_hex(0xFFD700), lv_color_hex(0xDC6B00), lv_color_hex(0x1A0A00)},

  // 3. Canyon Dawn - Pink desert canyon walls
  {"Canyon Dawn", lv_color_hex(0x87CEEB), lv_color_hex(0xFFB6C1), lv_color_hex(0xCD5C5C), lv_color_hex(0x8B4513)},

  // 4. Island Paradise - Tropical sunset with palm trees
  {"Island Paradise", lv_color_hex(0xE6B3CC), lv_color_hex(0x9370DB), lv_color_hex(0x4169E1), lv_color_hex(0x006994)},

  // 5. Alpine Meadow - Green hills with flowers
  {"Alpine Meadow", lv_color_hex(0x87CEEB), lv_color_hex(0xF0E68C), lv_color_hex(0x32CD32), lv_color_hex(0x228B22)},

  // 6. Twilight Ocean - Night sea with moon and stars
  {"Twilight Ocean", lv_color_hex(0x0D0D1A), lv_color_hex(0x191970), lv_color_hex(0x4169E1), lv_color_hex(0x000033)}
};
#define NUM_GRADIENT_WALLPAPERS 7

// ═══════════════════════════════════════════════════════════════════════════
// UI EVENT SYSTEM (FROM FIXED - STABLE)
// ═══════════════════════════════════════════════════════════════════════════
enum UIEventType {
    UI_EVENT_NONE = 0,
    UI_EVENT_NAV_LEFT,
    UI_EVENT_NAV_RIGHT,
    UI_EVENT_NAV_UP,
    UI_EVENT_NAV_DOWN,
    UI_EVENT_TAP,
    UI_EVENT_SCREEN_ON,
    UI_EVENT_SCREEN_OFF,
    UI_EVENT_REFRESH,
    UI_EVENT_LOW_BATTERY,
    UI_EVENT_SHUTDOWN,
    UI_EVENT_ENTER_SLEEP,
    UI_EVENT_WAKE_UP
};

volatile UIEventType ui_event = UI_EVENT_NONE;
volatile int ui_event_param1 = 0;
volatile int ui_event_param2 = 0;
TaskHandle_t ui_task_handle = NULL;

volatile uint32_t last_ui_activity = 0;
volatile uint32_t last_lvgl_response = 0;
const uint32_t LVGL_STALL_TIMEOUT_MS = 5000;

void panic_recover();
void ui_activity_ping() { last_ui_activity = millis(); }

// ═══════════════════════════════════════════════════════════════════════════
// SD CARD PATHS (FROM 206q - FULL STRUCTURE)
// ═══════════════════════════════════════════════════════════════════════════
#define SD_ROOT_PATH            "/WATCH"
#define SD_SYSTEM_PATH          "/WATCH/SYSTEM"
#define SD_SYSTEM_LOGS_PATH     "/WATCH/SYSTEM/logs"
#define SD_CONFIG_PATH          "/WATCH/CONFIG"
#define SD_FACES_PATH           "/WATCH/FACES"
#define SD_FACES_CUSTOM_PATH    "/WATCH/FACES/custom"
#define SD_FACES_IMPORTED_PATH  "/WATCH/FACES/imported"
#define SD_IMAGES_PATH          "/WATCH/IMAGES"
#define SD_MUSIC_PATH           "/WATCH/MUSIC"
#define SD_CACHE_PATH           "/WATCH/CACHE"
#define SD_CACHE_TEMP_PATH      "/WATCH/CACHE/temp"
#define SD_UPDATE_PATH          "/WATCH/UPDATE"
#define SD_WIFI_PATH            "/WATCH/wifi"
#define SD_BACKUP_PATH          "/WATCH/BACKUPS"
#define SD_FIRMWARE_PATH        "/WATCH/FIRMWARE"
#define SD_LOGS_PATH            "/WATCH/LOGS"
#define SD_WALLPAPERS_PATH      "/WATCH/WALLPAPERS"
#define SD_TXT_PATH             "/WATCH/TXT"
#define SD_POWER_LOGS_PATH      "/WATCH/POWER_LOGS"
#define SD_TIME_BACKUP_PATH     "/WATCH/TIME"
#define SD_TIME_BACKUP_FILE     "/WATCH/TIME/backup.txt"

// System files
#define SD_DEVICE_JSON          "/WATCH/SYSTEM/device.json"
#define SD_OS_JSON              "/WATCH/SYSTEM/os.json"
#define SD_BUILD_TXT            "/WATCH/SYSTEM/build.txt"
#define SD_BOOT_LOG             "/WATCH/SYSTEM/logs/boot.log"

// Config files
#define SD_USER_JSON            "/WATCH/CONFIG/user.json"
#define SD_DISPLAY_JSON         "/WATCH/CONFIG/display.json"
#define SD_POWER_JSON           "/WATCH/CONFIG/power.json"
#define SD_USER_DATA_PATH       "/WATCH/CONFIG/user_data.json"
#define SD_CONFIG_TXT           "/WATCH/CONFIG/config.txt"

// Power log files
#define SD_POWER_LOG_CURRENT    "/WATCH/POWER_LOGS/current.txt"
#define SD_POWER_SUMMARY        "/WATCH/POWER_LOGS/summary.txt"

// Other files
#define SD_UPDATE_README        "/WATCH/UPDATE/README.txt"
#define SD_WIFI_CONFIG          "/WATCH/wifi/config.txt"

enum SDCardStatus { 
    SD_STATUS_NOT_PRESENT = 0, 
    SD_STATUS_MOUNTED_OK, 
    SD_STATUS_MOUNT_FAILED,
    SD_STATUS_CORRUPT,
    SD_STATUS_READ_ONLY,
    SD_STATUS_INIT_IN_PROGRESS
};
SDCardStatus sdCardStatus = SD_STATUS_NOT_PRESENT;
bool sdCardInitialized = false;
bool sdStructureCreated = false;
String sdErrorMessage = "";
uint64_t sdCardSizeMB = 0;
String sdCardType = "Unknown";

#define AUTO_BACKUP_INTERVAL_MS (24UL * 60UL * 60UL * 1000UL)
bool autoBackupEnabled = true;
unsigned long lastAutoBackup = 0;
unsigned long lastBackupTime = 0;
int totalBackups = 0;

struct SDCardHealth {
    uint64_t totalBytes, usedBytes, freeBytes;
    float usedPercent;
    bool mounted, healthy;
    String lastError;
    int writeErrors, readErrors;
};
SDCardHealth sdHealth = {0};

// ═══════════════════════════════════════════════════════════════════════════
// WIFI CONFIG (ENHANCED FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
#define MAX_WIFI_NETWORKS 5

struct WiFiNetwork {
    char ssid[64];
    char password[64];
    bool valid;
    bool isOpen;
    int32_t rssi;
};
WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int numWifiNetworks = 0;
int connectedNetworkIndex = -1;

char weatherCity[64] = "Perth";
char weatherCountry[8] = "AU";
long gmtOffsetSec = 8 * 3600;
const char* NTP_SERVER = "pool.ntp.org";
const int DAYLIGHT_OFFSET_SEC = 0;

const char* OPENWEATHER_API = "3795c13a0d3f7e17799d638edda60e3c";
bool wifiConnected = false;
bool wifiConfigFromSD = false;

// ═══════════════════════════════════════════════════════════════════════════
// TIMING & BATTERY
// ═══════════════════════════════════════════════════════════════════════════
#define SAVE_INTERVAL_MS 7200000UL
#define SCREEN_OFF_TIMEOUT_MS 3000
#define SCREEN_OFF_TIMEOUT_SAVER_MS 3000
#define USAGE_HISTORY_SIZE 24
#define CARD_USAGE_SLOTS 12

#define LVGL_TICK_PERIOD_MS 2
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// ═══════════════════════════════════════════════════════════════════════════
// POWER SAVING CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════
#define CPU_FREQ_NORMAL 240
#define CPU_FREQ_SAVER 80
#define CPU_FREQ_SLEEP 40
#define SENSOR_POLL_NORMAL_MS 200   // 5 times per second is enough
#define SENSOR_POLL_SAVER_MS 500    // 2 times per second in saver mode
#define LVGL_REFRESH_NORMAL_MS 33
#define LVGL_REFRESH_SAVER_MS 67
#define WEATHER_SYNC_INTERVAL_MS (60UL * 60UL * 1000UL)

bool powerSaverActive = false;
unsigned long lastSensorPoll = 0;
unsigned long lastWeatherSync = 0;
unsigned long sensorPollInterval = SENSOR_POLL_NORMAL_MS;
bool touchEnabled = true;

// ═══════════════════════════════════════════════════════════════════════════
// POWER CONSUMPTION LOGGING
// ═══════════════════════════════════════════════════════════════════════════
#define POWER_LOG_INTERVAL_MS (5UL * 60UL * 1000UL)
#define POWER_SUMMARY_INTERVAL_MS (60UL * 60UL * 1000UL)

struct PowerLogEntry {
    uint32_t timestamp;
    uint8_t batteryPercent;
    uint16_t batteryVoltage;
    uint8_t saverLevel;
    uint16_t cpuFreqMHz;
    uint32_t screenOnSecs;
    uint32_t screenOffSecs;
    float drainRatePerHour;
};

struct PowerLogSession {
    uint32_t sessionStartMs;
    uint8_t startBatteryPercent;
    uint8_t currentBatteryPercent;
    uint32_t totalScreenOnMs;
    uint32_t totalScreenOffMs;
    uint32_t modeChanges;
    float avgDrainRate;
    uint8_t dominantMode;
};

PowerLogSession currentPowerSession = {0};
unsigned long lastPowerLogMs = 0;
unsigned long lastPowerSummaryMs = 0;
unsigned long lastLogRotationCheck = 0;
bool powerLoggingEnabled = true;

#define MAX_LOG_FILE_SIZE (50UL * 1024UL)
#define LOG_ROTATION_CHECK_MS (60UL * 60UL * 1000UL)
uint8_t lastLogDay = 0;

// ═══════════════════════════════════════════════════════════════════════════
// HARDWARE OBJECTS
// ═══════════════════════════════════════════════════════════════════════════
SensorQMI8658 qmi;
SensorPCF85063 rtc;
XPowersPMU power;
IMUdata acc, gyr;
Preferences prefs;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
Arduino_CO5300 *gfx = new Arduino_CO5300(bus, LCD_RESET, 0, LCD_WIDTH, LCD_HEIGHT, LCD_COL_OFFSET1, LCD_ROW_OFFSET1, LCD_COL_OFFSET2, LCD_ROW_OFFSET2);
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
void Arduino_IIC_Touch_Interrupt(void);
std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS, DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

#define NUM_IDENTITIES 15

// ═══════════════════════════════════════════════════════════════════════════
// NAVIGATION - 11 CATEGORIES
// ═══════════════════════════════════════════════════════════════════════════
#define NUM_CATEGORIES 12
enum Category {
  CAT_CLOCK = 0,
  CAT_COMPASS,
  CAT_ACTIVITY,
  CAT_GAMES,
  CAT_WEATHER,
  CAT_TIMER,
  CAT_TORCH,
  CAT_TOOLS,
  CAT_BLUETOOTH,
  CAT_SETTINGS,
  CAT_SYSTEM,
  CAT_ABOUT
};

int currentCategory = CAT_CLOCK;
int currentSubCard = 0;
const int maxSubCards[] = {1, 3, 4, 3, 2, 4, 2, 5, 2, 5, 1};

bool isTransitioning = false;
int transitionDir = 0;
float transitionProgress = 0.0;
unsigned long transitionStartMs = 0;
const unsigned long TRANSITION_DURATION = 200;

volatile bool navigationLocked = false;
unsigned long lastNavigationMs = 0;
const unsigned long NAVIGATION_COOLDOWN_MS = 150;

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY INTELLIGENCE CONFIGURATION - 350mAh battery
// ═══════════════════════════════════════════════════════════════════════════
#define BATTERY_CAPACITY_MAH 350
#define SCREEN_ON_CURRENT_MA 80
#define SCREEN_OFF_CURRENT_MA 15
#define SAVER_MODE_CURRENT_MA 40
#define LIGHT_SLEEP_CURRENT_MA 3  // NEW: Light sleep current

#define LOW_BATTERY_WARNING 20
#define CRITICAL_BATTERY_WARNING 10

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY INTELLIGENCE DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════════════
struct BatteryStats {
    uint32_t screenOnTimeMs;
    uint32_t screenOffTimeMs;
    uint32_t sessionStartMs;
    uint16_t hourlyScreenOnMins[USAGE_HISTORY_SIZE];
    uint16_t hourlyScreenOffMins[USAGE_HISTORY_SIZE];
    uint16_t hourlySteps[USAGE_HISTORY_SIZE];
    uint8_t currentHourIndex;
    uint32_t cardUsageTime[CARD_USAGE_SLOTS];
    uint8_t batteryAtHourStart;
    float avgDrainPerHour;
    float weightedDrainRate;
    float dailyAvgScreenOnHours[7];
    float dailyAvgDrainRate[7];
    uint8_t currentDayIndex;
    uint32_t simpleEstimateMins;
    uint32_t weightedEstimateMins;
    uint32_t learnedEstimateMins;
    uint32_t combinedEstimateMins;
    uint32_t lightSleepTimeMs;  // NEW: Track light sleep time
};

BatteryStats batteryStats = {0};

bool batterySaverMode = false;
bool batterySaverAutoEnabled = false;

// ═══════════════════════════════════════════════════════════════════════════
// DEEP SLEEP & POWER MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════
#define DEEP_SLEEP_TIMEOUT_MS 240000  // 4 minutes of no touch = deep sleep
unsigned long lastTouchTime = 0;
bool inDeepSleep = false;

// ═══════════════════════════════════════════════════════════════════════════
// AMOLED BURN-IN PREVENTION
// ═══════════════════════════════════════════════════════════════════════════
#define BURN_IN_SHIFT_INTERVAL_MS 60000  // Shift every 60 seconds
#define BURN_IN_SHIFT_PIXELS 2           // Shift by 2 pixels max
int8_t burnInOffsetX = 0;
int8_t burnInOffsetY = 0;
unsigned long lastBurnInShift = 0;

// ═══════════════════════════════════════════════════════════════════════════
// PARTIAL DISPLAY REFRESH
// ═══════════════════════════════════════════════════════════════════════════
bool partialRefreshEnabled = true;
uint32_t lastDisplayHash = 0;  // Track if display needs full refresh

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY SAVER LEVELS - ENHANCED WITH LIGHT SLEEP FOR EXTREME
// ═══════════════════════════════════════════════════════════════════════════
// BATTERY SAVER LEVELS - 3 OPTIONS ONLY: OFF, MEDIUM, EXTREME
// ═══════════════════════════════════════════════════════════════════════════
enum BatterySaverLevel {
    BATTERY_SAVER_OFF = 0,
    BATTERY_SAVER_MEDIUM = 1,      // Was index 2, now index 1
    BATTERY_SAVER_EXTREME = 2      // Was index 3, now index 2 - Light sleep enabled
};

BatterySaverLevel batterySaverLevel = BATTERY_SAVER_OFF;  // Default to OFF

struct BatterySaverSettings {
    const char* name;
    int brightness;
    int screenTimeoutMs;
    bool disableWifiSync;
    bool reduceRefresh;
    float estimatedLifeHours;
    bool enableLightSleep;  // Light sleep flag
    int cpuFreqMhz;         // CPU frequency
    bool bleAutoOff;        // Auto-disable BLE
    int clockRefreshSec;    // How often to update clock (seconds)
};

// 3 MODES ONLY: Off, Medium, Extreme
// Off: Balanced - reasonable settings for daily use
// Medium: Power saver - lower brightness, faster timeout
// Extreme: Max saving - CPU 80MHz, light sleep, minimal screen
BatterySaverSettings saverModes[3] = {
    // name,     bright, timeout, noWifi, slowRefresh, hours, sleep, CPU,  BLEoff, clockSec
    {"Off",      100,    5000,    false,  false,       18.0,  false, 160,  false,  1},
    {"Medium",    60,    3000,    false,  true,        28.0,  false, 80,   false,  5},
    {"Extreme",   30,    1500,    true,   true,        55.0,  true,  80,   true,   60}
};

// Forward declarations for functions using custom types
void applyBatterySaverMode(BatterySaverLevel level);
void handle_ui_event(UIEventType event, int param1, int param2);
GradientTheme* getSafeTheme();
int getSafeThemeIndex();


// ═══════════════════════════════════════════════════════════════════════════
// USER DATA
// ═══════════════════════════════════════════════════════════════════════════
struct UserData {
  uint32_t steps;
  uint32_t dailyGoal;
  int stepStreak;
  float totalDistance;
  float totalCalories;
  uint32_t stepHistory[7];
  int blackjackStreak;
  int gamesWon;
  int gamesPlayed;
  uint32_t clickerScore;
  int brightness;
  int screenTimeout;
  int themeIndex;
  int compassMode;
  int wallpaperIndex;
  int watchFaceIndex;
  bool identitiesUnlocked[NUM_IDENTITIES];
  uint32_t identityProgress[NUM_IDENTITIES];
  int selectedIdentity;
} userData = {0, 10000, 7, 0.0, 0.0, {0}, 0, 0, 0, 0, 200, 1, 0, 0, 0, 0};

bool screenOn = true;
unsigned long lastActivityMs = 0;
unsigned long screenOnStartMs = 0;
unsigned long screenOffStartMs = 0;

#define BOOT_BUTTON     0
#define PWR_BUTTON      10

bool powerButtonPressed = false;
unsigned long powerButtonPressStartMs = 0;

uint8_t clockHour = 10, clockMinute = 30, clockSecond = 0;
uint8_t currentDay = 3;

float weatherTemp = 24.0;
String weatherDesc = "Sunny";
float weatherHigh = 28.0;
float weatherLow = 18.0;
bool weatherDataLoaded = false;

uint16_t batteryVoltage = 4100;
uint8_t batteryPercent = 85;
bool isCharging = false;
uint32_t freeRAM = 234567;

bool hasIMU = false, hasRTC = false, hasPMU = false, hasSD = false;

const float ALPHA = 0.98;
float roll = 0.0, pitch = 0.0, yaw = 0.0;
float compassHeading = 0.0, compassHeadingSmooth = 0.0;
float compassNorthOffset = 0.0;
float tiltX = 0.0, tiltY = 0.0;

bool torchOn = false;
int torchBrightness = 255;
int torchColorIndex = 0;
uint32_t torchColors[] = {0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
const char* torchColorNames[] = {"White", "Red", "Green", "Blue", "Yellow"};
#define NUM_TORCH_COLORS 5

bool stopwatchRunning = false;
unsigned long stopwatchStartMs = 0;
unsigned long stopwatchElapsedMs = 0;

#define MAX_LAPS 10
unsigned long lapTimes[MAX_LAPS];
int lapCount = 0;

double calcValue = 0;
double calcOperand = 0;
double calcFirstNum = 0;
char calcOperator = ' ';
bool calcNewNumber = true;
char calcDisplay[16] = "0";

// Blackjack game state
int playerCards[10], dealerCards[10];
int playerCount = 0, dealerCount = 0;
int blackjackBet = 100;
bool blackjackGameActive = false;
bool playerStand = false;

uint32_t clickerCount = 0;

int32_t touchStartX = 0, touchStartY = 0;
int32_t touchCurrentX = 0, touchCurrentY = 0;
bool touchActive = false;
unsigned long touchStartMs = 0;
volatile bool touchInterruptFlag = false;

const int SWIPE_THRESHOLD_MIN = 25;
const int TAP_THRESHOLD = 20;
const unsigned long SWIPE_MAX_DURATION = 800;

unsigned long lastClockUpdate = 0;
unsigned long lastStepUpdate = 0;
unsigned long lastBatteryUpdate = 0;
unsigned long lastSaveTime = 0;
unsigned long lastNTPSync = 0;
unsigned long lastWeatherUpdate = 0;
bool ntpSyncedOnce = false;

// Time format
bool use24HourFormat = true;

// Sunrise/Sunset
float sunriseHour = 5.5;
float sunsetHour = 19.0;

// Low battery warning
bool lowBatteryWarningShown = false;
bool criticalBatteryWarningShown = false;
unsigned long lowBatteryPopupTime = 0;
bool showingLowBatteryPopup = false;

// Charging animation
uint8_t chargingAnimFrame = 0;
unsigned long lastChargingAnimMs = 0;

// TXT file reader
#define SD_SAMPLE_TXT "/WATCH/TXT/sample.txt"
#define MAX_TXT_FILES 20
#define TXT_FILE_MAX_SIZE (64 * 1024)

struct TxtFileInfo {
    char filename[64];
    uint32_t fileSize;
};

TxtFileInfo txtFiles[MAX_TXT_FILES];
int numTxtFiles = 0;
int selectedTxtFile = -1;
char* txtFileContent = NULL;
int txtScrollOffset = 0;
int txtFileListOffset = 0;
bool txtFileLoaded = false;

// WiFi Manager
#define MAX_SCANNED_NETWORKS 10

struct ScannedNetwork {
    char ssid[64];
    int32_t rssi;
    bool isOpen;
    bool isConnected;
};

ScannedNetwork scannedNetworks[MAX_SCANNED_NETWORKS];
int numScannedNetworks = 0;
bool wifiScanComplete = false;
int wifiCardScrollOffset = 0;

const char* hardcodedSSID = "Optus_9D2E3D";
const char* hardcodedPass = "snucktemptGLeQU";

// ═══════════════════════════════════════════════════════════════════════════
// THEME ARRAY
// ═══════════════════════════════════════════════════════════════════════════
GradientTheme gradientThemes[NUM_THEMES] = {
  {"Midnight", lv_color_hex(0x1C1C1E), lv_color_hex(0x2C2C2E), lv_color_hex(0xFFFFFF), lv_color_hex(0x0A84FF), lv_color_hex(0x5E5CE6)},
  {"Ocean", lv_color_hex(0x0D3B66), lv_color_hex(0x1A759F), lv_color_hex(0xFFFFFF), lv_color_hex(0x52B2CF), lv_color_hex(0x99E1D9)},
  {"Sunset", lv_color_hex(0xFF6B35), lv_color_hex(0xF7931E), lv_color_hex(0xFFFFFF), lv_color_hex(0xFFD166), lv_color_hex(0xFFA62F)},
  {"Aurora", lv_color_hex(0x7B2CBF), lv_color_hex(0x9D4EDD), lv_color_hex(0xFFFFFF), lv_color_hex(0xC77DFF), lv_color_hex(0xE0AAFF)},
  {"Forest", lv_color_hex(0x1B4332), lv_color_hex(0x2D6A4F), lv_color_hex(0xFFFFFF), lv_color_hex(0x52B788), lv_color_hex(0x95D5B2)},
  {"Ruby", lv_color_hex(0x9B2335), lv_color_hex(0xC41E3A), lv_color_hex(0xFFFFFF), lv_color_hex(0xFF6B6B), lv_color_hex(0xFFA07A)},
  {"Graphite", lv_color_hex(0x1C1C1E), lv_color_hex(0x3A3A3C), lv_color_hex(0xFFFFFF), lv_color_hex(0x8E8E93), lv_color_hex(0xAEAEB2)},
  {"Mint", lv_color_hex(0x00A896), lv_color_hex(0x02C39A), lv_color_hex(0x1C1C1E), lv_color_hex(0x00F5D4), lv_color_hex(0xB5FFE1)}
};

// ═══════════════════════════════════════════════════════════════════════════
// SAFE THEME GETTER
// ═══════════════════════════════════════════════════════════════════════════
int getSafeThemeIndex() {
    int idx = userData.themeIndex;
    if (idx < 0 || idx >= NUM_THEMES) {
        idx = 0;
        userData.themeIndex = 0;
    }
    return idx;
}

GradientTheme* getSafeTheme() {
    return &gradientThemes[getSafeThemeIndex()];
}

// ═══════════════════════════════════════════════════════════════════════════
// INTERRUPT SERVICE ROUTINES - FROM v7.3
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Touch controller interrupt handler
 * Sets flag for main loop - no I2C operations in ISR!
 */
void IRAM_ATTR Arduino_IIC_Touch_Interrupt(void) {
    touchInterruptFlag = true;
    touchWakeFlag = true;
    lastActivityMs = millis();
}

/**
 * IMU motion/step interrupt handler
 */
void IRAM_ATTR imuInterruptHandler(void) {
    stepDetectedFlag = true;
    imuWakeFlag = true;
    stepCounterISR++;
}

/**
 * Button interrupt handler
 */
void IRAM_ATTR buttonInterruptHandler(void) {
    buttonWakeFlag = true;
    lastActivityMs = millis();
}

// ═══════════════════════════════════════════════════════════════════════════
// I2C BUS RECOVERY - CRITICAL FOR WAKE FROM LIGHT SLEEP
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Recovers I2C bus after light sleep
 * This is the key fix for the I2C reinitialization issue!
 */
bool recoverI2CBus() {
    USBSerial.println("[I2C] Recovering bus after sleep...");
    
    // Step 1: End current Wire instance
    Wire.end();
    delay(10);
    
    // Step 2: Manually toggle SCL to clear any stuck slaves
    pinMode(IIC_SCL, OUTPUT);
    pinMode(IIC_SDA, INPUT_PULLUP);
    
    for (int i = 0; i < 9; i++) {
        digitalWrite(IIC_SCL, LOW);
        delayMicroseconds(5);
        digitalWrite(IIC_SCL, HIGH);
        delayMicroseconds(5);
    }
    
    // Step 3: Generate STOP condition
    pinMode(IIC_SDA, OUTPUT);
    digitalWrite(IIC_SDA, LOW);
    delayMicroseconds(5);
    digitalWrite(IIC_SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(IIC_SDA, HIGH);
    delayMicroseconds(5);
    
    // Step 4: Reinitialize Wire
    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setClock(400000);  // 400kHz I2C
    delay(10);
    
    // Step 5: Verify devices respond
    bool rtcOk = false, pmuOk = false, touchOk = false;
    
    Wire.beginTransmission(0x51);  // PCF85063 RTC address
    rtcOk = (Wire.endTransmission() == 0);
    
    Wire.beginTransmission(0x34);  // AXP2101 PMU address
    pmuOk = (Wire.endTransmission() == 0);
    
    Wire.beginTransmission(FT3168_DEVICE_ADDRESS);
    touchOk = (Wire.endTransmission() == 0);
    
    USBSerial.printf("[I2C] Recovery result - RTC:%d PMU:%d Touch:%d\n", rtcOk, pmuOk, touchOk);
    
    // Reinitialize touch controller
    if (touchOk) {
        // Reinitialize touch controller via I2C reset
        Wire.beginTransmission(FT3168_DEVICE_ADDRESS);
        Wire.endTransmission();
        delay(50);
        delay(20);
    }
    
    return rtcOk && pmuOk;
}

// ═══════════════════════════════════════════════════════════════════════════
// LIGHT SLEEP MANAGEMENT - FROM v7.3
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Check if light sleep should be enabled
 * FIX: Removed battery <20% requirement - light sleep is enabled in EXTREME mode always
 */
bool shouldEnableLightSleep() {
    return (batterySaverLevel == BATTERY_SAVER_EXTREME) && !isCharging;
}

/**
 * Configures wake sources for light sleep
 * FIX: Changed from 60 seconds to 5 MINUTES to reduce battery drain!
 */
void configureSleepWakeSources() {
    // Touch interrupt as wake source (active low)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)TP_INT, 0);
    
    // FIX: Timer wake for periodic step sync every 5 MINUTES (not 60 seconds!)
    // 60 second wake was causing massive battery drain due to:
    // - I2C recovery overhead
    // - Screen init overhead  
    // - ~200-500ms of high power each wake
    esp_sleep_enable_timer_wakeup(300 * 1000000ULL);  // 5 minutes = 300 seconds
    
    USBSerial.println("[SLEEP] Wake sources configured: Touch INT, Timer (5min)");
}

/**
 * Prepares system for light sleep
 * FIX: Added watchdog handling before sleep
 */
void prepareForSleep() {
    USBSerial.println("[SLEEP] Preparing for light sleep...");
    
    // FIX: Feed watchdog before entering sleep!
    esp_task_wdt_reset();
    
    // 1. Stop LVGL task processing
    lvglTaskRunning = false;
    
    // 2. Turn off display completely
    gfx->setBrightness(0);
    gfx->displayOff();
    
    // 3. Disable WiFi if active
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
    
    // 4. Configure IMU for low power with step interrupt
    if (hasIMU) {
        qmi.configPedometer(0x007D, 0x0014, 0x000A, 0x0008, 4, 1, 10, 1);  // Enable pedometer with proper params
    }
    
    // 5. Lower CPU frequency for sleep entry
    setCpuFrequencyMhz(CPU_FREQ_SLEEP);
    
    // 6. Configure wake sources
    configureSleepWakeSources();
    
    // 7. Clear any pending interrupts
    touchWakeFlag = false;
    imuWakeFlag = false;
    buttonWakeFlag = false;
    
    // FIX: Final watchdog reset before sleep
    esp_task_wdt_reset();
    
    USBSerial.println("[SLEEP] Ready for light sleep");
    USBSerial.flush();
    delay(10);
}

/**
 * Enters light sleep mode
 * CPU pauses, RAM retained, RTC continues
 */
void enterLightSleep() {
    if (!shouldEnableLightSleep()) {
        USBSerial.println("[SLEEP] Light sleep not enabled - conditions not met");
        return;
    }
    
    previousPowerState = currentPowerState;
    currentPowerState = POWER_STATE_LIGHT_SLEEP;
    
    // Track sleep start time for stats
    unsigned long sleepStartMs = millis();
    
    prepareForSleep();
    
    // Enter light sleep - CPU will pause here
    esp_light_sleep_start();
    
    // ═══ WAKE UP POINT ═══
    // Execution continues here after wake event
    
    // Track sleep duration
    batteryStats.lightSleepTimeMs += (millis() - sleepStartMs);
    
    wakeFromSleep();
}

/**
 * Handles wake up from light sleep
 * FIX: Added watchdog reset immediately on wake
 */
void wakeFromSleep() {
    currentPowerState = POWER_STATE_WAKING;
    uint32_t wakeStart = millis();
    
    // FIX: CRITICAL - Feed watchdog IMMEDIATELY on wake!
    esp_task_wdt_reset();
    
    // 1. Restore CPU frequency immediately
    setCpuFrequencyMhz(CPU_FREQ_SAVER);  // Start at saver freq, can increase
    
    // FIX: Feed watchdog again after CPU restore
    esp_task_wdt_reset();
    
    // 2. Determine wake cause
    esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    USBSerial.printf("[WAKE] Cause: %d\n", wakeupCause);
    
    // 3. CRITICAL: Recover I2C bus
    if (!recoverI2CBus()) {
        USBSerial.println("[WAKE] I2C recovery failed! Attempting full reset...");
        Wire.end();
        delay(50);
        Wire.begin(IIC_SDA, IIC_SCL);
        Wire.setClock(400000);
        delay(50);
    }
    
    // FIX: Feed watchdog after I2C recovery
    esp_task_wdt_reset();
    
    // 4. Read RTC time (persisted during sleep)
    if (hasRTC) {
        RTC_DateTime dt = rtc.getDateTime();
        clockHour = dt.getHour();
        clockMinute = dt.getMinute();
        clockSecond = dt.getSecond();
        USBSerial.printf("[WAKE] RTC time: %02d:%02d:%02d\n", clockHour, clockMinute, clockSecond);
    }
    
    // 5. Read step count from IMU (accumulated during sleep)
    if (hasIMU) {
        uint32_t imuSteps = qmi.getPedometerCounter();
        if (imuSteps > userData.steps) {
            userData.steps = imuSteps;
        }
    }
    
    // 6. Read battery status
    if (hasPMU) {
        batteryPercent = power.getBatteryPercent();
        batteryVoltage = power.getBattVoltage();
        isCharging = power.isCharging();
    }
    
    // FIX: Feed watchdog before display init
    esp_task_wdt_reset();
    
    // 7. Turn on display
    gfx->displayOn();
    gfx->setBrightness(saverModes[batterySaverLevel].brightness);
    
    // 8. Resume LVGL
    lvglTaskRunning = true;
    
    // 9. Update activity timestamp
    lastActivityMs = millis();
    
    // 10. Transition to active state
    currentPowerState = POWER_STATE_ACTIVE;
    screenOn = true;
    ui_event = UI_EVENT_WAKE_UP;
    
    // FIX: Final watchdog reset after full wake
    esp_task_wdt_reset();
    
    uint32_t wakeTime = millis() - wakeStart;
    USBSerial.printf("[WAKE] Complete in %lu ms\n", wakeTime);
}

// ═══════════════════════════════════════════════════════════════════════════
// STEP COUNTING - IMU PEDOMETER FROM v7.3
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Initialize IMU for low-power step counting
 */
void initStepCounter() {
    if (!hasIMU) return;
    
    // Enable accelerometer at low ODR for power saving
    qmi.configAccelerometer(
        SensorQMI8658::ACC_RANGE_4G,
        SensorQMI8658::ACC_ODR_LOWPOWER_21Hz
    );
    
    // Enable built-in pedometer
    qmi.enablePedometer();
    
    // Configure step detection sensitivity
    qmi.configPedometer(
        0x007D,  // Sample count
        0x0001,  // Step count threshold
        0x0014,  // Time low
        0x000C,  // Time up
        0x001E   // Time count
    );
    
    USBSerial.println("[STEP] IMU pedometer initialized");
}

/**
 * Read step count from IMU hardware counter
 */
void updateStepCountFromIMU() {
    if (!hasIMU) return;
    
    uint32_t hwSteps = qmi.getPedometerCounter();
    
    if (hwSteps > userData.steps) {
        uint32_t newSteps = hwSteps - userData.steps;
        userData.steps = hwSteps;
        
        // Update derived metrics
        userData.totalDistance = userData.steps * 0.0007;
        userData.totalCalories = userData.steps * 0.04;
        
        USBSerial.printf("[STEP] Count: %lu (+%lu)\n", userData.steps, newSteps);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY ESTIMATION - ENHANCED WITH LIGHT SLEEP
// ═══════════════════════════════════════════════════════════════════════════

void calculateBatteryEstimates() {
    float remainingMAh = (batteryPercent / 100.0) * BATTERY_CAPACITY_MAH;
    
    // Calculate current draw based on screen state and power state
    float avgCurrentMA;
    if (currentPowerState == POWER_STATE_LIGHT_SLEEP) {
        avgCurrentMA = LIGHT_SLEEP_CURRENT_MA;
    } else if (screenOn) {
        avgCurrentMA = batterySaverMode ? SAVER_MODE_CURRENT_MA : SCREEN_ON_CURRENT_MA;
    } else {
        avgCurrentMA = SCREEN_OFF_CURRENT_MA;
    }

    // Simple estimate
    batteryStats.simpleEstimateMins = (uint32_t)((remainingMAh / avgCurrentMA) * 60.0);

    // Weighted estimate using usage patterns
    uint32_t totalScreenOnMs = batteryStats.screenOnTimeMs;
    uint32_t totalScreenOffMs = batteryStats.screenOffTimeMs;
    uint32_t totalLightSleepMs = batteryStats.lightSleepTimeMs;
    uint32_t totalSessionMs = totalScreenOnMs + totalScreenOffMs + totalLightSleepMs;

    if (totalSessionMs > 0) {
        float screenOnRatio = (float)totalScreenOnMs / (float)totalSessionMs;
        float screenOffRatio = (float)totalScreenOffMs / (float)totalSessionMs;
        float sleepRatio = (float)totalLightSleepMs / (float)totalSessionMs;

        float weightedCurrentMA = (screenOnRatio * SCREEN_ON_CURRENT_MA) +
                                  (screenOffRatio * SCREEN_OFF_CURRENT_MA) +
                                  (sleepRatio * LIGHT_SLEEP_CURRENT_MA);

        if (batterySaverMode) {
            weightedCurrentMA *= 0.7;
        }

        batteryStats.weightedEstimateMins = (uint32_t)((remainingMAh / weightedCurrentMA) * 60.0);
        batteryStats.weightedDrainRate = weightedCurrentMA;
    } else {
        batteryStats.weightedEstimateMins = batteryStats.simpleEstimateMins;
    }

    // Learned estimate
    float avgDailyDrain = 0;
    int validDays = 0;

    for (int i = 0; i < 7; i++) {
        if (batteryStats.dailyAvgDrainRate[i] > 0) {
            avgDailyDrain += batteryStats.dailyAvgDrainRate[i];
            validDays++;
        }
    }

    if (validDays > 0) {
        avgDailyDrain /= validDays;
        batteryStats.avgDrainPerHour = avgDailyDrain;

        if (avgDailyDrain > 0) {
            float hoursRemaining = (remainingMAh / avgDailyDrain);
            batteryStats.learnedEstimateMins = (uint32_t)(hoursRemaining * 60.0);
        } else {
            batteryStats.learnedEstimateMins = batteryStats.weightedEstimateMins;
        }
    } else {
        batteryStats.learnedEstimateMins = batteryStats.weightedEstimateMins;
    }

    // Combined estimate
    batteryStats.combinedEstimateMins = (uint32_t)(
        (batteryStats.simpleEstimateMins * 0.2) +
        (batteryStats.weightedEstimateMins * 0.4) +
        (batteryStats.learnedEstimateMins * 0.4)
    );

    if (batteryStats.combinedEstimateMins > 2880) {
        batteryStats.combinedEstimateMins = 2880;
    }
}

void toggleBatterySaver() {
    batterySaverMode = !batterySaverMode;

    if (batterySaverMode) {
        gfx->setBrightness(50);
        USBSerial.println("[BATTERY] Saver mode ON - Brightness reduced to 50");
    } else {
        gfx->setBrightness(userData.brightness);
        USBSerial.println("[BATTERY] Saver mode OFF - Brightness restored");
    }
    
    prefs.begin("minios", false);
    prefs.putBool("batterySaver", batterySaverMode);
    prefs.end();
}

// Forward declaration
void logModeChange(BatterySaverLevel oldMode, BatterySaverLevel newMode);

// ═══════════════════════════════════════════════════════════════════════════
// APPLY BATTERY SAVER MODE - 3 MODES: OFF, MEDIUM, EXTREME
// ═══════════════════════════════════════════════════════════════════════════
void applyBatterySaverMode(BatterySaverLevel level) {
    BatterySaverLevel oldLevel = batterySaverLevel;
    
    batterySaverLevel = level;
    batterySaverMode = (level != BATTERY_SAVER_OFF);
    
    BatterySaverSettings &mode = saverModes[level];
    
    // Apply brightness
    gfx->setBrightness(mode.brightness);
    
    // Apply CPU frequency
    setCpuFrequencyMhz(mode.cpuFreqMhz);
    
    // Apply sensor poll interval based on mode
    switch (level) {
        case BATTERY_SAVER_OFF:
            sensorPollInterval = SENSOR_POLL_NORMAL_MS;
            lightSleepEnabled = false;
            break;
        case BATTERY_SAVER_MEDIUM:
            sensorPollInterval = SENSOR_POLL_SAVER_MS;
            lightSleepEnabled = false;
            break;
        case BATTERY_SAVER_EXTREME:
            sensorPollInterval = 500;
            lightSleepEnabled = shouldEnableLightSleep() && mode.enableLightSleep;
            if (lightSleepEnabled) {
                USBSerial.println("[POWER] Light sleep ENABLED (Extreme mode)");
            }
            break;
    }
    
    // Handle BLE auto-off
    if (mode.bleAutoOff && bleEnabled && !bleDeviceConnected) {
        // Will auto-off via timer, no immediate action needed
        USBSerial.println("[POWER] BLE auto-off enabled for this mode");
    }
    
    // Disable WiFi in extreme mode
    if (mode.disableWifiSync) {
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.mode(WIFI_OFF);
            wifiConnected = false;
            USBSerial.println("[POWER] WiFi disabled to save battery");
        }
    }
    
    // Log mode change
    if (oldLevel != level) {
        logModeChange(oldLevel, level);
    }
    
    // Save preference
    prefs.begin("minios", false);
    prefs.putUChar("saverLevel", (uint8_t)level);
    prefs.end();
    
    USBSerial.printf("[POWER] Mode: %s | CPU: %dMHz | Bright: %d | Timeout: %ds | Sleep: %s\n", 
        mode.name, getCpuFrequencyMhz(), mode.brightness, 
        mode.screenTimeoutMs / 1000, lightSleepEnabled ? "ON" : "off");
}

void logModeChange(BatterySaverLevel oldMode, BatterySaverLevel newMode) {
    currentPowerSession.modeChanges++;
    USBSerial.printf("[POWER] Mode change: %s -> %s\n", 
        saverModes[oldMode].name, saverModes[newMode].name);
}

// Forward declarations for deep sleep
void saveUserData();
void stopBLE();

// ═══════════════════════════════════════════════════════════════════════════
// DEEP SLEEP MODE - Ultra low power (tap to wake)
// ═══════════════════════════════════════════════════════════════════════════
void enterDeepSleep() {
    USBSerial.println("[POWER] Entering DEEP SLEEP (tap to wake)...");
    USBSerial.flush();
    
    inDeepSleep = true;
    
    // Save any pending data
    saveUserData();
    
    // Turn off display completely
    gfx->setBrightness(0);
    gfx->displayOff();
    
    // Disable WiFi and BLE
    WiFi.mode(WIFI_OFF);
    if (bleEnabled) {
        stopBLE();
    }
    
    // Configure touch pin as wake source
    esp_sleep_enable_ext0_wakeup((gpio_num_t)TP_INT, 0);  // Wake on touch (LOW)
    
    // Also wake on power button
    esp_sleep_enable_ext1_wakeup(1ULL << PWR_BUTTON, ESP_EXT1_WAKEUP_ALL_LOW);
    
    // Enter deep sleep
    esp_deep_sleep_start();
    
    // Never reaches here - device resets on wake
}

void checkDeepSleepTimeout() {
    // Don't enter deep sleep if recording voice memo
    if (voiceMemoRecording) return;
    
    // Don't enter deep sleep if BLE connected
    if (bleDeviceConnected) return;
    
    // Check if 4 minutes passed since last touch
    if (millis() - lastTouchTime > DEEP_SLEEP_TIMEOUT_MS) {
        enterDeepSleep();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// AMOLED BURN-IN PREVENTION - Pixel shifting
// ═══════════════════════════════════════════════════════════════════════════
void updateBurnInOffset() {
    if (millis() - lastBurnInShift < BURN_IN_SHIFT_INTERVAL_MS) return;
    
    lastBurnInShift = millis();
    
    // Shift in a small pattern to prevent burn-in
    static int8_t shiftPhase = 0;
    shiftPhase = (shiftPhase + 1) % 8;
    
    // Create a subtle shifting pattern
    switch (shiftPhase) {
        case 0: burnInOffsetX = 0;  burnInOffsetY = 0;  break;
        case 1: burnInOffsetX = 1;  burnInOffsetY = 0;  break;
        case 2: burnInOffsetX = 2;  burnInOffsetY = 1;  break;
        case 3: burnInOffsetX = 1;  burnInOffsetY = 2;  break;
        case 4: burnInOffsetX = 0;  burnInOffsetY = 2;  break;
        case 5: burnInOffsetX = -1; burnInOffsetY = 1;  break;
        case 6: burnInOffsetX = -2; burnInOffsetY = 0;  break;
        case 7: burnInOffsetX = -1; burnInOffsetY = -1; break;
    }
}

int8_t getBurnInOffsetX() { return burnInOffsetX; }
int8_t getBurnInOffsetY() { return burnInOffsetY; }

// ═══════════════════════════════════════════════════════════════════════════
// POWER SAVING MODE CONTROL
// ═══════════════════════════════════════════════════════════════════════════
void applyPowerSavingMode(bool enable) {
    powerSaverActive = enable;
    
    if (enable) {
        setCpuFrequencyMhz(CPU_FREQ_SAVER);
        sensorPollInterval = SENSOR_POLL_SAVER_MS;
    } else {
        setCpuFrequencyMhz(CPU_FREQ_NORMAL);
        sensorPollInterval = SENSOR_POLL_NORMAL_MS;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// SCREEN ON/OFF FUNCTIONS - ENHANCED WITH LIGHT SLEEP
// ═══════════════════════════════════════════════════════════════════════════
void setTouchEnabled(bool enabled) {
    touchEnabled = enabled;
}

void screenOff() {
    if (!screenOn) return;
    
    screenOn = false;
    screenOffStartMs = millis();
    batteryStats.screenOnTimeMs += (screenOffStartMs - screenOnStartMs);
    
    gfx->setBrightness(0);
    
    // In Medium or Extreme modes, apply power saving
    if (batterySaverLevel >= BATTERY_SAVER_MEDIUM) {
        applyPowerSavingMode(true);
    }
    
    // If in Extreme mode AND battery low, enter light sleep
    if (shouldEnableLightSleep()) {
        USBSerial.println("[POWER] Entering light sleep (EXTREME + low battery)...");
        enterLightSleep();
    }
    
    ui_event = UI_EVENT_SCREEN_OFF;
}

void screenOnFunc() {
    if (screenOn) return;
    
    // If waking from light sleep, recovery is already done
    if (currentPowerState == POWER_STATE_WAKING) {
        currentPowerState = POWER_STATE_ACTIVE;
    }
    
    screenOn = true;
    screenOnStartMs = millis();
    batteryStats.screenOffTimeMs += (screenOnStartMs - screenOffStartMs);
    lastActivityMs = millis();
    last_ui_activity = millis();  // Reset activity timer to prevent immediate screen-off
    
    // Restore brightness based on saver level
    gfx->setBrightness(saverModes[batterySaverLevel].brightness);
    
    // Restore CPU if power saving was active
    if (powerSaverActive && batterySaverLevel != BATTERY_SAVER_EXTREME) {
        applyPowerSavingMode(false);
    }
    
    setTouchEnabled(true);
    
    // Refresh the current screen
    navigateTo(currentCategory, currentSubCard);
    
    ui_event = UI_EVENT_SCREEN_ON;
}

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY WARNINGS - ENHANCED WITH AUTO EXTREME MODE
// ═══════════════════════════════════════════════════════════════════════════
void checkBatteryWarnings() {
    if (batteryPercent <= CRITICAL_BATTERY_WARNING && !criticalBatteryWarningShown && !isCharging) {
        criticalBatteryWarningShown = true;
        showingLowBatteryPopup = true;
        lowBatteryPopupTime = millis();
        
        // Auto-enable Extreme saver at critical battery
        if (batterySaverLevel != BATTERY_SAVER_EXTREME) {
            applyBatterySaverMode(BATTERY_SAVER_EXTREME);
            batterySaverAutoEnabled = true;
            USBSerial.println("[BATTERY] CRITICAL! Auto-enabled EXTREME saver with light sleep");
        }
        
    } else if (batteryPercent <= LOW_BATTERY_WARNING && !lowBatteryWarningShown && !isCharging) {
        lowBatteryWarningShown = true;
        showingLowBatteryPopup = true;
        lowBatteryPopupTime = millis();
        
        // Check if we should enable light sleep now
        if (batterySaverLevel == BATTERY_SAVER_EXTREME && !lightSleepEnabled) {
            lightSleepEnabled = true;
            USBSerial.println("[BATTERY] LOW! Light sleep now enabled in EXTREME mode");
        }
    }
    
    // Reset warnings when charging or battery recovers
    if (isCharging || batteryPercent > LOW_BATTERY_WARNING) {
        lowBatteryWarningShown = false;
        criticalBatteryWarningShown = false;
        
        // Disable auto-enabled battery saver when charging
        if (isCharging && batterySaverAutoEnabled) {
            batterySaverAutoEnabled = false;
        }
    }
    
    // Auto-dismiss popup after 3 seconds
    if (showingLowBatteryPopup && (millis() - lowBatteryPopupTime > 3000)) {
        showingLowBatteryPopup = false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

void disableAllScrolling(lv_obj_t *obj) {
    if (obj == NULL) return;
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

float getEstimatedCurrentMA() {
    switch (currentPowerState) {
        case POWER_STATE_ACTIVE:
            return 80.0 + (saverModes[batterySaverLevel].brightness / 255.0) * 50.0;
        case POWER_STATE_DIMMED:
            return 40.0;
        case POWER_STATE_SCREEN_OFF:
            return 15.0;
        case POWER_STATE_LIGHT_SLEEP:
            return LIGHT_SLEEP_CURRENT_MA;
        default:
            return 50.0;
    }
}

uint32_t getEstimatedBatteryMinutes() {
    float remainingMAh = (batteryPercent / 100.0) * BATTERY_CAPACITY_MAH;
    float currentMA = getEstimatedCurrentMA();
    
    if (currentMA <= 0) return 0;
    
    return (uint32_t)((remainingMAh / currentMA) * 60.0);
}

void printPowerStats() {
    USBSerial.println("\n═══ POWER STATISTICS ═══");
    USBSerial.printf("Power State: %d\n", currentPowerState);
    USBSerial.printf("Saver Level: %s\n", saverModes[batterySaverLevel].name);
    USBSerial.printf("Battery: %d%% (%dmV)\n", batteryPercent, batteryVoltage);
    USBSerial.printf("Charging: %s\n", isCharging ? "Yes" : "No");
    USBSerial.printf("Light Sleep: %s\n", lightSleepEnabled ? "ENABLED" : "disabled");
    USBSerial.printf("Est Current: %.1f mA\n", getEstimatedCurrentMA());
    USBSerial.printf("Est Remaining: %lu min\n", getEstimatedBatteryMinutes());
    USBSerial.printf("CPU Freq: %d MHz\n", getCpuFrequencyMhz());
    USBSerial.printf("Steps: %lu\n", userData.steps);
    USBSerial.printf("Light Sleep Time: %lu ms\n", batteryStats.lightSleepTimeMs);
    USBSerial.println("═════════════════════════\n");
}

void calculateSunTimes() {
    RTC_DateTime dt = rtc.getDateTime();
    int dayOfYear = dt.getDay();
    
    float seasonOffset = sin((dayOfYear - 80) * 3.14159 / 182.5) * 1.5;
    
    sunriseHour = 6.0 - seasonOffset;
    sunsetHour = 18.5 + seasonOffset;
    
    if (sunriseHour < 4.5) sunriseHour = 4.5;
    if (sunriseHour > 7.5) sunriseHour = 7.5;
    if (sunsetHour < 16.5) sunsetHour = 16.5;
    if (sunsetHour > 21.0) sunsetHour = 21.0;
}

// ═══════════════════════════════════════════════════════════════════════════
// FORWARD DECLARATIONS - UI Functions (FROM v7.2)
// ═══════════════════════════════════════════════════════════════════════════
void navigateTo(int category, int subCard);
void handleSwipe(int dx, int dy);
void handleTap(int x, int y);
void saveUserData();
void loadUserData();
void shutdownDevice();
void drawNikeSwoosh(lv_obj_t *parent, int cx, int cy, int size, uint32_t color);
void createClockCard();
void createCompassCard();
void createTiltCard();
void createStepsCard();
void createDistanceCard();
void createBlackjackCard();
void createWeatherCard();
void createForecastCard();
void createStopwatchCard();
void createTorchCard();
void createTorchSettingsCard();
void createCalculatorCard();
void createSettingsCard();
void createBatteryCard();
void createBatteryStatsCard();
void createUsagePatternsCard();
void createSDCardHealthCard();
void createPowerStatsCard();
void createAboutCard();
void createBatterySaverCard();
void createTxtFilesCard();
void createWiFiCard();
void connectToHardcodedWiFi();
void scanWiFiNetworks();
void fetchLocationFromIP();
void updateSDCardHealth();
void displayWallpaperImage(lv_obj_t *parent, int wallpaperIndex);

// Battery estimation functions
void handleSettingsTap(int x, int y);
void handleBluetoothTap(int x, int y);
void handleNotificationsTap(int x, int y);
void drawBLEIndicator(lv_obj_t *parent);

// TXT File functions
void loadTxtFileList();
void loadTxtFileContent(int index);
void createSampleTxtFile();

// TIME BACKUP functions
void saveTimeBackup();
void restoreTimeBackup();
bool hasTimeBackup();

// Power Logging functions
void initPowerLogging();
void logPowerConsumption();
void writePowerSummary();
String formatPowerLogEntry(PowerLogEntry &entry);
void rotatePowerLogIfNeeded();
void archiveDailyPowerLog();
void writeDailyStats();

// Blackjack game functions
void drawPlayingCard(lv_obj_t *parent, int cardValue, int x, int y, bool faceUp);
int calculateHandValue(int *cards, int count);
void dealCard(int *cards, int *count, bool toPlayer);
void startNewBlackjackGame();
void playerHit();
void playerStandAction();

// SD Card functions
bool createDirectoryIfNotExists(const char* path);
void logToBootLog(const char* message);
bool createWidgetOSFolderStructure();
bool initWidgetOSSDCard();
bool loadWiFiConfigFromSD();
void checkAutoBackup();
bool createBackup(bool isAuto);
bool restoreFromBackup(const String& backupName);
void handleFusionLabsProtocol();
void sendDeviceStatus();
void sendSDHealth();

static lv_obj_t *current_screen_obj = NULL;
// ═══════════════════════════════════════════════════════════════════════════
// DISPLAY DRIVER
// ═══════════════════════════════════════════════════════════════════════════
static void lvgl_tick_cb(void *arg) {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    int32_t x = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    int32_t y = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
    uint8_t fingers = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

    if (fingers > 0) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
        ui_activity_ping();
        lastActivityMs = millis();  // Keep screen alive on any touch

        // WAKE SCREEN ON TOUCH
        if (!screenOn) {
            ui_event = UI_EVENT_SCREEN_ON;
            return;  // Don't process touch as navigation when waking
        }

        if (!touchActive) {
            touchActive = true;
            touchStartX = x;
            touchStartY = y;
            touchStartMs = millis();
            USBSerial.printf("[TOUCH] Start at (%d, %d)\n", x, y);  // Debug
        }
        touchCurrentX = x;
        touchCurrentY = y;
    } else {
        data->state = LV_INDEV_STATE_REL;

        if (touchActive) {
            touchActive = false;
            unsigned long touchDuration = millis() - touchStartMs;
            int32_t dx = touchCurrentX - touchStartX;
            int32_t dy = touchCurrentY - touchStartY;

            USBSerial.printf("[TOUCH] End - dx=%d dy=%d duration=%lu\n", dx, dy, touchDuration);  // Debug

            if (touchDuration < SWIPE_MAX_DURATION) {
                if (abs(dx) > SWIPE_THRESHOLD_MIN && abs(dx) > abs(dy)) {
                    USBSerial.printf("[TOUCH] Swipe %s\n", (dx > 0) ? "LEFT" : "RIGHT");
                    ui_event = (dx > 0) ? UI_EVENT_NAV_LEFT : UI_EVENT_NAV_RIGHT;
                } else if (abs(dy) > SWIPE_THRESHOLD_MIN && abs(dy) > abs(dx)) {
                    USBSerial.printf("[TOUCH] Swipe %s\n", (dy > 0) ? "UP" : "DOWN");
                    ui_event = (dy > 0) ? UI_EVENT_NAV_UP : UI_EVENT_NAV_DOWN;
                } else if (abs(dx) < TAP_THRESHOLD && abs(dy) < TAP_THRESHOLD) {
                    USBSerial.printf("[TOUCH] Tap at (%d, %d)\n", touchCurrentX, touchCurrentY);
                    ui_event = UI_EVENT_TAP;
                    ui_event_param1 = touchCurrentX;
                    ui_event_param2 = touchCurrentY;
                }
            }
        }
    }
}

void shutdownDevice() {
    saveUserData();
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Shutting down...");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    lv_task_handler();
    delay(1000);

    gfx->setBrightness(0);
    if (hasPMU) power.shutdown();
    esp_deep_sleep_start();
}

// ═══════════════════════════════════════════════════════════════════════════
// WALLPAPER IMAGE DISPLAY (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void displayWallpaperImage(lv_obj_t *parent, int wallpaperIndex) {
    if (wallpaperIndex == 0) {
        return;  // Solid color - use current gradient theme
    }

    lv_obj_t *img = lv_img_create(parent);

    switch(wallpaperIndex) {
        case 1:
            lv_img_set_src(img, &AdobeStock_17557);
            break;
        case 2:
            lv_img_set_src(img, &AdobeStock_2026);
            break;
        case 3:
            lv_img_set_src(img, &AdobeStock_184869446);
            break;
        case 4:
            lv_img_set_src(img, &AdobeStock_174564);
            break;
        default:
            lv_obj_del(img);
            return;
    }

    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img, LCD_WIDTH - 24, LCD_HEIGHT - 60);
}

// ═══════════════════════════════════════════════════════════════════════════
// NAVIGATION (FROM FIXED - STABLE)
// ═══════════════════════════════════════════════════════════════════════════
bool canNavigate() {
    if (isTransitioning) return false;
    if (millis() - lastNavigationMs < NAVIGATION_COOLDOWN_MS) return false;
    return true;
}

void navigateTo(int category, int subCard) {
    if (!canNavigate() && (category != currentCategory || subCard != currentSubCard)) {
        return;
    }

    navigationLocked = true;
    lastNavigationMs = millis();

    currentCategory = category;
    currentSubCard = subCard;

    lv_obj_clean(lv_scr_act());

    switch (category) {
        case CAT_CLOCK:
            createClockCard();  // Only clock card now (no time settings)
            break;
        case CAT_COMPASS:
            if (subCard == 0) createCompassCard();
            else if (subCard == 1) createTiltCard();
            else createCompassCard();
            break;
        case CAT_ACTIVITY:
            if (subCard == 0) createStepsCard();
            else if (subCard == 1) createDistanceCard();
            else if (subCard == 2) createRunningCard();  // NEW: Running pace
            else createStepsCard();
            break;
        case CAT_GAMES:
            if (subCard == 0) createBlackjackCard();
            else if (subCard == 1) createDiceRollerCard();   // NEW: Dice Roller
            else if (subCard == 2) createMagic8BallCard();   // NEW: Magic 8 Ball
            else createBlackjackCard();
            break;
        case CAT_WEATHER:
            if (subCard == 0) createWeatherCard();
            else createForecastCard();
            break;
        case CAT_TIMER:
            createStopwatchCard();
            break;
        case CAT_TORCH:
            if (subCard == 0) createTorchCard();
            else createTorchSettingsCard();
            break;
        case CAT_TOOLS:
            if (subCard == 0) createCalculatorCard();
            else if (subCard == 1) createTallyCounterCard();  // NEW: Tally Counter
            else if (subCard == 2) createVoiceMemoCard();     // NEW: Voice Memo
            else if (subCard == 3) createTxtFilesCard();      // TXT Files
            else createCalculatorCard();
            break;
        case CAT_BLUETOOTH:
            if (subCard == 0) createBluetoothCard();
            else if (subCard == 1) createNotificationsCard();
            else createBluetoothCard();
            break;
        case CAT_SETTINGS:
            if (subCard == 0) createSettingsCard();
            else if (subCard == 1) createWiFiCard();  // WiFi moved to Settings
            else createSettingsCard();
            break;
        case CAT_SYSTEM:
            if (subCard == 0) createBatteryCard();
            else if (subCard == 1) createBatterySaverCard();  // NEW: Battery Saver Modes
            else if (subCard == 2) createBatteryStatsCard();
            else if (subCard == 3) createUsagePatternsCard();
            else if (subCard == 4) createSDCardHealthCard();
            else if (subCard == 5) createPowerStatsCard();  // NEW: Power Stats from logs
            else createBatteryCard();
            break;
        case CAT_ABOUT:
            createAboutCard();  // NEW: Software Info Card
            break;
        default:
            createClockCard();
            break;
    }

    lv_refr_now(NULL);
    lastActivityMs = millis();
    ui_activity_ping();
    navigationLocked = false;
}

void handleSwipe(int dx, int dy) {
    if (isTransitioning || millis() - lastNavigationMs < NAVIGATION_COOLDOWN_MS) return;
    
    lastTouchTime = millis();  // Reset deep sleep timer

    if (showingLowBatteryPopup) {
        showingLowBatteryPopup = false;
        navigateTo(currentCategory, currentSubCard);
        lastNavigationMs = millis();
        return;
    }

    int newCategory = currentCategory;
    int newSubCard = currentSubCard;

    if (abs(dx) > abs(dy) && abs(dx) > SWIPE_THRESHOLD_MIN) {
        if (dx < 0) {
            newCategory = (currentCategory + 1) % NUM_CATEGORIES;
        } else {
            newCategory = currentCategory - 1;
            if (newCategory < 0) newCategory = NUM_CATEGORIES - 1;
        }
        newSubCard = 0;
    } else if (abs(dy) > abs(dx) && abs(dy) > SWIPE_THRESHOLD_MIN) {
        if (dy > 0 && currentSubCard < maxSubCards[currentCategory] - 1) {
            newSubCard = currentSubCard + 1;
        } else if (dy < 0 && currentSubCard > 0) {
            newSubCard = currentSubCard - 1;
        }
    }

    if (newCategory != currentCategory || newSubCard != currentSubCard) {
        currentCategory = newCategory;
        currentSubCard = newSubCard;
        navigateTo(currentCategory, currentSubCard);
        lastNavigationMs = millis();
    }
}

void handleTap(int x, int y) {
    ui_activity_ping();
    lastActivityMs = millis();
    lastTouchTime = millis();  // Reset deep sleep timer

    // DEBUG: Always print current state
    USBSerial.printf("[TAP] x=%d y=%d cat=%d sub=%d\n", x, y, currentCategory, currentSubCard);

    // ═══ CLOCK SCREEN - TAP TO CYCLE! ═══
    if (currentCategory == CAT_CLOCK && currentSubCard == 0) {
        // Nike face: LEFT = cycle colors, RIGHT = cycle faces
        if (userData.watchFaceIndex == 3) {  // Nike face
            if (x < LCD_WIDTH / 2) {
                // LEFT HALF - cycle through Nike color variants
                currentNikeColor = (currentNikeColor + 1) % NUM_NIKE_COLORS;
                USBSerial.printf("[NIKE] Color: %s (%d/%d)\n", 
                                nikeColors[currentNikeColor].name, 
                                currentNikeColor + 1, NUM_NIKE_COLORS);
                saveUserData();
                navigateTo(currentCategory, currentSubCard);
                return;
            }
            // RIGHT HALF - fall through to cycle watch faces
        }
        // Cycle through watch faces (right side of Nike, or any tap on other faces)
        userData.watchFaceIndex = (userData.watchFaceIndex + 1) % NUM_WATCH_FACES;
        saveUserData();
        USBSerial.printf("[CLOCK] Face: %s (%d/%d)\n", 
                        watchFaces[userData.watchFaceIndex].name, 
                        userData.watchFaceIndex + 1, NUM_WATCH_FACES);
        navigateTo(currentCategory, currentSubCard);
        return;
    }
    
    if (currentCategory == CAT_TORCH && currentSubCard == 0) {
        torchOn = !torchOn;
        if (torchOn) {
            gfx->setBrightness(torchBrightness);
        } else {
            gfx->setBrightness(userData.brightness);
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_TIMER && currentSubCard == 0) {
        // Stopwatch tap handling
        static unsigned long lastStopwatchTap = 0;
        unsigned long now = millis();
        
        // Double tap to reset
        if (now - lastStopwatchTap < 400 && !stopwatchRunning) {
            // Reset stopwatch
            stopwatchElapsedMs = 0;
            lapCount = 0;
        } else {
            // Single tap to start/stop
            if (stopwatchRunning) {
                stopwatchRunning = false;
                stopwatchElapsedMs += (now - stopwatchStartMs);
                // Record lap
                if (lapCount < MAX_LAPS) {
                    lapTimes[lapCount++] = stopwatchElapsedMs;
                }
            } else {
                stopwatchRunning = true;
                stopwatchStartMs = now;
            }
        }
        lastStopwatchTap = now;
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_GAMES) {
        if (currentSubCard == 0) {
            // BLACKJACK TAP HANDLING - Fully functional
            if (!blackjackGameActive) {
                // Start new game
                startNewBlackjackGame();
            } else {
                int playerVal = calculateHandValue(playerCards, playerCount);
                
                if (playerVal > 21) {
                    // Bust - start new game
                    blackjackGameActive = false;
                    startNewBlackjackGame();
                } else if (!playerStand) {
                    // Check which button was tapped
                    if (y > LCD_HEIGHT - 100) {
                        // Bottom area - buttons
                        if (x < LCD_WIDTH / 2) {
                            // HIT button (left side)
                            playerHit();
                        } else {
                            // STAND button (right side)
                            playerStandAction();
                        }
                    }
                } else {
                    // Game over - tap to start new game
                    blackjackGameActive = false;
                    startNewBlackjackGame();
                }
            }
        } else if (currentSubCard == 1) {
            // DICE ROLLER - shake or tap to roll
            diceRolling = true;
            diceRollStart = millis();
            diceValue1 = random(1, 7);
            diceValue2 = random(1, 7);
        } else if (currentSubCard == 2) {
            // MAGIC 8 BALL - shake or tap for answer
            magic8Shaking = true;
            magic8BallAnswer = magic8Answers[random(NUM_8BALL_ANSWERS)];
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_TOOLS && currentSubCard == 0) {
        // CALCULATOR TAP HANDLING
        int btnSize = 48;
        int btnSpacing = 4;
        int startY = 110;
        int startX = 15;
        
        // Determine which button was tapped
        if (y >= startY && y < startY + 5 * (btnSize + btnSpacing)) {
            int row = (y - startY) / (btnSize + btnSpacing);
            int col = (x - startX) / (btnSize + btnSpacing);
            
            if (row >= 0 && row < 5 && col >= 0 && col < 4) {
                const char* btnTexts[5][4] = {
                    {"AC", "±", "%", "÷"},
                    {"7", "8", "9", "×"},
                    {"4", "5", "6", "-"},
                    {"1", "2", "3", "+"},
                    {"0", ".", "⌫", "="}
                };
                
                const char* btn = btnTexts[row][col];
                
                if (strcmp(btn, "AC") == 0) {
                    strcpy(calcDisplay, "0");
                    calcOperator = ' ';
                    calcFirstNum = 0;
                    calcNewNumber = true;
                } else if (strcmp(btn, "⌫") == 0) {
                    int len = strlen(calcDisplay);
                    if (len > 1) {
                        calcDisplay[len - 1] = '\0';
                    } else {
                        strcpy(calcDisplay, "0");
                    }
                } else if (strcmp(btn, "±") == 0) {
                    double val = atof(calcDisplay);
                    val = -val;
                    snprintf(calcDisplay, sizeof(calcDisplay), "%g", val);
                } else if (strcmp(btn, "%") == 0) {
                    double val = atof(calcDisplay);
                    val = val / 100.0;
                    snprintf(calcDisplay, sizeof(calcDisplay), "%g", val);
                } else if (strcmp(btn, "÷") == 0 || strcmp(btn, "×") == 0 || 
                           strcmp(btn, "-") == 0 || strcmp(btn, "+") == 0) {
                    calcFirstNum = atof(calcDisplay);
                    if (strcmp(btn, "÷") == 0) calcOperator = '/';
                    else if (strcmp(btn, "×") == 0) calcOperator = '*';
                    else if (strcmp(btn, "-") == 0) calcOperator = '-';
                    else calcOperator = '+';
                    calcNewNumber = true;
                } else if (strcmp(btn, "=") == 0) {
                    double secondNum = atof(calcDisplay);
                    double result = 0;
                    if (calcOperator == '+') result = calcFirstNum + secondNum;
                    else if (calcOperator == '-') result = calcFirstNum - secondNum;
                    else if (calcOperator == '*') result = calcFirstNum * secondNum;
                    else if (calcOperator == '/' && secondNum != 0) result = calcFirstNum / secondNum;
                    else result = secondNum;
                    
                    snprintf(calcDisplay, sizeof(calcDisplay), "%g", result);
                    calcOperator = ' ';
                    calcNewNumber = true;
                } else if (strcmp(btn, ".") == 0) {
                    if (strchr(calcDisplay, '.') == NULL) {
                        strcat(calcDisplay, ".");
                    }
                } else {
                    // Number button
                    if (calcNewNumber || strcmp(calcDisplay, "0") == 0) {
                        strcpy(calcDisplay, btn);
                        calcNewNumber = false;
                    } else if (strlen(calcDisplay) < 12) {
                        strcat(calcDisplay, btn);
                    }
                }
            }
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_TOOLS && currentSubCard == 1) {
        // TALLY COUNTER
        if (y < LCD_HEIGHT / 2) {
            tallyCount++;  // Top half = increment
        } else {
            if (x < LCD_WIDTH / 2) {
                tallyCount = max(0, tallyCount - 1);  // Bottom left = decrement
            } else {
                tallyCount = 0;  // Bottom right = reset
            }
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_TOOLS && currentSubCard == 2) {
        // VOICE MEMO - tap to record/stop
        if (y > LCD_HEIGHT - 80) {
            // Button area
            if (voiceMemoRecording) {
                stopVoiceRecording();
            } else {
                startVoiceRecording();
            }
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_ACTIVITY && currentSubCard == 2) {
        // RUNNING PACE - Start/Stop button at bottom
        if (y > LCD_HEIGHT - 80) {
            if (runningModeActive) {
                // Stop running
                runningModeActive = false;
            } else {
                // Start running
                runningModeActive = true;
                runningStartTime = millis();
                runningStartSteps = userData.steps;
                runningPace = 0;
                runningDistance = 0;
            }
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_TOOLS && currentSubCard == 3) {
        // TXT Files card - handle file selection and scrolling
        if (numTxtFiles > 0 && !txtFileLoaded) {
            // Check for scroll zones first (if more than 5 files)
            if (numTxtFiles > 5) {
                if (y < 60) {
                    // Scroll up in file list
                    txtFileListOffset = max(0, txtFileListOffset - 1);
                    navigateTo(currentCategory, currentSubCard);
                    return;
                } else if (y > LCD_HEIGHT - 50) {
                    // Scroll down in file list
                    int maxScroll = max(0, numTxtFiles - 5);
                    txtFileListOffset = min(maxScroll, txtFileListOffset + 1);
                    navigateTo(currentCategory, currentSubCard);
                    return;
                }
            }
            
            // Select a file based on tap position (accounting for scroll offset)
            int fileIndex = ((y - 48) / 48) + txtFileListOffset;
            if (fileIndex >= 0 && fileIndex < numTxtFiles) {
                loadTxtFileContent(fileIndex);
            }
        } else if (txtFileLoaded) {
            // Back button area (header)
            if (y < 50) {
                txtFileLoaded = false;
                txtScrollOffset = 0;
            }
            // FAST SCROLL - 10 lines per tap (was 5)
            // Scroll down on bottom tap
            else if (y > LCD_HEIGHT - 80) {
                txtScrollOffset += 10;  // Scroll down FASTER!
            }
            // Scroll up on middle-top tap
            else if (y < LCD_HEIGHT / 2) {
                txtScrollOffset = max(0, txtScrollOffset - 10);  // Scroll up FASTER!
            }
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_BLUETOOTH) {
        if (currentSubCard == 0) {
            // Bluetooth Card tap handling
            handleBluetoothTap(x, y);
        } else if (currentSubCard == 1) {
            // Notifications Card tap handling
            handleNotificationsTap(x, y);
        }
    }
    else if (currentCategory == CAT_SETTINGS && currentSubCard == 1) {
        // WiFi Card tap handling (moved from Tools)
        // Button layout: status card (52-122), connect button (130-185), saved networks (212-307)
        if (y >= 130 && y <= 185) {
            // CONNECT button area - Connect to hardcoded network
            USBSerial.println("[WIFI] Connect button tapped!");
            connectToHardcodedWiFi();
        } else if (y >= 212 && y <= 307) {
            // Saved networks section - tap to connect
            int netIndex = ((y - 212) / 40) + wifiCardScrollOffset;
            
            // Check if tapping on saved networks section
            if (netIndex >= 0 && netIndex < numWifiNetworks) {
                // Connect to saved network
                WiFi.begin(wifiNetworks[netIndex].ssid, wifiNetworks[netIndex].password);
                USBSerial.printf("[WIFI] Connecting to saved: %s\n", wifiNetworks[netIndex].ssid);
            } else {
                // Check roaming networks
                int roamIdx = netIndex - numWifiNetworks;
                if (roamIdx >= 0 && roamIdx < numScannedNetworks) {
                    if (scannedNetworks[roamIdx].isOpen) {
                        WiFi.begin(scannedNetworks[roamIdx].ssid);
                        USBSerial.printf("[WIFI] Connecting to open: %s\n", scannedNetworks[roamIdx].ssid);
                    }
                }
            }
        } else if (y < 52) {
            // Top area - rescan networks
            wifiScanComplete = false;
            scanWiFiNetworks();
        }
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_COMPASS && currentSubCard == 0) {
        // Calibrate compass - set current heading as north
        compassNorthOffset = -compassHeadingSmooth;
        prefs.begin("minios", false);
        prefs.putFloat("compassOffset", compassNorthOffset);
        prefs.end();
        navigateTo(currentCategory, currentSubCard);
    }
    else if (currentCategory == CAT_SYSTEM && currentSubCard == 0) {
        // Battery card - tap to go to battery saver modes card
        if (y > LCD_HEIGHT / 2) {
            currentSubCard = 1;  // Go to battery saver modes card
            navigateTo(currentCategory, currentSubCard);
        }
    }
    else if (currentCategory == CAT_SYSTEM && currentSubCard == 1) {
        // Battery Saver Modes card - 3 OPTIONS: Off, Medium, Extreme
        int selectedMode = -1;
        // Updated Y ranges for 3 larger rows (75px each starting at y=55)
        if (y > 55 && y < 130) selectedMode = 0;        // Off (Normal)
        else if (y > 130 && y < 205) selectedMode = 1;  // Medium
        else if (y > 205 && y < 280) selectedMode = 2;  // Extreme
        
        if (selectedMode >= 0 && selectedMode <= 2) {
            applyBatterySaverMode((BatterySaverLevel)selectedMode);
            navigateTo(currentCategory, currentSubCard);
        }
    }
    else if (currentCategory == CAT_SETTINGS && currentSubCard == 0) {
        // Handle settings taps - individual buttons
        // Button layout: each button is 50px tall with 8px gap, starting at y=40
        if (y >= 40 && y < 90) {
            // WATCH FACE button
            userData.watchFaceIndex = (userData.watchFaceIndex + 1) % NUM_WATCH_FACES;
        } 
        else if (y >= 98 && y < 148) {
            // WALLPAPER button
            userData.wallpaperIndex = (userData.wallpaperIndex + 1) % NUM_GRADIENT_WALLPAPERS;
        } 
        else if (y >= 156 && y < 206) {
            // THEME button
            userData.themeIndex = (userData.themeIndex + 1) % NUM_THEMES;
        } 
        else if (y >= 214 && y < 264) {
            // BRIGHTNESS button - tap left to decrease, right to increase
            if (x < LCD_WIDTH / 2) {
                userData.brightness = max(50, userData.brightness - 25);
            } else {
                userData.brightness = min(255, userData.brightness + 25);
            }
            gfx->setBrightness(batterySaverMode ? 50 : userData.brightness);
        } 
        else if (y >= 272 && y < 320) {
            // BATTERY SAVER button
            toggleBatterySaver();
        }
        saveUserData();
        navigateTo(currentCategory, currentSubCard);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// BLE STATUS INDICATOR - Shows on non-Nike watch faces
// ═══════════════════════════════════════════════════════════════════════════
void drawBLEIndicator(lv_obj_t *parent) {
    // Only show if BLE is enabled
    if (!bleEnabled) return;
    
    // Create small BLE indicator in top-left corner
    lv_obj_t *bleIcon = lv_label_create(parent);
    lv_label_set_text(bleIcon, LV_SYMBOL_BLUETOOTH);
    
    // Color based on connection status
    if (bleDeviceConnected) {
        lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x0A84FF), 0);  // Blue when connected
    } else {
        lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x636366), 0);  // Gray when searching
    }
    
    lv_obj_set_style_text_font(bleIcon, &lv_font_montserrat_12, 0);
    lv_obj_align(bleIcon, LV_ALIGN_TOP_LEFT, 8, 8);
}

// ═══════════════════════════════════════════════════════════════════════════
// HELPER: DISABLE SCROLLING (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════

// Forward declaration for digital face
void createDigitalFace(lv_obj_t *parent);

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE 1: WORD CLOCK - "IT IS ELEVEN O'CLOCK" STYLE (THEME AWARE)
// ═══════════════════════════════════════════════════════════════════════════
void createWordClockFace(lv_obj_t *parent) {
    GradientTheme *theme = getSafeTheme();
    RTC_DateTime dt = rtc.getDateTime();
    int hour = dt.getHour();
    int minute = dt.getMinute();
    
    // BLE indicator (non-Nike face)
    drawBLEIndicator(parent);
    
    int hour12 = hour % 12;
    if (hour12 == 0) hour12 = 12;
    
    const char* hourWords[] = {"TWELVE", "ONE", "TWO", "THREE", "FOUR", "FIVE", 
                               "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE"};
    
    // Use theme accent color instead of hardcoded
    uint32_t dimColor = 0x3A3A3A;
    
    int minRounded = (minute + 2) / 5 * 5;
    if (minRounded == 60) minRounded = 0;
    
    // Determine display hour for "TO" cases
    int displayHour = hour12;
    if (minRounded > 30) displayHour = (hour12 % 12) + 1;
    if (displayHour > 12) displayHour = 1;
    
    // IT IS always highlighted
    int yPos = 35;
    lv_obj_t *itLabel = lv_label_create(parent);
    lv_label_set_text(itLabel, "IT IS");
    lv_obj_set_style_text_color(itLabel, theme->accent, 0);
    lv_obj_set_style_text_font(itLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(itLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    
    // HALF / TEN / QUARTER / TWENTY / FIVE for minutes
    yPos += 35;
    if (minRounded == 30) {
        lv_obj_t *halfLabel = lv_label_create(parent);
        lv_label_set_text(halfLabel, "HALF");
        lv_obj_set_style_text_color(halfLabel, theme->accent, 0);
        lv_obj_set_style_text_font(halfLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(halfLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    } else if (minRounded == 15 || minRounded == 45) {
        lv_obj_t *quarterLabel = lv_label_create(parent);
        lv_label_set_text(quarterLabel, "QUARTER");
        lv_obj_set_style_text_color(quarterLabel, theme->accent, 0);
        lv_obj_set_style_text_font(quarterLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(quarterLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    } else if (minRounded == 20 || minRounded == 40) {
        lv_obj_t *twentyLabel = lv_label_create(parent);
        lv_label_set_text(twentyLabel, "TWENTY");
        lv_obj_set_style_text_color(twentyLabel, theme->accent, 0);
        lv_obj_set_style_text_font(twentyLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(twentyLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    } else if (minRounded == 5 || minRounded == 55 || minRounded == 25 || minRounded == 35) {
        if (minRounded == 25 || minRounded == 35) {
            lv_obj_t *twentyLabel = lv_label_create(parent);
            lv_label_set_text(twentyLabel, "TWENTY");
            lv_obj_set_style_text_color(twentyLabel, theme->accent, 0);
            lv_obj_set_style_text_font(twentyLabel, &lv_font_montserrat_18, 0);
            lv_obj_align(twentyLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
            yPos += 35;
        }
        lv_obj_t *fiveLabel = lv_label_create(parent);
        lv_label_set_text(fiveLabel, "FIVE");
        lv_obj_set_style_text_color(fiveLabel, theme->accent, 0);
        lv_obj_set_style_text_font(fiveLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(fiveLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    } else if (minRounded == 10 || minRounded == 50) {
        lv_obj_t *tenLabel = lv_label_create(parent);
        lv_label_set_text(tenLabel, "TEN");
        lv_obj_set_style_text_color(tenLabel, theme->accent, 0);
        lv_obj_set_style_text_font(tenLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(tenLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    }
    
    // PAST / TO
    yPos += 35;
    if (minRounded > 0 && minRounded != 0) {
        const char* pastTo = (minRounded <= 30) ? "PAST" : "TO";
        lv_obj_t *pastLabel = lv_label_create(parent);
        lv_label_set_text(pastLabel, pastTo);
        lv_obj_set_style_text_color(pastLabel, theme->accent, 0);
        lv_obj_set_style_text_font(pastLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(pastLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    }
    
    // Hour word - larger and bold
    yPos += 45;
    lv_obj_t *hourLabel = lv_label_create(parent);
    lv_label_set_text(hourLabel, hourWords[displayHour]);
    lv_obj_set_style_text_color(hourLabel, theme->accent, 0);
    lv_obj_set_style_text_font(hourLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(hourLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    
    // O'CLOCK only at exact hour
    if (minRounded == 0) {
        yPos += 40;
        lv_obj_t *oclockLabel = lv_label_create(parent);
        lv_label_set_text(oclockLabel, "O'CLOCK");
        lv_obj_set_style_text_color(oclockLabel, theme->accent, 0);
        lv_obj_set_style_text_font(oclockLabel, &lv_font_montserrat_24, 0);
        lv_obj_align(oclockLabel, LV_ALIGN_TOP_LEFT, 25, yPos);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE 2: ANALOG WITH ACTIVITY RINGS
// ═══════════════════════════════════════════════════════════════════════════
void createAnalogRingsFace(lv_obj_t *parent) {
    GradientTheme *theme = getSafeTheme();
    RTC_DateTime dt = rtc.getDateTime();
    
    // BLE indicator (non-Nike face)
    drawBLEIndicator(parent);
    
    int centerX = LCD_WIDTH / 2;
    int centerY = LCD_HEIGHT / 2;
    
    // Move ring - Red (outermost)
    lv_obj_t *moveRing = lv_arc_create(parent);
    lv_obj_set_size(moveRing, 220, 220);
    lv_obj_center(moveRing);
    lv_arc_set_rotation(moveRing, 270);
    lv_arc_set_bg_angles(moveRing, 0, 360);
    lv_arc_set_range(moveRing, 0, 100);
    int moveProgress = min(100, (int)(userData.steps * 100 / userData.dailyGoal));
    lv_arc_set_value(moveRing, moveProgress);
    lv_obj_set_style_arc_width(moveRing, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(moveRing, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(moveRing, lv_color_hex(0xFF2D55), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(moveRing, lv_color_hex(0x3A1520), LV_PART_MAIN);
    lv_obj_remove_style(moveRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(moveRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Exercise ring - Green
    lv_obj_t *exerciseRing = lv_arc_create(parent);
    lv_obj_set_size(exerciseRing, 195, 195);
    lv_obj_center(exerciseRing);
    lv_arc_set_rotation(exerciseRing, 270);
    lv_arc_set_bg_angles(exerciseRing, 0, 360);
    lv_arc_set_range(exerciseRing, 0, 100);
    lv_arc_set_value(exerciseRing, 65);
    lv_obj_set_style_arc_width(exerciseRing, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(exerciseRing, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(exerciseRing, lv_color_hex(0xA2FF00), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(exerciseRing, lv_color_hex(0x1A3A00), LV_PART_MAIN);
    lv_obj_remove_style(exerciseRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(exerciseRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Stand ring - Cyan
    lv_obj_t *standRing = lv_arc_create(parent);
    lv_obj_set_size(standRing, 170, 170);
    lv_obj_center(standRing);
    lv_arc_set_rotation(standRing, 270);
    lv_arc_set_bg_angles(standRing, 0, 360);
    lv_arc_set_range(standRing, 0, 12);
    lv_arc_set_value(standRing, 8);
    lv_obj_set_style_arc_width(standRing, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(standRing, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(standRing, lv_color_hex(0x00D4FF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(standRing, lv_color_hex(0x002A3A), LV_PART_MAIN);
    lv_obj_remove_style(standRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(standRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Hour numbers 12, 3, 6, 9
    int dialRadius = 60;
    int positions[][2] = {{0, -dialRadius}, {dialRadius, 0}, {0, dialRadius}, {-dialRadius, 0}};
    const char* nums[] = {"12", "3", "6", "9"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *numLabel = lv_label_create(parent);
        lv_label_set_text(numLabel, nums[i]);
        lv_obj_set_style_text_color(numLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(numLabel, &lv_font_montserrat_14, 0);
        lv_obj_align(numLabel, LV_ALIGN_CENTER, positions[i][0], positions[i][1]);
    }
    
    // Clock hands
    int hour = dt.getHour() % 12;
    int minute = dt.getMinute();
    int second = dt.getSecond();
    
    // Hour hand
    int hourLen = 35;
    lv_obj_t *hourHand = lv_obj_create(parent);
    lv_obj_set_size(hourHand, 6, hourLen);
    lv_obj_align(hourHand, LV_ALIGN_CENTER, 0, -hourLen/2);
    lv_obj_set_style_bg_color(hourHand, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(hourHand, 3, 0);
    lv_obj_set_style_border_width(hourHand, 0, 0);
    lv_obj_set_style_transform_pivot_x(hourHand, 3, 0);
    lv_obj_set_style_transform_pivot_y(hourHand, hourLen, 0);
    lv_obj_set_style_transform_angle(hourHand, (int)((hour * 30 + minute * 0.5) * 10), 0);
    disableAllScrolling(hourHand);
    
    // Minute hand
    int minLen = 50;
    lv_obj_t *minHand = lv_obj_create(parent);
    lv_obj_set_size(minHand, 4, minLen);
    lv_obj_align(minHand, LV_ALIGN_CENTER, 0, -minLen/2);
    lv_obj_set_style_bg_color(minHand, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(minHand, 2, 0);
    lv_obj_set_style_border_width(minHand, 0, 0);
    lv_obj_set_style_transform_pivot_x(minHand, 2, 0);
    lv_obj_set_style_transform_pivot_y(minHand, minLen, 0);
    lv_obj_set_style_transform_angle(minHand, minute * 60, 0);
    disableAllScrolling(minHand);
    
    // Second hand (red)
    int secLen = 55;
    lv_obj_t *secHand = lv_obj_create(parent);
    lv_obj_set_size(secHand, 2, secLen);
    lv_obj_align(secHand, LV_ALIGN_CENTER, 0, -secLen/2);
    lv_obj_set_style_bg_color(secHand, lv_color_hex(0xFF2D55), 0);
    lv_obj_set_style_radius(secHand, 1, 0);
    lv_obj_set_style_border_width(secHand, 0, 0);
    lv_obj_set_style_transform_pivot_x(secHand, 1, 0);
    lv_obj_set_style_transform_pivot_y(secHand, secLen, 0);
    lv_obj_set_style_transform_angle(secHand, second * 60, 0);
    disableAllScrolling(secHand);
    
    // Center dot
    lv_obj_t *centerDot = lv_obj_create(parent);
    lv_obj_set_size(centerDot, 10, 10);
    lv_obj_center(centerDot);
    lv_obj_set_style_bg_color(centerDot, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(centerDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(centerDot, 0, 0);
    disableAllScrolling(centerDot);
    
    // Day and date at bottom
    char dateBuf[16];
    const char* dayNames[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    snprintf(dateBuf, sizeof(dateBuf), "%s %d", dayNames[dt.getWeek()], dt.getDay());
    lv_obj_t *dateLabel = lv_label_create(parent);
    lv_label_set_text(dateLabel, dateBuf);
    lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(dateLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE 3: NIKE STYLE - AUTHENTIC APPLE WATCH NIKE FACE
// ═══════════════════════════════════════════════════════════════════════════

// Helper: Draw Nike swoosh - ENHANCED authentic curved tick shape
void drawNikeSwoosh(lv_obj_t *parent, int cx, int cy, int size, uint32_t color) {
    // AUTHENTIC Nike swoosh - simple and clean like reference image
    // Main curved body
    lv_obj_t *swoosh = lv_obj_create(parent);
    lv_obj_set_size(swoosh, size, size / 2.5);
    lv_obj_set_pos(swoosh, cx - size/2, cy);
    lv_obj_set_style_bg_color(swoosh, lv_color_hex(color), 0);
    lv_obj_set_style_radius(swoosh, size / 4, 0);
    lv_obj_set_style_border_width(swoosh, 0, 0);
    lv_obj_set_style_transform_angle(swoosh, -150, 0);
    disableAllScrolling(swoosh);
    
    // Tail pointing up-right
    lv_obj_t *tail = lv_obj_create(parent);
    lv_obj_set_size(tail, size * 0.7, size / 4);
    lv_obj_set_pos(tail, cx + size/5, cy - size/2);
    lv_obj_set_style_bg_color(tail, lv_color_hex(color), 0);
    lv_obj_set_style_radius(tail, size / 8, 0);
    lv_obj_set_style_border_width(tail, 0, 0);
    lv_obj_set_style_transform_angle(tail, 300, 0);
    disableAllScrolling(tail);
}

// Helper: Draw large Nike-style digit with bold italic effect
// Nike Globe font: Bold, condensed, slightly italic, with depth
void drawNikeDigit(lv_obj_t *parent, const char* digit, int x, int y, 
                   uint32_t fillColor, uint32_t shadowColor, int digitSize) {
    // Use largest available font and scale effect
    const lv_font_t *font = &lv_font_montserrat_48;
    
    // Shadow layer (offset for 3D depth effect)
    lv_obj_t *shadow = lv_label_create(parent);
    lv_label_set_text(shadow, digit);
    lv_obj_set_style_text_color(shadow, lv_color_hex(shadowColor), 0);
    lv_obj_set_style_text_font(shadow, font, 0);
    lv_obj_set_pos(shadow, x + 4, y + 4);
    
    // Main digit (front layer)
    lv_obj_t *main = lv_label_create(parent);
    lv_label_set_text(main, digit);
    lv_obj_set_style_text_color(main, lv_color_hex(fillColor), 0);
    lv_obj_set_style_text_font(main, font, 0);
    lv_obj_set_pos(main, x, y);
}

void createNikeStyleFace(lv_obj_t *parent) {
    RTC_DateTime dt = rtc.getDateTime();
    
    // Get current Nike color scheme
    NikeColorScheme colors = nikeColors[currentNikeColor];
    uint32_t textColor = colors.primary;
    uint32_t shadowColor = colors.secondary;
    uint32_t bgColor = colors.background;
    uint32_t swooshColor = colors.swoosh;  // NEW: Get swoosh color from scheme
    bool isFullColor = colors.fullColor;
    bool hasDial = colors.hasDial;          // NEW: Check if variant has dial
    
    // ═══ BACKGROUND ═══
    if (isFullColor) {
        // Solid color background (Volt, Purple, Orange, Pink, White variants)
        lv_obj_set_style_bg_color(parent, lv_color_hex(bgColor), 0);
        lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    } else {
        // Pure black AMOLED background (Blue variant)
        lv_obj_set_style_bg_color(parent, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    }
    
    // Blue variant has circular dial with complications
    // Use hasDial from color scheme (Blue Dial variant)
    bool hasCircularDial = hasDial;
    
    if (hasCircularDial) {
        // ════════════════════════════════════════════════════════════════
        // BLUE DIAL VARIANT - THICK TICK MARKS + BIG NUMBERS (Reference!)
        // ════════════════════════════════════════════════════════════════
        
        int centerX = LCD_WIDTH / 2;
        int centerY = LCD_HEIGHT / 2;
        
        // Outer dial ring - THICKER (was 1px, now 2px)
        lv_obj_t *dialRing = lv_arc_create(parent);
        lv_obj_set_size(dialRing, LCD_WIDTH - 10, LCD_HEIGHT - 10);
        lv_arc_set_rotation(dialRing, 270);
        lv_arc_set_bg_angles(dialRing, 0, 360);
        lv_arc_set_angles(dialRing, 0, 0);
        lv_obj_set_style_arc_color(dialRing, lv_color_hex(0x4A4A4A), LV_PART_MAIN);
        lv_obj_set_style_arc_width(dialRing, 2, LV_PART_MAIN);
        lv_obj_remove_style(dialRing, NULL, LV_PART_INDICATOR);
        lv_obj_remove_style(dialRing, NULL, LV_PART_KNOB);
        lv_obj_center(dialRing);
        
        // Draw ONLY major tick marks (12 ticks instead of 60 for performance!)
        int outerRadius = (LCD_WIDTH - 14) / 2;
        
        for (int i = 0; i < 12; i++) {
            float angle = (i * 30 - 90) * 3.14159 / 180.0;
            int tickLen = 14;
            int tickWidth = 4;
            int innerRadius = outerRadius - tickLen;
            
            int x1 = centerX + (int)(cos(angle) * outerRadius);
            int y1 = centerY + (int)(sin(angle) * outerRadius);
            int x2 = centerX + (int)(cos(angle) * innerRadius);
            int y2 = centerY + (int)(sin(angle) * innerRadius);
            
            lv_obj_t *tick = lv_obj_create(parent);
            lv_obj_set_size(tick, tickWidth, tickLen);
            lv_obj_set_pos(tick, (x1 + x2) / 2 - tickWidth/2, (y1 + y2) / 2 - tickLen / 2);
            lv_obj_set_style_bg_color(tick, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(tick, LV_OPA_90, 0);
            lv_obj_set_style_radius(tick, 1, 0);
            lv_obj_set_style_border_width(tick, 0, 0);
            disableAllScrolling(tick);
        }
        
        // Minute numbers around dial (reduced to 4 for performance)
        const char* dialNums[] = {"15", "30", "45", "00"};
        int dialPositions[] = {90, 180, 270, 0};
        for (int i = 0; i < 4; i++) {
            float angle = (dialPositions[i] - 90) * 3.14159 / 180.0;
            int numRadius = outerRadius - 22;
            int nx = centerX + (int)(cos(angle) * numRadius);
            int ny = centerY + (int)(sin(angle) * numRadius);
            
            lv_obj_t *numLabel = lv_label_create(parent);
            lv_label_set_text(numLabel, dialNums[i]);
            lv_obj_set_style_text_color(numLabel, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(numLabel, &lv_font_montserrat_12, 0);
            lv_obj_set_pos(numLabel, nx - 8, ny - 6);
        }
        
        // Date numbers on right side (19, 20, 21 for calendar)
        int dayNum = dt.getDay();
        char dateBuf[4];
        for (int i = -1; i <= 1; i++) {
            snprintf(dateBuf, sizeof(dateBuf), "%d", dayNum + i);
            lv_obj_t *dateLabel = lv_label_create(parent);
            lv_label_set_text(dateLabel, dateBuf);
            uint32_t dateColor = (i == 0) ? 0xFF3B30 : 0x8E8E93;  // Today = red
            lv_obj_set_style_text_color(dateLabel, lv_color_hex(dateColor), 0);
            lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_12, 0);
            lv_obj_align(dateLabel, LV_ALIGN_RIGHT_MID, -20, -20 + (i * 18));
        }
        
        // Temperature - top left
        char tempBuf[8];
        snprintf(tempBuf, sizeof(tempBuf), "%d°", (int)weatherTemp);
        lv_obj_t *tempLabel = lv_label_create(parent);
        lv_label_set_text(tempLabel, tempBuf);
        lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 25, 22);
        
        // Day name - top right
        const char* shortDays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
        lv_obj_t *dayLabel = lv_label_create(parent);
        lv_label_set_text(dayLabel, shortDays[dt.getWeek()]);
        lv_obj_set_style_text_color(dayLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(dayLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(dayLabel, LV_ALIGN_TOP_RIGHT, -25, 22);
        
        // "TAP TO OPEN" - top center
        lv_obj_t *tapText = lv_label_create(parent);
        lv_label_set_text(tapText, "TAP TO OPEN");
        lv_obj_set_style_text_color(tapText, lv_color_hex(0x6E6E73), 0);
        lv_obj_set_style_text_font(tapText, &lv_font_montserrat_10, 0);
        lv_obj_align(tapText, LV_ALIGN_TOP_MID, 0, 55);
        
        // Nike swoosh - LEFT OF CENTER, clearly visible (like reference!)
        drawNikeSwoosh(parent, centerX - 55, centerY - 35, 30, swooshColor);
        
        // Activity arc indicators - THICKER! (8px instead of 4px)
        lv_obj_t *arcLeft = lv_arc_create(parent);
        lv_obj_set_size(arcLeft, LCD_WIDTH - 35, LCD_HEIGHT - 35);
        lv_arc_set_rotation(arcLeft, 195);
        lv_arc_set_bg_angles(arcLeft, 0, 55);
        lv_arc_set_angles(arcLeft, 0, 40);
        lv_obj_set_style_arc_color(arcLeft, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arcLeft, lv_color_hex(textColor), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(arcLeft, 8, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arcLeft, 8, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(arcLeft, true, LV_PART_INDICATOR);
        lv_obj_remove_style(arcLeft, NULL, LV_PART_KNOB);
        lv_obj_center(arcLeft);
        
        lv_obj_t *arcBottom = lv_arc_create(parent);
        lv_obj_set_size(arcBottom, LCD_WIDTH - 35, LCD_HEIGHT - 35);
        lv_arc_set_rotation(arcBottom, 250);
        lv_arc_set_bg_angles(arcBottom, 0, 45);
        lv_arc_set_angles(arcBottom, 0, 35);
        lv_obj_set_style_arc_color(arcBottom, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arcBottom, lv_color_hex(textColor), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(arcBottom, 8, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arcBottom, 8, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(arcBottom, true, LV_PART_INDICATOR);
        lv_obj_remove_style(arcBottom, NULL, LV_PART_KNOB);
        lv_obj_center(arcBottom);
        
        // Bottom left stats (steps, distance, calories) - BIGGER font
        char statsBuf[20];
        snprintf(statsBuf, sizeof(statsBuf), "%d • %02d • %02d", 
                (int)(userData.totalDistance * 10) % 100,
                (int)userData.totalCalories % 100,
                (int)(userData.totalCalories / 10) % 100);
        lv_obj_t *statsLabel = lv_label_create(parent);
        lv_label_set_text(statsLabel, statsBuf);
        lv_obj_set_style_text_color(statsLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(statsLabel, &lv_font_montserrat_14, 0);
        lv_obj_align(statsLabel, LV_ALIGN_BOTTOM_LEFT, 22, -22);
        
        // Battery percentage - bottom right - BIGGER
        char battBuf[8];
        snprintf(battBuf, sizeof(battBuf), "%d%%", batteryPercent);
        lv_obj_t *battLabel = lv_label_create(parent);
        lv_label_set_text(battLabel, battBuf);
        lv_obj_set_style_text_color(battLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(battLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(battLabel, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
        
    } else {
        // ════════════════════════════════════════════════════════════════
        // FULL COLOR VARIANTS (Volt, Purple, Orange, Pink, White)
        // Simple, bold time with Nike swoosh - like reference images
        // ════════════════════════════════════════════════════════════════
        
        // Nike swoosh at bottom - use swoosh color from scheme (BIGGER swoosh!)
        drawNikeSwoosh(parent, LCD_WIDTH/2, LCD_HEIGHT - 50, 45, swooshColor);
        
        // "RUNNING TODAY?" text
        lv_obj_t *runText = lv_label_create(parent);
        lv_label_set_text(runText, "RUNNING TODAY?");
        lv_obj_set_style_text_color(runText, lv_color_hex(textColor), 0);
        lv_obj_set_style_text_opa(runText, LV_OPA_60, 0);
        lv_obj_set_style_text_font(runText, &lv_font_montserrat_12, 0);
        lv_obj_align(runText, LV_ALIGN_BOTTOM_MID, 0, -95);
    }
    
    // ════════════════════════════════════════════════════════════════
    // MAIN TIME - HUGE NIKE STYLE DIGITS (OPTIMIZED - less objects!)
    // ════════════════════════════════════════════════════════════════
    
    // Format time
    int displayHour = use24HourFormat ? dt.getHour() : (dt.getHour() % 12);
    if (!use24HourFormat && displayHour == 0) displayHour = 12;
    
    char hourBuf[4], minBuf[4];
    snprintf(hourBuf, sizeof(hourBuf), "%02d", displayHour);
    snprintf(minBuf, sizeof(minBuf), "%02d", dt.getMinute());
    
    // Position: Stacked in center
    int hourY = hasCircularDial ? -45 : -55;
    int minY = hasCircularDial ? 35 : 45;
    
    // ═══ HOUR - SIMPLE 4-LAYER SHADOW (was 48 layers - too slow!) ═══
    // Shadow layers - just 4 directions for performance
    int shadowOffsets[4][2] = {{-3, -3}, {3, -3}, {-3, 3}, {3, 3}};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *shadow = lv_label_create(parent);
        lv_label_set_text(shadow, hourBuf);
        lv_obj_set_style_text_color(shadow, lv_color_hex(0x000033), 0);
        lv_obj_set_style_text_font(shadow, &NIKE_FONT, 0);
        lv_obj_set_style_text_opa(shadow, LV_OPA_70, 0);
        lv_obj_align(shadow, LV_ALIGN_CENTER, shadowOffsets[i][0], hourY + shadowOffsets[i][1]);
    }
    // Main hour
    lv_obj_t *hourMain = lv_label_create(parent);
    lv_label_set_text(hourMain, hourBuf);
    lv_obj_set_style_text_color(hourMain, lv_color_hex(textColor), 0);
    lv_obj_set_style_text_font(hourMain, &NIKE_FONT, 0);
    lv_obj_align(hourMain, LV_ALIGN_CENTER, 0, hourY);
    
    // ═══ MINUTE - SIMPLE 4-LAYER SHADOW ═══
    for (int i = 0; i < 4; i++) {
        lv_obj_t *shadow = lv_label_create(parent);
        lv_label_set_text(shadow, minBuf);
        lv_obj_set_style_text_color(shadow, lv_color_hex(0x000033), 0);
        lv_obj_set_style_text_font(shadow, &NIKE_FONT, 0);
        lv_obj_set_style_text_opa(shadow, LV_OPA_70, 0);
        lv_obj_align(shadow, LV_ALIGN_CENTER, shadowOffsets[i][0], minY + shadowOffsets[i][1]);
    }
    // Main minute
    lv_obj_t *minMain = lv_label_create(parent);
    lv_label_set_text(minMain, minBuf);
    lv_obj_set_style_text_color(minMain, lv_color_hex(textColor), 0);
    lv_obj_set_style_text_font(minMain, &NIKE_FONT, 0);
    lv_obj_align(minMain, LV_ALIGN_CENTER, 0, minY);
    
    // Color variant indicator (non-dial variants only)
    if (!hasCircularDial) {
        lv_obj_t *colorHint = lv_label_create(parent);
        char hintBuf[24];
        snprintf(hintBuf, sizeof(hintBuf), "< %s >", colors.name);
        lv_label_set_text(colorHint, hintBuf);
        lv_obj_set_style_text_color(colorHint, lv_color_hex(textColor), 0);
        lv_obj_set_style_text_font(colorHint, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_opa(colorHint, LV_OPA_70, 0);
        lv_obj_align(colorHint, LV_ALIGN_BOTTOM_MID, 0, -8);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE 4: MINIMAL DARK
// ═══════════════════════════════════════════════════════════════════════════
void createMinimalDarkFace(lv_obj_t *parent) {
    GradientTheme *theme = getSafeTheme();
    RTC_DateTime dt = rtc.getDateTime();
    
    // BLE indicator (non-Nike face)
    drawBLEIndicator(parent);
    
    // ═══ ULTRA-MINIMAL PREMIUM FACE ═══
    
    // Subtle corner accent line
    lv_obj_t *accentLine = lv_obj_create(parent);
    lv_obj_set_size(accentLine, 3, 60);
    lv_obj_align(accentLine, LV_ALIGN_TOP_LEFT, 20, 40);
    lv_obj_set_style_bg_color(accentLine, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_radius(accentLine, 2, 0);
    lv_obj_set_style_border_width(accentLine, 0, 0);
    disableAllScrolling(accentLine);
    
    // Day name - all caps, subtle
    const char* dayNames[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    lv_obj_t *dayLabel = lv_label_create(parent);
    lv_label_set_text(dayLabel, dayNames[dt.getWeek()]);
    lv_obj_set_style_text_color(dayLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(dayLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(dayLabel, LV_ALIGN_TOP_LEFT, 32, 45);
    
    // Date number - large accent
    char dateBuf[4];
    snprintf(dateBuf, sizeof(dateBuf), "%d", dt.getDay());
    lv_obj_t *dateLabel = lv_label_create(parent);
    lv_label_set_text(dateLabel, dateBuf);
    lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(dateLabel, LV_ALIGN_TOP_LEFT, 32, 62);
    
    // MASSIVE time - left aligned for impact
    char hourBuf[4];
    snprintf(hourBuf, sizeof(hourBuf), "%02d", dt.getHour());
    
    lv_obj_t *hourLabel = lv_label_create(parent);
    lv_label_set_text(hourLabel, hourBuf);
    lv_obj_set_style_text_color(hourLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(hourLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(hourLabel, LV_ALIGN_LEFT_MID, 25, -15);
    
    char minBuf[4];
    snprintf(minBuf, sizeof(minBuf), "%02d", dt.getMinute());
    
    lv_obj_t *minLabel = lv_label_create(parent);
    lv_label_set_text(minLabel, minBuf);
    lv_obj_set_style_text_color(minLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(minLabel, LV_OPA_60, 0);
    lv_obj_set_style_text_font(minLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(minLabel, LV_ALIGN_LEFT_MID, 25, 40);
    
    // Seconds - small, floating
    char secBuf[4];
    snprintf(secBuf, sizeof(secBuf), "%02d", dt.getSecond());
    lv_obj_t *secLabel = lv_label_create(parent);
    lv_label_set_text(secLabel, secBuf);
    lv_obj_set_style_text_color(secLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(secLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(secLabel, LV_ALIGN_LEFT_MID, 115, 55);
    
    // Right side - minimal stats
    // Battery pill
    lv_obj_t *battPill = lv_obj_create(parent);
    lv_obj_set_size(battPill, 50, 24);
    lv_obj_align(battPill, LV_ALIGN_RIGHT_MID, -20, -40);
    lv_obj_set_style_bg_color(battPill, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(battPill, 12, 0);
    lv_obj_set_style_border_width(battPill, 0, 0);
    disableAllScrolling(battPill);
    
    char battBuf[8];
    snprintf(battBuf, sizeof(battBuf), "%d%%", batteryPercent);
    lv_obj_t *battLabel = lv_label_create(battPill);
    lv_label_set_text(battLabel, battBuf);
    uint32_t battColor = batteryPercent > 20 ? 0x30D158 : 0xFF3B30;
    lv_obj_set_style_text_color(battLabel, lv_color_hex(battColor), 0);
    lv_obj_set_style_text_font(battLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(battLabel);
    
    // Weather temp
    if (wifiConnected) {
        lv_obj_t *tempPill = lv_obj_create(parent);
        lv_obj_set_size(tempPill, 50, 24);
        lv_obj_align(tempPill, LV_ALIGN_RIGHT_MID, -20, 0);
        lv_obj_set_style_bg_color(tempPill, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(tempPill, 12, 0);
        lv_obj_set_style_border_width(tempPill, 0, 0);
        disableAllScrolling(tempPill);
        
        char tempBuf[8];
        snprintf(tempBuf, sizeof(tempBuf), "%.0f°", weatherTemp);
        lv_obj_t *tempLabel = lv_label_create(tempPill);
        lv_label_set_text(tempLabel, tempBuf);
        lv_obj_set_style_text_color(tempLabel, lv_color_hex(0x5AC8FA), 0);
        lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_12, 0);
        lv_obj_center(tempLabel);
    }
    
    // Moon Phase & Sunrise/Sunset - bottom row
    if (astroDataSynced) {
        // Moon phase (left)
        lv_obj_t *moonLabel = lv_label_create(parent);
        char moonBuf[24];
        snprintf(moonBuf, sizeof(moonBuf), "%s", moonPhaseName.c_str());
        lv_label_set_text(moonLabel, moonBuf);
        lv_obj_set_style_text_color(moonLabel, lv_color_hex(0xFFD60A), 0);
        lv_obj_set_style_text_font(moonLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(moonLabel, LV_ALIGN_BOTTOM_RIGHT, -15, -40);
        
        // Sunrise/Sunset (right)
        lv_obj_t *sunLabel = lv_label_create(parent);
        char sunBuf[24];
        snprintf(sunBuf, sizeof(sunBuf), "%s/%s", sunriseTime.c_str(), sunsetTime.c_str());
        lv_label_set_text(sunLabel, sunBuf);
        lv_obj_set_style_text_color(sunLabel, lv_color_hex(0xFF9F0A), 0);
        lv_obj_set_style_text_font(sunLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(sunLabel, LV_ALIGN_BOTTOM_RIGHT, -15, -25);
    }
    
    // Steps - bottom
    char stepBuf[16];
    snprintf(stepBuf, sizeof(stepBuf), "%lu", (unsigned long)userData.steps);
    lv_obj_t *stepLabel = lv_label_create(parent);
    lv_label_set_text(stepLabel, stepBuf);
    lv_obj_set_style_text_color(stepLabel, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(stepLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(stepLabel, LV_ALIGN_BOTTOM_LEFT, 25, -25);
    
    lv_obj_t *stepsWord = lv_label_create(parent);
    lv_label_set_text(stepsWord, "steps");
    lv_obj_set_style_text_color(stepsWord, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(stepsWord, &lv_font_montserrat_10, 0);
    lv_obj_align(stepsWord, LV_ALIGN_BOTTOM_LEFT, 25, -12);
}

// ═══════════════════════════════════════════════════════════════════════════
// WATCH FACE 5: FITNESS RINGS - ACTIVITY FOCUSED
// ═══════════════════════════════════════════════════════════════════════════
void createFitnessRingsFace(lv_obj_t *parent) {
    GradientTheme *theme = getSafeTheme();
    RTC_DateTime dt = rtc.getDateTime();
    
    // BLE indicator (non-Nike face)
    drawBLEIndicator(parent);
    
    int ringCenterX = 80;
    int ringCenterY = LCD_HEIGHT / 2;
    
    // Move ring - Red
    lv_obj_t *moveRing = lv_arc_create(parent);
    lv_obj_set_size(moveRing, 130, 130);
    lv_obj_set_pos(moveRing, ringCenterX - 65, ringCenterY - 65);
    lv_arc_set_rotation(moveRing, 270);
    lv_arc_set_bg_angles(moveRing, 0, 360);
    lv_arc_set_range(moveRing, 0, 100);
    int moveProgress = min(100, (int)(userData.steps * 100 / userData.dailyGoal));
    lv_arc_set_value(moveRing, moveProgress);
    lv_obj_set_style_arc_width(moveRing, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(moveRing, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(moveRing, lv_color_hex(0xFF2D55), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(moveRing, lv_color_hex(0x3A1520), LV_PART_MAIN);
    lv_obj_remove_style(moveRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(moveRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Exercise ring - Green
    lv_obj_t *exerciseRing = lv_arc_create(parent);
    lv_obj_set_size(exerciseRing, 95, 95);
    lv_obj_set_pos(exerciseRing, ringCenterX - 47, ringCenterY - 47);
    lv_arc_set_rotation(exerciseRing, 270);
    lv_arc_set_bg_angles(exerciseRing, 0, 360);
    lv_arc_set_range(exerciseRing, 0, 100);
    lv_arc_set_value(exerciseRing, 72);
    lv_obj_set_style_arc_width(exerciseRing, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(exerciseRing, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(exerciseRing, lv_color_hex(0xA2FF00), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(exerciseRing, lv_color_hex(0x1A3A00), LV_PART_MAIN);
    lv_obj_remove_style(exerciseRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(exerciseRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Stand ring - Cyan
    lv_obj_t *standRing = lv_arc_create(parent);
    lv_obj_set_size(standRing, 60, 60);
    lv_obj_set_pos(standRing, ringCenterX - 30, ringCenterY - 30);
    lv_arc_set_rotation(standRing, 270);
    lv_arc_set_bg_angles(standRing, 0, 360);
    lv_arc_set_range(standRing, 0, 12);
    lv_arc_set_value(standRing, 9);
    lv_obj_set_style_arc_width(standRing, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(standRing, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(standRing, lv_color_hex(0x00D4FF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(standRing, lv_color_hex(0x002A3A), LV_PART_MAIN);
    lv_obj_remove_style(standRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(standRing, LV_OBJ_FLAG_CLICKABLE);
    
    // Time on right side
    char timeBuf[8];
    int hour12 = dt.getHour() % 12;
    if (hour12 == 0) hour12 = 12;
    snprintf(timeBuf, sizeof(timeBuf), "%d:%02d", hour12, dt.getMinute());
    
    lv_obj_t *timeLabel = lv_label_create(parent);
    lv_label_set_text(timeLabel, timeBuf);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_32, 0);
    lv_obj_align(timeLabel, LV_ALIGN_TOP_RIGHT, -25, 70);
    
    // Calorie count (red)
    char calBuf[16];
    snprintf(calBuf, sizeof(calBuf), "%d", (int)userData.totalCalories);
    lv_obj_t *calLabel = lv_label_create(parent);
    lv_label_set_text(calLabel, calBuf);
    lv_obj_set_style_text_color(calLabel, lv_color_hex(0xFF2D55), 0);
    lv_obj_set_style_text_font(calLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(calLabel, LV_ALIGN_TOP_RIGHT, -25, 120);
    
    // Exercise minutes (green)
    lv_obj_t *exLabel = lv_label_create(parent);
    lv_label_set_text(exLabel, "32");
    lv_obj_set_style_text_color(exLabel, lv_color_hex(0xA2FF00), 0);
    lv_obj_set_style_text_font(exLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(exLabel, LV_ALIGN_TOP_RIGHT, -25, 155);
    
    // Stand hours (cyan)
    lv_obj_t *standLabel = lv_label_create(parent);
    lv_label_set_text(standLabel, "9");
    lv_obj_set_style_text_color(standLabel, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_text_font(standLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(standLabel, LV_ALIGN_TOP_RIGHT, -25, 190);
    
    // Icons at top
    lv_obj_t *musicIcon = lv_label_create(parent);
    lv_label_set_text(musicIcon, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(musicIcon, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(musicIcon, LV_ALIGN_TOP_RIGHT, -60, 25);
    
    lv_obj_t *cloudIcon = lv_label_create(parent);
    lv_label_set_text(cloudIcon, LV_SYMBOL_DOWNLOAD);
    lv_obj_set_style_text_color(cloudIcon, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(cloudIcon, LV_ALIGN_TOP_RIGHT, -25, 25);
}

// ═══════════════════════════════════════════════════════════════════════════
// DIGITAL FACE - ORIGINAL STYLE (REFACTORED INTO FUNCTION)
// ═══════════════════════════════════════════════════════════════════════════
void createDigitalFace(lv_obj_t *parent) {
    GradientTheme *theme = getSafeTheme();
    RTC_DateTime dt = rtc.getDateTime();
    
    // BLE indicator (non-Nike face)
    drawBLEIndicator(parent);
    
    // ═══ PREMIUM APPLE WATCH DIGITAL FACE ═══
    
    // Large time display - centered, massive, clean
    char timeBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dt.getHour(), dt.getMinute());

    // Subtle glow effect behind time
    lv_obj_t *timeGlow = lv_label_create(parent);
    lv_label_set_text(timeGlow, timeBuf);
    lv_obj_set_style_text_color(timeGlow, theme->accent, 0);
    lv_obj_set_style_text_opa(timeGlow, LV_OPA_20, 0);
    lv_obj_set_style_text_font(timeGlow, &lv_font_montserrat_48, 0);
    lv_obj_align(timeGlow, LV_ALIGN_CENTER, 2, -22);

    // Main time - crisp white
    lv_obj_t *clockLabel = lv_label_create(parent);
    lv_label_set_text(clockLabel, timeBuf);
    lv_obj_set_style_text_color(clockLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(clockLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(clockLabel, LV_ALIGN_CENTER, 0, -25);

    // Seconds - small accent badge
    char secBuf[4];
    snprintf(secBuf, sizeof(secBuf), "%02d", dt.getSecond());
    
    lv_obj_t *secBadge = lv_obj_create(parent);
    lv_obj_set_size(secBadge, 36, 24);
    lv_obj_align(secBadge, LV_ALIGN_CENTER, 75, -25);
    lv_obj_set_style_bg_color(secBadge, theme->accent, 0);
    lv_obj_set_style_bg_opa(secBadge, LV_OPA_30, 0);
    lv_obj_set_style_radius(secBadge, 12, 0);
    lv_obj_set_style_border_width(secBadge, 0, 0);
    disableAllScrolling(secBadge);
    
    lv_obj_t *secLabel = lv_label_create(secBadge);
    lv_label_set_text(secLabel, secBuf);
    lv_obj_set_style_text_color(secLabel, theme->accent, 0);
    lv_obj_set_style_text_font(secLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(secLabel);

    // Day and date - elegant top positioning
    const char* dayNames[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    const char* monthNames[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    
    // Day name - subtle
    lv_obj_t *dayLabel = lv_label_create(parent);
    lv_label_set_text(dayLabel, dayNames[dt.getWeek()]);
    lv_obj_set_style_text_color(dayLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(dayLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(dayLabel, LV_ALIGN_TOP_MID, 0, 35);

    // Date badge - pill style
    char dateBuf[12];
    snprintf(dateBuf, sizeof(dateBuf), "%s %d", monthNames[dt.getMonth()-1], dt.getDay());
    
    lv_obj_t *dateBadge = lv_obj_create(parent);
    lv_obj_set_size(dateBadge, 70, 24);
    lv_obj_align(dateBadge, LV_ALIGN_TOP_MID, 0, 52);
    lv_obj_set_style_bg_color(dateBadge, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_radius(dateBadge, 12, 0);
    lv_obj_set_style_border_width(dateBadge, 0, 0);
    disableAllScrolling(dateBadge);
    
    lv_obj_t *dateLabel = lv_label_create(dateBadge);
    lv_label_set_text(dateLabel, dateBuf);
    lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(dateLabel);

    // ═══ COMPLICATIONS BAR - Bottom ═══
    
    // Left complication - Steps with ring
    lv_obj_t *stepsComplication = lv_obj_create(parent);
    lv_obj_set_size(stepsComplication, 70, 70);
    lv_obj_align(stepsComplication, LV_ALIGN_BOTTOM_LEFT, 15, -15);
    lv_obj_set_style_bg_color(stepsComplication, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(stepsComplication, 35, 0);
    lv_obj_set_style_border_width(stepsComplication, 0, 0);
    disableAllScrolling(stepsComplication);
    
    // Steps progress arc
    int stepProgress = min(100, (int)(userData.steps * 100 / userData.dailyGoal));
    lv_obj_t *stepArc = lv_arc_create(stepsComplication);
    lv_obj_set_size(stepArc, 65, 65);
    lv_obj_center(stepArc);
    lv_arc_set_rotation(stepArc, 270);
    lv_arc_set_bg_angles(stepArc, 0, 360);
    lv_arc_set_range(stepArc, 0, 100);
    lv_arc_set_value(stepArc, stepProgress);
    lv_obj_set_style_arc_width(stepArc, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(stepArc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(stepArc, lv_color_hex(0x30D158), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(stepArc, lv_color_hex(0x2C2C2E), LV_PART_MAIN);
    lv_obj_remove_style(stepArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(stepArc, LV_OBJ_FLAG_CLICKABLE);
    
    char stepBuf[8];
    snprintf(stepBuf, sizeof(stepBuf), "%lu", (unsigned long)(userData.steps / 1000));
    lv_obj_t *stepNum = lv_label_create(stepsComplication);
    lv_label_set_text(stepNum, stepBuf);
    lv_obj_set_style_text_color(stepNum, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(stepNum, &lv_font_montserrat_18, 0);
    lv_obj_align(stepNum, LV_ALIGN_CENTER, 0, -5);
    
    lv_obj_t *stepK = lv_label_create(stepsComplication);
    lv_label_set_text(stepK, "K");
    lv_obj_set_style_text_color(stepK, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(stepK, &lv_font_montserrat_10, 0);
    lv_obj_align(stepK, LV_ALIGN_CENTER, 0, 12);

    // Center complication - Battery
    lv_obj_t *battComplication = lv_obj_create(parent);
    lv_obj_set_size(battComplication, 60, 60);
    lv_obj_align(battComplication, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(battComplication, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(battComplication, 30, 0);
    lv_obj_set_style_border_width(battComplication, 0, 0);
    disableAllScrolling(battComplication);
    
    // Battery arc
    lv_obj_t *battArc = lv_arc_create(battComplication);
    lv_obj_set_size(battArc, 55, 55);
    lv_obj_center(battArc);
    lv_arc_set_rotation(battArc, 270);
    lv_arc_set_bg_angles(battArc, 0, 360);
    lv_arc_set_range(battArc, 0, 100);
    lv_arc_set_value(battArc, batteryPercent);
    lv_obj_set_style_arc_width(battArc, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(battArc, 4, LV_PART_MAIN);
    uint32_t battColor = batteryPercent > 20 ? 0xFFD60A : 0xFF3B30;
    lv_obj_set_style_arc_color(battArc, lv_color_hex(battColor), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(battArc, lv_color_hex(0x2C2C2E), LV_PART_MAIN);
    lv_obj_remove_style(battArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(battArc, LV_OBJ_FLAG_CLICKABLE);
    
    char battBuf[5];
    snprintf(battBuf, sizeof(battBuf), "%d", batteryPercent);
    lv_obj_t *battNum = lv_label_create(battComplication);
    lv_label_set_text(battNum, battBuf);
    lv_obj_set_style_text_color(battNum, lv_color_hex(battColor), 0);
    lv_obj_set_style_text_font(battNum, &lv_font_montserrat_16, 0);
    lv_obj_align(battNum, LV_ALIGN_CENTER, 0, -3);
    
    lv_obj_t *battPct = lv_label_create(battComplication);
    lv_label_set_text(battPct, "%");
    lv_obj_set_style_text_color(battPct, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(battPct, &lv_font_montserrat_10, 0);
    lv_obj_align(battPct, LV_ALIGN_CENTER, 0, 12);

    // Right complication - WiFi/Weather
    lv_obj_t *wifiComplication = lv_obj_create(parent);
    lv_obj_set_size(wifiComplication, 70, 70);
    lv_obj_align(wifiComplication, LV_ALIGN_BOTTOM_RIGHT, -15, -15);
    lv_obj_set_style_bg_color(wifiComplication, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(wifiComplication, 35, 0);
    lv_obj_set_style_border_width(wifiComplication, 0, 0);
    disableAllScrolling(wifiComplication);
    
    // WiFi icon or temp
    if (wifiConnected) {
        char tempBuf[8];
        snprintf(tempBuf, sizeof(tempBuf), "%.0f", weatherTemp);
        lv_obj_t *tempLabel = lv_label_create(wifiComplication);
        lv_label_set_text(tempLabel, tempBuf);
        lv_obj_set_style_text_color(tempLabel, lv_color_hex(0x5AC8FA), 0);
        lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(tempLabel, LV_ALIGN_CENTER, 0, -5);
        
        lv_obj_t *degLabel = lv_label_create(wifiComplication);
        lv_label_set_text(degLabel, "C");
        lv_obj_set_style_text_color(degLabel, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(degLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(degLabel, LV_ALIGN_CENTER, 0, 12);
    } else {
        lv_obj_t *wifiIcon = lv_label_create(wifiComplication);
        lv_label_set_text(wifiIcon, LV_SYMBOL_WIFI);
        lv_obj_set_style_text_color(wifiIcon, lv_color_hex(0xFF3B30), 0);
        lv_obj_set_style_text_font(wifiIcon, &lv_font_montserrat_24, 0);
        lv_obj_center(wifiIcon);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// CLOCK CARD - DISPATCHES TO SELECTED WATCH FACE
// ═══════════════════════════════════════════════════════════════════════════
void createClockCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    // Background with wallpaper support
    lv_obj_t *bgCard = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bgCard, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_align(bgCard, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(bgCard, 0, 0);
    lv_obj_set_style_border_width(bgCard, 0, 0);
    lv_obj_set_style_pad_all(bgCard, 0, 0);
    disableAllScrolling(bgCard);

    // ═══════════════════════════════════════════════════════════════════════
    // PREMIUM CUSTOM WALLPAPER BACKGROUNDS - BETTER THAN APPLE!
    // ═══════════════════════════════════════════════════════════════════════
    if (userData.wallpaperIndex > 0 && userData.wallpaperIndex < NUM_GRADIENT_WALLPAPERS) {
        WallpaperTheme *wp = &gradientWallpapers[userData.wallpaperIndex];

        // Base gradient
        lv_obj_set_style_bg_color(bgCard, wp->top, 0);
        lv_obj_set_style_bg_grad_color(bgCard, wp->bottom, 0);
        lv_obj_set_style_bg_grad_dir(bgCard, LV_GRAD_DIR_VER, 0);

        // ═══ WALLPAPER 1: MOUNTAIN SUNSET - Epic mountain scene ═══
        if (userData.wallpaperIndex == 1) {
            // Sky gradient layers
            lv_obj_t *skyMid = lv_obj_create(bgCard);
            lv_obj_set_size(skyMid, LCD_WIDTH, LCD_HEIGHT / 2);
            lv_obj_align(skyMid, LV_ALIGN_TOP_MID, 0, 0);
            lv_obj_set_style_bg_color(skyMid, lv_color_hex(0x4A90D9), 0);
            lv_obj_set_style_bg_grad_color(skyMid, lv_color_hex(0xFF7F50), 0);
            lv_obj_set_style_bg_grad_dir(skyMid, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(skyMid, 0, 0);
            lv_obj_set_style_radius(skyMid, 0, 0);
            disableAllScrolling(skyMid);
            
            // Sun with glow
            lv_obj_t *sunGlow = lv_obj_create(bgCard);
            lv_obj_set_size(sunGlow, 80, 80);
            lv_obj_align(sunGlow, LV_ALIGN_CENTER, 40, -30);
            lv_obj_set_style_bg_color(sunGlow, lv_color_hex(0xFFD700), 0);
            lv_obj_set_style_bg_opa(sunGlow, LV_OPA_30, 0);
            lv_obj_set_style_radius(sunGlow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sunGlow, 0, 0);
            disableAllScrolling(sunGlow);
            
            lv_obj_t *sun = lv_obj_create(bgCard);
            lv_obj_set_size(sun, 40, 40);
            lv_obj_align(sun, LV_ALIGN_CENTER, 40, -30);
            lv_obj_set_style_bg_color(sun, lv_color_hex(0xFFE4B5), 0);
            lv_obj_set_style_radius(sun, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sun, 0, 0);
            disableAllScrolling(sun);
            
            // Far mountains (darkest, back layer)
            lv_obj_t *farMtn = lv_obj_create(bgCard);
            lv_obj_set_size(farMtn, LCD_WIDTH + 40, 120);
            lv_obj_align(farMtn, LV_ALIGN_BOTTOM_MID, 0, -60);
            lv_obj_set_style_bg_color(farMtn, lv_color_hex(0x4A3728), 0);
            lv_obj_set_style_border_width(farMtn, 0, 0);
            lv_obj_set_style_radius(farMtn, 60, 0);
            disableAllScrolling(farMtn);
            
            // Mid mountains
            lv_obj_t *midMtn = lv_obj_create(bgCard);
            lv_obj_set_size(midMtn, 180, 140);
            lv_obj_align(midMtn, LV_ALIGN_BOTTOM_LEFT, -30, -20);
            lv_obj_set_style_bg_color(midMtn, lv_color_hex(0x5D4037), 0);
            lv_obj_set_style_bg_grad_color(midMtn, lv_color_hex(0x8D6E63), 0);
            lv_obj_set_style_bg_grad_dir(midMtn, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(midMtn, 0, 0);
            lv_obj_set_style_radius(midMtn, 80, 0);
            disableAllScrolling(midMtn);
            
            // Close mountain (lighter, front)
            lv_obj_t *closeMtn = lv_obj_create(bgCard);
            lv_obj_set_size(closeMtn, 160, 160);
            lv_obj_align(closeMtn, LV_ALIGN_BOTTOM_RIGHT, 20, 0);
            lv_obj_set_style_bg_color(closeMtn, lv_color_hex(0x6D4C41), 0);
            lv_obj_set_style_bg_grad_color(closeMtn, lv_color_hex(0xA1887F), 0);
            lv_obj_set_style_bg_grad_dir(closeMtn, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(closeMtn, 0, 0);
            lv_obj_set_style_radius(closeMtn, 80, 0);
            disableAllScrolling(closeMtn);
            
            // Snow caps
            lv_obj_t *snow1 = lv_obj_create(bgCard);
            lv_obj_set_size(snow1, 50, 25);
            lv_obj_align(snow1, LV_ALIGN_BOTTOM_LEFT, 35, -130);
            lv_obj_set_style_bg_color(snow1, lv_color_hex(0xFFFAF0), 0);
            lv_obj_set_style_border_width(snow1, 0, 0);
            lv_obj_set_style_radius(snow1, 12, 0);
            disableAllScrolling(snow1);
            
            lv_obj_t *snow2 = lv_obj_create(bgCard);
            lv_obj_set_size(snow2, 40, 20);
            lv_obj_align(snow2, LV_ALIGN_BOTTOM_RIGHT, -45, -140);
            lv_obj_set_style_bg_color(snow2, lv_color_hex(0xFFFAF0), 0);
            lv_obj_set_style_border_width(snow2, 0, 0);
            lv_obj_set_style_radius(snow2, 10, 0);
            disableAllScrolling(snow2);
        }
        
        // ═══ WALLPAPER 2: GOLDEN PEAKS - Sunrise gold mountains ═══
        else if (userData.wallpaperIndex == 2) {
            // Warm sunrise sky
            lv_obj_set_style_bg_color(bgCard, lv_color_hex(0xFF6B35), 0);
            lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x1A0A00), 0);
            
            // Golden sun rays
            for (int i = 0; i < 8; i++) {
                lv_obj_t *ray = lv_obj_create(bgCard);
                lv_obj_set_size(ray, 8, 200);
                lv_obj_align(ray, LV_ALIGN_TOP_MID, 30, -50);
                lv_obj_set_style_bg_color(ray, lv_color_hex(0xFFD700), 0);
                lv_obj_set_style_bg_opa(ray, LV_OPA_20, 0);
                lv_obj_set_style_radius(ray, 4, 0);
                lv_obj_set_style_border_width(ray, 0, 0);
                lv_obj_set_style_transform_angle(ray, i * 200 - 700, 0);
                disableAllScrolling(ray);
            }
            
            // Sun disk
            lv_obj_t *sun = lv_obj_create(bgCard);
            lv_obj_set_size(sun, 60, 60);
            lv_obj_align(sun, LV_ALIGN_TOP_MID, 30, 20);
            lv_obj_set_style_bg_color(sun, lv_color_hex(0xFFE066), 0);
            lv_obj_set_style_radius(sun, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sun, 0, 0);
            lv_obj_set_style_shadow_width(sun, 40, 0);
            lv_obj_set_style_shadow_color(sun, lv_color_hex(0xFFD700), 0);
            lv_obj_set_style_shadow_opa(sun, LV_OPA_60, 0);
            disableAllScrolling(sun);
            
            // Layered golden mountains
            lv_obj_t *mtn1 = lv_obj_create(bgCard);
            lv_obj_set_size(mtn1, 200, 180);
            lv_obj_align(mtn1, LV_ALIGN_BOTTOM_MID, -60, 20);
            lv_obj_set_style_bg_color(mtn1, lv_color_hex(0x3D2817), 0);
            lv_obj_set_style_border_width(mtn1, 0, 0);
            lv_obj_set_style_radius(mtn1, 100, 0);
            disableAllScrolling(mtn1);
            
            lv_obj_t *mtn2 = lv_obj_create(bgCard);
            lv_obj_set_size(mtn2, 180, 200);
            lv_obj_align(mtn2, LV_ALIGN_BOTTOM_RIGHT, 30, 30);
            lv_obj_set_style_bg_color(mtn2, lv_color_hex(0x4A3520), 0);
            lv_obj_set_style_border_width(mtn2, 0, 0);
            lv_obj_set_style_radius(mtn2, 90, 0);
            disableAllScrolling(mtn2);
            
            // Golden rim light on peaks
            lv_obj_t *rimLight = lv_obj_create(bgCard);
            lv_obj_set_size(rimLight, 150, 10);
            lv_obj_align(rimLight, LV_ALIGN_BOTTOM_MID, -60, -140);
            lv_obj_set_style_bg_color(rimLight, lv_color_hex(0xFFD700), 0);
            lv_obj_set_style_bg_opa(rimLight, LV_OPA_70, 0);
            lv_obj_set_style_border_width(rimLight, 0, 0);
            lv_obj_set_style_radius(rimLight, 5, 0);
            disableAllScrolling(rimLight);
        }
        
        // ═══ WALLPAPER 3: CANYON DAWN - Pink desert canyon ═══
        else if (userData.wallpaperIndex == 3) {
            // Pink/orange sky
            lv_obj_set_style_bg_color(bgCard, lv_color_hex(0x87CEEB), 0);
            lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x8B4513), 0);
            
            // Gradient overlay for sunset effect
            lv_obj_t *sunsetGlow = lv_obj_create(bgCard);
            lv_obj_set_size(sunsetGlow, LCD_WIDTH, LCD_HEIGHT / 2);
            lv_obj_align(sunsetGlow, LV_ALIGN_CENTER, 0, -20);
            lv_obj_set_style_bg_color(sunsetGlow, lv_color_hex(0xFFB6C1), 0);
            lv_obj_set_style_bg_grad_color(sunsetGlow, lv_color_hex(0xFF6347), 0);
            lv_obj_set_style_bg_grad_dir(sunsetGlow, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_bg_opa(sunsetGlow, LV_OPA_60, 0);
            lv_obj_set_style_border_width(sunsetGlow, 0, 0);
            lv_obj_set_style_radius(sunsetGlow, 0, 0);
            disableAllScrolling(sunsetGlow);
            
            // Canyon walls - left
            lv_obj_t *canyonL = lv_obj_create(bgCard);
            lv_obj_set_size(canyonL, 100, LCD_HEIGHT);
            lv_obj_align(canyonL, LV_ALIGN_LEFT_MID, -30, 0);
            lv_obj_set_style_bg_color(canyonL, lv_color_hex(0xCD5C5C), 0);
            lv_obj_set_style_bg_grad_color(canyonL, lv_color_hex(0x8B0000), 0);
            lv_obj_set_style_bg_grad_dir(canyonL, LV_GRAD_DIR_HOR, 0);
            lv_obj_set_style_border_width(canyonL, 0, 0);
            lv_obj_set_style_radius(canyonL, 20, 0);
            disableAllScrolling(canyonL);
            
            // Canyon walls - right
            lv_obj_t *canyonR = lv_obj_create(bgCard);
            lv_obj_set_size(canyonR, 100, LCD_HEIGHT);
            lv_obj_align(canyonR, LV_ALIGN_RIGHT_MID, 30, 0);
            lv_obj_set_style_bg_color(canyonR, lv_color_hex(0x8B0000), 0);
            lv_obj_set_style_bg_grad_color(canyonR, lv_color_hex(0xCD5C5C), 0);
            lv_obj_set_style_bg_grad_dir(canyonR, LV_GRAD_DIR_HOR, 0);
            lv_obj_set_style_border_width(canyonR, 0, 0);
            lv_obj_set_style_radius(canyonR, 20, 0);
            disableAllScrolling(canyonR);
            
            // Distant mesa
            lv_obj_t *mesa = lv_obj_create(bgCard);
            lv_obj_set_size(mesa, 120, 60);
            lv_obj_align(mesa, LV_ALIGN_CENTER, 0, 50);
            lv_obj_set_style_bg_color(mesa, lv_color_hex(0xA0522D), 0);
            lv_obj_set_style_border_width(mesa, 0, 0);
            lv_obj_set_style_radius(mesa, 8, 0);
            disableAllScrolling(mesa);
        }
        
        // ═══ WALLPAPER 4: ISLAND PARADISE - Tropical sunset ═══
        else if (userData.wallpaperIndex == 4) {
            // Purple/pink sky
            lv_obj_set_style_bg_color(bgCard, lv_color_hex(0xE6B3CC), 0);
            lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x006994), 0);
            
            // Setting sun
            lv_obj_t *sunGlow = lv_obj_create(bgCard);
            lv_obj_set_size(sunGlow, 120, 120);
            lv_obj_align(sunGlow, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_style_bg_color(sunGlow, lv_color_hex(0xFF69B4), 0);
            lv_obj_set_style_bg_opa(sunGlow, LV_OPA_30, 0);
            lv_obj_set_style_radius(sunGlow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sunGlow, 0, 0);
            disableAllScrolling(sunGlow);
            
            lv_obj_t *sun = lv_obj_create(bgCard);
            lv_obj_set_size(sun, 50, 50);
            lv_obj_align(sun, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_style_bg_color(sun, lv_color_hex(0xFFB6C1), 0);
            lv_obj_set_style_radius(sun, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sun, 0, 0);
            disableAllScrolling(sun);
            
            // Ocean
            lv_obj_t *ocean = lv_obj_create(bgCard);
            lv_obj_set_size(ocean, LCD_WIDTH, LCD_HEIGHT / 3);
            lv_obj_align(ocean, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_obj_set_style_bg_color(ocean, lv_color_hex(0x4169E1), 0);
            lv_obj_set_style_bg_grad_color(ocean, lv_color_hex(0x006994), 0);
            lv_obj_set_style_bg_grad_dir(ocean, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(ocean, 0, 0);
            lv_obj_set_style_radius(ocean, 0, 0);
            disableAllScrolling(ocean);
            
            // Sun reflection on water
            lv_obj_t *reflection = lv_obj_create(bgCard);
            lv_obj_set_size(reflection, 8, 80);
            lv_obj_align(reflection, LV_ALIGN_BOTTOM_MID, 0, -20);
            lv_obj_set_style_bg_color(reflection, lv_color_hex(0xFFB6C1), 0);
            lv_obj_set_style_bg_opa(reflection, LV_OPA_50, 0);
            lv_obj_set_style_border_width(reflection, 0, 0);
            lv_obj_set_style_radius(reflection, 4, 0);
            disableAllScrolling(reflection);
            
            // Palm tree silhouette - left
            lv_obj_t *palmTrunk = lv_obj_create(bgCard);
            lv_obj_set_size(palmTrunk, 8, 100);
            lv_obj_align(palmTrunk, LV_ALIGN_BOTTOM_LEFT, 40, -40);
            lv_obj_set_style_bg_color(palmTrunk, lv_color_hex(0x1A1A2E), 0);
            lv_obj_set_style_border_width(palmTrunk, 0, 0);
            lv_obj_set_style_radius(palmTrunk, 4, 0);
            lv_obj_set_style_transform_angle(palmTrunk, 100, 0);
            disableAllScrolling(palmTrunk);
            
            // Palm leaves
            for (int i = 0; i < 5; i++) {
                lv_obj_t *leaf = lv_obj_create(bgCard);
                lv_obj_set_size(leaf, 40, 8);
                lv_obj_align(leaf, LV_ALIGN_BOTTOM_LEFT, 50, -120);
                lv_obj_set_style_bg_color(leaf, lv_color_hex(0x1A1A2E), 0);
                lv_obj_set_style_border_width(leaf, 0, 0);
                lv_obj_set_style_radius(leaf, 4, 0);
                lv_obj_set_style_transform_angle(leaf, (i - 2) * 250, 0);
                disableAllScrolling(leaf);
            }
        }
        
        // ═══ WALLPAPER 5: ALPINE MEADOW - Green mountains ═══
        else if (userData.wallpaperIndex == 5) {
            // Blue sky with golden glow
            lv_obj_set_style_bg_color(bgCard, lv_color_hex(0x87CEEB), 0);
            lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x228B22), 0);
            
            // Sun
            lv_obj_t *sun = lv_obj_create(bgCard);
            lv_obj_set_size(sun, 35, 35);
            lv_obj_align(sun, LV_ALIGN_TOP_RIGHT, -30, 30);
            lv_obj_set_style_bg_color(sun, lv_color_hex(0xFFD700), 0);
            lv_obj_set_style_radius(sun, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(sun, 0, 0);
            lv_obj_set_style_shadow_width(sun, 20, 0);
            lv_obj_set_style_shadow_color(sun, lv_color_hex(0xFFD700), 0);
            disableAllScrolling(sun);
            
            // Distant blue mountain
            lv_obj_t *distMtn = lv_obj_create(bgCard);
            lv_obj_set_size(distMtn, LCD_WIDTH + 40, 100);
            lv_obj_align(distMtn, LV_ALIGN_CENTER, 0, 20);
            lv_obj_set_style_bg_color(distMtn, lv_color_hex(0x6B8E9F), 0);
            lv_obj_set_style_border_width(distMtn, 0, 0);
            lv_obj_set_style_radius(distMtn, 50, 0);
            disableAllScrolling(distMtn);
            
            // Green hills
            lv_obj_t *hill1 = lv_obj_create(bgCard);
            lv_obj_set_size(hill1, 200, 140);
            lv_obj_align(hill1, LV_ALIGN_BOTTOM_LEFT, -40, 20);
            lv_obj_set_style_bg_color(hill1, lv_color_hex(0x32CD32), 0);
            lv_obj_set_style_bg_grad_color(hill1, lv_color_hex(0x228B22), 0);
            lv_obj_set_style_bg_grad_dir(hill1, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(hill1, 0, 0);
            lv_obj_set_style_radius(hill1, 100, 0);
            disableAllScrolling(hill1);
            
            lv_obj_t *hill2 = lv_obj_create(bgCard);
            lv_obj_set_size(hill2, 180, 120);
            lv_obj_align(hill2, LV_ALIGN_BOTTOM_RIGHT, 30, 10);
            lv_obj_set_style_bg_color(hill2, lv_color_hex(0x3CB371), 0);
            lv_obj_set_style_bg_grad_color(hill2, lv_color_hex(0x2E8B57), 0);
            lv_obj_set_style_bg_grad_dir(hill2, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(hill2, 0, 0);
            lv_obj_set_style_radius(hill2, 90, 0);
            disableAllScrolling(hill2);
            
            // Flowers (yellow dots)
            for (int i = 0; i < 8; i++) {
                lv_obj_t *flower = lv_obj_create(bgCard);
                lv_obj_set_size(flower, 6, 6);
                lv_obj_align(flower, LV_ALIGN_BOTTOM_MID, (i - 4) * 20, -40 - (i % 3) * 15);
                lv_obj_set_style_bg_color(flower, lv_color_hex(0xFFD700), 0);
                lv_obj_set_style_radius(flower, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(flower, 0, 0);
                disableAllScrolling(flower);
            }
        }
        
        // ═══ WALLPAPER 6: TWILIGHT OCEAN - Night sea ═══
        else if (userData.wallpaperIndex == 6) {
            // Deep night sky
            lv_obj_set_style_bg_color(bgCard, lv_color_hex(0x0D0D1A), 0);
            lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x1A1A3A), 0);
            
            // Stars
            for (int i = 0; i < 20; i++) {
                lv_obj_t *star = lv_obj_create(bgCard);
                int size = (i % 3) + 2;
                lv_obj_set_size(star, size, size);
                lv_obj_set_pos(star, (i * 37) % LCD_WIDTH, (i * 23) % (LCD_HEIGHT / 2));
                lv_obj_set_style_bg_color(star, lv_color_hex(0xFFFFFF), 0);
                lv_obj_set_style_bg_opa(star, LV_OPA_40 + (i % 4) * 20, 0);
                lv_obj_set_style_radius(star, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(star, 0, 0);
                disableAllScrolling(star);
            }
            
            // Moon
            lv_obj_t *moonGlow = lv_obj_create(bgCard);
            lv_obj_set_size(moonGlow, 70, 70);
            lv_obj_align(moonGlow, LV_ALIGN_TOP_RIGHT, -25, 35);
            lv_obj_set_style_bg_color(moonGlow, lv_color_hex(0xE6E6FA), 0);
            lv_obj_set_style_bg_opa(moonGlow, LV_OPA_20, 0);
            lv_obj_set_style_radius(moonGlow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(moonGlow, 0, 0);
            disableAllScrolling(moonGlow);
            
            lv_obj_t *moon = lv_obj_create(bgCard);
            lv_obj_set_size(moon, 35, 35);
            lv_obj_align(moon, LV_ALIGN_TOP_RIGHT, -25, 40);
            lv_obj_set_style_bg_color(moon, lv_color_hex(0xFFFACD), 0);
            lv_obj_set_style_radius(moon, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(moon, 0, 0);
            disableAllScrolling(moon);
            
            // Ocean waves
            lv_obj_t *ocean = lv_obj_create(bgCard);
            lv_obj_set_size(ocean, LCD_WIDTH, LCD_HEIGHT / 2 + 20);
            lv_obj_align(ocean, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_obj_set_style_bg_color(ocean, lv_color_hex(0x191970), 0);
            lv_obj_set_style_bg_grad_color(ocean, lv_color_hex(0x000033), 0);
            lv_obj_set_style_bg_grad_dir(ocean, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_width(ocean, 0, 0);
            lv_obj_set_style_radius(ocean, 0, 0);
            disableAllScrolling(ocean);
            
            // Moon reflection
            lv_obj_t *moonReflect = lv_obj_create(bgCard);
            lv_obj_set_size(moonReflect, 6, 100);
            lv_obj_align(moonReflect, LV_ALIGN_BOTTOM_RIGHT, -40, -20);
            lv_obj_set_style_bg_color(moonReflect, lv_color_hex(0xFFFACD), 0);
            lv_obj_set_style_bg_opa(moonReflect, LV_OPA_30, 0);
            lv_obj_set_style_border_width(moonReflect, 0, 0);
            lv_obj_set_style_radius(moonReflect, 3, 0);
            disableAllScrolling(moonReflect);
            
            // Wave lines
            for (int i = 0; i < 4; i++) {
                lv_obj_t *wave = lv_obj_create(bgCard);
                lv_obj_set_size(wave, LCD_WIDTH - 40, 3);
                lv_obj_align(wave, LV_ALIGN_BOTTOM_MID, 0, -30 - i * 25);
                lv_obj_set_style_bg_color(wave, lv_color_hex(0x4169E1), 0);
                lv_obj_set_style_bg_opa(wave, LV_OPA_30 - i * 5, 0);
                lv_obj_set_style_border_width(wave, 0, 0);
                lv_obj_set_style_radius(wave, 2, 0);
                disableAllScrolling(wave);
            }
        }
    } else {
        // Solid theme gradient (no wallpaper)
        lv_obj_set_style_bg_color(bgCard, theme->color1, 0);
        lv_obj_set_style_bg_grad_color(bgCard, theme->color2, 0);
        lv_obj_set_style_bg_grad_dir(bgCard, LV_GRAD_DIR_VER, 0);
    }

    // Main card container (transparent overlay for content)
    lv_obj_t *card = lv_obj_create(bgCard);
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    disableAllScrolling(card);

    // ═══════════════════════════════════════════════════════════════════════
    // DISPATCH TO SELECTED WATCH FACE
    // ═══════════════════════════════════════════════════════════════════════
    
    // Nike face requires pure black background
    if (userData.watchFaceIndex == 3) {
        lv_obj_set_style_bg_color(bgCard, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_grad_color(bgCard, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(bgCard, LV_OPA_COVER, 0);
    }
    
    switch (userData.watchFaceIndex) {
        case 0:
            createDigitalFace(card);
            break;
        case 1:
            createWordClockFace(card);
            break;
        case 2:
            createAnalogRingsFace(card);
            break;
        case 3:
            createNikeStyleFace(card);
            break;
        case 4:
            createMinimalDarkFace(card);
            break;
        case 5:
            createFitnessRingsFace(card);
            break;
        default:
            createDigitalFace(card);
            break;
    }
    
    // Add hint for Time Settings access (swipe down)
    lv_obj_t *hint = lv_label_create(card);
    lv_label_set_text(hint, LV_SYMBOL_DOWN " Set Time");
    lv_obj_set_style_text_color(hint, theme->secondary, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_opa(hint, 180, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// ═══════════════════════════════════════════════════════════════════════════
// COMPASS CARD - APPLE WATCH STYLE WITH SUNRISE/SUNSET
// ═══════════════════════════════════════════════════════════════════════════
void createCompassCard() {
    disableAllScrolling(lv_scr_act());
    calculateSunTimes();

    // Pure black background - Apple Watch style
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    disableAllScrolling(card);

    int centerX = LCD_WIDTH / 2;
    int centerY = LCD_HEIGHT / 2;
    int outerRadius = 125;  // BIGGER COMPASS - was 95, now 125

    // Calibrated heading
    float calibratedHeading = compassHeadingSmooth - compassNorthOffset;
    if (calibratedHeading < 0) calibratedHeading += 360;
    if (calibratedHeading >= 360) calibratedHeading -= 360;
    float headingRad = calibratedHeading * M_PI / 180.0;

    // ═══ OUTER TICK MARKS - Rotate with compass ═══
    for (int deg = 0; deg < 360; deg += 6) {
        float actualDeg = deg - calibratedHeading;
        float rad = actualDeg * M_PI / 180.0;
        
        bool isCardinal = (deg == 0 || deg == 90 || deg == 180 || deg == 270);
        bool isMajor = (deg % 30 == 0);
        int tickLen = isCardinal ? 20 : (isMajor ? 14 : 7);  // Bigger ticks
        int tickWidth = isCardinal ? 4 : (isMajor ? 3 : 2);
        
        int outerR = outerRadius;
        int innerR = outerRadius - tickLen;
        
        int x1 = centerX + (int)(outerR * sin(rad));
        int y1 = centerY - (int)(outerR * cos(rad));
        int x2 = centerX + (int)(innerR * sin(rad));
        int y2 = centerY - (int)(innerR * cos(rad));

        lv_obj_t *tick = lv_obj_create(card);
        lv_obj_set_size(tick, tickWidth, tickLen);
        lv_obj_align(tick, LV_ALIGN_CENTER, (int)(innerR * sin(rad)), -(int)(innerR * cos(rad)) - tickLen/2);
        
        uint32_t tickColor = isCardinal ? 0xFFFFFF : (isMajor ? 0x8E8E93 : 0x48484A);
        lv_obj_set_style_bg_color(tick, lv_color_hex(tickColor), 0);
        lv_obj_set_style_radius(tick, 0, 0);
        lv_obj_set_style_border_width(tick, 0, 0);
        lv_obj_set_style_transform_pivot_x(tick, tickWidth/2, 0);
        lv_obj_set_style_transform_pivot_y(tick, tickLen/2, 0);
        lv_obj_set_style_transform_angle(tick, (int)(actualDeg * 10), 0);
        disableAllScrolling(tick);
    }

    // ═══ CARDINAL DIRECTIONS - Rotate with compass ═══
    const char* cardinals[] = {"N", "E", "S", "W"};
    int cardinalDegs[] = {0, 90, 180, 270};
    uint32_t cardinalColors[] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
    
    for (int i = 0; i < 4; i++) {
        float actualDeg = cardinalDegs[i] - calibratedHeading;
        float rad = actualDeg * M_PI / 180.0;
        int labelR = outerRadius - 38;  // Adjusted for bigger compass
        
        int lx = centerX + (int)(labelR * sin(rad));
        int ly = centerY - (int)(labelR * cos(rad));
        
        lv_obj_t *cardLabel = lv_label_create(card);
        lv_label_set_text(cardLabel, cardinals[i]);
        lv_obj_set_style_text_color(cardLabel, lv_color_hex(cardinalColors[i]), 0);
        lv_obj_set_style_text_font(cardLabel, &lv_font_montserrat_24, 0);  // Bigger font
        lv_obj_set_pos(cardLabel, lx - 10, ly - 14);
    }

    // ═══ COMPASS NEEDLE - Apple Watch Red/Blue style ═══
    // Red needle pointing UP (North)
    int needleLen = 75;  // Bigger needle - was 55
    int needleWidth = 14;  // Slightly wider
    
    // Red (North) half - pointing up
    lv_obj_t *redNeedle = lv_obj_create(card);
    lv_obj_set_size(redNeedle, needleWidth, needleLen);
    lv_obj_align(redNeedle, LV_ALIGN_CENTER, 0, -needleLen/2);
    lv_obj_set_style_bg_color(redNeedle, lv_color_hex(0xFF3B30), 0);  // Apple red
    lv_obj_set_style_radius(redNeedle, 2, 0);
    lv_obj_set_style_border_width(redNeedle, 0, 0);
    disableAllScrolling(redNeedle);

    // Pointy tip for red needle
    lv_obj_t *redTip = lv_obj_create(card);
    lv_obj_set_size(redTip, needleWidth + 8, 22);
    lv_obj_align(redTip, LV_ALIGN_CENTER, 0, -needleLen + 6);
    lv_obj_set_style_bg_color(redTip, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_radius(redTip, 2, 0);
    lv_obj_set_style_border_width(redTip, 0, 0);
    disableAllScrolling(redTip);
    
    // Blue (South) half - pointing down
    lv_obj_t *blueNeedle = lv_obj_create(card);
    lv_obj_set_size(blueNeedle, needleWidth, needleLen);
    lv_obj_align(blueNeedle, LV_ALIGN_CENTER, 0, needleLen/2);
    lv_obj_set_style_bg_color(blueNeedle, lv_color_hex(0x007AFF), 0);  // Apple blue
    lv_obj_set_style_radius(blueNeedle, 2, 0);
    lv_obj_set_style_border_width(blueNeedle, 0, 0);
    disableAllScrolling(blueNeedle);

    // Pointy tip for blue needle
    lv_obj_t *blueTip = lv_obj_create(card);
    lv_obj_set_size(blueTip, needleWidth + 8, 22);
    lv_obj_align(blueTip, LV_ALIGN_CENTER, 0, needleLen - 6);
    lv_obj_set_style_bg_color(blueTip, lv_color_hex(0x007AFF), 0);
    lv_obj_set_style_radius(blueTip, 2, 0);
    lv_obj_set_style_border_width(blueTip, 0, 0);
    disableAllScrolling(blueTip);

    // Center white dot
    lv_obj_t *centerDot = lv_obj_create(card);
    lv_obj_set_size(centerDot, 22, 22);
    lv_obj_center(centerDot);
    lv_obj_set_style_bg_color(centerDot, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(centerDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(centerDot, 0, 0);
    disableAllScrolling(centerDot);

    // Heading display at top
    char headingBuf[16];
    snprintf(headingBuf, sizeof(headingBuf), "%d°", (int)calibratedHeading);
    lv_obj_t *headingLabel = lv_label_create(card);
    lv_label_set_text(headingLabel, headingBuf);
    lv_obj_set_style_text_color(headingLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(headingLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(headingLabel, LV_ALIGN_TOP_MID, 0, 8);

    // Calibration hint
    lv_obj_t *calibHint = lv_label_create(card);
    lv_label_set_text(calibHint, "TAP TO CALIBRATE");
    lv_obj_set_style_text_color(calibHint, lv_color_hex(0x3A3A3C), 0);
    lv_obj_set_style_text_font(calibHint, &lv_font_montserrat_10, 0);
    lv_obj_align(calibHint, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// ═══════════════════════════════════════════════════════════════════════════
// TILT CARD
// ═══════════════════════════════════════════════════════════════════════════
void createTiltCard() {
    GradientTheme *theme = getSafeTheme();

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, theme->color1, 0);
    lv_obj_set_style_bg_grad_color(card, theme->color2, 0);
    lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "TILT SENSOR");
    lv_obj_set_style_text_color(title, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    char tiltStr[32];
    snprintf(tiltStr, sizeof(tiltStr), "X: %.1f°", tiltX);
    lv_obj_t *tiltXLbl = lv_label_create(card);
    lv_label_set_text(tiltXLbl, tiltStr);
    lv_obj_set_style_text_color(tiltXLbl, theme->text, 0);
    lv_obj_set_style_text_font(tiltXLbl, &lv_font_montserrat_28, 0);
    lv_obj_align(tiltXLbl, LV_ALIGN_CENTER, 0, -30);

    snprintf(tiltStr, sizeof(tiltStr), "Y: %.1f°", tiltY);
    lv_obj_t *tiltYLbl = lv_label_create(card);
    lv_label_set_text(tiltYLbl, tiltStr);
    lv_obj_set_style_text_color(tiltYLbl, theme->text, 0);
    lv_obj_set_style_text_font(tiltYLbl, &lv_font_montserrat_28, 0);
    lv_obj_align(tiltYLbl, LV_ALIGN_CENTER, 0, 30);

    lv_obj_t *levelLbl = lv_label_create(card);
    bool isLevel = (abs(tiltX) < 3 && abs(tiltY) < 3);
    lv_label_set_text(levelLbl, isLevel ? "LEVEL" : "TILTED");
    lv_obj_set_style_text_color(levelLbl, isLevel ? lv_color_hex(0x34C759) : lv_color_hex(0xFF9500), 0);
    lv_obj_align(levelLbl, LV_ALIGN_BOTTOM_MID, 0, -40);
}

// ═══════════════════════════════════════════════════════════════════════════
// STEPS CARD - USBO GRADIENT STYLE (Purple/Cyan like reference image)
// ═══════════════════════════════════════════════════════════════════════════
void createStepsCard() {
    disableAllScrolling(lv_scr_act());

    // Deep dark background for contrast
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D1A), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Main gradient card - USBO style with purple/cyan gradient feel
    lv_obj_t *mainCard = lv_obj_create(card);
    lv_obj_set_size(mainCard, LCD_WIDTH - 24, LCD_HEIGHT - 40);
    lv_obj_align(mainCard, LV_ALIGN_CENTER, 0, 0);
    // Use a purple-blue gradient base color
    lv_obj_set_style_bg_color(mainCard, lv_color_hex(0x6B4CE6), 0);  // Rich purple
    lv_obj_set_style_bg_grad_color(mainCard, lv_color_hex(0x00D4FF), 0);  // Cyan
    lv_obj_set_style_bg_grad_dir(mainCard, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(mainCard, 28, 0);
    lv_obj_set_style_border_width(mainCard, 0, 0);
    lv_obj_set_style_shadow_width(mainCard, 30, 0);
    lv_obj_set_style_shadow_color(mainCard, lv_color_hex(0x6B4CE6), 0);
    lv_obj_set_style_shadow_opa(mainCard, LV_OPA_40, 0);
    disableAllScrolling(mainCard);

    // App title badge - top left (like USBO App)
    lv_obj_t *badge = lv_obj_create(mainCard);
    lv_obj_set_size(badge, 100, 30);
    lv_obj_align(badge, LV_ALIGN_TOP_LEFT, 16, 16);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_20, 0);
    lv_obj_set_style_radius(badge, 8, 0);
    lv_obj_set_style_border_width(badge, 0, 0);
    disableAllScrolling(badge);

    lv_obj_t *badgeIcon = lv_label_create(badge);
    lv_label_set_text(badgeIcon, LV_SYMBOL_CHARGE " USBO App");
    lv_obj_set_style_text_color(badgeIcon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(badgeIcon, &lv_font_montserrat_12, 0);
    lv_obj_center(badgeIcon);

    // "Steps:" label
    lv_obj_t *stepsLabel = lv_label_create(mainCard);
    lv_label_set_text(stepsLabel, "Steps:");
    lv_obj_set_style_text_color(stepsLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(stepsLabel, LV_OPA_80, 0);
    lv_obj_set_style_text_font(stepsLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(stepsLabel, LV_ALIGN_TOP_LEFT, 18, 60);

    // HUGE step count - NIKE FONT bold white number (main focus like reference)
    char stepBuf[16];
    snprintf(stepBuf, sizeof(stepBuf), "%lu", (unsigned long)userData.steps);
    lv_obj_t *stepCount = lv_label_create(mainCard);
    lv_label_set_text(stepCount, stepBuf);
    lv_obj_set_style_text_color(stepCount, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stepCount, &NIKE_FONT, 0);  // Use Nike font for bigger, bolder display
    lv_obj_align(stepCount, LV_ALIGN_CENTER, 0, -15);
    
    // Distance below steps - using Nike font at size 20
    char distCardBuf[24];
    snprintf(distCardBuf, sizeof(distCardBuf), "%.2f KM", userData.totalDistance);
    lv_obj_t *distanceOnCard = lv_label_create(mainCard);
    lv_label_set_text(distanceOnCard, distCardBuf);
    lv_obj_set_style_text_color(distanceOnCard, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(distanceOnCard, LV_OPA_90, 0);
    lv_obj_set_style_text_font(distanceOnCard, &lv_font_montserrat_20, 0);  // Bigger distance text
    lv_obj_align(distanceOnCard, LV_ALIGN_CENTER, 0, 25);

    // Goal progress calculation
    int progress = (userData.steps * 100) / userData.dailyGoal;
    if (progress > 100) progress = 100;

    // Progress bar container - milestone style like reference (2000 4000 6000 8000)
    lv_obj_t *progressContainer = lv_obj_create(mainCard);
    lv_obj_set_size(progressContainer, LCD_WIDTH - 60, 50);
    lv_obj_align(progressContainer, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_opa(progressContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(progressContainer, 0, 0);
    disableAllScrolling(progressContainer);

    // Milestone labels
    const int milestones[] = {2000, 4000, 6000, 8000};
    const int numMilestones = 4;
    int barWidth = (LCD_WIDTH - 80) / numMilestones;

    for (int i = 0; i < numMilestones; i++) {
        // Milestone number label
        char mileBuf[8];
        snprintf(mileBuf, sizeof(mileBuf), "%d", milestones[i]);
        lv_obj_t *mileLabel = lv_label_create(progressContainer);
        lv_label_set_text(mileLabel, mileBuf);
        lv_obj_set_style_text_color(mileLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(mileLabel, LV_OPA_80, 0);
        lv_obj_set_style_text_font(mileLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(mileLabel, LV_ALIGN_TOP_LEFT, i * barWidth + 10, 0);

        // Progress bar segment
        lv_obj_t *barSegment = lv_obj_create(progressContainer);
        lv_obj_set_size(barSegment, barWidth - 8, 6);
        lv_obj_align(barSegment, LV_ALIGN_TOP_LEFT, i * barWidth + 5, 22);
        lv_obj_set_style_radius(barSegment, 3, 0);
        lv_obj_set_style_border_width(barSegment, 0, 0);
        disableAllScrolling(barSegment);

        // Color based on whether milestone is reached
        if ((int)userData.steps >= milestones[i]) {
            lv_obj_set_style_bg_color(barSegment, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(barSegment, LV_OPA_100, 0);
        } else if (i == 0 || (int)userData.steps >= milestones[i-1]) {
            // Partial progress in current segment
            lv_obj_set_style_bg_color(barSegment, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(barSegment, LV_OPA_40, 0);
        } else {
            lv_obj_set_style_bg_color(barSegment, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(barSegment, LV_OPA_20, 0);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// DISTANCE CARD - DARK MINIMAL STYLE (Like reference image with daily goal)
// ═══════════════════════════════════════════════════════════════════════════
void createDistanceCard() {
    disableAllScrolling(lv_scr_act());

    // Pure dark AMOLED background
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Main dark card container - rounded corners like reference
    lv_obj_t *mainCard = lv_obj_create(card);
    lv_obj_set_size(mainCard, LCD_WIDTH - 24, LCD_HEIGHT - 40);
    lv_obj_align(mainCard, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(mainCard, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_radius(mainCard, 28, 0);
    lv_obj_set_style_border_width(mainCard, 0, 0);
    disableAllScrolling(mainCard);

    // Top text section - "Today you've walked X steps, that's around X KM."
    lv_obj_t *todayLabel = lv_label_create(mainCard);
    lv_label_set_text(todayLabel, "Today");
    lv_obj_set_style_text_color(todayLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(todayLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(todayLabel, LV_ALIGN_TOP_LEFT, 20, 30);

    lv_obj_t *walkedLabel = lv_label_create(mainCard);
    lv_label_set_text(walkedLabel, "you've walked");
    lv_obj_set_style_text_color(walkedLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(walkedLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(walkedLabel, LV_ALIGN_TOP_LEFT, 75, 33);

    // Steps count - bold white
    char stepsBuf[24];
    snprintf(stepsBuf, sizeof(stepsBuf), "%lu steps,", (unsigned long)userData.steps);
    lv_obj_t *stepsCountLabel = lv_label_create(mainCard);
    lv_label_set_text(stepsCountLabel, stepsBuf);
    lv_obj_set_style_text_color(stepsCountLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stepsCountLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(stepsCountLabel, LV_ALIGN_TOP_LEFT, 20, 58);

    // "that's around"
    lv_obj_t *aroundLabel = lv_label_create(mainCard);
    lv_label_set_text(aroundLabel, "that's around");
    lv_obj_set_style_text_color(aroundLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(aroundLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(aroundLabel, LV_ALIGN_TOP_LEFT, 20, 88);

    // Distance KM - BIGGER with Nike-inspired styling (size 20)
    char distBuf[16];
    snprintf(distBuf, sizeof(distBuf), "%.1f KM", userData.totalDistance);
    lv_obj_t *distLabel = lv_label_create(mainCard);
    lv_label_set_text(distLabel, distBuf);
    lv_obj_set_style_text_color(distLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(distLabel, &lv_font_montserrat_20, 0);  // Bigger distance text (20pt)
    lv_obj_align(distLabel, LV_ALIGN_TOP_LEFT, 120, 83);

    // Divider line
    lv_obj_t *divider = lv_obj_create(mainCard);
    lv_obj_set_size(divider, LCD_WIDTH - 70, 1);
    lv_obj_align(divider, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(divider, 0, 0);
    lv_obj_set_style_border_width(divider, 0, 0);
    disableAllScrolling(divider);

    // Bottom section - Daily goal with footprint icon
    // Footprint icon (using GPS as placeholder since LVGL doesn't have footprint)
    lv_obj_t *footIcon = lv_label_create(mainCard);
    lv_label_set_text(footIcon, LV_SYMBOL_SHUFFLE);  // Using shuffle as footsteps placeholder
    lv_obj_set_style_text_color(footIcon, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(footIcon, &lv_font_montserrat_32, 0);
    lv_obj_align(footIcon, LV_ALIGN_BOTTOM_LEFT, 25, -50);

    // "Your daily goal:" text
    lv_obj_t *goalTextLabel = lv_label_create(mainCard);
    lv_label_set_text(goalTextLabel, "Your");
    lv_obj_set_style_text_color(goalTextLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(goalTextLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(goalTextLabel, LV_ALIGN_BOTTOM_LEFT, 70, -75);

    lv_obj_t *dailyGoalLabel = lv_label_create(mainCard);
    lv_label_set_text(dailyGoalLabel, "daily goal:");
    lv_obj_set_style_text_color(dailyGoalLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(dailyGoalLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(dailyGoalLabel, LV_ALIGN_BOTTOM_LEFT, 105, -72);

    // Daily goal steps value
    char goalBuf[24];
    // Format with comma: 3,500 steps
    if (userData.dailyGoal >= 1000) {
        snprintf(goalBuf, sizeof(goalBuf), "%d,%03d steps.", 
                 (int)(userData.dailyGoal / 1000), 
                 (int)(userData.dailyGoal % 1000));
    } else {
        snprintf(goalBuf, sizeof(goalBuf), "%lu steps.", (unsigned long)userData.dailyGoal);
    }
    lv_obj_t *goalValueLabel = lv_label_create(mainCard);
    lv_label_set_text(goalValueLabel, goalBuf);
    lv_obj_set_style_text_color(goalValueLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(goalValueLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(goalValueLabel, LV_ALIGN_BOTTOM_LEFT, 70, -45);
}

// ═══════════════════════════════════════════════════════════════════════════
// BLACKJACK HELPER FUNCTIONS - PREMIUM VISUAL CARDS
// ═══════════════════════════════════════════════════════════════════════════
void drawPlayingCard(lv_obj_t *parent, int cardValue, int x, int y, bool faceUp) {
    lv_obj_t *cardObj = lv_obj_create(parent);
    lv_obj_set_size(cardObj, 32, 45);
    lv_obj_set_pos(cardObj, x, y);
    lv_obj_set_style_radius(cardObj, 4, 0);
    lv_obj_set_style_border_width(cardObj, 1, 0);
    disableAllScrolling(cardObj);

    if (faceUp) {
        lv_obj_set_style_bg_color(cardObj, lv_color_hex(0xFFFFF0), 0);
        lv_obj_set_style_border_color(cardObj, lv_color_hex(0x000000), 0);

        // Card value
        char valStr[3];
        int displayVal = (cardValue % 13) + 1;
        if (displayVal == 1) strcpy(valStr, "A");
        else if (displayVal == 11) strcpy(valStr, "J");
        else if (displayVal == 12) strcpy(valStr, "Q");
        else if (displayVal == 13) strcpy(valStr, "K");
        else snprintf(valStr, sizeof(valStr), "%d", displayVal);

        lv_obj_t *valLabel = lv_label_create(cardObj);
        lv_label_set_text(valLabel, valStr);

        // Red for hearts/diamonds, black for clubs/spades
        int suit = cardValue / 13;
        lv_color_t suitColor = (suit < 2) ? lv_color_hex(0xFF0000) : lv_color_hex(0x000000);
        lv_obj_set_style_text_color(valLabel, suitColor, 0);
        lv_obj_set_style_text_font(valLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(valLabel, LV_ALIGN_TOP_LEFT, 2, 2);

        // Suit symbol
        const char* suits[] = {"H", "D", "C", "S"};  // Simplified
        lv_obj_t *suitLabel = lv_label_create(cardObj);
        lv_label_set_text(suitLabel, suits[suit]);
        lv_obj_set_style_text_color(suitLabel, suitColor, 0);
        lv_obj_set_style_text_font(suitLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(suitLabel, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    } else {
        // Face down - show pattern
        lv_obj_set_style_bg_color(cardObj, lv_color_hex(0x1E3A5F), 0);
        lv_obj_set_style_border_color(cardObj, lv_color_hex(0xFFD700), 0);

        lv_obj_t *pattern = lv_label_create(cardObj);
        lv_label_set_text(pattern, "?");
        lv_obj_set_style_text_color(pattern, lv_color_hex(0xFFD700), 0);
        lv_obj_center(pattern);
    }
}

int calculateHandValue(int *cards, int count) {
    int value = 0;
    int aces = 0;

    for (int i = 0; i < count; i++) {
        int cardVal = (cards[i] % 13) + 1;
        if (cardVal == 1) {
            aces++;
            value += 11;
        } else if (cardVal >= 10) {
            value += 10;
        } else {
            value += cardVal;
        }
    }

    // Convert aces from 11 to 1 if busting
    while (value > 21 && aces > 0) {
        value -= 10;
        aces--;
    }

    return value;
}

void dealCard(int *cards, int *count, bool toPlayer) {
    if (*count >= 10) return;
    cards[*count] = random(52);
    (*count)++;
}

// ═══════════════════════════════════════════════════════════════════════════
// BLACKJACK CARD - GREEN CASINO TABLE STYLE
// ═══════════════════════════════════════════════════════════════════════════
void createBlackjackCard() {
    disableAllScrolling(lv_scr_act());

    // Casino green felt table background
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D4A2B), 0);  // Deep casino green
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Felt texture overlay - darker green border effect
    lv_obj_t *feltBorder = lv_obj_create(card);
    lv_obj_set_size(feltBorder, LCD_WIDTH - 8, LCD_HEIGHT - 8);
    lv_obj_center(feltBorder);
    lv_obj_set_style_bg_color(feltBorder, lv_color_hex(0x1A6B40), 0);  // Lighter casino green felt
    lv_obj_set_style_radius(feltBorder, 20, 0);
    lv_obj_set_style_border_width(feltBorder, 3, 0);
    lv_obj_set_style_border_color(feltBorder, lv_color_hex(0x8B4513), 0);  // Wood brown border
    disableAllScrolling(feltBorder);

    // BLACKJACK title - gold on green
    lv_obj_t *titleLabel = lv_label_create(feltBorder);
    lv_label_set_text(titleLabel, "BLACKJACK");
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xFFD700), 0);  // Gold
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 8);

    // Dealer section - semi-transparent darker area
    lv_obj_t *dealerSection = lv_obj_create(feltBorder);
    lv_obj_set_size(dealerSection, LCD_WIDTH - 32, 85);
    lv_obj_align(dealerSection, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_bg_color(dealerSection, lv_color_hex(0x0D4A2B), 0);
    lv_obj_set_style_bg_opa(dealerSection, LV_OPA_60, 0);
    lv_obj_set_style_radius(dealerSection, 12, 0);
    lv_obj_set_style_border_width(dealerSection, 1, 0);
    lv_obj_set_style_border_color(dealerSection, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_border_opa(dealerSection, LV_OPA_30, 0);
    disableAllScrolling(dealerSection);

    lv_obj_t *dealerLabel = lv_label_create(dealerSection);
    lv_label_set_text(dealerLabel, "DEALER");
    lv_obj_set_style_text_color(dealerLabel, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_text_font(dealerLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(dealerLabel, LV_ALIGN_TOP_LEFT, 10, 5);

    // Draw dealer cards
    for (int i = 0; i < dealerCount; i++) {
        bool showCard = (i == 0) || playerStand || !blackjackGameActive;
        drawPlayingCard(dealerSection, dealerCards[i], 10 + i * 36, 22, showCard);
    }

    // Show dealer value
    if ((playerStand && blackjackGameActive) || !blackjackGameActive) {
        int dealerVal = calculateHandValue(dealerCards, dealerCount);
        if (dealerCount > 0) {
            char dealerBuf[8];
            snprintf(dealerBuf, sizeof(dealerBuf), "%d", dealerVal);
            lv_obj_t *dealerValLabel = lv_label_create(dealerSection);
            lv_label_set_text(dealerValLabel, dealerBuf);
            lv_obj_set_style_text_color(dealerValLabel, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(dealerValLabel, &lv_font_montserrat_24, 0);
            lv_obj_align(dealerValLabel, LV_ALIGN_RIGHT_MID, -12, 5);
        }
    }

    // Player section
    lv_obj_t *playerSection = lv_obj_create(feltBorder);
    lv_obj_set_size(playerSection, LCD_WIDTH - 32, 85);
    lv_obj_align(playerSection, LV_ALIGN_TOP_MID, 0, 128);
    lv_obj_set_style_bg_color(playerSection, lv_color_hex(0x0D4A2B), 0);
    lv_obj_set_style_bg_opa(playerSection, LV_OPA_60, 0);
    lv_obj_set_style_radius(playerSection, 12, 0);
    lv_obj_set_style_border_width(playerSection, 1, 0);
    lv_obj_set_style_border_color(playerSection, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_border_opa(playerSection, LV_OPA_30, 0);
    disableAllScrolling(playerSection);

    lv_obj_t *playerLabel = lv_label_create(playerSection);
    lv_label_set_text(playerLabel, "YOUR HAND");
    lv_obj_set_style_text_color(playerLabel, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_text_font(playerLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(playerLabel, LV_ALIGN_TOP_LEFT, 10, 5);

    // Draw player cards
    for (int i = 0; i < playerCount; i++) {
        drawPlayingCard(playerSection, playerCards[i], 10 + i * 36, 22, true);
    }

    // Player value
    int playerVal = calculateHandValue(playerCards, playerCount);
    if (playerCount > 0) {
        char playerBuf[8];
        snprintf(playerBuf, sizeof(playerBuf), "%d", playerVal);
        lv_obj_t *playerValLabel = lv_label_create(playerSection);
        lv_label_set_text(playerValLabel, playerBuf);
        uint32_t valColor = (playerVal > 21) ? 0xFF453A : (playerVal == 21 ? 0xFFD700 : 0xFFFFFF);
        lv_obj_set_style_text_color(playerValLabel, lv_color_hex(valColor), 0);
        lv_obj_set_style_text_font(playerValLabel, &lv_font_montserrat_24, 0);
        lv_obj_align(playerValLabel, LV_ALIGN_RIGHT_MID, -12, 5);
    }

    // Bottom action area - on the felt
    lv_obj_t *actionArea = lv_obj_create(feltBorder);
    lv_obj_set_size(actionArea, LCD_WIDTH - 32, 75);
    lv_obj_align(actionArea, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(actionArea, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actionArea, 0, 0);
    disableAllScrolling(actionArea);

    if (!blackjackGameActive) {
        // New game state - gold deal button
        lv_obj_t *dealBtn = lv_obj_create(actionArea);
        lv_obj_set_size(dealBtn, LCD_WIDTH - 70, 42);
        lv_obj_align(dealBtn, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_set_style_bg_color(dealBtn, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_radius(dealBtn, 21, 0);
        lv_obj_set_style_border_width(dealBtn, 2, 0);
        lv_obj_set_style_border_color(dealBtn, lv_color_hex(0x8B4513), 0);
        lv_obj_set_style_shadow_width(dealBtn, 8, 0);
        lv_obj_set_style_shadow_color(dealBtn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(dealBtn, LV_OPA_40, 0);
        disableAllScrolling(dealBtn);

        lv_obj_t *dealLabel = lv_label_create(dealBtn);
        lv_label_set_text(dealLabel, "TAP TO DEAL");
        lv_obj_set_style_text_color(dealLabel, lv_color_hex(0x0D4A2B), 0);
        lv_obj_set_style_text_font(dealLabel, &lv_font_montserrat_16, 0);
        lv_obj_center(dealLabel);

        // Stats row with Win/Loss ratio
        char statsBuf[48];
        int losses = userData.gamesPlayed - userData.gamesWon;
        float winRate = userData.gamesPlayed > 0 ? (float)userData.gamesWon / userData.gamesPlayed * 100.0f : 0.0f;
        snprintf(statsBuf, sizeof(statsBuf), "W:%d L:%d (%.0f%%)", userData.gamesWon, losses, winRate);
        lv_obj_t *statsLabel = lv_label_create(actionArea);
        lv_label_set_text(statsLabel, statsBuf);
        lv_obj_set_style_text_color(statsLabel, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_text_opa(statsLabel, LV_OPA_70, 0);
        lv_obj_set_style_text_font(statsLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(statsLabel, LV_ALIGN_BOTTOM_MID, 0, -5);

    } else if (playerVal > 21) {
        // Bust state - red text
        lv_obj_t *bustLabel = lv_label_create(actionArea);
        lv_label_set_text(bustLabel, "BUST!");
        lv_obj_set_style_text_color(bustLabel, lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_text_font(bustLabel, &lv_font_montserrat_32, 0);
        lv_obj_align(bustLabel, LV_ALIGN_TOP_MID, 0, 8);

        lv_obj_t *tapLabel = lv_label_create(actionArea);
        lv_label_set_text(tapLabel, "Tap for new game");
        lv_obj_set_style_text_color(tapLabel, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_text_opa(tapLabel, LV_OPA_60, 0);
        lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -8);

    } else if (!playerStand) {
        // Action buttons - HIT (green) and STAND (red)
        lv_obj_t *hitBtn = lv_obj_create(actionArea);
        lv_obj_set_size(hitBtn, 85, 48);
        lv_obj_align(hitBtn, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_bg_color(hitBtn, lv_color_hex(0x228B22), 0);  // Forest green
        lv_obj_set_style_radius(hitBtn, 24, 0);
        lv_obj_set_style_border_width(hitBtn, 2, 0);
        lv_obj_set_style_border_color(hitBtn, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_shadow_width(hitBtn, 6, 0);
        lv_obj_set_style_shadow_color(hitBtn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(hitBtn, LV_OPA_50, 0);
        disableAllScrolling(hitBtn);

        lv_obj_t *hitLabel = lv_label_create(hitBtn);
        lv_label_set_text(hitLabel, "HIT");
        lv_obj_set_style_text_color(hitLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(hitLabel, &lv_font_montserrat_16, 0);
        lv_obj_center(hitLabel);

        lv_obj_t *standBtn = lv_obj_create(actionArea);
        lv_obj_set_size(standBtn, 85, 48);
        lv_obj_align(standBtn, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_bg_color(standBtn, lv_color_hex(0x8B0000), 0);  // Dark red
        lv_obj_set_style_radius(standBtn, 24, 0);
        lv_obj_set_style_border_width(standBtn, 2, 0);
        lv_obj_set_style_border_color(standBtn, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_shadow_width(standBtn, 6, 0);
        lv_obj_set_style_shadow_color(standBtn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(standBtn, LV_OPA_50, 0);
        disableAllScrolling(standBtn);

        lv_obj_t *standLabel = lv_label_create(standBtn);
        lv_label_set_text(standLabel, "STAND");
        lv_obj_set_style_text_color(standLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(standLabel, &lv_font_montserrat_14, 0);
        lv_obj_center(standLabel);

    } else {
        // Result state
        int dealerVal = calculateHandValue(dealerCards, dealerCount);
        const char* result;
        uint32_t resultColor;

        if (dealerVal > 21 || playerVal > dealerVal) {
            result = "YOU WIN!";
            resultColor = 0xFFD700;  // Gold for win
        } else if (playerVal < dealerVal) {
            result = "DEALER WINS";
            resultColor = 0xFF453A;  // Red for loss
        } else {
            result = "PUSH";
            resultColor = 0xFFFFFF;  // White for tie
        }

        lv_obj_t *resultLabel = lv_label_create(actionArea);
        lv_label_set_text(resultLabel, result);
        lv_obj_set_style_text_color(resultLabel, lv_color_hex(resultColor), 0);
        lv_obj_set_style_text_font(resultLabel, &lv_font_montserrat_24, 0);
        lv_obj_align(resultLabel, LV_ALIGN_TOP_MID, 0, 12);

        lv_obj_t *tapLabel = lv_label_create(actionArea);
        lv_label_set_text(tapLabel, "Tap for new game");
        lv_obj_set_style_text_color(tapLabel, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_text_opa(tapLabel, LV_OPA_60, 0);
        lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -8);
    }
}

// Blackjack game logic handlers
void startNewBlackjackGame() {
    playerCount = 0;
    dealerCount = 0;
    playerStand = false;
    blackjackGameActive = true;

    // Deal initial cards
    dealCard(playerCards, &playerCount, true);
    dealCard(dealerCards, &dealerCount, false);
    dealCard(playerCards, &playerCount, true);
    dealCard(dealerCards, &dealerCount, false);

    userData.gamesPlayed++;
}

void playerHit() {
    if (!blackjackGameActive || playerStand) return;

    dealCard(playerCards, &playerCount, true);

    int playerVal = calculateHandValue(playerCards, playerCount);
    if (playerVal > 21) {
        // Bust - game over
        blackjackGameActive = false;
    }
}

void playerStandAction() {
    if (!blackjackGameActive || playerStand) return;

    playerStand = true;

    // Dealer draws until 17+
    int dealerVal = calculateHandValue(dealerCards, dealerCount);
    while (dealerVal < 17) {
        dealCard(dealerCards, &dealerCount, false);
        dealerVal = calculateHandValue(dealerCards, dealerCount);
    }

    // Determine winner
    int playerVal = calculateHandValue(playerCards, playerCount);
    if (dealerVal > 21 || playerVal > dealerVal) {
        userData.gamesWon++;
        userData.blackjackStreak++;
    } else if (playerVal < dealerVal) {
        userData.blackjackStreak = 0;
    }

    blackjackGameActive = false;
}

// ═══════════════════════════════════════════════════════════════════════════
// ═══════════════════════════════════════════════════════════════════════════
// WEATHER CARD - BERLIN MINIMALIST STYLE (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void createWeatherCard() {
    disableAllScrolling(lv_scr_act());

    // Deep matte black background
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    disableAllScrolling(card);

    // Subtle top gradient for depth
    lv_obj_t *topGradient = lv_obj_create(card);
    lv_obj_set_size(topGradient, LCD_WIDTH, 150);
    lv_obj_align(topGradient, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(topGradient, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(topGradient, LV_OPA_30, 0);
    lv_obj_set_style_radius(topGradient, 0, 0);
    lv_obj_set_style_border_width(topGradient, 0, 0);
    disableAllScrolling(topGradient);

    // Large temperature with shadow
    lv_obj_t *tempLabel = lv_label_create(card);
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.0f°", weatherTemp);
    lv_label_set_text(tempLabel, tempStr);
    lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 32, 80);

    // City name in CAPS with letter spacing
    lv_obj_t *cityLabel = lv_label_create(card);
    char cityUpper[64];
    strncpy(cityUpper, weatherCity, sizeof(cityUpper));
    for (int i = 0; cityUpper[i]; i++) cityUpper[i] = toupper(cityUpper[i]);
    lv_label_set_text(cityLabel, cityUpper);
    lv_obj_set_style_text_color(cityLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(cityLabel, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_letter_space(cityLabel, 3, 0);
    lv_obj_align(cityLabel, LV_ALIGN_TOP_LEFT, 32, 145);

    // Accent line under city
    lv_obj_t *accentLine = lv_obj_create(card);
    lv_obj_set_size(accentLine, 60, 3);
    lv_obj_align(accentLine, LV_ALIGN_TOP_LEFT, 32, 175);
    lv_obj_set_style_bg_color(accentLine, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_radius(accentLine, 2, 0);
    lv_obj_set_style_border_width(accentLine, 0, 0);
    disableAllScrolling(accentLine);

    // High/Low container with glass effect
    lv_obj_t *rangeContainer = lv_obj_create(card);
    lv_obj_set_size(rangeContainer, 180, 50);
    lv_obj_align(rangeContainer, LV_ALIGN_TOP_LEFT, 32, 195);
    lv_obj_set_style_bg_color(rangeContainer, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(rangeContainer, LV_OPA_40, 0);
    lv_obj_set_style_radius(rangeContainer, 12, 0);
    lv_obj_set_style_border_width(rangeContainer, 0, 0);
    disableAllScrolling(rangeContainer);

    lv_obj_t *rangeLabel = lv_label_create(rangeContainer);
    char rangeStr[32];
    snprintf(rangeStr, sizeof(rangeStr), "H: %.0f°  L: %.0f°", weatherHigh, weatherLow);
    lv_label_set_text(rangeLabel, rangeStr);
    lv_obj_set_style_text_color(rangeLabel, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(rangeLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(rangeLabel);

    // Weather icon with glow effect
    lv_obj_t *iconContainer = lv_obj_create(card);
    lv_obj_set_size(iconContainer, 80, 80);
    lv_obj_align(iconContainer, LV_ALIGN_TOP_LEFT, 32, 270);
    lv_obj_set_style_bg_color(iconContainer, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_bg_opa(iconContainer, LV_OPA_20, 0);
    lv_obj_set_style_radius(iconContainer, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(iconContainer, 2, 0);
    lv_obj_set_style_border_color(iconContainer, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_border_opa(iconContainer, LV_OPA_50, 0);
    disableAllScrolling(iconContainer);

    lv_obj_t *icon = lv_label_create(iconContainer);
    // Get proper weather icon based on condition
    uint32_t iconColor = getWeatherIconColor(weatherDesc.c_str());
    lv_obj_set_style_bg_color(iconContainer, lv_color_hex(iconColor), 0);
    lv_obj_set_style_bg_opa(iconContainer, LV_OPA_20, 0);
    lv_obj_set_style_border_color(iconContainer, lv_color_hex(iconColor), 0);
    
    // Weather icon symbols mapped to conditions
    const char* iconSymbol;
    if (weatherDesc.indexOf("Clear") >= 0 || weatherDesc.indexOf("Sunny") >= 0) {
        iconSymbol = LV_SYMBOL_IMAGE;  // Sun icon
    } else if (weatherDesc.indexOf("Cloud") >= 0 || weatherDesc.indexOf("Overcast") >= 0) {
        iconSymbol = LV_SYMBOL_WIFI;  // Cloud icon
    } else if (weatherDesc.indexOf("Rain") >= 0 || weatherDesc.indexOf("Drizzle") >= 0) {
        iconSymbol = LV_SYMBOL_REFRESH;  // Rain icon
    } else if (weatherDesc.indexOf("Snow") >= 0 || weatherDesc.indexOf("Sleet") >= 0) {
        iconSymbol = LV_SYMBOL_OK;  // Snow icon
    } else if (weatherDesc.indexOf("Thunder") >= 0 || weatherDesc.indexOf("Storm") >= 0) {
        iconSymbol = LV_SYMBOL_CHARGE;  // Storm icon
    } else if (weatherDesc.indexOf("Mist") >= 0 || weatherDesc.indexOf("Fog") >= 0 || weatherDesc.indexOf("Haze") >= 0) {
        iconSymbol = LV_SYMBOL_LIST;  // Fog icon
    } else {
        iconSymbol = LV_SYMBOL_EYE_OPEN;  // Default weather
    }
    lv_label_set_text(icon, iconSymbol);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_32, 0);
    lv_obj_center(icon);

    // Weather description
    lv_obj_t *descLabel = lv_label_create(card);
    lv_label_set_text(descLabel, weatherDesc.c_str());
    lv_obj_set_style_text_color(descLabel, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(descLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(descLabel, LV_ALIGN_TOP_LEFT, 130, 295);

    // Bottom hint bar
    lv_obj_t *bottomBar = lv_obj_create(card);
    lv_obj_set_size(bottomBar, LCD_WIDTH, 60);
    lv_obj_align(bottomBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottomBar, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(bottomBar, LV_OPA_30, 0);
    lv_obj_set_style_radius(bottomBar, 0, 0);
    lv_obj_set_style_border_width(bottomBar, 0, 0);
    disableAllScrolling(bottomBar);

    lv_obj_t *hint = lv_label_create(bottomBar);
    lv_label_set_text(hint, LV_SYMBOL_UP " Swipe for more");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_center(hint);
}

// ═══════════════════════════════════════════════════════════════════════════
// FORECAST CARD
// ═══════════════════════════════════════════════════════════════════════════
void createForecastCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "5-DAY FORECAST");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    // City name
    lv_obj_t *cityLabel = lv_label_create(card);
    char cityUpper[64];
    strncpy(cityUpper, weatherCity, sizeof(cityUpper));
    for (int i = 0; cityUpper[i]; i++) cityUpper[i] = toupper(cityUpper[i]);
    lv_label_set_text(cityLabel, cityUpper);
    lv_obj_set_style_text_color(cityLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(cityLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(cityLabel, LV_ALIGN_TOP_MID, 0, 32);

    // 5-day forecast rows
    int startY = 58;
    int rowHeight = 58;

    for (int i = 0; i < 5; i++) {
        // Day row container
        lv_obj_t *dayRow = lv_obj_create(card);
        lv_obj_set_size(dayRow, LCD_WIDTH - 30, 50);
        lv_obj_align(dayRow, LV_ALIGN_TOP_MID, 0, startY + i * rowHeight);
        lv_obj_set_style_bg_color(dayRow, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(dayRow, 12, 0);
        lv_obj_set_style_border_width(dayRow, 0, 0);
        disableAllScrolling(dayRow);

        // Day name
        lv_obj_t *dayLabel = lv_label_create(dayRow);
        lv_label_set_text(dayLabel, forecast5Day[i].dayName);
        lv_obj_set_style_text_color(dayLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(dayLabel, &lv_font_montserrat_14, 0);
        lv_obj_align(dayLabel, LV_ALIGN_LEFT_MID, 15, 0);

        // Weather icon
        lv_obj_t *iconLabel = lv_label_create(dayRow);
        const char* iconSymbol;
        uint32_t iconColor;
        
        // Map icon string to proper LVGL symbols and colors
        if (strcmp(forecast5Day[i].icon, "sun") == 0 || strcmp(forecast5Day[i].icon, "clear") == 0) {
            iconSymbol = LV_SYMBOL_IMAGE;  // Sun
            iconColor = 0xFFD60A;  // Yellow/Gold
        } else if (strcmp(forecast5Day[i].icon, "cloud") == 0 || strcmp(forecast5Day[i].icon, "overcast") == 0) {
            iconSymbol = LV_SYMBOL_WIFI;  // Cloud
            iconColor = 0xAEAEB2;  // Gray
        } else if (strcmp(forecast5Day[i].icon, "rain") == 0 || strcmp(forecast5Day[i].icon, "drizzle") == 0) {
            iconSymbol = LV_SYMBOL_REFRESH;  // Rain drops
            iconColor = 0x0A84FF;  // Blue
        } else if (strcmp(forecast5Day[i].icon, "snow") == 0 || strcmp(forecast5Day[i].icon, "sleet") == 0) {
            iconSymbol = LV_SYMBOL_OK;  // Snowflake
            iconColor = 0xE0F7FA;  // Light cyan
        } else if (strcmp(forecast5Day[i].icon, "storm") == 0 || strcmp(forecast5Day[i].icon, "thunder") == 0) {
            iconSymbol = LV_SYMBOL_CHARGE;  // Lightning bolt
            iconColor = 0xFFD60A;  // Yellow
        } else if (strcmp(forecast5Day[i].icon, "mist") == 0 || strcmp(forecast5Day[i].icon, "fog") == 0) {
            iconSymbol = LV_SYMBOL_LIST;  // Horizontal lines (fog)
            iconColor = 0x9E9E9E;  // Light gray
        } else if (strcmp(forecast5Day[i].icon, "wind") == 0) {
            iconSymbol = LV_SYMBOL_NEXT;  // Wind arrow
            iconColor = 0xB3E5FC;  // Light blue
        } else {
            iconSymbol = LV_SYMBOL_EYE_OPEN;  // Default - partial cloud
            iconColor = 0x8E8E93;  // Gray
        }
        
        lv_label_set_text(iconLabel, iconSymbol);
        lv_obj_set_style_text_color(iconLabel, lv_color_hex(iconColor), 0);
        lv_obj_set_style_text_font(iconLabel, &lv_font_montserrat_18, 0);
        lv_obj_align(iconLabel, LV_ALIGN_CENTER, -15, 0);

        // High temp
        char highBuf[8];
        snprintf(highBuf, sizeof(highBuf), "%.0f°", forecast5Day[i].tempHigh);
        lv_obj_t *highLabel = lv_label_create(dayRow);
        lv_label_set_text(highLabel, highBuf);
        lv_obj_set_style_text_color(highLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(highLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(highLabel, LV_ALIGN_RIGHT_MID, -55, 0);

        // Low temp
        char lowBuf[8];
        snprintf(lowBuf, sizeof(lowBuf), "%.0f°", forecast5Day[i].tempLow);
        lv_obj_t *lowLabel = lv_label_create(dayRow);
        lv_label_set_text(lowLabel, lowBuf);
        lv_obj_set_style_text_color(lowLabel, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(lowLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(lowLabel, LV_ALIGN_RIGHT_MID, -12, 0);
    }

    // Refresh hint
    lv_obj_t *hint = lv_label_create(card);
    lv_label_set_text(hint, forecastLoaded ? (weatherDataLoaded ? "Cached data" : "Updated from API") : "Sample data");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -8);
}

// ═══════════════════════════════════════════════════════════════════════════
// STOPWATCH CARD - PREMIUM DESIGN WITH LAP TIMES
// ═══════════════════════════════════════════════════════════════════════════
void createStopwatchCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "STOPWATCH");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Calculate elapsed time
    unsigned long elapsed = stopwatchElapsedMs;
    if (stopwatchRunning) {
        elapsed += (millis() - stopwatchStartMs);
    }

    int mins = elapsed / 60000;
    int secs = (elapsed / 1000) % 60;
    int ms = (elapsed % 1000) / 10;

    // Main time display container
    lv_obj_t *timeCard = lv_obj_create(card);
    lv_obj_set_size(timeCard, LCD_WIDTH - 40, 100);
    lv_obj_align(timeCard, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(timeCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(timeCard, 20, 0);
    lv_obj_set_style_border_width(timeCard, 0, 0);
    disableAllScrolling(timeCard);

    // Large time display
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", mins, secs);
    lv_obj_t *timeLabel = lv_label_create(timeCard);
    lv_label_set_text(timeLabel, timeBuf);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, -15, 0);

    // Milliseconds
    char msBuf[8];
    snprintf(msBuf, sizeof(msBuf), ".%02d", ms);
    lv_obj_t *msLabel = lv_label_create(timeCard);
    lv_label_set_text(msLabel, msBuf);
    lv_obj_set_style_text_color(msLabel, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(msLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(msLabel, LV_ALIGN_CENTER, 70, 10);

    // Status indicator
    lv_obj_t *statusDot = lv_obj_create(card);
    lv_obj_set_size(statusDot, 12, 12);
    lv_obj_align(statusDot, LV_ALIGN_TOP_MID, 0, 155);
    lv_obj_set_style_bg_color(statusDot, stopwatchRunning ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_radius(statusDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(statusDot, 0, 0);
    disableAllScrolling(statusDot);

    // Status text
    lv_obj_t *statusLabel = lv_label_create(card);
    lv_label_set_text(statusLabel, stopwatchRunning ? "RUNNING" : (elapsed > 0 ? "PAUSED" : "READY"));
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 15, 153);

    // Lap times panel (if any laps recorded)
    if (lapCount > 0) {
        lv_obj_t *lapCard = lv_obj_create(card);
        lv_obj_set_size(lapCard, LCD_WIDTH - 40, 100);
        lv_obj_align(lapCard, LV_ALIGN_CENTER, 0, 50);
        lv_obj_set_style_bg_color(lapCard, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(lapCard, 15, 0);
        lv_obj_set_style_border_width(lapCard, 0, 0);
        disableAllScrolling(lapCard);

        lv_obj_t *lapTitle = lv_label_create(lapCard);
        lv_label_set_text(lapTitle, "LAP TIMES");
        lv_obj_set_style_text_color(lapTitle, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(lapTitle, &lv_font_montserrat_10, 0);
        lv_obj_align(lapTitle, LV_ALIGN_TOP_MID, 0, 8);

        // Show last 3 laps
        int startLap = max(0, lapCount - 3);
        for (int i = startLap; i < lapCount && i < startLap + 3; i++) {
            int lapMins = lapTimes[i] / 60000;
            int lapSecs = (lapTimes[i] / 1000) % 60;
            int lapMs = (lapTimes[i] % 1000) / 10;

            char lapBuf[32];
            snprintf(lapBuf, sizeof(lapBuf), "Lap %d: %02d:%02d.%02d", i + 1, lapMins, lapSecs, lapMs);
            lv_obj_t *lapLabel = lv_label_create(lapCard);
            lv_label_set_text(lapLabel, lapBuf);
            lv_obj_set_style_text_color(lapLabel, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(lapLabel, &lv_font_montserrat_12, 0);
            lv_obj_align(lapLabel, LV_ALIGN_TOP_LEFT, 15, 25 + (i - startLap) * 22);
        }
    }

    // Control buttons row
    lv_obj_t *btnRow = lv_obj_create(card);
    lv_obj_set_size(btnRow, LCD_WIDTH - 40, 55);
    lv_obj_align(btnRow, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(btnRow, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(btnRow, 27, 0);
    lv_obj_set_style_border_width(btnRow, 0, 0);
    disableAllScrolling(btnRow);

    // Start/Stop indicator
    lv_obj_t *actionLabel = lv_label_create(btnRow);
    lv_label_set_text(actionLabel, stopwatchRunning ? "TAP TO STOP" : "TAP TO START");
    lv_obj_set_style_text_color(actionLabel, stopwatchRunning ? lv_color_hex(0xFF453A) : lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(actionLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(actionLabel);

    // Reset hint
    lv_obj_t *resetHint = lv_label_create(card);
    lv_label_set_text(resetHint, "DOUBLE TAP TO RESET");
    lv_obj_set_style_text_color(resetHint, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(resetHint, &lv_font_montserrat_10, 0);
    lv_obj_align(resetHint, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// ═══════════════════════════════════════════════════════════════════════════
// TORCH CARD - PREMIUM DESIGN
// ═══════════════════════════════════════════════════════════════════════════
void createTorchCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);

    // Background changes based on torch state
    if (torchOn) {
        // Bright background matching torch color
        lv_obj_set_style_bg_color(card, lv_color_hex(torchColors[torchColorIndex]), 0);
    } else {
        // Dark background when off
        lv_obj_set_style_bg_color(card, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_bg_grad_color(card, lv_color_hex(0x2C2C2E), 0);
        lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
    }

    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "TORCH");
    lv_obj_set_style_text_color(title, torchOn ? lv_color_hex(0x1C1C1E) : lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Large power button icon
    lv_obj_t *powerBtn = lv_obj_create(card);
    lv_obj_set_size(powerBtn, 150, 150);
    lv_obj_center(powerBtn);
    lv_obj_set_style_radius(powerBtn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(powerBtn, 4, 0);
    disableAllScrolling(powerBtn);

    if (torchOn) {
        lv_obj_set_style_bg_color(powerBtn, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_bg_opa(powerBtn, LV_OPA_30, 0);
        lv_obj_set_style_border_color(powerBtn, lv_color_hex(0x1C1C1E), 0);
    } else {
        lv_obj_set_style_bg_color(powerBtn, lv_color_hex(0x2C2C2E), 0);
        lv_obj_set_style_border_color(powerBtn, lv_color_hex(0x0A84FF), 0);
    }

    // Power icon
    lv_obj_t *powerIcon = lv_label_create(powerBtn);
    lv_label_set_text(powerIcon, LV_SYMBOL_POWER);
    lv_obj_set_style_text_color(powerIcon, torchOn ? lv_color_hex(0x1C1C1E) : lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(powerIcon, &lv_font_montserrat_48, 0);
    lv_obj_center(powerIcon);

    // Status text
    lv_obj_t *statusLabel = lv_label_create(card);
    lv_label_set_text(statusLabel, torchOn ? "ON" : "OFF");
    lv_obj_set_style_text_color(statusLabel, torchOn ? lv_color_hex(0x1C1C1E) : lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_32, 0);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -100);

    // Color indicator (when on)
    if (torchOn) {
        lv_obj_t *colorLabel = lv_label_create(card);
        lv_label_set_text(colorLabel, torchColorNames[torchColorIndex]);
        lv_obj_set_style_text_color(colorLabel, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_text_font(colorLabel, &lv_font_montserrat_16, 0);
        lv_obj_align(colorLabel, LV_ALIGN_BOTTOM_MID, 0, -70);
    }

    // Hint
    lv_obj_t *hintLabel = lv_label_create(card);
    lv_label_set_text(hintLabel, "TAP TO TOGGLE");
    lv_obj_set_style_text_color(hintLabel, torchOn ? lv_color_hex(0x1C1C1E) : lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_t *hint2Label = lv_label_create(card);
    lv_label_set_text(hint2Label, "SWIPE DOWN FOR SETTINGS");
    lv_obj_set_style_text_color(hint2Label, torchOn ? lv_color_hex(0x1C1C1E) : lv_color_hex(0x5E5E5E), 0);
    lv_obj_set_style_text_font(hint2Label, &lv_font_montserrat_10, 0);
    lv_obj_align(hint2Label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// ═══════════════════════════════════════════════════════════════════════════
// TORCH SETTINGS CARD - PREMIUM DESIGN
// ═══════════════════════════════════════════════════════════════════════════
void createTorchSettingsCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, theme->color1, 0);
    lv_obj_set_style_bg_grad_color(card, theme->color2, 0);
    lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "TORCH SETTINGS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Color selection
    lv_obj_t *colorTitle = lv_label_create(card);
    lv_label_set_text(colorTitle, "COLOR");
    lv_obj_set_style_text_color(colorTitle, theme->text, 0);
    lv_obj_align(colorTitle, LV_ALIGN_TOP_MID, 0, 50);

    // Color swatches
    int swatchSize = 50;
    int startX = (LCD_WIDTH - (NUM_TORCH_COLORS * swatchSize + (NUM_TORCH_COLORS - 1) * 10)) / 2;

    for (int i = 0; i < NUM_TORCH_COLORS; i++) {
        lv_obj_t *swatch = lv_obj_create(card);
        lv_obj_set_size(swatch, swatchSize, swatchSize);
        lv_obj_set_pos(swatch, startX + i * (swatchSize + 10), 80);
        lv_obj_set_style_bg_color(swatch, lv_color_hex(torchColors[i]), 0);
        lv_obj_set_style_radius(swatch, LV_RADIUS_CIRCLE, 0);

        // Highlight selected
        if (i == torchColorIndex) {
            lv_obj_set_style_border_width(swatch, 3, 0);
            lv_obj_set_style_border_color(swatch, theme->accent, 0);
        } else {
            lv_obj_set_style_border_width(swatch, 1, 0);
            lv_obj_set_style_border_color(swatch, lv_color_hex(0x3A3A3C), 0);
        }

        disableAllScrolling(swatch);
    }

    // Selected color name
    lv_obj_t *colorName = lv_label_create(card);
    lv_label_set_text(colorName, torchColorNames[torchColorIndex]);
    lv_obj_set_style_text_color(colorName, theme->accent, 0);
    lv_obj_set_style_text_font(colorName, &lv_font_montserrat_16, 0);
    lv_obj_align(colorName, LV_ALIGN_TOP_MID, 0, 145);

    // Brightness section
    lv_obj_t *brightTitle = lv_label_create(card);
    lv_label_set_text(brightTitle, "BRIGHTNESS");
    lv_obj_set_style_text_color(brightTitle, theme->text, 0);
    lv_obj_align(brightTitle, LV_ALIGN_TOP_MID, 0, 190);

    // Brightness bar
    lv_obj_t *brightBar = lv_bar_create(card);
    lv_obj_set_size(brightBar, LCD_WIDTH - 60, 20);
    lv_obj_align(brightBar, LV_ALIGN_TOP_MID, 0, 220);
    lv_bar_set_range(brightBar, 50, 255);
    lv_bar_set_value(brightBar, torchBrightness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(brightBar, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(brightBar, theme->accent, LV_PART_INDICATOR);
    lv_obj_set_style_radius(brightBar, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(brightBar, 10, LV_PART_INDICATOR);

    // Brightness value
    char brightBuf[16];
    int brightPercent = ((torchBrightness - 50) * 100) / 205;
    snprintf(brightBuf, sizeof(brightBuf), "%d%%", brightPercent);
    lv_obj_t *brightVal = lv_label_create(card);
    lv_label_set_text(brightVal, brightBuf);
    lv_obj_set_style_text_color(brightVal, theme->secondary, 0);
    lv_obj_align(brightVal, LV_ALIGN_TOP_MID, 0, 250);

    // Tap hint
    lv_obj_t *hintLabel = lv_label_create(card);
    lv_label_set_text(hintLabel, "TAP COLORS TO SELECT");
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ═══════════════════════════════════════════════════════════════════════════
// CALCULATOR CARD
// ═══════════════════════════════════════════════════════════════════════════
void createCalculatorCard() {
    disableAllScrolling(lv_scr_act());

    // ═══ PREMIUM APPLE-STYLE CALCULATOR ═══
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Display area - premium glass effect
    lv_obj_t *displayBg = lv_obj_create(card);
    lv_obj_set_size(displayBg, LCD_WIDTH - 20, 90);
    lv_obj_align(displayBg, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(displayBg, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(displayBg, 20, 0);
    lv_obj_set_style_border_width(displayBg, 1, 0);
    lv_obj_set_style_border_color(displayBg, lv_color_hex(0x3A3A3C), 0);
    disableAllScrolling(displayBg);

    // Display number - right aligned, large
    lv_obj_t *displayLabel = lv_label_create(displayBg);
    lv_label_set_text(displayLabel, calcDisplay);
    lv_obj_set_style_text_color(displayLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(displayLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(displayLabel, LV_ALIGN_RIGHT_MID, -15, 0);

    // Operation indicator
    if (calcOperator != ' ') {
        char opBuf[4];
        snprintf(opBuf, sizeof(opBuf), "%c", calcOperator);
        lv_obj_t *opLabel = lv_label_create(displayBg);
        lv_label_set_text(opLabel, opBuf);
        lv_obj_set_style_text_color(opLabel, lv_color_hex(0xFF9F0A), 0);
        lv_obj_set_style_text_font(opLabel, &lv_font_montserrat_24, 0);
        lv_obj_align(opLabel, LV_ALIGN_LEFT_MID, 15, 0);
    }

    // Button grid - 4x5 layout
    int btnSize = 48;
    int btnSpacing = 4;
    int startY = 110;
    int startX = 15;

    // Button definitions: text, color, row, col
    struct CalcBtn {
        const char* text;
        uint32_t bgColor;
        uint32_t textColor;
    };
    
    CalcBtn buttons[5][4] = {
        {{"AC", 0xA5A5A5, 0x000000}, {"±", 0xA5A5A5, 0x000000}, {"%", 0xA5A5A5, 0x000000}, {"÷", 0xFF9F0A, 0xFFFFFF}},
        {{"7", 0x333333, 0xFFFFFF}, {"8", 0x333333, 0xFFFFFF}, {"9", 0x333333, 0xFFFFFF}, {"×", 0xFF9F0A, 0xFFFFFF}},
        {{"4", 0x333333, 0xFFFFFF}, {"5", 0x333333, 0xFFFFFF}, {"6", 0x333333, 0xFFFFFF}, {"-", 0xFF9F0A, 0xFFFFFF}},
        {{"1", 0x333333, 0xFFFFFF}, {"2", 0x333333, 0xFFFFFF}, {"3", 0x333333, 0xFFFFFF}, {"+", 0xFF9F0A, 0xFFFFFF}},
        {{"0", 0x333333, 0xFFFFFF}, {".", 0x333333, 0xFFFFFF}, {"⌫", 0x333333, 0xFFFFFF}, {"=", 0x30D158, 0xFFFFFF}}
    };

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            lv_obj_t *btn = lv_obj_create(card);
            lv_obj_set_size(btn, btnSize, btnSize);
            lv_obj_set_pos(btn, startX + col * (btnSize + btnSpacing), startY + row * (btnSize + btnSpacing));
            lv_obj_set_style_bg_color(btn, lv_color_hex(buttons[row][col].bgColor), 0);
            lv_obj_set_style_radius(btn, btnSize/2, 0);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_shadow_width(btn, 8, 0);
            lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), 0);
            lv_obj_set_style_shadow_opa(btn, LV_OPA_30, 0);
            disableAllScrolling(btn);

            lv_obj_t *btnLabel = lv_label_create(btn);
            lv_label_set_text(btnLabel, buttons[row][col].text);
            lv_obj_set_style_text_color(btnLabel, lv_color_hex(buttons[row][col].textColor), 0);
            lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_20, 0);
            lv_obj_center(btnLabel);
        }
    }

    // Hint at bottom
    lv_obj_t *hintLabel = lv_label_create(card);
    lv_label_set_text(hintLabel, "TAP BUTTONS TO CALCULATE");
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x48484A), 0);
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// ═══════════════════════════════════════════════════════════════════════════
// TALLY COUNTER CARD
// ═══════════════════════════════════════════════════════════════════════════
void createTallyCounterCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "TALLY COUNTER");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Large count display
    char countBuf[16];
    snprintf(countBuf, sizeof(countBuf), "%d", tallyCount);
    lv_obj_t *countLabel = lv_label_create(card);
    lv_label_set_text(countLabel, countBuf);
    lv_obj_set_style_text_color(countLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(countLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(countLabel, LV_ALIGN_CENTER, 0, -30);

    // Tap zones explanation
    lv_obj_t *plusZone = lv_obj_create(card);
    lv_obj_set_size(plusZone, LCD_WIDTH - 40, 80);
    lv_obj_align(plusZone, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(plusZone, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_bg_opa(plusZone, LV_OPA_30, 0);
    lv_obj_set_style_radius(plusZone, 15, 0);
    lv_obj_set_style_border_width(plusZone, 0, 0);
    disableAllScrolling(plusZone);

    lv_obj_t *plusLabel = lv_label_create(plusZone);
    lv_label_set_text(plusLabel, LV_SYMBOL_PLUS " TAP TO ADD");
    lv_obj_set_style_text_color(plusLabel, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(plusLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(plusLabel);

    // Bottom buttons
    lv_obj_t *minusBtn = lv_obj_create(card);
    lv_obj_set_size(minusBtn, (LCD_WIDTH - 50) / 2, 60);
    lv_obj_align(minusBtn, LV_ALIGN_BOTTOM_LEFT, 15, -20);
    lv_obj_set_style_bg_color(minusBtn, lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_radius(minusBtn, 12, 0);
    lv_obj_set_style_border_width(minusBtn, 0, 0);
    disableAllScrolling(minusBtn);

    lv_obj_t *minusLabel = lv_label_create(minusBtn);
    lv_label_set_text(minusLabel, LV_SYMBOL_MINUS " 1");
    lv_obj_set_style_text_color(minusLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(minusLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(minusLabel);

    lv_obj_t *resetBtn = lv_obj_create(card);
    lv_obj_set_size(resetBtn, (LCD_WIDTH - 50) / 2, 60);
    lv_obj_align(resetBtn, LV_ALIGN_BOTTOM_RIGHT, -15, -20);
    lv_obj_set_style_bg_color(resetBtn, lv_color_hex(0x636366), 0);
    lv_obj_set_style_radius(resetBtn, 12, 0);
    lv_obj_set_style_border_width(resetBtn, 0, 0);
    disableAllScrolling(resetBtn);

    lv_obj_t *resetLabel = lv_label_create(resetBtn);
    lv_label_set_text(resetLabel, "RESET");
    lv_obj_set_style_text_color(resetLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(resetLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(resetLabel);
}

// ═══════════════════════════════════════════════════════════════════════════
// VOICE MEMO CARD - ES8311 Recording
// ═══════════════════════════════════════════════════════════════════════════

// Initialize ES8311 I2S for recording
bool initVoiceI2S() {
    if (voiceI2SInitialized) return true;
    
    // Enable PA
    pinMode(ES8311_PA_PIN, OUTPUT);
    digitalWrite(ES8311_PA_PIN, HIGH);
    
    // Initialize I2S
    voiceI2S.setPins(I2S_BCK_PIN, I2S_WS_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCLK_PIN);
    if (!voiceI2S.begin(I2S_MODE_STD, VOICE_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
        USBSerial.println("[VOICE] Failed to init I2S!");
        return false;
    }
    
    // Initialize ES8311 codec
    es8311_handle_t es_handle = es8311_create(0, ES8311_ADDRESS_0);
    if (es_handle) {
        es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = VOICE_SAMPLE_RATE * 256,
            .sample_frequency = VOICE_SAMPLE_RATE
        };
        
        es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
        es8311_microphone_config(es_handle, false);
        es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN_SETTING);
        es8311_voice_volume_set(es_handle, 85, NULL);
        USBSerial.println("[VOICE] ES8311 codec initialized");
    } else {
        USBSerial.println("[VOICE] ES8311 not found, using raw I2S");
    }
    
    voiceI2SInitialized = true;
    USBSerial.println("[VOICE] I2S initialized for recording");
    return true;
}

// Start recording voice memo to SD card
void startVoiceRecording() {
    if (!initVoiceI2S()) return;
    
    // Create memo folder if needed
    if (!SD_MMC.exists(VOICE_MEMO_FOLDER)) {
        SD_MMC.mkdir(VOICE_MEMO_FOLDER);
        USBSerial.println("[VOICE] Created memo folder");
    }
    
    // Generate filename with timestamp
    char filename[64];
    snprintf(filename, sizeof(filename), "%s/memo_%lu.raw", VOICE_MEMO_FOLDER, millis());
    
    voiceMemoFile = SD_MMC.open(filename, FILE_WRITE);
    if (!voiceMemoFile) {
        USBSerial.println("[VOICE] Failed to create file!");
        return;
    }
    
    voiceMemoRecording = true;
    voiceRecordStart = millis();
    voiceRecordDuration = 0;
    USBSerial.printf("[VOICE] Recording to: %s\n", filename);
}

// Stop and save recording
void stopVoiceRecording() {
    if (!voiceMemoRecording) return;
    
    voiceMemoRecording = false;
    voiceRecordDuration = (millis() - voiceRecordStart) / 1000;
    
    if (voiceMemoFile) {
        voiceMemoFile.close();
    }
    
    voiceMemoCount++;
    USBSerial.printf("[VOICE] Recording saved (%ds)\n", voiceRecordDuration);
}

// Record audio samples (call in loop when recording)
void recordVoiceSamples() {
    if (!voiceMemoRecording || !voiceMemoFile) return;
    
    static uint8_t audioBuffer[VOICE_BUF_SIZE];
    size_t bytesRead = voiceI2S.readBytes((char*)audioBuffer, VOICE_BUF_SIZE);
    
    if (bytesRead > 0) {
        voiceMemoFile.write(audioBuffer, bytesRead);
    }
}

// Count existing memos on SD
void countVoiceMemos() {
    if (!SD_MMC.exists(VOICE_MEMO_FOLDER)) {
        voiceMemoCount = 0;
        return;
    }
    
    File dir = SD_MMC.open(VOICE_MEMO_FOLDER);
    voiceMemoCount = 0;
    while (File entry = dir.openNextFile()) {
        if (!entry.isDirectory()) {
            voiceMemoCount++;
        }
        entry.close();
    }
    dir.close();
}

void createVoiceMemoCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_AUDIO " VOICE MEMO");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Recording indicator
    if (voiceMemoRecording) {
        // Recording animation - pulsing red circle
        lv_obj_t *recCircle = lv_obj_create(card);
        lv_obj_set_size(recCircle, 80, 80);
        lv_obj_align(recCircle, LV_ALIGN_CENTER, 0, -30);
        lv_obj_set_style_bg_color(recCircle, lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_radius(recCircle, 40, 0);
        lv_obj_set_style_border_width(recCircle, 0, 0);
        disableAllScrolling(recCircle);

        // Duration
        int elapsed = (millis() - voiceRecordStart) / 1000;
        char durBuf[16];
        snprintf(durBuf, sizeof(durBuf), "%02d:%02d", elapsed / 60, elapsed % 60);
        lv_obj_t *durLabel = lv_label_create(card);
        lv_label_set_text(durLabel, durBuf);
        lv_obj_set_style_text_color(durLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(durLabel, &lv_font_montserrat_36, 0);
        lv_obj_align(durLabel, LV_ALIGN_CENTER, 0, 40);

        // Stop button
        lv_obj_t *stopBtn = lv_obj_create(card);
        lv_obj_set_size(stopBtn, LCD_WIDTH - 60, 50);
        lv_obj_align(stopBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(stopBtn, lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_radius(stopBtn, 25, 0);
        lv_obj_set_style_border_width(stopBtn, 0, 0);
        disableAllScrolling(stopBtn);

        lv_obj_t *stopLabel = lv_label_create(stopBtn);
        lv_label_set_text(stopLabel, LV_SYMBOL_STOP " STOP & SAVE");
        lv_obj_set_style_text_color(stopLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(stopLabel, &lv_font_montserrat_14, 0);
        lv_obj_center(stopLabel);
    } else {
        // Not recording - show record button
        
        // Mic icon
        lv_obj_t *micCircle = lv_obj_create(card);
        lv_obj_set_size(micCircle, 100, 100);
        lv_obj_align(micCircle, LV_ALIGN_CENTER, 0, -30);
        lv_obj_set_style_bg_color(micCircle, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(micCircle, 50, 0);
        lv_obj_set_style_border_width(micCircle, 3, 0);
        lv_obj_set_style_border_color(micCircle, lv_color_hex(0xFF453A), 0);
        disableAllScrolling(micCircle);

        lv_obj_t *micIcon = lv_label_create(micCircle);
        lv_label_set_text(micIcon, LV_SYMBOL_AUDIO);
        lv_obj_set_style_text_color(micIcon, lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_text_font(micIcon, &lv_font_montserrat_36, 0);
        lv_obj_center(micIcon);

        // Memo count
        char memoBuf[32];
        snprintf(memoBuf, sizeof(memoBuf), "%d memo%s saved", voiceMemoCount, voiceMemoCount == 1 ? "" : "s");
        lv_obj_t *memoLabel = lv_label_create(card);
        lv_label_set_text(memoLabel, memoBuf);
        lv_obj_set_style_text_color(memoLabel, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(memoLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(memoLabel, LV_ALIGN_CENTER, 0, 50);

        // Record button
        lv_obj_t *recBtn = lv_obj_create(card);
        lv_obj_set_size(recBtn, LCD_WIDTH - 60, 50);
        lv_obj_align(recBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(recBtn, lv_color_hex(0x30D158), 0);
        lv_obj_set_style_radius(recBtn, 25, 0);
        lv_obj_set_style_border_width(recBtn, 0, 0);
        disableAllScrolling(recBtn);

        lv_obj_t *recLabel = lv_label_create(recBtn);
        lv_label_set_text(recLabel, LV_SYMBOL_PLAY " TAP TO RECORD");
        lv_obj_set_style_text_color(recLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(recLabel, &lv_font_montserrat_14, 0);
        lv_obj_center(recLabel);
    }

    // SD folder path
    lv_obj_t *pathLabel = lv_label_create(card);
    lv_label_set_text(pathLabel, VOICE_MEMO_FOLDER);
    lv_obj_set_style_text_color(pathLabel, lv_color_hex(0x48484A), 0);
    lv_obj_set_style_text_font(pathLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(pathLabel, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// ═══════════════════════════════════════════════════════════════════════════
// DICE ROLLER CARD
// ═══════════════════════════════════════════════════════════════════════════
void createDiceRollerCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_REFRESH " DICE ROLLER");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Dice display boxes
    int diceSize = 80;
    int spacing = 20;

    // Dice 1
    lv_obj_t *dice1 = lv_obj_create(card);
    lv_obj_set_size(dice1, diceSize, diceSize);
    lv_obj_align(dice1, LV_ALIGN_CENTER, -(diceSize/2 + spacing/2), -20);
    lv_obj_set_style_bg_color(dice1, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(dice1, 15, 0);
    lv_obj_set_style_border_width(dice1, 0, 0);
    disableAllScrolling(dice1);

    char d1Buf[4];
    snprintf(d1Buf, sizeof(d1Buf), "%d", diceValue1);
    lv_obj_t *d1Label = lv_label_create(dice1);
    lv_label_set_text(d1Label, d1Buf);
    lv_obj_set_style_text_color(d1Label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(d1Label, &lv_font_montserrat_48, 0);
    lv_obj_center(d1Label);

    // Dice 2
    lv_obj_t *dice2 = lv_obj_create(card);
    lv_obj_set_size(dice2, diceSize, diceSize);
    lv_obj_align(dice2, LV_ALIGN_CENTER, (diceSize/2 + spacing/2), -20);
    lv_obj_set_style_bg_color(dice2, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(dice2, 15, 0);
    lv_obj_set_style_border_width(dice2, 0, 0);
    disableAllScrolling(dice2);

    char d2Buf[4];
    snprintf(d2Buf, sizeof(d2Buf), "%d", diceValue2);
    lv_obj_t *d2Label = lv_label_create(dice2);
    lv_label_set_text(d2Label, d2Buf);
    lv_obj_set_style_text_color(d2Label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(d2Label, &lv_font_montserrat_48, 0);
    lv_obj_center(d2Label);

    // Total
    char totalBuf[16];
    snprintf(totalBuf, sizeof(totalBuf), "Total: %d", diceValue1 + diceValue2);
    lv_obj_t *totalLabel = lv_label_create(card);
    lv_label_set_text(totalLabel, totalBuf);
    lv_obj_set_style_text_color(totalLabel, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(totalLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(totalLabel, LV_ALIGN_CENTER, 0, 70);

    // Hint
    lv_obj_t *hintLabel = lv_label_create(card);
    lv_label_set_text(hintLabel, "TAP OR SHAKE TO ROLL");
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAGIC 8 BALL CARD
// ═══════════════════════════════════════════════════════════════════════════
void createMagic8BallCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "MAGIC 8 BALL");
    lv_obj_set_style_text_color(title, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_12, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // 8 Ball circle
    lv_obj_t *ball = lv_obj_create(card);
    lv_obj_set_size(ball, 180, 180);
    lv_obj_align(ball, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_bg_color(ball, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(ball, 90, 0);
    lv_obj_set_style_border_width(ball, 4, 0);
    lv_obj_set_style_border_color(ball, lv_color_hex(0x3A3A3C), 0);
    disableAllScrolling(ball);

    // Inner triangle/window
    lv_obj_t *window = lv_obj_create(ball);
    lv_obj_set_size(window, 100, 100);
    lv_obj_center(window);
    lv_obj_set_style_bg_color(window, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_radius(window, 50, 0);
    lv_obj_set_style_border_width(window, 0, 0);
    disableAllScrolling(window);

    // Answer text
    lv_obj_t *answerLabel = lv_label_create(window);
    lv_label_set_text(answerLabel, magic8BallAnswer.c_str());
    lv_obj_set_style_text_color(answerLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(answerLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(answerLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(answerLabel);

    // Hint
    lv_obj_t *hintLabel = lv_label_create(card);
    lv_label_set_text(hintLabel, "TAP OR SHAKE TO ASK");
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ═══════════════════════════════════════════════════════════════════════════
// RUNNING PACE CARD (Activity + Stopwatch)
// ═══════════════════════════════════════════════════════════════════════════
void createRunningCard() {
    disableAllScrolling(lv_scr_act());
    GradientTheme *theme = getSafeTheme();

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_SHUFFLE " RUNNING");
    lv_obj_set_style_text_color(title, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Timer display
    unsigned long elapsed = 0;
    if (runningModeActive) {
        elapsed = millis() - runningStartTime;
    }
    int mins = (elapsed / 60000) % 60;
    int secs = (elapsed / 1000) % 60;
    int ms = (elapsed % 1000) / 10;

    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d.%02d", mins, secs, ms);
    lv_obj_t *timeLabel = lv_label_create(card);
    lv_label_set_text(timeLabel, timeBuf);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_36, 0);
    lv_obj_align(timeLabel, LV_ALIGN_TOP_MID, 0, 40);

    // Stats row
    int currentSteps = userData.steps;
    int runSteps = runningModeActive ? (currentSteps - runningStartSteps) : 0;
    
    // Calculate pace (steps per minute)
    if (runningModeActive && elapsed > 10000) {
        runningPace = (float)runSteps / (elapsed / 60000.0);
    }
    
    // Distance (assuming 0.7m per step)
    runningDistance = runSteps * 0.0007;  // km

    // Steps during run
    char stepsBuf[24];
    snprintf(stepsBuf, sizeof(stepsBuf), "%d", runSteps);
    lv_obj_t *stepsVal = lv_label_create(card);
    lv_label_set_text(stepsVal, stepsBuf);
    lv_obj_set_style_text_color(stepsVal, theme->accent, 0);
    lv_obj_set_style_text_font(stepsVal, &lv_font_montserrat_24, 0);
    lv_obj_align(stepsVal, LV_ALIGN_CENTER, -60, 20);

    lv_obj_t *stepsLbl = lv_label_create(card);
    lv_label_set_text(stepsLbl, "STEPS");
    lv_obj_set_style_text_color(stepsLbl, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(stepsLbl, &lv_font_montserrat_10, 0);
    lv_obj_align(stepsLbl, LV_ALIGN_CENTER, -60, 45);

    // Pace
    char paceBuf[16];
    snprintf(paceBuf, sizeof(paceBuf), "%.0f", runningPace);
    lv_obj_t *paceVal = lv_label_create(card);
    lv_label_set_text(paceVal, paceBuf);
    lv_obj_set_style_text_color(paceVal, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(paceVal, &lv_font_montserrat_24, 0);
    lv_obj_align(paceVal, LV_ALIGN_CENTER, 60, 20);

    lv_obj_t *paceLbl = lv_label_create(card);
    lv_label_set_text(paceLbl, "PACE/MIN");
    lv_obj_set_style_text_color(paceLbl, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(paceLbl, &lv_font_montserrat_10, 0);
    lv_obj_align(paceLbl, LV_ALIGN_CENTER, 60, 45);

    // Distance
    char distBuf[16];
    snprintf(distBuf, sizeof(distBuf), "%.2f km", runningDistance);
    lv_obj_t *distLabel = lv_label_create(card);
    lv_label_set_text(distLabel, distBuf);
    lv_obj_set_style_text_color(distLabel, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(distLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(distLabel, LV_ALIGN_CENTER, 0, 80);

    // Start/Stop button
    lv_obj_t *actionBtn = lv_obj_create(card);
    lv_obj_set_size(actionBtn, LCD_WIDTH - 60, 50);
    lv_obj_align(actionBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(actionBtn, runningModeActive ? lv_color_hex(0xFF453A) : lv_color_hex(0x30D158), 0);
    lv_obj_set_style_radius(actionBtn, 25, 0);
    lv_obj_set_style_border_width(actionBtn, 0, 0);
    disableAllScrolling(actionBtn);

    lv_obj_t *actionLabel = lv_label_create(actionBtn);
    lv_label_set_text(actionLabel, runningModeActive ? "STOP RUN" : "START RUN");
    lv_obj_set_style_text_color(actionLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(actionLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(actionLabel);
}
// ═══════════════════════════════════════════════════════════════════════════
// SETTINGS CARD - WITH INDIVIDUAL TAP BUTTONS + 12/24HR TOGGLE
// ═══════════════════════════════════════════════════════════════════════════
void createSettingsCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " SETTINGS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    int yPos = 32;
    int btnHeight = 42;
    int btnGap = 6;

    // ═══ WATCH FACE BUTTON ═══
    lv_obj_t *faceBtn = lv_obj_create(card);
    lv_obj_set_size(faceBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(faceBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(faceBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(faceBtn, 12, 0);
    lv_obj_set_style_border_width(faceBtn, 0, 0);
    disableAllScrolling(faceBtn);

    lv_obj_t *faceLabel = lv_label_create(faceBtn);
    lv_label_set_text(faceLabel, "WATCH FACE");
    lv_obj_set_style_text_color(faceLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(faceLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(faceLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *faceVal = lv_label_create(faceBtn);
    lv_label_set_text(faceVal, watchFaces[userData.watchFaceIndex].name);
    lv_obj_set_style_text_color(faceVal, theme->accent, 0);
    lv_obj_set_style_text_font(faceVal, &lv_font_montserrat_16, 0);
    lv_obj_align(faceVal, LV_ALIGN_LEFT_MID, 15, 10);

    lv_obj_t *faceArrow = lv_label_create(faceBtn);
    lv_label_set_text(faceArrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(faceArrow, lv_color_hex(0x636366), 0);
    lv_obj_align(faceArrow, LV_ALIGN_RIGHT_MID, -10, 0);

    yPos += btnHeight + btnGap;

    // ═══ WALLPAPER BUTTON ═══
    lv_obj_t *wallBtn = lv_obj_create(card);
    lv_obj_set_size(wallBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(wallBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(wallBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(wallBtn, 12, 0);
    lv_obj_set_style_border_width(wallBtn, 0, 0);
    disableAllScrolling(wallBtn);

    lv_obj_t *wallLabel = lv_label_create(wallBtn);
    lv_label_set_text(wallLabel, "WALLPAPER");
    lv_obj_set_style_text_color(wallLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(wallLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(wallLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *wallVal = lv_label_create(wallBtn);
    lv_label_set_text(wallVal, gradientWallpapers[userData.wallpaperIndex].name);
    lv_obj_set_style_text_color(wallVal, theme->accent, 0);
    lv_obj_set_style_text_font(wallVal, &lv_font_montserrat_16, 0);
    lv_obj_align(wallVal, LV_ALIGN_LEFT_MID, 15, 10);

    lv_obj_t *wallArrow = lv_label_create(wallBtn);
    lv_label_set_text(wallArrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(wallArrow, lv_color_hex(0x636366), 0);
    lv_obj_align(wallArrow, LV_ALIGN_RIGHT_MID, -10, 0);

    yPos += btnHeight + btnGap;

    // ═══ THEME BUTTON ═══
    lv_obj_t *themeBtn = lv_obj_create(card);
    lv_obj_set_size(themeBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(themeBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(themeBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(themeBtn, 12, 0);
    lv_obj_set_style_border_width(themeBtn, 0, 0);
    disableAllScrolling(themeBtn);

    lv_obj_t *themeLabel = lv_label_create(themeBtn);
    lv_label_set_text(themeLabel, "THEME");
    lv_obj_set_style_text_color(themeLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(themeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(themeLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *themeVal = lv_label_create(themeBtn);
    lv_label_set_text(themeVal, theme->name);
    lv_obj_set_style_text_color(themeVal, theme->accent, 0);
    lv_obj_set_style_text_font(themeVal, &lv_font_montserrat_16, 0);
    lv_obj_align(themeVal, LV_ALIGN_LEFT_MID, 15, 10);

    // Theme color preview
    lv_obj_t *themePreview = lv_obj_create(themeBtn);
    lv_obj_set_size(themePreview, 24, 24);
    lv_obj_align(themePreview, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(themePreview, theme->accent, 0);
    lv_obj_set_style_radius(themePreview, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(themePreview, 0, 0);
    disableAllScrolling(themePreview);

    yPos += btnHeight + btnGap;

    // ═══ BRIGHTNESS BUTTON ═══
    lv_obj_t *brightBtn = lv_obj_create(card);
    lv_obj_set_size(brightBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(brightBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(brightBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(brightBtn, 12, 0);
    lv_obj_set_style_border_width(brightBtn, 0, 0);
    disableAllScrolling(brightBtn);

    lv_obj_t *brightLabel = lv_label_create(brightBtn);
    lv_label_set_text(brightLabel, "BRIGHTNESS");
    lv_obj_set_style_text_color(brightLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(brightLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(brightLabel, LV_ALIGN_LEFT_MID, 15, -8);

    char brightBuf[16];
    int brightPercent = ((userData.brightness - 50) * 100) / 205;
    snprintf(brightBuf, sizeof(brightBuf), "%d%%", brightPercent);
    lv_obj_t *brightVal = lv_label_create(brightBtn);
    lv_label_set_text(brightVal, brightBuf);
    lv_obj_set_style_text_color(brightVal, theme->accent, 0);
    lv_obj_set_style_text_font(brightVal, &lv_font_montserrat_16, 0);
    lv_obj_align(brightVal, LV_ALIGN_LEFT_MID, 15, 10);

    // Brightness bar mini preview
    lv_obj_t *brightBar = lv_obj_create(brightBtn);
    lv_obj_set_size(brightBar, 60, 8);
    lv_obj_align(brightBar, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(brightBar, lv_color_hex(0x3A3A3C), 0);
    lv_obj_set_style_radius(brightBar, 4, 0);
    lv_obj_set_style_border_width(brightBar, 0, 0);
    disableAllScrolling(brightBar);

    // Filled portion
    int fillWidth = (userData.brightness - 50) * 60 / 205;
    lv_obj_t *brightFill = lv_obj_create(brightBar);
    lv_obj_set_size(brightFill, fillWidth, 8);
    lv_obj_align(brightFill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(brightFill, theme->accent, 0);
    lv_obj_set_style_radius(brightFill, 4, 0);
    lv_obj_set_style_border_width(brightFill, 0, 0);
    disableAllScrolling(brightFill);

    yPos += btnHeight + btnGap;

    // ═══ 12/24 HOUR FORMAT TOGGLE ═══
    lv_obj_t *timeFormatBtn = lv_obj_create(card);
    lv_obj_set_size(timeFormatBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(timeFormatBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(timeFormatBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(timeFormatBtn, 12, 0);
    lv_obj_set_style_border_width(timeFormatBtn, 0, 0);
    disableAllScrolling(timeFormatBtn);

    lv_obj_t *timeFormatLabel = lv_label_create(timeFormatBtn);
    lv_label_set_text(timeFormatLabel, "TIME FORMAT");
    lv_obj_set_style_text_color(timeFormatLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(timeFormatLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(timeFormatLabel, LV_ALIGN_LEFT_MID, 15, 0);

    lv_obj_t *timeFormatVal = lv_label_create(timeFormatBtn);
    lv_label_set_text(timeFormatVal, use24HourFormat ? "24HR" : "12HR");
    lv_obj_set_style_text_color(timeFormatVal, theme->accent, 0);
    lv_obj_set_style_text_font(timeFormatVal, &lv_font_montserrat_16, 0);
    lv_obj_align(timeFormatVal, LV_ALIGN_RIGHT_MID, -15, 0);

    yPos += btnHeight + btnGap;

    // ═══ BATTERY SAVER BUTTON ═══
    lv_obj_t *saverBtn = lv_obj_create(card);
    lv_obj_set_size(saverBtn, LCD_WIDTH - 30, 40);
    lv_obj_align(saverBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(saverBtn, batterySaverMode ? lv_color_hex(0x30D158) : lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(saverBtn, 12, 0);
    lv_obj_set_style_border_width(saverBtn, 0, 0);
    disableAllScrolling(saverBtn);

    lv_obj_t *saverLabel = lv_label_create(saverBtn);
    lv_label_set_text(saverLabel, batterySaverMode ? LV_SYMBOL_OK " SAVER ON" : "BATTERY SAVER");
    lv_obj_set_style_text_color(saverLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(saverLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(saverLabel);
}

// ═══════════════════════════════════════════════════════════════════════════
// HANDLE TAP IN SETTINGS - Individual buttons for each setting
// ═══════════════════════════════════════════════════════════════════════════
void handleSettingsTap(int x, int y) {
    // Button layout: btnHeight=42, btnGap=6, starting at y=32
    // Button 1 (Watch Face): y=32 to y=74
    // Button 2 (Wallpaper): y=80 to y=122
    // Button 3 (Theme): y=128 to y=170
    // Button 4 (Brightness): y=176 to y=218
    // Button 5 (Time Format): y=224 to y=266
    // Button 6 (Battery Saver): y=272 to y=312
    
    if (y >= 32 && y < 74) {
        // WATCH FACE button - cycle watch face
        userData.watchFaceIndex = (userData.watchFaceIndex + 1) % NUM_WATCH_FACES;
        USBSerial.printf("[SETTINGS] Watch face: %s\n", watchFaces[userData.watchFaceIndex].name);
    } 
    else if (y >= 80 && y < 122) {
        // WALLPAPER button - cycle wallpaper
        userData.wallpaperIndex = (userData.wallpaperIndex + 1) % NUM_GRADIENT_WALLPAPERS;
        USBSerial.printf("[SETTINGS] Wallpaper: %s\n", gradientWallpapers[userData.wallpaperIndex].name);
    } 
    else if (y >= 128 && y < 170) {
        // THEME button - cycle theme
        userData.themeIndex = (userData.themeIndex + 1) % NUM_THEMES;
        USBSerial.printf("[SETTINGS] Theme: %s\n", gradientThemes[userData.themeIndex].name);
    } 
    else if (y >= 176 && y < 218) {
        // BRIGHTNESS button - tap left to decrease, right to increase
        if (x < LCD_WIDTH / 2) {
            userData.brightness = max(50, userData.brightness - 25);
        } else {
            userData.brightness = min(255, userData.brightness + 25);
        }
        gfx->setBrightness(batterySaverMode ? 50 : userData.brightness);
        USBSerial.printf("[SETTINGS] Brightness: %d%%\n", ((userData.brightness - 50) * 100) / 205);
    } 
    else if (y >= 224 && y < 266) {
        // TIME FORMAT button - toggle 12/24 hour
        use24HourFormat = !use24HourFormat;
        USBSerial.printf("[SETTINGS] Time format: %s\n", use24HourFormat ? "24HR" : "12HR");
    }
    else if (y >= 272 && y < 320) {
        // BATTERY SAVER button - toggle
        toggleBatterySaver();
        USBSerial.printf("[SETTINGS] Battery saver: %s\n", batterySaverMode ? "ON" : "OFF");
    }

    saveUserData();
    navigateTo(currentCategory, currentSubCard);
}

// ═══════════════════════════════════════════════════════════════════════════
// BLUETOOTH SETTINGS CARD
// ═══════════════════════════════════════════════════════════════════════════
void createBluetoothCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title with Bluetooth icon
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_BLUETOOTH " BLUETOOTH");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    int yPos = 35;
    int btnHeight = 42;
    int btnGap = 6;

    // ═══ BLUETOOTH TOGGLE BUTTON ═══
    lv_obj_t *bleBtn = lv_obj_create(card);
    lv_obj_set_size(bleBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(bleBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(bleBtn, bleEnabled ? lv_color_hex(0x0A84FF) : lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(bleBtn, 12, 0);
    lv_obj_set_style_border_width(bleBtn, 0, 0);
    disableAllScrolling(bleBtn);

    lv_obj_t *bleLabel = lv_label_create(bleBtn);
    lv_label_set_text(bleLabel, "BLUETOOTH");
    lv_obj_set_style_text_color(bleLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(bleLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(bleLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *bleStatus = lv_label_create(bleBtn);
    // Show status with timer
    if (bleEnabled && bleDeviceConnected) {
        lv_label_set_text(bleStatus, "Connected");
        lv_obj_set_style_text_color(bleStatus, lv_color_hex(0x30D158), 0);
    } else if (bleEnabled) {
        // Show countdown timer
        unsigned long elapsed = millis() - bleLastActivityTime;
        unsigned long remaining = (BLE_AUTO_OFF_MS > elapsed) ? (BLE_AUTO_OFF_MS - elapsed) : 0;
        int secsLeft = remaining / 1000;
        int mins = secsLeft / 60;
        int secs = secsLeft % 60;
        char timerBuf[24];
        snprintf(timerBuf, sizeof(timerBuf), "ON - %d:%02d left", mins, secs);
        lv_label_set_text(bleStatus, timerBuf);
        lv_obj_set_style_text_color(bleStatus, lv_color_hex(0xFF9F0A), 0);
    } else {
        lv_label_set_text(bleStatus, "OFF - Tap to enable");
        lv_obj_set_style_text_color(bleStatus, lv_color_hex(0x8E8E93), 0);
    }
    lv_obj_set_style_text_font(bleStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(bleStatus, LV_ALIGN_LEFT_MID, 15, 8);

    yPos += btnHeight + btnGap;

    // ═══ CONNECTION STATUS ═══
    lv_obj_t *connBtn = lv_obj_create(card);
    lv_obj_set_size(connBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(connBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(connBtn, bleDeviceConnected ? lv_color_hex(0x30D158) : lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(connBtn, 12, 0);
    lv_obj_set_style_border_width(connBtn, 0, 0);
    disableAllScrolling(connBtn);

    lv_obj_t *connLabel = lv_label_create(connBtn);
    lv_label_set_text(connLabel, "STATUS");
    lv_obj_set_style_text_color(connLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(connLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(connLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *connStatus = lv_label_create(connBtn);
    if (bleDeviceConnected) {
        lv_label_set_text(connStatus, bleConnectedDeviceName.length() > 0 ? bleConnectedDeviceName.c_str() : "Connected");
    } else if (bleEnabled) {
        lv_label_set_text(connStatus, "Searching...");
    } else {
        lv_label_set_text(connStatus, "Not Connected");
    }
    lv_obj_set_style_text_color(connStatus, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(connStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(connStatus, LV_ALIGN_LEFT_MID, 15, 8);

    yPos += btnHeight + btnGap;

    // ═══ TIME SYNC STATUS ═══
    lv_obj_t *syncBtn = lv_obj_create(card);
    lv_obj_set_size(syncBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(syncBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(syncBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(syncBtn, 12, 0);
    lv_obj_set_style_border_width(syncBtn, 0, 0);
    disableAllScrolling(syncBtn);

    lv_obj_t *syncLabel = lv_label_create(syncBtn);
    lv_label_set_text(syncLabel, "TIME SYNC");
    lv_obj_set_style_text_color(syncLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(syncLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(syncLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *syncStatus = lv_label_create(syncBtn);
    lv_label_set_text(syncStatus, bleTimeSynced ? "Synced via BLE" : "Not synced");
    lv_obj_set_style_text_color(syncStatus, bleTimeSynced ? lv_color_hex(0x30D158) : lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(syncStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(syncStatus, LV_ALIGN_LEFT_MID, 15, 8);

    yPos += btnHeight + btnGap;

    // ═══ DEVICE NAME ═══
    lv_obj_t *nameBtn = lv_obj_create(card);
    lv_obj_set_size(nameBtn, LCD_WIDTH - 30, btnHeight);
    lv_obj_align(nameBtn, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_color(nameBtn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(nameBtn, 12, 0);
    lv_obj_set_style_border_width(nameBtn, 0, 0);
    disableAllScrolling(nameBtn);

    lv_obj_t *nameLabel = lv_label_create(nameBtn);
    lv_label_set_text(nameLabel, "DEVICE NAME");
    lv_obj_set_style_text_color(nameLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(nameLabel, LV_ALIGN_LEFT_MID, 15, -8);

    lv_obj_t *nameVal = lv_label_create(nameBtn);
    lv_label_set_text(nameVal, BLE_DEVICE_NAME);
    lv_obj_set_style_text_color(nameVal, theme->accent, 0);
    lv_obj_set_style_text_font(nameVal, &lv_font_montserrat_12, 0);
    lv_obj_align(nameVal, LV_ALIGN_LEFT_MID, 15, 8);

    yPos += btnHeight + btnGap;

    // ═══ HINT ═══
    lv_obj_t *hint = lv_label_create(card);
    lv_label_set_text(hint, "Tap top button to toggle BLE");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -8);
}

// ═══════════════════════════════════════════════════════════════════════════
// HANDLE BLUETOOTH CARD TAP
// ═══════════════════════════════════════════════════════════════════════════
void handleBluetoothTap(int x, int y) {
    // Button layout: btnHeight=42, btnGap=6, starting at y=35
    // Button 1 (BLE Toggle): y=35 to y=77
    
    if (y >= 35 && y < 77) {
        // Toggle BLE on/off
        if (bleEnabled) {
            stopBLE();
            USBSerial.println("[BLE] Manually disabled");
        } else {
            initGadgetbridgeBLE();
            USBSerial.println("[BLE] Manually enabled (3min timer)");
        }
    }
    
    navigateTo(currentCategory, currentSubCard);
}

// ═══════════════════════════════════════════════════════════════════════════
// NOTIFICATIONS CARD - PHONE NOTIFICATIONS VIA BLE
// ═══════════════════════════════════════════════════════════════════════════
int notifScrollOffset = 0;

// Update unique app filters list
void updateNotifAppFilters() {
    notifAppFilters[0] = "All";
    notifAppFilterCount = 1;
    
    for (int i = 0; i < notificationCount; i++) {
        bool found = false;
        for (int j = 1; j < notifAppFilterCount; j++) {
            if (notifAppFilters[j] == notifications[i].app) {
                found = true;
                break;
            }
        }
        if (!found && notifAppFilterCount < MAX_NOTIFICATIONS + 1) {
            notifAppFilters[notifAppFilterCount++] = notifications[i].app;
        }
    }
}

void addNotification(String app, String title, String body) {
    // Shift all notifications down
    for (int i = MAX_NOTIFICATIONS - 1; i > 0; i--) {
        notifications[i] = notifications[i-1];
    }
    // Add new notification at top
    notifications[0].app = app;
    notifications[0].title = title;
    notifications[0].body = body;
    notifications[0].timestamp = millis();
    notifications[0].read = false;
    
    if (notificationCount < MAX_NOTIFICATIONS) notificationCount++;
    
    // Update app filter list
    updateNotifAppFilters();
    
    USBSerial.printf("[NOTIF] New: %s - %s\n", app.c_str(), title.c_str());
    
    // Show popup if screen is on
    if (screenOn) {
        showNotificationPopup(0);
    }
}

void showNotificationPopup(int index) {
    if (index < 0 || index >= notificationCount) return;
    
    // Create popup overlay
    lv_obj_t *popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup, LCD_WIDTH - 20, 80);
    lv_obj_align(popup, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_90, 0);
    lv_obj_set_style_radius(popup, 16, 0);
    lv_obj_set_style_border_width(popup, 1, 0);
    lv_obj_set_style_border_color(popup, lv_color_hex(0x3A3A3C), 0);
    
    // App name
    lv_obj_t *appLabel = lv_label_create(popup);
    lv_label_set_text(appLabel, notifications[index].app.c_str());
    lv_obj_set_style_text_color(appLabel, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(appLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(appLabel, LV_ALIGN_TOP_LEFT, 10, 5);
    
    // Title
    lv_obj_t *titleLabel = lv_label_create(popup);
    lv_label_set_text(titleLabel, notifications[index].title.c_str());
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 10, 20);
    
    // Body (truncated)
    lv_obj_t *bodyLabel = lv_label_create(popup);
    String truncBody = notifications[index].body;
    if (truncBody.length() > 40) truncBody = truncBody.substring(0, 40) + "...";
    lv_label_set_text(bodyLabel, truncBody.c_str());
    lv_obj_set_style_text_color(bodyLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(bodyLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(bodyLabel, LV_ALIGN_TOP_LEFT, 10, 38);
    
    notifications[index].read = true;
}

// Get filtered notification count
int getFilteredNotifCount() {
    if (notificationFilter == 0) return notificationCount;  // "All"
    
    int count = 0;
    String filterApp = notifAppFilters[notificationFilter];
    for (int i = 0; i < notificationCount; i++) {
        if (notifications[i].app == filterApp) count++;
    }
    return count;
}

// Get notification index by filtered position
int getFilteredNotifIndex(int filteredPos) {
    if (notificationFilter == 0) return filteredPos;  // "All" - direct mapping
    
    String filterApp = notifAppFilters[notificationFilter];
    int count = 0;
    for (int i = 0; i < notificationCount; i++) {
        if (notifications[i].app == filterApp) {
            if (count == filteredPos) return i;
            count++;
        }
    }
    return -1;
}

void createNotificationsCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // ═══ FULL NOTIFICATION VIEW ═══
    if (selectedNotifIndex >= 0 && selectedNotifIndex < notificationCount) {
        // Back button
        lv_obj_t *backBtn = lv_obj_create(card);
        lv_obj_set_size(backBtn, LCD_WIDTH - 30, 32);
        lv_obj_align(backBtn, LV_ALIGN_TOP_MID, 0, 8);
        lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(backBtn, 8, 0);
        lv_obj_set_style_border_width(backBtn, 0, 0);
        disableAllScrolling(backBtn);

        lv_obj_t *backLabel = lv_label_create(backBtn);
        lv_label_set_text(backLabel, LV_SYMBOL_LEFT " Back to list");
        lv_obj_set_style_text_color(backLabel, lv_color_hex(0x0A84FF), 0);
        lv_obj_set_style_text_font(backLabel, &lv_font_montserrat_12, 0);
        lv_obj_center(backLabel);

        // App name header
        lv_obj_t *appHeader = lv_label_create(card);
        lv_label_set_text(appHeader, notifications[selectedNotifIndex].app.c_str());
        lv_obj_set_style_text_color(appHeader, lv_color_hex(0x0A84FF), 0);
        lv_obj_set_style_text_font(appHeader, &lv_font_montserrat_14, 0);
        lv_obj_align(appHeader, LV_ALIGN_TOP_LEFT, 15, 50);

        // Time ago
        lv_obj_t *timeLabel = lv_label_create(card);
        unsigned long ago = (millis() - notifications[selectedNotifIndex].timestamp) / 1000;
        char timeBuf[20];
        if (ago < 60) snprintf(timeBuf, sizeof(timeBuf), "%lu seconds ago", ago);
        else if (ago < 3600) snprintf(timeBuf, sizeof(timeBuf), "%lu minutes ago", ago / 60);
        else snprintf(timeBuf, sizeof(timeBuf), "%lu hours ago", ago / 3600);
        lv_label_set_text(timeLabel, timeBuf);
        lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(timeLabel, LV_ALIGN_TOP_RIGHT, -15, 52);

        // Title (full)
        lv_obj_t *titleFull = lv_label_create(card);
        lv_label_set_text(titleFull, notifications[selectedNotifIndex].title.c_str());
        lv_obj_set_style_text_color(titleFull, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(titleFull, &lv_font_montserrat_14, 0);
        lv_obj_set_width(titleFull, LCD_WIDTH - 30);
        lv_label_set_long_mode(titleFull, LV_LABEL_LONG_WRAP);
        lv_obj_align(titleFull, LV_ALIGN_TOP_LEFT, 15, 75);

        // Body (full, scrollable area)
        lv_obj_t *bodyArea = lv_obj_create(card);
        lv_obj_set_size(bodyArea, LCD_WIDTH - 20, LCD_HEIGHT - 120);
        lv_obj_align(bodyArea, LV_ALIGN_TOP_MID, 0, 110);
        lv_obj_set_style_bg_color(bodyArea, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(bodyArea, 12, 0);
        lv_obj_set_style_border_width(bodyArea, 0, 0);
        lv_obj_set_style_pad_all(bodyArea, 10, 0);
        lv_obj_set_scroll_dir(bodyArea, LV_DIR_VER);

        lv_obj_t *bodyFull = lv_label_create(bodyArea);
        lv_label_set_text(bodyFull, notifications[selectedNotifIndex].body.c_str());
        lv_obj_set_style_text_color(bodyFull, lv_color_hex(0xE5E5E7), 0);
        lv_obj_set_style_text_font(bodyFull, &lv_font_montserrat_12, 0);
        lv_obj_set_width(bodyFull, LCD_WIDTH - 50);
        lv_label_set_long_mode(bodyFull, LV_LABEL_LONG_WRAP);

        notifications[selectedNotifIndex].read = true;
        return;
    }

    // ═══ LIST VIEW ═══
    // Title with bell icon
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_BELL " NOTIFICATIONS");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    // App filter button (tap to cycle through apps)
    lv_obj_t *filterBtn = lv_obj_create(card);
    lv_obj_set_size(filterBtn, LCD_WIDTH - 30, 36);
    lv_obj_align(filterBtn, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_style_bg_color(filterBtn, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_radius(filterBtn, 10, 0);
    lv_obj_set_style_border_width(filterBtn, 0, 0);
    disableAllScrolling(filterBtn);

    lv_obj_t *filterLabel = lv_label_create(filterBtn);
    char filterBuf[40];
    if (notificationFilter < notifAppFilterCount) {
        snprintf(filterBuf, sizeof(filterBuf), LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, notifAppFilters[notificationFilter].c_str());
    } else {
        snprintf(filterBuf, sizeof(filterBuf), LV_SYMBOL_LEFT " All " LV_SYMBOL_RIGHT);
    }
    lv_label_set_text(filterLabel, filterBuf);
    lv_obj_set_style_text_color(filterLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(filterLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(filterLabel);

    // Synced steps display
    lv_obj_t *stepsRow = lv_obj_create(card);
    lv_obj_set_size(stepsRow, LCD_WIDTH - 30, 32);
    lv_obj_align(stepsRow, LV_ALIGN_TOP_MID, 0, 72);
    lv_obj_set_style_bg_color(stepsRow, stepsSynced ? lv_color_hex(0x30D158) : lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(stepsRow, 10, 0);
    lv_obj_set_style_border_width(stepsRow, 0, 0);
    disableAllScrolling(stepsRow);

    lv_obj_t *stepsLabel = lv_label_create(stepsRow);
    char stepsBuf[50];
    if (stepsSynced) {
        snprintf(stepsBuf, sizeof(stepsBuf), LV_SYMBOL_REFRESH " %d steps | %d cal", syncedSteps, syncedCalories);
    } else {
        snprintf(stepsBuf, sizeof(stepsBuf), LV_SYMBOL_REFRESH " Steps: Connect to sync");
    }
    lv_label_set_text(stepsLabel, stepsBuf);
    lv_obj_set_style_text_color(stepsLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stepsLabel, &lv_font_montserrat_10, 0);
    lv_obj_center(stepsLabel);

    // Notification count for current filter
    int filteredCount = getFilteredNotifCount();
    lv_obj_t *countLabel = lv_label_create(card);
    char countBuf[40];
    snprintf(countBuf, sizeof(countBuf), "%d notification%s", filteredCount, filteredCount == 1 ? "" : "s");
    lv_label_set_text(countLabel, countBuf);
    lv_obj_set_style_text_color(countLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(countLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(countLabel, LV_ALIGN_TOP_LEFT, 18, 108);

    // Notification list area
    int yPos = 124;
    int itemHeight = 52;
    int maxVisible = 3;
    
    int displayedCount = 0;
    for (int filteredIdx = notifScrollOffset; filteredIdx < filteredCount && displayedCount < maxVisible; filteredIdx++) {
        int realIdx = getFilteredNotifIndex(filteredIdx);
        if (realIdx < 0) continue;
        
        lv_obj_t *notifItem = lv_obj_create(card);
        lv_obj_set_size(notifItem, LCD_WIDTH - 20, itemHeight - 4);
        lv_obj_align(notifItem, LV_ALIGN_TOP_MID, 0, yPos + (displayedCount * itemHeight));
        lv_obj_set_style_bg_color(notifItem, notifications[realIdx].read ? lv_color_hex(0x1C1C1E) : lv_color_hex(0x2C2C2E), 0);
        lv_obj_set_style_radius(notifItem, 12, 0);
        lv_obj_set_style_border_width(notifItem, 0, 0);
        disableAllScrolling(notifItem);

        // App name (top left, colored)
        lv_obj_t *appLbl = lv_label_create(notifItem);
        lv_label_set_text(appLbl, notifications[realIdx].app.c_str());
        lv_obj_set_style_text_color(appLbl, lv_color_hex(0x0A84FF), 0);
        lv_obj_set_style_text_font(appLbl, &lv_font_montserrat_10, 0);
        lv_obj_align(appLbl, LV_ALIGN_TOP_LEFT, 10, 3);

        // Time ago (top right)
        lv_obj_t *timeLbl = lv_label_create(notifItem);
        unsigned long ago = (millis() - notifications[realIdx].timestamp) / 1000;
        char timeBuf[16];
        if (ago < 60) snprintf(timeBuf, sizeof(timeBuf), "%lus", ago);
        else if (ago < 3600) snprintf(timeBuf, sizeof(timeBuf), "%lum", ago / 60);
        else snprintf(timeBuf, sizeof(timeBuf), "%luh", ago / 3600);
        lv_label_set_text(timeLbl, timeBuf);
        lv_obj_set_style_text_color(timeLbl, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(timeLbl, &lv_font_montserrat_10, 0);
        lv_obj_align(timeLbl, LV_ALIGN_TOP_RIGHT, -10, 3);

        // Title
        lv_obj_t *titleLbl = lv_label_create(notifItem);
        String titleTrunc = notifications[realIdx].title;
        if (titleTrunc.length() > 28) titleTrunc = titleTrunc.substring(0, 28) + "...";
        lv_label_set_text(titleLbl, titleTrunc.c_str());
        lv_obj_set_style_text_color(titleLbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(titleLbl, &lv_font_montserrat_12, 0);
        lv_obj_align(titleLbl, LV_ALIGN_TOP_LEFT, 10, 17);

        // Body preview
        lv_obj_t *bodyLbl = lv_label_create(notifItem);
        String bodyTrunc = notifications[realIdx].body;
        if (bodyTrunc.length() > 38) bodyTrunc = bodyTrunc.substring(0, 38) + "...";
        lv_label_set_text(bodyLbl, bodyTrunc.c_str());
        lv_obj_set_style_text_color(bodyLbl, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(bodyLbl, &lv_font_montserrat_10, 0);
        lv_obj_align(bodyLbl, LV_ALIGN_TOP_LEFT, 10, 32);

        displayedCount++;
    }

    // Empty state
    if (filteredCount == 0) {
        lv_obj_t *emptyLabel = lv_label_create(card);
        if (notificationCount == 0) {
            lv_label_set_text(emptyLabel, "No notifications\nConnect via Gadgetbridge");
        } else {
            lv_label_set_text(emptyLabel, "No notifications\nfrom this app");
        }
        lv_obj_set_style_text_color(emptyLabel, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(emptyLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(emptyLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(emptyLabel, LV_ALIGN_CENTER, 0, 30);
    }

    // Scroll hint
    if (filteredCount > maxVisible) {
        lv_obj_t *scrollHint = lv_label_create(card);
        char scrollBuf[30];
        snprintf(scrollBuf, sizeof(scrollBuf), LV_SYMBOL_DOWN " %d more", filteredCount - maxVisible - notifScrollOffset);
        lv_label_set_text(scrollHint, scrollBuf);
        lv_obj_set_style_text_color(scrollHint, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(scrollHint, &lv_font_montserrat_10, 0);
        lv_obj_align(scrollHint, LV_ALIGN_BOTTOM_MID, 0, -8);
    }
}

void handleNotificationsTap(int x, int y) {
    // ═══ FULL VIEW - Back button ═══
    if (selectedNotifIndex >= 0) {
        if (y < 45) {
            // Back button tapped
            selectedNotifIndex = -1;
            navigateTo(currentCategory, currentSubCard);
            return;
        }
        return;  // In full view, only back button works
    }

    // ═══ LIST VIEW ═══
    // Filter button area (32-68) - tap LEFT or RIGHT side to change filter
    if (y >= 32 && y < 68) {
        if (notifAppFilterCount <= 1) return;  // No apps to filter
        
        if (x < LCD_WIDTH / 2) {
            // LEFT tap - go to previous filter
            notificationFilter--;
            if (notificationFilter < 0) notificationFilter = notifAppFilterCount - 1;
        } else {
            // RIGHT tap - go to next filter
            notificationFilter++;
            if (notificationFilter >= notifAppFilterCount) notificationFilter = 0;
        }
        notifScrollOffset = 0;  // Reset scroll when changing filter
        USBSerial.printf("[NOTIF] Filter: %s\n", notifAppFilters[notificationFilter].c_str());
        navigateTo(currentCategory, currentSubCard);
        return;
    }
    
    // Notification list area (124+)
    if (y >= 124) {
        int itemHeight = 52;
        int tappedPos = (y - 124) / itemHeight;
        int filteredIdx = tappedPos + notifScrollOffset;
        int filteredCount = getFilteredNotifCount();
        
        // Scroll down if tapping bottom area
        if (y > LCD_HEIGHT - 40 && filteredCount > 3) {
            notifScrollOffset = min(notifScrollOffset + 1, max(0, filteredCount - 3));
            navigateTo(currentCategory, currentSubCard);
            return;
        }
        
        // Tap notification to view full details
        if (filteredIdx >= 0 && filteredIdx < filteredCount) {
            int realIdx = getFilteredNotifIndex(filteredIdx);
            if (realIdx >= 0) {
                selectedNotifIndex = realIdx;
                USBSerial.printf("[NOTIF] Viewing notification %d\n", realIdx);
                navigateTo(currentCategory, currentSubCard);
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY CARD - PREMIUM DESIGN (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void createBatteryCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "BATTERY");
    lv_obj_set_style_text_color(title, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_12, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);

    // Large percentage
    char percBuf[8];
    snprintf(percBuf, sizeof(percBuf), "%d%%", batteryPercent);
    lv_obj_t *percLbl = lv_label_create(card);
    lv_label_set_text(percLbl, percBuf);
    lv_obj_set_style_text_color(percLbl, batteryPercent > 20 ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_text_font(percLbl, &lv_font_montserrat_48, 0);
    lv_obj_align(percLbl, LV_ALIGN_TOP_MID, 0, 40);

    // Charging/estimate status
    lv_obj_t *statusLbl = lv_label_create(card);
    if (isCharging) {
        lv_label_set_text(statusLbl, LV_SYMBOL_CHARGE " Charging");
        lv_obj_set_style_text_color(statusLbl, lv_color_hex(0x30D158), 0);
    } else {
        char estBuf[32];
        uint32_t hrs = batteryStats.combinedEstimateMins / 60;
        uint32_t mins = batteryStats.combinedEstimateMins % 60;
        snprintf(estBuf, sizeof(estBuf), "~%luh %lum remaining", hrs, mins);
        lv_label_set_text(statusLbl, estBuf);
        lv_obj_set_style_text_color(statusLbl, lv_color_hex(0x8E8E93), 0);
    }
    lv_obj_set_style_text_font(statusLbl, &lv_font_montserrat_14, 0);
    lv_obj_align(statusLbl, LV_ALIGN_TOP_MID, 0, 100);

    // Voltage
    char voltBuf[16];
    snprintf(voltBuf, sizeof(voltBuf), "%dmV", batteryVoltage);
    lv_obj_t *voltLbl = lv_label_create(card);
    lv_label_set_text(voltLbl, voltBuf);
    lv_obj_set_style_text_color(voltLbl, lv_color_hex(0x636366), 0);
    lv_obj_align(voltLbl, LV_ALIGN_TOP_MID, 0, 125);

    // Battery saver toggle row
    lv_obj_t *saverRow = lv_obj_create(card);
    lv_obj_set_size(saverRow, LCD_WIDTH - 40, 50);
    lv_obj_align(saverRow, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(saverRow, lv_color_hex(0x2C2C2E), 0);
    lv_obj_set_style_radius(saverRow, 14, 0);
    lv_obj_set_style_border_width(saverRow, 0, 0);
    disableAllScrolling(saverRow);

    lv_obj_t *saverLbl = lv_label_create(saverRow);
    lv_label_set_text(saverLbl, "Battery Saver");
    lv_obj_set_style_text_color(saverLbl, theme->text, 0);
    lv_obj_align(saverLbl, LV_ALIGN_LEFT_MID, 15, 0);

    lv_obj_t *saverStatus = lv_label_create(saverRow);
    lv_label_set_text(saverStatus, batterySaverMode ? "ON" : "OFF");
    lv_obj_set_style_text_color(saverStatus, batterySaverMode ? lv_color_hex(0xFF9F0A) : lv_color_hex(0x636366), 0);
    lv_obj_align(saverStatus, LV_ALIGN_RIGHT_MID, -15, 0);

    // System info row
    lv_obj_t *infoRow = lv_obj_create(card);
    lv_obj_set_size(infoRow, LCD_WIDTH - 40, 80);
    lv_obj_align(infoRow, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_color(infoRow, lv_color_hex(0x2C2C2E), 0);
    lv_obj_set_style_radius(infoRow, 14, 0);
    lv_obj_set_style_border_width(infoRow, 0, 0);
    disableAllScrolling(infoRow);

    // WiFi status
    char wifiBuf[48];
    if (wifiConnected) {
        snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: %s", wifiNetworks[0].ssid);
    } else {
        snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: Disconnected");
    }
    lv_obj_t *wifiLbl = lv_label_create(infoRow);
    lv_label_set_text(wifiLbl, wifiBuf);
    lv_obj_set_style_text_color(wifiLbl, wifiConnected ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_text_font(wifiLbl, &lv_font_montserrat_12, 0);
    lv_obj_align(wifiLbl, LV_ALIGN_TOP_LEFT, 15, 12);

    // SD status
    char sdBuf[32];
    snprintf(sdBuf, sizeof(sdBuf), "SD: %s", hasSD ? "Mounted" : "Not Found");
    lv_obj_t *sdLbl = lv_label_create(infoRow);
    lv_label_set_text(sdLbl, sdBuf);
    lv_obj_set_style_text_color(sdLbl, hasSD ? lv_color_hex(0x30D158) : lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(sdLbl, &lv_font_montserrat_12, 0);
    lv_obj_align(sdLbl, LV_ALIGN_TOP_LEFT, 15, 32);

    // Free RAM
    char ramBuf[32];
    snprintf(ramBuf, sizeof(ramBuf), "Free RAM: %luKB", freeRAM / 1024);
    lv_obj_t *ramLbl = lv_label_create(infoRow);
    lv_label_set_text(ramLbl, ramBuf);
    lv_obj_set_style_text_color(ramLbl, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(ramLbl, &lv_font_montserrat_12, 0);
    lv_obj_align(ramLbl, LV_ALIGN_TOP_LEFT, 15, 52);
}

// ═══════════════════════════════════════════════════════════════════════════
// BATTERY STATS CARD - PREMIUM DESIGN
// ═══════════════════════════════════════════════════════════════════════════
void createBatteryStatsCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title with accent
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "BATTERY STATS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    unsigned long screenOnMins = batteryStats.screenOnTimeMs / 60000;
    unsigned long screenOffMins = batteryStats.screenOffTimeMs / 60000;
    
    // Screen On Time Card
    lv_obj_t *onCard = lv_obj_create(card);
    lv_obj_set_size(onCard, LCD_WIDTH - 40, 70);
    lv_obj_align(onCard, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(onCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(onCard, 15, 0);
    lv_obj_set_style_border_width(onCard, 0, 0);
    disableAllScrolling(onCard);

    lv_obj_t *onIcon = lv_label_create(onCard);
    lv_label_set_text(onIcon, LV_SYMBOL_EYE_OPEN);
    lv_obj_set_style_text_color(onIcon, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(onIcon, &lv_font_montserrat_24, 0);
    lv_obj_align(onIcon, LV_ALIGN_LEFT_MID, 15, 0);

    lv_obj_t *onLabel = lv_label_create(onCard);
    lv_label_set_text(onLabel, "Screen On");
    lv_obj_set_style_text_color(onLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(onLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(onLabel, LV_ALIGN_LEFT_MID, 55, -12);

    char onBuf[16];
    snprintf(onBuf, sizeof(onBuf), "%lu min", screenOnMins);
    lv_obj_t *onValue = lv_label_create(onCard);
    lv_label_set_text(onValue, onBuf);
    lv_obj_set_style_text_color(onValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(onValue, &lv_font_montserrat_20, 0);
    lv_obj_align(onValue, LV_ALIGN_LEFT_MID, 55, 12);

    // Screen Off Time Card  
    lv_obj_t *offCard = lv_obj_create(card);
    lv_obj_set_size(offCard, LCD_WIDTH - 40, 70);
    lv_obj_align(offCard, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_set_style_bg_color(offCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(offCard, 15, 0);
    lv_obj_set_style_border_width(offCard, 0, 0);
    disableAllScrolling(offCard);

    lv_obj_t *offIcon = lv_label_create(offCard);
    lv_label_set_text(offIcon, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_text_color(offIcon, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(offIcon, &lv_font_montserrat_24, 0);
    lv_obj_align(offIcon, LV_ALIGN_LEFT_MID, 15, 0);

    lv_obj_t *offLabel = lv_label_create(offCard);
    lv_label_set_text(offLabel, "Screen Off");
    lv_obj_set_style_text_color(offLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(offLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(offLabel, LV_ALIGN_LEFT_MID, 55, -12);

    char offBuf[16];
    snprintf(offBuf, sizeof(offBuf), "%lu min", screenOffMins);
    lv_obj_t *offValue = lv_label_create(offCard);
    lv_label_set_text(offValue, offBuf);
    lv_obj_set_style_text_color(offValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(offValue, &lv_font_montserrat_20, 0);
    lv_obj_align(offValue, LV_ALIGN_LEFT_MID, 55, 12);

    // Voltage & Estimate Card
    lv_obj_t *voltCard = lv_obj_create(card);
    lv_obj_set_size(voltCard, LCD_WIDTH - 40, 100);
    lv_obj_align(voltCard, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(voltCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(voltCard, 15, 0);
    lv_obj_set_style_border_width(voltCard, 0, 0);
    disableAllScrolling(voltCard);

    char voltBuf[32];
    snprintf(voltBuf, sizeof(voltBuf), "%d mV", batteryVoltage);
    lv_obj_t *voltLabel = lv_label_create(voltCard);
    lv_label_set_text(voltLabel, voltBuf);
    lv_obj_set_style_text_color(voltLabel, lv_color_hex(0xFFD60A), 0);
    lv_obj_set_style_text_font(voltLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(voltLabel, LV_ALIGN_TOP_MID, 0, 15);

    char estBuf[48];
    uint32_t hrs = batteryStats.combinedEstimateMins / 60;
    uint32_t mins = batteryStats.combinedEstimateMins % 60;
    snprintf(estBuf, sizeof(estBuf), "~%lu hrs %lu min remaining", hrs, mins);
    lv_obj_t *estLabel = lv_label_create(voltCard);
    lv_label_set_text(estLabel, estBuf);
    lv_obj_set_style_text_color(estLabel, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(estLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(estLabel, LV_ALIGN_BOTTOM_MID, 0, -15);
}

// ═══════════════════════════════════════════════════════════════════════════
// USAGE PATTERNS CARD - PREMIUM SYSTEM STATS DESIGN
// ═══════════════════════════════════════════════════════════════════════════
void createUsagePatternsCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "SYSTEM STATS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // ═══ RAM USAGE GAUGE ═══
    lv_obj_t *ramCard = lv_obj_create(card);
    lv_obj_set_size(ramCard, (LCD_WIDTH - 50) / 2, 120);
    lv_obj_align(ramCard, LV_ALIGN_TOP_LEFT, 15, 50);
    lv_obj_set_style_bg_color(ramCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(ramCard, 15, 0);
    lv_obj_set_style_border_width(ramCard, 0, 0);
    disableAllScrolling(ramCard);

    lv_obj_t *ramTitle = lv_label_create(ramCard);
    lv_label_set_text(ramTitle, "RAM");
    lv_obj_set_style_text_color(ramTitle, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(ramTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(ramTitle, LV_ALIGN_TOP_MID, 0, 8);

    // RAM arc gauge
    uint32_t totalRAM = 512 * 1024;  // 512KB total
    int ramPercent = ((totalRAM - freeRAM) * 100) / totalRAM;
    
    lv_obj_t *ramArc = lv_arc_create(ramCard);
    lv_obj_set_size(ramArc, 70, 70);
    lv_obj_align(ramArc, LV_ALIGN_CENTER, 0, 5);
    lv_arc_set_rotation(ramArc, 135);
    lv_arc_set_bg_angles(ramArc, 0, 270);
    lv_arc_set_range(ramArc, 0, 100);
    lv_arc_set_value(ramArc, ramPercent);
    lv_obj_set_style_arc_width(ramArc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ramArc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ramArc, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_arc_color(ramArc, lv_color_hex(0x30D158), LV_PART_INDICATOR);
    lv_obj_remove_style(ramArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ramArc, LV_OBJ_FLAG_CLICKABLE);

    char ramBuf[16];
    snprintf(ramBuf, sizeof(ramBuf), "%d%%", ramPercent);
    lv_obj_t *ramPerc = lv_label_create(ramArc);
    lv_label_set_text(ramPerc, ramBuf);
    lv_obj_set_style_text_color(ramPerc, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ramPerc, &lv_font_montserrat_14, 0);
    lv_obj_center(ramPerc);

    char freeRamBuf[16];
    snprintf(freeRamBuf, sizeof(freeRamBuf), "%luKB free", freeRAM / 1024);
    lv_obj_t *ramFree = lv_label_create(ramCard);
    lv_label_set_text(ramFree, freeRamBuf);
    lv_obj_set_style_text_color(ramFree, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(ramFree, &lv_font_montserrat_10, 0);
    lv_obj_align(ramFree, LV_ALIGN_BOTTOM_MID, 0, -5);

    // ═══ CPU USAGE GAUGE ═══
    lv_obj_t *cpuCard = lv_obj_create(card);
    lv_obj_set_size(cpuCard, (LCD_WIDTH - 50) / 2, 120);
    lv_obj_align(cpuCard, LV_ALIGN_TOP_RIGHT, -15, 50);
    lv_obj_set_style_bg_color(cpuCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(cpuCard, 15, 0);
    lv_obj_set_style_border_width(cpuCard, 0, 0);
    disableAllScrolling(cpuCard);

    lv_obj_t *cpuTitle = lv_label_create(cpuCard);
    lv_label_set_text(cpuTitle, "CPU");
    lv_obj_set_style_text_color(cpuTitle, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(cpuTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(cpuTitle, LV_ALIGN_TOP_MID, 0, 8);

    // Simulated CPU load (based on screen state)
    int cpuPercent = screenOn ? random(15, 35) : random(5, 15);
    
    lv_obj_t *cpuArc = lv_arc_create(cpuCard);
    lv_obj_set_size(cpuArc, 70, 70);
    lv_obj_align(cpuArc, LV_ALIGN_CENTER, 0, 5);
    lv_arc_set_rotation(cpuArc, 135);
    lv_arc_set_bg_angles(cpuArc, 0, 270);
    lv_arc_set_range(cpuArc, 0, 100);
    lv_arc_set_value(cpuArc, cpuPercent);
    lv_obj_set_style_arc_width(cpuArc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(cpuArc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(cpuArc, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_arc_color(cpuArc, lv_color_hex(0x0A84FF), LV_PART_INDICATOR);
    lv_obj_remove_style(cpuArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(cpuArc, LV_OBJ_FLAG_CLICKABLE);

    char cpuBuf[16];
    snprintf(cpuBuf, sizeof(cpuBuf), "%d%%", cpuPercent);
    lv_obj_t *cpuPerc = lv_label_create(cpuArc);
    lv_label_set_text(cpuPerc, cpuBuf);
    lv_obj_set_style_text_color(cpuPerc, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(cpuPerc, &lv_font_montserrat_14, 0);
    lv_obj_center(cpuPerc);

    lv_obj_t *cpuSpeed = lv_label_create(cpuCard);
    lv_label_set_text(cpuSpeed, "240MHz");
    lv_obj_set_style_text_color(cpuSpeed, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(cpuSpeed, &lv_font_montserrat_10, 0);
    lv_obj_align(cpuSpeed, LV_ALIGN_BOTTOM_MID, 0, -5);

    // ═══ SESSION INFO ═══
    lv_obj_t *sessionCard = lv_obj_create(card);
    lv_obj_set_size(sessionCard, LCD_WIDTH - 30, 70);
    lv_obj_align(sessionCard, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_set_style_bg_color(sessionCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(sessionCard, 15, 0);
    lv_obj_set_style_border_width(sessionCard, 0, 0);
    disableAllScrolling(sessionCard);

    unsigned long sessionMins = (millis() - batteryStats.sessionStartMs) / 60000;
    char sessionBuf[32];
    snprintf(sessionBuf, sizeof(sessionBuf), "Session: %lu min", sessionMins);
    lv_obj_t *sessionLabel = lv_label_create(sessionCard);
    lv_label_set_text(sessionLabel, sessionBuf);
    lv_obj_set_style_text_color(sessionLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(sessionLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(sessionLabel, LV_ALIGN_TOP_MID, 0, 12);

    char drainBuf[32];
    snprintf(drainBuf, sizeof(drainBuf), "Drain: %.1f%%/hr", batteryStats.avgDrainPerHour);
    lv_obj_t *drainLabel = lv_label_create(sessionCard);
    lv_label_set_text(drainLabel, drainBuf);
    lv_obj_set_style_text_color(drainLabel, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(drainLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(drainLabel, LV_ALIGN_BOTTOM_MID, 0, -12);

    // ═══ MEMORY BREAKDOWN ═══
    lv_obj_t *memCard = lv_obj_create(card);
    lv_obj_set_size(memCard, LCD_WIDTH - 30, 80);
    lv_obj_align(memCard, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(memCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(memCard, 15, 0);
    lv_obj_set_style_border_width(memCard, 0, 0);
    disableAllScrolling(memCard);

    lv_obj_t *memTitle = lv_label_create(memCard);
    lv_label_set_text(memTitle, "MEMORY BREAKDOWN");
    lv_obj_set_style_text_color(memTitle, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(memTitle, &lv_font_montserrat_10, 0);
    lv_obj_align(memTitle, LV_ALIGN_TOP_MID, 0, 8);

    // Progress bar showing memory usage
    lv_obj_t *memBar = lv_bar_create(memCard);
    lv_obj_set_size(memBar, LCD_WIDTH - 70, 12);
    lv_obj_align(memBar, LV_ALIGN_CENTER, 0, 5);
    lv_bar_set_range(memBar, 0, 100);
    lv_bar_set_value(memBar, ramPercent, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(memBar, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(memBar, lv_color_hex(0x5E5CE6), LV_PART_INDICATOR);
    lv_obj_set_style_radius(memBar, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(memBar, 6, LV_PART_INDICATOR);

    char memInfoBuf[48];
    snprintf(memInfoBuf, sizeof(memInfoBuf), "Used: %luKB | Total: %luKB", (totalRAM - freeRAM) / 1024, totalRAM / 1024);
    lv_obj_t *memInfo = lv_label_create(memCard);
    lv_label_set_text(memInfo, memInfoBuf);
    lv_obj_set_style_text_color(memInfo, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(memInfo, &lv_font_montserrat_10, 0);
    lv_obj_align(memInfo, LV_ALIGN_BOTTOM_MID, 0, -8);
}

// ═══════════════════════════════════════════════════════════════════════════
// SD CARD HEALTH CARD (ENHANCED FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void updateSDCardHealth() {
    if (hasSD) {
        sdHealth.totalBytes = SD_MMC.totalBytes();
        sdHealth.usedBytes = SD_MMC.usedBytes();
        sdHealth.freeBytes = sdHealth.totalBytes - sdHealth.usedBytes;
        sdHealth.usedPercent = (sdHealth.usedBytes * 100.0) / sdHealth.totalBytes;
        sdHealth.mounted = true;
        sdHealth.healthy = (sdHealth.freeBytes > 1024 * 1024);
    } else {
        sdHealth.mounted = false;
        sdHealth.healthy = false;
    }
}

void createSDCardHealthCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "SD CARD");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    updateSDCardHealth();

    if (sdHealth.mounted) {
        // Status badge
        lv_obj_t *statusBadge = lv_obj_create(card);
        lv_obj_set_size(statusBadge, 100, 28);
        lv_obj_align(statusBadge, LV_ALIGN_TOP_MID, 0, 45);
        lv_obj_set_style_bg_color(statusBadge, sdHealth.healthy ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_radius(statusBadge, 14, 0);
        lv_obj_set_style_border_width(statusBadge, 0, 0);
        disableAllScrolling(statusBadge);

        lv_obj_t *statusText = lv_label_create(statusBadge);
        lv_label_set_text(statusText, sdHealth.healthy ? "HEALTHY" : "LOW SPACE");
        lv_obj_set_style_text_color(statusText, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(statusText, &lv_font_montserrat_12, 0);
        lv_obj_center(statusText);

        // Storage gauge
        lv_obj_t *storageCard = lv_obj_create(card);
        lv_obj_set_size(storageCard, LCD_WIDTH - 40, 110);
        lv_obj_align(storageCard, LV_ALIGN_TOP_MID, 0, 85);
        lv_obj_set_style_bg_color(storageCard, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(storageCard, 15, 0);
        lv_obj_set_style_border_width(storageCard, 0, 0);
        disableAllScrolling(storageCard);

        // Arc gauge for storage
        lv_obj_t *storageArc = lv_arc_create(storageCard);
        lv_obj_set_size(storageArc, 80, 80);
        lv_obj_align(storageArc, LV_ALIGN_LEFT_MID, 15, 0);
        lv_arc_set_rotation(storageArc, 135);
        lv_arc_set_bg_angles(storageArc, 0, 270);
        lv_arc_set_range(storageArc, 0, 100);
        lv_arc_set_value(storageArc, (int)sdHealth.usedPercent);
        lv_obj_set_style_arc_width(storageArc, 10, LV_PART_MAIN);
        lv_obj_set_style_arc_width(storageArc, 10, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(storageArc, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
        lv_obj_set_style_arc_color(storageArc, lv_color_hex(0xFF9F0A), LV_PART_INDICATOR);
        lv_obj_remove_style(storageArc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(storageArc, LV_OBJ_FLAG_CLICKABLE);

        char percBuf[8];
        snprintf(percBuf, sizeof(percBuf), "%.0f%%", sdHealth.usedPercent);
        lv_obj_t *percLabel = lv_label_create(storageArc);
        lv_label_set_text(percLabel, percBuf);
        lv_obj_set_style_text_color(percLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(percLabel, &lv_font_montserrat_16, 0);
        lv_obj_center(percLabel);

        // Storage details
        char totalBuf[24];
        snprintf(totalBuf, sizeof(totalBuf), "Total: %llu MB", sdHealth.totalBytes / (1024 * 1024));
        lv_obj_t *totalLabel = lv_label_create(storageCard);
        lv_label_set_text(totalLabel, totalBuf);
        lv_obj_set_style_text_color(totalLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(totalLabel, &lv_font_montserrat_14, 0);
        lv_obj_align(totalLabel, LV_ALIGN_RIGHT_MID, -15, -25);

        char usedBuf[24];
        snprintf(usedBuf, sizeof(usedBuf), "Used: %llu MB", sdHealth.usedBytes / (1024 * 1024));
        lv_obj_t *usedLabel = lv_label_create(storageCard);
        lv_label_set_text(usedLabel, usedBuf);
        lv_obj_set_style_text_color(usedLabel, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(usedLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(usedLabel, LV_ALIGN_RIGHT_MID, -15, 0);

        char freeBuf[24];
        snprintf(freeBuf, sizeof(freeBuf), "Free: %llu MB", sdHealth.freeBytes / (1024 * 1024));
        lv_obj_t *freeLabel = lv_label_create(storageCard);
        lv_label_set_text(freeLabel, freeBuf);
        lv_obj_set_style_text_color(freeLabel, lv_color_hex(0x30D158), 0);
        lv_obj_set_style_text_font(freeLabel, &lv_font_montserrat_12, 0);
        lv_obj_align(freeLabel, LV_ALIGN_RIGHT_MID, -15, 25);

        // Backup info card
        lv_obj_t *backupCard = lv_obj_create(card);
        lv_obj_set_size(backupCard, LCD_WIDTH - 40, 70);
        lv_obj_align(backupCard, LV_ALIGN_TOP_MID, 0, 205);
        lv_obj_set_style_bg_color(backupCard, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(backupCard, 15, 0);
        lv_obj_set_style_border_width(backupCard, 0, 0);
        disableAllScrolling(backupCard);

        lv_obj_t *backupTitle = lv_label_create(backupCard);
        lv_label_set_text(backupTitle, "AUTO BACKUP");
        lv_obj_set_style_text_color(backupTitle, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(backupTitle, &lv_font_montserrat_10, 0);
        lv_obj_align(backupTitle, LV_ALIGN_TOP_LEFT, 15, 10);

        lv_obj_t *backupStatus = lv_label_create(backupCard);
        lv_label_set_text(backupStatus, autoBackupEnabled ? "ENABLED" : "DISABLED");
        lv_obj_set_style_text_color(backupStatus, autoBackupEnabled ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_text_font(backupStatus, &lv_font_montserrat_14, 0);
        lv_obj_align(backupStatus, LV_ALIGN_LEFT_MID, 15, 8);

        char countBuf[24];
        snprintf(countBuf, sizeof(countBuf), "Backups: %d", totalBackups);
        lv_obj_t *countLabel = lv_label_create(backupCard);
        lv_label_set_text(countLabel, countBuf);
        lv_obj_set_style_text_color(countLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(countLabel, &lv_font_montserrat_14, 0);
        lv_obj_align(countLabel, LV_ALIGN_RIGHT_MID, -15, 0);

        // Card type info
        lv_obj_t *typeCard = lv_obj_create(card);
        lv_obj_set_size(typeCard, LCD_WIDTH - 40, 45);
        lv_obj_align(typeCard, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(typeCard, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_radius(typeCard, 12, 0);
        lv_obj_set_style_border_width(typeCard, 0, 0);
        disableAllScrolling(typeCard);

        char typeBuf[32];
        snprintf(typeBuf, sizeof(typeBuf), "Type: %s | %llu MB", sdCardType.c_str(), sdCardSizeMB);
        lv_obj_t *typeLabel = lv_label_create(typeCard);
        lv_label_set_text(typeLabel, typeBuf);
        lv_obj_set_style_text_color(typeLabel, lv_color_hex(0x636366), 0);
        lv_obj_set_style_text_font(typeLabel, &lv_font_montserrat_12, 0);
        lv_obj_center(typeLabel);

    } else {
        // No SD card
        lv_obj_t *noSDIcon = lv_label_create(card);
        lv_label_set_text(noSDIcon, LV_SYMBOL_SD_CARD);
        lv_obj_set_style_text_color(noSDIcon, lv_color_hex(0xFF453A), 0);
        lv_obj_set_style_text_font(noSDIcon, &lv_font_montserrat_48, 0);
        lv_obj_align(noSDIcon, LV_ALIGN_CENTER, 0, -30);

        lv_obj_t *noSDLabel = lv_label_create(card);
        lv_label_set_text(noSDLabel, "NO SD CARD");
        lv_obj_set_style_text_color(noSDLabel, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(noSDLabel, &lv_font_montserrat_20, 0);
        lv_obj_align(noSDLabel, LV_ALIGN_CENTER, 0, 30);

        lv_obj_t *insertLabel = lv_label_create(card);
        lv_label_set_text(insertLabel, "Insert SD card to enable\nbackups and storage");
        lv_obj_set_style_text_color(insertLabel, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(insertLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(insertLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(insertLabel, LV_ALIGN_BOTTOM_MID, 0, -60);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// POWER STATS CARD - DISPLAYS LOGGED POWER CONSUMPTION DATA
// ═══════════════════════════════════════════════════════════════════════════
void createPowerStatsCard() {
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title with power icon
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_CHARGE " POWER STATS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    // Calculate current session stats
    uint32_t sessionMs = millis() - currentPowerSession.sessionStartMs;
    float sessionHours = sessionMs / 3600000.0;
    float drainPercent = currentPowerSession.startBatteryPercent - batteryPercent;
    float drainRatePerHour = (sessionHours > 0.1) ? (drainPercent / sessionHours) : 0;
    float remainingHours = (drainRatePerHour > 0.5) ? (batteryPercent / drainRatePerHour) : 99;

    // Current drain rate gauge
    lv_obj_t *gaugeCard = lv_obj_create(card);
    lv_obj_set_size(gaugeCard, LCD_WIDTH - 30, 95);
    lv_obj_align(gaugeCard, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(gaugeCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(gaugeCard, 15, 0);
    lv_obj_set_style_border_width(gaugeCard, 0, 0);
    disableAllScrolling(gaugeCard);

    // Drain rate arc
    lv_obj_t *drainArc = lv_arc_create(gaugeCard);
    lv_obj_set_size(drainArc, 70, 70);
    lv_obj_align(drainArc, LV_ALIGN_LEFT_MID, 10, 0);
    lv_arc_set_rotation(drainArc, 135);
    lv_arc_set_bg_angles(drainArc, 0, 270);
    lv_arc_set_range(drainArc, 0, 50);  // 0-50%/hour range
    int drainVal = (drainRatePerHour > 50) ? 50 : (int)drainRatePerHour;
    lv_arc_set_value(drainArc, drainVal);
    lv_obj_set_style_arc_width(drainArc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(drainArc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(drainArc, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    
    // Color based on drain rate (green = good, orange = moderate, red = high)
    uint32_t drainColor = (drainRatePerHour < 10) ? 0x30D158 : 
                          (drainRatePerHour < 20) ? 0xFF9F0A : 0xFF453A;
    lv_obj_set_style_arc_color(drainArc, lv_color_hex(drainColor), LV_PART_INDICATOR);
    lv_obj_remove_style(drainArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(drainArc, LV_OBJ_FLAG_CLICKABLE);

    // Drain rate value in center
    char drainBuf[16];
    snprintf(drainBuf, sizeof(drainBuf), "%.1f%%", drainRatePerHour);
    lv_obj_t *drainLabel = lv_label_create(drainArc);
    lv_label_set_text(drainLabel, drainBuf);
    lv_obj_set_style_text_color(drainLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(drainLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(drainLabel);

    lv_obj_t *perHrLabel = lv_label_create(drainArc);
    lv_label_set_text(perHrLabel, "/hr");
    lv_obj_set_style_text_color(perHrLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(perHrLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(perHrLabel, LV_ALIGN_BOTTOM_MID, 0, -5);

    // Stats on the right
    lv_obj_t *statsTitle = lv_label_create(gaugeCard);
    lv_label_set_text(statsTitle, "CURRENT SESSION");
    lv_obj_set_style_text_color(statsTitle, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(statsTitle, &lv_font_montserrat_10, 0);
    lv_obj_align(statsTitle, LV_ALIGN_TOP_RIGHT, -10, 8);

    char sessionBuf[24];
    snprintf(sessionBuf, sizeof(sessionBuf), "Runtime: %.1fh", sessionHours);
    lv_obj_t *sessionLabel = lv_label_create(gaugeCard);
    lv_label_set_text(sessionLabel, sessionBuf);
    lv_obj_set_style_text_color(sessionLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(sessionLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(sessionLabel, LV_ALIGN_TOP_RIGHT, -10, 28);

    char usedBuf[24];
    snprintf(usedBuf, sizeof(usedBuf), "Used: %.1f%%", drainPercent);
    lv_obj_t *usedLabel = lv_label_create(gaugeCard);
    lv_label_set_text(usedLabel, usedBuf);
    lv_obj_set_style_text_color(usedLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(usedLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(usedLabel, LV_ALIGN_TOP_RIGHT, -10, 48);

    char estBuf[24];
    if (remainingHours > 48) {
        snprintf(estBuf, sizeof(estBuf), "Est: 48h+");
    } else {
        snprintf(estBuf, sizeof(estBuf), "Est: %.1fh left", remainingHours);
    }
    lv_obj_t *estLabel = lv_label_create(gaugeCard);
    lv_label_set_text(estLabel, estBuf);
    lv_obj_set_style_text_color(estLabel, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(estLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(estLabel, LV_ALIGN_TOP_RIGHT, -10, 68);

    // Mode & logging status card
    lv_obj_t *modeCard = lv_obj_create(card);
    lv_obj_set_size(modeCard, LCD_WIDTH - 30, 55);
    lv_obj_align(modeCard, LV_ALIGN_TOP_MID, 0, 145);
    lv_obj_set_style_bg_color(modeCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(modeCard, 12, 0);
    lv_obj_set_style_border_width(modeCard, 0, 0);
    disableAllScrolling(modeCard);

    lv_obj_t *modeTitle = lv_label_create(modeCard);
    lv_label_set_text(modeTitle, "Power Saver Mode:");
    lv_obj_set_style_text_color(modeTitle, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(modeTitle, &lv_font_montserrat_10, 0);
    lv_obj_align(modeTitle, LV_ALIGN_TOP_LEFT, 12, 8);

    lv_obj_t *modeVal = lv_label_create(modeCard);
    lv_label_set_text(modeVal, saverModes[batterySaverLevel].name);
    lv_obj_set_style_text_color(modeVal, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(modeVal, &lv_font_montserrat_16, 0);
    lv_obj_align(modeVal, LV_ALIGN_LEFT_MID, 12, 8);

    // CPU frequency
    char cpuBuf[16];
    snprintf(cpuBuf, sizeof(cpuBuf), "%dMHz", getCpuFrequencyMhz());
    lv_obj_t *cpuLabel = lv_label_create(modeCard);
    lv_label_set_text(cpuLabel, cpuBuf);
    lv_obj_set_style_text_color(cpuLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(cpuLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(cpuLabel, LV_ALIGN_RIGHT_MID, -12, 0);

    // Logging status card
    lv_obj_t *logCard = lv_obj_create(card);
    lv_obj_set_size(logCard, LCD_WIDTH - 30, 55);
    lv_obj_align(logCard, LV_ALIGN_TOP_MID, 0, 210);
    lv_obj_set_style_bg_color(logCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(logCard, 12, 0);
    lv_obj_set_style_border_width(logCard, 0, 0);
    disableAllScrolling(logCard);

    lv_obj_t *logIcon = lv_label_create(logCard);
    lv_label_set_text(logIcon, LV_SYMBOL_FILE);
    lv_obj_set_style_text_color(logIcon, powerLoggingEnabled && hasSD ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_text_font(logIcon, &lv_font_montserrat_20, 0);
    lv_obj_align(logIcon, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t *logTitle = lv_label_create(logCard);
    lv_label_set_text(logTitle, "SD Card Logging");
    lv_obj_set_style_text_color(logTitle, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(logTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(logTitle, LV_ALIGN_LEFT_MID, 45, -10);

    lv_obj_t *logStatus = lv_label_create(logCard);
    if (!hasSD) {
        lv_label_set_text(logStatus, "No SD Card");
        lv_obj_set_style_text_color(logStatus, lv_color_hex(0xFF453A), 0);
    } else if (powerLoggingEnabled) {
        lv_label_set_text(logStatus, "Active - /WATCH/POWER_LOGS/");
        lv_obj_set_style_text_color(logStatus, lv_color_hex(0x30D158), 0);
    } else {
        lv_label_set_text(logStatus, "Disabled");
        lv_obj_set_style_text_color(logStatus, lv_color_hex(0x8E8E93), 0);
    }
    lv_obj_set_style_text_font(logStatus, &lv_font_montserrat_10, 0);
    lv_obj_align(logStatus, LV_ALIGN_LEFT_MID, 45, 8);

    // Mode changes count
    char changesBuf[24];
    snprintf(changesBuf, sizeof(changesBuf), "%lu changes", currentPowerSession.modeChanges);
    lv_obj_t *changesLabel = lv_label_create(logCard);
    lv_label_set_text(changesLabel, changesBuf);
    lv_obj_set_style_text_color(changesLabel, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(changesLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(changesLabel, LV_ALIGN_RIGHT_MID, -12, 0);

    // Efficiency rating bar
    lv_obj_t *effCard = lv_obj_create(card);
    lv_obj_set_size(effCard, LCD_WIDTH - 30, 40);
    lv_obj_align(effCard, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_bg_color(effCard, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_radius(effCard, 10, 0);
    lv_obj_set_style_border_width(effCard, 0, 0);
    disableAllScrolling(effCard);

    const char* effRating;
    uint32_t effColor;
    if (drainRatePerHour < 5) { effRating = "EXCELLENT"; effColor = 0x30D158; }
    else if (drainRatePerHour < 10) { effRating = "GOOD"; effColor = 0x30D158; }
    else if (drainRatePerHour < 20) { effRating = "MODERATE"; effColor = 0xFF9F0A; }
    else if (drainRatePerHour < 30) { effRating = "HIGH USAGE"; effColor = 0xFF9F0A; }
    else { effRating = "VERY HIGH"; effColor = 0xFF453A; }

    lv_obj_t *effLabel = lv_label_create(effCard);
    char effBuf[32];
    snprintf(effBuf, sizeof(effBuf), "Efficiency: %s", effRating);
    lv_label_set_text(effLabel, effBuf);
    lv_obj_set_style_text_color(effLabel, lv_color_hex(effColor), 0);
    lv_obj_set_style_text_font(effLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(effLabel);
}

// ═══════════════════════════════════════════════════════════════════════════
// ABOUT/SOFTWARE INFO CARD - WIGET OS INFO
// ═══════════════════════════════════════════════════════════════════════════
void createAboutCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    // Deep dark background with subtle gradient
    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0A0A0F), 0);
    lv_obj_set_style_bg_grad_color(card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Ambient glow effect at top (gradient accent)
    lv_obj_t *glowTop = lv_obj_create(card);
    lv_obj_set_size(glowTop, LCD_WIDTH, 120);
    lv_obj_align(glowTop, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(glowTop, theme->accent, 0);
    lv_obj_set_style_bg_opa(glowTop, LV_OPA_10, 0);
    lv_obj_set_style_radius(glowTop, 0, 0);
    lv_obj_set_style_border_width(glowTop, 0, 0);
    disableAllScrolling(glowTop);

    // Main glass-morphism card container
    lv_obj_t *glassCard = lv_obj_create(card);
    lv_obj_set_size(glassCard, LCD_WIDTH - 28, LCD_HEIGHT - 50);
    lv_obj_align(glassCard, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_bg_color(glassCard, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(glassCard, LV_OPA_10, 0);  // Glass transparency
    lv_obj_set_style_radius(glassCard, 28, 0);
    lv_obj_set_style_border_width(glassCard, 1, 0);
    lv_obj_set_style_border_color(glassCard, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(glassCard, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(glassCard, 40, 0);
    lv_obj_set_style_shadow_color(glassCard, theme->accent, 0);
    lv_obj_set_style_shadow_opa(glassCard, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(glassCard, 2, 0);
    disableAllScrolling(glassCard);

    // Inner glass highlight (top edge glow)
    lv_obj_t *innerGlow = lv_obj_create(glassCard);
    lv_obj_set_size(innerGlow, LCD_WIDTH - 60, 3);
    lv_obj_align(innerGlow, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_bg_color(innerGlow, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(innerGlow, LV_OPA_30, 0);
    lv_obj_set_style_radius(innerGlow, 2, 0);
    lv_obj_set_style_border_width(innerGlow, 0, 0);
    disableAllScrolling(innerGlow);

    // Logo/Icon circle with gradient
    lv_obj_t *logoCircle = lv_obj_create(glassCard);
    lv_obj_set_size(logoCircle, 70, 70);
    lv_obj_align(logoCircle, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(logoCircle, theme->accent, 0);
    lv_obj_set_style_bg_grad_color(logoCircle, theme->secondary, 0);
    lv_obj_set_style_bg_grad_dir(logoCircle, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(logoCircle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(logoCircle, 2, 0);
    lv_obj_set_style_border_color(logoCircle, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(logoCircle, LV_OPA_40, 0);
    lv_obj_set_style_shadow_width(logoCircle, 20, 0);
    lv_obj_set_style_shadow_color(logoCircle, theme->accent, 0);
    lv_obj_set_style_shadow_opa(logoCircle, LV_OPA_50, 0);
    disableAllScrolling(logoCircle);

    // Logo letter "W" for Widget
    lv_obj_t *logoLetter = lv_label_create(logoCircle);
    lv_label_set_text(logoLetter, "W");
    lv_obj_set_style_text_color(logoLetter, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(logoLetter, &lv_font_montserrat_32, 0);
    lv_obj_center(logoLetter);

    // OS Name - WIDGET OS with letter spacing
    lv_obj_t *osNameLabel = lv_label_create(glassCard);
    lv_label_set_text(osNameLabel, "WIDGET OS");
    lv_obj_set_style_text_color(osNameLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(osNameLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_letter_space(osNameLabel, 4, 0);
    lv_obj_align(osNameLabel, LV_ALIGN_TOP_MID, 0, 105);

    // Version badge with gradient accent
    lv_obj_t *versionBadge = lv_obj_create(glassCard);
    lv_obj_set_size(versionBadge, 90, 32);
    lv_obj_align(versionBadge, LV_ALIGN_TOP_MID, 0, 140);
    lv_obj_set_style_bg_color(versionBadge, theme->accent, 0);
    lv_obj_set_style_bg_opa(versionBadge, LV_OPA_80, 0);
    lv_obj_set_style_radius(versionBadge, 16, 0);
    lv_obj_set_style_border_width(versionBadge, 0, 0);
    disableAllScrolling(versionBadge);

    lv_obj_t *versionLabel = lv_label_create(versionBadge);
    lv_label_set_text(versionLabel, "v7.2.0");
    lv_obj_set_style_text_color(versionLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(versionLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(versionLabel);

    // Divider with gradient dots
    int dotY = 185;
    for (int i = 0; i < 5; i++) {
        lv_obj_t *dot = lv_obj_create(glassCard);
        lv_obj_set_size(dot, 4, 4);
        lv_obj_align(dot, LV_ALIGN_TOP_MID, -32 + i * 16, dotY);
        lv_obj_set_style_bg_color(dot, theme->accent, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_40 + i * 15, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        disableAllScrolling(dot);
    }

    // Info rows container
    lv_obj_t *infoContainer = lv_obj_create(glassCard);
    lv_obj_set_size(infoContainer, LCD_WIDTH - 60, 90);
    lv_obj_align(infoContainer, LV_ALIGN_TOP_MID, 0, 200);
    lv_obj_set_style_bg_color(infoContainer, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(infoContainer, LV_OPA_30, 0);
    lv_obj_set_style_radius(infoContainer, 16, 0);
    lv_obj_set_style_border_width(infoContainer, 0, 0);
    disableAllScrolling(infoContainer);

    // Device ID row
    lv_obj_t *deviceLabel = lv_label_create(infoContainer);
    lv_label_set_text(deviceLabel, "Device");
    lv_obj_set_style_text_color(deviceLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(deviceLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(deviceLabel, LV_ALIGN_TOP_LEFT, 15, 12);

    lv_obj_t *deviceValue = lv_label_create(infoContainer);
    lv_label_set_text(deviceValue, DEVICE_ID);
    lv_obj_set_style_text_color(deviceValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(deviceValue, &lv_font_montserrat_14, 0);
    lv_obj_align(deviceValue, LV_ALIGN_TOP_RIGHT, -15, 10);

    // Screen row
    lv_obj_t *screenLabel = lv_label_create(infoContainer);
    lv_label_set_text(screenLabel, "Display");
    lv_obj_set_style_text_color(screenLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(screenLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(screenLabel, LV_ALIGN_TOP_LEFT, 15, 38);

    lv_obj_t *screenValue = lv_label_create(infoContainer);
    lv_label_set_text(screenValue, "2.06\" AMOLED");
    lv_obj_set_style_text_color(screenValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(screenValue, &lv_font_montserrat_14, 0);
    lv_obj_align(screenValue, LV_ALIGN_TOP_RIGHT, -15, 36);

    // Build row
    lv_obj_t *buildLabel = lv_label_create(infoContainer);
    lv_label_set_text(buildLabel, "Build");
    lv_obj_set_style_text_color(buildLabel, lv_color_hex(0x8E8E93), 0);
    lv_obj_set_style_text_font(buildLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(buildLabel, LV_ALIGN_TOP_LEFT, 15, 64);

    lv_obj_t *buildValue = lv_label_create(infoContainer);
    lv_label_set_text(buildValue, "Nike Enhanced");
    lv_obj_set_style_text_color(buildValue, theme->accent, 0);
    lv_obj_set_style_text_font(buildValue, &lv_font_montserrat_14, 0);
    lv_obj_align(buildValue, LV_ALIGN_TOP_RIGHT, -15, 62);

    // Footer credit with glow
    lv_obj_t *creditLabel = lv_label_create(glassCard);
    lv_label_set_text(creditLabel, "Fusion Labs 2026");
    lv_obj_set_style_text_color(creditLabel, theme->accent, 0);
    lv_obj_set_style_text_font(creditLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_letter_space(creditLabel, 2, 0);
    lv_obj_align(creditLabel, LV_ALIGN_BOTTOM_MID, 0, -15);

    // Bottom accent line
    lv_obj_t *accentLine = lv_obj_create(card);
    lv_obj_set_size(accentLine, 60, 4);
    lv_obj_align(accentLine, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(accentLine, theme->accent, 0);
    lv_obj_set_style_radius(accentLine, 2, 0);
    lv_obj_set_style_border_width(accentLine, 0, 0);
    disableAllScrolling(accentLine);
}

// ═══════════════════════════════════════════════════════════════════════════
// CREATE BATTERY SAVER MODES CARD (NEW!)
// ═══════════════════════════════════════════════════════════════════════════
void createBatterySaverCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, theme->color1, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Title with power icon
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_CHARGE " POWER SAVER");
    lv_obj_set_style_text_color(title, lv_color_hex(0x30D158), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Subtitle showing power savings
    lv_obj_t *subtitle = lv_label_create(card);
    lv_label_set_text(subtitle, "Tap to select power mode");
    lv_obj_set_style_text_color(subtitle, theme->secondary, 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_10, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 32);

    // 3 MODE OPTIONS ONLY: Off, Medium, Extreme
    const char* modeNames[] = {"NORMAL", "MEDIUM", "EXTREME"};
    const char* modeDescs[] = {
        "240MHz | Full sync | ~3hrs",      // Normal/Off
        "80MHz | Slow poll | ~5hrs",       // Medium
        "80MHz | No WiFi | ~10+hrs"        // Extreme with light sleep
    };
    const char* savingsInfo[] = {
        "Full performance",
        "~50% power saving", 
        "~75% power saving"
    };
    uint32_t modeColors[] = {0x8E8E93, 0xFF9F0A, 0xFF453A};

    int yStart = 55;
    int rowHeight = 75;  // Larger rows for 3 options

    for (int i = 0; i < 3; i++) {
        lv_obj_t *modeRow = lv_obj_create(card);
        lv_obj_set_size(modeRow, LCD_WIDTH - 24, 65);
        lv_obj_align(modeRow, LV_ALIGN_TOP_MID, 0, yStart + i * rowHeight);
        
        // Highlight selected mode
        bool isSelected = (i == (int)batterySaverLevel);
        if (isSelected) {
            lv_obj_set_style_bg_color(modeRow, lv_color_hex(modeColors[i]), 0);
            lv_obj_set_style_bg_opa(modeRow, 60, 0);
            lv_obj_set_style_border_width(modeRow, 2, 0);
            lv_obj_set_style_border_color(modeRow, lv_color_hex(modeColors[i]), 0);
        } else {
            lv_obj_set_style_bg_color(modeRow, theme->color2, 0);
            lv_obj_set_style_border_width(modeRow, 1, 0);
            lv_obj_set_style_border_color(modeRow, theme->secondary, 0);
            lv_obj_set_style_border_opa(modeRow, 50, 0);
        }
        lv_obj_set_style_radius(modeRow, 12, 0);
        disableAllScrolling(modeRow);

        // Mode name with color indicator
        lv_obj_t *modeName = lv_label_create(modeRow);
        lv_label_set_text(modeName, modeNames[i]);
        lv_obj_set_style_text_color(modeName, isSelected ? theme->text : lv_color_hex(modeColors[i]), 0);
        lv_obj_set_style_text_font(modeName, &lv_font_montserrat_14, 0);
        lv_obj_align(modeName, LV_ALIGN_LEFT_MID, 12, -12);

        // Technical details
        lv_obj_t *modeDesc = lv_label_create(modeRow);
        lv_label_set_text(modeDesc, modeDescs[i]);
        lv_obj_set_style_text_color(modeDesc, theme->secondary, 0);
        lv_obj_set_style_text_font(modeDesc, &lv_font_montserrat_10, 0);
        lv_obj_align(modeDesc, LV_ALIGN_LEFT_MID, 12, 8);

        // Power savings percentage on right
        lv_obj_t *savingsLabel = lv_label_create(modeRow);
        lv_label_set_text(savingsLabel, savingsInfo[i]);
        lv_obj_set_style_text_color(savingsLabel, lv_color_hex(modeColors[i]), 0);
        lv_obj_set_style_text_font(savingsLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(savingsLabel, LV_ALIGN_RIGHT_MID, -10, -10);

        // Checkmark for selected
        if (isSelected) {
            lv_obj_t *check = lv_label_create(modeRow);
            lv_label_set_text(check, LV_SYMBOL_OK);
            lv_obj_set_style_text_color(check, theme->text, 0);
            lv_obj_set_style_text_font(check, &lv_font_montserrat_16, 0);
            lv_obj_align(check, LV_ALIGN_RIGHT_MID, -10, 10);
        }
    }

    // Current CPU frequency display
    lv_obj_t *cpuLabel = lv_label_create(card);
    char cpuBuf[48];
    snprintf(cpuBuf, sizeof(cpuBuf), "CPU: %d MHz | Poll: %ldms", 
             getCpuFrequencyMhz(), sensorPollInterval);
    lv_label_set_text(cpuLabel, cpuBuf);
    lv_obj_set_style_text_color(cpuLabel, theme->accent, 0);
    lv_obj_set_style_text_font(cpuLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(cpuLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// ═══════════════════════════════════════════════════════════════════════════
// TXT FILE FUNCTIONS (NEW!)
// ═══════════════════════════════════════════════════════════════════════════
void loadTxtFileList() {
    numTxtFiles = 0;
    
    if (!hasSD) return;
    
    // Create TXT folder if it doesn't exist
    if (!SD_MMC.exists(SD_TXT_PATH)) {
        SD_MMC.mkdir(SD_TXT_PATH);
        USBSerial.println("[SD] Created /WATCH/TXT folder");
        
        // Create sample file
        createSampleTxtFile();
    }
    
    File dir = SD_MMC.open(SD_TXT_PATH);
    if (!dir || !dir.isDirectory()) {
        USBSerial.println("[SD] Failed to open TXT directory");
        return;
    }
    
    File file = dir.openNextFile();
    while (file && numTxtFiles < MAX_TXT_FILES) {
        String name = file.name();
        if (name.endsWith(".txt") || name.endsWith(".TXT")) {
            strncpy(txtFiles[numTxtFiles].filename, file.name(), sizeof(txtFiles[numTxtFiles].filename) - 1);
            txtFiles[numTxtFiles].fileSize = file.size();
            numTxtFiles++;
        }
        file = dir.openNextFile();
    }
    dir.close();
    
    USBSerial.printf("[TXT] Found %d text files\n", numTxtFiles);
}

void createSampleTxtFile() {
    File sample = SD_MMC.open(SD_SAMPLE_TXT, FILE_WRITE);
    if (sample) {
        sample.println("===================================");
        sample.println("  WIGET OS - Sample Text File");
        sample.println("===================================");
        sample.println("");
        sample.println("Welcome to the Text File Reader!");
        sample.println("");
        sample.println("This is a sample text file that was");
        sample.println("automatically created by Widget OS.");
        sample.println("");
        sample.println("You can add your own .txt files to:");
        sample.println("  /WATCH/TXT/");
        sample.println("");
        sample.println("Features:");
        sample.println("- Tap a file to view it");
        sample.println("- Tap top to scroll up");
        sample.println("- Tap bottom to scroll down");
        sample.println("- Tap header bar to go back");
        sample.println("");
        sample.println("This file reader supports plain text");
        sample.println("files up to 4KB in size.");
        sample.println("");
        sample.println("===================================");
        sample.println("  Fusion Labs 2026");
        sample.println("===================================");
        sample.close();
        USBSerial.println("[SD] Created sample.txt");
    }
}

void loadTxtFileContent(int index) {
    if (index < 0 || index >= numTxtFiles || !hasSD) {
        txtFileLoaded = false;
        return;
    }
    
    selectedTxtFile = index;
    txtScrollOffset = 0;
    
    // Allocate buffer from PSRAM if not already allocated (for large files!)
    if (txtFileContent == NULL) {
        txtFileContent = (char*)heap_caps_malloc(TXT_FILE_MAX_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (txtFileContent == NULL) {
            // Fallback to smaller internal RAM buffer
            txtFileContent = (char*)malloc(8192);  // 8KB fallback
            if (txtFileContent == NULL) {
                USBSerial.println("[TXT] Failed to allocate buffer!");
                txtFileLoaded = false;
                return;
            }
            USBSerial.println("[TXT] Using 8KB internal buffer (PSRAM unavailable)");
        } else {
            USBSerial.println("[TXT] Allocated 64KB PSRAM buffer");
        }
    }
    
    String fullPath = String(SD_TXT_PATH) + "/" + txtFiles[index].filename;
    File file = SD_MMC.open(fullPath.c_str(), FILE_READ);
    
    if (file) {
        int maxRead = min((int)file.size(), TXT_FILE_MAX_SIZE - 1);
        int bytesRead = file.readBytes(txtFileContent, maxRead);
        txtFileContent[bytesRead] = 0;  // null terminator
        file.close();
        txtFileLoaded = true;
        USBSerial.printf("[TXT] Loaded: %s (%d bytes)\n", txtFiles[index].filename, bytesRead);
    } else {
        txtFileLoaded = false;
        USBSerial.println("[TXT] Failed to open file");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// TIME BACKUP FOR WATCHDOG RESET
// Saves current time + offset to SD before reset, restores when WiFi fails
// FIX: Changed from 5 to 10 seconds to account for actual reset duration
// ═══════════════════════════════════════════════════════════════════════════

#define WATCHDOG_RESET_SECONDS 10  // FIX: Increased from 5 to 10 seconds for accurate time restore

void saveTimeBackup() {
    if (!hasSD || !hasRTC) return;
    
    // Create TIME folder if needed
    if (!SD_MMC.exists(SD_TIME_BACKUP_PATH)) {
        SD_MMC.mkdir(SD_TIME_BACKUP_PATH);
    }
    
    RTC_DateTime dt = rtc.getDateTime();
    
    // Add a few seconds for the watchdog reset duration
    int newSecond = dt.getSecond() + WATCHDOG_RESET_SECONDS;
    int newMinute = dt.getMinute();
    int newHour = dt.getHour();
    int newDay = dt.getDay();
    int newMonth = dt.getMonth();
    int newYear = dt.getYear();
    
    // Handle overflow
    if (newSecond >= 60) {
        newSecond -= 60;
        newMinute++;
    }
    if (newMinute >= 60) {
        newMinute -= 60;
        newHour++;
    }
    if (newHour >= 24) {
        newHour -= 24;
        newDay++;
    }
    // Simple month overflow (not handling all months perfectly)
    if (newDay > 28) {
        // Just reset to 1 for simplicity - user can adjust
        newDay = 1;
        newMonth++;
    }
    if (newMonth > 12) {
        newMonth = 1;
        newYear++;
    }
    
    // Write to backup file
    File f = SD_MMC.open(SD_TIME_BACKUP_FILE, FILE_WRITE);
    if (f) {
        f.printf("%04d\n%02d\n%02d\n%02d\n%02d\n%02d\n", 
                newYear, newMonth, newDay, newHour, newMinute, newSecond);
        f.close();
        USBSerial.printf("[TIME BACKUP] Saved: %04d-%02d-%02d %02d:%02d:%02d\n",
                        newYear, newMonth, newDay, newHour, newMinute, newSecond);
    }
}

bool hasTimeBackup() {
    return hasSD && SD_MMC.exists(SD_TIME_BACKUP_FILE);
}

void restoreTimeBackup() {
    if (!hasSD || !hasRTC) return;
    if (!SD_MMC.exists(SD_TIME_BACKUP_FILE)) {
        USBSerial.println("[TIME BACKUP] No backup found");
        return;
    }
    
    File f = SD_MMC.open(SD_TIME_BACKUP_FILE, FILE_READ);
    if (f) {
        int year = f.parseInt();
        int month = f.parseInt();
        int day = f.parseInt();
        int hour = f.parseInt();
        int minute = f.parseInt();
        int second = f.parseInt();
        f.close();
        
        // Validate values
        if (year >= 2020 && year <= 2099 && month >= 1 && month <= 12 &&
            day >= 1 && day <= 31 && hour >= 0 && hour <= 23 &&
            minute >= 0 && minute <= 59 && second >= 0 && second <= 59) {
            
            rtc.setDateTime(year, month, day, hour, minute, second);
            USBSerial.printf("[TIME BACKUP] Restored: %04d-%02d-%02d %02d:%02d:%02d\n",
                            year, month, day, hour, minute, second);
            
            // Delete backup after successful restore
            SD_MMC.remove(SD_TIME_BACKUP_FILE);
        } else {
            USBSerial.println("[TIME BACKUP] Invalid backup data");
        }
    }
}

void deleteTimeBackup() {
    if (hasSD && SD_MMC.exists(SD_TIME_BACKUP_FILE)) {
        SD_MMC.remove(SD_TIME_BACKUP_FILE);
        USBSerial.println("[TIME BACKUP] Deleted after WiFi sync");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// POWER CONSUMPTION LOGGING FUNCTIONS (NEW!)
// Logs battery drain, power modes, and usage patterns to SD card
// ═══════════════════════════════════════════════════════════════════════════

void initPowerLogging() {
    if (!hasSD) return;
    
    // Create power logs folder if it doesn't exist
    if (!SD_MMC.exists(SD_POWER_LOGS_PATH)) {
        SD_MMC.mkdir(SD_POWER_LOGS_PATH);
        USBSerial.println("[POWER] Created /WATCH/POWER_LOGS folder");
    }
    
    // Initialize session
    currentPowerSession.sessionStartMs = millis();
    currentPowerSession.startBatteryPercent = batteryPercent;
    currentPowerSession.currentBatteryPercent = batteryPercent;
    currentPowerSession.totalScreenOnMs = 0;
    currentPowerSession.totalScreenOffMs = 0;
    currentPowerSession.modeChanges = 0;
    currentPowerSession.avgDrainRate = 0;
    currentPowerSession.dominantMode = (uint8_t)batterySaverLevel;
    
    // Write session start header
    File logFile = SD_MMC.open(SD_POWER_LOG_CURRENT, FILE_WRITE);
    if (logFile) {
        RTC_DateTime dt = rtc.getDateTime();
        logFile.printf("═══════════════════════════════════════════════════\n");
        logFile.printf("POWER LOG SESSION STARTED\n");
        logFile.printf("Date: %04d-%02d-%02d %02d:%02d:%02d\n", 
                      dt.getYear(), dt.getMonth(), dt.getDay(),
                      dt.getHour(), dt.getMinute(), dt.getSecond());
        logFile.printf("Starting Battery: %d%% (%dmV)\n", batteryPercent, batteryVoltage);
        logFile.printf("Initial Mode: %s\n", saverModes[batterySaverLevel].name);
        logFile.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
        logFile.printf("═══════════════════════════════════════════════════\n\n");
        logFile.printf("TIME       | BAT%% | mV   | MODE    | CPU  | ON/OFF  | DRAIN\n");
        logFile.printf("-----------|------|------|---------|------|---------|-------\n");
        logFile.close();
        USBSerial.println("[POWER] Log session started");
    }
    
    lastPowerLogMs = millis();
    lastPowerSummaryMs = millis();
}

String formatPowerLogEntry(PowerLogEntry &entry) {
    char buf[128];
    int hrs = entry.timestamp / 3600;
    int mins = (entry.timestamp % 3600) / 60;
    int secs = entry.timestamp % 60;
    
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d | %3d%% | %4d | %-7s | %3d  | %4lu/%4lu | %.2f%%/h",
             hrs, mins, secs,
             entry.batteryPercent,
             entry.batteryVoltage,
             saverModes[entry.saverLevel].name,
             entry.cpuFreqMHz,
             entry.screenOnSecs / 60,
             entry.screenOffSecs / 60,
             entry.drainRatePerHour);
    
    return String(buf);
}

void logPowerConsumption() {
    if (!hasSD || !powerLoggingEnabled) return;
    if (millis() - lastPowerLogMs < POWER_LOG_INTERVAL_MS) return;
    
    // Calculate drain rate
    uint32_t elapsedMs = millis() - currentPowerSession.sessionStartMs;
    float elapsedHours = elapsedMs / 3600000.0;
    float drainPercent = currentPowerSession.startBatteryPercent - batteryPercent;
    float drainRatePerHour = (elapsedHours > 0.1) ? (drainPercent / elapsedHours) : 0;
    
    // Create log entry
    PowerLogEntry entry;
    entry.timestamp = elapsedMs / 1000;
    entry.batteryPercent = batteryPercent;
    entry.batteryVoltage = batteryVoltage;
    entry.saverLevel = (uint8_t)batterySaverLevel;
    entry.cpuFreqMHz = getCpuFrequencyMhz();
    entry.screenOnSecs = batteryStats.screenOnTimeMs / 1000;
    entry.screenOffSecs = batteryStats.screenOffTimeMs / 1000;
    entry.drainRatePerHour = drainRatePerHour;
    
    // Update session
    currentPowerSession.currentBatteryPercent = batteryPercent;
    currentPowerSession.avgDrainRate = drainRatePerHour;
    
    // Append to log file
    File logFile = SD_MMC.open(SD_POWER_LOG_CURRENT, FILE_APPEND);
    if (logFile) {
        logFile.println(formatPowerLogEntry(entry));
        logFile.close();
    }
    
    lastPowerLogMs = millis();
    USBSerial.printf("[POWER] Logged: %d%%, %.2f%%/hr drain\n", batteryPercent, drainRatePerHour);
}


void writePowerSummary() {
    if (!hasSD || !powerLoggingEnabled) return;
    if (millis() - lastPowerSummaryMs < POWER_SUMMARY_INTERVAL_MS) return;
    
    uint32_t elapsedMs = millis() - currentPowerSession.sessionStartMs;
    float elapsedHours = elapsedMs / 3600000.0;
    float drainPercent = currentPowerSession.startBatteryPercent - batteryPercent;
    float drainRatePerHour = (elapsedHours > 0.1) ? (drainPercent / elapsedHours) : 0;
    
    // Calculate estimated remaining time
    float remainingHours = (drainRatePerHour > 0.5) ? (batteryPercent / drainRatePerHour) : 99;
    
    // Calculate screen on percentage
    uint32_t totalMs = batteryStats.screenOnTimeMs + batteryStats.screenOffTimeMs;
    float screenOnPercent = (totalMs > 0) ? (batteryStats.screenOnTimeMs * 100.0 / totalMs) : 0;
    
    // Write/update summary file
    File summaryFile = SD_MMC.open(SD_POWER_SUMMARY, FILE_WRITE);
    if (summaryFile) {
        RTC_DateTime dt = rtc.getDateTime();
        
        summaryFile.println("╔═══════════════════════════════════════════════════╗");
        summaryFile.println("║         POWER CONSUMPTION SUMMARY                 ║");
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.printf("║ Last Update: %04d-%02d-%02d %02d:%02d                   ║\n",
                          dt.getYear(), dt.getMonth(), dt.getDay(),
                          dt.getHour(), dt.getMinute());
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.println("║ CURRENT SESSION                                   ║");
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.printf("║ Session Duration:     %.1f hours                   \n", elapsedHours);
        summaryFile.printf("║ Battery Start:        %d%%                         \n", currentPowerSession.startBatteryPercent);
        summaryFile.printf("║ Battery Current:      %d%% (%dmV)                  \n", batteryPercent, batteryVoltage);
        summaryFile.printf("║ Battery Drained:      %.1f%%                       \n", drainPercent);
        summaryFile.printf("║ Drain Rate:           %.2f%%/hour                  \n", drainRatePerHour);
        summaryFile.printf("║ Est. Remaining:       %.1f hours                   \n", remainingHours);
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.println("║ USAGE BREAKDOWN                                   ║");
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.printf("║ Screen ON Time:       %lu min (%.0f%%)             \n", 
                          batteryStats.screenOnTimeMs / 60000, screenOnPercent);
        summaryFile.printf("║ Screen OFF Time:      %lu min                      \n", 
                          batteryStats.screenOffTimeMs / 60000);
        summaryFile.printf("║ Mode Changes:         %lu                          \n", 
                          currentPowerSession.modeChanges);
        summaryFile.printf("║ Current Mode:         %s                          \n", 
                          saverModes[batterySaverLevel].name);
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        summaryFile.println("║ POWER EFFICIENCY                                  ║");
        summaryFile.println("╠═══════════════════════════════════════════════════╣");
        
        // Power efficiency rating
        const char* efficiencyRating;
        if (drainRatePerHour < 5) efficiencyRating = "EXCELLENT - Deep Sleep Working!";
        else if (drainRatePerHour < 10) efficiencyRating = "GOOD - Saver Mode Effective";
        else if (drainRatePerHour < 20) efficiencyRating = "MODERATE - Normal Usage";
        else if (drainRatePerHour < 30) efficiencyRating = "HIGH - Consider Saver Mode";
        else efficiencyRating = "VERY HIGH - Enable Extreme Saver";
        
        summaryFile.printf("║ Rating: %s                                        \n", efficiencyRating);
        summaryFile.printf("║ CPU Frequency:        %d MHz                       \n", getCpuFrequencyMhz());
        summaryFile.printf("║ WiFi Status:          %s                          \n", 
                          wifiConnected ? "Connected" : "Disconnected");
        summaryFile.println("╚═══════════════════════════════════════════════════╝");
        summaryFile.println("");
        summaryFile.println("TIP: Check current.txt for detailed timeline logs");
        summaryFile.println("TIP: Lower drain rate = better battery optimization");
        summaryFile.close();
        
        USBSerial.println("[POWER] Summary updated");
    }
    
    lastPowerSummaryMs = millis();
}

// ═══════════════════════════════════════════════════════════════════════════
// POWER LOG ROTATION & DAILY ARCHIVING
// ═══════════════════════════════════════════════════════════════════════════

// Rotate current log file if it exceeds max size
void rotatePowerLogIfNeeded() {
    if (!hasSD || !powerLoggingEnabled) return;
    if (millis() - lastLogRotationCheck < LOG_ROTATION_CHECK_MS) return;
    lastLogRotationCheck = millis();
    
    File logFile = SD_MMC.open(SD_POWER_LOG_CURRENT, FILE_READ);
    if (!logFile) return;
    
    size_t fileSize = logFile.size();
    logFile.close();
    
    if (fileSize > MAX_LOG_FILE_SIZE) {
        // Archive current log with timestamp
        RTC_DateTime dt = rtc.getDateTime();
        char archivePath[64];
        snprintf(archivePath, sizeof(archivePath), 
                "/WATCH/POWER_LOGS/archive_%04d%02d%02d_%02d%02d.txt",
                dt.getYear(), dt.getMonth(), dt.getDay(),
                dt.getHour(), dt.getMinute());
        
        // Rename current to archive
        SD_MMC.rename(SD_POWER_LOG_CURRENT, archivePath);
        USBSerial.printf("[POWER] Log rotated to %s\n", archivePath);
        
        // Start a new log session
        initPowerLogging();
    }
}

// Archive daily power log at midnight
void archiveDailyPowerLog() {
    if (!hasSD || !powerLoggingEnabled) return;
    
    // Skip on first few seconds of boot to avoid WDT issues
    static bool firstRunSkipped = false;
    if (!firstRunSkipped) {
        if (millis() < 10000) return;  // Skip first 10 seconds
        firstRunSkipped = true;
    }
    
    RTC_DateTime dt = rtc.getDateTime();
    uint8_t currentDay = dt.getDay();
    
    // Check if day has changed
    if (lastLogDay != 0 && lastLogDay != currentDay) {
        // Create daily summary before archiving
        writeDailyStats();
        
        // Archive current log with yesterday's date
        char archivePath[64];
        // Get yesterday's date (simplified - doesn't handle month boundaries perfectly)
        uint8_t prevDay = (lastLogDay > 0) ? lastLogDay : currentDay;
        snprintf(archivePath, sizeof(archivePath), 
                "/WATCH/POWER_LOGS/daily_%04d%02d%02d.txt",
                dt.getYear(), dt.getMonth(), prevDay);
        
        // Copy current to daily archive if it exists - USE BUFFERED COPY
        if (SD_MMC.exists(SD_POWER_LOG_CURRENT)) {
            File srcFile = SD_MMC.open(SD_POWER_LOG_CURRENT, FILE_READ);
            if (srcFile) {
                File dstFile = SD_MMC.open(archivePath, FILE_WRITE);
                if (dstFile) {
                    uint8_t buf[512];  // Buffered copy - much faster!
                    size_t bytesRead;
                    while ((bytesRead = srcFile.read(buf, sizeof(buf))) > 0) {
                        dstFile.write(buf, bytesRead);
                        esp_task_wdt_reset();  // Feed watchdog during long operations
                    }
                    dstFile.close();
                    USBSerial.printf("[POWER] Daily log archived: %s\n", archivePath);
                }
                srcFile.close();
            }
        }
        
        // Start fresh log for new day
        initPowerLogging();
    }
    
    lastLogDay = currentDay;
}

// Write daily statistics summary
void writeDailyStats() {
    if (!hasSD) return;
    
    RTC_DateTime dt = rtc.getDateTime();
    
    // Calculate session stats
    uint32_t elapsedMs = millis() - currentPowerSession.sessionStartMs;
    float elapsedHours = elapsedMs / 3600000.0;
    float drainPercent = currentPowerSession.startBatteryPercent - batteryPercent;
    float drainRatePerHour = (elapsedHours > 0.1) ? (drainPercent / elapsedHours) : 0;
    
    // Calculate screen on percentage
    uint32_t totalMs = batteryStats.screenOnTimeMs + batteryStats.screenOffTimeMs;
    float screenOnPercent = (totalMs > 0) ? (batteryStats.screenOnTimeMs * 100.0 / totalMs) : 0;
    
    // Append to history file
    char historyPath[64];
    snprintf(historyPath, sizeof(historyPath), "/WATCH/POWER_LOGS/history_%04d%02d.txt",
            dt.getYear(), dt.getMonth());
    
    File histFile = SD_MMC.open(historyPath, FILE_APPEND);
    if (histFile) {
        histFile.printf("─────────────────────────────────────\n");
        histFile.printf("DATE: %04d-%02d-%02d\n", dt.getYear(), dt.getMonth(), dt.getDay());
        histFile.printf("─────────────────────────────────────\n");
        histFile.printf("Runtime:        %.1f hours\n", elapsedHours);
        histFile.printf("Battery Used:   %.1f%%\n", drainPercent);
        histFile.printf("Drain Rate:     %.2f%%/hour\n", drainRatePerHour);
        histFile.printf("Screen ON:      %.0f%% of time\n", screenOnPercent);
        histFile.printf("Mode Changes:   %lu\n", currentPowerSession.modeChanges);
        histFile.printf("Final Battery:  %d%%\n", batteryPercent);
        histFile.printf("\n");
        histFile.close();
        
        USBSerial.printf("[POWER] Daily stats saved to history\n");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// CREATE TXT FILES CARD - ENHANCED UI
// ═══════════════════════════════════════════════════════════════════════════
void createTxtFilesCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, theme->color1, 0);  // Use theme color instead of hardcoded
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    if (!txtFileLoaded) {
        // ═══ FILE LIST VIEW - ENHANCED UI ═══
        
        // Title bar with gradient effect
        lv_obj_t *titleBar = lv_obj_create(card);
        lv_obj_set_size(titleBar, LCD_WIDTH, 48);
        lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(titleBar, theme->color2, 0);
        lv_obj_set_style_radius(titleBar, 0, 0);
        lv_obj_set_style_border_width(titleBar, 0, 0);
        lv_obj_set_style_shadow_width(titleBar, 8, 0);
        lv_obj_set_style_shadow_color(titleBar, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(titleBar, 100, 0);
        disableAllScrolling(titleBar);

        // Title icon and text
        lv_obj_t *titleIcon = lv_label_create(titleBar);
        lv_label_set_text(titleIcon, LV_SYMBOL_FILE);
        lv_obj_set_style_text_color(titleIcon, theme->accent, 0);
        lv_obj_set_style_text_font(titleIcon, &lv_font_montserrat_18, 0);
        lv_obj_align(titleIcon, LV_ALIGN_LEFT_MID, 15, 0);

        lv_obj_t *title = lv_label_create(titleBar);
        lv_label_set_text(title, "TEXT FILES");
        lv_obj_set_style_text_color(title, theme->text, 0);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_align(title, LV_ALIGN_LEFT_MID, 42, 0);

        // Load file list
        loadTxtFileList();

        if (numTxtFiles == 0) {
            // Empty state with better styling
            lv_obj_t *emptyIcon = lv_label_create(card);
            lv_label_set_text(emptyIcon, LV_SYMBOL_DIRECTORY);
            lv_obj_set_style_text_color(emptyIcon, theme->secondary, 0);
            lv_obj_set_style_text_font(emptyIcon, &lv_font_montserrat_24, 0);
            lv_obj_align(emptyIcon, LV_ALIGN_CENTER, 0, -30);

            lv_obj_t *noFiles = lv_label_create(card);
            lv_label_set_text(noFiles, "No text files found");
            lv_obj_set_style_text_color(noFiles, theme->secondary, 0);
            lv_obj_set_style_text_font(noFiles, &lv_font_montserrat_14, 0);
            lv_obj_align(noFiles, LV_ALIGN_CENTER, 0, 10);

            lv_obj_t *hint = lv_label_create(card);
            lv_label_set_text(hint, "Add .txt files to:\n/WATCH/TXT/");
            lv_obj_set_style_text_color(hint, theme->accent, 0);
            lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(hint, LV_ALIGN_CENTER, 0, 55);
        } else {
            // ═══ BEAUTIFUL FILE LIST ═══
            int yOffset = 56;
            int itemHeight = 52;
            int maxVisible = 5;
            
            // Clamp scroll offset
            int maxScroll = max(0, numTxtFiles - maxVisible);
            if (txtFileListOffset > maxScroll) txtFileListOffset = maxScroll;
            if (txtFileListOffset < 0) txtFileListOffset = 0;
            
            // Scroll indicators with theme colors
            if (numTxtFiles > maxVisible) {
                if (txtFileListOffset > 0) {
                    lv_obj_t *upArrow = lv_label_create(card);
                    lv_label_set_text(upArrow, LV_SYMBOL_UP);
                    lv_obj_set_style_text_color(upArrow, theme->accent, 0);
                    lv_obj_set_style_text_font(upArrow, &lv_font_montserrat_16, 0);
                    lv_obj_align(upArrow, LV_ALIGN_TOP_RIGHT, -12, 52);
                }
                if (txtFileListOffset < maxScroll) {
                    lv_obj_t *downArrow = lv_label_create(card);
                    lv_label_set_text(downArrow, LV_SYMBOL_DOWN);
                    lv_obj_set_style_text_color(downArrow, theme->accent, 0);
                    lv_obj_set_style_text_font(downArrow, &lv_font_montserrat_16, 0);
                    lv_obj_align(downArrow, LV_ALIGN_BOTTOM_RIGHT, -12, -30);
                }
            }
            
            // Display visible files with enhanced styling
            for (int i = 0; i < min(numTxtFiles - txtFileListOffset, maxVisible); i++) {
                int fileIdx = i + txtFileListOffset;
                
                lv_obj_t *fileRow = lv_obj_create(card);
                lv_obj_set_size(fileRow, LCD_WIDTH - 24, 46);
                lv_obj_align(fileRow, LV_ALIGN_TOP_MID, 0, yOffset + i * itemHeight);
                lv_obj_set_style_bg_color(fileRow, theme->color2, 0);
                lv_obj_set_style_radius(fileRow, 12, 0);
                lv_obj_set_style_border_width(fileRow, 1, 0);
                lv_obj_set_style_border_color(fileRow, theme->secondary, 0);
                lv_obj_set_style_border_opa(fileRow, 50, 0);
                disableAllScrolling(fileRow);

                // File icon with theme accent
                lv_obj_t *icon = lv_label_create(fileRow);
                lv_label_set_text(icon, LV_SYMBOL_FILE);
                lv_obj_set_style_text_color(icon, theme->accent, 0);
                lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
                lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

                // Filename
                char displayName[22];
                strncpy(displayName, txtFiles[fileIdx].filename, 18);
                displayName[18] = '\0';
                if (strlen(txtFiles[fileIdx].filename) > 18) {
                    displayName[15] = '.';
                    displayName[16] = '.';
                    displayName[17] = '.';
                }
                
                lv_obj_t *nameLabel = lv_label_create(fileRow);
                lv_label_set_text(nameLabel, displayName);
                lv_obj_set_style_text_color(nameLabel, theme->text, 0);
                lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_12, 0);
                lv_obj_align(nameLabel, LV_ALIGN_LEFT_MID, 35, -6);

                // File size with subtle color
                char sizeBuf[16];
                if (txtFiles[fileIdx].fileSize < 1024) {
                    snprintf(sizeBuf, sizeof(sizeBuf), "%lu bytes", (unsigned long)txtFiles[fileIdx].fileSize);
                } else {
                    snprintf(sizeBuf, sizeof(sizeBuf), "%.1f KB", txtFiles[fileIdx].fileSize / 1024.0);
                }
                lv_obj_t *sizeLabel = lv_label_create(fileRow);
                lv_label_set_text(sizeLabel, sizeBuf);
                lv_obj_set_style_text_color(sizeLabel, theme->secondary, 0);
                lv_obj_set_style_text_font(sizeLabel, &lv_font_montserrat_10, 0);
                lv_obj_align(sizeLabel, LV_ALIGN_LEFT_MID, 35, 10);

                // Chevron arrow
                lv_obj_t *arrow = lv_label_create(fileRow);
                lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
                lv_obj_set_style_text_color(arrow, theme->secondary, 0);
                lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -8, 0);
            }

            // Bottom hint
            lv_obj_t *hint = lv_label_create(card);
            lv_label_set_text(hint, numTxtFiles > maxVisible ? 
                            "Tap edges to scroll" : "Tap file to open");
            lv_obj_set_style_text_color(hint, theme->secondary, 0);
            lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, 0);
            lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -8);
        }
    } else {
        // ═══ FILE CONTENT VIEW - ENHANCED READER ═══
        
        // Header bar
        lv_obj_t *headerBar = lv_obj_create(card);
        lv_obj_set_size(headerBar, LCD_WIDTH, 48);
        lv_obj_align(headerBar, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(headerBar, theme->color2, 0);
        lv_obj_set_style_radius(headerBar, 0, 0);
        lv_obj_set_style_border_width(headerBar, 0, 0);
        lv_obj_set_style_shadow_width(headerBar, 8, 0);
        lv_obj_set_style_shadow_color(headerBar, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(headerBar, 100, 0);
        disableAllScrolling(headerBar);

        // Back button
        lv_obj_t *backArrow = lv_label_create(headerBar);
        lv_label_set_text(backArrow, LV_SYMBOL_LEFT);
        lv_obj_set_style_text_color(backArrow, theme->accent, 0);
        lv_obj_set_style_text_font(backArrow, &lv_font_montserrat_16, 0);
        lv_obj_align(backArrow, LV_ALIGN_LEFT_MID, 12, 0);

        // Filename in header
        char headerName[18];
        strncpy(headerName, txtFiles[selectedTxtFile].filename, 14);
        headerName[14] = '\0';
        if (strlen(txtFiles[selectedTxtFile].filename) > 14) {
            headerName[11] = '.';
            headerName[12] = '.';
            headerName[13] = '.';
        }
        lv_obj_t *fileName = lv_label_create(headerBar);
        lv_label_set_text(fileName, headerName);
        lv_obj_set_style_text_color(fileName, theme->text, 0);
        lv_obj_set_style_text_font(fileName, &lv_font_montserrat_14, 0);
        lv_obj_center(fileName);

        // Content area with nice styling
        lv_obj_t *contentArea = lv_obj_create(card);
        lv_obj_set_size(contentArea, LCD_WIDTH - 16, LCD_HEIGHT - 85);
        lv_obj_align(contentArea, LV_ALIGN_TOP_MID, 0, 52);
        lv_obj_set_style_bg_color(contentArea, theme->color2, 0);
        lv_obj_set_style_radius(contentArea, 16, 0);
        lv_obj_set_style_border_width(contentArea, 0, 0);
        lv_obj_set_style_pad_all(contentArea, 14, 0);
        
        // Enable smooth scrolling for content
        lv_obj_set_scroll_dir(contentArea, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_style_bg_color(contentArea, theme->accent, LV_PART_SCROLLBAR);
        lv_obj_set_style_bg_opa(contentArea, LV_OPA_70, LV_PART_SCROLLBAR);
        lv_obj_set_style_width(contentArea, 4, LV_PART_SCROLLBAR);

        // Text content with readable styling
        lv_obj_t *textLabel = lv_label_create(contentArea);
        lv_label_set_text(textLabel, txtFileContent);
        lv_obj_set_style_text_color(textLabel, theme->text, 0);
        lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_line_space(textLabel, 4, 0);  // Better line spacing
        lv_obj_set_width(textLabel, LCD_WIDTH - 50);
        lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
        lv_obj_align(textLabel, LV_ALIGN_TOP_LEFT, 0, 0);

        // Scroll progress bar at bottom
        lv_obj_t *progressBg = lv_obj_create(card);
        lv_obj_set_size(progressBg, LCD_WIDTH - 80, 4);
        lv_obj_align(progressBg, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_set_style_bg_color(progressBg, theme->secondary, 0);
        lv_obj_set_style_bg_opa(progressBg, 100, 0);
        lv_obj_set_style_radius(progressBg, 2, 0);
        lv_obj_set_style_border_width(progressBg, 0, 0);

        // Scroll hint
        lv_obj_t *scrollHint = lv_label_create(card);
        lv_label_set_text(scrollHint, "Tap header to go back");
        lv_obj_set_style_text_color(scrollHint, theme->secondary, 0);
        lv_obj_set_style_text_font(scrollHint, &lv_font_montserrat_10, 0);
        lv_obj_align(scrollHint, LV_ALIGN_BOTTOM_MID, 0, -18);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WIFI NETWORK SCANNING
// ═══════════════════════════════════════════════════════════════════════════
void scanWiFiNetworks() {
    numScannedNetworks = 0;
    
    int n = WiFi.scanNetworks(false, false);  // Synchronous scan
    
    if (n > 0) {
        for (int i = 0; i < min(n, MAX_SCANNED_NETWORKS); i++) {
            strncpy(scannedNetworks[numScannedNetworks].ssid, WiFi.SSID(i).c_str(), 63);
            scannedNetworks[numScannedNetworks].ssid[63] = '\0';
            scannedNetworks[numScannedNetworks].rssi = WiFi.RSSI(i);
            scannedNetworks[numScannedNetworks].isOpen = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            scannedNetworks[numScannedNetworks].isConnected = (WiFi.status() == WL_CONNECTED && 
                                                              WiFi.SSID() == WiFi.SSID(i));
            numScannedNetworks++;
        }
    }
    
    wifiScanComplete = true;
    WiFi.scanDelete();  // Free memory
    USBSerial.printf("[WIFI] Scan complete: %d networks found\n", numScannedNetworks);
}

// ═══════════════════════════════════════════════════════════════════════════
// CONNECT TO HARDCODED WIFI + TIME SYNC (NEW!)
// ═══════════════════════════════════════════════════════════════════════════
void connectToHardcodedWiFi() {
    USBSerial.printf("[WIFI] Connecting to hardcoded: %s\n", hardcodedSSID);
    
    WiFi.begin(hardcodedSSID, hardcodedPass);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
        USBSerial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        USBSerial.printf("\n[WIFI] Connected to: %s\n", hardcodedSSID);
        
        // Sync time via NTP
        USBSerial.println("[NTP] Syncing time...");
        configTime(gmtOffsetSec, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        delay(2000);
        
        // Update RTC from NTP
        if (hasRTC) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                rtc.setDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                ntpSyncedOnce = true;
                lastNTPSync = millis();
                USBSerial.printf("[NTP] Time synced: %04d-%02d-%02d %02d:%02d:%02d\n",
                                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            }
        }
        
        // Fetch weather for location
        fetchLocationFromIP();
    } else {
        wifiConnected = false;
        USBSerial.println("\n[WIFI] Failed to connect to hardcoded network");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// WIFI CARD - NETWORK MANAGER (NOW IN SETTINGS)
// ═══════════════════════════════════════════════════════════════════════════
void createWiFiCard() {
    GradientTheme *theme = getSafeTheme();
    disableAllScrolling(lv_scr_act());

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_set_size(card, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    disableAllScrolling(card);

    // Top accent bar - WiFi blue
    lv_obj_t *accentBar = lv_obj_create(card);
    lv_obj_set_size(accentBar, LCD_WIDTH, 4);
    lv_obj_align(accentBar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(accentBar, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_radius(accentBar, 0, 0);
    lv_obj_set_style_border_width(accentBar, 0, 0);
    disableAllScrolling(accentBar);

    // Title badge
    lv_obj_t *titleBadge = lv_obj_create(card);
    lv_obj_set_size(titleBadge, 130, 32);
    lv_obj_align(titleBadge, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_bg_color(titleBadge, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_bg_opa(titleBadge, LV_OPA_20, 0);
    lv_obj_set_style_radius(titleBadge, 16, 0);
    lv_obj_set_style_border_width(titleBadge, 1, 0);
    lv_obj_set_style_border_color(titleBadge, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_border_opa(titleBadge, LV_OPA_50, 0);
    disableAllScrolling(titleBadge);

    lv_obj_t *title = lv_label_create(titleBadge);
    lv_label_set_text(title, LV_SYMBOL_WIFI " WiFi");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_center(title);

    // Connection status card
    lv_obj_t *statusCard = lv_obj_create(card);
    lv_obj_set_size(statusCard, LCD_WIDTH - 24, 70);
    lv_obj_align(statusCard, LV_ALIGN_TOP_MID, 0, 52);
    lv_obj_set_style_bg_color(statusCard, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_radius(statusCard, 16, 0);
    lv_obj_set_style_border_width(statusCard, 1, 0);
    lv_obj_set_style_border_color(statusCard, wifiConnected ? lv_color_hex(0x30D158) : lv_color_hex(0x2A2A2A), 0);
    disableAllScrolling(statusCard);

    lv_obj_t *statusIcon = lv_label_create(statusCard);
    lv_label_set_text(statusIcon, wifiConnected ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(statusIcon, wifiConnected ? lv_color_hex(0x30D158) : lv_color_hex(0xFF453A), 0);
    lv_obj_set_style_text_font(statusIcon, &lv_font_montserrat_24, 0);
    lv_obj_align(statusIcon, LV_ALIGN_LEFT_MID, 15, 0);

    lv_obj_t *statusLabel = lv_label_create(statusCard);
    if (wifiConnected) {
        char ssidBuf[24];
        strncpy(ssidBuf, WiFi.SSID().c_str(), 20);
        ssidBuf[20] = '\0';
        lv_label_set_text(statusLabel, ssidBuf);
        lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFFFFF), 0);
    } else {
        lv_label_set_text(statusLabel, "Not Connected");
        lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x8E8E93), 0);
    }
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(statusLabel, LV_ALIGN_LEFT_MID, 50, -8);

    lv_obj_t *infoLabel = lv_label_create(statusCard);
    if (wifiConnected && ntpSyncedOnce) {
        lv_label_set_text(infoLabel, "Time synced " LV_SYMBOL_OK);
        lv_obj_set_style_text_color(infoLabel, lv_color_hex(0x30D158), 0);
    } else {
        lv_label_set_text(infoLabel, "Tap below to connect");
        lv_obj_set_style_text_color(infoLabel, lv_color_hex(0x636366), 0);
    }
    lv_obj_set_style_text_font(infoLabel, &lv_font_montserrat_10, 0);
    lv_obj_align(infoLabel, LV_ALIGN_LEFT_MID, 50, 10);

    // Connect button with glow effect
    lv_obj_t *connectBtn = lv_obj_create(card);
    lv_obj_set_size(connectBtn, LCD_WIDTH - 24, 55);
    lv_obj_align(connectBtn, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_set_style_bg_color(connectBtn, wifiConnected ? lv_color_hex(0x30D158) : lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_radius(connectBtn, 16, 0);
    lv_obj_set_style_border_width(connectBtn, 0, 0);
    lv_obj_set_style_shadow_width(connectBtn, 12, 0);
    lv_obj_set_style_shadow_color(connectBtn, wifiConnected ? lv_color_hex(0x30D158) : lv_color_hex(0x0A84FF), 0);
    lv_obj_set_style_shadow_opa(connectBtn, LV_OPA_30, 0);
    disableAllScrolling(connectBtn);

    lv_obj_t *connectIcon = lv_label_create(connectBtn);
    lv_label_set_text(connectIcon, wifiConnected ? LV_SYMBOL_OK : LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(connectIcon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(connectIcon, &lv_font_montserrat_20, 0);
    lv_obj_align(connectIcon, LV_ALIGN_LEFT_MID, 20, 0);

    lv_obj_t *connectLabel = lv_label_create(connectBtn);
    char btnBuf[32];
    snprintf(btnBuf, sizeof(btnBuf), wifiConnected ? "CONNECTED" : "Connect: %s", hardcodedSSID);
    lv_label_set_text(connectLabel, btnBuf);
    lv_obj_set_style_text_color(connectLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(connectLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(connectLabel, LV_ALIGN_LEFT_MID, 55, 0);

    // Saved networks section
    lv_obj_t *savedTitle = lv_label_create(card);
    lv_label_set_text(savedTitle, "SAVED NETWORKS");
    lv_obj_set_style_text_color(savedTitle, lv_color_hex(0x636366), 0);
    lv_obj_set_style_text_font(savedTitle, &lv_font_montserrat_10, 0);
    lv_obj_align(savedTitle, LV_ALIGN_TOP_LEFT, 18, 195);

    lv_obj_t *savedContainer = lv_obj_create(card);
    lv_obj_set_size(savedContainer, LCD_WIDTH - 24, 95);
    lv_obj_align(savedContainer, LV_ALIGN_TOP_MID, 0, 212);
    lv_obj_set_style_bg_color(savedContainer, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_radius(savedContainer, 16, 0);
    lv_obj_set_style_border_width(savedContainer, 0, 0);
    disableAllScrolling(savedContainer);

    if (numWifiNetworks > 0) {
        int yOff = 10;
        for (int i = 0; i < min(numWifiNetworks, 2); i++) {
            lv_obj_t *netIcon = lv_label_create(savedContainer);
            lv_label_set_text(netIcon, LV_SYMBOL_WIFI);
            bool isConnected = wifiConnected && (String(wifiNetworks[i].ssid) == WiFi.SSID());
            lv_obj_set_style_text_color(netIcon, isConnected ? lv_color_hex(0x30D158) : lv_color_hex(0x636366), 0);
            lv_obj_align(netIcon, LV_ALIGN_TOP_LEFT, 15, yOff + 5);

            char ssidDisplay[24];
            strncpy(ssidDisplay, wifiNetworks[i].ssid, 20);
            ssidDisplay[20] = '\0';
            lv_obj_t *ssidLabel = lv_label_create(savedContainer);
            lv_label_set_text(ssidLabel, ssidDisplay);
            lv_obj_set_style_text_color(ssidLabel, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(ssidLabel, &lv_font_montserrat_14, 0);
            lv_obj_align(ssidLabel, LV_ALIGN_TOP_LEFT, 45, yOff + 5);
            yOff += 40;
        }
    } else {
        lv_obj_t *noNetLabel = lv_label_create(savedContainer);
        lv_label_set_text(noNetLabel, "No saved networks");
        lv_obj_set_style_text_color(noNetLabel, lv_color_hex(0x636366), 0);
        lv_obj_center(noNetLabel);
    }

    lv_obj_t *hint = lv_label_create(card);
    lv_label_set_text(hint, "TAP BUTTON TO CONNECT & SYNC");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x4A4A4A), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// ═══════════════════════════════════════════════════════════════════════════
// USER DATA PERSISTENCE
// ═══════════════════════════════════════════════════════════════════════════
void saveUserData() {
    prefs.begin("minios", false);
    prefs.putUInt("steps", userData.steps);
    prefs.putUInt("dailyGoal", userData.dailyGoal);
    prefs.putInt("brightness", userData.brightness);
    prefs.putInt("themeIndex", userData.themeIndex);
    prefs.putInt("wallpaper", userData.wallpaperIndex);
    prefs.putInt("watchFace", userData.watchFaceIndex);  // Save watch face
    prefs.putInt("nikeColor", currentNikeColor);  // Save Nike color variant
    prefs.putFloat("distance", userData.totalDistance);
    prefs.putFloat("calories", userData.totalCalories);
    prefs.putInt("gamesWon", userData.gamesWon);
    prefs.putInt("gamesPlayed", userData.gamesPlayed);
    prefs.putInt("batterySaverLvl", (int)batterySaverLevel);  // Save battery saver level
    prefs.putBool("use24Hr", use24HourFormat);  // NEW: Save 24hr format preference
    
    // Save weather data (persists until next WiFi sync)
    prefs.putFloat("weatherTemp", weatherTemp);
    prefs.putFloat("weatherHigh", weatherHigh);
    prefs.putFloat("weatherLow", weatherLow);
    prefs.putString("weatherDesc", weatherDesc);
    prefs.putBool("weatherLoaded", true);
    
    // Save 5-day forecast
    for (int i = 0; i < 5; i++) {
        char keyHigh[16], keyLow[16], keyIcon[16], keyDay[16];
        snprintf(keyHigh, sizeof(keyHigh), "fcHigh%d", i);
        snprintf(keyLow, sizeof(keyLow), "fcLow%d", i);
        snprintf(keyIcon, sizeof(keyIcon), "fcIcon%d", i);
        snprintf(keyDay, sizeof(keyDay), "fcDay%d", i);
        prefs.putFloat(keyHigh, forecast5Day[i].tempHigh);
        prefs.putFloat(keyLow, forecast5Day[i].tempLow);
        prefs.putString(keyIcon, forecast5Day[i].icon);
        prefs.putString(keyDay, forecast5Day[i].dayName);
    }
    prefs.end();
    lastSaveTime = millis();
    
    // Also backup time to SD for watchdog recovery
    saveTimeBackup();
}

void loadUserData() {
    prefs.begin("minios", true);
    userData.steps = prefs.getUInt("steps", 0);
    userData.dailyGoal = prefs.getUInt("dailyGoal", 10000);
    userData.brightness = prefs.getInt("brightness", 200);
    userData.themeIndex = prefs.getInt("themeIndex", 0);
    userData.wallpaperIndex = prefs.getInt("wallpaper", 0);
    userData.watchFaceIndex = prefs.getInt("watchFace", 0);  // Load watch face
    currentNikeColor = prefs.getInt("nikeColor", 0);  // Load Nike color variant
    userData.totalDistance = prefs.getFloat("distance", 0.0);
    userData.totalCalories = prefs.getFloat("calories", 0.0);
    userData.gamesWon = prefs.getInt("gamesWon", 0);
    userData.gamesPlayed = prefs.getInt("gamesPlayed", 0);
    batterySaverLevel = (BatterySaverLevel)prefs.getInt("batterySaverLvl", 0);  // Default to OFF mode
    use24HourFormat = prefs.getBool("use24Hr", true);  // NEW: Load 24hr format (default true)
    
    // Load cached weather data (persists until WiFi sync)
    weatherDataLoaded = prefs.getBool("weatherLoaded", false);
    if (weatherDataLoaded) {
        weatherTemp = prefs.getFloat("weatherTemp", 24.0);
        weatherHigh = prefs.getFloat("weatherHigh", 28.0);
        weatherLow = prefs.getFloat("weatherLow", 18.0);
        weatherDesc = prefs.getString("weatherDesc", "Sunny");
        
        // Load 5-day forecast
        for (int i = 0; i < 5; i++) {
            char keyHigh[16], keyLow[16], keyIcon[16], keyDay[16];
            snprintf(keyHigh, sizeof(keyHigh), "fcHigh%d", i);
            snprintf(keyLow, sizeof(keyLow), "fcLow%d", i);
            snprintf(keyIcon, sizeof(keyIcon), "fcIcon%d", i);
            snprintf(keyDay, sizeof(keyDay), "fcDay%d", i);
            forecast5Day[i].tempHigh = prefs.getFloat(keyHigh, forecast5Day[i].tempHigh);
            forecast5Day[i].tempLow = prefs.getFloat(keyLow, forecast5Day[i].tempLow);
            String iconStr = prefs.getString(keyIcon, forecast5Day[i].icon);
            strncpy(forecast5Day[i].icon, iconStr.c_str(), 15);
            String dayStr = prefs.getString(keyDay, forecast5Day[i].dayName);
            strncpy(forecast5Day[i].dayName, dayStr.c_str(), 3);
        }
        forecastLoaded = true;
        USBSerial.println("[WEATHER] Loaded cached weather data from Preferences");
    }
    prefs.end();
    
    // Validate indices
    if (userData.watchFaceIndex < 0 || userData.watchFaceIndex >= NUM_WATCH_FACES) {
        userData.watchFaceIndex = 0;
    }
    if (currentNikeColor >= NUM_NIKE_COLORS) {
        currentNikeColor = 0;
    }
    if ((int)batterySaverLevel < 0 || (int)batterySaverLevel > 2) {
        batterySaverLevel = BATTERY_SAVER_OFF;  // 3 modes: 0, 1, 2
    }
    batterySaverMode = (batterySaverLevel != BATTERY_SAVER_OFF);
    getSafeThemeIndex();
}

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR FUSION
// ═══════════════════════════════════════════════════════════════════════════
void updateSensorFusion() {
    if (!hasIMU) return;
    qmi.getAccelerometer(acc.x, acc.y, acc.z);
    qmi.getGyroscope(gyr.x, gyr.y, gyr.z);

    tiltX = atan2(acc.y, acc.z) * 180.0 / M_PI;
    tiltY = atan2(-acc.x, sqrt(acc.y * acc.y + acc.z * acc.z)) * 180.0 / M_PI;

    float heading = atan2(acc.y, acc.x) * 180.0 / M_PI;
    if (heading < 0) heading += 360.0;
    heading += compassNorthOffset;
    if (heading >= 360) heading -= 360;
    if (heading < 0) heading += 360;

    compassHeadingSmooth = compassHeadingSmooth * 0.9 + heading * 0.1;
}

void updateStepCount() {
    if (!hasIMU) return;
    static float lastMag = 0;
    static bool stepDet = false;
    static unsigned long lastStepTime = 0;

    float mag = sqrt(acc.x*acc.x + acc.y*acc.y + acc.z*acc.z);
    if (mag > 1.2 && lastMag <= 1.2 && !stepDet && millis() - lastStepTime > 250) {
        userData.steps++;
        userData.totalDistance += 0.0007;
        userData.totalCalories += 0.04;
        lastStepTime = millis();
        stepDet = true;
    }
    if (mag < 1.2) stepDet = false;
    lastMag = mag;
}

// ═══════════════════════════════════════════════════════════════════════════
// WEATHER FETCH
// ═══════════════════════════════════════════════════════════════════════════
void fetchWeatherData() {
    if (!wifiConnected) return;

    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    url += weatherCity;
    url += ",";
    url += weatherCountry;
    url += "&units=metric&appid=";
    url += OPENWEATHER_API;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);

        weatherTemp = doc["main"]["temp"];
        weatherHigh = doc["main"]["temp_max"];
        weatherLow = doc["main"]["temp_min"];
        weatherDesc = doc["weather"][0]["main"].as<String>();
    }

    http.end();
    lastWeatherUpdate = millis();
}

// ═══════════════════════════════════════════════════════════════════════════
// FETCH 5-DAY FORECAST FROM OPENWEATHERMAP API
// ═══════════════════════════════════════════════════════════════════════════
void fetch5DayForecast() {
    if (!wifiConnected) return;

    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/forecast?q=";
    url += weatherCity;
    url += ",";
    url += weatherCountry;
    url += "&units=metric&cnt=40&appid=";
    url += OPENWEATHER_API;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(16384);  // Larger buffer for forecast data
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonArray list = doc["list"];
            const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            
            // Get one forecast per day (every 8th entry = 24 hours)
            for (int i = 0; i < 5; i++) {
                int idx = i * 8;  // Every 24 hours (8 * 3 hour intervals)
                if (idx < (int)list.size()) {
                    JsonObject entry = list[idx];
                    
                    // Get day of week from timestamp
                    long dt = entry["dt"];
                    time_t timestamp = dt;
                    struct tm *timeinfo = localtime(&timestamp);
                    
                    strncpy(forecast5Day[i].dayName, dayNames[timeinfo->tm_wday], 3);
                    forecast5Day[i].dayName[3] = '\0';
                    
                    // Temperature
                    forecast5Day[i].tempHigh = entry["main"]["temp_max"];
                    forecast5Day[i].tempLow = entry["main"]["temp_min"];
                    
                    // Condition
                    const char* main = entry["weather"][0]["main"];
                    strncpy(forecast5Day[i].condition, main, 15);
                    forecast5Day[i].condition[15] = '\0';
                    
                    // Icon mapping
                    if (strstr(main, "Clear") || strstr(main, "Sun")) {
                        strcpy(forecast5Day[i].icon, "sun");
                    } else if (strstr(main, "Cloud")) {
                        strcpy(forecast5Day[i].icon, "cloud");
                    } else if (strstr(main, "Rain") || strstr(main, "Drizzle")) {
                        strcpy(forecast5Day[i].icon, "rain");
                    } else if (strstr(main, "Snow")) {
                        strcpy(forecast5Day[i].icon, "snow");
                    } else if (strstr(main, "Thunder")) {
                        strcpy(forecast5Day[i].icon, "storm");
                    } else {
                        strcpy(forecast5Day[i].icon, "cloud");
                    }
                }
            }
            forecastLoaded = true;
            USBSerial.println("[WEATHER] 5-day forecast loaded from API");
        }
    }

    http.end();
}

// ═══════════════════════════════════════════════════════════════════════════
// AUTO LOCATION FETCH FROM IP (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void fetchLocationFromIP() {
    if (!wifiConnected) return;

    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=city,country,timezone");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);

        if (doc.containsKey("city")) {
            strncpy(weatherCity, doc["city"].as<const char*>(), sizeof(weatherCity) - 1);
        }
        if (doc.containsKey("country")) {
            strncpy(weatherCountry, doc["country"].as<const char*>(), sizeof(weatherCountry) - 1);
        }

        USBSerial.printf("[LOCATION] Detected: %s, %s\n", weatherCity, weatherCountry);
    }

    http.end();
    
    // Now fetch weather for detected location
    fetchWeatherData();
    fetch5DayForecast();  // Also fetch 5-day forecast
}

// ═══════════════════════════════════════════════════════════════════════════
// SD CARD FOLDER STRUCTURE (FROM 206q - FUSION LABS COMPATIBLE)
// ═══════════════════════════════════════════════════════════════════════════
bool createDirectoryIfNotExists(const char* path) {
    if (!SD_MMC.exists(path)) {
        if (SD_MMC.mkdir(path)) {
            USBSerial.printf("[SD] Created: %s\n", path);
            return true;
        } else {
            USBSerial.printf("[SD] Failed to create: %s\n", path);
            return false;
        }
    }
    return true;
}

void logToBootLog(const char* message) {
    if (!hasSD) return;
    File file = SD_MMC.open(SD_BOOT_LOG, FILE_APPEND);
    if (file) {
        file.printf("[%lu] %s\n", millis(), message);
        file.close();
    }
}

bool createWidgetOSFolderStructure() {
    bool success = true;

    // Root
    success &= createDirectoryIfNotExists(SD_ROOT_PATH);

    // SYSTEM
    success &= createDirectoryIfNotExists(SD_SYSTEM_PATH);
    success &= createDirectoryIfNotExists(SD_SYSTEM_LOGS_PATH);

    // CONFIG
    success &= createDirectoryIfNotExists(SD_CONFIG_PATH);

    // FACES
    success &= createDirectoryIfNotExists(SD_FACES_PATH);
    success &= createDirectoryIfNotExists(SD_FACES_CUSTOM_PATH);
    success &= createDirectoryIfNotExists(SD_FACES_IMPORTED_PATH);

    // IMAGES
    success &= createDirectoryIfNotExists(SD_IMAGES_PATH);

    // MUSIC
    success &= createDirectoryIfNotExists(SD_MUSIC_PATH);

    // CACHE
    success &= createDirectoryIfNotExists(SD_CACHE_PATH);
    success &= createDirectoryIfNotExists(SD_CACHE_TEMP_PATH);

    // UPDATE
    success &= createDirectoryIfNotExists(SD_UPDATE_PATH);

    // WiFi
    success &= createDirectoryIfNotExists(SD_WIFI_PATH);

    // BACKUP (Fusion Labs)
    success &= createDirectoryIfNotExists(SD_BACKUP_PATH);

    // FIRMWARE (Fusion Labs)
    success &= createDirectoryIfNotExists(SD_FIRMWARE_PATH);

    // LOGS
    success &= createDirectoryIfNotExists(SD_LOGS_PATH);

    // WALLPAPERS
    success &= createDirectoryIfNotExists(SD_WALLPAPERS_PATH);
    
    // NEW: Power consumption logs folder
    success &= createDirectoryIfNotExists(SD_POWER_LOGS_PATH);

    if (!success) {
        sdCardStatus = SD_STATUS_CORRUPT;
        return false;
    }

    // Create boot log
    File bootLog = SD_MMC.open(SD_BOOT_LOG, FILE_WRITE);
    if (bootLog) {
        bootLog.println("═══════════════════════════════════════════════════════════════════");
        bootLog.printf("  %s %s - Boot Log\n", WIDGET_OS_NAME, WIDGET_OS_VERSION);
        bootLog.printf("  Device: %s\n", DEVICE_ID);
        bootLog.println("═══════════════════════════════════════════════════════════════════");
        bootLog.close();
    }

    // Create default config.txt if not exists (Fusion Labs compatible)
    if (!SD_MMC.exists(SD_CONFIG_TXT)) {
        File cfg = SD_MMC.open(SD_CONFIG_TXT, FILE_WRITE);
        if (cfg) {
            cfg.println("# Widget OS Configuration");
            cfg.println("# Fusion Labs Compatible v1.0");
            cfg.println("");
            cfg.println("[device]");
            cfg.println("name=Widget OS Watch");
            cfg.println("board=2.06");
            cfg.println("");
            cfg.println("[backup]");
            cfg.println("auto_backup=true");
            cfg.println("backup_interval_hours=24");
            cfg.println("");
            cfg.println("[display]");
            cfg.printf("brightness=%d\n", userData.brightness);
            cfg.println("screen_timeout=30");
            cfg.println("");
            cfg.println("[wifi]");
            cfg.println("# Add WiFi networks here:");
            cfg.println("# ssid=YourNetwork");
            cfg.println("# password=YourPassword");
            cfg.close();
            USBSerial.println("[SD] Created config.txt");
        }
    }

    // Create WiFi config template with 5 network slots
    if (!SD_MMC.exists(SD_WIFI_CONFIG)) {
        File wifiCfg = SD_MMC.open(SD_WIFI_CONFIG, FILE_WRITE);
        if (wifiCfg) {
            wifiCfg.println("# Widget OS WiFi Configuration");
            wifiCfg.println("# Supports up to 5 WiFi networks");
            wifiCfg.println("");
            wifiCfg.println("# Network 1 (Primary)");
            wifiCfg.println("SSID1=YourHomeWiFi");
            wifiCfg.println("PASSWORD1=YourPassword");
            wifiCfg.println("OPEN1=false");
            wifiCfg.println("");
            wifiCfg.println("# Network 2");
            wifiCfg.println("SSID2=");
            wifiCfg.println("PASSWORD2=");
            wifiCfg.println("OPEN2=false");
            wifiCfg.println("");
            wifiCfg.println("# Network 3");
            wifiCfg.println("SSID3=");
            wifiCfg.println("PASSWORD3=");
            wifiCfg.println("OPEN3=false");
            wifiCfg.println("");
            wifiCfg.println("# Network 4");
            wifiCfg.println("SSID4=");
            wifiCfg.println("PASSWORD4=");
            wifiCfg.println("OPEN4=false");
            wifiCfg.println("");
            wifiCfg.println("# Network 5");
            wifiCfg.println("SSID5=");
            wifiCfg.println("PASSWORD5=");
            wifiCfg.println("OPEN5=false");
            wifiCfg.println("");
            wifiCfg.println("# Weather Location");
            wifiCfg.println("CITY=Perth");
            wifiCfg.println("COUNTRY=AU");
            wifiCfg.println("");
            wifiCfg.println("# Timezone (hours from GMT)");
            wifiCfg.println("GMT_OFFSET=8");
            wifiCfg.close();
            USBSerial.println("[SD] Created wifi/config.txt with 5 network slots");
        }
    }

    sdStructureCreated = true;
    logToBootLog("Widget OS folder structure created");
    return true;
}

bool initWidgetOSSDCard() {
    sdCardStatus = SD_STATUS_INIT_IN_PROGRESS;

    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);

    if (!SD_MMC.begin("/sdcard", true, true)) {
        sdCardStatus = SD_STATUS_MOUNT_FAILED;
        sdErrorMessage = "SD card mount failed";
        hasSD = false;
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        sdCardStatus = SD_STATUS_NOT_PRESENT;
        sdErrorMessage = "No SD card detected";
        hasSD = false;
        return false;
    }

    switch(cardType) {
        case CARD_MMC:  sdCardType = "MMC"; break;
        case CARD_SD:   sdCardType = "SDSC"; break;
        case CARD_SDHC: sdCardType = "SDHC"; break;
        default:        sdCardType = "UNKNOWN"; break;
    }

    sdCardSizeMB = SD_MMC.cardSize() / (1024 * 1024);
    sdCardInitialized = true;
    hasSD = true;
    sdCardStatus = SD_STATUS_MOUNTED_OK;

    createWidgetOSFolderStructure();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// WIFI CONFIG FROM SD (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
bool loadWiFiConfigFromSD() {
    if (!hasSD || !sdCardInitialized) return false;

    if (!SD_MMC.exists(SD_WIFI_CONFIG)) return false;

    File file = SD_MMC.open(SD_WIFI_CONFIG, FILE_READ);
    if (!file) return false;

    // DON'T reset networks - keep hardcoded in slot 0!
    // SD networks will be added starting from slot 1
    int sdNetworkCount = 0;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.length() == 0 || line.startsWith("#")) continue;

        int eqPos = line.indexOf('=');
        if (eqPos <= 0) continue;

        String key = line.substring(0, eqPos);
        String value = line.substring(eqPos + 1);
        key.trim();
        value.trim();

        // Support for SD WiFi networks: SSID1-SSID4, PASSWORD1-PASSWORD4
        // These map to slots 1-4 (slot 0 is hardcoded)
        for (int slot = 1; slot <= 4; slot++) {
            char ssidKey[16], passKey[16], openKey[16];
            snprintf(ssidKey, sizeof(ssidKey), "SSID%d", slot);
            snprintf(passKey, sizeof(passKey), "PASSWORD%d", slot);
            snprintf(openKey, sizeof(openKey), "OPEN%d", slot);
            
            // Map SD slot 1-4 to array index 1-4 (0 is hardcoded)
            int idx = slot;
            
            if (key == ssidKey && idx < MAX_WIFI_NETWORKS) {
                strncpy(wifiNetworks[idx].ssid, value.c_str(), sizeof(wifiNetworks[idx].ssid) - 1);
                wifiNetworks[idx].valid = true;
                sdNetworkCount++;
                if (idx >= numWifiNetworks) numWifiNetworks = idx + 1;
            }
            else if (key == passKey && idx < MAX_WIFI_NETWORKS) {
                strncpy(wifiNetworks[idx].password, value.c_str(), sizeof(wifiNetworks[idx].password) - 1);
            }
            else if (key == openKey && idx < MAX_WIFI_NETWORKS) {
                wifiNetworks[idx].isOpen = (value == "1" || value.equalsIgnoreCase("true"));
            }
        }
        
        // Legacy single network support (SSID, PASSWORD without number) -> slot 1
        if (key == "SSID") {
            strncpy(wifiNetworks[1].ssid, value.c_str(), sizeof(wifiNetworks[1].ssid) - 1);
            wifiNetworks[1].valid = true;
            sdNetworkCount++;
            if (numWifiNetworks < 2) numWifiNetworks = 2;
        }
        else if (key == "PASSWORD") {
            strncpy(wifiNetworks[1].password, value.c_str(), sizeof(wifiNetworks[1].password) - 1);
        }
        else if (key == "CITY") {
            strncpy(weatherCity, value.c_str(), sizeof(weatherCity) - 1);
        }
        else if (key == "COUNTRY") {
            strncpy(weatherCountry, value.c_str(), sizeof(weatherCountry) - 1);
        }
        else if (key == "GMT_OFFSET") {
            gmtOffsetSec = value.toInt() * 3600;
        }
    }

    file.close();

    if (sdNetworkCount > 0) {
        wifiConfigFromSD = true;
        logToBootLog("Loaded WiFi config from SD card");
        char logMsg[64];
        snprintf(logMsg, sizeof(logMsg), "[SD] Loaded %d WiFi networks from SD (slots 1-%d)", 
                 sdNetworkCount, numWifiNetworks - 1);
        USBSerial.println(logMsg);
        return true;
    }

    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// BACKUP SYSTEM (FROM 206q - FUSION LABS COMPATIBLE)
// ═══════════════════════════════════════════════════════════════════════════
bool createBackup(bool isAuto) {
    if (!hasSD) return false;

    // Create timestamp for backup folder
    char backupDir[64];
    RTC_DateTime dt = rtc.getDateTime();
    snprintf(backupDir, sizeof(backupDir), "%s/%04d%02d%02d_%02d%02d%02d%s",
             SD_BACKUP_PATH,
             dt.getYear(), dt.getMonth(), dt.getDay(),
             dt.getHour(), dt.getMinute(), dt.getSecond(),
             isAuto ? "_auto" : "_manual");

    if (!SD_MMC.mkdir(backupDir)) {
        sdHealth.writeErrors++;
        return false;
    }

    // Save user data
    String userDataPath = String(backupDir) + "/user_data.json";
    File userFile = SD_MMC.open(userDataPath.c_str(), FILE_WRITE);
    if (userFile) {
        DynamicJsonDocument doc(1024);
        doc["steps"] = userData.steps;
        doc["daily_goal"] = userData.dailyGoal;
        doc["distance"] = userData.totalDistance;
        doc["calories"] = userData.totalCalories;
        doc["games_won"] = userData.gamesWon;
        doc["games_played"] = userData.gamesPlayed;
        doc["brightness"] = userData.brightness;
        doc["theme"] = userData.themeIndex;
        doc["wallpaper"] = userData.wallpaperIndex;
        doc["backup_time"] = millis();
        doc["is_auto"] = isAuto;

        serializeJsonPretty(doc, userFile);
        userFile.close();
    }

    // Save compass calibration
    String compassPath = String(backupDir) + "/compass_calibration.txt";
    File compassFile = SD_MMC.open(compassPath.c_str(), FILE_WRITE);
    if (compassFile) {
        compassFile.println(compassNorthOffset);
        compassFile.close();
    }

    totalBackups++;
    lastBackupTime = millis();
    logToBootLog(isAuto ? "Auto backup created" : "Manual backup created");
    USBSerial.printf("[BACKUP] Created: %s\n", backupDir);
    return true;
}

bool restoreFromBackup(const String& backupName) {
    if (!hasSD) return false;

    String backupDir = String(SD_BACKUP_PATH) + "/" + backupName;

    if (!SD_MMC.exists(backupDir.c_str())) {
        USBSerial.println("[RESTORE] Backup not found");
        return false;
    }

    // Restore user data
    String userDataPath = backupDir + "/user_data.json";
    File userFile = SD_MMC.open(userDataPath.c_str(), FILE_READ);
    if (userFile) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, userFile);
        userFile.close();

        if (!error) {
            userData.steps = doc["steps"] | 0;
            userData.dailyGoal = doc["daily_goal"] | 10000;
            userData.totalDistance = doc["distance"] | 0.0;
            userData.totalCalories = doc["calories"] | 0.0;
            userData.gamesWon = doc["games_won"] | 0;
            userData.gamesPlayed = doc["games_played"] | 0;
            userData.brightness = doc["brightness"] | 200;
            userData.themeIndex = doc["theme"] | 0;
            userData.wallpaperIndex = doc["wallpaper"] | 0;

            gfx->setBrightness(userData.brightness);
            saveUserData();
        }
    }

    // Restore compass calibration
    String compassPath = backupDir + "/compass_calibration.txt";
    File compassFile = SD_MMC.open(compassPath.c_str(), FILE_READ);
    if (compassFile) {
        String line = compassFile.readStringUntil('\n');
        compassNorthOffset = line.toFloat();
        compassFile.close();

        prefs.begin("minios", false);
        prefs.putFloat("compassOffset", compassNorthOffset);
        prefs.end();
    }

    logToBootLog("Restored from backup");
    USBSerial.printf("[RESTORE] Restored from: %s\n", backupName.c_str());
    return true;
}

void checkAutoBackup() {
    if (!autoBackupEnabled || !hasSD) return;

    unsigned long currentTime = millis();
    if (currentTime - lastAutoBackup >= AUTO_BACKUP_INTERVAL_MS) {
        if (createBackup(true)) {
            lastAutoBackup = currentTime;
        }
    }
}

String listBackups() {
    String result = "";
    if (!hasSD) return "NO_SD";

    File dir = SD_MMC.open(SD_BACKUP_PATH);
    if (!dir || !dir.isDirectory()) return "NO_BACKUPS";

    File file = dir.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            result += file.name();
            result += "\n";
        }
        file = dir.openNextFile();
    }
    dir.close();

    return result.length() > 0 ? result : "NO_BACKUPS";
}

// ═══════════════════════════════════════════════════════════════════════════
// FUSION LABS WEB SERIAL PROTOCOL (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void sendDeviceStatus() {
    DynamicJsonDocument doc(1024);
    doc["type"] = "WIDGET_STATUS_RESPONSE";
    doc["firmware"] = WIDGET_OS_VERSION;
    doc["protocol"] = FUSION_PROTOCOL_VERSION;
    doc["board"] = DEVICE_SCREEN;
    doc["display"] = "CO5300";
    doc["battery"] = hasPMU ? (int)power.getBatteryPercent() : -1;
    doc["charging"] = hasPMU ? power.isCharging() : false;
    doc["wifi"] = wifiConnected;
    doc["sd_mounted"] = hasSD;
    doc["brightness"] = userData.brightness;
    doc["theme"] = userData.themeIndex;
    doc["wallpaper"] = userData.wallpaperIndex;
    doc["steps"] = userData.steps;
    doc["uptime_ms"] = millis();

    String output;
    serializeJson(doc, output);
    USBSerial.println(output);
}

void sendSDHealth() {
    updateSDCardHealth();

    DynamicJsonDocument doc(512);
    doc["type"] = "WIDGET_SD_HEALTH_RESPONSE";
    doc["mounted"] = sdHealth.mounted;
    doc["healthy"] = sdHealth.healthy;
    doc["total_mb"] = sdHealth.totalBytes / (1024 * 1024);
    doc["used_mb"] = sdHealth.usedBytes / (1024 * 1024);
    doc["free_mb"] = sdHealth.freeBytes / (1024 * 1024);
    doc["used_percent"] = sdHealth.usedPercent;
    doc["write_errors"] = sdHealth.writeErrors;
    doc["read_errors"] = sdHealth.readErrors;
    doc["auto_backup"] = autoBackupEnabled;
    doc["total_backups"] = totalBackups;

    String output;
    serializeJson(doc, output);
    USBSerial.println(output);
}

String readConfigFromSD() {
    if (!hasSD) return "NO_SD";

    File cfg = SD_MMC.open(SD_CONFIG_TXT, FILE_READ);
    if (!cfg) return "NO_CONFIG";

    String content = cfg.readString();
    cfg.close();
    return content;
}

bool saveConfigToSD(const String& configData) {
    if (!hasSD) return false;

    File cfg = SD_MMC.open(SD_CONFIG_TXT, FILE_WRITE);
    if (!cfg) {
        sdHealth.writeErrors++;
        return false;
    }

    cfg.print(configData);
    cfg.close();
    return true;
}

void processWebSerialCommand(const String& cmd) {
    String trimmedCmd = cmd;
    trimmedCmd.trim();

    if (trimmedCmd == "WIDGET_PING") {
        USBSerial.println("WIDGET_PONG");
    }
    else if (trimmedCmd == "WIDGET_STATUS") {
        sendDeviceStatus();
    }
    else if (trimmedCmd == "WIDGET_SD_HEALTH") {
        sendSDHealth();
    }
    else if (trimmedCmd == "WIDGET_READ_CONFIG") {
        String config = readConfigFromSD();
        USBSerial.println(config);
    }
    else if (trimmedCmd == "WIDGET_BACKUP") {
        bool success = createBackup(false);
        USBSerial.println(success ? "BACKUP_OK" : "BACKUP_FAIL");
    }
    else if (trimmedCmd.startsWith("WIDGET_RESTORE:")) {
        String backupName = trimmedCmd.substring(15);
        bool success = restoreFromBackup(backupName);
        USBSerial.println(success ? "RESTORE_OK" : "RESTORE_FAIL");
    }
    else if (trimmedCmd == "WIDGET_LIST_BACKUPS") {
        String backups = listBackups();
        USBSerial.println(backups);
    }
    else if (trimmedCmd == "WIDGET_TOGGLE_AUTO_BACKUP") {
        autoBackupEnabled = !autoBackupEnabled;
        USBSerial.printf("AUTO_BACKUP_%s\n", autoBackupEnabled ? "ON" : "OFF");
    }
    else if (trimmedCmd == "WIDGET_REBOOT") {
        USBSerial.println("REBOOTING...");
        delay(100);
        ESP.restart();
    }
}

void handleFusionLabsProtocol() {
    while (USBSerial.available()) {
        char c = USBSerial.read();

        if (c == '\n') {
            webSerialBuffer[webSerialBufferIndex] = '\0';
            String cmd = String(webSerialBuffer);
            webSerialBufferIndex = 0;

            processWebSerialCommand(cmd);
        } else if (webSerialBufferIndex < WEB_SERIAL_BUFFER_SIZE - 1) {
            webSerialBuffer[webSerialBufferIndex++] = c;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// UI EVENT HANDLER
// ═══════════════════════════════════════════════════════════════════════════
void handle_ui_event(UIEventType event, int param1, int param2) {
    if (event != UI_EVENT_NONE && event != UI_EVENT_REFRESH) {
        USBSerial.printf("[UI_EVENT] %d (canNav=%d, isTrans=%d)\n", event, canNavigate(), isTransitioning);
    }
    switch (event) {
        case UI_EVENT_NAV_LEFT: if (canNavigate()) handleSwipe(50, 0); break;
        case UI_EVENT_NAV_RIGHT: if (canNavigate()) handleSwipe(-50, 0); break;
        case UI_EVENT_NAV_UP: if (canNavigate()) handleSwipe(0, 50); break;
        case UI_EVENT_NAV_DOWN: if (canNavigate()) handleSwipe(0, -50); break;
        case UI_EVENT_TAP: handleTap(param1, param2); break;
        case UI_EVENT_SCREEN_ON: screenOnFunc(); break;
        case UI_EVENT_SCREEN_OFF: screenOff(); break;
        case UI_EVENT_REFRESH: if (screenOn) navigateTo(currentCategory, currentSubCard); break;
        case UI_EVENT_SHUTDOWN: shutdownDevice(); break;
        default: break;
    }
    last_lvgl_response = millis();
}

// ═══════════════════════════════════════════════════════════════════════════
// UI TASK (FROM FIXED - STABLE)
// ═══════════════════════════════════════════════════════════════════════════
void ui_task(void *pvParameters) {
    USBSerial.println("[UI_TASK] Started on Core 1");

    esp_err_t wdt_err = esp_task_wdt_add(xTaskGetCurrentTaskHandle());
    if (wdt_err != ESP_OK) {
        USBSerial.printf("[WDT] Warning: Failed to add ui_task: %d\n", wdt_err);
    } else {
        USBSerial.println("[WDT] ui_task registered with watchdog");
    }

    while (true) {
        esp_task_wdt_reset();
        last_lvgl_response = millis();

        lv_task_handler();

        if (ui_event != UI_EVENT_NONE) {
            UIEventType evt = ui_event;
            int p1 = ui_event_param1, p2 = ui_event_param2;

            ui_event = UI_EVENT_NONE;
            ui_event_param1 = ui_event_param2 = 0;

            handle_ui_event(evt, p1, p2);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void panic_recover() {
    USBSerial.println("[PANIC] Attempting LVGL recovery...");

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1C1C1E), 0);
    lv_obj_t *lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "Recovering...");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFF6B6B), 0);
    lv_obj_center(lbl);
    lv_scr_load(scr);

    isTransitioning = false;
    navigationLocked = false;
    currentCategory = CAT_CLOCK;
    currentSubCard = 0;

    USBSerial.println("[PANIC] Recovery complete, returning to clock");

    lv_timer_create([](lv_timer_t *t) {
        navigateTo(CAT_CLOCK, 0);
        lv_timer_del(t);
    }, 500, NULL);
}

void backlight_manager() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 500) return;
    lastCheck = millis();

    // Use timeout from current battery saver mode
    unsigned long timeout = saverModes[batterySaverLevel].screenTimeoutMs;
    if (screenOn && millis() - last_ui_activity > timeout) {
        ui_event = UI_EVENT_SCREEN_OFF;
    }
}

void check_lvgl_stall() {
    if (millis() - last_lvgl_response > LVGL_STALL_TIMEOUT_MS) {
        USBSerial.println("[STALL] LVGL stall detected! Initiating recovery...");
        panic_recover();
        last_lvgl_response = millis();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// GADGETBRIDGE BLE - SERVER CALLBACKS
// ═══════════════════════════════════════════════════════════════════════════
class GadgetbridgeServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        bleDeviceConnected = true;
        bleLastActivity = millis();
        // Get connected device info
        bleConnectedDeviceName = "Gadgetbridge";  // Default name
        USBSerial.println("[BLE] ✓ Device connected!");
        USBSerial.println("[BLE] Auto-off timer cancelled (device connected)");
        
        // Flash screen to confirm connection (no vibration motor)
        for (int i = 0; i < 2; i++) {
            gfx->setBrightness(255);
            delay(80);
            gfx->setBrightness(100);
            delay(80);
        }
        gfx->setBrightness(saverModes[batterySaverLevel].brightness);
    }

    void onDisconnect(BLEServer* pServer) {
        bleDeviceConnected = false;
        bleConnectedDeviceName = "";
        USBSerial.println("[BLE] Device disconnected");
        // Always restart advertising for auto-reconnect
        if (bleEnabled) {
            delay(500);
            pServer->startAdvertising();
            USBSerial.println("[BLE] Advertising restarted (always-on mode)");
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GADGETBRIDGE BLE - CHARACTERISTIC CALLBACKS (Receive data)
// ═══════════════════════════════════════════════════════════════════════════
class GadgetbridgeCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String rxValue = pCharacteristic->getValue().c_str();
        
        if (rxValue.length() > 0) {
            bleLastActivity = millis();
            
            // Accumulate data (messages may come in chunks)
            for (int i = 0; i < rxValue.length(); i++) {
                char c = rxValue[i];
                if (c == '\n' || c == '\r') {
                    if (bleIncomingBuffer.length() > 0) {
                        USBSerial.printf("[BLE] Received: %s\n", bleIncomingBuffer.c_str());
                        handleGadgetbridgeMessage(bleIncomingBuffer);
                        bleIncomingBuffer = "";
                    }
                } else {
                    bleIncomingBuffer += c;
                }
            }
            
            // Handle message without newline (some clients don't send it)
            if (bleIncomingBuffer.length() > 0 && bleIncomingBuffer.startsWith("{")) {
                // Check if it's a complete JSON
                int braceCount = 0;
                bool inString = false;
                for (int i = 0; i < bleIncomingBuffer.length(); i++) {
                    char c = bleIncomingBuffer[i];
                    if (c == '"' && (i == 0 || bleIncomingBuffer[i-1] != '\\')) inString = !inString;
                    if (!inString) {
                        if (c == '{') braceCount++;
                        if (c == '}') braceCount--;
                    }
                }
                if (braceCount == 0) {
                    USBSerial.printf("[BLE] Received JSON: %s\n", bleIncomingBuffer.c_str());
                    handleGadgetbridgeMessage(bleIncomingBuffer);
                    bleIncomingBuffer = "";
                }
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GADGETBRIDGE PROTOCOL HANDLER
// Handles incoming messages from Gadgetbridge app
// ═══════════════════════════════════════════════════════════════════════════
void handleGadgetbridgeMessage(String message) {
    // Bangle.js sends commands in format: command\n or {json}\n
    message.trim();
    
    // Check for simple text commands first (Gadgetbridge sends these)
    if (message.startsWith("GB(")) {
        // Gadgetbridge JavaScript command - extract and process
        USBSerial.printf("[BLE] GB command: %s\n", message.c_str());
        
        if (message.indexOf("setTime") >= 0) {
            // Extract time from GB({...}) format
            int start = message.indexOf("{");
            int end = message.lastIndexOf("}");
            if (start > 0 && end > start) {
                message = message.substring(start, end + 1);
            }
        }
    }
    
    // Handle plain text commands
    if (message == "BT.println(JSON.stringify(Bangle.getHealthStatus()))" ||
        message.indexOf("getHealthStatus") >= 0) {
        // Send health data
        char response[128];
        snprintf(response, sizeof(response), 
            "{\"steps\":%lu,\"bpm\":0,\"movement\":0}", 
            (unsigned long)userData.steps);
        sendBLEResponse(response);
        return;
    }
    
    if (message.indexOf("getBattery") >= 0) {
        char response[32];
        snprintf(response, sizeof(response), "%d", batteryPercent);
        sendBLEResponse(response);
        return;
    }
    
    // Parse JSON message
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        // Not JSON - might be a simple command, ignore silently
        return;
    }
    
    const char* msgType = doc["t"];
    if (!msgType) {
        return;
    }
    
    USBSerial.printf("[BLE] Message type: %s\n", msgType);
    
    // ═══ TIME SYNC ═══
    if (strcmp(msgType, "setTime") == 0) {
        // Gadgetbridge sends Unix timestamp
        unsigned long unixTime = doc["sec"];
        int tzOffset = doc["tz"] | 0;  // Timezone offset in minutes
        
        if (unixTime > 0) {
            // Convert Unix timestamp to datetime
            time_t rawTime = unixTime;
            struct tm *timeinfo = localtime(&rawTime);
            
            if (hasRTC && timeinfo) {
                rtc.setDateTime(
                    timeinfo->tm_year + 1900,
                    timeinfo->tm_mon + 1,
                    timeinfo->tm_mday,
                    timeinfo->tm_hour,
                    timeinfo->tm_min,
                    timeinfo->tm_sec
                );
                
                // Update display variables
                clockHour = timeinfo->tm_hour;
                clockMinute = timeinfo->tm_min;
                clockSecond = timeinfo->tm_sec;
                
                bleTimeSynced = true;
                bleLastActivityTime = millis();  // Reset auto-off timer
                
                USBSerial.printf("[BLE] ✓ Time synced: %04d-%02d-%02d %02d:%02d:%02d\n",
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                
                // Save fresh time backup after BLE sync
                saveTimeBackup();
                USBSerial.println("[BLE] Time backup saved to SD");
                
                // Send confirmation back to Gadgetbridge
                sendBLEResponse("{\"t\":\"ver\",\"fw\":\"MiniOS 7.4.1\"}");
            }
        }
    }
    // ═══ GET INFO ═══
    else if (strcmp(msgType, "info") == 0) {
        // Gadgetbridge requesting device info
        char response[128];
        snprintf(response, sizeof(response), 
            "{\"t\":\"info\",\"fw\":\"MiniOS 7.4.1\",\"bat\":%d,\"chg\":%s}",
            batteryPercent, isCharging ? "true" : "false");
        sendBLEResponse(response);
    }
    // ═══ FIND DEVICE (make screen flash) ═══
    else if (strcmp(msgType, "find") == 0) {
        bool findOn = doc["n"] | false;
        if (findOn) {
            // Flash screen to help find watch
            for (int i = 0; i < 3; i++) {
                gfx->setBrightness(255);
                delay(200);
                gfx->setBrightness(50);
                delay(200);
            }
            gfx->setBrightness(saverModes[batterySaverLevel].brightness);
        }
        sendBLEResponse("{\"t\":\"find\",\"n\":false}");
    }
    // ═══ BATTERY STATUS REQUEST ═══
    else if (strcmp(msgType, "status") == 0) {
        char response[64];
        snprintf(response, sizeof(response), 
            "{\"t\":\"status\",\"bat\":%d,\"chg\":%s}",
            batteryPercent, isCharging ? "true" : "false");
        sendBLEResponse(response);
    }
    // ═══ NOTIFICATION FROM PHONE ═══
    else if (strcmp(msgType, "notify") == 0) {
        const char* title = doc["title"] | "Notification";
        const char* body = doc["body"] | "";
        const char* src = doc["src"] | "App";
        
        addNotification(String(src), String(title), String(body));
        bleLastActivityTime = millis();  // Reset auto-off timer
        
        USBSerial.printf("[BLE] Notification: %s - %s\n", src, title);
        sendBLEResponse("{\"t\":\"notify\",\"r\":true}");
    }
    // ═══ ACTIVITY/STEP SYNC FROM PHONE ═══
    else if (strcmp(msgType, "act") == 0) {
        // Activity data from Gadgetbridge
        syncedSteps = doc["stp"] | 0;
        syncedCalories = doc["cal"] | 0;
        syncedDistance = doc["dst"] | 0;
        bleLastActivityTime = millis();  // Reset auto-off timer
        stepsSynced = true;
        
        USBSerial.printf("[BLE] Steps synced: %d steps, %d cal\n", syncedSteps, syncedCalories);
        sendBLEResponse("{\"t\":\"act\",\"r\":true}");
    }
    // ═══ MUSIC INFO (for future use) ═══
    else if (strcmp(msgType, "musicinfo") == 0) {
        const char* artist = doc["artist"] | "";
        const char* track = doc["track"] | "";
        USBSerial.printf("[BLE] Music: %s - %s\n", artist, track);
    }
    // ═══ CALL NOTIFICATION ═══
    else if (strcmp(msgType, "call") == 0) {
        const char* cmd = doc["cmd"] | "";
        const char* name = doc["name"] | "Unknown";
        const char* number = doc["number"] | "";
        
        if (strcmp(cmd, "incoming") == 0) {
            addNotification("Phone", String("Incoming Call"), String(name));
        }
    }
    // ═══ ASTRO DATA (Moon Phase, Sunrise/Sunset) ═══
    else if (strcmp(msgType, "astro") == 0) {
        moonPhase = doc["moon"] | 0.0;
        const char* moonName = doc["moonName"] | "---";
        moonPhaseName = String(moonName);
        const char* rise = doc["sunrise"] | "--:--";
        const char* set = doc["sunset"] | "--:--";
        sunriseTime = String(rise);
        sunsetTime = String(set);
        astroDataSynced = true;
        USBSerial.printf("[BLE] Astro: Moon=%s, Rise=%s, Set=%s\n", 
            moonPhaseName.c_str(), sunriseTime.c_str(), sunsetTime.c_str());
    }
    // ═══ VIBRATE/PAIR REQUEST - Auto-confirm (no vibration motor) ═══
    else if (strcmp(msgType, "vibrate") == 0) {
        // Gadgetbridge wants us to vibrate for pairing confirmation
        // We don't have a motor, so just flash screen and auto-confirm
        for (int i = 0; i < 3; i++) {
            gfx->setBrightness(255);
            delay(100);
            gfx->setBrightness(50);
            delay(100);
        }
        gfx->setBrightness(saverModes[batterySaverLevel].brightness);
        // Send confirmation that we "vibrated"
        sendBLEResponse("{\"t\":\"vibrate\",\"n\":false}");
        USBSerial.println("[BLE] Pair vibrate request - flashed screen instead");
    }
    // ═══ HTTP REQUEST (Bangle.js protocol) ═══
    else if (strcmp(msgType, "http") == 0) {
        // Respond that we received it
        sendBLEResponse("{\"t\":\"http\",\"r\":true}");
    }
    // ═══ FIRMWARE INFO REQUEST ═══
    else if (strcmp(msgType, "ver") == 0) {
        sendBLEResponse("{\"t\":\"ver\",\"fw\":\"MiniOS 7.4.1\",\"hw\":\"ESP32-S3\"}");
    }
    // ═══ IS GADGETBRIDGE? - Auto-confirm connection ═══
    else if (strcmp(msgType, "is_gB") == 0) {
        // Gadgetbridge asking if we support it - YES!
        sendBLEResponse("{\"t\":\"is_gB\",\"r\":true}");
        USBSerial.println("[BLE] Gadgetbridge confirmed!");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// SEND BLE RESPONSE
// ═══════════════════════════════════════════════════════════════════════════
void sendBLEResponse(String response) {
    if (bleDeviceConnected && pTxCharacteristic) {
        pTxCharacteristic->setValue(response.c_str());
        pTxCharacteristic->notify();
        USBSerial.printf("[BLE] Sent: %s\n", response.c_str());
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// STOP BLE - Disable Bluetooth to save battery
// ═══════════════════════════════════════════════════════════════════════════
void stopBLE() {
    if (bleEnabled) {
        BLEDevice::deinit(true);
        bleEnabled = false;
        bleDeviceConnected = false;
        bleConnectedDeviceName = "";
        USBSerial.println("[BLE] ✓ Bluetooth disabled to save battery");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// INITIALIZE GADGETBRIDGE BLE
// ═══════════════════════════════════════════════════════════════════════════
void initGadgetbridgeBLE() {
    USBSerial.println("[BLE] ═══ Initializing Gadgetbridge BLE ═══");
    
    // Create BLE Device
    BLEDevice::init(BLE_DEVICE_NAME);
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new GadgetbridgeServerCallbacks());
    
    // Create Nordic UART Service (NUS)
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Create TX Characteristic (Notify - watch to phone)
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());
    
    // Create RX Characteristic (Write - phone to watch)
    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new GadgetbridgeCallbacks());
    
    // Start Service
    pService->start();
    
    // Start Advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    // Mark BLE as enabled and start auto-off timer
    bleEnabled = true;
    bleStartTime = millis();
    bleLastActivityTime = millis();  // Reset activity timer
    
    USBSerial.println("[BLE] ✓ Gadgetbridge BLE ready!");
    USBSerial.printf("[BLE] Device name: %s\n", BLE_DEVICE_NAME);
    USBSerial.println("[BLE] Auto-off in 3 minutes (if no activity)");
}

// ═══════════════════════════════════════════════════════════════════════════
// SMART WIFI CONNECT (FROM 206q)
// ═══════════════════════════════════════════════════════════════════════════
void smartWiFiConnect() {
    USBSerial.println("[WiFi] ═══ Starting Smart WiFi Connect ═══");
    USBSerial.printf("[WiFi] Networks to try: %d\n", numWifiNetworks);
    
    // Try networks in order (slot 0 = hardcoded, slots 1+ = SD card)
    for (int i = 0; i < numWifiNetworks; i++) {
        if (!wifiNetworks[i].valid) continue;

        USBSerial.printf("[WiFi] Trying [%d]: %s %s\n", i, 
                        wifiNetworks[i].ssid,
                        i == 0 ? "(hardcoded)" : "(SD card)");
        
        WiFi.begin(wifiNetworks[i].ssid, wifiNetworks[i].password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            USBSerial.print(".");
            attempts++;
        }
        USBSerial.println();

        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            connectedNetworkIndex = i;
            USBSerial.printf("[WiFi] ✓ Connected to: %s (IP: %s)\n", 
                            wifiNetworks[i].ssid, 
                            WiFi.localIP().toString().c_str());
            
            // WiFi success - delete old time backup, NTP will provide fresh time
            deleteTimeBackup();
            USBSerial.println("[WiFi] ═══ WiFi Connected Successfully ═══");
            return;
        } else {
            USBSerial.printf("[WiFi] ✗ Failed to connect to: %s\n", wifiNetworks[i].ssid);
        }
    }

    wifiConnected = false;
    USBSerial.println("[WiFi] ✗ Failed to connect to any network");
    
    // WiFi FAILED - restore time from SD backup!
    if (hasTimeBackup()) {
        USBSerial.println("[TIME] WiFi failed - restoring time from SD backup...");
        restoreTimeBackup();
    } else {
        USBSerial.println("[TIME] No time backup available on SD card");
    }
    USBSerial.println("[WiFi] ═══ WiFi Connection Failed ═══");
}

// ═══════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════
void setup() {
    USBSerial.begin(115200);
    delay(100);

    USBSerial.println("═══════════════════════════════════════════════════════════════");
    USBSerial.println("  S3 MiniOS v7.4 - MERGED LOW POWER + NIKE ENHANCED EDITION");
    USBSerial.println("  Full Features + Light Sleep (EXTREME mode + <20% battery)");
    USBSerial.println("═══════════════════════════════════════════════════════════════");

    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 10000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    esp_err_t wdt_init_err = esp_task_wdt_init(&wdt_config);
    if (wdt_init_err == ESP_ERR_INVALID_STATE) {
        USBSerial.println("[WDT] Already initialized, reconfiguring...");
        esp_task_wdt_deinit();
        esp_task_wdt_init(&wdt_config);
    }
    USBSerial.println("[WDT] Watchdog initialized (10s timeout)");

    pinMode(PWR_BUTTON, INPUT_PULLUP);
    pinMode(BOOT_BUTTON, INPUT_PULLUP);

    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setClock(400000);

    pinMode(LCD_RESET, OUTPUT);
    pinMode(TP_RESET, OUTPUT);
    digitalWrite(LCD_RESET, LOW);
    digitalWrite(TP_RESET, LOW);
    delay(20);
    digitalWrite(LCD_RESET, HIGH);
    digitalWrite(TP_RESET, HIGH);
    delay(50);

    gfx->begin();
    gfx->setBrightness(100);  // Start at medium brightness to save power
    gfx->fillScreen(0x0000);

    pinMode(TP_INT, INPUT_PULLUP);
    while (!FT3168->begin()) delay(1000);
    FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                   FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);
    attachInterrupt(digitalPinToInterrupt(TP_INT), Arduino_IIC_Touch_Interrupt, FALLING);

    if (qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
        // LOW POWER: 62.5Hz is enough for step counting, disable gyro
        qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_62_5Hz);
        qmi.enableAccelerometer();
        // DISABLE gyroscope to save power (only need accel for steps)
        qmi.disableGyroscope();
        hasIMU = true;
        USBSerial.println("[IMU] QMI8658 initialized (low power mode)");
        
        // Initialize step counter for low-power mode
        initStepCounter();
    }

    if (rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
        hasRTC = true;
        rtc.setDateTime(2025, 1, 26, 12, 0, 0);
        USBSerial.println("[RTC] PCF85063 initialized");
    }

    if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
        hasPMU = true;
        power.disableTSPinMeasure();
        power.enableBattVoltageMeasure();
        USBSerial.println("[PMU] AXP2101 initialized");
    }

    // Initialize SD card with full Widget OS folder structure (FROM 206q)
    if (initWidgetOSSDCard()) {
        USBSerial.printf("[SD] %s card mounted (%lluMB)\n", sdCardType.c_str(), sdCardSizeMB);
        updateSDCardHealth();
        USBSerial.printf("[SD] Total: %lluMB, Used: %lluMB (%.1f%%)\n", 
                        sdHealth.totalBytes / (1024*1024),
                        sdHealth.usedBytes / (1024*1024),
                        sdHealth.usedPercent);
    } else {
        USBSerial.println("[SD] No SD card or mount failed");
    }

    loadUserData();

    prefs.begin("minios", true);
    compassNorthOffset = prefs.getFloat("compassOffset", 0.0);
    prefs.end();

    // ═══ WIFI CONNECTION PRIORITY ═══
    // 1. Hardcoded WiFi (primary - always add first)
    // 2. SD card WiFi networks (additional options)
    
    // Add hardcoded WiFi as primary (slot 0)
    strncpy(wifiNetworks[0].ssid, "Optus_9D2E3D", sizeof(wifiNetworks[0].ssid) - 1);
    strncpy(wifiNetworks[0].password, "snucktemptGLeQU", sizeof(wifiNetworks[0].password) - 1);
    wifiNetworks[0].valid = true;
    numWifiNetworks = 1;
    USBSerial.println("[WiFi] Hardcoded network added as primary");
    
    // Load additional WiFi networks from SD card (slots 1-4)
    if (hasSD) {
        loadWiFiConfigFromSD();  // This appends to existing networks
        USBSerial.printf("[WiFi] Total networks available: %d\n", numWifiNetworks);
    }

    // Smart WiFi connect - tries all networks in order
    // If connected: deletes time backup (NTP will sync fresh time)
    // If failed: restores time from SD backup
    smartWiFiConnect();

    if (wifiConnected) {
        USBSerial.println("[WiFi] Connected - syncing time and weather");
        
        // Sync time via NTP
        configTime(gmtOffsetSec, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        delay(2000);

        // Update RTC from NTP
        if (hasRTC) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                rtc.setDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                ntpSyncedOnce = true;
                lastNTPSync = millis();
                USBSerial.println("[NTP] Time synced to RTC");
                
                // Save fresh time backup after NTP sync
                saveTimeBackup();
                USBSerial.println("[TIME] Fresh backup saved after NTP sync");
            }
        }

        // Auto-detect location and fetch weather (FROM 206q)
        fetchLocationFromIP();
        
        // POWER SAVE: Disconnect WiFi after initial sync
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        wifiConnected = false;
        USBSerial.println("[WiFi] Disconnected after sync (power save)");
    }

    // ═══ SD CARD TIME OFFSET (if WiFi failed) ═══
    if (!wifiConnected && hasTimeBackup()) {
        restoreTimeBackup();
        USBSerial.println("[TIME] Restored from SD card backup");
    }

    // ═══ GADGETBRIDGE BLE - START AFTER WiFi/SD ═══
    // Boot order: WiFi sync → SD card backup → BLE (3 min timer)
    USBSerial.println("[BLE] Starting Bluetooth (3 min auto-off)...");
    initGadgetbridgeBLE();
    
    // Initialize deep sleep timer
    lastTouchTime = millis();

    lv_init();

    size_t buf_size = LCD_WIDTH * 50 * sizeof(lv_color_t);
    buf1 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buf1 || !buf2) {
        buf1 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        buf2 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_WIDTH * 50);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    const esp_timer_create_args_t timer_args = { .callback = &lvgl_tick_cb, .name = "lvgl_tick" };
    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, LVGL_TICK_PERIOD_MS * 1000);

    lastActivityMs = millis();
    screenOnStartMs = millis();
    last_ui_activity = millis();
    last_lvgl_response = millis();
    batteryStats.sessionStartMs = millis();
    lastAutoBackup = millis();  // Reset auto backup timer

    // Initialize power consumption logging to SD card - DISABLED (causes freezing)
    // initPowerLogging();
    // USBSerial.println("[POWER] Power consumption logging initialized");

    // Apply saved battery saver level
    applyBatterySaverMode(batterySaverLevel);

    xTaskCreatePinnedToCore(ui_task, "ui_task", 10240, NULL, 2, &ui_task_handle, 1);
    USBSerial.println("[UI_TASK] Created with 10KB stack on Core 1");

    ui_event = UI_EVENT_REFRESH;

    USBSerial.println("═══════════════════════════════════════════════════════════════");
    USBSerial.printf("  Battery: %d%% | Saver: %s | Light Sleep: %s\n", 
        batteryPercent, saverModes[batterySaverLevel].name,
        shouldEnableLightSleep() ? "READY" : "STANDBY");
    USBSerial.println("  Light sleep activates when: EXTREME mode + battery < 20%");
    USBSerial.println("═══════════════════════════════════════════════════════════════");
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════
void loop() {
    // ═══ VOICE MEMO RECORDING ═══
    if (voiceMemoRecording) {
        recordVoiceSamples();
    }

    // ═══ BLE AUTO-OFF CHECK (3 minute timeout if no activity) ═══
    if (bleEnabled && bleAutoOffEnabled) {
        // Check if 3 minutes passed with no activity (no connection, no notifications)
        unsigned long timeSinceActivity = millis() - bleLastActivityTime;
        if (!bleDeviceConnected && timeSinceActivity > BLE_AUTO_OFF_MS) {
            USBSerial.println("[BLE] Auto-off: 3 minute timeout (no activity)");
            stopBLE();
        }
    }

    // ═══ POWER BUTTON - FIXED: Simple debounced toggle ═══
    static bool lastPwrState = HIGH;
    static unsigned long lastPwrToggle = 0;
    const unsigned long BUTTON_DEBOUNCE_MS = 200;

    bool currentPwrState = digitalRead(PWR_BUTTON);

    // Detect button press with debounce
    if (lastPwrState == HIGH && currentPwrState == LOW) {
        if (millis() - lastPwrToggle > BUTTON_DEBOUNCE_MS) {
            lastPwrToggle = millis();
            lastTouchTime = millis();  // Reset deep sleep timer
            if (screenOn) {
                screenOff();
            } else {
                screenOnFunc();
            }
        }
    }
    lastPwrState = currentPwrState;

    // ═══ BOOT BUTTON HANDLING (optional secondary button) ═══
    static bool lastBootState = HIGH;
    bool currentBootState = digitalRead(BOOT_BUTTON);
    
    if (lastBootState == HIGH && currentBootState == LOW) {
        lastTouchTime = millis();  // Reset deep sleep timer
        // Boot button pressed - wake screen if off, or go to clock if on
        if (!screenOn) {
            ui_event = UI_EVENT_SCREEN_ON;
        } else {
            // Navigate to clock
            currentCategory = CAT_CLOCK;
            currentSubCard = 0;
            navigateTo(currentCategory, currentSubCard);
        }
    }
    lastBootState = currentBootState;

    // ═══ TOUCH TO WAKE - Wake screen on any touch ═══
    // Only check touch wake periodically to avoid false wakes from touch noise
    static unsigned long lastTouchWakeCheck = 0;
    if (!screenOn && millis() - lastTouchWakeCheck > 200) {  // Check every 200ms
        lastTouchWakeCheck = millis();
        uint8_t fingers = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);
        // Require sustained touch (check twice with delay) to prevent false wakes
        if (fingers > 0) {
            delay(50);  // Short delay
            fingers = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);
            if (fingers > 0) {
                USBSerial.println("[WAKE] Touch detected - waking screen");
                lastTouchTime = millis();  // Reset deep sleep timer
                ui_event = UI_EVENT_SCREEN_ON;
            }
        }
    }

    // Handle Fusion Labs Web Serial Protocol (FROM 206q)
    handleFusionLabsProtocol();

    // Check 24-hour auto backup (FROM 206q)
    checkAutoBackup();

    check_lvgl_stall();
    backlight_manager();

    if (hasRTC && millis() - lastClockUpdate >= 1000) {
        lastClockUpdate = millis();
        RTC_DateTime dt = rtc.getDateTime();
        clockHour = dt.getHour();
        clockMinute = dt.getMinute();
        clockSecond = dt.getSecond();
    }

    if (hasIMU && millis() - lastStepUpdate >= sensorPollInterval) {
        lastStepUpdate = millis();
        updateSensorFusion();
        updateStepCount();
    }

    if (hasPMU && millis() - lastBatteryUpdate >= 3000) {
        lastBatteryUpdate = millis();
        batteryVoltage = power.getBattVoltage();
        batteryPercent = power.getBatteryPercent();
        isCharging = power.isCharging();
        freeRAM = ESP.getFreeHeap();
    }

    if (millis() - lastSaveTime >= SAVE_INTERVAL_MS) saveUserData();

    // Weather update - SCHEDULED SYNC (hourly in saver modes for ~15% power saving)
    unsigned long weatherInterval = (batterySaverMode) ? WEATHER_SYNC_INTERVAL_MS : 1800000;  // 1hr vs 30min
    if (wifiConnected && !saverModes[batterySaverLevel].disableWifiSync && 
        millis() - lastWeatherUpdate >= weatherInterval) {
        fetchWeatherData();
        lastWeatherSync = millis();
    }

    // NTP resync every hour (FROM 206q)
    if (wifiConnected && millis() - lastNTPSync >= 3600000) {
        configTime(gmtOffsetSec, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        if (hasRTC) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                rtc.setDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            }
        }
        lastNTPSync = millis();
    }

    // Clock refresh
    if (screenOn && currentCategory == CAT_CLOCK && !isTransitioning) {
        static unsigned long lastRef = 0;
        if (millis() - lastRef >= 5000) {
            lastRef = millis();
            ui_event = UI_EVENT_REFRESH;
        }
    }

    // Compass refresh
    if (screenOn && currentCategory == CAT_COMPASS && !isTransitioning) {
        static unsigned long lastRef = 0;
        if (millis() - lastRef >= 500) {
            lastRef = millis();
            ui_event = UI_EVENT_REFRESH;
        }
    }

    // Stopwatch refresh
    if (stopwatchRunning && screenOn && currentCategory == CAT_TIMER && !isTransitioning) {
        static unsigned long lastRef = 0;
        if (millis() - lastRef >= 100) {
            lastRef = millis();
            ui_event = UI_EVENT_REFRESH;
        }
    }
    
    // Running mode refresh
    if (runningModeActive && screenOn && currentCategory == CAT_ACTIVITY && currentSubCard == 2 && !isTransitioning) {
        static unsigned long lastRef = 0;
        if (millis() - lastRef >= 500) {
            lastRef = millis();
            ui_event = UI_EVENT_REFRESH;
        }
    }

    // ═══ AMOLED BURN-IN PREVENTION ═══
    if (screenOn) {
        updateBurnInOffset();
    }
    
    // ═══ DEEP SLEEP CHECK (4 min no touch) ═══
    if (!screenOn) {
        checkDeepSleepTimeout();
    }

    // ═══ POWER CONSUMPTION LOGGING - DISABLED (was causing device freezes) ═══
    // logPowerConsumption();
    // writePowerSummary();
    
    // Log rotation and daily archiving - DISABLED
    // rotatePowerLogIfNeeded();
    // archiveDailyPowerLog();

    delay(10);
}
