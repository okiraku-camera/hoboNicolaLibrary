/**
  askb_xiao_hid2.ino Main sketch of "ASKeyboard (askb_xiao_rev01)" using hoboNicola Library 1.6.3.
  Copyright (c) 2022 Takeshi Higasa

  This file is part of "ASKeyboard (askb_xiao_rev01)".

  "ASKeyboard (askb_xiao_rev01)" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "ASKeyboard (askb_xiao_rev01)" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "ASKeyboard (askb_xiao_rev01)".  If not, see <http://www.gnu.org/licenses/>.

  SAMD21 version 1.0.0  August. 20, 2022.
*/
#include "Adafruit_TinyUSB.h"
#include "hobo_sleep.h"
#include "askb_xiao.h"
#include "hobo_nicola.h"

// ライブラリで使っているマクロと置き換える
#if defined(LED1)
#undef LED1
#endif
#if defined(LED2)
#undef LED2
#endif

#define LED1  0
#define LED2  1
#define LED3  2

static const uint16_t fn_keys[] PROGMEM = {
  HID_S     | WITH_R_CTRL , FN_SETUP_MODE,
  HID_ESCAPE | WITH_R_CTRL,  FN_SYSTEM_SLEEP,   // Ctrl + App + Esc 
  0, 0
};

class askb_hobo_nicola : public HoboNicola {
public:
  askb_hobo_nicola() {}
 	const uint16_t* fn_keys_table() { return fn_keys; }
  void nicola_led(uint8_t on) { digitalWrite(LED1, on ? 0 : 1); }
  void toggle_nicola_led()  { digitalWrite(LED1, digitalRead(LED1) ^ 1); }
  void scrlock_led(uint8_t on) { digitalWrite(LED3, on ? 0 : 1); }  // ScrollLock LED
  void capslock_led(uint8_t on)  { digitalWrite(LED2, on ? 0 : 1); }  // CapsLock LED

	const uint8_t fn_key() { return HID_X_FN1; }
  const bool is_fn_key_solid() { return true; }	// solidなFnキー
	void fn_event(uint8_t key, bool pressed)	{ table_change(key, pressed);  } 
	const bool has_fn_keytable() { return true; }

};

askb_hobo_nicola hobo_nicola;

void askb_key_event(uint8_t key, bool keyon) {
  if (is_usb_suspended()) {
#if !defined(ARDUINO_ARCH_RP2040)
    usb_wakeup();
    clear_data();
    delay(400);
#endif
    return;
  }
   hobo_nicola.key_event(key, keyon);
}

void led_off() {
  digitalWrite(LED1, 1);
  digitalWrite(LED2, 1);
  digitalWrite(LED3, 1);
  HoboNicola::last_hid_led_state = 0;
}

#include "hobo_settings.h"
void setup() {
  init_askb_xiao();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  led_off();
// samd21では、eepromの役割としてFlashStorageライブラリを使っているでファーム更新でそれまでの設定が消えてしまう。
// そのため好ましい設定をここでやってしまう。
  uint32_t tmp_set = global_setting;
  uint32_t save_set = tmp_set;
  tmp_set |= SCR_AS_NICOLA; // clear SCR_AS_NICOLA
  tmp_set |= KANA_TO_NICOLA_ON;
  tmp_set |= EISU_TO_NICOLA_OFF;
  tmp_set |= HENKAN_TO_SPC; // 親指右は変換キーに割り当てているが、空白に変更する。
  tmp_set |= SPC_TO_RIGHT;  // 空白キーは右親指キーとして機能させる。
  tmp_set &= ~SPC_TO_LEFT;
  if (save_set != tmp_set) {
    pSettings->save(tmp_set);
    global_setting = tmp_set;
  }
  HoboNicola::init_hobo_nicola(&hobo_nicola);
  delay(10);
}
static const uint32_t hobo_sleep_ms = 1000;
// 30 minutes経過したらキーボードによる復帰はやらない。
static const uint32_t no_resume_ms = 1800L * 1000L; // 30minutes.
static const uint32_t no_resume_count = (no_resume_ms / hobo_sleep_ms);
uint32_t sleep_counter = 0;
bool deep_suspended = false;
bool suspended = false;

void loop() {
  if (is_usb_suspended() ) {
    led_off();
		table_change(HID_X_FN1, false); // Fnキーリリース状態に戻す。
    enter_sleep(hobo_sleep_ms);
    if (sleep_counter > no_resume_count)
      deep_suspended = true;
    else if (!deep_suspended) {
      sleep_counter++;
      matrix_scan();  // check resume request.
    }
  } else { 
    if (deep_suspended || suspended) {
      delay(100);
      sleep_counter = 0;
      suspended = deep_suspended = false;  
    }
    hobo_nicola.idle();
    matrix_scan();
    enter_sleep(0); 
    enter_sleep(0); // スキャン間隔を2msec程度にしてみる。1mAほど減る。
  }
}
