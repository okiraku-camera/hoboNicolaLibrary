/**
 * 
  hobo_nicola.h HoboNicola class definitions of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2021-2023 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include "hid_keycode.h"
#include "hid_wrap.h"

#include "hobo_settings.h"
#include "hobo_led.h"

#if !defined(__HOBO_NICOLA_H__)
#define __HOBO_NICOLA_H__

#define HOBO_NICOLA_LIBRARY_VERSION "171"

#if !defined(ARDUINO_ARCH_AVR)
#define PROGMEM
#define PGM_READ_ADDR(a)	(uint8_t*)a
#define PGM_READ_BYTE(a)	*a
#define PGM_READ_WORD(a)	*(uint16_t*)a
#else
#define PGM_READ_ADDR(a)	(uint8_t*)pgm_read_word(&a)
#define PGM_READ_BYTE(a)	(uint8_t)pgm_read_byte(a)
#define PGM_READ_WORD(a)	(uint16_t)pgm_read_word(a)
#endif

#if !defined(_BV)
	#define _BV(b) (1 << (b))
#endif

#define LOWBYTE(a) (uint8_t)(a & 0xff)
#define HIGHBYTE(a) (uint8_t)((a >> 8) &  0xff)
#define MKWORD(k, n) (uint16_t)( k << 8 | n ) 

typedef enum {  // ステートマシンに与えるイベント
	Init = 0,
	Moji_pressed,
	Oyayubi_pressed,
	Key_released,
	Time_out,
	All_off,
	Key_repeat
} nicola_event_t;

extern _Settings& Settings();

// following bytes have hid-id
#define HID_DIRECT_PREFIX	0x01

// 同時打鍵用の仮想コード定義
static const uint8_t NID_LEFT_OYAYUBI	= 0x80;
static const uint8_t NID_RIGHT_OYAYUBI = 0x81;
static const uint8_t NID_SHIFT_KEY = 0x82;
static const uint8_t NID_LONG_PRESSED = 0x83;
static const uint8_t NID_SETUP_KEY = 0xf0;

class HoboNicola  {
	key_report_t report;
	uint32_t settings;	// current copy of eeprom.

	uint8_t nicola_mode;	// 同時打鍵しますよ。
	
	bool disable_nicola;
	bool dedicated_oyakeys;
	// 左右の親指キーが専用または決め打ちの場合に実装側で値をセットすること。
	uint8_t left_oyayubi_code;
	uint8_t right_oyayubi_code;
	
public:
	HoboNicola();
	const uint8_t isNicola() ;
	void key_event(uint8_t code, bool pressed);
	void releaseAll(bool all = true);
	void idle();
	void error_blink(int period = 100);
	void set_disable_nicola(bool f = true) { disable_nicola = f; }

// 親指キーのコードを変換、無変換、空白以外で指定したいようなとき
	void set_oyayubi_keys(uint8_t left, uint8_t right) { left_oyayubi_code = left; right_oyayubi_code = right; }
	void has_dedicated_oyakeys(bool f = true) { dedicated_oyakeys = f; }

	enum {
		Initial_State = 0,		// 初期状態
		Oyayubi_State,			// 親指キー押下状態
		Character_State,		// 文字キー押下状態
		Char_Oya_State,			// 文字キー押下中の親指キー押下状態
		Repeat_State,			// リピート中状態
		Release_Wait_State		// 文字確定後リリース待ち（長押し用）
	} state;
	
	bool is_initial_state() const { return (bool) ( state == Initial_State); }

	void enter_setup_mode(bool f = true) { setup_mode = f; }
	bool is_setup_mode() const { return setup_mode; }

	uint8_t isScrLock() const;
	uint8_t isNumLock() const;

	static uint8_t get_hid_led_state();
	static uint8_t hid_led_state;
	static uint8_t last_hid_led_state;
// 1.6.2 hoboNicolaアダプター用の実装を呼ぶようにしたのでキーボード組み込み時は別途実装する。
	virtual void nicola_led(uint8_t on) { led_nicola(on); }
	virtual void error_led(uint8_t on) { led_error(on); }
	virtual void toggle_nicola_led()  { led_toggle_nicola(); }
// LED通知を反映。LEDあれば実装する。
	virtual void capslock_led(uint8_t on) {}	
	virtual void scrlock_led(uint8_t on) {}
	virtual void numlock_led(uint8_t on) {}

	static void init_hobo_nicola(HoboNicola* kbd_impl, const char* device_name = 0) ;
	bool doFunction(const uint8_t code, bool pressed);
	virtual void extra_function(uint8_t fk, bool pressed) {}
	virtual const uint16_t* fn_keys_table();	// Fnキー押しながらキーのテーブル
	virtual const bool is_fn_key_solid() { return false; }
	virtual const uint8_t fn_key() { return HID_APP; }	// default timed Fn_key code.
	virtual const bool has_fn_keytable() { return false; }
	virtual void fn_event(uint8_t key, bool on) {}

	void apply_kbd_led();
	void restore_kbd_led();


protected:
	uint8_t modifiers;

	void report_press(uint8_t key, uint8_t mod);
	void report_release(uint8_t key, uint8_t mod, bool use_mod = false);
	void key_report(uint8_t k, uint8_t mod, bool pressed);

	bool isAllReleased() const;
	
	void stroke(uint8_t key, uint8_t mod, bool no_release = false);		// press and release. hid-code.
	void strokeChar(uint8_t c, uint8_t& mod, bool no_release = false);	// press and release. ascii-code.
	void strokeChar(uint8_t c);
	void send_PGM_string(const uint8_t* p, unsigned long delay_ms = 1);	// send pgm string by char.
	void send_PGM_string2(const uint8_t* p);	// まとめて出力

	static void set_nid_table(bool us_layout = true);

private:
	void nicola_state(nicola_event_t e, uint16_t param = 0);
	void moji_set(uint16_t param);
	void oyayubi_set(uint16_t param);
	void timer_tick(unsigned long now);

	uint16_t get_nid(uint8_t& k);
	void output() ;
	const uint8_t* get_output_data(uint16_t moji, uint16_t oyayubi);  // return pgm address.

	unsigned long event_time;
	unsigned long moji_time;
	unsigned long oyayubi_time;
	unsigned long repeat_time;
	
	uint16_t oyayubi;
	uint16_t moji;
	uint16_t repeat_moji;
	uint16_t repeat_oyayubi;

	const unsigned long e_charTime = 200;
	const unsigned long e_oyaTime = 200;
	const unsigned long e_nicolaTime = 80;
	const unsigned long repeat_delay = 250;
	const unsigned long repeat_interval = 80;
	const unsigned long e_longpressTime = 60;	// 長押し

	void immediate_output(uint16_t moji);

	bool setup_mode;
	void setup_options(uint8_t nid);
	void show_setting();
	void show_hex();
};
#endif