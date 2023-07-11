/**
 * 
  ps2_kbd.cpp  PS/2 Keyboard interface of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
  Copyright (c) 2018,2023 Takeshi Higasa
  
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
  
	Included in hoboNicola 1.6.4.		Feb. 1. 2023.
*/

#include "ps2_kbd.h"
#include "hid_keycode.h"

// PRO MICRoのLEDは、LOW で点灯することに注意。

static const uint8_t KBD_BUFFER_SIZE = 12;
static uint8_t kbd_buffer[KBD_BUFFER_SIZE];  //  fifo key buffer 

static volatile bus_state_t bus_state = None;
static volatile uint8_t kbd_in, kbd_out;
static uint8_t cmd_to_send;
static uint8_t led_port;

static void clear_buffer() {
	cli();
	kbd_in = kbd_out = 0;
	sei();  
}

static bool put_buffer(uint8_t s) {
	uint8_t rx = kbd_in + 1;
	if (rx >= KBD_BUFFER_SIZE) 
		rx = 0;
	if (rx == kbd_out) 
		return false; // buffer is full.  
	kbd_buffer[kbd_in] = s;
	kbd_in = rx;    
	return true;  
}

static uint8_t get_buffer() {
	if (kbd_in == kbd_out)
		return 0;  // empty.
	uint8_t s = kbd_buffer[kbd_out];
	cli();
	if (++kbd_out >= KBD_BUFFER_SIZE)
		kbd_out = 0;
	sei();    
	return s;    
}

static ps2_kbd* pThis = 0;

ps2_kbd* ps2_kbd::getInstance() {
	if (!pThis)
		pThis = new ps2_kbd;
	return pThis;    
}

ps2_kbd::ps2_kbd() {
	notifier = 0; // = new keyboard_notify();
	bus_state = None;
	pThis = this;
}

bool ps2_kbd::begin(keyboard_notify* p, uint8_t led_pin) {
	if (p)
		notifier = p;
	led_port = led_pin;    
	if (led_port != 0xff) {
		pinMode(led_port, OUTPUT);
		digitalWrite(led_port, 1);	// LED off.
	}
	pinMode(PS2_DATA, INPUT);
	pinMode(PS2_CLK, INPUT);
	kbd_led_state = 0;
	bus_state = Idle;
	::clear_buffer();
	attachInterrupt(CLK_INT, clk_interrupt, FALLING);
	delay(500);
	return kbd_reset();
}

#if 0
void ps2_kbd::show_error(int ms) {
	if (led_port != 0xff) {
		digitalWrite(led_port, 1);
		delay(ms);
		digitalWrite(led_port, 0);
		delay(ms);
	}
}
#endif

void  ps2_kbd::clk_interrupt() {
	volatile static uint8_t data = 0;
	volatile static uint8_t clocks = 0;
	volatile static uint8_t par = 0;
	switch(bus_state) {
	case Idle:
		clocks = 0;
		data = 0;
//		if (led_port != 0xff)
//			digitalWrite(led_port, 0);	// LED on.
		bus_state = Receiving; // found Start bit 
		break;
	case Receiving:
		clocks++;
		if (clocks < 9) {
			data = data >> 1 | (digitalRead(PS2_DATA) ? 0x80 : 0);
			if (clocks == 8) {   // パリティとストップビットは無視。
				::put_buffer(data); // とりあえずバッファフルは見ない。
				//if (led_port != 0xff)
				//	digitalWrite(led_port, 1);	// LED off.
			}
		} else if (clocks == 10)    // STOP bit.
			bus_state = Idle;
		break;
	case WaitForStart:   // Start bit will be fetched. (DATA is LOW)
		par = 0;
		clocks = 0;
		data = cmd_to_send;
		bus_state = Sending;
		break;      
	case Sending:
		clocks++;
		switch(clocks) {
		case 9:	  // parity
			digitalWrite(PS2_DATA, par & 1 ? LOW : HIGH);
			break;
		case 10:	  // STOP
			pinMode(PS2_DATA, INPUT); // release DATA
			break;
		case 11:	  // ACK bit from keyboard.
			bus_state = Idle;
			break;
		default:	// clocks = 1～8			
			if (data & 1) {
				digitalWrite(PS2_DATA, HIGH);
				par++;
			} else 
				digitalWrite(PS2_DATA, LOW);
			data = data >> 1;
		}    
		break;
	case None:
		break;		
	}
}

bool ps2_kbd::kbd_reset() {
	uint8_t tmp[2];
	send_command(0xff, 2, tmp); // reset keyboard. 
	if (tmp[0] == 0xfa && tmp[1] == 0xaa)
		return true;
	return false;    
} 

bool ps2_kbd::kbd_led(uint8_t led) {
	if (kbd_led_state == led)
		return true;
	delay(2); // キー検出直後に呼んでもいいように、少し待つ。
	bool f = send_command(0xed); // LED
	delay(2);
	if (f) {
		kbd_led_state = led;
		f = send_command(led); // LED parameter.
	} else
		kbd_reset();
	return f;
}

void ps2_kbd::toggle_led(uint8_t led) {
	uint8_t new_led = kbd_led_state;
	if (new_led & led)
		new_led &= ~led;
  	else
		new_led |= led;
	kbd_led(new_led);  
}

bool ps2_kbd::send_command(uint8_t cmd, uint8_t resp_count, uint8_t* resp) {
	unsigned long  timeout = 15;  
	unsigned long start = millis();
	while(bus_state != Idle && (millis() - start) < timeout)
		delayMicroseconds(200);
	::clear_buffer();    
	cmd_to_send = cmd;
	bus_state = WaitForStart;
	pinMode(PS2_CLK, OUTPUT);  // drive LOW. Interrupt may occur.
	delayMicroseconds(100);    // at least 100usec.
	pinMode(PS2_DATA, OUTPUT);  // drive LOW (START bit)
	delayMicroseconds(50);
	pinMode(PS2_CLK, INPUT);  // release clock. CLK goes to HIGH. 

	start = millis();
	while(bus_state != Idle && (millis() - start) < timeout)
		delayMicroseconds(100);	// wait until command is sent.

	uint8_t ret = 0;
	for(int8_t i = 0; i < resp_count; i++) {
		start = millis();
		if (cmd == 0xff && i == 1)
			timeout = 500;  // for Basic Assuarance Test.
		while((ret = ::get_buffer()) == 0 && (millis() - start) < timeout)
			;
		if (resp)
			resp[i] = ret;
	}
	return (ret == 0xfa);
}

static const uint8_t LCTL = HID_L_CTRL;
static const uint8_t LSFT = HID_L_SHIFT;
static const uint8_t LALT = HID_L_ALT;
static const uint8_t LGUI = HID_L_GUI;
static const uint8_t RCTL = HID_R_CTRL;
static const uint8_t RSFT = HID_R_SHIFT;
static const uint8_t RALT = HID_R_ALT;
static const uint8_t RGUI = HID_R_GUI;

// ps/2 ---> hid code conversion table.
// hid_usage_id = table[scan_code - 1]; (single coded keys only).
static const uint8_t table[] = {
//  0   1      2     3    4     5     6    7    8    9     a     b     c     d    e   f
  0x42, 0,    0x3e, 0x3c, 0x3a, 0x3b, 0x45, 0,    0x43, 0x41, 0x3f, 0x3d, 0x2b, 0x35, 0, 0,   // 0 F9   --   F5    F3     F1  F2  F12      --   F10  F8  F6  F4  TAB HANKAKU -- --
  LALT, LSFT, 0x88, LCTL, 0x14, 0x1e, 0,    0,    0,    0x1d, 0x16, 0x04, 0x1a, 0x1f, 0, 0,   // 1 LALT LSFT HIRA  LCTL   Q   1   --       --   --   Z   S   A   W   2       -- --
  0x06, 0x1b, 0x07, 0x08, 0x21, 0x20, 0,    0,    0x2c, 0x19, 0x09, 0x17, 0x15, 0x22, 0, 0,   // 2 C    X    D     E      4   3   --       --   SPC  V   F   T   R   5       -- --
  0x11, 0x05, 0x0b, 0x0a, 0x1c, 0x23, 0,    0,    0,    0x10, 0x0d, 0x18, 0x24, 0x25, 0, 0,   // 3 N    B    H     G      Y   6   --       --   --   M   J   U   7   8       -- --
  0x36, 0x0e, 0x0c, 0x12, 0x27, 0x26, 0,    0,    0x37, 0x38, 0x0f, 0x33, 0x13, 0x2d, 0, 0,   // 4 ,    K    I     O      0   9   --       --   .    /   L   ;   P   -       -- --
  0x87, 0x34, 0,    0x2f, 0x2e, 0,    0,    0x39, RSFT, 0x28, 0x30, 0,    0x32, 0,    0, 0,   // 5 _    :    --    @      ^   --  --       CAPS RSFT ENT [   --  ]   --      -- --
  0,    0,    0,    0x8a, 0,    0x2a, 0x8b, 0,    0x59, 0x89, 0x5c, 0x5f, 0,    0,    0, 0x62,// 6 --   --   --    HENKAN --  BS  MUHENKAN --   P1   \   P4  P7  --  --      -- P0
  0x63, 0x5a, 0x5d, 0x5e, 0x60, 0x29, 0x53, 0x44, 0x57, 0x5b, 0x56, 0x55, 0x61, 0x47, 0, 0    // 7 P.   P2   P5    P6     P8  ESC NUMLOCK  F11  P+   P3  P-  P*  P9  SCRLOCK -- --
};
//
//  table for E0 prefixed  keys.
// E0 12を排除すること。
static const uint8_t e0_table[][2] = {
	{0x11, RALT},   // R ALT.
	{0x14, RCTL},   // R CTRL   
	{0x1f, LGUI},
	{0x27, RGUI},
	{0x2f, 0x65},   // application key.
	{0x4a, 0x54},    // Pad /
	{0x5a, 0x58},    // Pad Enter.
	{0x69, 0x4d},    // End
	{0x6b, 0x50},    // ← Left Arrow.
	{0x6c, 0x4a},    // Home.
	{0x70, 0x49},    // Insert
	{0x71, 0x4c},    // Delete
	{0x72, 0x51},    // ↓
	{0x74, 0x4f},    // →
	{0x75, 0x52},    // ↑
	{0x7a, 0x4e},    // Page Down.
	{0x7d, 0x4b},    // Page Up.
	{0x7c, 0x46},	// Print Screen.
	{0x12, 0},	// NumLockやShift押しながらのときに出てくるコード
	{0x59, 0},
	{0, 0}
};

#if 0
static const uint8_t HID_PAUSE     = 0x48;
static const uint8_t HID_PRNT_SCRN = 0x46;
static const uint8_t HID_F7        = 0x40;
#endif

#define has_E0  0x01
#define has_F0  0x02
#define has_E1  0x04

#define SET_E0(a)	(a |= has_E0)
#define SET_F0(a)	(a |= has_F0)
#define SET_E1(a)	(a |= has_E1)

#define HAS_E0(a) (bool)(flag & has_E0)
#define HAS_F0(a) (bool)(flag & has_F0)
#define HAS_E1(a) (bool)(flag & has_E1)

uint8_t ps2_kbd::hid_code(uint8_t k, bool& released) {
	released = false;
	static uint8_t flag = 0;
	uint8_t code = 0;
	switch(k) {
	case 0xe0:
		SET_E0(flag);
		return 0;
	case 0xe1:	
		SET_E1(flag);
		return 0;
	case 0xf0:	
		SET_F0(flag);
		return 0;
	case 0x12:
	case 0x59:
		if (HAS_E0(flag)) {
			flag = 0;
			return 0;
		}
		break;
	case 0x77:
		if (HAS_E1(flag) && HAS_F0(flag)) {
			flag = 0;
			return HID_PAUSE;
		}
		break;
	case 0x83:
		code = HID_F7;
		break;
	case 0x84:		// Alt + PrintScreen限定
		code = HID_PRNTSCRN;
		break;		
		
	}
	if (!code && !HAS_E1(flag) && (k > 0 && k < 0x80) ) {
		if (HAS_E0(flag)) {
			for(uint8_t i = 0; i < sizeof(e0_table) / 2; i++) {
				if (e0_table[i][0] == k) {
					code = e0_table[i][1];
					break;
				}
			}
		} else      
			code = table[k - 1];
	}
	if (code) {
		released =  (bool)(flag & has_F0) ;
		flag = 0;
	}
	return code;
}

// HIDのLEDビットをPS/2用にする。
#define HIDLED_TO_PS2LED(a) ((a << 1 & 0x6) | (a >> 2 & 1))

//
//	メインスケッチの loop() から繰り返して呼び続けることで、
//	キーボードから受信した文字の引き上げ、コード変換を実行して
//	コードが出来上がれば返す。
// led : HID style LED state.
//
void ps2_kbd::task(uint8_t led) {
	static uint8_t last_hid = 0;
	uint8_t s = ::get_buffer();
	if (s) {
		if (notifier)
			notifier->raw_key_event(s);
		bool released;      
		uint8_t hid = hid_code(s, released);
		if (hid == 0 || notifier == 0)
			return;
		if (released) {
			notifier->key_released(hid, modifier_state);
			last_hid = 0;
		} else {
			if (last_hid != hid) {
				notifier->key_pressed(hid, modifier_state); 
				last_hid = hid;
			}
			if (hid == HID_PAUSE) // PAUSEはBREAKがないので。
				notifier->key_released(HID_PAUSE, modifier_state);
		}
	} else //ヒマなときにキーボードLEDをHIDレポートと同期させる。
		kbd_led(HIDLED_TO_PS2LED(led));
}
