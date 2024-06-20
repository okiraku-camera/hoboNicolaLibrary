/**
  hobo_led.h  On board LED configs of "Hobo-nicola usb/ble adapter".
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

#if !defined(__HOBO_LED__H__)
#define __HOBO_LED__H__

#include "Arduino.h"

void all_led_off();

void led_nicola(bool on);
void led_error(bool on);
void led_toggle_nicola();
void led_toggle_ready();
void led_ready(bool on);

void neo_pixel_begin();
void neo_pixel_end();
void ble_led(bool ble_connected);

#endif // __HOBO_LED__H__