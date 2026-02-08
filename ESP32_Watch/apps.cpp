/**
 * Apps Module Implementation
 */

#include "apps.h"
#include "ui_manager.h"
#include "themes.h"

Quest dailyQuests[5];
int numQuests = 0;

static bool musicPlaying = false;
static int currentTrack = 0;
static const char* trackNames[] = {"Track 1", "Track 2", "Track 3", "Track 4", "Track 5"};

// ═══════════════════════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════
void initApps() {
  generateDailyQuests();
  Serial.println("[OK] Apps initialized");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MUSIC SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
static lv_obj_t* trackLabel = nullptr;
static lv_obj_t* playBtn = nullptr;

lv_obj_t* createMusicScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "Music");
  
  // Album art placeholder (circular)
  lv_obj_t* art = lv_obj_create(scr);
  lv_obj_set_size(art, 180, 180);
  lv_obj_set_style_radius(art, 90, 0);
  lv_obj_set_style_bg_color(art, lv_color_hex(colors.primary), 0);
  lv_obj_align(art, LV_ALIGN_CENTER, 0, -40);
  
  lv_obj_t* musicIcon = lv_label_create(art);
  lv_obj_set_style_text_font(musicIcon, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(musicIcon, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(musicIcon, LV_SYMBOL_AUDIO);
  lv_obj_center(musicIcon);
  
  // Track name
  trackLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(trackLabel, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(trackLabel, lv_color_hex(colors.text), 0);
  lv_label_set_text(trackLabel, trackNames[currentTrack]);
  lv_obj_align(trackLabel, LV_ALIGN_CENTER, 0, 80);
  
  // Controls
  int btnY = 340;
  
  // Previous
  lv_obj_t* prevBtn = lv_btn_create(scr);
  lv_obj_set_size(prevBtn, 70, 50);
  lv_obj_set_pos(prevBtn, 40, btnY);
  lv_obj_set_style_bg_color(prevBtn, lv_color_hex(colors.secondary), 0);
  lv_obj_add_event_cb(prevBtn, [](lv_event_t* e) { prevTrack(); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* prevLbl = lv_label_create(prevBtn);
  lv_label_set_text(prevLbl, LV_SYMBOL_PREV);
  lv_obj_center(prevLbl);
  
  // Play/Pause
  playBtn = lv_btn_create(scr);
  lv_obj_set_size(playBtn, 90, 60);
  lv_obj_set_pos(playBtn, (LCD_WIDTH - 90) / 2, btnY - 5);
  lv_obj_set_style_bg_color(playBtn, lv_color_hex(colors.primary), 0);
  lv_obj_set_style_radius(playBtn, 30, 0);
  lv_obj_add_event_cb(playBtn, [](lv_event_t* e) {
    if (musicPlaying) pauseMusic();
    else playMusic();
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* playLbl = lv_label_create(playBtn);
  lv_label_set_text(playLbl, LV_SYMBOL_PLAY);
  lv_obj_center(playLbl);
  
  // Next
  lv_obj_t* nextBtn = lv_btn_create(scr);
  lv_obj_set_size(nextBtn, 70, 50);
  lv_obj_set_pos(nextBtn, LCD_WIDTH - 110, btnY);
  lv_obj_set_style_bg_color(nextBtn, lv_color_hex(colors.secondary), 0);
  lv_obj_add_event_cb(nextBtn, [](lv_event_t* e) { nextTrack(); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* nextLbl = lv_label_create(nextBtn);
  lv_label_set_text(nextLbl, LV_SYMBOL_NEXT);
  lv_obj_center(nextLbl);
  
  // SD card note
  lv_obj_t* note = lv_label_create(scr);
  lv_obj_set_style_text_font(note, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(note, lv_color_hex(colors.secondary), 0);
  lv_label_set_text(note, hasSD ? "SD Card: Ready" : "No SD Card");
  lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -15);
  
  return scr;
}

void playMusic() {
  musicPlaying = true;
  if (playBtn) {
    lv_obj_t* lbl = lv_obj_get_child(playBtn, 0);
    if (lbl) lv_label_set_text(lbl, LV_SYMBOL_PAUSE);
  }
}

void pauseMusic() {
  musicPlaying = false;
  if (playBtn) {
    lv_obj_t* lbl = lv_obj_get_child(playBtn, 0);
    if (lbl) lv_label_set_text(lbl, LV_SYMBOL_PLAY);
  }
}

void nextTrack() {
  currentTrack = (currentTrack + 1) % 5;
  if (trackLabel) lv_label_set_text(trackLabel, trackNames[currentTrack]);
}

void prevTrack() {
  currentTrack = (currentTrack + 4) % 5;
  if (trackLabel) lv_label_set_text(trackLabel, trackNames[currentTrack]);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SETTINGS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
lv_obj_t* createSettingsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "Settings");
  
  int yPos = 60;
  
  // Brightness
  lv_obj_t* brightLbl = lv_label_create(scr);
  lv_obj_set_style_text_color(brightLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text(brightLbl, "Brightness");
  lv_obj_set_pos(brightLbl, 20, yPos);
  
  lv_obj_t* brightSlider = lv_slider_create(scr);
  lv_obj_set_size(brightSlider, LCD_WIDTH - 40, 30);
  lv_obj_set_pos(brightSlider, 20, yPos + 30);
  lv_slider_set_range(brightSlider, 50, 255);
  lv_slider_set_value(brightSlider, watch.brightness, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(brightSlider, lv_color_hex(colors.primary), LV_PART_INDICATOR);
  lv_obj_add_event_cb(brightSlider, [](lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    watch.brightness = lv_slider_get_value(slider);
    // gfx->setBrightness(watch.brightness);
  }, LV_EVENT_VALUE_CHANGED, NULL);
  
  yPos += 80;
  
  // Step Goal
  lv_obj_t* goalLbl = lv_label_create(scr);
  lv_obj_set_style_text_color(goalLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text(goalLbl, "Daily Step Goal");
  lv_obj_set_pos(goalLbl, 20, yPos);
  
  lv_obj_t* goalVal = lv_label_create(scr);
  lv_obj_set_style_text_color(goalVal, lv_color_hex(colors.accent), 0);
  lv_label_set_text_fmt(goalVal, "%lu steps", watch.stepGoal);
  lv_obj_set_pos(goalVal, 20, yPos + 25);
  
  lv_obj_t* incBtn = lv_btn_create(scr);
  lv_obj_set_size(incBtn, 60, 40);
  lv_obj_set_pos(incBtn, LCD_WIDTH - 140, yPos + 15);
  lv_obj_set_style_bg_color(incBtn, lv_color_hex(colors.primary), 0);
  lv_obj_add_event_cb(incBtn, [](lv_event_t* e) {
    watch.stepGoal += 1000;
    if (watch.stepGoal > 50000) watch.stepGoal = 50000;
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* incLbl = lv_label_create(incBtn);
  lv_label_set_text(incLbl, "+");
  lv_obj_center(incLbl);
  
  lv_obj_t* decBtn = lv_btn_create(scr);
  lv_obj_set_size(decBtn, 60, 40);
  lv_obj_set_pos(decBtn, LCD_WIDTH - 70, yPos + 15);
  lv_obj_set_style_bg_color(decBtn, lv_color_hex(colors.secondary), 0);
  lv_obj_add_event_cb(decBtn, [](lv_event_t* e) {
    if (watch.stepGoal > 1000) watch.stepGoal -= 1000;
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t* decLbl = lv_label_create(decBtn);
  lv_label_set_text(decLbl, "-");
  lv_obj_center(decLbl);
  
  yPos += 80;
  
  // WiFi Status
  lv_obj_t* wifiLbl = lv_label_create(scr);
  lv_obj_set_style_text_color(wifiLbl, lv_color_hex(colors.text), 0);
  lv_label_set_text(wifiLbl, "WiFi Status");
  lv_obj_set_pos(wifiLbl, 20, yPos);
  
  lv_obj_t* wifiStatus = lv_label_create(scr);
  lv_obj_set_style_text_color(wifiStatus, lv_color_hex(watch.wifiConnected ? 0x4CAF50 : 0xFF5722), 0);
  lv_label_set_text(wifiStatus, watch.wifiConnected ? "Connected" : "Not Connected");
  lv_obj_set_pos(wifiStatus, 20, yPos + 25);
  
  yPos += 80;
  
  // Save button
  lv_obj_t* saveBtn = lv_btn_create(scr);
  lv_obj_set_size(saveBtn, LCD_WIDTH - 40, 50);
  lv_obj_set_pos(saveBtn, 20, yPos);
  lv_obj_set_style_bg_color(saveBtn, lv_color_hex(colors.primary), 0);
  lv_obj_add_event_cb(saveBtn, [](lv_event_t* e) {
    saveUserData();
  }, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* saveLbl = lv_label_create(saveBtn);
  lv_label_set_text(saveLbl, "Save Settings");
  lv_obj_center(saveLbl);
  
  return scr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  QUESTS SCREEN
// ═══════════════════════════════════════════════════════════════════════════════
void generateDailyQuests() {
  const char* titles[] = {"Morning Walk", "Power Up!", "Step Master", "Explorer", "Champion"};
  const char* descs[] = {"Walk 2000 steps", "Walk 5000 steps", "Walk 8000 steps", "Walk 10000 steps", "Walk 15000 steps"};
  int targets[] = {2000, 5000, 8000, 10000, 15000};
  int rewards[] = {50, 100, 150, 200, 300};
  
  numQuests = 5;
  for (int i = 0; i < numQuests; i++) {
    dailyQuests[i].title = titles[i];
    dailyQuests[i].description = descs[i];
    dailyQuests[i].target = targets[i];
    dailyQuests[i].progress = watch.steps;
    dailyQuests[i].reward = rewards[i];
    dailyQuests[i].completed = (watch.steps >= targets[i]);
    dailyQuests[i].daily = true;
  }
}

void updateQuestProgress() {
  for (int i = 0; i < numQuests; i++) {
    dailyQuests[i].progress = watch.steps;
    if (watch.steps >= dailyQuests[i].target && !dailyQuests[i].completed) {
      dailyQuests[i].completed = true;
      userData.questsCompleted++;
    }
  }
}

lv_obj_t* createQuestsScreen() {
  lv_obj_t* scr = lv_obj_create(NULL);
  ThemeColors colors = getThemeColors(watch.theme);
  lv_obj_set_style_bg_color(scr, lv_color_hex(colors.background), 0);
  
  createTitleBar(scr, "Daily Quests");
  
  updateQuestProgress();
  
  int yPos = 60;
  for (int i = 0; i < numQuests && i < 4; i++) {
    Quest& q = dailyQuests[i];
    
    lv_obj_t* card = lv_obj_create(scr);
    lv_obj_set_size(card, LCD_WIDTH - 20, 85);
    lv_obj_set_pos(card, 10, yPos);
    lv_obj_set_style_bg_color(card, lv_color_hex(q.completed ? 0x2E7D32 : 0x333333), 0);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    
    lv_obj_t* title = lv_label_create(card);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(title, q.title);
    lv_obj_set_pos(title, 10, 5);
    
    lv_obj_t* desc = lv_label_create(card);
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(desc, lv_color_hex(0xAAAAAA), 0);
    lv_label_set_text(desc, q.description);
    lv_obj_set_pos(desc, 10, 25);
    
    // Progress bar
    lv_obj_t* bar = lv_bar_create(card);
    lv_obj_set_size(bar, LCD_WIDTH - 60, 15);
    lv_obj_set_pos(bar, 10, 50);
    lv_bar_set_range(bar, 0, q.target);
    lv_bar_set_value(bar, q.progress > q.target ? q.target : q.progress, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_hex(colors.primary), LV_PART_INDICATOR);
    
    // Reward
    lv_obj_t* reward = lv_label_create(card);
    lv_obj_set_style_text_font(reward, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(reward, lv_color_hex(colors.accent), 0);
    lv_label_set_text_fmt(reward, "+%d XP", q.reward);
    lv_obj_align(reward, LV_ALIGN_TOP_RIGHT, -10, 5);
    
    yPos += 95;
  }
  
  return scr;
}
