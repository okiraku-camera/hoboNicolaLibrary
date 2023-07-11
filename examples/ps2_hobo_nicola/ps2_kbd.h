/**
 * 
  ps2_kbd.h  PS/2 Keyboard interface of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
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

	Included in hoboNicola 1.6.4.		Feb. 1. 2023.

*/
#include <Arduino.h>

#define PS2_DATA    5
#define PS2_CLK     3
// PRO MINI(328P)のD3はINT1だが、PRO MICRO(32u4)ではINT0
// #define CLK_INT     1
#define CLK_INT     0  

typedef enum { None = 0, Idle, Receiving, WaitForStart, Sending }  bus_state_t;

#define PS2_LED_CAPSLOCK    4
#define PS2_LED_NUMLOCK 2
#define PS2_LED_SCRLOCK 1

class keyboard_notify {
public:
	virtual void key_pressed(uint8_t k, uint8_t mod) {}; // notify key pressed with hid usage id.
	virtual void key_released(uint8_t k, uint8_t mod) {}; // notify key released with hid usage id.
	virtual void modifier_changed(uint8_t before, uint8_t after) {}; // notify any modifier-key is changed.
	virtual void raw_key_event(uint8_t scan, bool released = false) {};  // notify all key event with raw scancode.
	virtual void key_released_all() {}; // all keys are released.
};
class ps2_kbd {
public:
	static ps2_kbd* getInstance();
	bool begin(keyboard_notify* p = 0, uint8_t led = 0xff);
	void task(uint8_t led = 0);  // do everything.
	bool send_command(uint8_t cmd, uint8_t count = 1, uint8_t* resp = 0);
	bool kbd_reset();
	bool kbd_led(uint8_t led);
	void toggle_led(uint8_t led);

	void toggle_capslock_led() { toggle_led(PS2_LED_CAPSLOCK); }
	void toggle_numlock_led() { toggle_led(PS2_LED_NUMLOCK); }
	void toggle_scrlock_led() { toggle_led(PS2_LED_SCRLOCK); }
	bool is_capslock_led() const { return (kbd_led_state & PS2_LED_CAPSLOCK); }
	bool is_numlock_led() const { return (kbd_led_state & PS2_LED_NUMLOCK); }
	bool is_scrlock_led() const { return (kbd_led_state & PS2_LED_SCRLOCK); }

private:
	ps2_kbd();
	uint8_t kbd_led_state;
	uint8_t modifier_state;
	uint8_t last_modifier_state;

	static void clk_interrupt();
	void show_error(int ms);

	bool set_modifier_state(uint8_t k, bool released);
	uint8_t hid_code(uint8_t k, bool& released);
	keyboard_notify* notifier;
};
