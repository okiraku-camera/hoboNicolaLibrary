/**
 * 
  hobo_settings.cpp  small data storage functionalities of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2022 Takeshi Higasa, okiraku-camera.tokyo
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
  
  hoboNicola 1.7.6.	Mar. 8. 2024	
*/

#include "Arduino.h"
#include "hobo_settings.h"

_Settings* _Settings::pInstance = 0;

static const uint16_t  SETTINGS_ADDR =	0;
static const uint16_t  EXTRA_ADDR	= 8;
static const uint16_t  COUNTER_ADDR	= 16;

static const int8_t SET_COUNT = 3;
static const uint16_t  SET_ADDR[]	= { 32, 40, 48 };

typedef struct {
//	uint8_t _m[32];
	uint8_t _m[64];
} nvm_data_t;

static nvm_data_t __attribute__((__aligned__(8))) nvm_data;

uint32_t _Settings::_size() { return sizeof(nvm_data_t)	; }
void _Settings::_write(uint16_t  addr, uint32_t value) { 
	*((uint32_t*)&nvm_data._m[addr]) = value;
	*((uint32_t*)&nvm_data._m[addr+4]) = ~value;
}

uint32_t _Settings::get_at(uint16_t  addr) { 
	uint32_t a = *((uint32_t*)&nvm_data._m[addr]);
	uint32_t b = *((uint32_t*)&nvm_data._m[addr+4]);
	if (a != ~b) a = 0;
	return a;
}

void _Settings::set_at(uint16_t  addr, uint32_t data) {
	_write(addr, data);
	_write(COUNTER_ADDR, ++flush_count);
	flush();
}

void _Settings::load() {
	settings = get_at(SETTINGS_ADDR);
	extra_settings = get_at(EXTRA_ADDR);
	flush_count = get_at(COUNTER_ADDR);
}

void _Settings::save(uint32_t new_set) {
	if (new_set == settings)
		return ;
	set_at(SETTINGS_ADDR, new_set);			
	settings = new_set;
}

void _Settings::save_extra(uint32_t new_extra) {
	if (new_extra == extra_settings)
		return ;
	set_at(EXTRA_ADDR, new_extra);			
	extra_settings = new_extra;
}

void _Settings::save_xd_rgb_value(uint8_t val) { save_extra((extra_settings & 0xffffff00) | val); }
void _Settings::save_rp_pwm_max_value(int16_t val) { save_extra((extra_settings & 0xffff0000) | val); }


// 現在のsettingsの値を指定のスロットに保存する。
void _Settings::save_set(int8_t index) {
	if (index <  0 || index >= SET_COUNT)
		return ;
	set_at(SET_ADDR[index], settings);
}

// 指定スロットの値を返す。
uint32_t _Settings::load_set(int8_t index) {
	if (index < 0 || index >= SET_COUNT)
		return 0;
	return get_at(SET_ADDR[index]);
}

// AVR ATmega32u_4
#if defined(ARDUINO_ARCH_AVR)
#include "EEPROM.h"
static const uint16_t EE_BASEADDR = 0;

void _Settings::begin() { EEPROM.get(EE_BASEADDR, nvm_data); }
void _Settings::flush() { EEPROM.put(EE_BASEADDR, nvm_data); }

#elif defined(ARDUINO_ARCH_RP2040)
#include "EEPROM.h"	// Arduino-Pico core eeprom library
static const uint16_t EE_BASEADDR = 0;
void _Settings::begin() {
	EEPROM.begin(256);
	EEPROM.get(EE_BASEADDR, nvm_data); 
}
void _Settings::flush() { 
	EEPROM.put(EE_BASEADDR, nvm_data); 
	EEPROM.commit();
}

// samd21
#elif defined(ARDUINO_ARCH_SAMD)
#include "FlashStorage/FlashStorage.h"
// https://www.arduino.cc/reference/en/libraries/flashstorage/
FlashStorage(flash_store, nvm_data_t);

void _Settings::begin() {
	memset(&nvm_data, 0, sizeof(nvm_data_t));
  flash_store.read(&nvm_data);
}
void _Settings::flush() {  flash_store.write(nvm_data); }

#elif defined(ARDUINO_NRF52_ADAFRUIT)

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;
static const char filename[] = "private.dat";
static File ifs(InternalFS);

void _Settings::begin() {
	memset(&nvm_data, 0, sizeof(nvm_data_t));
	InternalFS.begin();
	ifs.open(filename, FILE_O_READ);
	if (ifs) {
		ifs.read(&nvm_data, sizeof(nvm_data_t));
		ifs.close();
	}
}
void _Settings::flush() { 
	if (ifs.open(filename, FILE_O_WRITE)) {
		ifs.seek(LFS_SEEK_SET);
		ifs.write((uint8_t const *)&nvm_data, sizeof(nvm_data_t));
		ifs.close();
	}
}

#else
	#error "un-supported board."
#endif

// 
