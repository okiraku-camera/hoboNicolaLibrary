/**
 * 
  nk80_hobo_nicola.ino  Main sketch of "Hobo-nicola nk80 revision 2 PCB" using Hobo-nicola keyboard and adapter library.
  Copyright (c) 2024 Takeshi Higasa, okiraku-camera.tokyo
  
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

    Included in hoboNicola 1.7.6.		Mar. 8. 2024.
	
		Select "Generic RP2040 (Raspberry Pi Pico/RP2040)" as target board.

*/
#include "nk80_rev2.h"
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
	WITH_R_CTRL | HID_R,	FN_MEMORY_READ_MODE,	// read stored settings
	WITH_R_CTRL | HID_W,	FN_MEMORY_WRITE_MODE,	// store current settings

	WITH_R_CTRL | HID_ESCAPE,	FN_SYSTEM_SLEEP,		// Ctrl + Fn + Esc 
	WITH_R_CTRL | HID_ZENHAN,	 FN_SYSTEM_SLEEP,		// Fn + Escを半全キーとしている場合 
	WITH_R_CTRL | HID_ENTER,	FN_MEDIA_PLAY_PAUSE,
	WITH_R_CTRL | HID_END, 		FN_MEDIA_SCAN_NEXT,
	WITH_R_CTRL | HID_HOME,		FN_MEDIA_SCAN_PREV,
	WITH_R_CTRL | WITH_L_CTRL | HID_B, FN_START_DFU,
	WITH_R_CTRL | HID_D_ARROW, FN_BG_DIMMER,			// R-Ctrl + ↓ 
	WITH_R_CTRL | HID_U_ARROW, 	FN_BG_BRIGHTER,		// R-Ctrl + ↑ 
	HID_IME_OFF, HID_CAPS,
	0, 0
};

class nk80HoboNicola : public HoboNicola {
	uint8_t _nicola_led;
public:
	nk80HoboNicola() { 
		_nicola_led = 0;
	}
	const uint16_t* fn_keys_table() { return fn_keys; }
	const uint8_t fn_key() { return HID_X_FN1; }
	const bool is_fn_key_solid() { return true; }	// solidなFnキー
	void fn_event(uint8_t key, bool pressed)	{ nk80_table_change(key, pressed);  } 
	const bool has_fn_keytable() { return true; }
	void extra_function(uint8_t fk, bool pressed)	;

	void capslock_led(uint8_t on) { CAPSLED(on); }
	void error_led(uint8_t on) { CAPSLED(on); }
	void nicola_led(uint8_t on) {
		_nicola_led = on;
		BGLED(on);
	}
	void toggle_nicola_led() {
		_nicola_led ^= 1;
		BGLED(_nicola_led);
	}
};

nk80HoboNicola hobonicola;

extern "C" {
	#include "pico/bootrom.h"
}
#include "nk_rp_pwmled.h"

void nk80HoboNicola::extra_function(uint8_t k, bool pressed) {
	if (!pressed)
		return ;  
	switch(k) {
	case FN_START_DFU:
		releaseAll();	
		delay(10);
		reset_usb_boot(0, 0);	// never return.
		break;
	case FN_BG_DIMMER:
 		rp_pwm_dimmer();
		pSettings->save_rp_pwm_max_value(get_rp_pwm_max_value());
		break;
	case FN_BG_BRIGHTER:
		rp_pwm_brighter();
		pSettings->save_rp_pwm_max_value(get_rp_pwm_max_value());
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
	Serial.end();	// disable CDC (1.7.4)
	init_nk80();
	nk80HoboNicola::init_hobo_nicola(&hobonicola, "nk80_2");
	set_rp_pwm_max_value(pSettings->get_rp_pwm_max_value());
	delay(10);
	watchdog_start_tick(12);
	watchdog_enable(1000, false);
}

static bool suspended = false;
void loop() {
	bool pressed;
	watchdog_update ();
	uint8_t key = nk80_get_key(pressed, _US_LAYOUT(global_setting));
	if (!is_usb_suspended()) {
   	if (suspended) {
			suspended = false;
			hobonicola.restore_kbd_led();
			nk80_table_change(HID_X_FN1, false);
		}
		if (key) {
			hobonicola.key_event(key, pressed);
			return;
		}
		hobonicola.idle();
		delay(5);
	} else { // usb suspended.
		if (key != 0) {
			usb_wakeup();
			delay(3);
			return;
		}
		if (!suspended) {
			hobonicola.nicola_off();	// force off nicode mode.
			led_sleep();
			delay(3);
			suspended = true;
		}
	}
}
