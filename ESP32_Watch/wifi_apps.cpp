/**
 * WiFi Apps Implementation
 */

#include "wifi_apps.h"
#include "ui_manager.h"
#include "themes.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherData weather = {24.0, 65.0, "Sunny", "sun"};
NewsItem news[5];
int newsCount = 0;

static const char* weatherCity = "Perth";

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initWifiApps() {
  // Initialize default news
  news[0] = {"Loading news...", "Please connect WiFi"};
  newsCount = 1;
  Serial.println("[OK] WiFi Apps initialized");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WIFI FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void connectWiFi(const char* ssid, const char* password) {
  Serial.printf("[WiFi] Connecting to %s...\n", ssid);
  WiFi.begin(ssid, password);
  
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    timeout--;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    watch.wifiConnected = true;
    Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    watch.wifiConnected = false;
    Serial.println("[WiFi] Connection failed");
  }
}

void disconnectWiFi() {
  WiFi.disconnect();
  watch.wifiConnected = false;
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WEATHER
// ═══════════════════════════════════════════════════════════════════════════════
void fetchWeather(const char* city) {
  if (!isWiFiConnected()) {
    weather.description = "No WiFi";
    return;
  }
  
  // OpenWeatherMap API (free tier)
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += city;
  url += "&units=metric&appid=YOUR_API_KEY"; // Replace with actual API key
  
  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  
  if (code == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    weather.temp = doc["main"]["temp"];
    weather.humidity = doc["main"]["humidity"];
    weather.description = "Fetched";
  } else {
    weather.description = "API Error";
  }
  
  http.end();
}

lv_obj_t* createWeatherScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "Weather");
  
  // Location
  lv_obj_t* locLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(locLbl, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(locLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text(locLbl, weatherCity);
  lv_obj_align(locLbl, LV_ALIGN_TOP_MID, 0, 70);
  
  // Weather icon (placeholder)
  lv_obj_t* iconBg = lv_obj_create(scr);
  lv_obj_set_size(iconBg, 120, 120);
  lv_obj_set_style_radius(iconBg, 60, 0);
  lv_obj_set_style_bg_color(iconBg, lv_color_hex(0xFFD700), 0);
  lv_obj_align(iconBg, LV_ALIGN_CENTER, 0, -30);
  
  lv_obj_t* sunIcon = lv_label_create(iconBg);
  lv_obj_set_style_text_font(sunIcon, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(sunIcon, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(sunIcon, LV_SYMBOL_IMAGE); // Use as sun placeholder
  lv_obj_center(sunIcon);
  
  // Temperature
  lv_obj_t* tempLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(tempLbl, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(tempLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text_fmt(tempLbl, "%.0f\xC2\xB0C", weather.temp);
  lv_obj_align(tempLbl, LV_ALIGN_CENTER, 0, 80);
  
  // Humidity
  lv_obj_t* humLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(humLbl, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(humLbl, lv_color_hex(colors.secondary), 0);
  lv_label_set_text_fmt(humLbl, "Humidity: %.0f%%", weather.humidity);
  lv_obj_align(humLbl, LV_ALIGN_CENTER, 0, 130);
  
  // Description
  lv_obj_t* descLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(descLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(descLbl, lv_color_hex(colors.accent), 0);
  lv_label_set_text(descLbl, weather.description);
  lv_obj_align(descLbl, LV_ALIGN_CENTER, 0, 160);
  
  // Refresh button
  lv_obj_t* refreshBtn = lv_btn_create(scr);
  lv_obj_set_size(refreshBtn, 150, 45);
  lv_obj_align(refreshBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(refreshBtn, lv_color_hex(colors.primary), 0);
  lv_obj_add_event_cb(refreshBtn, [](lv_event_t* e) {
    fetchWeather(weatherCity);
  }, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* refLbl = lv_label_create(refreshBtn);
  lv_label_set_text(refLbl, "Refresh");
  lv_obj_center(refLbl);
  
  // WiFi status
  lv_obj_t* wifiLbl = lv_label_create(scr);
  lv_obj_set_style_text_font(wifiLbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(wifiLbl, lv_color_hex(watch.wifiConnected ? 0x4CAF50 : 0xFF5722), 0);
  lv_label_set_text(wifiLbl, watch.wifiConnected ? "WiFi: Connected" : "WiFi: Not Connected");
  lv_obj_align(wifiLbl, LV_ALIGN_BOTTOM_MID, 0, -70);
  
  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  NEWS
// ═══════════════════════════════════════════════════════════════════════════════
void fetchNews() {
  // Placeholder news for demo
  news[0] = {"Tech stocks rally amid AI boom", "Reuters"};
  news[1] = {"New gaming console announced", "IGN"};
  news[2] = {"Sports: Championship results", "ESPN"};
  news[3] = {"Weather: Storm warning issued", "Weather.com"};
  news[4] = {"Science: New discovery made", "Nature"};
  newsCount = 5;
}

lv_obj_t* createNewsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "News");
  
  fetchNews();
  
  int yPos = 60;
  for (int i = 0; i < newsCount && i < 4; i++) {
    lv_obj_t* card = lv_obj_create(scr);
    lv_obj_set_size(card, LCD_WIDTH - 20, 80);
    lv_obj_set_pos(card, 10, yPos);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    
    lv_obj_t* title = lv_label_create(card);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(colors.text), 0);
    lv_obj_set_width(title, LCD_WIDTH - 60);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(title, news[i].title);
    lv_obj_set_pos(title, 10, 10);
    
    lv_obj_t* source = lv_label_create(card);
    lv_obj_set_style_text_font(source, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(source, lv_color_hex(colors.secondary), 0);
    lv_label_set_text(source, news[i].source);
    lv_obj_set_pos(source, 10, 50);
    
    yPos += 90;
  }
  
  // WiFi note
  lv_obj_t* note = lv_label_create(scr);
  lv_obj_set_style_text_font(note, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(note, lv_color_hex(colors.secondary), 0);
  lv_label_set_text(note, watch.wifiConnected ? "Live news feed" : "Demo news (no WiFi)");
  lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -15);
  
  return scr;
}
