/**
 * 
  xd87.h   XD87 PCB interface of "Hobo-nicola keyboard and adapter library".
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
  
  Included in hoboNicola 1.6.4.		Feb. 1. 2023.
	
*/

#include "xd_commons.h"

#ifndef __XD87__H__
#define __XD87__H__

void init_xd87();
void matrix_scan();
uint8_t xd87_get_key(bool& pressed, bool us_layout);
void xd87_table_change(uint8_t key, bool pressed);

#endif
