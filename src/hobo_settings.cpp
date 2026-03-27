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
  
  hoboNicola 1.8.0.	Mar. 22. 2026	
*/

#include "Arduino.h"
#include "hobo_settings.h"

_Settings* _Settings::pInstance = 0;


// アドレスマップ
// 0 : 現在の設定
// 8 : Extra設定
// 16 : カウンタ
// 32, 40, 48 : 設定スロット1, 2, 3
// 64 - : nicola同時打鍵パラメータ

//
// 設定を記憶するスロットは8バイトずつ3つある。
// カレントおよびExtra, counterは固定。
//
static const int8_t SET_COUNT = 3;
static const uint16_t  SET_ADDR[]	= { 32, 40, 48 };


// SAMD21のFlashStorageはコンストラクタの引数に構造体を要求するため定義している。
// 他のやり方もあるだろうが。
typedef struct {
	uint8_t m[128];
} nvm_data_t;

#if defined(ARDUINO_ARCH_SAMD)

static nvm_data_t  __attribute__((__aligned__(8), __packed__)) _nvm_data;
#define nvm_data _nvm_data.m	
#else
static uint8_t nvm_data[128] __attribute__((__aligned__(8)));
#endif


uint8_t read_nvm_block(uint8_t start, uint8_t count, uint8_t* buffer) {
	if (start + count > sizeof(nvm_data))
		return 0;
	memcpy(buffer, &nvm_data[start], count);
	return count;
}

uint32_t _Settings::_size() { return sizeof(nvm_data); }
void _Settings::_write(uint16_t  addr, uint32_t value) { 
	*((uint32_t*)&nvm_data[addr]) = value;
	*((uint32_t*)&nvm_data[addr+4]) = ~value;
}

uint32_t _Settings::get_at(uint16_t  addr) { 
	uint32_t a = *((uint32_t*)&nvm_data[addr]);
	uint32_t b = *((uint32_t*)&nvm_data[addr+4]);
	if (a != ~b) a = 0;
	return a;
}

uint16_t _Settings::get_at16(uint16_t  addr) { 
	uint16_t a = nvm_data[addr] + (nvm_data[addr+1] << 8);	
	uint16_t b = ~(nvm_data[addr+2] + (nvm_data[addr+3] << 8));	
	if (a != b) a = 0;
	return a;
}

void _Settings::save_at16(uint16_t  addr, uint16_t data, bool flush_now) {
	nvm_data[addr] = data & 0xff;
	nvm_data[addr+1] = (data >> 8) & 0xff;
	nvm_data[addr+2] = (~data) & 0xff;
	nvm_data[addr+3] = ((~data) >> 8) & 0xff;
	if (flush_now) {
		_write(COUNTER_ADDR, ++flush_count);
		flush();
	}
}


bool _Settings::check_at(uint16_t  addr) { 
	uint32_t a = *((uint32_t*)&nvm_data[addr]);
	uint32_t b = *((uint32_t*)&nvm_data[addr+4]);
	return a == ~b;
}

void _Settings::set_at(uint16_t  addr, uint32_t data, bool flush_now) {
	_write(addr, data);
	_write(COUNTER_ADDR, ++flush_count);
	if (flush_now)
		flush();
}

void _Settings::load() {
	extra_settings = get_at(EXTRA_ADDR);
	flush_count = get_at(COUNTER_ADDR);
	load_set(get_current_set_index());
}

void _Settings::save(uint32_t new_set) {
	if (new_set == settings)
		return ;
	save_set(get_current_set_index(), new_set);
}

void _Settings::save_extra(uint32_t new_extra) {
	if (new_extra == extra_settings)
		return ;
	set_at(EXTRA_ADDR, new_extra);			
	extra_settings = new_extra;
}

void _Settings::save_xd_rgb_value(uint8_t val) { save_extra((extra_settings & 0xffffff00) | val); }
void _Settings::save_rp_pwm_max_value(int16_t val) { save_extra((extra_settings & 0xffff0000) | val); }


// 設定値を現在アクティブなスロットに保存する。
// 保存した値はsettingsにも反映される。
void _Settings::save_set(int8_t index, uint32_t data) {
	if (index <  0 || index >= SET_COUNT)
		return ;
	settings = data;
	set_at(SETTINGS_ADDR, data, false);
	set_at(SET_ADDR[index], data);
}

// 指定スロットの値を返す。
uint32_t _Settings::load_set(uint8_t index) {
	uint32_t data;
	if (index >= SET_COUNT)
		data = get_at(SETTINGS_ADDR);
	else
		data = get_at(SET_ADDR[index]);
	settings = data;
	return settings;
}

// 現在アクティブなスロットのインデックスを返す。
uint8_t _Settings::get_current_set_index() {
	uint8_t data = (extra_settings >> 16) & 0x00ff; 
	return data;
}

// アクティブスロットのインデックスを返す。
void _Settings::save_current_set_index(uint8_t index) {
	if (index >= SET_COUNT)
		return ;
	uint32_t data = (uint32_t)index << 16;
	data &= 0x00ff0000;
	save_extra((extra_settings & 0xff00ffff) | data);
}

// AVR ATmega32u_4
#if defined(ARDUINO_ARCH_AVR)
#include "EEPROM.h"

void _Settings::begin() { EEPROM.get(EE_BASEADDR, nvm_data); }
void _Settings::flush() { EEPROM.put(EE_BASEADDR, nvm_data); }

#elif defined(ARDUINO_ARCH_RP2040)
#include "EEPROM.h"	// Arduino-Pico core eeprom library
//static const uint16_t EE_BASEADDR = 0;
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
	flash_store.read(&_nvm_data);
	if (!check_at(SETTINGS_ADDR) || !check_at(EXTRA_ADDR))
		memset(&_nvm_data, 0, sizeof(nvm_data_t));
}
void _Settings::flush() {  flash_store.write(_nvm_data); }

#elif defined(ARDUINO_NRF52_ADAFRUIT)

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;
static const char filename[] = "private.dat";
static File ifs(InternalFS);

void _Settings::begin() {
	memset(nvm_data, 0, sizeof(nvm_data));
	InternalFS.begin();
	ifs.open(filename, FILE_O_READ);
	if (ifs) {
		ifs.read(nvm_data, sizeof(nvm_data));
		ifs.close();
	}
}
void _Settings::flush() { 
	if (ifs.open(filename, FILE_O_WRITE)) {
		ifs.seek(LFS_SEEK_SET);
		ifs.write(nvm_data, sizeof(nvm_data));
		ifs.close();
	}
}

#else
	#error "un-supported board."
#endif

// 
