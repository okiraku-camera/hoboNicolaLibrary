/**
  hobo_led.cpp  On board LED configs of "Hobo-nicola usb/ble adapter".
  Copyright (c) 2022 Takeshi Higasa

  This file is part of "Hobo-nicola keyboard and adapter".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hobo_led.h"
#include "hobo_board_config.h"

#ifdef USE_NEOPIXEL
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel neo_pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
static uint32_t neo_pixel_color; // rgb composite value.
static const uint8_t neo_pixel_brt = 32; // brightness (0 - 255)
static const uint8_t neo_pixel_index = 0;

static bool neo_pixel_init = false;
void neo_pixel_begin() {
#if defined(PIN_NEOPIXEL_VCC)	
	digitalWrite(PIN_NEOPIXEL_VCC, 1);
#endif
	neo_pixel.begin();
	neo_pixel_color = 0;
	neo_pixel.setPixelColor(neo_pixel_index, neo_pixel_color);
	neo_pixel.show();
	neo_pixel_init = true;
}

void neo_pixel_end() {
#if defined(PIN_NEOPIXEL_VCC)	
	digitalWrite(PIN_NEOPIXEL_VCC, 0);
	neo_pixel_init = false;
#endif
}

static const int neo_pixel_r_pos = 16;
static const int neo_pixel_g_pos = 8;
static const int neo_pixel_b_pos = 0;

static void neo_pixel_ctl(bool on, int pos) {
	if (!neo_pixel_init)
		neo_pixel_begin();

	if (on)
		neo_pixel_color |= (neo_pixel_brt << pos);
	else
		neo_pixel_color &= ~(0xff << pos);
	neo_pixel.setPixelColor(neo_pixel_index, neo_pixel_color);
	neo_pixel.show();
}

static void neo_pixel_toggle(int pos) {
	if ((neo_pixel_color & (0xff << pos)) != 0)
		neo_pixel_ctl(false, pos);
	else
		neo_pixel_ctl(true, pos);
}
static void neo_pixel_red(bool on) { neo_pixel_ctl(on, neo_pixel_r_pos); }
static void neo_pixel_green(bool on) { neo_pixel_ctl(on, neo_pixel_g_pos); }
static void neo_pixel_blue(bool on) { neo_pixel_ctl(on, neo_pixel_b_pos); }
static void neo_pixel_toggle_green() { neo_pixel_toggle(neo_pixel_g_pos); }
#else	// USE_NEOPIXEL
	void neo_pixel_begin() {}
	void neo_pixel_end() {}
#endif


#if defined(BLE_LED) && defined(NRF52_SERIES)
static const unsigned long bleled_interval = 50;
static unsigned long bleled_timing = 0;
static const int16_t bleled_delta = 8;
static const int16_t bleled_max = 256; 
static const int16_t bleled_min = 128;

static int16_t dir = -1;
static int16_t bleled_value = bleled_min;

void ble_led(bool ble_connected) {
	if (!ble_connected) {
		if (bleled_value != bleled_max) {
			bleled_value = bleled_max;
			analogWrite(BLE_LED, bleled_max);
			digitalWrite(BLE_LED, LED_OFF);  // 一度analogWrite()するとdigitalWrite()で消えないような。
		}
		return;
	}
	unsigned long now = millis();
	if (bleled_timing == 0)
		bleled_timing = now;
	else if (now - bleled_timing > bleled_interval) {
		analogWrite(BLE_LED, bleled_value);
		bleled_timing = now;
		bleled_value += (bleled_delta * dir);
		if (bleled_value >= bleled_max)
			dir = -1;
		else if (bleled_value <= bleled_min)
			dir = 1;
	}
}
#else
	void ble_led(bool f) {}
#endif

void all_led_off() {
#ifdef USE_NEOPIXEL
	neo_pixel_end();
#endif
	led_nicola(false);
	led_error(false);
#if defined(BLE_LED)
	digitalWrite(BLE_LED, LED_OFF);
#endif
#if defined(READY_LED)
	digitalWrite(READY_LED, LED_OFF);
#endif

}


#ifdef USE_NEOPIXEL  
	void led_nicola(bool on) { neo_pixel_green(on); }
	void led_toggle_nicola() { neo_pixel_toggle_green(); }
	void led_error(bool on) { neo_pixel_red(on);}
	void led_ready(bool on) {}
	void led_toggle_ready()  {}
#else
	void led_nicola(bool on) { digitalWrite(LED1, on ? LED_ON : LED_OFF); }
	void led_toggle_nicola() { digitalWrite(LED1, digitalRead(LED1) ^ 1); }
	void led_error(bool on) { digitalWrite(LED2, on ? LED_ON : LED_OFF); }
	#if defined(READY_LED)
		void led_ready(bool on) { digitalWrite(READY_LED, on ? LED_ON : LED_OFF);}
		void led_toggle_ready()  { digitalWrite(READY_LED, digitalRead(READY_LED) ^ 1);}
	#else
		void led_ready(bool on) {}
		void led_toggle_ready() {}
	#endif
#endif
