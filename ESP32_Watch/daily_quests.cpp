/*
 * daily_quests.cpp - Daily Quest System Implementation
 */

#include "daily_quests.h"
#include "config.h"
#include "display.h"
#include "themes.h"
#include "navigation.h"
#include "steps_tracker.h"
#include "gacha.h"
#include <Preferences.h>

extern Arduino_SH8601 *gfx;
extern SystemState system_state;

DailyQuestsData quest_data;
Preferences questPrefs;

const char* QUEST_NAMES[] = {
  "Daily Steps",
  "Active Time",
  "Play Games",
  "Training",
  "Boss Battle",
  "Distance Goal"
};

const char* QUEST_DESCRIPTIONS[] = {
  "Walk %d steps",
  "Be active %d min",
  "Play %d games",
  "Train %d times",
  "Defeat %d bosses",
  "Walk %d km"
};

void initDailyQuests() {
  loadDailyQuestsData();
  checkDailyReset();
  Serial.println("[Quests] Daily quests initialized");
}

void drawDailyQuestsScreen() {
  gfx->fillScreen(RGB565(8, 8, 12));
  ThemeColors* theme = getCurrentTheme();
  
  // Header
  gfx->fillRoundRect(0, 0, LCD_WIDTH, 55, 0, RGB565(16, 18, 24));
  gfx->drawFastHLine(0, 55, LCD_WIDTH, theme->primary);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->setCursor(LCD_WIDTH/2 - 60, 18);
  gfx->print("Daily Quests");
  
  // Streak banner
  gfx->fillRoundRect(10, 65, 340, 35, 10, RGB565(40, 25, 60));
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GOLD);
  gfx->setCursor(20, 75);
  gfx->printf("Streak: %d days", quest_data.streak_days);
  gfx->setCursor(200, 75);
  gfx->printf("Completed: %d", quest_data.total_completed);
  
  // Quest cards
  int questY = 115;
  for (int i = 0; i < 3; i++) {
    Quest& q = quest_data.daily_quests[i];
    
    // Quest card background
    uint16_t bgColor = q.completed ? RGB565(25, 50, 25) : RGB565(25, 27, 35);
    uint16_t borderColor = q.completed ? COLOR_GREEN : theme->primary;
    
    gfx->fillRoundRect(10, questY, 340, 85, 12, bgColor);
    gfx->drawRoundRect(10, questY, 340, 85, 12, borderColor);
    
    // Quest icon (difficulty stars)
    gfx->setTextColor(COLOR_GOLD);
    gfx->setTextSize(1);
    gfx->setCursor(20, questY + 10);
    for (int s = 0; s <= (int)q.difficulty; s++) {
      gfx->print("*");
    }
    
    // Quest name
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(20, questY + 25);
    gfx->print(q.name);
    
    // Quest description
    gfx->setTextSize(1);
    gfx->setTextColor(RGB565(180, 180, 190));
    gfx->setCursor(20, questY + 48);
    char desc[50];
    sprintf(desc, q.description, q.target);
    gfx->print(desc);
    
    // Progress bar
    int barX = 20;
    int barY = questY + 63;
    int barW = 200;
    int barH = 12;
    
    gfx->fillRoundRect(barX, barY, barW, barH, 6, RGB565(40, 40, 50));
    
    float progress = (float)q.progress / q.target;
    if (progress > 1.0) progress = 1.0;
    int fillW = barW * progress;
    
    uint16_t progressColor = q.completed ? COLOR_GREEN : theme->accent;
    gfx->fillRoundRect(barX, barY, fillW, barH, 6, progressColor);
    
    // Progress text
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(barX + barW + 10, barY + 2);
    gfx->printf("%d/%d", q.progress, q.target);
    
    // Rewards
    gfx->setTextColor(COLOR_GOLD);
    gfx->setCursor(240, questY + 25);
    gfx->printf("+%d", q.reward_gold);
    gfx->setTextSize(1);
    gfx->setCursor(285, questY + 28);
    gfx->print("gold");
    
    gfx->setTextColor(COLOR_CYAN);
    gfx->setCursor(240, questY + 43);
    gfx->printf("+%d XP", q.reward_xp);
    
    if (q.reward_gems > 0) {
      gfx->setTextColor(COLOR_PURPLE);
      gfx->setCursor(240, questY + 58);
      gfx->printf("+%d gems", q.reward_gems);
    }
    
    // Claim button
    if (q.completed) {
      gfx->fillRoundRect(270, questY + 8, 70, 30, 8, COLOR_GREEN);
      gfx->setTextSize(1);
      gfx->setTextColor(COLOR_BLACK);
      gfx->setCursor(280, questY + 18);
      gfx->print("CLAIM!");
    }
    
    questY += 95;
  }
  
  // Info footer
  gfx->setTextSize(1);
  gfx->setTextColor(RGB565(120, 120, 130));
  gfx->setCursor(60, 430);
  gfx->print("Resets daily at midnight");
  
  drawSwipeIndicator();
}

void generateNewDailyQuests() {
  Serial.println("[Quests] Generating new daily quests");
  
  // Reset all quests
  for (int i = 0; i < 3; i++) {
    quest_data.daily_quests[i].completed = false;
    quest_data.daily_quests[i].progress = 0;
  }
  
  // Generate 3 random quests (one easy, one medium, one hard)
  QuestDifficulty difficulties[] = {QUEST_EASY, QUEST_MEDIUM, QUEST_HARD};
  
  for (int i = 0; i < 3; i++) {
    Quest& q = quest_data.daily_quests[i];
    q.type = (QuestType)random(0, 6);  // 6 quest types (0-5)
    q.difficulty = difficulties[i];
    q.name = QUEST_NAMES[(int)q.type];
    q.description = QUEST_DESCRIPTIONS[(int)q.type];
    
    // Set targets based on difficulty
    switch(q.difficulty) {
      case QUEST_EASY:
        q.target = (q.type == QUEST_STEPS) ? 2000 : 
                   (q.type == QUEST_DISTANCE) ? 2 : 3;
        q.reward_gold = 100;
        q.reward_xp = 50;
        q.reward_gems = 0;
        break;
      case QUEST_MEDIUM:
        q.target = (q.type == QUEST_STEPS) ? 5000 : 
                   (q.type == QUEST_DISTANCE) ? 5 : 5;
        q.reward_gold = 250;
        q.reward_xp = 100;
        q.reward_gems = 1;
        break;
      case QUEST_HARD:
        q.target = (q.type == QUEST_STEPS) ? 10000 : 
                   (q.type == QUEST_DISTANCE) ? 10 : 10;
        q.reward_gold = 500;
        q.reward_xp = 200;
        q.reward_gems = 3;
        break;
    }
    
    q.progress = 0;
    q.completed = false;
  }
  
  saveDailyQuestsData();
}

void updateQuestProgress(QuestType type, uint32_t amount) {
  for (int i = 0; i < 3; i++) {
    Quest& q = quest_data.daily_quests[i];
    if (q.type == type && !q.completed) {
      q.progress += amount;
      if (q.progress >= q.target) {
        q.progress = q.target;
        q.completed = true;
        Serial.printf("[Quests] Quest completed: %s\n", q.name);
      }
      saveDailyQuestsData();
    }
  }
}

void claimQuestReward(int questIndex) {
  if (questIndex < 0 || questIndex >= 3) return;
  
  Quest& q = quest_data.daily_quests[questIndex];
  if (!q.completed) return;
  
  // Give rewards (only gems for now - gold/XP system doesn't exist globally yet)
  int total_gems = q.reward_gems + (q.reward_gold / 100) + (q.reward_xp / 50);
  system_state.player_gems += total_gems;
  
  Serial.printf("[Quests] Claimed rewards: %d gems total\n", total_gems);
  
  // Remove quest (mark as claimed)
  q.reward_gold = 0;
  q.reward_xp = 0;
  q.reward_gems = 0;
  
  quest_data.total_completed++;
  saveDailyQuestsData();
  
  drawDailyQuestsScreen();
}

void checkDailyReset() {
  WatchTime current_time = getCurrentTime();
  
  // Check if it's a new day (midnight passed)
  if (current_time.hour == 0 && quest_data.current_day != current_time.day) {
    Serial.println("[Quests] Daily reset triggered");
    
    // Check if all quests were completed yesterday
    bool all_completed = true;
    for (int i = 0; i < 3; i++) {
      if (!quest_data.daily_quests[i].completed) {
        all_completed = false;
        break;
      }
    }
    
    // Update streak
    if (all_completed) {
      quest_data.streak_days++;
    } else {
      quest_data.streak_days = 0;
    }
    
    // Generate new quests
    generateNewDailyQuests();
    quest_data.current_day = current_time.day;
    quest_data.last_reset = millis();
    saveDailyQuestsData();
  }
}

void saveDailyQuestsData() {
  questPrefs.begin("quests", false);
  questPrefs.putUChar("day", quest_data.current_day);
  questPrefs.putUInt("total", quest_data.total_completed);
  questPrefs.putUInt("streak", quest_data.streak_days);
  
  for (int i = 0; i < 3; i++) {
    char key[20];
    sprintf(key, "q%d_type", i);
    questPrefs.putUChar(key, (uint8_t)quest_data.daily_quests[i].type);
    sprintf(key, "q%d_prog", i);
    questPrefs.putUInt(key, quest_data.daily_quests[i].progress);
    sprintf(key, "q%d_comp", i);
    questPrefs.putBool(key, quest_data.daily_quests[i].completed);
  }
  questPrefs.end();
}

void loadDailyQuestsData() {
  questPrefs.begin("quests", true);
  quest_data.current_day = questPrefs.getUChar("day", 0);
  quest_data.total_completed = questPrefs.getUInt("total", 0);
  quest_data.streak_days = questPrefs.getUInt("streak", 0);
  questPrefs.end();
  
  // If no saved quests, generate new ones
  WatchTime current_time = getCurrentTime();
  if (quest_data.current_day != current_time.day) {
    generateNewDailyQuests();
    quest_data.current_day = current_time.day;
  }
}

void handleDailyQuestsTouch(TouchGesture& gesture) {
  if (gesture.event == TOUCH_SWIPE_UP) {
    returnToAppGrid();
    return;
  }
  
  if (gesture.event != TOUCH_TAP) return;
  
  // Check claim buttons
  int questY = 115;
  for (int i = 0; i < 3; i++) {
    // Claim button area: x: 270-340, y: questY+8 to questY+38
    if (gesture.x >= 270 && gesture.x <= 340 &&
        gesture.y >= questY + 8 && gesture.y <= questY + 38) {
      claimQuestReward(i);
      return;
    }
    questY += 95;
  }
}
