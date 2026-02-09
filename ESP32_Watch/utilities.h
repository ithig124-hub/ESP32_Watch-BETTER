/**
 * Utility Apps - Calculator, Flashlight, Coin Flip, Stopwatch
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <lvgl.h>
#include "config.h"

namespace Calculator {
  void init();
  lv_obj_t* createScreen();
}

namespace Flashlight {
  void init();
  lv_obj_t* createScreen();
  void toggle();
}

namespace CoinFlip {
  void init();
  lv_obj_t* createScreen();
  void flip();
}

namespace Stopwatch {
  void init();
  void update();
  void start();
  void stop();
  void reset();
  void lap();
  lv_obj_t* createScreen();
}

#endif
