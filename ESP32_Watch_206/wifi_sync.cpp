/*
 * wifi_sync.cpp - WiFi Boot Sync Implementation
 * FUSION OS Feature
 * 
 * NOTE: SD Card support DISABLED due to hardware conflict
 * GPIO 4,5,6,7 are shared between QSPI display and SD_MMC
 */

#include "wifi_sync.h"
#include "config.h"
#include "hardware.h"
#include <WiFi.h>
#include <time.h>
// SD_MMC removed - hardware conflict with QSPI display

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
// INITIALIZE WIFI SYNC
// =============================================================================
void initWiFiSync() {
  Serial.println("[WiFiSync] Initializing WiFi sync system");
  
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
// PERFORM BOOT SYNC
// =============================================================================
bool performBootSync() {
  extern void feedWatchdog();  // External watchdog feed function
  
  if (!wifi_sync_state.sync_enabled) {
    Serial.println("[WiFiSync] Sync disabled, skipping");
    feedWatchdog();
    return false;
  }
  
  Serial.println("[WiFiSync] ===== BOOT SYNC START =====");
  Serial.printf("[WiFiSync] Attempting to connect to %d networks...\n", 
                wifi_sync_state.networks_loaded);
  feedWatchdog();
  
  bool connected = false;
  
  // Try each network TWICE before moving to next
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
        delay(500);  // Reduced from 1000ms
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
// CONNECT TO WIFI
// =============================================================================
bool connectToWiFi(const char* ssid, const char* password, int timeout_ms) {
  extern void feedWatchdog();  // External watchdog feed function
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
    delay(500);
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
// SYNC NTP TIME
// =============================================================================
bool syncNTPTime() {
  extern void feedWatchdog();  // External watchdog feed function
  
  if (WiFi.status() != WL_CONNECTED) return false;
  
  Serial.println("[WiFiSync] Syncing NTP time...");
  
  // Configure NTP
  configTime(0, 0, NTP_SERVER);  // GMT+0, no DST
  
  // Wait for time sync
  struct tm timeinfo;
  unsigned long start = millis();
  
  while (!getLocalTime(&timeinfo) && (millis() - start) < NTP_TIMEOUT) {
    delay(100);
    feedWatchdog();  // CRITICAL: Feed watchdog during NTP sync
  }
  
  if (getLocalTime(&timeinfo)) {
    // Update RTC chip using hardware function
    WatchTime new_time;
    new_time.year = timeinfo.tm_year + 1900;
    new_time.month = timeinfo.tm_mon + 1;
    new_time.day = timeinfo.tm_mday;
    new_time.hour = timeinfo.tm_hour;
    new_time.minute = timeinfo.tm_min;
    new_time.second = timeinfo.tm_sec;
    new_time.weekday = timeinfo.tm_wday;  // Corrected: weekday not day_of_week
    
    setCurrentTime(new_time);
    
    Serial.printf("[WiFiSync] Time synced: %04d-%02d-%02d %02d:%02d:%02d\n",
                  new_time.year,
                  new_time.month,
                  new_time.day,
                  new_time.hour,
                  new_time.minute,
                  new_time.second);
    
    return true;
  }
  
  Serial.println("[WiFiSync] NTP sync failed");
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
