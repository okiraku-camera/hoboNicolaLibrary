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

	hoboNicola 1.7.6.	  Mar. 11.  2024. setup related.	
		
*/

#include "hobo_nicola.h"
#include "hobo_settings.h"
#if defined(ARDUINO_ARCH_RP2040)
#include "hardware/watchdog.h"
#endif
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
static const uint8_t set_19[] PROGMEM = 	"S : SPC -> MUHENKAN/F14";
static const uint8_t set_20[] PROGMEM = 	"I : CAPS -> ImeOff";
static const uint8_t set_21[] PROGMEM = 	"B : MUHENKAN/F14 -> NICOLA ON";
static const uint8_t set_22[] PROGMEM = 	"F : MUHENKAN/F14 -> ImeOff";
static const uint8_t set_23[] PROGMEM = 	"O : HIRAGANA -> ImeOn";
static const uint8_t set_24[] PROGMEM = 	"X : DISABLE NICOLA";
static const uint8_t set_25[] PROGMEM = 	"J : HENKAN/F15 -> NICOLA ON";
static const uint8_t set_26[] PROGMEM = 	"R : REDUCE OUPUT DELAY";
static const uint8_t set_27[] PROGMEM = 	"Y : LEFT OYAYUBI -> ENTER";
static const uint8_t set_28[] PROGMEM = 	"Z : HK/MHK -> F14/F15";

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
	set_21,
	set_22,
	set_23,
	set_24,
	set_25,
	set_26,
	set_27,
	set_28,
	set_end 
};

static char hex_char(uint8_t b) {
	if (b < 10)
		return '0' + b;
	else	
		return 'A' + (b - 10);
}

static char* hex_string32(uint32_t val, char* hex) {
	for (uint8_t i = 0; i < 4; ++i) {
		uint8_t b = (val >> (8 * (3 - i))) & 0xFF;
		hex[i * 2] = hex_char(b >> 4);
		hex[i * 2 + 1] = hex_char(b & 0x0F);
	}
	hex[8] = '\0';
	return hex;
}

#if 0
char* to_string32(uint32_t val, char* str) {
    char temp[11];
    int index = 0;
    do {
        temp[index++] = '0' + val % 10; 
        val /= 10;
    } while (val > 0);
    for (int i = 0; i < index; ++i)
        str[i] = temp[index - 1 - i];
    str[index] = '\0';
		return str;
}
#endif

static const char _sep[] =	" : ";

void HoboNicola::show_hex() {
	char tmp[64];
	char hex[12];
//	sprintf(tmp, "%08lx : %08lx : %ld : %ld", pSettings->get_data(), pSettings->get_extra(), pSettings->get_flush_count(), pSettings->get_size());
	hex_string32(pSettings->get_data(), tmp);
	strcat(tmp, _sep);
	strcat(tmp, hex_string32(pSettings->get_extra(), hex));
	strcat(tmp, _sep);
	strcat(tmp, hex_string32(pSettings->get_flush_count(), hex));
	strcat(tmp, _sep);
	strcat(tmp, hex_string32(pSettings->get_size(), hex));
	for(uint8_t i = 0; i < strlen(tmp); i++) {
		strokeChar(tmp[i]);
		delay(10);
	}
	stroke(HID_ENTER, 0);
}

void HoboNicola::show_setting() {
	if (!_US_LAYOUT(global_setting))
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
					f = _SPC_TO_LEFT(global_setting);
					break;
				case 2:
					f = _SPC_TO_RIGHT(global_setting);
					break;
				case 3:
					f = _SPC_TO_NONE(global_setting);
					break;
				case 4:
					f = _SINGLE_OYAYUBI_MODE(global_setting);
					break;
				case 5:
					f = _SELF_REPEAT(global_setting);
					break;
				case 6:
					f = _SCR_AS_NICOLA(global_setting);
					break;
				case 7:
					f = _EISU_TO_NICOLA_OFF(global_setting);
					break;
				case 8:
					f = _KANA_TO_NICOLA_ON(global_setting);
					break;
				case 9:
					f = _KANJI_TOGGLE_NICOLA(global_setting);
					break;
				case 10:
					f = _KANJI_TO_NICOLA_OFF(global_setting);
					break;
				case 11:
					f = _OUTPUT_IMMEDIATE_ON(global_setting);
					break;
				case 12:
					f = _RALT_TO_HIRAGANA(global_setting);
					break;
				case 13:
					f = _SWAP_CAPS_CTRL(global_setting);
					break;
				case 14:
					f = _HENKAN_TO_SPC(global_setting);
					break;
				case 15:
					f = _US_LAYOUT(global_setting);
					break;
				case 16:
					f = _USE_MSC_NOTIFY(global_setting);
					break;
				case 17:
					f = _USE_KBD_SUSPEND(global_setting);
					break;
				case 18:
					f = _NUML_AS_NICOLA(global_setting);
					break;
				case 19:
					f = _SPC_TO_MUHENKAN(global_setting);
					break;
				case 20:
					f = _CAPS_TO_IMEOFF(global_setting);
					break;
				case 21:
					f = _MUHENKAN_TO_NICOLA_ON(global_setting);
					break;
				case 22:
					f = _MUHENKAN_TO_IMEOFF(global_setting);
					break;
				case 23:
					f = _KANA_TO_IMEON(global_setting);
					break;
				case 24:
					f = _DISABLE_NICOLA(global_setting);
					break;
				case 25:
					f = _HENKAN_TO_NICOLA_ON(global_setting);
					break;
				case 26:
					f = _REDUCE_DELAY(global_setting);
					break;
				case 27:
					f = _MUHENKAN_F14_TO_LEFT(global_setting);
					break;
				case 28:
					f = _HENKAN_MUHENKAN_FK(global_setting);
					break;
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
#if defined(ARDUINO_ARCH_RP2040)
	watchdog_update ();
#endif

	}
	show_hex();
	stroke(HID_ENTER, 0);
	stroke(HID_ENTER, 0);
	if (isNicola())
		nicola_mode = false;
}
//  指定のスロットからの読出し、または指定のスロットへの書き込みを行う。
// 読出し後にはマイコンをリセットしたい。
void HoboNicola::setup_memory_select(uint8_t hid) {
	int8_t index = 0;
	switch(hid) {
	case HID_A:
	case HID_B:
	case HID_C:
		index = hid - HID_A;
		if (memory_setup_option == Memory_Setup_Write)
			pSettings->save_set(index);
		else if (memory_setup_option == Memory_Setup_Read) {
			uint32_t data = pSettings->load_set(index);
			pSettings->save(data);
			global_setting = pSettings->get_data();
		}
#if defined(ARDUINO_ARCH_RP2040)
 	// rp-hobo-nicolaはreboot
		if (use_pio_usb) {
			watchdog_reboot(0, 0, 100);
			return;
		}
#endif

		break;
	case HID_P:	// 3つのセットの値を表示する。
		{
			char tmp[64] = "cur:";
			stroke(HID_ENTER, 0);
//			sprintf(tmp, "cur: %08lx, A: %08lx, B: %08lx, C: %08lx", pSettings->get_data(), pSettings->load_set(0), pSettings->load_set(1), pSettings->load_set(2));
			char hex[12];
			strcat(tmp, hex_string32(pSettings->get_data(), hex));
			strcat(tmp, ", A:");
			strcat(tmp, hex_string32(pSettings->load_set(0), hex));
			strcat(tmp, ", B:");
			strcat(tmp, hex_string32(pSettings->load_set(1), hex));
			strcat(tmp, ", C:");
			strcat(tmp, hex_string32(pSettings->load_set(2), hex));
			for(uint8_t i = 0; i < strlen(tmp); i++) {
				strokeChar(tmp[i]);
				delay(10);
			}
			stroke(HID_ENTER, 0);
		}
		break;
	default:
		break;
	};
	memory_setup_option = Memory_Setup_None;
}


void HoboNicola::setup_options(uint8_t hid) {
	uint32_t new_settings = pSettings->get_data();
	uint32_t save_set = new_settings;
	switch(hid) {
	case HID_J_BSLASH:	// 上段右端
	case HID_BSLASH:	// US backslash or HID_J_RBRACK
	case HID_BACKSP:
		show_setting();
		setup_mode = false;
		return;
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
		if (_KANJI_TOGGLE_NICOLA(global_setting))
			new_settings &= ~KANJI_TOGGLE_NICOLA;
		else {
			new_settings |= KANJI_TOGGLE_NICOLA;
			new_settings &= ~KANJI_TO_NICOLA_OFF;
		}
		break;
	case HID_0:
		if (_KANJI_TO_NICOLA_OFF(global_setting))
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
		new_settings ^= US_LAYOUT;	// WindowsがUSレイアウトのときに使う。英数出力をUSにするわけではない。
		set_nid_table(new_settings & US_LAYOUT);
		if (new_settings & US_LAYOUT)
			new_settings |= HENKAN_MUHENKAN_FK;
		break;
	case HID_M:
#if defined(USE_TINYUSB)
		new_settings ^= USE_MSC_NOTIFY;	// TINYUSBのときだけ。
#endif
		break;
	case HID_K:
		new_settings ^= USE_KBD_SUSPEND;
		break;
	case HID_S:
		new_settings ^= SPC_TO_MUHENKAN;
		break;
	case HID_I:
		new_settings ^= CAPS_TO_IMEOFF;
		break;
	case HID_B:
		new_settings ^= MUHENKAN_TO_NICOLA_ON;
		break;
	case HID_F:
		new_settings ^= MUHENKAN_TO_IMEOFF;
		break;
	case HID_O:
		new_settings ^= KANA_TO_IMEON;
		break;
	case HID_X:
		new_settings ^= DISABLE_NICOLA;
		break;
	case HID_J:
		new_settings ^= HENKAN_TO_NICOLA_ON;
		break;
	case HID_R:
		new_settings ^= REDUCE_DELAY;
		break;
	case HID_Y:
		new_settings ^= MUHENKAN_F14_TO_LEFT;
		break;
	case HID_Z:
		new_settings ^= HENKAN_MUHENKAN_FK;
		break;
	}
	setup_mode = false;
	if (save_set != new_settings) {
		pSettings->save(new_settings);
		global_setting = pSettings->get_data();
		if (new_settings & REDUCE_DELAY)
			set_hid_output_delay(HID_DELAY_SHORT);
		else
			set_hid_output_delay(HID_DELAY_NORMAL);
	} 	
// SAMDのみ。nRF52ではうまくいかないので。	
#if defined(__SAMD21G18A__) || defined(__SAMD21E18A__) 
	if ((new_settings ^ save_set) & USE_MSC_NOTIFY)
		NVIC_SystemReset();
#elif defined(ARDUINO_ARCH_RP2040)
 	// rp-hobo-nicolaの場合、rebootしないと入力処理が停止してしまう。eeprom libraryのcommit() がcore1に影響しているのか
	if (use_pio_usb && (new_settings != save_set)) { 
		watchdog_reboot(0, 0, 100);	// reboot after a while.
		return;
	}
#endif
}

