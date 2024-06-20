/**
 * 
  xd_commons.cpp   XD64(ver3), XD87 PCB common functions of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2023 Takeshi Higasa
  
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
  
    Included in hoboNicola 1.7.0.		Mar. 3. 2023.  xd64 ver3 PCB support.
*/

#if defined(__AVR_ATmega32U4__)

#include <avr/io.h>
#include "Arduino.h"
#include "xd_commons.h"
#include "hobo_nicola.h"

static const uint8_t RGB_VAL_MAX = 0x7f; // ws2812 一個あたりで 約14mA消費する。
	
static int16_t rgb_val = 0;	// current data of ws2812
static int16_t last_rgb_val = 0;

static const uint8_t rgb_count = 0; // ws2812の実装数
static const volatile uint8_t *ws2812_port;
static uint8_t ws2812_pin_mask; 
static uint8_t* rgb_buffer = 0; // g r bの順に並ぶ
static uint8_t pixel_count = 0;

// ws2812用の初期設定
void set_xd_rgb_port(const uint8_t _count, const volatile uint8_t* port, uint8_t pin) {
	ws2812_pin_mask = _BV(pin); 
	ws2812_port = port;
	pixel_count = _count * 3;
	if (rgb_buffer) {
		free(rgb_buffer);
		rgb_buffer = 0;
	}
	rgb_buffer = (uint8_t*)malloc(pixel_count);
}

extern void ws2812_sendarray_mask(uint8_t *array,uint16_t length, uint8_t pinmask,uint8_t *port);
// すべて同色
void xd_update_rgb(uint8_t val) {
	if (!rgb_buffer)
		return;
	memset(rgb_buffer, val,  pixel_count );  
	ws2812_sendarray_mask(rgb_buffer, pixel_count, ws2812_pin_mask, (uint8_t*)ws2812_port);	
}
/**
 * @brief apply current rgb brightness (rgb_val) to each pixels and save it.
 *       when current data != previous data.
*/
void xd_rgb_sync(bool test) {
	if (test && last_rgb_val == rgb_val)
		return;
	xd_update_rgb(rgb_val);
	pSettings->save_xd_rgb_value(rgb_val);
	last_rgb_val = rgb_val;
}

#define rgb_unit 4
void xd_rgb_dimmer(bool set_off) {
	if (set_off)
		rgb_val = 0;
	else {
		rgb_val -= rgb_unit;
		if (rgb_val < 0) rgb_val = 0;
	}
	xd_rgb_sync();
}

void xd_rgb_brighter(bool set_max) {
	if (set_max)
		rgb_val = RGB_VAL_MAX;
	else {
		rgb_val += rgb_unit;
		if (rgb_val > RGB_VAL_MAX) rgb_val = RGB_VAL_MAX;
	}
	xd_rgb_sync();
}

void set_xd_rgb_value(const int16_t val) { rgb_val = val; }
const int16_t get_xd_rgb_value() { return rgb_val; }

//  fifo key buffer 
static const uint8_t KBD_BUFFER_SIZE = 16;
static uint8_t kbd_buffer[KBD_BUFFER_SIZE]; 
static volatile uint8_t kbd_in = 0;
static volatile uint8_t kbd_out = 0;

void xd_clear_buffer() {
	uint8_t save_sreg = SREG;
	cli();  
	kbd_in = kbd_out = 0;
	SREG = save_sreg;
}

bool xd_put_buffer(uint8_t s) {
	uint8_t rx = kbd_in + 1;
	if (rx >= KBD_BUFFER_SIZE) 
		rx = 0;
	if (rx == kbd_out) 
		return false; // buffer is full.  
	kbd_buffer[kbd_in] = s;
	kbd_in = rx;    
	return true;  
}

uint8_t xd_get_buffer() {
	if (kbd_in == kbd_out)
		return 0;  // empty.
	uint8_t s = kbd_buffer[kbd_out];
	cli();
	if (++kbd_out >= KBD_BUFFER_SIZE)
		kbd_out = 0;
	sei();    
	return s;    
}

// Start Atmel DFU bootloader
void start_dfu_bootloader() {
	cli();
	delay(20);
	UDCON = 1;
	USBCON = (1<<FRZCLK);
	delay(20);
	TIMSK0 = TIMSK1 = 0; 
	DDRB = DDRC = DDRD = DDRE = DDRF = 0; 
	PORTB = PORTC = PORTD = PORTE = PORTF = 0;
	asm volatile("jmp 0x7000");
	for(;;) {}  // never return
}

#endif
