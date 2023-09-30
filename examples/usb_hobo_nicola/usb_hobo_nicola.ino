/**
	usb_hobo_nicola.ino Main sketch of "Hobo-nicola usb/ble adapter for MAX3421EE".
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

	version 1.7.0  Jul. 1, 2023.
		support Arduino boards below.
		Pro Micro(+3.3V/8MHz),
		QTPy-m0 and XIAO-m0, 
		SSCI ISP1807MB, XIAO-nRF52840,
		XIAO-RP2040, Adafruit KB2040,
*/

#include "hobo_nicola.h"
#include "hobo_sleep.h"
#include "uhslib2.h"
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
	HID_U_ARROW,					HID_PGUP,
	HID_D_ARROW,					HID_PGDOWN,
	HID_R_ARROW,					HID_END,
	HID_L_ARROW,					HID_HOME,
	HID_ENTER,						FN_MEDIA_PLAY_PAUSE,
	HID_IME_OFF,						HID_CAPS,								// Fn + ImeOff (Caps) = CapsLock
	HID_ESCAPE | WITH_R_CTRL,  FN_SYSTEM_SLEEP,   // Ctrl + App + Esc 
#if defined(NRF52_SERIES)
	HID_B | WITH_L_CTRL | WITH_L_ALT,  FN_BLE_SWITH,
#endif
	0, 0
};

class usb_hobo_nicola : public HoboNicola {
public:
	usb_hobo_nicola() {}
	const uint16_t* fn_keys_table() { return fn_keys; }
	// if you want to change soft-Fn key, un-comment below. 
	//  const uint8_t fn_key() { return HID_APP; }
	void extra_function(uint8_t fk, bool pressed);
};

void usb_hobo_nicola::extra_function(uint8_t k, bool pressed) {
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

usb_hobo_nicola hobo_nicola;

class KeyboardEvent : public KeyboardReportParser {
	USBHID *phid;
	uint8_t last_led;
	public:
		KeyboardEvent() {
			phid = 0;
			last_led = 0;
		};
		void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
			if (is_usb_suspended()) {
				usb_wakeup();
				delay(100);
				return;
			}
			phid = hid;
			KeyboardReportParser::Parse(hid, is_rpt_id, len, buf);
		};
		void sync_led( uint8_t led ) {
			if (phid && last_led != led) {
				phid->SetReport(0, 0, 2, 0, 1, &led);
				last_led = led;
			}
		};

	protected:
		void OnControlKeysChanged(uint8_t before, uint8_t after) {
			uint8_t change = after ^ before;
			if (!change) return;
			uint8_t mask = 1;
			uint8_t key = HID_MODIFIERS;
			for (uint8_t i = 0; i < HID_MODIFIERS_COUNT; i++, mask <<= 1)  {
				if (change & mask)  hobo_nicola.key_event(key + i, after & mask);
			}
		};
		void OnKeyDown  (uint8_t mod, uint8_t key) { hobo_nicola.key_event(key, true); };
		void OnKeyUp  (uint8_t mod, uint8_t key) { hobo_nicola.key_event(key, false); };
};

USBHost Usbhost;
USBHub Hub1(&Usbhost);
USBHub Hub2(&Usbhost);
USBHub Hub3(&Usbhost);
USBHub Hub4(&Usbhost);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usbhost);
KeyboardEvent kbd;

void setup() {
	hobo_device_setup();
	HoboNicola::init_hobo_nicola(&hobo_nicola, "usb_hobo_nicola");
	if (Usbhost.Init() == -1)
		hobo_nicola.error_blink();
	delay( 200 );
	HidKeyboard.SetReportParser(0, &kbd);
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
		kbd.sync_led(0);
		all_led_off();
		if (Settings().is_use_kbd_suspend()){
			Usbhost.suspendKeyboard(); // suspend keyboard and max3421E
			suspended = true;
		}
		enter_sleep(hobo_sleep_ms);	// suspend myself.
		if (sleep_counter > no_resume_count)
			deep_suspended = true;
		else if (!deep_suspended) {      
			sleep_counter++;
			if (suspended) {
				Usbhost.resumeKeyboard(); // resume keyboard and max3421E to poll keyboard
				suspended = false;      
			}
			Usbhost.Task();  // check any key pressed.
		} 
	} else {
		if (deep_suspended || suspended) {
			Usbhost.resumeKeyboard();
			delay(100);
			sleep_counter = 0;
			deep_suspended = false;
			suspended = false;      
		}    
		Usbhost.Task();
		hobo_nicola.idle();
		kbd.sync_led(HoboNicola::get_hid_led_state());
		ble_led(is_ble_connected());
		enter_sleep();
	}
}
