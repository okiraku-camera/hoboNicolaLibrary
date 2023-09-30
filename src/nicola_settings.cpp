/**
 * 
  nicola_settings.cpp  Settings UI of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2021 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

  Included in...
	  hoboNicola 1.5.0.		Aug. 10. 2021.
	  hoboNicola 1.6.1.		Apr. 1.  2022.	M,K,N settings.
		hoboNicola 1.7.0.	  Apr. 1.  2023.	
	
*/

#include "hobo_nicola.h"
#include "hobo_settings.h"
#if defined(ARDUINO_NRF52_ADAFRUIT)
#include "nrf_nvic.h"
#endif
//
//	output current settings as keyboard inputs.
//
static const uint8_t empty_str[] PROGMEM = "";
static const uint8_t set_title[] PROGMEM =	"CURRENT SETTINGS OF HOBO_NICOLA : "  HOBO_NICOLA_LIBRARY_VERSION "...";
static const uint8_t set_1[] PROGMEM =		"1 : SPC -> LEFT OYAYUBI";
static const uint8_t set_2[] PROGMEM =		"2 : SPC -> RIGHT OYAYUBI";
static const uint8_t set_3[] PROGMEM =		"3 : SPC -> NONE";
static const uint8_t set_4[] PROGMEM =		"4 : SINGLE OYAYUBI (override 1-3)";
static const uint8_t set_5[] PROGMEM =		"5 : REPEAT ON";
static const uint8_t set_6[] PROGMEM =		"6 : SCRLOCK = NICOLA MODE";
static const uint8_t set_7[] PROGMEM =		"7 : EISU/ImeOff -> NICOLA OFF";
static const uint8_t set_8[] PROGMEM =		"8 : HIRAGANA/ImeOn -> NICOLA ON";
static const uint8_t set_9[] PROGMEM =		"9 : HAN/ZEN -> TOGGLE NICOLA";
static const uint8_t set_10[] PROGMEM =		"0 : HAN/ZEN -> NICOLA OFF";
static const uint8_t set_11[] PROGMEM = 	"- : IMMEDIATE OUTPUT";
static const uint8_t set_12[] PROGMEM = 	"A : R-ALT -> HIRAGANA";
static const uint8_t set_13[] PROGMEM = 	"C : CAPS <-> LEFT CTRL";
static const uint8_t set_14[] PROGMEM = 	"H : HENKAN/F15 -> SPC";
static const uint8_t set_15[] PROGMEM = 	"U : US LAYOUT";
static const uint8_t set_16[] PROGMEM = 	"M : MSC NOTIFY (TINYUSB)";
static const uint8_t set_17[] PROGMEM = 	"K : KEYBOARD SUSPEND";
static const uint8_t set_18[] PROGMEM = 	"N : NUMLOCK = NICOLA MODE";
static const uint8_t set_19[] PROGMEM = 	"S : SPC    -> MUHENKAN (US:F14)";
static const uint8_t set_20[] PROGMEM = 	"I : CapsLock -> ImeOff (US)";
// static const uint8_t set_21[] PROGMEM = 	"Z : HAN/ZEN -> SHIFT+SPACE";

static const uint8_t set_end[] PROGMEM = 	"...";

// display order
const uint8_t* const output_settings[] PROGMEM = {
	set_title,
	set_1, 
	set_2, 
	set_3, 
	set_4, 
	set_5, 
	set_6, 
	set_7, 
	set_8, 
	set_9,
	set_10,
	set_11,
	set_12,
	set_13,
	set_14,
	set_15,
	set_16,
	set_17,
	set_18,
	set_19,
	set_20,
//	set_21,
	set_end 
};

void HoboNicola::show_hex() {
	char tmp[64];
	sprintf(tmp, "%08lx : %08lx : %ld : %ld", 
			Settings().get_data(), Settings().get_extra(), Settings().get_flush_count(), Settings().get_size());
	for(uint8_t i = 0; i < strlen(tmp); i++) {
		strokeChar(tmp[i]);
		delay(10);
	}
	stroke(HID_ENTER, 0);
}

void HoboNicola::show_setting() {
	if (!Settings().is_us_layout())
		stroke(HID_CAPS, 0);
	delay(5);
	stroke(HID_ENTER, 0);
	for(uint8_t i = 0; i < sizeof(output_settings) / sizeof(uint8_t*); i++) {
		uint8_t* address = PGM_READ_ADDR(output_settings[i]);
		if (address) {
			if (pgm_read_byte(address)) {
				bool f = false;
				switch(i) {
				case 1:
					f = Settings().is_spc_to_left();
					break;
				case 2:
					f = Settings().is_spc_to_right();
					break;
				case 3:
					f = Settings().is_spc_to_none();
					break;
				case 4:
					f = Settings().is_single_oyayubi_mode();
					break;
				case 5:
					f = Settings().is_self_repeat();
					break;
				case 6:
					f = Settings().is_scrlock_as_nicola();
					break;
				case 7:
					f = Settings().is_eisu_to_nicola_off();
					break;
				case 8:
					f = Settings().is_kana_to_nicola_on();
					break;
				case 9:
					f = Settings().is_kanji_toggle_nicola();
					break;
				case 10:
					f = Settings().is_kanji_to_nicola_off();
					break;
				case 11:
					f = Settings().is_immediate_output();
					break;
				case 12:
					f = Settings().is_ralt_to_hiragana();
					break;
				case 13:
					f = Settings().is_swap_caps_ctrl();
					break;
				case 14:
					f = Settings().is_henkan_to_spc();
					break;
				case 15:
					f = Settings().is_us_layout();
					break;
				case 16:
					f = Settings().is_use_msc_notify();
					break;
				case 17:
					f = Settings().is_use_kbd_suspend();
					break;
				case 18:
					f = Settings().is_numlock_as_nicola();
					break;
				case 19:
					f = Settings().is_spc_to_muhenkan();
					break;
				case 20:
					f = Settings().is_caps_to_imeoff_us();
					break;
//				case 21:
//					f = Settings().is_kanji_shift_space();
//					break;
				default:
					break;
				}
				if (i > 0 && i < (sizeof(output_settings) / sizeof(uint8_t*)) - 1)
					strokeChar(f ? '*' : ' ');
				delay(10);
				send_PGM_string((const uint8_t*)address, 10);
				stroke(HID_ENTER, 0);
				delay(10);
			}
		}
	}
	show_hex();
	stroke(HID_ENTER, 0);
	stroke(HID_ENTER, 0);
	if (isNicola())
		nicola_mode = false;
}

#if defined(ARDUINO_ARCH_RP2040)
#include "hardware/watchdog.h"
#endif
// nicola_state から呼ばれる。
void HoboNicola::setup_options(uint8_t hid) {
	uint32_t new_settings = Settings().get_data();
#if defined(__SAMD21G18A__) || defined(__SAMD21E18A__) || defined(ARDUINO_ARCH_RP2040)
	uint32_t save_set = new_settings;
#endif
	switch(hid) {
	case HID_J_BSLASH:	// 上段右端
	case HID_BSLASH:	// US backslash or HID_J_RBRACK
	case HID_BACKSP:
		show_setting();
		break;
	case HID_1:
		new_settings |= SPC_TO_LEFT;
		new_settings &= ~SPC_TO_RIGHT;
		break;
	case HID_2:
		new_settings |= SPC_TO_RIGHT;
		new_settings &= ~SPC_TO_LEFT;
		break;
	case HID_3:
		new_settings &= ~SPC_TO_OYAYUBI;
		break;
	case HID_4:	// シングル親指モードのときは、親指キー設定は触らないが動作はこちらが優先し空白キーが親指キーになる
		new_settings ^= SINGLE_OYAYUBI_MODE;
		break;
	case HID_5:
		new_settings ^= SELF_REPEAT;
		break;
	case HID_6:
		new_settings ^= SCR_AS_NICOLA;
		nicola_mode = false;
		break;
	case HID_N:
		new_settings ^= NUML_AS_NICOLA;
		nicola_mode = false;
		break;
	case HID_7:
		new_settings ^= EISU_TO_NICOLA_OFF;
		break;
	case HID_8:
		new_settings ^= KANA_TO_NICOLA_ON;
		break;
	case HID_9:
		if (Settings().is_kanji_toggle_nicola())
			new_settings &= ~KANJI_TOGGLE_NICOLA;
		else {
			new_settings |= KANJI_TOGGLE_NICOLA;
			new_settings &= ~KANJI_TO_NICOLA_OFF;
		}
		break;
	case HID_0:
		if (Settings().is_kanji_to_nicola_off())
			new_settings &= ~KANJI_TO_NICOLA_OFF;
		else {
			new_settings |= KANJI_TO_NICOLA_OFF;
			new_settings &= ~KANJI_TOGGLE_NICOLA;
		}
	case HID_MINUS:
		new_settings ^= OUTPUT_IMMEDIATE_ON;
		break;
	case HID_A:
		new_settings ^= RALT_TO_HIRAGANA;
		break;
	case HID_C:
		new_settings ^= SWAP_CAPS_CTRL;
		break;
	case HID_H:
		new_settings ^= HENKAN_TO_SPC;
		break;
	case HID_U:
		new_settings ^= US_LAYOUT;	// Windows10がUSレイアウトのときに使う。英数出力をUSにするわけではない。
		set_nid_table(new_settings & US_LAYOUT);
		break;
	case HID_M:
#if defined(USE_TINYUSB)
		new_settings ^= USE_MSC_NOTIFY;	// TINYUSBのときだけ。
#endif
		break;
	case HID_K:
		new_settings ^= USE_KBD_SUSPEND;
		break;
//	case HID_Z:
//		new_settings ^= KANJI_TO_SHIFT_SPACE;
//		break;
	case HID_S:
		new_settings ^= SPC_TO_MUHENKAN;
		break;
	case HID_I:
		new_settings ^= CAPS_TO_IMEOFF_US;
		break;
	}
	setup_mode = false;
	Settings().save(new_settings);
// SAMDのみ。nRF52ではうまくいかないので。	
#if defined(__SAMD21G18A__) || defined(__SAMD21E18A__) 
	if ((new_settings ^ save_set) & USE_MSC_NOTIFY)
		NVIC_SystemReset();
#elif defined(ARDUINO_ARCH_RP2040)
	if (new_settings != save_set) { 
		delay(100);
	// rp-hobo-nicolaの場合、rebootしないと入力処理が停止してしまう。eeprom libraryのcommit() でcore1が止まるのか。
		watchdog_reboot(0, 0, 10);	// reboot after a while.
	}
#endif
}

