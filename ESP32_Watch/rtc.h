/**
 * RTC Interface - PCF85063
 */

#ifndef RTC_H
#define RTC_H

#include "config.h"

bool initRTC();
void readRTC();
void setRTC(int year, int month, int day, int hour, int minute, int second);

#endif
