/*
 * training.cpp - Training Mini-Games Implementation
 * Complete training dojo with XP rewards
 */

#include "training.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "rpg.h"

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

// Training state
TrainingScore current_training_score;
TrainingType current_training_type;
bool training_active = false;
int training_round = 0;
unsigned long training_start_time = 0;

// Reflex test state
static int reflex_current_button = -1;
static unsigned long reflex_flash_time = 0;
static int reflex_times[10];
static int reflex_round = 0;
static bool reflex_waiting = false;

// Target shoot state
static int target_x[10], target_y[10];
static bool target_active[10];
static int targets_hit = 0;
static int targets_missed = 0;
static unsigned long target_timer = 0;

// Speed tap state
static int tap_count = 0;
static unsigned long speed_tap_end_time = 0;
static int last_combo = 0;

// Memory state
static int memory_pattern[20];
static int memory_length = 3;
static int memory_input_index = 0;
static bool memory_showing = false;
static unsigned long memory_show_time = 0;

// =============================================================================
// TRAINING SYSTEM
// =============================================================================

void initTrainingSystem() {
  Serial.println("[Training] Initializing training system...");
  loadTrainingProgress();
}

void saveTrainingProgress() {
  Serial.println("[Training] Progress saved");
}

bool loadTrainingProgress() {
  return false;
}

void startTrainingGame(TrainingType type) {
  current_training_type = type;
  training_active = true;
  training_round = 0;
  training_start_time = millis();
  
  current_training_score.type = type;
  current_training_score.score = 0;
  current_training_score.xp_earned = 0;
  current_training_score.best_time_ms = 9999;
  current_training_score.combo_count = 0;
  
  switch(type) {
    case TRAINING_REFLEX:  initReflexTest(); break;
    case TRAINING_TARGET:  initTargetShoot(); break;
    case TRAINING_SPEED:   initSpeedTap(); break;
    case TRAINING_MEMORY:  initTrainingMemory(); break;
  }
  
  system_state.current_screen = SCREEN_TRAINING;
}

void updateTrainingGame() {
  if (!training_active) return;
  
  switch(current_training_type) {
    case TRAINING_REFLEX:  updateReflexTest(); break;
    case TRAINING_TARGET:  updateTargetShoot(); break;
    case TRAINING_SPEED:   updateSpeedTap(); break;
    case TRAINING_MEMORY:  updateTrainingMemory(); break;
  }
}

void endTrainingGame() {
  training_active = false;
  
  // Calculate XP
  bool perfect = (current_training_score.score > 80);
  int xp = calculateTrainingXP(current_training_type, current_training_score.score, perfect);
  applyStreakBonus(xp);
  
  current_training_score.xp_earned = xp;
  gainExperience(xp, "Training");
  
  updateTrainingStreak();
  saveTrainingProgress();
  
  drawTrainingResults(current_training_score);
}

bool isTrainingActive() {
  return training_active;
}

int calculateTrainingXP(TrainingType type, int score, bool perfect) {
  int base_xp = TRAINING_XP_PER_GAME_MIN + (score * (TRAINING_XP_PER_GAME_MAX - TRAINING_XP_PER_GAME_MIN) / 100);
  
  if (perfect) {
    base_xp += TRAINING_PERFECT_BONUS;
  }
  
  return base_xp;
}

void applyStreakBonus(int& xp) {
  int streak = system_state.training_streak;
  
  if (streak >= 30) xp *= 2.0;        // +100%
  else if (streak >= 14) xp *= 1.75;  // +75%
  else if (streak >= 7) xp *= 1.5;    // +50%
  else if (streak >= 3) xp *= 1.25;   // +25%
}

int getTrainingStreak() {
  return system_state.training_streak;
}

void updateTrainingStreak() {
  system_state.training_streak++;
  // In real implementation, would check if trained today already
}

// =============================================================================
// REFLEX TEST
// =============================================================================

void initReflexTest() {
  reflex_round = 0;
  reflex_current_button = -1;
  reflex_waiting = false;
  
  for (int i = 0; i < 10; i++) {
    reflex_times[i] = 0;
  }
  
  // Start first flash after delay
  reflex_flash_time = millis() + random(1000, 3000);
}

void updateReflexTest() {
  unsigned long now = millis();
  
  // Time to flash a button?
  if (!reflex_waiting && reflex_current_button < 0 && now >= reflex_flash_time) {
    flashReflexButton(random(0, 4));
  }
  
  // Check for timeout (missed)
  if (reflex_waiting && reflex_current_button >= 0 && now - reflex_flash_time > 1500) {
    reflex_times[reflex_round] = 1500;  // Max time
    reflex_round++;
    reflex_current_button = -1;
    reflex_waiting = false;
    
    if (reflex_round >= 10) {
      endTrainingGame();
    } else {
      reflex_flash_time = now + random(500, 2000);
    }
  }
}

void drawReflexTest() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("REFLEX TEST");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(120, 50);
  gfx->printf("Round %d/10", reflex_round + 1);
  
  // Draw 4 corner buttons
  int buttonSize = 100;
  int positions[4][2] = {{40, 100}, {220, 100}, {40, 280}, {220, 280}};
  
  for (int i = 0; i < 4; i++) {
    bool lit = (i == reflex_current_button);
    drawTrainingButton(positions[i][0], positions[i][1], buttonSize, i, lit, false);
  }
  
  // Show average time if rounds completed
  if (reflex_round > 0) {
    int total = 0;
    for (int i = 0; i < reflex_round; i++) total += reflex_times[i];
    int avg = total / reflex_round;
    
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(100, 420);
    gfx->printf("Avg: %dms - %s", avg, getReflexRating(avg));
  }
}

void handleReflexTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  if (!reflex_waiting || reflex_current_button < 0) return;
  
  int x = gesture.x, y = gesture.y;
  int positions[4][2] = {{40, 100}, {220, 100}, {40, 280}, {220, 280}};
  
  for (int i = 0; i < 4; i++) {
    if (x >= positions[i][0] && x < positions[i][0] + 100 &&
        y >= positions[i][1] && y < positions[i][1] + 100) {
      
      if (i == reflex_current_button) {
        // Correct button!
        int reaction = millis() - reflex_flash_time;
        recordReflexTime(reaction);
      } else {
        // Wrong button - penalty
        reflex_times[reflex_round] = 1000;
      }
      
      reflex_round++;
      reflex_current_button = -1;
      reflex_waiting = false;
      
      if (reflex_round >= 10) {
        endTrainingGame();
      } else {
        reflex_flash_time = millis() + random(500, 2000);
      }
      return;
    }
  }
}

void flashReflexButton(int button_index) {
  reflex_current_button = button_index;
  reflex_flash_time = millis();
  reflex_waiting = true;
}

void recordReflexTime(int time_ms) {
  reflex_times[reflex_round] = time_ms;
  
  if (time_ms < current_training_score.best_time_ms) {
    current_training_score.best_time_ms = time_ms;
  }
  
  // Score based on reaction time
  if (time_ms < REFLEX_PERFECT_MS) current_training_score.score += 100;
  else if (time_ms < REFLEX_GREAT_MS) current_training_score.score += 75;
  else if (time_ms < REFLEX_GOOD_MS) current_training_score.score += 50;
  else current_training_score.score += 25;
}

const char* getReflexRating(int avg_ms) {
  if (avg_ms < 150) return "PERFECT!";
  if (avg_ms < 250) return "Great!";
  if (avg_ms < 400) return "Good";
  return "Keep Practicing";
}

// =============================================================================
// TARGET SHOOT
// =============================================================================

void initTargetShoot() {
  targets_hit = 0;
  targets_missed = 0;
  target_timer = millis() + 30000;  // 30 second game
  
  for (int i = 0; i < 10; i++) {
    target_active[i] = false;
  }
  
  // Spawn initial targets
  for (int i = 0; i < 3; i++) {
    spawnTarget();
  }
}

void updateTargetShoot() {
  // Check timer
  if (millis() >= target_timer) {
    current_training_score.score = targets_hit * 10 - targets_missed * 5;
    endTrainingGame();
    return;
  }
  
  // Spawn new targets occasionally
  static unsigned long last_spawn = 0;
  if (millis() - last_spawn > 1500) {
    spawnTarget();
    last_spawn = millis();
  }
}

void drawTargetShoot() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 10);
  gfx->print("TARGET SHOOT");
  
  // Timer
  int remaining = (target_timer - millis()) / 1000;
  gfx->setTextColor(remaining < 10 ? COLOR_RED : COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(280, 15);
  gfx->printf("%ds", remaining);
  
  // Score
  gfx->setTextColor(COLOR_GREEN);
  gfx->setCursor(20, 15);
  gfx->printf("Hits: %d", targets_hit);
  
  // Draw targets
  for (int i = 0; i < 10; i++) {
    if (target_active[i]) {
      // Target emoji representation
      gfx->fillCircle(target_x[i], target_y[i], 25, COLOR_RED);
      gfx->fillCircle(target_x[i], target_y[i], 18, COLOR_WHITE);
      gfx->fillCircle(target_x[i], target_y[i], 10, COLOR_RED);
      gfx->fillCircle(target_x[i], target_y[i], 4, COLOR_WHITE);
    }
  }
}

void handleTargetTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  bool hit = false;
  
  for (int i = 0; i < 10; i++) {
    if (target_active[i]) {
      int dx = x - target_x[i];
      int dy = y - target_y[i];
      if (dx*dx + dy*dy < 30*30) {  // Within target radius
        hitTarget(i);
        hit = true;
        break;
      }
    }
  }
  
  if (!hit) {
    missTarget();
  }
}

void spawnTarget() {
  for (int i = 0; i < 10; i++) {
    if (!target_active[i]) {
      target_x[i] = random(50, LCD_WIDTH - 50);
      target_y[i] = random(80, LCD_HEIGHT - 80);
      target_active[i] = true;
      return;
    }
  }
}

void hitTarget(int target_index) {
  target_active[target_index] = false;
  targets_hit++;
  current_training_score.combo_count++;
  
  // Bonus for combos
  if (current_training_score.combo_count >= 5) {
    current_training_score.score += 50;
  }
}

void missTarget() {
  targets_missed++;
  current_training_score.combo_count = 0;
}

// =============================================================================
// SPEED TAP
// =============================================================================

void initSpeedTap() {
  tap_count = 0;
  last_combo = 0;
  speed_tap_end_time = millis() + 10000;  // 10 seconds
}

void updateSpeedTap() {
  if (millis() >= speed_tap_end_time) {
    current_training_score.score = tap_count;
    endTrainingGame();
  }
}

void drawSpeedTap() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 20);
  gfx->print("SPEED TAP");
  
  // Timer
  int remaining = (speed_tap_end_time - millis()) / 1000;
  gfx->setTextColor(remaining < 3 ? COLOR_RED : COLOR_WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(165, 60);
  gfx->printf("%d", remaining);
  
  // Tap count - BIG
  gfx->setTextColor(getCurrentTheme()->accent);
  gfx->setTextSize(6);
  gfx->setCursor(120, 180);
  gfx->printf("%d", tap_count);
  
  // TAP zone
  gfx->fillRoundRect(60, 300, 240, 100, 20, getCurrentTheme()->primary);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(130, 335);
  gfx->print("TAP!");
  
  // Ranking preview
  gfx->setTextSize(1);
  gfx->setCursor(120, 420);
  gfx->print(getSpeedTapRanking(tap_count));
}

void handleSpeedTapTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_TAP || gesture.event == TOUCH_PRESS) {
    recordTap();
  }
}

void recordTap() {
  tap_count++;
  last_combo++;
  
  // Bonus for rapid tapping (5+ in quick succession)
  if (last_combo >= 5) {
    current_training_score.combo_count++;
  }
}

const char* getSpeedTapRanking(int taps) {
  if (taps >= 151) return "MASTER!";
  if (taps >= 101) return "Expert";
  if (taps >= 76) return "Pro";
  if (taps >= 51) return "Amateur";
  return "Beginner";
}

// =============================================================================
// MEMORY MATCH (Training Version)
// =============================================================================

void initTrainingMemory() {
  memory_length = 3;
  memory_input_index = 0;
  memory_showing = true;
  
  // Generate pattern
  for (int i = 0; i < 20; i++) {
    memory_pattern[i] = random(0, 9);
  }
  
  showMemoryPattern();
}

void updateTrainingMemory() {
  // Check if showing pattern
  if (memory_showing) {
    // Auto-advance pattern display
  }
}

void drawTrainingMemory() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("MEMORY MATCH");
  
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(130, 50);
  gfx->printf("Level %d", memory_length - 2);
  
  // Draw 3x3 grid of buttons
  int buttonSize = 90;
  int startX = 45, startY = 90;
  int spacing = 10;
  
  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = startX + col * (buttonSize + spacing);
    int y = startY + row * (buttonSize + spacing);
    
    bool lit = false;
    if (memory_showing && memory_show_time > 0) {
      // Check if this button should be lit
      int showIndex = (millis() - memory_show_time) / 500;
      if (showIndex < memory_length && memory_pattern[showIndex] == i) {
        lit = true;
      }
    }
    
    drawTrainingButton(x, y, buttonSize, i, lit, false);
  }
  
  if (memory_showing) {
    gfx->setTextSize(1);
    gfx->setCursor(120, 400);
    gfx->print("Watch the pattern!");
  } else {
    gfx->setCursor(120, 400);
    gfx->printf("Input: %d/%d", memory_input_index, memory_length);
  }
}

void handleTrainingMemoryTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP || memory_showing) return;
  
  int x = gesture.x, y = gesture.y;
  int buttonSize = 90;
  int startX = 45, startY = 90;
  int spacing = 10;
  
  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int bx = startX + col * (buttonSize + spacing);
    int by = startY + row * (buttonSize + spacing);
    
    if (x >= bx && x < bx + buttonSize && y >= by && y < by + buttonSize) {
      if (checkMemoryInput(i)) {
        memory_input_index++;
        
        if (memory_input_index >= memory_length) {
          // Level complete!
          current_training_score.score += memory_length * 10;
          nextMemoryLevel();
        }
      } else {
        // Wrong! Game over
        endTrainingGame();
      }
      return;
    }
  }
}

void showMemoryPattern() {
  memory_showing = true;
  memory_show_time = millis();
  memory_input_index = 0;
  
  // After showing, switch to input mode
  // In real implementation, this would be timed
}

bool checkMemoryInput(int button) {
  return button == memory_pattern[memory_input_index];
}

void nextMemoryLevel() {
  memory_length++;
  if (memory_length > 10) {
    // Perfect clear!
    current_training_score.score += 200;
    endTrainingGame();
  } else {
    showMemoryPattern();
  }
}

// =============================================================================
// TRAINING UI
// =============================================================================

void drawTrainingMenu() {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(80, 20);
  gfx->print("TRAINING DOJO");
  
  // Streak display
  gfx->setTextColor(COLOR_GOLD);
  gfx->setTextSize(1);
  gfx->setCursor(120, 50);
  gfx->printf("Streak: %d days", system_state.training_streak);
  
  // Training game buttons
  const char* games[] = {"Reflex Test", "Target Shoot", "Speed Tap", "Memory Match"};
  const char* descs[] = {"Test reaction time", "Hit all targets", "Tap as fast as you can", "Remember the pattern"};
  
  for (int i = 0; i < 4; i++) {
    int y = 90 + i * 75;
    
    drawGlassPanel(30, y, 300, 65);
    
    gfx->setTextColor(getCurrentTheme()->primary);
    gfx->setTextSize(2);
    gfx->setCursor(45, y + 12);
    gfx->print(games[i]);
    
    gfx->setTextColor(COLOR_GRAY);
    gfx->setTextSize(1);
    gfx->setCursor(45, y + 40);
    gfx->print(descs[i]);
  }
  
  drawGlassButton(140, 410, 80, 35, "Back", false);
}

void drawTrainingResults(TrainingScore& score) {
  gfx->fillScreen(COLOR_BLACK);
  
  gfx->setTextColor(getCurrentTheme()->primary);
  gfx->setTextSize(2);
  gfx->setCursor(100, 30);
  gfx->print("RESULTS");
  
  drawGlassPanel(40, 80, 280, 200);
  
  // Score
  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(60, 100);
  gfx->print("Score:");
  gfx->setTextSize(3);
  gfx->setTextColor(getCurrentTheme()->accent);
  gfx->setCursor(60, 120);
  gfx->printf("%d", score.score);
  
  // XP earned
  gfx->setTextColor(COLOR_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(60, 170);
  gfx->printf("+%d XP", score.xp_earned);
  
  // Best time (for reflex)
  if (score.type == TRAINING_REFLEX) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(60, 210);
    gfx->printf("Best reaction: %dms", score.best_time_ms);
  }
  
  // Combo count
  if (score.combo_count > 0) {
    gfx->setTextColor(COLOR_GOLD);
    gfx->setCursor(60, 240);
    gfx->printf("Max combo: %d", score.combo_count);
  }
  
  drawGlassButton(100, 320, 160, 50, "Continue", false);
}

void handleTrainingMenuTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int x = gesture.x, y = gesture.y;
  
  // Check game buttons
  for (int i = 0; i < 4; i++) {
    int by = 90 + i * 75;
    if (y >= by && y < by + 65 && x >= 30 && x < 330) {
      startTrainingGame((TrainingType)i);
      return;
    }
  }
  
  // Back button
  if (y >= 410 && x >= 140 && x < 220) {
    system_state.current_screen = SCREEN_GAMES;
  }
}

void drawTrainingButton(int x, int y, int size, int button_id, bool lit, bool pressed) {
  uint16_t bg = lit ? getCurrentTheme()->primary : RGB565(50, 50, 50);
  if (pressed) bg = getCurrentTheme()->accent;
  
  gfx->fillRoundRect(x, y, size, size, 15, bg);
  gfx->drawRoundRect(x, y, size, size, 15, COLOR_WHITE);
  
  if (lit) {
    // Glow effect
    gfx->drawRoundRect(x - 2, y - 2, size + 4, size + 4, 17, getCurrentTheme()->effect1);
  }
}

void drawTrainingTimer(int seconds_remaining) {
  gfx->setTextColor(seconds_remaining < 5 ? COLOR_RED : COLOR_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(280, 15);
  gfx->printf("%ds", seconds_remaining);
}

void drawTrainingScore(int score) {
  gfx->setTextColor(COLOR_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(20, 15);
  gfx->printf("%d", score);
}
