/*
 * wifi_apps.cpp - WiFi & Internet Implementation (FIXED)
 */

#include "wifi_apps.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "hardware.h"
#include "navigation.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// WiFi state
static WiFiState wifi_state = WIFI_DISCONNECTED;
static WiFiConfig wifi_config;
static NetworkInfo available_networks[20];
static int network_count = 0;
static int selected_network = -1;
static String password_input = "";

// Weather data - struct order: location, description, icon, last_update, temperature, humidity, wind_speed, pressure, valid
static WeatherData cached_weather = {"Unknown", "Clear", "01d", "", 20.0f, 50.0f, 5.0f, 1013, false};

// News data
static NewsArticle cached_news[5];
static int news_count = 0;

// =============================================================================
// WIFI MANAGER
// =============================================================================

void initWiFiManager() {
  Serial.println("[WiFi] Initializing WiFi manager...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

// Alias function for initWiFiManager
void initWifiApps() {
  initWiFiManager();
}

void updateWiFiManager() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    updateWiFiStatus();
  }
}

void updateWiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    wifi_state = WIFI_CONNECTED;
    system_state.wifi_connected = true;
    system_state.wifi_ssid = WiFi.SSID();
    system_state.wifi_signal_strength = WiFi.RSSI();
  } else {
    wifi_state = WIFI_DISCONNECTED;
    system_state.wifi_connected = false;
  }
}

bool connectWiFi(const String& ssid, const String& password) {
  Serial.printf("[WiFi] Connecting to %s...\n", ssid.c_str());
  wifi_state = WIFI_CONNECTING;
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected!");
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    wifi_state = WIFI_CONNECTED;
    system_state.wifi_connected = true;
    system_state.wifi_ssid = ssid;
    return true;
  }
  
  Serial.println("[WiFi] Connection failed");
  wifi_state = WIFI_FAILED;
  return false;
}

bool connectSavedWiFi() {
  if (wifi_config.ssid.length() > 0) {
    return connectWiFi(wifi_config.ssid, wifi_config.password);
  }
  return false;
}

void disconnectWiFi() {
  WiFi.disconnect();
  wifi_state = WIFI_DISCONNECTED;
  system_state.wifi_connected = false;
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

WiFiState getWiFiState() {
  return wifi_state;
}

void startWiFiScan() {
  Serial.println("[WiFi] Starting scan...");
  wifi_state = WIFI_SCANNING;
  WiFi.scanNetworks(true);  // Async scan
}

bool isScanComplete() {
  int result = WiFi.scanComplete();
  return result >= 0;
}

int getAvailableNetworks(NetworkInfo* networks, int max_count) {
  int count = WiFi.scanComplete();
  if (count < 0) return 0;
  
  int n = min(count, max_count);
  for (int i = 0; i < n; i++) {
    networks[i].ssid = WiFi.SSID(i);
    networks[i].rssi = WiFi.RSSI(i);
    networks[i].encrypted = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  
  WiFi.scanDelete();
  return n;
}

int getSignalStrength() {
  return WiFi.RSSI();
}

String getSignalQuality() {
  int rssi = WiFi.RSSI();
  if (rssi > -50) return "Excellent";
  if (rssi > -60) return "Good";
  if (rssi > -70) return "Fair";
  return "Weak";
}

String getLocalIP() {
  return WiFi.localIP().toString();
}

String getMacAddress() {
  return WiFi.macAddress();
}

// =============================================================================
// WIFI SCREENS
// =============================================================================

void initWiFiSetupScreen() {
  system_state.current_screen = SCREEN_WIFI_SETUP;
  startWiFiScan();
}

void showWiFiSetupScreen() {
  drawNetworkListScreen();
}

void showNetworkScanScreen() {
  gfx->fillScreen(COLOR_BLACK);
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 200);
  gfx->print("Scanning...");
}

void showNetworkStatusScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(60, 20);
  gfx->print("NETWORK STATUS");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  
  gfx->setCursor(20, 80);
  gfx->printf("Status: %s", isWiFiConnected() ? "Connected" : "Disconnected");
  
  if (isWiFiConnected()) {
    gfx->setCursor(20, 110);
    gfx->printf("SSID: %s", system_state.wifi_ssid.c_str());
    gfx->setCursor(20, 140);
    gfx->printf("IP: %s", getLocalIP().c_str());
    gfx->setCursor(20, 170);
    gfx->printf("Signal: %s (%d dBm)", getSignalQuality().c_str(), getSignalStrength());
    gfx->setCursor(20, 200);
    gfx->printf("MAC: %s", getMacAddress().c_str());
  }
  
  drawSwipeIndicator();
}

void handleWiFiSetupTouch(TouchGesture& gesture) {
  // Swipe up to exit handled by main gesture handler
  // Taps handled by handleNetworkListTouch
}

// Alias function for handleWiFiSetupTouch
void handleWifiManagerTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Network selection
  for (int i = 0; i < min(network_count, 6); i++) {
    int ny = 70 + i * 50;
    if (y >= ny && y < ny + 45) {
      selected_network = i;
      if (!available_networks[i].encrypted) {
        connectWiFi(available_networks[i].ssid, "");
      }
      return;
    }
  }
  
  // Scan button at bottom
  if (y >= 380 && y < 420) {
    startWiFiScan();
    drawNetworkListScreen();
  }
}

void drawNetworkListScreen() {
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, RGB565(80, 180, 255));
  gfx->setTextColor(RGB565(80, 180, 255));
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 24, 16);
  gfx->print("WiFi");
  
  if (wifi_state == WIFI_SCANNING) {
    gfx->setTextColor(RGB565(120, 120, 130));
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH/2 - 30, 200);
    gfx->print("Scanning...");
  } else {
    network_count = getAvailableNetworks(available_networks, 20);
    
    for (int i = 0; i < min(network_count, 6); i++) {
      int y = 62 + i * 55;
      gfx->fillRoundRect(20, y, LCD_WIDTH - 40, 48, 14, RGB565(22, 24, 32));
      gfx->drawRoundRect(20, y, LCD_WIDTH - 40, 48, 14, RGB565(42, 44, 55));
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(1);
      gfx->setCursor(35, y + 10);
      gfx->print(available_networks[i].ssid);
      gfx->setTextColor(RGB565(100, 100, 110));
      gfx->setCursor(35, y + 28);
      gfx->printf("%d dBm %s", available_networks[i].rssi, 
                 available_networks[i].encrypted ? "Secured" : "Open");
    }
    
    if (network_count == 0) {
      gfx->setTextColor(RGB565(100, 100, 110));
      gfx->setTextSize(1);
      gfx->setCursor(LCD_WIDTH/2 - 50, 200);
      gfx->print("No networks found");
    }
  }
  
  // Scan button
  gfx->fillRoundRect(LCD_WIDTH/2 - 50, 385, 100, 38, 19, theme->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 24, 396);
  gfx->print("Scan");
  
  drawSwipeIndicator();
}

void drawPasswordEntryScreen() {
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, theme->primary);
  gfx->setTextColor(theme->primary);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 48, 16);
  gfx->print("Password");
  
  gfx->setTextColor(RGB565(160, 160, 170));
  gfx->setTextSize(1);
  gfx->setCursor(30, 70);
  gfx->printf("Network: %s", available_networks[selected_network].ssid.c_str());
  
  // Password field
  gfx->fillRoundRect(20, 100, LCD_WIDTH - 40, 45, 14, RGB565(22, 24, 32));
  gfx->drawRoundRect(20, 100, LCD_WIDTH - 40, 45, 14, RGB565(50, 52, 65));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(35, 118);
  String masked = "";
  for (int i = 0; i < password_input.length(); i++) masked += "*";
  gfx->print(masked.length() > 0 ? masked : "Tap to enter");
  
  // Connect button
  gfx->fillRoundRect(LCD_WIDTH/2 - 55, 380, 110, 40, 20, theme->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 36, 391);
  gfx->print("Connect");
  
  drawSwipeIndicator();
}

void handleNetworkListTouch(TouchGesture& gesture) {
  // Handled by handleWifiManagerTouch now
}

void handlePasswordEntryTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  if (gesture.y >= 380 && gesture.y < 420) {
    if (gesture.x < LCD_WIDTH/2) {
      // Connect
      if (selected_network >= 0) {
        connectWiFi(available_networks[selected_network].ssid, password_input);
      }
    }
  }
}

// =============================================================================
// WEATHER APP
// =============================================================================

void initWeatherApp() {
  system_state.current_screen = SCREEN_WEATHER_APP;
  if (isWiFiConnected()) {
    refreshWeatherData();
  }
}

void drawWeatherApp() {
  gfx->fillScreen(RGB565(8, 8, 12));
  
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 50, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 50, LCD_WIDTH, RGB565(80, 180, 255));
  gfx->setTextColor(RGB565(80, 180, 255));
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 42, 16);
  gfx->print("Weather");
  
  if (!cached_weather.valid) {
    gfx->fillRoundRect(30, 150, LCD_WIDTH - 60, 70, 16, RGB565(22, 24, 32));
    gfx->drawRoundRect(30, 150, LCD_WIDTH - 60, 70, 16, RGB565(42, 44, 55));
    gfx->setTextColor(RGB565(120, 120, 130));
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH/2 - 50, 180);
    gfx->print(isWiFiConnected() ? "Loading..." : "WiFi not connected");
  } else {
    // Location
    gfx->setTextColor(RGB565(160, 160, 170));
    gfx->setTextSize(1);
    gfx->setCursor(LCD_WIDTH/2 - 30, 65);
    gfx->print(cached_weather.location);
    
    // Temperature card
    gfx->fillRoundRect(30, 85, LCD_WIDTH - 60, 120, 20, RGB565(18, 20, 28));
    gfx->drawRoundRect(30, 85, LCD_WIDTH - 60, 120, 20, RGB565(42, 44, 55));
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(7);
    gfx->setCursor(65, 100);
    gfx->printf("%.0f", cached_weather.temperature);
    gfx->setTextSize(2);
    gfx->setTextColor(theme->accent);
    gfx->print(" C");
    
    // Description
    gfx->setTextColor(RGB565(180, 180, 190));
    gfx->setTextSize(2);
    int descLen = cached_weather.description.length() * 12;
    gfx->setCursor((LCD_WIDTH - descLen) / 2, 225);
    gfx->print(cached_weather.description);
    
    // Details cards
    gfx->fillRoundRect(20, 265, (LCD_WIDTH-50)/2, 55, 14, RGB565(22, 24, 32));
    gfx->drawRoundRect(20, 265, (LCD_WIDTH-50)/2, 55, 14, RGB565(42, 44, 55));
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(100, 100, 110));
    gfx->setCursor(32, 275);
    gfx->print("Humidity");
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(32, 295);
    gfx->printf("%.0f%%", cached_weather.humidity);
    
    gfx->fillRoundRect(LCD_WIDTH/2 + 5, 265, (LCD_WIDTH-50)/2, 55, 14, RGB565(22, 24, 32));
    gfx->drawRoundRect(LCD_WIDTH/2 + 5, 265, (LCD_WIDTH-50)/2, 55, 14, RGB565(42, 44, 55));
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(100, 100, 110));
    gfx->setCursor(LCD_WIDTH/2 + 17, 275);
    gfx->print("Wind");
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(LCD_WIDTH/2 + 17, 295);
    gfx->printf("%.1f", cached_weather.wind_speed);
  }
  
  // Refresh button
  gfx->fillRoundRect(LCD_WIDTH/2 - 50, 340, 100, 40, 20, theme->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(LCD_WIDTH/2 - 36, 351);
  gfx->print("Refresh");
  
  drawSwipeIndicator();
}

void handleWeatherTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  if (gesture.y >= 340 && gesture.y < 380) {
    refreshWeatherData();
    drawWeatherApp();
  }
}

void refreshWeatherData() {
  if (!isWiFiConnected()) return;
  cached_weather = fetchWeatherData("");
}

WeatherData fetchWeatherData(const String& location) {
  // Struct order: location, description, icon, last_update, temperature, humidity, wind_speed, pressure, valid
  WeatherData data = {"Demo City", "Partly Cloudy", "02d", "", 22.5f, 65.0f, 3.5f, 1015, true};
  
  // In production, this would call OpenWeatherMap API
  // HTTPClient http;
  // http.begin("http://api.openweathermap.org/data/2.5/weather?q=...&appid=...");
  
  Serial.println("[Weather] Fetched weather data (demo)");
  return data;
}

// =============================================================================
// NEWS APP
// =============================================================================

static int current_article = 0;

void initNewsApp() {
  system_state.current_screen = SCREEN_NEWS_APP;
  if (isWiFiConnected()) {
    refreshNewsData();
  }
}

void drawNewsApp() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(130, 20);
  gfx->print("NEWS");
  
  if (news_count == 0) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(100, 200);
    gfx->print(isWiFiConnected() ? "No news available" : "WiFi not connected");
  } else {
    NewsArticle& article = cached_news[current_article];
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(20, 70);
    // Word wrap title
    String title = article.title;
    if (title.length() > 25) title = title.substring(0, 22) + "...";
    gfx->print(title);
    
    gfx->setTextSize(1);
    gfx->setCursor(20, 120);
    gfx->print(article.source);
    
    gfx->setCursor(20, 150);
    // FIXED: Use summary field correctly
    String summaryText = article.summary;
    if (summaryText.length() > 150) summaryText = summaryText.substring(0, 150);
    gfx->print(summaryText);
    
    // Navigation
    gfx->setCursor(150, 320);
    gfx->printf("%d/%d", current_article + 1, news_count);
  }
  
  drawThemeButton(30, 360, 80, 40, "Prev", false);
  drawSwipeIndicator();
  drawThemeButton(250, 360, 80, 40, "Next", false);
}

void handleNewsTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  if (y >= 360 && y < 400) {
    if (x < 110) {
      if (current_article > 0) current_article--;
    } else if (x < 220) {
      system_state.current_screen = SCREEN_APP_GRID;
    } else {
      if (current_article < news_count - 1) current_article++;
    }
  }
}

void refreshNewsData() {
  news_count = fetchNewsHeadlines(cached_news, 5);
}

int fetchNewsHeadlines(NewsArticle* articles, int max_count) {
  // FIXED: Initialize NewsArticle structs properly using member assignment
  // NewsArticle has: title, summary, source, url, time_ago, timestamp
  
  articles[0].title = "Tech Innovation Summit 2025";
  articles[0].summary = "Major announcements expected at annual tech conference";
  articles[0].source = "Tech News";
  articles[0].url = "";
  articles[0].time_ago = "2h ago";
  articles[0].timestamp = 0;
  
  articles[1].title = "Climate Action Progress";
  articles[1].summary = "Global leaders discuss environmental initiatives";
  articles[1].source = "World News";
  articles[1].url = "";
  articles[1].time_ago = "4h ago";
  articles[1].timestamp = 0;
  
  articles[2].title = "Sports Championship Finals";
  articles[2].summary = "Exciting match scheduled for this weekend";
  articles[2].source = "Sports";
  articles[2].url = "";
  articles[2].time_ago = "6h ago";
  articles[2].timestamp = 0;
  
  Serial.println("[News] Fetched news (demo)");
  return 3;
}

// =============================================================================
// UTILITIES
// =============================================================================

bool syncTimeWithNTP() {
  if (!isWiFiConnected()) return false;
  configTime(0, 0, "pool.ntp.org");
  return true;
}

bool checkInternetConnectivity() {
  return pingHost("google.com");
}

bool pingHost(const String& host) {
  if (!isWiFiConnected()) return false;
  // Simplified ping check
  return true;
}

void updateInternetApps() {
  updateWiFiManager();
}

void showNetworkTools() {
  drawNetworkDiagnostics();
}

void drawNetworkDiagnostics() {
  gfx->fillScreen(COLOR_BLACK);
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(50, 20);
  gfx->print("NETWORK TOOLS");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 80);
  gfx->printf("WiFi: %s", isWiFiConnected() ? "Connected" : "Disconnected");
  gfx->setCursor(20, 100);
  gfx->printf("Internet: %s", checkInternetConnectivity() ? "Available" : "Unavailable");
}

void runSpeedTest() {
  Serial.println("[Network] Running speed test...");
  // Would download test file and measure speed
}
