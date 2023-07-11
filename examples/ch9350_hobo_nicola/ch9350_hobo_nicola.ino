/**
  ch9350_hobonicola.ino Main sketch of "Hobo-nicola usb/ble adapter using CH9350L".
  Copyright (c) 2022-2023 Takeshi Higasa

  This file is part of "Hobo-nicola keyboard and adapter".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

  version 1.6.4  Feb. 15, 2023.
		support Arduino boards below.
			Pro Micro(+3.3V/8MHz),
			QTPy-m0 and XIAO-m0, 
			SSCI ISP1807MB, XIAO-nRF52840,
			XIAO-RP2040, Adafruit KB2040,

*/
#include "hobo_nicola.h"
#include "hobo_sleep.h"
#include "hobo_ch9350if.h"
#include "hobo_board_config.h"

#if defined(NRF52_SERIES)
static const uint8_t FN_BLE_SWITH  = FN_EXTRA_START;
#endif
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
#if defined(NRF52_SERIES)
	HID_B | WITH_L_CTRL | WITH_L_ALT,  FN_BLE_SWITH,
#endif
	0, 0
};

class ch9350_hobo_nicola : public HoboNicola {
public:
	ch9350_hobo_nicola() {}
	const uint16_t* fn_keys_table() { return fn_keys; }
// if you want to change soft-Fn key, un-comment below. 
//  const uint8_t fn_key() { return HID_APP; }
	void extra_function(uint8_t fk, bool pressed);
};

void ch9350_hobo_nicola::extra_function(uint8_t k, bool pressed) {
	if (!pressed)
		return;
	switch (k) {
#if defined(NRF52_SERIES)
	case FN_BLE_SWITH:
		releaseAll();
		delay(10);
		if (is_ble_connected())
			stop_ble();
		else
			start_ble();
		break;
#endif
	default:
	break;
	}
}

ch9350_hobo_nicola hobo_nicola;

class _ch9350if : public ch9350if {
public:  
	_ch9350if(uint8_t rst) : ch9350if(rst) { }
	void key_event(uint8_t hid_code, bool on) {
		if (is_usb_suspended()) {
			usb_wakeup();
			delay(400);
			return;
		}
		hobo_nicola.key_event(hid_code, on);
	}
};

_ch9350if ch9350(CH_RST_PORT);  // use D2 or D3 to reset CH9350L
void setup() {
	hobo_device_setup(false, true);
	HoboNicola::init_hobo_nicola(&hobo_nicola, "ch9350_hobo_nicola");
	delay(300);
	ch9350.ch9350_setup();
}

static const uint32_t hobo_sleep_ms = 2000;  // 4000にするとうまくいかない。
// 30min経過したらキーボードによる復帰はやらない。
static const uint32_t no_resume_ms = 1800L * 1000L; // 30minutes.
static const uint32_t no_resume_count = (no_resume_ms / hobo_sleep_ms);

uint32_t sleep_counter = 0;
bool deep_suspended = false;
bool suspended = false;
void loop() {
	if (is_usb_suspended()) {
		ch9350.set_led_state(0);
		all_led_off();
		if (Settings().is_use_kbd_suspend()){
			ch9350.suspend();
			suspended = true;
		}
		enter_sleep(hobo_sleep_ms); 
		if (sleep_counter > no_resume_count)
			deep_suspended = true;
		else if (!deep_suspended) {      
			sleep_counter++;
			if (suspended) {
				ch9350.resume();
				suspended = false;
			}
			unsigned long n = millis() + 300;
			while(n > millis()) ch9350.ch9350_loop();  // chance to resume by myself.
		}
	} else {
		if (deep_suspended || suspended) {
			ch9350.resume();
			delay(100);
			sleep_counter = 0;
			deep_suspended = false;
			suspended = false;
		}
		ch9350.set_led_state(hobo_nicola.get_hid_led_state());
		ch9350.ch9350_loop();
		hobo_nicola.idle();
		ble_led(is_ble_connected());
		enter_sleep();
	}
}
