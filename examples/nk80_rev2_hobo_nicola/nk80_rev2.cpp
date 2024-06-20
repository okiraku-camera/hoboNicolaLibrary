/**
 * 
  nk80_rev2.cpp   NK80 (rev.2) PCB interfaces of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2024 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
  
  	hoboNicola 1.7.4.		Jan. 4. 2024.
*/

#include <Arduino.h>
#include "nk80_rev2.h"
#include "hid_keycode.h"

// gpio usage.
static const uint8_t _CAPS_LED = 24;
static const uint8_t _BG_LED = 0;

static const uint8_t ROWS[] = {29, 28, 27, 26, 25, 7};
static const uint8_t COLS[] = {16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15};
static const uint8_t SW_COUNT = sizeof(ROWS) * sizeof(COLS);

typedef struct {
	uint8_t flag;
	uint8_t code;
} queue_entry_t;

static queue_t key_event_queue;

void send_key_event(uint8_t code, uint8_t flag) {
	queue_entry_t key;
	key.flag = flag;
	key.code = code;
	if (!queue_try_add(&key_event_queue, &key))
		watchdog_reboot(0, 0, 10);		// キューがフルのときリブートしてみる。
}

static const int STATE_COUNT = sizeof(ROWS);
static uint32_t key_state[STATE_COUNT] = { 0 }; // 現在確定状態
static uint32_t last_key_state[STATE_COUNT] = { 0 };  // 前回確定状態

void matrix_scan() {
  uint32_t tmp_state[STATE_COUNT];
	for(uint8_t row = 0; row < sizeof(ROWS); row++) {	// scan all rows.
		gpio_put(ROWS[row], 0);
		delayMicroseconds(5);
		uint32_t cols = gpio_get_all();
		tmp_state[row] = ~(((cols & 0x00ff0000) >> 16) | (cols & 0x0000ff00));
		gpio_put(ROWS[row], 1);
	}

	if (memcmp(tmp_state, key_state, sizeof(key_state))) {
		memcpy(key_state, tmp_state, sizeof(key_state));
		return;
	}

// 変化のあったキーを抽出する。
	uint8_t code = 1;
	for(uint8_t i = 0; i < sizeof(ROWS); i++) {
		uint32_t pressed = tmp_state[i];
		uint32_t change = pressed ^ last_key_state[i];
		if (change)	{
			uint32_t mask = 1;
			for(uint8_t k = 0; k < sizeof(COLS); k++, mask <<= 1) {
				if (change & mask) {
					send_key_event(code + k, (pressed & mask) ? true : false);
					//Serial.printf("%d (%d)\n", code + k, (pressed & mask) ? true : false);
				}
			}
			last_key_state[i] = pressed;			
		}
		code += sizeof(COLS);
	}	
}

#include "nk_rp_pwmled.h"

void CAPSLED(uint8_t on) {
	pinMode(_CAPS_LED, OUTPUT);
	digitalWrite(_CAPS_LED, on ? 1 : 0);
}

void BGLED(uint8_t on) {
#if 1	
	on ? rp_start_pwm() : rp_stop_pwm();
#else
	pinMode(_BG_LED, OUTPUT);
	digitalWrite(_BG_LED, on ? 1 : 0);
#endif
}

void init_nk80() {
	pinMode(_BG_LED, OUTPUT);
	digitalWrite(_BG_LED, 0);
	pinMode(_CAPS_LED, OUTPUT);
	digitalWrite(_CAPS_LED, 0);

	queue_init(&key_event_queue, sizeof(queue_entry_t), 16);

	for(uint8_t i = 0; i < sizeof(ROWS); i++) {
		gpio_init(ROWS[i]);
		gpio_set_drive_strength(ROWS[i], GPIO_DRIVE_STRENGTH_2MA);
		gpio_set_dir(ROWS[i], true);
		gpio_put(ROWS[i], 1);
	}		

	for(uint8_t i = 0; i < sizeof(COLS); i++) {
		gpio_init(COLS[i]);
		gpio_set_dir(COLS[i], false);
		gpio_pull_up(COLS[i]);
	}
	rp_pwm_init(_BG_LED);
	BGLED(0);
	CAPSLED(0);

	memset(key_state, 0, sizeof(key_state));
	memset(last_key_state, 0, sizeof(last_key_state));

//	Serial.begin(115200);
//	while(!Serial)
//		;
}

void led_sleep() {
	//pinMode(_CAPS_LED, INPUT_PULLUP);
	//pinMode(_BG_LED1, INPUT_PULLUP);
	BGLED(0);
	CAPSLED(0);
}


// scancode --> HID UsageID
// usageid = hid_table[][ scancode - 1 ];
static const uint8_t HID_TABLE_BASE = 0;
static const uint8_t HID_TABLE_FN1 = 1;
static const uint8_t scan_to_hid_table[2][SW_COUNT] = {
  {     
	HID_ESCAPE,  HID_F1,   HID_F2,   HID_F3,   HID_F4, HID_F5,    HID_F6,  HID_F7, HID_F8,   HID_F9,    HID_F10,    HID_F11,    HID_F12,     HID_PRNTSCRN,HID_SCRLOCK,HID_PAUSE,		// SW1～16
	HID_GRAVE_AC,HID_1,    HID_2,    HID_3,    HID_4,  HID_5,     HID_6,   HID_7,  HID_8,    HID_9, 	  HID_0,      HID_MINUS,  HID_EQL,     HID_J_BSLASH,HID_BACKSP, HID_UNDEF,   	// sw17～32
	HID_TAB,     HID_Q,    HID_W,    HID_E,    HID_R,  HID_T,     HID_Y,   HID_U,  HID_I,    HID_O,     HID_P,      HID_J_AT,   HID_J_LBRACK,HID_ENTER,   HID_UNDEF,  HID_UNDEF,		// sw33～48
	HID_CAPS,    HID_A,    HID_S,    HID_D,    HID_F,  HID_G,     HID_H,   HID_J,  HID_K,    HID_L,     HID_SEMICOL,HID_QUOTE,  HID_J_RBR_32,HID_INSERT,  HID_HOME,   HID_PGUP,		// sw49～64
	HID_L_SHIFT, HID_Z,    HID_X,    HID_C,    HID_V,  HID_B,     HID_N,   HID_M,  HID_COMMA,HID_PERIOD,HID_SLASH,  HID_J_UL,   HID_R_SHIFT, HID_DELETE,  HID_END,    HID_PGDOWN,	// SW65～80
  HID_L_CTRL,  HID_L_GUI,HID_L_ALT,HID_MUHEN,HID_SPC,HID_HENKAN,HID_HIRA,HID_APP,HID_R_ALT,HID_R_CTRL,HID_X_FN1,  HID_L_ARROW,HID_D_ARROW, HID_U_ARROW, HID_R_ARROW,HID_UNDEF		// SW81～	96
},
  {     
	HID_ESCAPE,  HID_F13,  HID_F14,  HID_F15,  HID_F16,HID_F17,   HID_F18, HID_F19,  HID_F20,  HID_F21,   HID_F22,    HID_F23,     HID_F24,     HID_PRNTSCRN,HID_SCRLOCK,HID_PAUSE,		// SW1～16
	HID_GRAVE_AC,HID_1,    HID_2,    HID_3,    HID_4,  HID_5,     HID_6,   HID_7,    HID_8,    HID_9, 	  HID_0,      HID_MINUS,  HID_EQL,     HID_J_BSLASH,HID_BACKSP, HID_UNDEF,   	// sw17～32
	HID_TAB,     HID_Q,    HID_W,    HID_E,    HID_R,  HID_T,     HID_Y,   HID_U,    HID_I,    HID_O,     HID_P,      HID_J_AT,   HID_J_LBRACK,HID_ENTER,   HID_UNDEF,  HID_UNDEF,		// sw33～48
	HID_CAPS,    HID_A,    HID_S,    HID_D,    HID_F,  HID_G,     HID_H,   HID_J,    HID_K,    HID_L,     HID_SEMICOL,HID_QUOTE,  HID_J_RBR_32,HID_INSERT,  HID_HOME,   HID_PGUP,		// sw49～64
	HID_L_SHIFT, HID_Z,    HID_X,    HID_C,    HID_V,  HID_B,     HID_N,   FN_M_MUTE,FN_M_V_DN,FN_M_V_UP, HID_SLASH,  HID_J_UL,   HID_R_SHIFT, HID_DELETE,  HID_END,    HID_PGDOWN,	// SW65～80
  HID_L_CTRL,  HID_L_GUI,HID_L_ALT,HID_MUHEN,HID_SPC,HID_HENKAN,HID_HIRA,HID_APP,  HID_R_ALT,HID_R_CTRL,HID_X_FN1,  HID_L_ARROW,HID_D_ARROW, HID_U_ARROW, HID_R_ARROW,HID_UNDEF		// SW81～	96
  }
};

static uint8_t hid_table_index = HID_TABLE_BASE;
void nk80_table_change(uint8_t key, bool pressed) {
	if (key == HID_X_FN1 && pressed)
		hid_table_index = HID_TABLE_FN1;
	else
		hid_table_index = HID_TABLE_BASE;
}

#include "hobo_settings.h"
extern uint32_t global_setting;

uint8_t nk80_get_key(bool& pressed, bool us_layout) { 
	uint8_t k;
	queue_entry_t key;
	if (!queue_try_remove(&key_event_queue, &key))
		return 0;
	if (key.code > 0 && key.code <= SW_COUNT) {
		uint8_t hid = scan_to_hid_table[hid_table_index][key.code - 1];
		pressed = key.flag;
		bool fn = hid_table_index == HID_TABLE_FN1;
		if (us_layout) {
			switch(hid) {
			case HID_ENTER:				//sw46
				return HID_BSLASH;	
			case HID_J_RBR_32:		// sw61
				return HID_ENTER;
			case HID_J_UL:
				return fn ? hid : HID_R_SHIFT;	// Fn + R-Shiftで HID_J_ULを出す。
//			return HID_R_SHIFT;
			case HID_CAPS:	// USレイアウトのときCapsはFnオンが条件。
				if (!_SWAP_CAPS_CTRL(global_setting) && !fn)
					return HID_IME_OFF;	// CapsとCtrlを入れ替えるときはHID_CAPSとして出す。	
				break;
			case HID_HIRAGANA:
				return HID_IME_ON;
			default:
				break;
			}
		}
		return hid;
	}
	return 0;
}
