/**
 * 
	ps2_hobo_nicola.ino Main sketch of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
	Copyright (c) 2018 Takeshi Higasa

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
 
  hoboNicolaLibrary 1.6.4   Feb. 1. 2023
*/

#include "hobo_nicola.h"
#include "hobo_board_config.h"
#include "hobo_sleep.h"
#include "ps2_kbd.h"

ps2_kbd* ps2;

// Function keys with Fn-key pressed.
static const uint16_t fn_keys[] PROGMEM = {
	HID_S | WITH_R_CTRL,	FN_SETUP_MODE,
	HID_M,								FN_MEDIA_MUTE,
	HID_COMMA,						FN_MEDIA_VOL_DOWN,
	HID_PERIOD,						FN_MEDIA_VOL_UP,
	HID_U_ARROW,					FN_MEDIA_VOL_UP,
	HID_D_ARROW,					FN_MEDIA_VOL_DOWN,
	HID_R_ARROW,					FN_MEDIA_SCAN_NEXT,
	HID_L_ARROW,					FN_MEDIA_SCAN_PREV,
	HID_ENTER,						FN_MEDIA_PLAY_PAUSE,
	HID_ESCAPE | WITH_R_CTRL,  FN_SYSTEM_SLEEP,   // Ctrl + App + Esc 
	0, 0
};

class ps2_hobo_nicola : public HoboNicola {
public:
	ps2_hobo_nicola() {}
	const uint16_t* fn_keys_table() { return fn_keys; }
	// if you want to change soft-Fn key, un-comment below. 
	//  const uint8_t fn_key() { return HID_APP; }
};

ps2_hobo_nicola hobo_nicola;

class ps2_event : public keyboard_notify {
	public:
		void raw_key_event(uint8_t key, bool released = false) {
			if (is_usb_suspended()) {
				usb_wakeup();
				ps2->kbd_reset();
				return;
			}
		};
		void key_pressed  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, true);  };
		void key_released  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, false); };
};

void setup() {
	hobo_device_setup(false);
	HoboNicola::init_hobo_nicola(&hobo_nicola);
	ps2 = ps2_kbd::getInstance();
	if (!ps2->begin(new ps2_event, LED_BUILTIN_RX))
		hobo_nicola.error_blink(400);
}

void loop() {
	if (is_usb_suspended() ) {
		ps2->task(0);
		all_led_off();
		enter_sleep(500);
	} else {
		ps2->task(hobo_nicola.get_hid_led_state());
		hobo_nicola.idle();
		enter_sleep();
	}
}
