/*
 * wifi_sync.cpp - WiFi Boot Sync Implementation (FIXED)
 * FUSION OS Feature
 * 
 * FIX: Added quick scan - skips sync entirely if target network not visible
 * This prevents infinite loop when offline
 * 
 * NOTE: SD Card support DISABLED due to hardware conflict
 * GPIO 4,5,6,7 are shared between QSPI display and SD_MMC
 * 
 * UPDATED: Google NTP + GMT+8 timezone support
 */

#include "wifi_sync.h"
#include "config.h"
#include "hardware.h"
#include <WiFi.h>
#include <time.h>

// Timezone configuration (can be changed via settings later)
static int timezone_offset_hours = 8;  // Default GMT+8
static int timezone_offset_minutes = 0;
static bool daylight_saving = false;

// SD_MMC removed - hardware conflict with QSPI display
extern WiFiState wifi_state;
extern SystemState system_state;
// No external RTC object needed - using hardware.cpp functions

WiFiSyncState wifi_sync_state = {
  .sync_enabled = true,
  .last_sync_success = false,
  .last_sync_time = 0,
  .networks_loaded = 0
};

// Hardcoded WiFi network (fallback)
const char* HARDCODED_SSID = "Optus_9D2E3D";
const char* HARDCODED_PASSWORD = "snucktemptGLeQU";

// =============================================================================
// CONSTANTS - REDUCED TIMEOUTS
// =============================================================================
#define WIFI_RETRY_PER_NETWORK 2       // Reduced from 3
#define WIFI_CONNECT_TIMEOUT 5000      // Reduced from 10000ms to 5 seconds
#define NTP_TIMEOUT 5000               // Reduced from 10000ms to 5 seconds

// =============================================================================
// QUICK NETWORK SCAN - NEW FUNCTION (FIX FOR INFINITE LOOP)
// Returns true if target network is visible, false otherwise
// =============================================================================
bool quickNetworkScan() {
  extern void feedWatchdog();
  
  Serial.println("[WiFiSync] Quick scan: Checking for available networks...");
  feedWatchdog();
  
  // Set WiFi to station mode for scanning
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  feedWatchdog();
  
  // Do a quick scan - 300ms per channel is fast
  Serial.println("[WiFiSync] Scanning...");
  int scanResult = WiFi.scanNetworks(false, false, false, 300);
  feedWatchdog();
  
  if (scanResult <= 0) {
    Serial.println("[WiFiSync] Quick scan: NO NETWORKS FOUND");
    Serial.println("[WiFiSync] -> Skipping WiFi sync (no networks in range)");
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  Serial.printf("[WiFiSync] Quick scan: Found %d networks\n", scanResult);
  
  // Check if our target network is in the list
  bool targetFound = false;
  for (int i = 0; i < scanResult; i++) {
    feedWatchdog();
    String foundSSID = WiFi.SSID(i);
    if (foundSSID.equals(HARDCODED_SSID)) {
      targetFound = true;
      Serial.printf("[WiFiSync] Quick scan: TARGET FOUND '%s' (RSSI: %d dBm)\n", 
                    HARDCODED_SSID, WiFi.RSSI(i));
      break;
    }
  }
  
  // Clean up scan results
  WiFi.scanDelete();
  
  if (!targetFound) {
    Serial.printf("[WiFiSync] Quick scan: Target network '%s' NOT VISIBLE\n", HARDCODED_SSID);
    Serial.println("[WiFiSync] -> Skipping WiFi sync (target not in range)");
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  Serial.println("[WiFiSync] Quick scan: Target network available, proceeding with sync");
  return true;
}

// =============================================================================
// INITIALIZE WIFI SYNC
// =============================================================================
void initWiFiSync() {
  Serial.println("[WiFiSync] Initializing WiFi sync system");
  Serial.printf("[WiFiSync] Default timezone: GMT%+d\n", timezone_offset_hours);
  
  // Ensure WiFi is off initially
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // Load networks from SD card and hardcoded
  wifi_sync_state.networks_loaded = loadWiFiNetworks();
  
  Serial.printf("[WiFiSync] Loaded %d WiFi networks\n", wifi_sync_state.networks_loaded);
}

// =============================================================================
// LOAD WIFI NETWORKS
// =============================================================================
int loadWiFiNetworks() {
  int count = 0;
  
  // Add hardcoded network (ONLY source - SD card disabled due to hardware conflict)
  strcpy(wifi_sync_state.networks[count].ssid, HARDCODED_SSID);
  strcpy(wifi_sync_state.networks[count].password, HARDCODED_PASSWORD);
  wifi_sync_state.networks[count].valid = true;
  count++;
  
  Serial.println("[WiFiSync] ✓ Hardcoded WiFi network loaded: Optus_9D2E3D");
  Serial.println("[WiFiSync] ℹ SD Card disabled (hardware conflict with display)");
  Serial.println("[WiFiSync] → To add more networks, update wifi_sync.cpp hardcoded credentials");
  
  return count;  // Return 1 network (hardcoded only)
}

// =============================================================================
// PERFORM BOOT SYNC - FIXED WITH QUICK SCAN
// =============================================================================
bool performBootSync() {
  extern void feedWatchdog();  // External watchdog feed function
  
  if (!wifi_sync_state.sync_enabled) {
    Serial.println("[WiFiSync] Sync disabled, skipping");
    feedWatchdog();
    return false;
  }
  
  Serial.println("[WiFiSync] ===== BOOT SYNC START =====");
  feedWatchdog();
  
  // =========================================================================
  // FIX: QUICK SCAN FIRST - Skip entirely if target network not visible
  // This prevents the infinite loop when offline!
  // =========================================================================
  if (!quickNetworkScan()) {
    Serial.println("[WiFiSync] ===== BOOT SYNC SKIPPED (No WiFi) =====");
    wifi_sync_state.last_sync_success = false;
    return false;  // Exit immediately - no waiting, no retries!
  }
  
  Serial.printf("[WiFiSync] Attempting to connect to %d networks...\n", 
                wifi_sync_state.networks_loaded);
  feedWatchdog();
  
  bool connected = false;
  
  // Try each network with REDUCED retries (was 3, now 2)
  for (int net = 0; net < wifi_sync_state.networks_loaded && !connected; net++) {
    WiFiCredential* cred = &wifi_sync_state.networks[net];
    if (!cred->valid) continue;
    
    Serial.printf("[WiFiSync] Trying network: %s\n", cred->ssid);
    feedWatchdog();  // Feed before each network attempt
    
    for (int attempt = 1; attempt <= WIFI_RETRY_PER_NETWORK && !connected; attempt++) {
      Serial.printf("[WiFiSync]   Attempt %d/%d...\n", attempt, WIFI_RETRY_PER_NETWORK);
      feedWatchdog();  // Feed before each connection attempt
      
      if (connectToWiFi(cred->ssid, cred->password, WIFI_CONNECT_TIMEOUT)) {
        connected = true;
        Serial.printf("[WiFiSync] ✓ Connected to %s\n", cred->ssid);
        break;
      } else {
        Serial.printf("[WiFiSync]   Failed (attempt %d)\n", attempt);
        feedWatchdog();
        delay(300);  // Reduced from 500ms
      }
    }
  }
  
  feedWatchdog();
  
  if (!connected) {
    Serial.println("[WiFiSync] ✗ Failed to connect to any network");
    WiFi.mode(WIFI_OFF);
    wifi_sync_state.last_sync_success = false;
    return false;
  }
  
  // Connected! Now sync time and weather
  bool time_synced = syncNTPTime();
  feedWatchdog();
  bool weather_fetched = fetchWeather();
  feedWatchdog();
  
  wifi_sync_state.last_sync_success = (time_synced || weather_fetched);
  wifi_sync_state.last_sync_time = millis();
  
  // Disconnect to save power
  disconnectWiFi();
  
  Serial.println("[WiFiSync] ===== BOOT SYNC COMPLETE =====");
  Serial.printf("[WiFiSync] Time: %s, Weather: %s\n",
                time_synced ? "✓" : "✗",
                weather_fetched ? "✓" : "✗");
  
  return wifi_sync_state.last_sync_success;
}

// =============================================================================
// CONNECT TO WIFI - Reduced delay for faster response
// =============================================================================
bool connectToWiFi(const char* ssid, const char* password, int timeout_ms) {
  extern void feedWatchdog();  // External watchdog feed function
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
    delay(250);  // Reduced from 500ms for faster checking
    feedWatchdog();  // CRITICAL: Feed watchdog to prevent timeout during WiFi connection
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFiSync] IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  
  WiFi.disconnect();
  return false;
}

// =============================================================================
// SYNC NTP TIME - With Timezone Support (Default GMT+8)
// =============================================================================
bool syncNTPTime() {
  extern void feedWatchdog();

  if (WiFi.status() != WL_CONNECTED) return false;

  Serial.println("[WiFiSync] Syncing NTP time from time.google.com...");
  Serial.printf("[WiFiSync] Timezone: GMT%+d:%02d\n", timezone_offset_hours, timezone_offset_minutes);

  // Calculate total offset in seconds
  long gmtOffset_sec = (timezone_offset_hours * 3600) + (timezone_offset_minutes * 60);
  int daylightOffset_sec = daylight_saving ? 3600 : 0;

  // Configure NTP with timezone - Using Google's reliable NTP servers
  configTime(gmtOffset_sec, daylightOffset_sec, 
             "time.google.com", 
             "time1.google.com", 
             "time2.google.com");

  // Wait for time sync with timeout
  struct tm timeinfo;
  unsigned long start = millis();
  int attempts = 0;

  Serial.println("[WiFiSync] Waiting for NTP response...");
  
  while (!getLocalTime(&timeinfo) && (millis() - start) < NTP_TIMEOUT) {
    delay(100);
    feedWatchdog();
    attempts++;
    if (attempts % 10 == 0) {
      Serial.printf("[WiFiSync] Still waiting... (%d ms)\n", (int)(millis() - start));
    }
  }

  feedWatchdog();

  if (getLocalTime(&timeinfo)) {
    // Successfully got time - update RTC chip
    WatchTime new_time;
    new_time.year = timeinfo.tm_year + 1900;
    new_time.month = timeinfo.tm_mon + 1;
    new_time.day = timeinfo.tm_mday;
    new_time.hour = timeinfo.tm_hour;
    new_time.minute = timeinfo.tm_min;
    new_time.second = timeinfo.tm_sec;
    new_time.weekday = timeinfo.tm_wday;

    // Set the hardware RTC
    setCurrentTime(new_time);

    Serial.println("[WiFiSync] ========== TIME SYNC SUCCESS ==========");
    Serial.printf("[WiFiSync] UTC Time would be: %02d:%02d:%02d\n",
                  (new_time.hour - timezone_offset_hours + 24) % 24,
                  new_time.minute,
                  new_time.second);
    Serial.printf("[WiFiSync] Local Time (GMT%+d): %04d-%02d-%02d %02d:%02d:%02d\n",
                  timezone_offset_hours,
                  new_time.year,
                  new_time.month,
                  new_time.day,
                  new_time.hour,
                  new_time.minute,
                  new_time.second);
    Serial.printf("[WiFiSync] Day of week: %d (0=Sun, 1=Mon, ...)\n", new_time.weekday);
    Serial.println("[WiFiSync] ==========================================");

    return true;
  }

  Serial.println("[WiFiSync] ✗ NTP sync failed - timeout");
  Serial.println("[WiFiSync] Check: WiFi connected? Firewall blocking NTP?");
  return false;
}

// =============================================================================
// FETCH WEATHER
// =============================================================================
bool fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return false;
  
  Serial.println("[WiFiSync] Fetching weather...");
  
  // TODO: Implement weather API call
  // For now, this is a placeholder
  // You can integrate OpenWeatherMap or similar API here
  
  Serial.println("[WiFiSync] Weather fetch not implemented (placeholder)");
  return false;
}

// =============================================================================
// DISCONNECT WIFI
// =============================================================================
void disconnectWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  wifi_state = WIFI_DISCONNECTED;
  system_state.wifi_connected = false;
  Serial.println("[WiFiSync] WiFi disconnected to save power");
}

// =============================================================================
// TIME SINCE LAST SYNC
// =============================================================================
unsigned long timeSinceLastSync() {
  if (wifi_sync_state.last_sync_time == 0) return 0xFFFFFFFF;
  return millis() - wifi_sync_state.last_sync_time;
}

// =============================================================================
// MANUAL SYNC TRIGGER
// =============================================================================
bool triggerManualSync() {
  Serial.println("[WiFiSync] Manual sync triggered");
  return performBootSync();
}

// =============================================================================
// TIMEZONE CONFIGURATION FUNCTIONS
// =============================================================================

// Set timezone offset (hours and minutes from GMT)
void setTimezoneOffset(int hours, int minutes) {
  timezone_offset_hours = hours;
  timezone_offset_minutes = minutes;
  Serial.printf("[WiFiSync] Timezone set to GMT%+d:%02d\n", hours, minutes);
}

// Get current timezone offset in hours
int getTimezoneOffsetHours() {
  return timezone_offset_hours;
}

// Get current timezone offset in minutes (the extra minutes part)
int getTimezoneOffsetMinutes() {
  return timezone_offset_minutes;
}

// Enable/disable daylight saving time
void setDaylightSaving(bool enabled) {
  daylight_saving = enabled;
  Serial.printf("[WiFiSync] Daylight saving: %s\n", enabled ? "ON" : "OFF");
}

// Check if daylight saving is enabled
bool isDaylightSavingEnabled() {
  return daylight_saving;
}

// Get timezone string (e.g., "GMT+8:00")
void getTimezoneString(char* buffer, size_t bufferSize) {
  if (timezone_offset_minutes == 0) {
    snprintf(buffer, bufferSize, "GMT%+d", timezone_offset_hours);
  } else {
    snprintf(buffer, bufferSize, "GMT%+d:%02d", timezone_offset_hours, abs(timezone_offset_minutes));
  }
}
