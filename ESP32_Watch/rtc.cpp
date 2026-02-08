/**
 * RTC Implementation - PCF85063 (Fixed function order)
 */

#include "rtc.h"
#include <Wire.h>
#include <WiFi.h>
#include <time.h>

RTCTime rtcTime = {0, 0, 12, 1, 0, 1, 2025};

static const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* monthNames[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// ═══════════════════════════════════════════════════════════════════════════════
//  HELPER FUNCTIONS (defined first)
// ═══════════════════════════════════════════════════════════════════════════════
static uint8_t bcdToDec(uint8_t val) {
  return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t decToBcd(uint8_t val) {
  return ((val / 10) << 4) | (val % 10);
}

static const char* getMonthName(uint8_t month) {
  if (month >= 1 && month <= 12) return monthNames[month - 1];
  return "???";
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PUBLIC FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
bool initRTC() {
  Wire.beginTransmission(RTC_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("[WARN] RTC not found");
    hasRTC = false;
    return false;
  }
  
  // Initialize RTC control registers
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00); // Control_1 register
  Wire.write(0x00); // Normal mode
  Wire.endTransmission();
  
  hasRTC = true;
  Serial.println("[OK] RTC PCF85063 initialized");
  
  return readRTC();
}

bool readRTC() {
  if (!hasRTC) return false;
  
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x04); // Seconds register
  if (Wire.endTransmission(false) != 0) return false;
  
  Wire.requestFrom((uint8_t)RTC_ADDR, (uint8_t)7);
  if (Wire.available() < 7) return false;
  
  rtcTime.second  = bcdToDec(Wire.read() & 0x7F);
  rtcTime.minute  = bcdToDec(Wire.read() & 0x7F);
  rtcTime.hour    = bcdToDec(Wire.read() & 0x3F);
  rtcTime.day     = bcdToDec(Wire.read() & 0x3F);
  rtcTime.weekday = Wire.read() & 0x07;
  rtcTime.month   = bcdToDec(Wire.read() & 0x1F);
  uint8_t yearVal = bcdToDec(Wire.read());
  rtcTime.year    = 2000 + yearVal;
  
  // Update watch state
  watch.hour = rtcTime.hour;
  watch.minute = rtcTime.minute;
  watch.second = rtcTime.second;
  watch.day = rtcTime.day;
  watch.month = rtcTime.month;
  watch.year = rtcTime.year;
  watch.dayOfWeek = rtcTime.weekday;
  
  clockHour = rtcTime.hour;
  clockMinute = rtcTime.minute;
  clockSecond = rtcTime.second;
  
  return true;
}

bool setRTC(RTCTime& time) {
  if (!hasRTC) return false;
  
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x04); // Start at seconds register
  Wire.write(decToBcd(time.second) & 0x7F);
  Wire.write(decToBcd(time.minute));
  Wire.write(decToBcd(time.hour));
  Wire.write(decToBcd(time.day));
  Wire.write(time.weekday);
  Wire.write(decToBcd(time.month));
  Wire.write(decToBcd(time.year - 2000));
  
  return Wire.endTransmission() == 0;
}

bool syncWithNTP(const char* ntpServer) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[RTC] Cannot sync - WiFi not connected");
    return false;
  }
  
  Serial.printf("[RTC] Syncing with NTP server: %s\n", ntpServer);
  
  configTime(0, 0, ntpServer);
  
  struct tm timeinfo;
  int retries = 10;
  while (!getLocalTime(&timeinfo) && retries > 0) {
    delay(500);
    retries--;
  }
  
  if (retries == 0) {
    Serial.println("[RTC] NTP sync failed");
    return false;
  }
  
  RTCTime newTime;
  newTime.second  = timeinfo.tm_sec;
  newTime.minute  = timeinfo.tm_min;
  newTime.hour    = timeinfo.tm_hour;
  newTime.day     = timeinfo.tm_mday;
  newTime.weekday = timeinfo.tm_wday;
  newTime.month   = timeinfo.tm_mon + 1;
  newTime.year    = timeinfo.tm_year + 1900;
  
  if (setRTC(newTime)) {
    Serial.printf("[RTC] Synced: %02d:%02d:%02d %02d/%02d/%04d\n",
      newTime.hour, newTime.minute, newTime.second,
      newTime.day, newTime.month, newTime.year);
    return true;
  }
  
  return false;
}

String getTimeString() {
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", rtcTime.hour, rtcTime.minute);
  return String(buf);
}

String getDateString() {
  char buf[32];
  snprintf(buf, sizeof(buf), "%s, %s %d",
    dayNames[rtcTime.weekday],
    getMonthName(rtcTime.month),
    rtcTime.day);
  return String(buf);
}

const char* getDayName(uint8_t day) {
  if (day < 7) return dayNames[day];
  return "???";
}
