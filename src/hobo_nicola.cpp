/**
 * 
  hobo_nicola.cpp   Main functionalities of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2021 Takeshi Higasa, okiraku-camera.tokyo
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
  
		hoboNicola 1.5.0. First build.
		hoboNicola 1.6.1. support MSC Notify
		hoboNicola 1.6.2.	Feb. 22. 2022	Add MSC Notify Functionalities. 
		hoboNicola 1.7.0		Mar. 3.  2023   xd64(ver3) support, rearrangement symbols (nicol mode), xd87 us-layout.
*/

#include "hobo_nicola.h"
#include "fake_drive.h"
#include "char_to_hid.h"

_Settings& Settings() {
	return _Settings::instance();
}

static HoboNicola* kbd;

HoboNicola::HoboNicola()  {
	memset(&report, 0, sizeof(key_report_t));
	modifiers = 0;
	setup_mode = disable_nicola = dedicated_oyakeys = false;
	nicola_mode = 0;
	left_oyayubi_code = right_oyayubi_code = 0;
	nicola_state(Init);
}

// fake_driveからコールバックされる。
uint8_t last_fd_data = 0;
void msc_notify(uint8_t code) { last_fd_data = code; }

// IME状態通知でScrLockとMSC Notifyがどちらも有効のときは、MSC Notifyが優先する。
const uint8_t HoboNicola::isNicola() {
	if (disable_nicola) return 0;
	if (Settings().is_use_msc_notify() && !is_ble_connected())
		nicola_mode = last_fd_data;
	else if (Settings().is_scrlock_as_nicola()) nicola_mode = isScrLock();	
	else if (Settings().is_numlock_as_nicola()) nicola_mode = isNumLock();	
	return nicola_mode;
}

static const bool is_system_code(uint8_t code) {
	return (code >= FN_SYSTEM_CODE_START && code < FN_SYSTEM_CODE_END);
}

static const bool is_media_code(uint8_t code) {
	return (code >= FN_MEDIA_CODE_START && code < FN_MEDIA_CODE_END);
}

static const uint8_t send_consumer = 1;
static const uint8_t send_system = 2;
static uint8_t send_syscon = 0;
static void send_system_code(uint8_t code, bool pressed) {
	uint16_t data = pressed ? _BV(code - FN_SYSTEM_CODE_START) : 0;
	delay(4);
	send_hid_report(REPORT_ID_SYSTEM, &data, sizeof(data));
	send_syscon = pressed ? (send_syscon | send_system) : (send_syscon & ~send_system);
}

static void send_media_code(uint8_t code, bool pressed) {
	uint16_t data = pressed ? _BV(code - FN_MEDIA_CODE_START) : 0;
	delay(4);
	send_hid_report(REPORT_ID_CONSUMER, &data, sizeof(data));
	send_syscon = pressed ? (send_syscon | send_consumer) : (send_syscon & ~send_consumer);
}

static const uint16_t fn_keys[] PROGMEM = {	0, 0 };
const uint16_t* HoboNicola::fn_keys_table() { return fn_keys; }
// 
/**
 @brief Fnキーを押下した状態で別のキーがオンになったとき、実装側に機能キーテーブルがあればそれに従って処理する。
 @param code キーコード
 @param mod 現在の修飾キー情報
 @param pressed ONにtrue、OFF時にfalse
 @return コードを処理した場合trueを返す。
*/
bool HoboNicola::doFunction(const uint8_t code, bool pressed) {
	static const uint8_t send_consumer = 1;
	static const uint8_t send_system = 2;
	static uint8_t send_syscon = 0;
	const uint16_t* p = fn_keys_table();
	if (!p) return false;

	uint16_t data = 0;
	if (code == 0) {	// force all off.
		if (send_syscon & send_consumer)
			send_hid_report(REPORT_ID_CONSUMER, &data, sizeof(data));
		if (send_syscon & send_system)
			send_hid_report(REPORT_ID_SYSTEM, &data, sizeof(data));
		send_syscon = 0;
		releaseAll(false);
		return true;
	}

	uint16_t k;
	while((k = PGM_READ_WORD(p++)) != 0) {
		if (HIGHBYTE(k) == modifiers && LOWBYTE(k) == code) {
			uint8_t fk = LOWBYTE(PGM_READ_WORD(p));
			if (is_media_code(fk)) 
				send_media_code(fk, pressed);
  		else if (is_system_code(fk)) 
				send_system_code(fk, pressed);
			else if (fk >= HID_KBD_START && fk < HID_KBD_END)	// 0x04 - 0x9f
				key_report(fk, modifiers, pressed);
			else if (fk == FN_SETUP_MODE) {
				if (pressed)
					setup_mode ^= 1;
				releaseAll();
			} else if (fk >= FN_EXTRA_START && fk < FN_EXTRA_END)
				extra_function(fk, pressed);
			return true;
		}
		p++;
	}
	return false;
}

// timed fn_key は指定msec以上押していたらFnキーになる
// solid fn_keyは押した時点でFnキーになる。

static const unsigned long fn_timeout = 200;	
static unsigned long fn_pressed_time = 0;
static bool fn_pressed = false;
// Caps押下中フラグ　押下中はNICOLAにしない。
static bool caps_pressed = false;

void HoboNicola::key_event(uint8_t code, bool pressed) {
	if (code == fn_key()) {	// is Fn key pressed?
		if (pressed) {
			if (!is_fn_key_solid())
				fn_pressed_time = millis();
			else {
				fn_pressed = true;
				fn_event(code, true);
				nicola_state(All_off); // flush pending chars.
			}
		} else { 
			if (fn_pressed_time != 0) {// timed-fn key is released before timeout.
				stroke(code, modifiers);	// Fnkey raw code stroke.
				fn_pressed_time = 0;
			}
			fn_pressed = false;
			fn_event(0, false);
			doFunction(0, false);
		}
		return;	
	} else if (pressed && fn_pressed_time != 0)	{// Fnキー押下中でタイムアウト前に別のキーがオン
		fn_pressed_time = 0;
		fn_event(0, false);
	}

// キーの入れ替えなど
	if (Settings().is_swap_caps_ctrl()) {	// CAPSと左CTRL入れ替え
		if (code == HID_CAPS)
			code = HID_L_CTRL;
		else if (code == HID_L_CTRL)
			code = HID_CAPS;
	}
	if (code == HID_CAPS) {
		caps_pressed = pressed;
		// USレイアウトのときCapsLockキーをImeOffにする。
		// メインスケッチのFnキーテーブルで、Fn + HID_IMEOFFでCapsLockとなるようにしておく。
		if (Settings().is_caps_to_imeoff_us() && Settings().is_us_layout())
			code = HID_IME_OFF;
	}
	if (Settings().is_ralt_to_hiragana() && code == HID_R_ALT)
		code = HID_HIRAGANA;
	// アダプターをカスタマイズ不能なキーボードに接続した場合、ここで変換してやる
	if (code == HID_HIRAGANA && Settings().is_us_layout())
		code = HID_IME_ON;
/**
* 空白キーが左親指の位置にあるとき、これを無変換(F14)キーとした方が便利ならば設定する。
* 無変換キーを親指左キーとして使うことになる。空白キーを消滅させるわけにはいかないので変換キーを空白とする。
*/
	if (Settings().is_spc_to_muhenkan()) {
		if (code == HID_SPACE)
			code = Settings().is_us_layout() ? HID_F14 : HID_MUHENKAN;	
		else if (code == HID_MUHENKAN)
			code = HID_IME_OFF;	// 元々の無変換はIME-OFFにしてみる。(1.7.1)
	} 
/**
 * 変換(F15)キーが右親指の位置にあるとき、空白キーとした方が便利なら設定する 
 * 空白キーを親指右キーとして使うことになる。変換キーは消滅する。
*/
	if (Settings().is_henkan_to_spc() && (code == HID_HENKAN || code == HID_F15))
		code = HID_SPACE;
	
	if (is_media_code(code)) { 
		send_media_code(code, pressed);
		return;
	} else if (is_system_code(code)) {
		send_system_code(code, pressed);
		return;
	}
	if (code >= HID_MODIFIERS && code < HID_MODIFIERS_MAX) {	// 修飾キーをビットマップに
		uint8_t n = _BV(code - HID_MODIFIERS);
		if (pressed) {
			modifiers |= n;
			if (isNicola())
				nicola_state(All_off); // flush pending chars.
		} else
			modifiers &= ~n;
		key_report(0, modifiers, pressed);
		return;
	}
	if (fn_pressed) {	// Fnキー押下中
		if (doFunction(code, pressed))
			return;
		if (!has_fn_keytable())	// timed-fnの場合fn_pressedでは定義のないコードを出さない。
			return;
	}
	if (setup_mode) {	// 設定中
		if (pressed)
			setup_options(code);
		releaseAll();
		return;
	}

#if 0
// 全角／半角でShift+Spaceを出してみる(暫定)。ついでにNicolaモードもオンオフする。
// Settings().is_kanji_toggle_nicola() も兼ねることになる。
	if (code == HID_ZENHAN && Settings().is_kanji_shift_space()) {
		if (pressed) {	
			if (isNicola()) {
				nicola_state(All_off);  // flush
				if (Settings().is_scrlock_as_nicola()) {
					stroke(HID_SCRLOCK, modifiers);	 // scrlock off
					delay(10);
				} else
					nicola_mode = 0;
			} else {
				if (Settings().is_scrlock_as_nicola()) {
					stroke(HID_SCRLOCK, modifiers);	// scrlock on
					delay(10);
				} else
					nicola_mode = 1;
			}
			bool r_shift_on = false;
			if (!(modifiers & HID_R_SHIFT_MASK)) {
				r_shift_on = true;
				modifiers |= HID_R_SHIFT_MASK;
			}
			stroke(HID_SPACE, modifiers);
			if (r_shift_on) {
				modifiers &= ~HID_R_SHIFT_MASK;
				report_release(0, modifiers);
			}
		}
		return;
	}
#endif
	if (pressed) {
		if (!isNicola()) {
			report_press(code, modifiers);
			if (((code == HID_HIRAGANA || code == HID_IME_ON) && Settings().is_kana_to_nicola_on()) || 
					(code == HID_ZENHAN && Settings().is_kanji_toggle_nicola())) {
				if (!Settings().is_use_msc_notify()) {
					if (Settings().is_scrlock_as_nicola()) 	
						stroke(HID_SCRLOCK, modifiers);	// make scrlock on
					else if (Settings().is_numlock_as_nicola()) 
						stroke(HID_KEYPAD_NUMLOCK, modifiers);	// make numlock on
					else 
						nicola_mode = 1;
				}
			}
			return;
		} else {
			if (((code == HID_CAPS || code == HID_IME_OFF) && Settings().is_eisu_to_nicola_off()) || 
					(code == HID_ZENHAN && Settings().is_kanji_toggle_nicola()) ||
					(code == HID_ZENHAN && Settings().is_kanji_to_nicola_off())) {
				nicola_mode = 0;
				if (!Settings().is_use_msc_notify()) {
					if (Settings().is_scrlock_as_nicola()) 	
						stroke(HID_SCRLOCK, modifiers);	// make scrlock off
					else if (Settings().is_numlock_as_nicola()) 
						stroke(HID_KEYPAD_NUMLOCK, modifiers);	// make numlock off
				}
			}
		}
	} else if (!isNicola()) {
		report_release(code, modifiers);
		return;
	}

	uint16_t m = 0;
	if ((modifiers & (HID_CTRL_MASK | HID_ALT_MASK | HID_GUI_MASK)) == 0)	// SHIFT以外の修飾キーなら除外
		m = get_nid(code);
	if (m == 0 || caps_pressed) {	// NICOLA配列にしない	
		if (pressed) {
			nicola_state(All_off);  // flush
			report_press(code, modifiers);	// 関係ないキー
			return;
		} else if (state == Initial_State) {	// キーオフはInitialのみリリース。
			report_release(code, modifiers);
			return ;
		}
	}
	if (pressed) {
		if (LOWBYTE(m) == NID_LEFT_OYAYUBI || LOWBYTE(m) == NID_RIGHT_OYAYUBI)
			nicola_state(Oyayubi_pressed, m);
		else
			nicola_state(Moji_pressed, m);    
	} else {
		if (state != Initial_State)
			nicola_state(Key_released, m);
		else
			report_release(code, modifiers);
	}

}

static unsigned long blink_timer = 0;
static const unsigned long blink_interval = 300;
static const unsigned long setup_blink_interval = 300;
static const unsigned long kdblock_blink_interval = 600;
bool keyboard_lock = false;	// 暫定

void HoboNicola::idle() {
	unsigned long now = millis();
	timer_tick(now);	// drive state-machine.
	if (!setup_mode && !keyboard_lock) {
		nicola_led(isNicola());
    	blink_timer = 0;
	} else {
		const unsigned long n = setup_mode ? setup_blink_interval : kdblock_blink_interval;
		if (blink_timer == 0)
			blink_timer = now;
		else if ((now - blink_timer) > n) {
			toggle_nicola_led();
			blink_timer = now;
		}
	}
	if (fn_pressed_time != 0) {	
		if (now - fn_pressed_time > fn_timeout) {	// timed fn key is now active.
			fn_pressed_time = 0;
			fn_pressed = true;
			fn_event(fn_key(), true);
			nicola_state(All_off); // flush pending chars.
			//releaseAll(false);
		}
	}
	apply_kbd_led();
}
//
// キーボード初期化失敗時などに、 LEDを点滅して無限にループする。
//
void HoboNicola::error_blink(int period) {
	nicola_led(0);
	error_led(0);
	for(;;) {
		nicola_led(1); delay(period); nicola_led(0);delay(period);
		error_led(1); delay(period);error_led(0); delay(period);
  }  
}

// 修飾キーは変更しないオプションを追加。
void HoboNicola::releaseAll(bool all) {
	if (all) modifiers = 0;
	memset(&report, 0, sizeof(key_report_t));
	report.modifiers = modifiers;
	send_hid_report(REPORT_ID_KBD, &report, sizeof(key_report_t));
}

void HoboNicola::key_report(uint8_t k, uint8_t mod, bool pressed) {
	if (pressed) report_press(k, mod);
	else		 report_release(k, mod);
}

void HoboNicola::report_press(uint8_t key, uint8_t mod) {
	if (key != 0) {
		bool already = false;
		int empty_slot = -1;
		for(uint8_t i = 0; i < REPORT_KEY_COUNT; i++) {
			if (report.keys[i] == key)
				already = true;
			if (report.keys[i] == 0 && empty_slot < 0)
				empty_slot = i;
		}
		if (empty_slot < 0)	// error condition.
			return;			// should report it ? 
		if (!already)
			report.keys[empty_slot] = key;
	}
	report.modifiers = mod;
	send_hid_report(REPORT_ID_KBD, &report, sizeof(key_report_t));
	report.modifiers = modifiers;
}

void HoboNicola::report_release(uint8_t key, uint8_t mod, bool use_mod) {
	if (key != 0) {
		for(uint8_t i = 0; i < REPORT_KEY_COUNT; i++) {
			if (report.keys[i] == key) {
				report.keys[i] = 0;
				break;
			}
		}
	}
	if (!use_mod)
		report.modifiers = modifiers;
	else		
		report.modifiers = mod;
	send_hid_report(REPORT_ID_KBD, &report, sizeof(key_report_t));
	report.modifiers = modifiers;

}

bool HoboNicola::isAllReleased() const {
	for(uint8_t i = 0; i < REPORT_KEY_COUNT; i++) {
		if (report.keys[i] != 0) return false;
	}
	return true;
}

// 次に別の文字がすぐに出力されることが分かっているとき、リリースをせずにコードだけ消す。
// "nn"を出力するとき、「ん」にならない。同じコードが続くときは、no_release == falseであること。

void HoboNicola::stroke(uint8_t key, uint8_t mod, bool no_release) {
	if (key) {
		report_press(key, mod);
		if (!no_release)
			report_release(key, mod, true);
		else {
			for(uint8_t i = 0; i < REPORT_KEY_COUNT; i++) {
				if (report.keys[i] == key) {
					report.keys[i] = 0;
					break;
				}
			}
		}			
	}
}
void HoboNicola::strokeChar(uint8_t c) {
	uint8_t m = 0;
	strokeChar(c, m, false);
}

void HoboNicola::strokeChar(uint8_t c, uint8_t& mod, bool no_release) {
	uint16_t k = char_to_hid(c, Settings().is_us_layout());
	if ((k & WITH_SHIFT) && !(mod & HID_SHIFT_MASK))
		mod |= HID_L_SHIFT_MASK;           
	else if (!(k & WITH_SHIFT) && (mod & HID_SHIFT_MASK))
		mod &= ~HID_SHIFT_MASK;
	stroke(k, mod, no_release);
}


/**
NICOLAモードのときだけでも、先にHIDコードに変換してまとめて出力するようにしたい。
Shiftキーに無関係な文字だけを対象とする。
2キー分以上送信すると、Explorerの検索フィールドが取りこぼす。
*/
void HoboNicola::send_PGM_string2(const uint8_t* p) {
	if (!p) return;
	if (!is_ble_connected()) {	//暫定。
		send_PGM_string(p);
		return;
	}
	const uint8_t* tp = p;
	uint8_t chars[REPORT_KEY_COUNT] = { 0 };
	uint8_t i = 0;
	while(*tp) {
		uint8_t c = PGM_READ_BYTE(tp++);
		uint16_t k = char_to_hid(c, Settings().is_us_layout());
		if (k & WITH_SHIFT) {
			send_PGM_string(p);	// やっぱりやめる。
			return;
		}
		chars[i++] = k;
	}
	memcpy(report.keys, chars, i);
	report.modifiers = modifiers;
	send_hid_report(REPORT_ID_KBD, &report, sizeof(key_report_t));
	delay(i);
	memset(report.keys, 0, REPORT_KEY_COUNT);
	send_hid_report(REPORT_ID_KBD, &report, sizeof(key_report_t));
}

//
// p holds an address of string in pgm.
// 複数コードを送出するとき最後のコードのときだけ、releaseする。
void HoboNicola::send_PGM_string(const uint8_t* p, unsigned long delay_ms) {
	if (!p) return;
	uint8_t mod = modifiers;
	bool hid_direct = false;
	for(;;) {
		uint8_t c = PGM_READ_BYTE(p++);
		if (c == 0)
			break;
		if (c == HID_DIRECT_PREFIX) {
			hid_direct = true;
			continue;
		}
		if (!hid_direct) {
			// 同じ文字が連続するときはリリースしておく必要がある。
			uint8_t next_c = PGM_READ_BYTE(p);
			strokeChar(c, mod, (next_c != 0 && next_c != c));
		} else {
			stroke(c, 0);
			hid_direct = false;
		}
		delay(delay_ms);
	}
}

uint8_t HoboNicola::hid_led_state = 0;
uint8_t HoboNicola::last_hid_led_state = 0;
static void hid_led_notify(uint8_t data) {
	HoboNicola::hid_led_state = data;
}

void HoboNicola::apply_kbd_led() {
	if (HoboNicola::hid_led_state != HoboNicola::last_hid_led_state) {
		kbd->capslock_led(HoboNicola::hid_led_state & HID_LED_CAPSLOCK);
		kbd->scrlock_led(HoboNicola::hid_led_state & HID_LED_SCRLOCK);
		kbd->numlock_led(HoboNicola::hid_led_state & HID_LED_NUMLOCK);
		HoboNicola::last_hid_led_state = HoboNicola::hid_led_state;
	}
}

void HoboNicola::restore_kbd_led() {
	kbd->capslock_led(HoboNicola::hid_led_state & HID_LED_CAPSLOCK);
	kbd->scrlock_led(HoboNicola::hid_led_state & HID_LED_SCRLOCK);
	kbd->numlock_led(HoboNicola::hid_led_state & HID_LED_NUMLOCK);
	HoboNicola::last_hid_led_state = 0;
}


uint8_t HoboNicola::get_hid_led_state() {
	return hid_led_state;
}

inline uint8_t HoboNicola::isScrLock() const { return (hid_led_state & HID_LED_SCRLOCK) != 0 ? 1 : 0; }
inline uint8_t HoboNicola::isNumLock() const { return (hid_led_state & HID_LED_NUMLOCK) != 0 ? 1 : 0; }

void HoboNicola::init_hobo_nicola(HoboNicola* kbd_impl, const char* device_name) {
	kbd = kbd_impl;
	if (!device_name)
		device_name = "hoboNicola";
	set_fd_notify_cb( msc_notify );	// 常にセット

	set_hid_led_callback(hid_led_notify);
	set_nid_table(Settings().is_us_layout());
	hid_begin(device_name);
	delay(10);
	if (Settings().is_use_msc_notify())
		fake_drive_init();	// とりあえず常時オン
}