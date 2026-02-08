/**
 * Utility Apps - Calculator, Flashlight, Coin Flip, Stopwatch
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "config.h"
#include <lvgl.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  CALCULATOR APP
// ═══════════════════════════════════════════════════════════════════════════════
namespace Calculator {
  void init();
  lv_obj_t* createScreen();
  void pressKey(char key);
  void calculate();
  void clear();
  
  extern char display[32];
  extern double currentValue;
  extern double storedValue;
  extern char lastOperator;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  FLASHLIGHT APP
// ═══════════════════════════════════════════════════════════════════════════════
namespace Flashlight {
  void init();
  lv_obj_t* createScreen();
  void toggle();
  void setBrightness(int level);
  
  extern bool isOn;
  extern int brightness;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COIN FLIP APP
// ═══════════════════════════════════════════════════════════════════════════════
namespace CoinFlip {
  void init();
  lv_obj_t* createScreen();
  void flip();
  
  extern bool isHeads;
  extern int headsCount;
  extern int tailsCount;
  extern bool flipping;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STOPWATCH APP
// ═══════════════════════════════════════════════════════════════════════════════
namespace Stopwatch {
  void init();
  lv_obj_t* createScreen();
  void start();
  void stop();
  void reset();
  void lap();
  void update();
  String formatTime(unsigned long ms);
  
  extern StopwatchData data;
}

#endif
