/**
 * 
  xd_commons.h   XD64(vers) and XD87 PCB common function definitions of "Hobo-nicola keyboard and adapter library".
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

#ifndef __XD_HOBO_NICOLA_H__
#define __XD_HOBO_NICOLA_H__
#include "hobo_nicola.h"

// xd64/87 extra functions.
static const uint8_t FN_XD_START_DFU	= FN_EXTRA_START;
static const uint8_t FN_RGB_BRIGHTER	= FN_EXTRA_START + 1;
static const uint8_t FN_RGB_DIMMER		= FN_EXTRA_START + 2;
static const uint8_t FN_RGB_MAX	      = FN_EXTRA_START + 3;
static const uint8_t FN_RGB_OFF		    = FN_EXTRA_START + 4;

void xd_clear_buffer();
bool xd_put_buffer(uint8_t s);
uint8_t xd_get_buffer();

void set_xd_rgb_port(const uint8_t _count, const volatile uint8_t* port, uint8_t pin) ;    
void set_xd_rgb_value(const int16_t val);
const int16_t get_xd_rgb_value();
void xd_rgb_dimmer(bool set_off = false);
void xd_rgb_brighter(bool set_max = false);
void xd_rgb_sync(bool test = true);
void xd_update_rgb(uint8_t val) ;
void start_dfu_bootloader();

#endif
