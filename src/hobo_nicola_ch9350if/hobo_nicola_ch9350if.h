/**
 * hobo_nicola_ch9350if.h CH9350L interface module of "Hobo-nicola keyboard and adapter library".
 * Copyright (c) 2022 Takeshi Higasa, okiraku-camera.tokyo
 *
  This file is part of "Hobo-nicola keyboard and adapter library".
 *
 * "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 *
 * "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
*/

#include "arduino.h"

#ifndef __HOBO_CH9350L_H__
#define __HOBO_CH9350L_H__

static const uint8_t keys_count = 8;     // key_array size of input report.
static const uint8_t report_size = keys_count + 2; // key_array and modifiers and reserved.
static const uint8_t data_size = report_size + 3; // (dev_kind, report, serial, sum)
static const uint8_t version_frame[] =   { 0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x00,0xFF,0xA0,0x07 };
static const uint8_t status_frame[] =    { 0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x00,0x81,0xAC,0x20 };

class ch9350if {
	static const uint8_t led_value_index = 7;

// ch9350_loop state and variables
	enum { has_none = 0, has_57, has_57ab, has_op, wait_data };
  uint8_t ch_rx_state;
  uint8_t ch_opcode;
  uint8_t ch_data_length;
  uint8_t ch_rx_length;
	
	uint8_t rst_port;
	uint8_t led_state;
	uint8_t reponse_frame[sizeof(status_frame)];
	int8_t composite_keyboard;
	uint8_t composite_report_id;

	bool sum_check(uint8_t datalen);
	void _init() {
		ch_rx_state = has_none;
 		ch_opcode = 0;
		ch_data_length = 0;
 		ch_rx_length = 0;
		composite_keyboard = -1;
		memset(prev_data, 0, data_size);	
	}
	void reinit();
public:
	ch9350if(uint8_t _rst_port = 0xff) {
		_init();
		composite_report_id = 1;
		led_state = 0;
		rst_port = _rst_port;
	  memcpy(reponse_frame, status_frame, sizeof(status_frame));
	};
	
	void ch9350_setup();
	void ch9350_loop();
	const void set_led_state(uint8_t led) { led_state = led; }
	uint8_t get_led_state() { return led_state; }

	void reset();
	void suspend();
	void resume();

	virtual void key_event(uint8_t hid_code, bool on) {};
	virtual void dataframe(uint8_t* data, uint8_t data_length)  {};
	virtual void newframe() {};
	virtual void rx_byte(uint8_t c) {};
	virtual bool modifiers_changed(uint8_t prev, uint8_t current) { return false; };

	void set_composite_report_id(uint8_t id = 0) { composite_report_id = id; }
protected:
	void parse_report(uint8_t datalen);
	uint8_t data_frame[data_size];
	uint8_t prev_data[data_size];
};

#endif // __HOBO_CH9350L_H__
