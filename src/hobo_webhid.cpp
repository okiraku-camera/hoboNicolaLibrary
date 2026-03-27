/**
 * hobo_webhid.cpp HID raw request handling part of hoboNicola library for webhid.
 * Copyright (c)  Takeshi Higasa, okiraku-camera.tokyo
 * 
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 * 
 * this file is added for hoboNicola 1.8.0 to handle raw HID requests from webhid client.
 * nvm keymap related functions are stubbed out in this file, as they are not used in this version.
 */


#include "Arduino.h"
#include "hobo_nicola.h"
//#include "nvm_keymap.h"	

// extern void enter_bootloader_mode();

#define VERSION_NUM 0x1800

#define GET_VERSION		1
#define READ_NVM_BLOCK		0x0a
#define STARTBOOT_LOADER	0x0c

#define RESET_KEYMAP	0x10
#define READ_KEY		0x11
#define WRITE_KEY		0x12
#define READ_KEYMAP		0x15
#define WRITE_KEYMAP	0x16
#define READ_MACROS		0x18
#define WRITE_MACROS	0x19
#define WRITE_RE_KEYS	0x1c
#define READ_RE_KEYS	0x1d

#define SET_NICOLA_PARAMETER		0x20
#define GET_NICOLA_PARAMETER		0x21


//
int8_t raw_input(uint8_t* data) {
	uint16_t bytes = 0;
	if (data) {
		switch(*data) {
		case RESET_KEYMAP:		
		// 	reset nvm keymap and return version number.
			// reset_nvm();
			// through
		case GET_VERSION:
			data[1] = VERSION_NUM & 0xFF;  // lower
			data[2] = (VERSION_NUM >> 8) & 0xFF;  // upper
			bytes = 2;
			break;
		case GET_NICOLA_PARAMETER:
			// data[1] onwards : buffer to store parameter values as int16_t array.
			bytes = HoboNicola::get_nicola_parameter((uint8_t*)&data[1], (uint16_t*)&data[2]) ;
			break;
		case SET_NICOLA_PARAMETER:
			// data[1] : parameter count, data[2] onwards : buffer containing parameter value (4 bytes).
			HoboNicola::set_nicola_parameter(data[1], (uint16_t*)&data[2]);
			bytes = 1; // number of bytes written.
			break;
		case READ_NVM_BLOCK:
			// read nvm to buffer. data[1] : start address, data[2] : number of bytes, data[3] onwards : buffer to store read bytes.
			bytes = read_nvm_block(data[1], data[2], (uint8_t*)&data[2]);
			data[1] = bytes; // number of bytes read.
			break;
#if 0	
		case READ_KEY:		
		// read single keycode from nvm 
		// data[1] : switch no, data[2] : layer no,
			bytes = read_nvm_code(data[1], data[2]);
			data[1] = bytes & 0xFF; // lower
			data[2] = (bytes >> 8) & 0xFF; // upper
			bytes = 2;
			break;
		case WRITE_KEY:		
		// write single keycode to nvm.
		// data[1] : switch no, data[2] : layer no, data[3] : lower byte, data[4] : upper byte
			write_nvm_code(data[1], data[2], (uint16_t)(data[3] + (data[4] << 8))); 
			data[1] = 1;
			bytes = 1;
			break;
		case READ_KEYMAP:	
		// read keymap of nvm.
		// data[1] : start switch no, data[2] : number of switches, data[3] : layer no, keycodes are stored after data[4].
			bytes = read_keys(data[1], data[2], data[3], (uint16_t*)&data[4]);
			data[1] = bytes; // number of switches read.
			bytes = 1;
			break;
		case WRITE_KEYMAP:
		// write keymap to nvm.
			bytes = write_keys(data[1], data[2], data[3], (uint16_t*)&data[4]); // 1 key = 2 bytes
			data[1] = bytes; // number of switches written. keycodes are stored after data[4].
			break;
		case READ_MACROS:
			// data[1] : start address (0~), data[2] : number of bytes, data[3] onwards : macro code string. 
			bytes = read_macros(data[1], data[2], (char*)&data[3]); 
			data[1] = bytes; // number of bytes read. macro code string is stored after data[3].
			break;
		case WRITE_MACROS:
			// write macro code. data[1] : start index, data[2] : number of bytes, data[3] onwards : macro code string.
			bytes = write_macros(data[1], data[2], (char*)&data[3]);
			data[1] = bytes; // number of bytes written.
			break;
		case STARTBOOT_LOADER:
			enter_bootloader_mode();
			break;
		case WRITE_RE_KEYS:
			// write RE keys. data[1] : RE number, data[2] : layer number, data[3] : buffer address containing 16-bit codes in CW, CCW order
			bytes = write_re_keys(data[1], data[2], (uint16_t*)&data[3]);
			data[1] = bytes; // number of bytes written.
			break;
		case READ_RE_KEYS:
			// read RE keys. data[1] : RE number, data[2] : layer number, data[3] : buffer address to store 16-bit codes in CW, CCW order
			bytes = read_re_keys(data[1], data[2], (uint16_t*)&data[3]);
			data[1] = bytes; // number of bytes read.
			break;
#endif
		default:
			data[1] = 0xde;  // 8-bit integer
			data[2] = 0xad;  // 8-bit integer
			bytes = 2;
			break;
		}
	}
	return bytes;
}
