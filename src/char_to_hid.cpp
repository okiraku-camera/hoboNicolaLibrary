/**
 * 
	char_to_hid.cpp Convert tables and functions of ASCII code to HID usage ID. 
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
#include "hid_keycode.h"
#include "char_to_hid.h"

//
// chart to hid usage id
// 文字コード - 0x20 でテーブルを引くことで、hid usage id を得る。
// SHIFTが必要なのものは、0x100をORして返す。
// 指定の文字をキーボードから入力するとき、何キーを押せばいいのか？的な機能。

// 日本語レイアウト用
static const uint16_t char_to_hid_table_jp[] PROGMEM = { 
	HID_SPACE,      //  ' '
	HID_1|WITH_SHIFT,    // !
	HID_2|WITH_SHIFT,    // "
	HID_3|WITH_SHIFT,    // #
	HID_4|WITH_SHIFT,    // $
	HID_5|WITH_SHIFT,    // %
	HID_6|WITH_SHIFT,    // &
	HID_7|WITH_SHIFT,    // '
	HID_8|WITH_SHIFT,    // (
	HID_9|WITH_SHIFT,    // )
	HID_J_COLON|WITH_SHIFT,    // *
	HID_SEMICOLON|WITH_SHIFT,    // +
	HID_COMMA,          // ,
	HID_MINUS,          // -
	HID_PERIOD,          // .
	HID_SLASH,          // /
	HID_0,          // 0
	HID_1,          // 1
	HID_2,          // 2
	HID_3,          // 3
	HID_4,          // 4
	HID_5,          // 5
	HID_6,          // 6
	HID_7,          // 7
	HID_8,          // 8
	HID_9,          // 9
	HID_J_COLON,   // :
	HID_SEMICOLON,     // ;
	HID_COMMA|WITH_SHIFT,   // <
	HID_MINUS|WITH_SHIFT,   // =
	HID_PERIOD|WITH_SHIFT,   // >
	HID_SLASH|WITH_SHIFT,   // ?
	HID_J_AT,     // @
	HID_A|WITH_SHIFT,   // A
	HID_B|WITH_SHIFT,   // B
	HID_C|WITH_SHIFT,   // C
	HID_D|WITH_SHIFT,   // D
	HID_E|WITH_SHIFT,   // E
	HID_F|WITH_SHIFT,   // F
	HID_G|WITH_SHIFT,   // G
	HID_H|WITH_SHIFT,   // H
	HID_I|WITH_SHIFT,   // I
	HID_J|WITH_SHIFT,   // J
	HID_K|WITH_SHIFT,   // K
	HID_L|WITH_SHIFT,   // L
	HID_M|WITH_SHIFT,   // M
	HID_N|WITH_SHIFT,   // N
	HID_O|WITH_SHIFT,   // O
	HID_P|WITH_SHIFT,   // P
	HID_Q|WITH_SHIFT,   // Q
	HID_R|WITH_SHIFT,   // R
	HID_S|WITH_SHIFT,   // S
	HID_T|WITH_SHIFT,   // T
	HID_U|WITH_SHIFT,   // U
	HID_V|WITH_SHIFT,   // V
	HID_W|WITH_SHIFT,   // W
	HID_X|WITH_SHIFT,   // X
	HID_Y|WITH_SHIFT,   // Y
	HID_Z|WITH_SHIFT,   // Z
	HID_J_LBRACK,     // [
	HID_J_BSLASH,          // \   !!!!
	HID_J_RBRACK,          // ]
	HID_J_CFLEX,     // ^
	HID_J_UL|WITH_SHIFT,   // _  !!!!
	HID_J_AT|WITH_SHIFT,   // `
	HID_A,   // a
	HID_B,   // b
	HID_C,   // c
	HID_D,   // d
	HID_E,   // e
	HID_F,   // f
	HID_G,   // g
	HID_H,   // h
	HID_I,   // i
	HID_J,   // j
	HID_K,   // k
	HID_L,   // l
	HID_M,   // m
	HID_N,   // n
	HID_O,   // o
	HID_P,   // p
	HID_Q,   // q
	HID_R,   // r
	HID_S,   // s
	HID_T,   // t
	HID_U,   // u
	HID_V,   // v
	HID_W,   // w
	HID_X,   // x
	HID_Y,   // y
	HID_Z,   // z
	HID_J_LBRACK|WITH_SHIFT,   // {
	HID_J_BSLASH|WITH_SHIFT,   // |    !!!!
	HID_J_RBRACK|WITH_SHIFT,   // }
	HID_J_CFLEX|WITH_SHIFT    // ~
};

// USレイアウト用
static const uint16_t char_to_hid_table_us[] PROGMEM = { 
	HID_SPACE,			      //  ' '
	HID_1|WITH_SHIFT,    // !
	HID_QUOTE|WITH_SHIFT,    // "
	HID_3|WITH_SHIFT,    // #
	HID_4|WITH_SHIFT,    // $
	HID_5|WITH_SHIFT,    // %
	HID_7|WITH_SHIFT,    // &
	HID_QUOTE,    // '
	HID_9|WITH_SHIFT,    // (
	HID_0|WITH_SHIFT,    // )
	HID_8|WITH_SHIFT,    // *
	HID_EQUALS|WITH_SHIFT,    // +
	HID_COMMA,          // ,
	HID_MINUS,          // -
	HID_PERIOD,          // .
	HID_SLASH,          // /
	HID_0,          // 0
	HID_1,          // 1
	HID_2,          // 2
	HID_3,          // 3
	HID_4,          // 4
	HID_5,          // 5
	HID_6,          // 6
	HID_7,          // 7
	HID_8,          // 8
	HID_9,          // 9
	HID_SEMICOLON|WITH_SHIFT,   // :
	HID_SEMICOLON,     // ;
	HID_COMMA|WITH_SHIFT,   // <
	HID_EQUALS,   // =
	HID_PERIOD|WITH_SHIFT,   // >
	HID_SLASH|WITH_SHIFT,   // ?
	HID_2|WITH_SHIFT,     // @
	HID_A|WITH_SHIFT,   // A
	HID_B|WITH_SHIFT,   // B
	HID_C|WITH_SHIFT,   // C
	HID_D|WITH_SHIFT,   // D
	HID_E|WITH_SHIFT,   // E
	HID_F|WITH_SHIFT,   // F
	HID_G|WITH_SHIFT,   // G
	HID_H|WITH_SHIFT,   // H
	HID_I|WITH_SHIFT,   // I
	HID_J|WITH_SHIFT,   // J
	HID_K|WITH_SHIFT,   // K
	HID_L|WITH_SHIFT,   // L
	HID_M|WITH_SHIFT,   // M
	HID_N|WITH_SHIFT,   // N
	HID_O|WITH_SHIFT,   // O
	HID_P|WITH_SHIFT,   // P
	HID_Q|WITH_SHIFT,   // Q
	HID_R|WITH_SHIFT,   // R
	HID_S|WITH_SHIFT,   // S
	HID_T|WITH_SHIFT,   // T
	HID_U|WITH_SHIFT,   // U
	HID_V|WITH_SHIFT,   // V
	HID_W|WITH_SHIFT,   // W
	HID_X|WITH_SHIFT,   // X
	HID_Y|WITH_SHIFT,   // Y
	HID_Z|WITH_SHIFT,   // Z
	HID_LBRACK,     // [
	HID_BSLASH,          // \... 
	HID_RBRACK,          // ]
	HID_6|WITH_SHIFT,     // ^
	HID_MINUS|WITH_SHIFT,   // _ 
	HID_GRAVE_AC,   // `
	HID_A,   // a
	HID_B,   // b
	HID_C,   // c
	HID_D,   // d
	HID_E,   // e
	HID_F,   // f
	HID_G,   // g
	HID_H,   // h
	HID_I,   // i
	HID_J,   // j
	HID_K,   // k
	HID_L,   // l
	HID_M,   // m
	HID_N,   // n
	HID_O,   // o
	HID_P,   // p
	HID_Q,   // q
	HID_R,   // r
	HID_S,   // s
	HID_T,   // t
	HID_U,   // u
	HID_V,   // v
	HID_W,   // w
	HID_X,   // x
	HID_Y,   // y
	HID_Z,   // z
	HID_LBRACK|WITH_SHIFT,   // {
	HID_BSLASH|WITH_SHIFT,   // |    !!!!
	HID_RBRACK|WITH_SHIFT,   // }
	HID_GRAVE_AC|WITH_SHIFT    // ~
};

// 文字に対応するhid usage id を返す。
uint16_t char_to_hid(uint8_t c, bool us) {
	if (c < 0x20 || c > 0x7f) 
		return 0;
	const uint16_t* p = us ? char_to_hid_table_us : char_to_hid_table_jp;
	return pgm_read_word(&p[c - 0x20]);
}

