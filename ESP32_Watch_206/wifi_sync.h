/*
 * wifi_sync.h - WiFi Boot Sync System
 * FUSION OS Feature
 * 
 * Functionality:
 * - Scans hardcoded WiFi network
 * - Loads additional networks from SD: /WATCH/wifi/config.txt
 * - Tries each network TWICE before moving to next
 * - Syncs NTP time to RTC chip (time.google.com)
 * - Fetches weather data
 * - Disconnects after sync to save power
 * 
 * UPDATED: Google NTP + GMT+8 timezone support
 */

#ifndef WIFI_SYNC_H
#define WIFI_SYNC_H

#include <Arduino.h>
#include <WiFi.h>

// =============================================================================
// CONFIGURATION
// =============================================================================
#define MAX_WIFI_NETWORKS    10
#define WIFI_CONNECT_TIMEOUT 5000   // 5 seconds per attempt
#define WIFI_RETRY_PER_NETWORK 1    // Try each network once

// NTP Configuration - Google Time Servers
#define NTP_SERVER "time.google.com"
#define NTP_SERVER_BACKUP1 "time1.google.com"
#define NTP_SERVER_BACKUP2 "time2.google.com"
#define NTP_TIMEOUT 5000  // 5 seconds

// Timezone Configuration (GMT+8 default - Malaysia/Singapore/Perth)
// Offset in seconds: GMT+8 = 8 * 3600 = 28800
#define TIMEZONE_OFFSET_SEC 28800   // GMT+8 in seconds
#define DAYLIGHT_OFFSET_SEC 0       // No DST
#define TIMEZONE_NAME "MYT"         // Malaysia Time (or SGT, AWST, etc.)

// =============================================================================
// STRUCTURES
// =============================================================================
struct WiFiCredential {
  char ssid[64];
  char password[64];
  bool valid;
};

struct WiFiSyncState {
  bool sync_enabled;
  bool last_sync_success;
  unsigned long last_sync_time;
  int networks_loaded;
  WiFiCredential networks[MAX_WIFI_NETWORKS];
};

extern WiFiSyncState wifi_sync_state;

// =============================================================================
// CORE FUNCTIONS
// =============================================================================

// Initialize WiFi sync system
void initWiFiSync();

// Perform boot-time WiFi sync (blocking)
bool performBootSync();

// Load WiFi networks from hardcoded + SD card
int loadWiFiNetworks();

// Try to connect to a specific network
bool connectToWiFi(const char* ssid, const char* password, int timeout_ms);

// Sync time via NTP
bool syncNTPTime();

// Fetch weather data
bool fetchWeather();

// Disconnect WiFi and turn off radio
void disconnectWiFi();

// Check if time since last sync
unsigned long timeSinceLastSync();

// Manual sync trigger (for settings menu)
bool triggerManualSync();

// =============================================================================
// TIMEZONE CONFIGURATION FUNCTIONS
// =============================================================================

// Set timezone offset from GMT (e.g., setTimezoneOffset(8, 0) for GMT+8)
void setTimezoneOffset(int hours, int minutes);

// Get current timezone offset
int getTimezoneOffsetHours();
int getTimezoneOffsetMinutes();

// Daylight saving time control
void setDaylightSaving(bool enabled);
bool isDaylightSavingEnabled();

// Get formatted timezone string (e.g., "GMT+8" or "GMT+5:30")
void getTimezoneString(char* buffer, size_t bufferSize);

// =============================================================================
// COMMON TIMEZONE PRESETS (use with setTimezoneOffset)
// =============================================================================
#define TZ_GMT_PLUS_8   8, 0    // Malaysia, Singapore, Perth, Hong Kong, Beijing
#define TZ_GMT_PLUS_9   9, 0    // Japan, Korea
#define TZ_GMT_PLUS_7   7, 0    // Thailand, Vietnam, Indonesia (WIB)
#define TZ_GMT_PLUS_530 5, 30   // India
#define TZ_GMT_PLUS_0   0, 0    // UK, Portugal (GMT/UTC)
#define TZ_GMT_MINUS_5  -5, 0   // US Eastern (EST)
#define TZ_GMT_MINUS_8  -8, 0   // US Pacific (PST)
#define TZ_GMT_PLUS_10  10, 0   // Australia Eastern (AEST)
#define TZ_GMT_PLUS_12  12, 0   // New Zealand

#endif // WIFI_SYNC_H
