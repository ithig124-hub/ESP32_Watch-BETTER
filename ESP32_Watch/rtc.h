/**
 * RTC Module - PCF85063 Real Time Clock
 */

#ifndef RTC_H
#define RTC_H

#include "config.h"

#define RTC_ADDR 0x51

struct RTCTime {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t weekday;
  uint8_t month;
  uint16_t year;
};

extern RTCTime rtcTime;

// Initialize RTC
bool initRTC();

// Read time from RTC
bool readRTC();

// Set time on RTC
bool setRTC(RTCTime& time);

// Sync with NTP
bool syncWithNTP(const char* ntpServer = "pool.ntp.org");

// Get formatted time string
String getTimeString();
String getDateString();
const char* getDayName(uint8_t day);

#endif
