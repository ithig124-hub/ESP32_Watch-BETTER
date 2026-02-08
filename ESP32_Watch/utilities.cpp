/**
 * Utility Apps Implementation
 */

#include "utilities.h"
#include "ui_manager.h"
#include "themes.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  CALCULATOR
// ═══════════════════════════════════════════════════════════════════════════════
namespace Calculator {
  char display[32] = "0";
  double currentValue = 0;
  double storedValue = 0;
  char lastOperator = 0;
  bool newNumber = true;
  
  static lv_obj_t* displayLabel = nullptr;
  
  void init() {
    strcpy(display, "0");
    currentValue = 0;
    storedValue = 0;
    lastOperator = 0;
    newNumber = true;
  }
  
  void clear() {
    init();
    if (displayLabel) lv_label_set_text(displayLabel, "0");
  }
  
  void pressKey(char key) {
    if (key >= '0' && key <= '9') {
      if (newNumber) {
        display[0] = key;
        display[1] = '\0';
        newNumber = false;
      } else if (strlen(display) < 15) {
        int len = strlen(display);
        display[len] = key;
        display[len + 1] = '\0';
      }
      currentValue = atof(display);
    } else if (key == '.') {
      if (!strchr(display, '.') && strlen(display) < 15) {
        int len = strlen(display);
        display[len] = '.';
        display[len + 1] = '\0';
        newNumber = false;
      }
    } else if (key == '+' || key == '-' || key == '*' || key == '/') {
      calculate();
      storedValue = currentValue;
      lastOperator = key;
      newNumber = true;
    } else if (key == '=') {
      calculate();
      lastOperator = 0;
      newNumber = true;
    } else if (key == 'C') {
      clear();
    }
    
    if (displayLabel) lv_label_set_text(displayLabel, display);
  }
  
  void calculate() {
    if (lastOperator == 0) return;
    
    switch (lastOperator) {
      case '+': currentValue = storedValue + currentValue; break;
      case '-': currentValue = storedValue - currentValue; break;
      case '*': currentValue = storedValue * currentValue; break;
      case '/':
        if (currentValue != 0) currentValue = storedValue / currentValue;
        else { strcpy(display, "Error"); return; }
        break;
    }
    
    if (currentValue == (long)currentValue) {
      snprintf(display, sizeof(display), "%ld", (long)currentValue);
    } else {
      snprintf(display, sizeof(display), "%.6g", currentValue);
    }
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    
    createTitleBar(scr, "Calculator");
    
    // Display
    lv_obj_t* dispBg = lv_obj_create(scr);
    lv_obj_set_size(dispBg, LCD_WIDTH - 20, 60);
    lv_obj_set_pos(dispBg, 10, 55);
    lv_obj_set_style_bg_color(dispBg, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(dispBg, 8, 0);
    
    displayLabel = lv_label_create(dispBg);
    lv_obj_set_style_text_font(displayLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(displayLabel, lv_color_hex(colors.text), 0);
    lv_label_set_text(displayLabel, display);
    lv_obj_align(displayLabel, LV_ALIGN_RIGHT_MID, -10, 0);
    
    // Button grid
    const char* keys[] = {
      "C", "/", "*", "-",
      "7", "8", "9", "+",
      "4", "5", "6", "=",
      "1", "2", "3", ".",
      "0"
    };
    
    int btnW = 80, btnH = 55;
    int gap = 6;
    int startX = 10, startY = 125;
    
    for (int i = 0; i < 17; i++) {
      int row = i / 4;
      int col = i % 4;
      
      if (i == 16) { // Wide zero button
        row = 4; col = 0;
        btnW = 166;
      } else {
        btnW = 80;
      }
      
      lv_obj_t* btn = lv_btn_create(scr);
      lv_obj_set_size(btn, btnW, btnH);
      lv_obj_set_pos(btn, startX + col * 86, startY + row * (btnH + gap));
      
      // Color based on key type
      uint32_t btnColor = 0x333333;
      if (keys[i][0] == 'C') btnColor = 0xFF5722;
      else if (strchr("+-*/=", keys[i][0])) btnColor = colors.primary;
      
      lv_obj_set_style_bg_color(btn, lv_color_hex(btnColor), 0);
      lv_obj_set_style_radius(btn, 10, 0);
      
      char keyChar = keys[i][0];
      lv_obj_set_user_data(btn, (void*)(intptr_t)keyChar);
      
      lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        char key = (char)(intptr_t)lv_obj_get_user_data(btn);
        pressKey(key);
      }, LV_EVENT_CLICKED, NULL);
      
      lv_obj_t* lbl = lv_label_create(btn);
      lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
      lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
      lv_label_set_text(lbl, keys[i]);
      lv_obj_center(lbl);
    }
    
    return scr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  FLASHLIGHT
// ═══════════════════════════════════════════════════════════════════════════════
namespace Flashlight {
  bool isOn = false;
  int brightness = 255;
  static lv_obj_t* statusLabel = nullptr;
  static lv_obj_t* flashScreen = nullptr;
  
  void init() {
    isOn = false;
    brightness = 255;
  }
  
  void toggle() {
    isOn = !isOn;
    if (flashScreen) {
      lv_obj_set_style_bg_color(flashScreen, 
        isOn ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x111111), 0);
    }
    if (statusLabel) {
      lv_label_set_text(statusLabel, isOn ? "TAP TO TURN OFF" : "TAP TO TURN ON");
      lv_obj_set_style_text_color(statusLabel, 
        isOn ? lv_color_hex(0x000000) : lv_color_hex(0xFFFFFF), 0);
    }
  }
  
  void setBrightness(int level) {
    brightness = constrain(level, 50, 255);
  }
  
  lv_obj_t* createScreen() {
    flashScreen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(flashScreen, lv_color_hex(0x111111), 0);
    
    // Make entire screen tappable
    lv_obj_add_event_cb(flashScreen, [](lv_event_t* e) {
      toggle();
    }, LV_EVENT_CLICKED, NULL);
    
    // Icon
    lv_obj_t* icon = lv_label_create(flashScreen);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFD700), 0);
    lv_label_set_text(icon, LV_SYMBOL_CHARGE);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -40);
    
    // Status
    statusLabel = lv_label_create(flashScreen);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(statusLabel, "TAP TO TURN ON");
    lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, 40);
    
    // Back button
    lv_obj_t* backBtn = lv_btn_create(flashScreen);
    lv_obj_set_size(backBtn, 80, 40);
    lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x333333), 0);
    lv_obj_add_event_cb(backBtn, [](lv_event_t* e) { 
      isOn = false;
      goBack(); 
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* backLbl = lv_label_create(backBtn);
    lv_label_set_text(backLbl, LV_SYMBOL_LEFT " Back");
    lv_obj_center(backLbl);
    
    return flashScreen;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  COIN FLIP
// ═══════════════════════════════════════════════════════════════════════════════
namespace CoinFlip {
  bool isHeads = true;
  int headsCount = 0;
  int tailsCount = 0;
  bool flipping = false;
  
  static lv_obj_t* coinLabel = nullptr;
  static lv_obj_t* statsLabel = nullptr;
  
  void init() {
    isHeads = true;
    headsCount = 0;
    tailsCount = 0;
    flipping = false;
  }
  
  void flip() {
    isHeads = random(2) == 0;
    if (isHeads) headsCount++;
    else tailsCount++;
    
    if (coinLabel) {
      lv_label_set_text(coinLabel, isHeads ? "HEADS" : "TAILS");
    }
    if (statsLabel) {
      char buf[64];
      snprintf(buf, sizeof(buf), "H: %d | T: %d", headsCount, tailsCount);
      lv_label_set_text(statsLabel, buf);
    }
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    
    createTitleBar(scr, "Coin Flip");
    
    // Coin circle
    lv_obj_t* coin = lv_obj_create(scr);
    lv_obj_set_size(coin, 180, 180);
    lv_obj_set_style_radius(coin, 90, 0);
    lv_obj_set_style_bg_color(coin, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_border_width(coin, 4, 0);
    lv_obj_set_style_border_color(coin, lv_color_hex(0xDAA520), 0);
    lv_obj_align(coin, LV_ALIGN_CENTER, 0, -30);
    
    coinLabel = lv_label_create(coin);
    lv_obj_set_style_text_font(coinLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(coinLabel, lv_color_hex(0x000000), 0);
    lv_label_set_text(coinLabel, "TAP");
    lv_obj_center(coinLabel);
    
    // Make coin tappable
    lv_obj_add_flag(coin, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(coin, [](lv_event_t* e) {
      flip();
    }, LV_EVENT_CLICKED, NULL);
    
    // Stats
    statsLabel = lv_label_create(scr);
    lv_obj_set_style_text_font(statsLabel, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(statsLabel, lv_color_hex(colors.text), 0);
    lv_label_set_text(statsLabel, "H: 0 | T: 0");
    lv_obj_align(statsLabel, LV_ALIGN_CENTER, 0, 100);
    
    // Reset button
    lv_obj_t* resetBtn = lv_btn_create(scr);
    lv_obj_set_size(resetBtn, 120, 45);
    lv_obj_align(resetBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(resetBtn, lv_color_hex(colors.secondary), 0);
    lv_obj_add_event_cb(resetBtn, [](lv_event_t* e) {
      headsCount = 0;
      tailsCount = 0;
      if (coinLabel) lv_label_set_text(coinLabel, "TAP");
      if (statsLabel) lv_label_set_text(statsLabel, "H: 0 | T: 0");
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* resetLbl = lv_label_create(resetBtn);
    lv_label_set_text(resetLbl, "Reset");
    lv_obj_center(resetLbl);
    
    return scr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  STOPWATCH
// ═══════════════════════════════════════════════════════════════════════════════
namespace Stopwatch {
  StopwatchData data = {0, 0, {0}, 0, false, false, 0};
  
  static lv_obj_t* timeLabel = nullptr;
  static lv_obj_t* startBtn = nullptr;
  static lv_obj_t* lapList = nullptr;
  
  void init() {
    data.startTime = 0;
    data.elapsed = 0;
    data.lapCount = 0;
    data.running = false;
    data.paused = false;
    data.pausedTime = 0;
  }
  
  String formatTime(unsigned long ms) {
    int hours = ms / 3600000;
    int mins = (ms % 3600000) / 60000;
    int secs = (ms % 60000) / 1000;
    int millis = (ms % 1000) / 10;
    
    char buf[32];
    if (hours > 0) {
      snprintf(buf, sizeof(buf), "%d:%02d:%02d.%02d", hours, mins, secs, millis);
    } else {
      snprintf(buf, sizeof(buf), "%02d:%02d.%02d", mins, secs, millis);
    }
    return String(buf);
  }
  
  void start() {
    if (data.paused) {
      data.startTime = millis() - data.pausedTime;
      data.paused = false;
    } else {
      data.startTime = millis();
    }
    data.running = true;
  }
  
  void stop() {
    if (data.running) {
      data.pausedTime = millis() - data.startTime;
      data.running = false;
      data.paused = true;
    }
  }
  
  void reset() {
    init();
    if (timeLabel) lv_label_set_text(timeLabel, "00:00.00");
  }
  
  void lap() {
    if (data.running && data.lapCount < 10) {
      data.lapTimes[data.lapCount++] = millis() - data.startTime;
    }
  }
  
  void update() {
    if (data.running && timeLabel) {
      unsigned long elapsed = millis() - data.startTime;
      lv_label_set_text(timeLabel, formatTime(elapsed).c_str());
    }
  }
  
  lv_obj_t* createScreen() {
    lv_obj_t* scr = lv_obj_create(NULL);
    ThemeColors colors = getThemeColors(watch.theme);
    lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
    
    createTitleBar(scr, "Stopwatch");
    
    // Time display
    timeLabel = lv_label_create(scr);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(colors.text), 0);
    lv_label_set_text(timeLabel, "00:00.00");
    lv_obj_align(timeLabel, LV_ALIGN_TOP_MID, 0, 80);
    
    // Control buttons
    startBtn = lv_btn_create(scr);
    lv_obj_set_size(startBtn, 140, 60);
    lv_obj_align(startBtn, LV_ALIGN_CENTER, -80, 50);
    lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x4CAF50), 0);
    lv_obj_add_event_cb(startBtn, [](lv_event_t* e) {
      if (data.running) {
        stop();
        lv_obj_t* btn = lv_event_get_target(e);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), 0);
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        if (lbl) lv_label_set_text(lbl, "Start");
      } else {
        start();
        lv_obj_t* btn = lv_event_get_target(e);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5722), 0);
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        if (lbl) lv_label_set_text(lbl, "Stop");
      }
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* startLbl = lv_label_create(startBtn);
    lv_obj_set_style_text_font(startLbl, &lv_font_montserrat_18, 0);
    lv_label_set_text(startLbl, "Start");
    lv_obj_center(startLbl);
    
    // Lap/Reset button
    lv_obj_t* lapBtn = lv_btn_create(scr);
    lv_obj_set_size(lapBtn, 140, 60);
    lv_obj_align(lapBtn, LV_ALIGN_CENTER, 80, 50);
    lv_obj_set_style_bg_color(lapBtn, lv_color_hex(colors.secondary), 0);
    lv_obj_add_event_cb(lapBtn, [](lv_event_t* e) {
      if (data.running) {
        lap();
      } else {
        reset();
        // Reset start button
        if (startBtn) {
          lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x4CAF50), 0);
          lv_obj_t* lbl = lv_obj_get_child(startBtn, 0);
          if (lbl) lv_label_set_text(lbl, "Start");
        }
      }
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* lapLbl = lv_label_create(lapBtn);
    lv_obj_set_style_text_font(lapLbl, &lv_font_montserrat_18, 0);
    lv_label_set_text(lapLbl, "Lap/Reset");
    lv_obj_center(lapLbl);
    
    // Lap times display
    lv_obj_t* lapBg = lv_obj_create(scr);
    lv_obj_set_size(lapBg, LCD_WIDTH - 40, 150);
    lv_obj_align(lapBg, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(lapBg, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(lapBg, 10, 0);
    lv_obj_set_scrollbar_mode(lapBg, LV_SCROLLBAR_MODE_AUTO);
    
    lv_obj_t* lapTitle = lv_label_create(lapBg);
    lv_obj_set_style_text_color(lapTitle, lv_color_hex(colors.accent), 0);
    lv_label_set_text(lapTitle, "Lap Times");
    lv_obj_align(lapTitle, LV_ALIGN_TOP_LEFT, 10, 5);
    
    return scr;
  }
}
