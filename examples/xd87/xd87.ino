/**
 * 
  xd87.ino  Main sketch of "Hobo-nicola xd87 PCB" using Hobo-nicola keyboard and adapter library.
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

    Included in hoboNicola 1.6.4.		Feb. 23. 2023.
		solid Fnkey version.
*/
#include "xd87.h"
#include "hobo_nicola.h"
#include "hid_keycode.h"
#include "hobo_sleep.h"

#define BGLED(a)    PORTD = a ? PORTD & ~_BV(0) : PORTD | _BV(0)
#define CAPSLED(a)  PORTE = a ? PORTE & ~_BV(2) : PORTE | _BV(2)

// Extended function keys with Fn-key pressed.
// Fnオンでscan_to_hidテーブルが切り替わるならば Fnオン時のコードで定義する。
static const uint16_t fn_keys[] PROGMEM = {
	WITH_R_CTRL | HID_S, FN_SETUP_MODE,
	WITH_L_CTRL | WITH_R_CTRL | HID_B, FN_XD_START_DFU,
	WITH_R_CTRL | HID_ESCAPE,	 FN_SYSTEM_SLEEP,		// Ctrl + Fn + Esc 
	WITH_R_CTRL | HID_PGUP, FN_MEDIA_VOL_UP,
	WITH_R_CTRL | HID_PGDOWN, FN_MEDIA_VOL_DOWN,
	WITH_R_CTRL | HID_DELETE,	 FN_MEDIA_PLAY_PAUSE,
	WITH_R_CTRL | HID_END, FN_MEDIA_SCAN_NEXT,
	WITH_R_CTRL | HID_HOME, FN_MEDIA_SCAN_PREV,
	WITH_L_CTRL | HID_PGDOWN, FN_RGB_DIMMER,
	WITH_L_CTRL | HID_PGUP, FN_RGB_BRIGHTER,
	WITH_L_ALT | WITH_L_CTRL | HID_PGDOWN, FN_RGB_OFF,
	WITH_L_ALT | WITH_L_CTRL | HID_PGUP, FN_RGB_MAX,
	0, 0
};

class xd87Keyboard : public HoboNicola {
	uint8_t _nicola_led;
public:
	xd87Keyboard() { 
		_nicola_led = 0;
	}
	const uint16_t* fn_keys_table() { return fn_keys; }
	const uint8_t fn_key() { return HID_X_FN1; }
	const bool is_fn_key_solid() { return true; }	// solidなFnキー
	void extra_function(uint8_t fk, bool pressed)	;
	void fn_event(uint8_t key, bool pressed)	{ xd87_table_change(key, pressed);  } 
	const bool has_fn_keytable() { return true; }

	void capslock_led(uint8_t on) { CAPSLED(on); }
	void scrlock_led(uint8_t on)  { BGLED(on); }
	void error_led(uint8_t on) { }
	void nicola_led(uint8_t on) {
		_nicola_led = on;
		scrlock_led(on);      
	}
	void toggle_nicola_led() {
		_nicola_led ^= 1;
		nicola_led(_nicola_led);
	}
};

xd87Keyboard xd87;
void xd87Keyboard::extra_function(uint8_t k, bool pressed) {
	if (!pressed)
		return ;  
	switch(k) {
	case FN_XD_START_DFU:
		releaseAll();	
		delay(10);
		start_dfu_bootloader();	// never return.
		break;
	case FN_RGB_DIMMER:
		xd_rgb_dimmer();
		break;
		case FN_RGB_BRIGHTER:
		xd_rgb_brighter(); 
		break;
	case FN_RGB_OFF:
		xd_rgb_dimmer(true);
		break;
	case FN_RGB_MAX:
		xd_rgb_brighter(true); 
		break;
	default:
		break;
	}
	return;
}

static const int16_t RGB_COUNT = 14;
void setup() {
	init_xd87();
	set_xd_rgb_port(RGB_COUNT, &PORTB, PINB7);
	set_xd_rgb_value(Settings().get_xd_rgb_value());
	xd_rgb_sync();
	BGLED(0);
	CAPSLED(0);
	xd87Keyboard::init_hobo_nicola(&xd87, "xd87");
	delay(300);
}

static bool suspended = false;
void loop() {
	bool pressed;
	uint8_t key = xd87_get_key(pressed, Settings().is_us_layout());
	if (!is_usb_suspended()) {
   	if (suspended) {
			xd_clear_buffer();
			suspended = false;
			xd_rgb_sync(false);
			xd87.restore_kbd_led();
		}
		if (key) {
			xd87.key_event(key, pressed);
			return;
		}
		xd87.idle();
		enter_sleep();	// idle sleep.																							
	} else { // usb suspended.
		if (key != 0) {
			usb_wakeup();
			delay(3);
			return;
		}
		if (!suspended) {
			xd_update_rgb(0);
			BGLED(0);
			CAPSLED(0);			
			delay(3);
			suspended = true;
		}
		if (Settings().is_use_kbd_suspend()) {
// 全ポートピンをINPUT_PULLUPとしておくことで、12mAほど節約できる。
			DDRB = DDRC = DDRD = DDRE = DDRF = 0; 					// all input
			PORTB = PORTC = PORTD = PORTE = PORTF = 0xff;	// all input-pullup
			DDRB = _BV(PINB7); // rgbledは0を出力し続けないと一部点灯したりする。
			PORTB &= ~_BV(PINB7);
			// CapsLock LED用のPINE2について。
			// 以下のようにLOWを出力すると消灯する。しかしながらINPUT_PULLUPのままの方が消費電流が少ない。
			// ただしスリープ中も薄く点灯してしまう。
			//DDRE = _BV(PINE2);
			//PORTE &= ~_BV(PINE2);
			enter_sleep(250);	// deepsleep
			init_xd87();
			delay(5);
			matrix_scan();
		}
	}
}
