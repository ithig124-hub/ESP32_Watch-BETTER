/*
 * wifi_sync.cpp - WiFi Boot Sync Implementation (FIXED)
 * FUSION OS Feature - With SD Card WiFi Support
 *
 * NOTE: SD Card is initialized by sd_manager.cpp - we just use it here
 */

#include "wifi_sync.h"
#include "config.h"
#include "pin_config.h"
#include "hardware.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SD_MMC.h>
#include <FS.h>

// =============================================================================
// SD CARD TOGGLE - Set to false if hardware conflict with display
// =============================================================================
#define SD_CARD_ENABLED true

// =============================================================================
// CONFIGURATION
// =============================================================================

// Timezone configuration (auto-detected or manual)
static int timezone_offset_hours = 8;    // Default GMT+8 (will be auto-detected)
static int timezone_offset_minutes = 0;
static bool daylight_saving = false;

// Detected location info
static char detectedCity[64] = "Unknown";
static char detectedCountry[8] = "??";
static bool locationDetected = false;

// SD Card state - USE THE ONE FROM sd_manager.cpp
extern bool sdCardInitialized;  // From sd_manager.cpp
#define sdCardMounted sdCardInitialized  // Alias for compatibility

// External declarations
extern void feedWatchdog();
extern WiFiState wifi_state;
extern SystemState system_state;

// =============================================================================
// GLOBAL STATE
// =============================================================================

WiFiSyncState wifi_sync_state = {
    .sync_enabled = true,
    .last_sync_success = false,
    .last_sync_time = 0,
    .networks_loaded = 0
};

// Hardcoded WiFi network (PRIMARY - always slot 0)
const char* HARDCODED_SSID = "Optus_9D2E3D";
const char* HARDCODED_PASSWORD = "snucktemptGLeQU";

// =============================================================================
// CONSTANTS - REDUCED TIMEOUTS
// =============================================================================
#define WIFI_RETRY_PER_NETWORK 2
#define WIFI_CONNECT_TIMEOUT 5000      // 5 seconds per attempt
#define NTP_TIMEOUT 5000               // 5 seconds for NTP
#define QUICK_SCAN_TIMEOUT 300         // 300ms per WiFi channel
#define HTTP_TIMEOUT 5000              // 5 seconds for HTTP requests

// SD Card paths
#define SD_WIFI_PATH "/WATCH/wifi"
#define SD_WIFI_CONFIG_FILE "/WATCH/wifi/config.txt"
#define SD_TIME_BACKUP_PATH "/WATCH/TIME"
#define SD_TIME_BACKUP_FILE "/WATCH/TIME/backup.txt"

// =============================================================================
// SD CARD FOLDER MANAGEMENT (Updated for SD_MMC)
// =============================================================================

bool createWiFiFolders() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) {
        Serial.println("[WiFiSync] SD card not mounted, skipping folder creation");
        return false;
    }
    
    Serial.println("[WiFiSync] Checking/creating WiFi folders on SD card...");
    
    // Create WATCH root folder
    if (!SD_MMC.exists("/WATCH")) {
        if (SD_MMC.mkdir("/WATCH")) {
            Serial.println("[WiFiSync] Created /WATCH folder");
        }
    }
    
    // Create wifi folder
    if (!SD_MMC.exists(SD_WIFI_PATH)) {
        if (SD_MMC.mkdir(SD_WIFI_PATH)) {
            Serial.println("[WiFiSync] Created /WATCH/wifi folder");
        }
    }
    
    // Create TIME folder for backups
    if (!SD_MMC.exists(SD_TIME_BACKUP_PATH)) {
        if (SD_MMC.mkdir(SD_TIME_BACKUP_PATH)) {
            Serial.println("[WiFiSync] Created /WATCH/TIME folder");
        }
    }
    
    return true;
#else
    return false;
#endif
}

bool createExampleConfig() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) return false;
    
    // Only create if config doesn't exist
    if (SD_MMC.exists(SD_WIFI_CONFIG_FILE)) {
        Serial.println("[WiFiSync] Config file already exists, skipping creation");
        return true;
    }
    
    Serial.println("[WiFiSync] Creating example config file...");
    
    File configFile = SD_MMC.open(SD_WIFI_CONFIG_FILE, FILE_WRITE);
    if (!configFile) {
        Serial.println("[WiFiSync] Failed to create config file");
        return false;
    }
    
    // Write example config
    configFile.println("# WiFi Configuration File for ESP32 Watch");
    configFile.println("# Add your WiFi networks below (up to 5 networks)");
    configFile.println("# Format: WIFIn_SSID=YourNetworkName");
    configFile.println("#         WIFIn_PASS=YourPassword");
    configFile.println("# Leave password blank for open networks");
    configFile.println("#");
    configFile.println("# Timezone is AUTO-DETECTED from your IP location!");
    configFile.println("# But you can override it here if needed:");
    configFile.println("# GMT_OFFSET=8");
    configFile.println("#");
    configFile.println("# Example networks:");
    configFile.println("# WIFI1_SSID=HomeNetwork");
    configFile.println("# WIFI1_PASS=secretpassword");
    configFile.println("#");
    configFile.println("# WIFI2_SSID=OfficeNetwork");
    configFile.println("# WIFI2_PASS=office123");
    configFile.println("");
    configFile.println("# Add your networks below:");
    configFile.println("# WIFI1_SSID=");
    configFile.println("# WIFI1_PASS=");
    configFile.println("# WIFI2_SSID=");
    configFile.println("# WIFI2_PASS=");
    configFile.println("# WIFI3_SSID=");
    configFile.println("# WIFI3_PASS=");
    configFile.println("# WIFI4_SSID=");
    configFile.println("# WIFI4_PASS=");
    configFile.println("# WIFI5_SSID=");
    configFile.println("# WIFI5_PASS=");
    
    configFile.close();
    
    Serial.println("[WiFiSync] Example config file created");
    return true;
#else
    return false;
#endif
}

// =============================================================================
// LOAD WIFI NETWORKS FROM SD CARD (Updated for SD_MMC)
// =============================================================================

int loadWiFiNetworksFromSD() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) {
        Serial.println("[WiFiSync] SD card not mounted, skipping SD WiFi load");
        return 0;
    }
    
    // First, create folders if they don't exist
    createWiFiFolders();
    
    // Create example config if none exists
    createExampleConfig();
    
    // Now try to read the config file
    if (!SD_MMC.exists(SD_WIFI_CONFIG_FILE)) {
        Serial.println("[WiFiSync] No config file found on SD card");
        return 0;
    }
    
    File configFile = SD_MMC.open(SD_WIFI_CONFIG_FILE, FILE_READ);
    if (!configFile) {
        Serial.println("[WiFiSync] Failed to open config file");
        return 0;
    }
    
    Serial.println("[WiFiSync] Reading WiFi config from SD card...");
    
    int networksAdded = 0;
    char line[128];
    char currentSSID[64] = "";
    char currentPass[64] = "";
    bool hasSSID = false;
    
    // Parse the config file (up to 5 networks from SD)
    while (configFile.available() && networksAdded < 5) {
        // Read a line
        int lineLen = 0;
        while (configFile.available() && lineLen < 127) {
            char c = configFile.read();
            if (c == '\n' || c == '\r') break;
            line[lineLen++] = c;
        }
        line[lineLen] = (char)0;
        
        // Skip empty lines and comments
        if (lineLen == 0 || line[0] == '#') continue;
        
        // Parse WIFI_SSID or WIFIn_SSID
        if (strstr(line, "SSID=") != NULL) {
            char* value = strchr(line, '=');
            if (value) {
                value++;  // Skip '='
                // Trim whitespace
                while (*value == ' ') value++;
                if (strlen(value) > 0) {
                    strncpy(currentSSID, value, sizeof(currentSSID) - 1);
                    currentSSID[sizeof(currentSSID) - 1] = (char)0;
                    hasSSID = true;
                    Serial.printf("[WiFiSync] Found SSID: %s\n", currentSSID);
                }
            }
        }
        // Parse WIFI_PASS or WIFIn_PASS
        else if (strstr(line, "PASS=") != NULL) {
            char* value = strchr(line, '=');
            if (value) {
                value++;  // Skip '='
                // Trim whitespace
                while (*value == ' ') value++;
                strncpy(currentPass, value, sizeof(currentPass) - 1);
                currentPass[sizeof(currentPass) - 1] = (char)0;
                
                // If we have an SSID, add the network
                if (hasSSID && strlen(currentSSID) > 0) {
                    // Find next available slot (slot 0 is hardcoded)
                    int slot = wifi_sync_state.networks_loaded;
                    if (slot < MAX_WIFI_NETWORKS) {
                        strncpy(wifi_sync_state.networks[slot].ssid, currentSSID, sizeof(wifi_sync_state.networks[slot].ssid) - 1);
                        strncpy(wifi_sync_state.networks[slot].password, currentPass, sizeof(wifi_sync_state.networks[slot].password) - 1);
                        wifi_sync_state.networks[slot].valid = true;
                        wifi_sync_state.networks[slot].isFromSD = true;
                        wifi_sync_state.networks_loaded++;
                        networksAdded++;
                        Serial.printf("[WiFiSync] Added SD network [%d]: %s\n", slot, currentSSID);
                    }
                    
                    // Reset for next network
                    hasSSID = false;
                    currentSSID[0] = (char)0;
                    currentPass[0] = (char)0;
                }
            }
        }
        // Parse manual timezone override (only if user explicitly sets it)
        else if (strstr(line, "GMT_OFFSET=") != NULL) {
            char* value = strchr(line, '=');
            if (value) {
                value++;
                timezone_offset_hours = atoi(value);
                Serial.printf("[WiFiSync] Manual timezone override: GMT%+d\n", timezone_offset_hours);
            }
        }
    }
    
    configFile.close();
    
    Serial.printf("[WiFiSync] Loaded %d networks from SD card\n", networksAdded);
    return networksAdded;
#else
    return 0;
#endif
}

// =============================================================================
// LOAD ALL WIFI NETWORKS (Hardcoded + SD)
// =============================================================================

int loadWiFiNetworks() {
    int count = 0;
    
    // Clear all networks first
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        wifi_sync_state.networks[i].valid = false;
        wifi_sync_state.networks[i].ssid[0] = (char)0;
        wifi_sync_state.networks[i].password[0] = (char)0;
        wifi_sync_state.networks[i].isFromSD = false;
    }
    
    // Add hardcoded network as PRIMARY (slot 0)
    strncpy(wifi_sync_state.networks[0].ssid, HARDCODED_SSID, sizeof(wifi_sync_state.networks[0].ssid) - 1);
    strncpy(wifi_sync_state.networks[0].password, HARDCODED_PASSWORD, sizeof(wifi_sync_state.networks[0].password) - 1);
    wifi_sync_state.networks[0].valid = true;
    wifi_sync_state.networks[0].isFromSD = false;
    wifi_sync_state.networks_loaded = 1;
    count = 1;
    
    Serial.println("[WiFiSync] OK Hardcoded WiFi added as primary network");
    Serial.printf("[WiFiSync]   Primary: %s\n", HARDCODED_SSID);
    
#if SD_CARD_ENABLED
    // Load additional networks from SD card (slots 1+)
    int sdNetworks = loadWiFiNetworksFromSD();
    count += sdNetworks;
#else
    Serial.println("[WiFiSync] INFO SD Card disabled (hardware conflict with display)");
    Serial.println("[WiFiSync] -> To add more networks, update wifi_sync.cpp hardcoded credentials");
#endif
    
    Serial.printf("[WiFiSync] Total networks available: %d\n", wifi_sync_state.networks_loaded);
    
    return count;
}

// =============================================================================
// AUTO TIMEZONE DETECTION VIA IP GEOLOCATION
// =============================================================================

bool fetchLocationFromIP() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    Serial.println("[WiFiSync] === AUTO TIMEZONE DETECTION ===");
    Serial.println("[WiFiSync] Fetching location from IP...");
    feedWatchdog();
    
    HTTPClient http;
    
    // Use ip-api.com (free, no API key needed, includes timezone)
    http.begin("http://ip-api.com/json/?fields=status,country,countryCode,city,timezone,offset");
    http.setTimeout(HTTP_TIMEOUT);
    
    int httpCode = http.GET();
    feedWatchdog();
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[WiFiSync] IP geolocation failed: HTTP %d\n", httpCode);
        http.end();
        return false;
    }
    
    String payload = http.getString();
    http.end();
    feedWatchdog();
    
    // Parse JSON response
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.printf("[WiFiSync] JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    // Check status
    const char* status = doc["status"] | "fail";
    if (strcmp(status, "success") != 0) {
        Serial.println("[WiFiSync] IP geolocation returned fail status");
        return false;
    }
    
    // Extract location data
    const char* city = doc["city"] | "Unknown";
    const char* countryCode = doc["countryCode"] | "??";
    const char* timezone = doc["timezone"] | "Unknown";
    int offsetSeconds = doc["offset"] | 0;  // Offset in seconds from UTC
    
    // Store detected location
    strncpy(detectedCity, city, sizeof(detectedCity) - 1);
    strncpy(detectedCountry, countryCode, sizeof(detectedCountry) - 1);
    locationDetected = true;
    
    // Convert offset from seconds to hours/minutes
    timezone_offset_hours = offsetSeconds / 3600;
    timezone_offset_minutes = (abs(offsetSeconds) % 3600) / 60;
    
    Serial.println("[WiFiSync] =======================================");
    Serial.println("[WiFiSync] OK LOCATION DETECTED!");
    Serial.printf("[WiFiSync]   City: %s, %s\n", detectedCity, detectedCountry);
    Serial.printf("[WiFiSync]   Timezone: %s\n", timezone);
    Serial.printf("[WiFiSync]   UTC Offset: GMT%+d:%02d (%d seconds)\n", 
        timezone_offset_hours, timezone_offset_minutes, offsetSeconds);
    Serial.println("[WiFiSync] =======================================");
    
    return true;
}

const char* getDetectedCity() {
    return detectedCity;
}

const char* getDetectedCountry() {
    return detectedCountry;
}

// =============================================================================
// TIME BACKUP/RESTORE (FOR OFFLINE BOOT) - Updated for SD_MMC
// =============================================================================

bool saveTimeBackup() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) return false;
    
    // Get current time from RTC
    WatchTime current_time = getCurrentTime();
    
    // Ensure folder exists
    if (!SD_MMC.exists(SD_TIME_BACKUP_PATH)) {
        SD_MMC.mkdir(SD_TIME_BACKUP_PATH);
    }
    
    File backupFile = SD_MMC.open(SD_TIME_BACKUP_FILE, FILE_WRITE);
    if (!backupFile) {
        Serial.println("[WiFiSync] Failed to create time backup file");
        return false;
    }
    
    // Save time and timezone info
    backupFile.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
        current_time.year, current_time.month, current_time.day,
        current_time.hour, current_time.minute, current_time.second);
    backupFile.printf("TZ_OFFSET=%d\n", timezone_offset_hours);
    backupFile.printf("TZ_MINUTES=%d\n", timezone_offset_minutes);
    backupFile.printf("CITY=%s\n", detectedCity);
    backupFile.printf("COUNTRY=%s\n", detectedCountry);
    
    backupFile.close();
    
    Serial.printf("[WiFiSync] Time backup saved: %04d-%02d-%02d %02d:%02d:%02d (GMT%+d)\n",
        current_time.year, current_time.month, current_time.day,
        current_time.hour, current_time.minute, current_time.second,
        timezone_offset_hours);
    
    return true;
#else
    return false;
#endif
}

bool hasTimeBackup() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) return false;
    return SD_MMC.exists(SD_TIME_BACKUP_FILE);
#else
    return false;
#endif
}

bool restoreTimeBackup() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) return false;
    
    if (!SD_MMC.exists(SD_TIME_BACKUP_FILE)) {
        Serial.println("[WiFiSync] No time backup file found");
        return false;
    }
    
    File backupFile = SD_MMC.open(SD_TIME_BACKUP_FILE, FILE_READ);
    if (!backupFile) {
        Serial.println("[WiFiSync] Failed to open time backup file");
        return false;
    }
    
    char line[128];
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    
    while (backupFile.available()) {
        int lineLen = 0;
        while (backupFile.available() && lineLen < 127) {
            char c = backupFile.read();
            if (c == '\n' || c == '\r') break;
            line[lineLen++] = c;
        }
        line[lineLen] = '';
        
        if (lineLen == 0) continue;
        
        // Parse time line (first line)
        if (sscanf(line, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
            // Time parsed
        }
        // Parse timezone offset
        else if (strstr(line, "TZ_OFFSET=") != NULL) {
            timezone_offset_hours = atoi(line + 10);
        }
        else if (strstr(line, "TZ_MINUTES=") != NULL) {
            timezone_offset_minutes = atoi(line + 11);
        }
        else if (strstr(line, "CITY=") != NULL) {
            strncpy(detectedCity, line + 5, sizeof(detectedCity) - 1);
        }
        else if (strstr(line, "COUNTRY=") != NULL) {
            strncpy(detectedCountry, line + 8, sizeof(detectedCountry) - 1);
        }
    }
    
    backupFile.close();
    
    if (year > 2020) {
        WatchTime restored_time;
        restored_time.year = year;
        restored_time.month = month;
        restored_time.day = day;
        restored_time.hour = hour;
        restored_time.minute = minute;
        restored_time.second = second;
        restored_time.weekday = 0;
        
        setCurrentTime(restored_time);
        
        Serial.printf("[WiFiSync] Time restored: %04d-%02d-%02d %02d:%02d:%02d\n",
            year, month, day, hour, minute, second);
        Serial.printf("[WiFiSync] Timezone restored: GMT%+d (%s, %s)\n",
            timezone_offset_hours, detectedCity, detectedCountry);
        
        return true;
    }
    
    Serial.println("[WiFiSync] Failed to parse time backup");
    return false;
#else
    return false;
#endif
}

void deleteTimeBackup() {
#if SD_CARD_ENABLED
    if (!sdCardMounted) return;
    
    if (SD_MMC.exists(SD_TIME_BACKUP_FILE)) {
        SD_MMC.remove(SD_TIME_BACKUP_FILE);
        Serial.println("[WiFiSync] Old time backup deleted (fresh sync)");
    }
#endif
}

// =============================================================================
// QUICK NETWORK SCAN - Check if any known network is visible
// =============================================================================

bool quickNetworkScan() {
    Serial.println("[WiFiSync] Quick scan: Checking for available networks...");
    feedWatchdog();
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    feedWatchdog();
    
    Serial.println("[WiFiSync] Scanning...");
    int scanResult = WiFi.scanNetworks(false, false, false, QUICK_SCAN_TIMEOUT);
    feedWatchdog();
    
    if (scanResult <= 0) {
        Serial.println("[WiFiSync] Quick scan: NO NETWORKS FOUND");
        Serial.println("[WiFiSync] -> Skipping WiFi sync (no networks in range)");
        WiFi.scanDelete();
        WiFi.mode(WIFI_OFF);
        return false;
    }
    
    Serial.printf("[WiFiSync] Quick scan: Found %d networks\n", scanResult);
    
    bool anyTargetFound = false;
    for (int net = 0; net < wifi_sync_state.networks_loaded; net++) {
        if (!wifi_sync_state.networks[net].valid) continue;
        
        for (int i = 0; i < scanResult; i++) {
            feedWatchdog();
            String foundSSID = WiFi.SSID(i);
            if (foundSSID.equals(wifi_sync_state.networks[net].ssid)) {
                anyTargetFound = true;
                Serial.printf("[WiFiSync] Quick scan: FOUND '%s' (RSSI: %d dBm) [%s]\n",
                    wifi_sync_state.networks[net].ssid,
                    WiFi.RSSI(i),
                    wifi_sync_state.networks[net].isFromSD ? "SD" : "hardcoded");
            }
        }
    }
    
    WiFi.scanDelete();
    
    if (!anyTargetFound) {
        Serial.println("[WiFiSync] Quick scan: No known networks visible");
        Serial.println("[WiFiSync] -> Skipping WiFi sync (known networks not in range)");
        WiFi.mode(WIFI_OFF);
        return false;
    }
    
    Serial.println("[WiFiSync] Quick scan: Known network(s) available, proceeding with sync");
    return true;
}

// =============================================================================
// INITIALIZE WIFI SYNC - SD Card is already initialized by sd_manager.cpp
// =============================================================================

void initWiFiSync() {
    Serial.println("[WiFiSync] === Initializing WiFi Sync System ===");
    Serial.println("[WiFiSync] Auto-timezone detection: ENABLED");
    Serial.printf("[WiFiSync] Default timezone: GMT%+d\n", timezone_offset_hours);
    
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // NOTE: SD Card is already initialized by sd_manager.cpp
    // We just check if it's available
#if SD_CARD_ENABLED
    Serial.printf("[WiFiSync] SD Card: %s\n", sdCardMounted ? "Available" : "Not available");
#else
    Serial.println("[WiFiSync] SD Card: DISABLED (hardware conflict mode)");
#endif
    
    wifi_sync_state.networks_loaded = loadWiFiNetworks();
    
    Serial.printf("[WiFiSync] Loaded %d WiFi network(s)\n", wifi_sync_state.networks_loaded);
    Serial.println("[WiFiSync] === WiFi Sync System Ready ===");
}

// =============================================================================
// CONNECT TO WIFI
// =============================================================================

bool connectToWiFi(const char* ssid, const char* password, int timeout_ms) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    unsigned long start = millis();
    
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
        delay(250);
        feedWatchdog();
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFiSync] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    
    WiFi.disconnect();
    return false;
}

// =============================================================================
// PERFORM BOOT SYNC - SAFE: Never causes boot loop
// =============================================================================

bool performBootSync() {
    if (!wifi_sync_state.sync_enabled) {
        Serial.println("[WiFiSync] Sync disabled, skipping");
        feedWatchdog();
        return false;
    }
    
    Serial.println("[WiFiSync] =======================================");
    Serial.println("[WiFiSync]          BOOT SYNC START");
    Serial.println("[WiFiSync] =======================================");
    feedWatchdog();
    
    // CRITICAL: Quick scan first - skip if no known networks visible
    if (!quickNetworkScan()) {
        Serial.println("[WiFiSync] =======================================");
        Serial.println("[WiFiSync]    BOOT SYNC SKIPPED (No WiFi)");
        Serial.println("[WiFiSync]    -> Boot continues WITHOUT WiFi");
        Serial.println("[WiFiSync] =======================================");
        
        wifi_sync_state.last_sync_success = false;
        
        // Try to restore time from backup if available
        if (hasTimeBackup()) {
            Serial.println("[WiFiSync] Restoring time from SD backup...");
            restoreTimeBackup();
        }
        
        return false;
    }
    
    Serial.printf("[WiFiSync] Attempting to connect to %d network(s)...\n",
        wifi_sync_state.networks_loaded);
    feedWatchdog();
    
    bool connected = false;
    
    for (int net = 0; net < wifi_sync_state.networks_loaded && !connected; net++) {
        WiFiCredential* cred = &wifi_sync_state.networks[net];
        if (!cred->valid) continue;
        
        Serial.printf("[WiFiSync] Trying [%d]: %s (%s)\n", 
            net, cred->ssid, cred->isFromSD ? "SD card" : "hardcoded");
        feedWatchdog();
        
        for (int attempt = 1; attempt <= WIFI_RETRY_PER_NETWORK && !connected; attempt++) {
            Serial.printf("[WiFiSync]   Attempt %d/%d...\n", attempt, WIFI_RETRY_PER_NETWORK);
            feedWatchdog();
            
            if (connectToWiFi(cred->ssid, cred->password, WIFI_CONNECT_TIMEOUT)) {
                connected = true;
                Serial.printf("[WiFiSync] OK Connected to: %s\n", cred->ssid);
                break;
            } else {
                Serial.printf("[WiFiSync] X Failed (attempt %d)\n", attempt);
                feedWatchdog();
                delay(300);
            }
        }
    }
    
    feedWatchdog();
    
    if (!connected) {
        Serial.println("[WiFiSync] =======================================");
        Serial.println("[WiFiSync]    FAILED to connect to any network");
        Serial.println("[WiFiSync]    -> Boot continues WITHOUT WiFi");
        Serial.println("[WiFiSync] =======================================");
        
        WiFi.mode(WIFI_OFF);
        wifi_sync_state.last_sync_success = false;
        
        // Try to restore time from backup if available
        if (hasTimeBackup()) {
            Serial.println("[WiFiSync] Restoring time from SD backup...");
            restoreTimeBackup();
        }
        
        return false;
    }
    
    // ===================================================================
    // CONNECTED! Now sync everything
    // ===================================================================
    Serial.println("[WiFiSync] OK WiFi connected, syncing...");
    
    // Step 1: Auto-detect timezone from IP (BEFORE NTP sync)
    bool location_detected = fetchLocationFromIP();
    feedWatchdog();
    
    // Step 2: Sync time via NTP (uses detected timezone)
    bool time_synced = syncNTPTime();
    feedWatchdog();
    
    // Step 3: Fetch weather (optional)
    bool weather_fetched = fetchWeather();
    feedWatchdog();
    
    // Save backup with timezone info
    if (time_synced) {
        deleteTimeBackup();
        saveTimeBackup();
    }
    
    wifi_sync_state.last_sync_success = (time_synced || weather_fetched);
    wifi_sync_state.last_sync_time = millis();
    
    // Disconnect to save power
    disconnectWiFi();
    
    Serial.println("[WiFiSync] =======================================");
    Serial.println("[WiFiSync]        BOOT SYNC COMPLETE");
    Serial.printf("[WiFiSync]   Location: %s (%s, %s)\n",
        location_detected ? "OK Detected" : "X Failed",
        detectedCity, detectedCountry);
    Serial.printf("[WiFiSync]   Timezone: GMT%+d:%02d\n", 
        timezone_offset_hours, timezone_offset_minutes);
    Serial.printf("[WiFiSync]   Time: %s | Weather: %s\n",
        time_synced ? "OK Synced" : "X Failed",
        weather_fetched ? "OK Fetched" : "X Failed");
    Serial.println("[WiFiSync] =======================================");
    
    return wifi_sync_state.last_sync_success;
}

// =============================================================================
// SYNC NTP TIME - With Timezone Support
// =============================================================================

bool syncNTPTime() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    Serial.println("[WiFiSync] Syncing NTP time from time.google.com...");
    Serial.printf("[WiFiSync] Using timezone: GMT%+d:%02d\n", 
        timezone_offset_hours, timezone_offset_minutes);
    
    long gmtOffset_sec = (timezone_offset_hours * 3600) + (timezone_offset_minutes * 60);
    int daylightOffset_sec = daylight_saving ? 3600 : 0;
    
    configTime(gmtOffset_sec, daylightOffset_sec,
        "time.google.com",
        "time1.google.com",
        "time2.google.com");
    
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
        WatchTime new_time;
        new_time.year = timeinfo.tm_year + 1900;
        new_time.month = timeinfo.tm_mon + 1;
        new_time.day = timeinfo.tm_mday;
        new_time.hour = timeinfo.tm_hour;
        new_time.minute = timeinfo.tm_min;
        new_time.second = timeinfo.tm_sec;
        new_time.weekday = timeinfo.tm_wday;
        
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
    
    Serial.println("[WiFiSync] X NTP sync failed - timeout");
    Serial.println("[WiFiSync] Check: WiFi connected? Firewall blocking NTP?");
    return false;
}

// =============================================================================
// FETCH WEATHER (PLACEHOLDER)
// =============================================================================

bool fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    Serial.println("[WiFiSync] Weather fetch: Using detected location");
    Serial.printf("[WiFiSync] Location: %s, %s\n", detectedCity, detectedCountry);
    // TODO: Implement OpenWeatherMap API call using detectedCity
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
    Serial.println("[WiFiSync] WiFi disconnected (power save)");
}

// =============================================================================
// STATUS FUNCTIONS
// =============================================================================

unsigned long timeSinceLastSync() {
    if (wifi_sync_state.last_sync_time == 0) return 0xFFFFFFFF;
    return millis() - wifi_sync_state.last_sync_time;
}

int getNetworkCount() {
    return wifi_sync_state.networks_loaded;
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

void setTimezoneOffset(int hours, int minutes) {
    timezone_offset_hours = hours;
    timezone_offset_minutes = minutes;
    Serial.printf("[WiFiSync] Timezone set to GMT%+d:%02d\n", hours, minutes);
}

int getTimezoneOffsetHours() {
    return timezone_offset_hours;
}

int getTimezoneOffsetMinutes() {
    return timezone_offset_minutes;
}

void setDaylightSaving(bool enabled) {
    daylight_saving = enabled;
    Serial.printf("[WiFiSync] Daylight saving: %s\n", enabled ? "ON" : "OFF");
}

bool isDaylightSavingEnabled() {
    return daylight_saving;
}

void getTimezoneString(char* buffer, size_t bufferSize) {
    if (timezone_offset_minutes == 0) {
        snprintf(buffer, bufferSize, "GMT%+d", timezone_offset_hours);
    } else {
        snprintf(buffer, bufferSize, "GMT%+d:%02d", timezone_offset_hours, abs(timezone_offset_minutes));
    }
}

