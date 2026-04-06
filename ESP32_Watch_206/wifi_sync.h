/*
 * wifi_sync.h - WiFi Boot Sync System
 * FUSION OS Feature
 * 
 * Functionality:
 * - Scans hardcoded WiFi network
 * - Loads additional networks from SD: /WATCH/wifi/config.txt
 * - Tries each network TWICE before moving to next
 * - Syncs NTP time to RTC chip
 * - Fetches weather data
 * - Disconnects after sync to save power
 */

#ifndef WIFI_SYNC_H
#define WIFI_SYNC_H

#include <Arduino.h>
#include <WiFi.h>

// =============================================================================
// CONFIGURATION
// =============================================================================
#define MAX_WIFI_NETWORKS    10
#define WIFI_CONNECT_TIMEOUT 5000   // 5 seconds per attempt (was 15s - too long!)
#define WIFI_RETRY_PER_NETWORK 1     // Try each network once (was 2)
#define NTP_SERVER          "pool.ntp.org"
#define NTP_TIMEOUT         5000    // 5 seconds (was 10s)

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
// FUNCTIONS
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

#endif // WIFI_SYNC_H
