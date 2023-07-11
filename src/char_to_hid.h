/**
 * 
  char_to_hid.h Definitions of character codes to HID Usage id converter.
  Copyright (c) 2021 Takeshi Higasa, okiraku-camera.tokyo
  
  This file is part of "Hobo-nicola keyboard and adapter".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
  
    Included in hoboNicola 1.5.0.		July. 31. 2021.
*/
#include "Arduino.h"

#ifndef __CHAR_TO_HID_H__
#define __CHAR_TO_HID_H__


#define WITH_SHIFT  0x100
extern uint16_t char_to_hid(uint8_t c, bool us = false);

#endif 