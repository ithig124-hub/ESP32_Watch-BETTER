/**
 * Utility Apps - Calculator, Flashlight, Coin Flip, Stopwatch
 */

#include "utilities.h"
#include "ui_manager.h"
#include "themes.h"

// stopwatch is defined in user_data.cpp

// ═══════════════════════════════════════════════════════════════════════════════
//  CALCULATOR
// ═══════════════════════════════════════════════════════════════════════════════
namespace Calculator {
  void init() {
    // Calculator init
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    createTitleBar(scr, "Calculator");
    return scr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  FLASHLIGHT
// ═══════════════════════════════════════════════════════════════════════════════
namespace Flashlight {
  static bool isOn = false;
  
  void init() {
    isOn = false;
  }
  
  void toggle() {
    isOn = !isOn;
    // In real implementation, would control display brightness
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, isOn ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x000000), 0);
    return scr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COIN FLIP
// ═══════════════════════════════════════════════════════════════════════════════
namespace CoinFlip {
  static bool lastResult = false;
  
  void init() {
    lastResult = false;
  }
  
  void flip() {
    lastResult = random(2) == 1;
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    createTitleBar(scr, "Coin Flip");
    return scr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STOPWATCH
// ═══════════════════════════════════════════════════════════════════════════════
namespace Stopwatch {
  void init() {
    stopwatch.elapsedMs = 0;
    stopwatch.startTime = 0;
    stopwatch.lapCount = 0;
    stopwatch.running = false;
    stopwatch.paused = false;
    stopwatch.pausedTime = 0;
    for (int i = 0; i < 10; i++) {
      stopwatch.laps[i] = 0;
    }
  }
  
  void update() {
    if (stopwatch.running && !stopwatch.paused) {
      stopwatch.elapsedMs = millis() - stopwatch.startTime;
    }
  }
  
  void start() {
    if (!stopwatch.running) {
      stopwatch.startTime = millis();
      stopwatch.running = true;
      stopwatch.paused = false;
    } else if (stopwatch.paused) {
      stopwatch.startTime = millis() - stopwatch.pausedTime;
      stopwatch.paused = false;
    }
  }
  
  void stop() {
    if (stopwatch.running && !stopwatch.paused) {
      stopwatch.pausedTime = stopwatch.elapsedMs;
      stopwatch.paused = true;
    }
  }
  
  void reset() {
    init();
  }
  
  void lap() {
    if (stopwatch.running && stopwatch.lapCount < 10) {
      stopwatch.laps[stopwatch.lapCount++] = stopwatch.elapsedMs;
    }
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    createTitleBar(scr, "Stopwatch");
    return scr;
  }
}
