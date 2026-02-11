# ESP32 Watch - Changelog

## v2.0.0 - CRITICAL FIX (2026-02-10)

### 🔴 CRITICAL BUG FIXED: Touch Controller Not Working

**Root Cause Analysis:**
The touch controller was using the wrong interrupt pin. GPIO 21 is actually the IMU (QMI8658) interrupt pin, not the touch (FT3168) interrupt pin.

By comparing with the working `S3_MiniOS` reference implementation, we identified the following issues:

### Changes Made:

#### 1. `pin_config.h` - Pin Definition Fixes

| Pin | Original (Broken) | Fixed | Description |
|-----|-------------------|-------|-------------|
| `TP_INT` | 21 | **38** | Touch interrupt - GPIO 21 is IMU! |
| `TP_RST` | -1 | **9** | Touch reset - was disabled |
| `IMU_INT` | (missing) | **21** | Now correctly defined |
| `RTC_INT` | (missing) | **39** | RTC interrupt |
| `LCD_RESET` | (missing) | **8** | Display reset |
| `FT3168_DEVICE_ADDRESS` | (missing) | **0x38** | For Arduino_DriveBus |
| `QMI8658_L_SLAVE_ADDRESS` | (missing) | **0x6B** | For sensor library |

#### 2. `ESP32_Watch.ino` - Touch Initialization Fix

**Before:**
```cpp
void initTouch() {
  pinMode(TP_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TP_INT), Arduino_IIC_Touch_Interrupt, FALLING);
  
  if (FT3168->begin()) {
    // ...
  }
}
```

**After (matching S3_MiniOS):**
```cpp
void initTouch() {
  // Hardware reset sequence (was missing!)
  #if TP_RST >= 0
    pinMode(TP_RST, OUTPUT);
    digitalWrite(TP_RST, LOW);
    delay(20);
    digitalWrite(TP_RST, HIGH);
    delay(100);
  #endif

  pinMode(TP_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TP_INT), Arduino_IIC_Touch_Interrupt, FALLING);

  // Software reset (was missing!)
  FT3168->IIC_Device_Reset();
  delay(50);

  if (FT3168->begin()) {
    FT3168->IIC_Init();  // Was missing!
    // ...
  }
}
```

### Reference

These fixes were derived from the working `S3_MiniOS` implementation:
- Repository: `https://github.com/ithig124-hub/ESP32-MINI-Ino`
- File: `S3_MiniOS/pin_config.h` and `S3_MiniOS/S3_MiniOS.ino`

### Testing

After applying these fixes, the touch controller should:
1. Initialize successfully (no retry loops)
2. Respond to touch input
3. Support tap-to-wake functionality
4. Support swipe gestures

---

## v1.0.0 - Initial Release

- Premium anime character themes
- LVGL-based UI
- Power management with screen timeout disable
- Tap to wake (not working due to pin bug)
- Power button toggle
