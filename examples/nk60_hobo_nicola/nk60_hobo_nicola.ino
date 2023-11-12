/**
 * 
  nk60_hobo_nicola.ino  Main sketch of "Hobo-nicola nk60 rev031 PCB" using Hobo-nicola keyboard and adapter library.
  Copyright (c) 2023 Takeshi Higasa, okiraku-camera.tokyo
  
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

    Included in hoboNicola 1.7.2.		Oct. 10. 2023.
		solid Fnkey version.

		Select "Generic RP2040 (Raspberry Pi Pico/RP2040)" as target board.

*/
#include "nk60.h"
#include "hobo_nicola.h"
#include "hid_keycode.h"
#include "hobo_sleep.h"

// nk60 extra functions.
static const uint8_t FN_START_DFU	= FN_EXTRA_START;
static const uint8_t FN_BG_BRIGHTER	= FN_EXTRA_START + 1;
static const uint8_t FN_BG_DIMMER		= FN_EXTRA_START + 2;
static const uint8_t FN_BG_MAX	      = FN_EXTRA_START + 3;
static const uint8_t FN_BG_OFF		    = FN_EXTRA_START + 4;


// Extended function keys with Fn-key pressed.
// Fnオンでscan_to_hidテーブルが切り替わるならば Fnオン時のコードで定義する。
static const uint16_t fn_keys[] PROGMEM = {
	WITH_R_CTRL | HID_S, FN_SETUP_MODE,
	WITH_R_CTRL | HID_ESCAPE,	 FN_SYSTEM_SLEEP,		// Ctrl + Fn + Esc 
	WITH_R_CTRL | HID_ZENHAN,	 FN_SYSTEM_SLEEP,		// Fn + Escを半全キーとしている場合 
	WITH_R_CTRL | HID_PGUP, FN_MEDIA_VOL_UP,
	WITH_R_CTRL | HID_PGDOWN, FN_MEDIA_VOL_DOWN,
	WITH_R_CTRL | HID_DELETE,	 FN_MEDIA_PLAY_PAUSE,
	WITH_R_CTRL | HID_END, FN_MEDIA_SCAN_NEXT,
	WITH_R_CTRL | HID_HOME, FN_MEDIA_SCAN_PREV,
	WITH_R_CTRL | WITH_L_CTRL | HID_B, FN_START_DFU,
	HID_IME_OFF, HID_CAPS,
	0, 0
};

class nk60Keyboard : public HoboNicola {
	uint8_t _nicola_led;
public:
	nk60Keyboard() { 
		_nicola_led = 0;
	}
	const uint16_t* fn_keys_table() { return fn_keys; }
	const uint8_t fn_key() { return HID_X_FN1; }
	const bool is_fn_key_solid() { return true; }	// solidなFnキー
	void fn_event(uint8_t key, bool pressed)	{ nk60_table_change(key, pressed);  } 
	const bool has_fn_keytable() { return true; }
	void extra_function(uint8_t fk, bool pressed)	;

	void capslock_led(uint8_t on) { CAPSLED(on); }
	void error_led(uint8_t on) { }
	void nicola_led(uint8_t on) {
		_nicola_led = on;
		BGLED(on);      
	}
	void toggle_nicola_led() {
		_nicola_led ^= 1;
		nicola_led(_nicola_led);
	}
};

nk60Keyboard kbd;

extern "C" {
	#include "pico/bootrom.h"
}

void nk60Keyboard::extra_function(uint8_t k, bool pressed) {
	if (!pressed)
		return ;  
	switch(k) {
	case FN_START_DFU:
		releaseAll();	
		delay(10);
		reset_usb_boot(0, 0);	// never return.
		break;
	default:
		break;
	}
	return;
}

void setup1() {
	delay(10);	// wait for core0 setup done.
}

static unsigned long scan_interval = 5;
void loop1() {
	delay(scan_interval);
	matrix_scan();
}

void setup() {
	init_nk60();
	nk60Keyboard::init_hobo_nicola(&kbd, "nk60");
	delay(100);
}

static bool suspended = false;
void loop() {
	bool pressed;
	uint8_t key = nk60_get_key(pressed, Settings().is_us_layout());
	if (!is_usb_suspended()) {
   	if (suspended) {
			scan_interval = 5;
			suspended = false;
			kbd.restore_kbd_led();
			nk60_table_change(HID_X_FN1, false);
		}
		if (key) {
			kbd.key_event(key, pressed);
			return;
		}
		kbd.idle();
		delay(5);
	} else { // usb suspended.
		if (key != 0) {
			usb_wakeup();
			delay(3);
			return;
		}
		if (!suspended) {
			led_sleep();
			scan_interval = 200;
			delay(3);
			suspended = true;
		}
		if (Settings().is_use_kbd_suspend())	//ほとんど効果がない。
			delay(500);
	}
}
