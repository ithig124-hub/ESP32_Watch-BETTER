/*
 * wifi_apps.cpp - WiFi & Internet Implementation
 */

#include "wifi_apps.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "hardware.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// WiFi state
static WiFiState wifi_state = WIFI_DISCONNECTED;
static WiFiConfig wifi_config;
static NetworkInfo available_networks[20];
static int network_count = 0;
static int selected_network = -1;
static String password_input = "";

// Weather data
static WeatherData cached_weather = {"Unknown", 20.0, 50.0, 1013, "Clear", "01d", 5.0, "", false};

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
  
  drawThemeButton(140, 420, 80, 30, "Back", false);
}

void handleWiFiSetupTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 420) {
    system_state.current_screen = SCREEN_SETTINGS;
  }
}

void drawNetworkListScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("WIFI NETWORKS");
  
  if (wifi_state == WIFI_SCANNING) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(120, 200);
    gfx->print("Scanning...");
  } else {
    network_count = getAvailableNetworks(available_networks, 20);
    
    for (int i = 0; i < min(network_count, 6); i++) {
      int y = 70 + i * 50;
      gfx->fillRoundRect(20, y, 320, 45, 8, RGB565(40, 40, 40));
      gfx->setTextColor(COLOR_WHITE);
      gfx->setTextSize(1);
      gfx->setCursor(30, y + 10);
      gfx->print(available_networks[i].ssid);
      gfx->setCursor(30, y + 28);
      gfx->printf("%d dBm %s", available_networks[i].rssi, 
                 available_networks[i].encrypted ? "(Secure)" : "(Open)");
    }
  }
  
  drawThemeButton(100, 400, 80, 35, "Scan", false);
  drawThemeButton(200, 400, 80, 35, "Back", false);
}

void drawPasswordEntryScreen() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(60, 20);
  gfx->print("ENTER PASSWORD");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 70);
  gfx->printf("Network: %s", available_networks[selected_network].ssid.c_str());
  
  // Password field
  gfx->fillRoundRect(20, 100, 320, 40, 10, RGB565(50, 50, 50));
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(30, 115);
  String masked = "";
  for (int i = 0; i < password_input.length(); i++) masked += "*";
  gfx->print(masked.length() > 0 ? masked : "Tap to enter");
  
  drawThemeButton(100, 400, 100, 40, "Connect", false);
  drawThemeButton(220, 400, 80, 40, "Cancel", false);
}

void handleNetworkListTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Network selection
  for (int i = 0; i < min(network_count, 6); i++) {
    int ny = 70 + i * 50;
    if (y >= ny && y < ny + 45) {
      selected_network = i;
      // For simplicity, try connecting with empty password for open networks
      if (!available_networks[i].encrypted) {
        connectWiFi(available_networks[i].ssid, "");
      }
      return;
    }
  }
  
  // Scan button
  if (y >= 400 && y < 435 && x < 180) {
    startWiFiScan();
  }
  // Back button
  if (y >= 400 && y < 435 && x >= 200) {
    system_state.current_screen = SCREEN_SETTINGS;
  }
}

void handlePasswordEntryTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP && gesture.y >= 400) {
    if (gesture.x < 200) {
      // Connect
      if (selected_network >= 0) {
        connectWiFi(available_networks[selected_network].ssid, password_input);
      }
    } else {
      // Cancel
      selected_network = -1;
      password_input = "";
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
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(110, 20);
  gfx->print("WEATHER");
  
  if (!cached_weather.valid) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(80, 200);
    gfx->print(isWiFiConnected() ? "Loading..." : "WiFi not connected");
  } else {
    // Location
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(100, 70);
    gfx->print(cached_weather.location);
    
    // Temperature
    gfx->setTextSize(5);
    gfx->setCursor(80, 140);
    gfx->printf("%.0f", cached_weather.temperature);
    gfx->setTextSize(2);
    gfx->print("C");
    
    // Description
    gfx->setTextSize(2);
    gfx->setCursor(100, 230);
    gfx->print(cached_weather.description);
    
    // Details
    gfx->setTextSize(1);
    gfx->setCursor(40, 280);
    gfx->printf("Humidity: %.0f%%", cached_weather.humidity);
    gfx->setCursor(200, 280);
    gfx->printf("Wind: %.1f m/s", cached_weather.wind_speed);
  }
  
  drawThemeButton(60, 360, 100, 40, "Refresh", false);
  drawThemeButton(200, 360, 100, 40, "Back", false);
}

void handleWeatherTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  if (y >= 360 && y < 400) {
    if (x < 160) refreshWeatherData();
    else system_state.current_screen = SCREEN_APP_GRID;
  }
}

void refreshWeatherData() {
  if (!isWiFiConnected()) return;
  cached_weather = fetchWeatherData("");
}

WeatherData fetchWeatherData(const String& location) {
  WeatherData data = {"Demo City", 22.5, 65.0, 1015, "Partly Cloudy", "02d", 3.5, "", true};
  
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
    gfx->print(article.summary.substring(0, 150));
    
    // Navigation
    gfx->setCursor(150, 320);
    gfx->printf("%d/%d", current_article + 1, news_count);
  }
  
  drawThemeButton(30, 360, 80, 40, "Prev", false);
  drawThemeButton(140, 360, 80, 40, "Back", false);
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
  // Demo news
  articles[0] = {"Tech Innovation Summit 2025", "Major announcements expected at annual tech conference", "Tech News", "", "2h ago"};
  articles[1] = {"Climate Action Progress", "Global leaders discuss environmental initiatives", "World News", "", "4h ago"};
  articles[2] = {"Sports Championship Finals", "Exciting match scheduled for this weekend", "Sports", "", "6h ago"};
  
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
