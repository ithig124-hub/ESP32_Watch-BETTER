/**
 * WiFi Apps - Weather & News
 */

#ifndef WIFI_APPS_H
#define WIFI_APPS_H

#include "config.h"
#include <lvgl.h>

// Initialize WiFi apps
void initWifiApps();

// Create screens
lv_obj_t* createWeatherScreen();
lv_obj_t* createNewsScreen();

// WiFi connection
void connectWiFi(const char* ssid, const char* password);
void disconnectWiFi();
bool isWiFiConnected();

// Weather
void fetchWeather(const char* city);
struct WeatherData {
  float temp;
  float humidity;
  const char* description;
  const char* icon;
};
extern WeatherData weather;

// News
void fetchNews();
struct NewsItem {
  const char* title;
  const char* source;
};
extern NewsItem news[5];
extern int newsCount;

#endif
