/**
 * 
  xd60.h   NK60 (rev.031) PCB interface of "Hobo-nicola keyboard and adapter library".
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
  
  Included in hoboNicola 1.7.0		Feb. 1. 2023.
	
*/

#ifndef __NK60__H__
#define __NK60__H__

void init_nk60();
void matrix_scan();
uint8_t nk60_get_key(bool& pressed, bool us_layout);
void nk60_table_change(uint8_t key, bool pressed);

void CAPSLED(uint8_t on);
void BGLED(uint8_t on);
void led_sleep() ;

#endif
