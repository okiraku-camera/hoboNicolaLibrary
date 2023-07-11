#include "Arduino.h"
/**
 * 
	output_string.h  Definitions of output strings  of "Hobo-nicola keyboard and adapter library".
	Copyright (c) 2021 Takeshi Higasa

	This file is part of "Hobo-nicola keyboard and adapter library".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	Included in hoboNicola 1.5.0.		Aug. 10. 2021.

	ローマ字として送信する文字列を pgmに保持する。
	このヘッダは出力テーブルの直前にのみincludeすること。
*/

#ifndef __OUTPUT_STRINGS_H__
#define __OUTPUT_STRINGS_H__

//
// 出力する文字列（ローマ字シーケンス）の定義
// 1文字ずつ char_to_hid() してHIDコード化してから送信する。
// 先頭バイトが0x01ならば引き続くバイトはHIDコード。
// 
static const uint8_t empty_str[] PROGMEM = "";

static const uint8_t s_1[] PROGMEM = "1";
static const uint8_t s_2[] PROGMEM = "2";
static const uint8_t s_3[] PROGMEM = "3";
static const uint8_t s_4[] PROGMEM = "4";
static const uint8_t s_5[] PROGMEM = "5";
static const uint8_t s_6[] PROGMEM = "6";
static const uint8_t s_7[] PROGMEM = "7";
static const uint8_t s_8[] PROGMEM = "8";
static const uint8_t s_9[] PROGMEM = "9";
static const uint8_t s_0[] PROGMEM = "0";

static const uint8_t s_excl[] PROGMEM	 = "!";
static const uint8_t s_dquot[] PROGMEM	 = "\"";
static const uint8_t s_hash[] PROGMEM 	 = "#";
static const uint8_t s_doll[] PROGMEM	 = "$";  // 「
static const uint8_t s_perct[] PROGMEM = "%";  // 」
static const uint8_t s_amp[] PROGMEM 	 = "&";  // [
static const uint8_t s_quot[] PROGMEM 	 = "'";  // ]
static const uint8_t s_less[] PROGMEM    = "<";
static const uint8_t s_gt[] PROGMEM 	 = ">";
static const uint8_t s_minus[] PROGMEM   = "-";
static const uint8_t s_equ[] PROGMEM  = "=";
static const uint8_t s_plus[] PROGMEM = "+";
static const uint8_t s_tild[] PROGMEM   = "~";
static const uint8_t s_pipe[] PROGMEM    = "|";
static const uint8_t s_bslas[] PROGMEM  = "\\";

static const uint8_t s_peri[] PROGMEM  = ".";  //ピリオドと句点は区別できない
static const uint8_t s_comm[] PROGMEM   = ",";  // カンマと読点も区別できない
static const uint8_t s_colon[] PROGMEM   = ":";
static const uint8_t s_acgra[] PROGMEM = "`"; // back-q
static const uint8_t s_aste[] PROGMEM 	 = "*";
static const uint8_t s_quest[] PROGMEM 	 = "?";
static const uint8_t s_slash[] PROGMEM   = "/";  // スラッシュと中黒は区別できない
static const uint8_t s_l_brac[] PROGMEM = "[";
static const uint8_t s_r_brac[] PROGMEM = "]";
static const uint8_t s_l_pren[] PROGMEM = "(";
static const uint8_t s_r_pren[] PROGMEM = ")";
static const uint8_t s_l_cbrac[] PROGMEM = "{";
static const uint8_t s_r_cbrac[] PROGMEM = "}";
static const uint8_t s_uline[] PROGMEM 	  = "_";
static const uint8_t s_at[] PROGMEM				= "@";
static const uint8_t s_carret[] PROGMEM		= "^";

static const uint8_t s_a[] PROGMEM = "a";  // あ
static const uint8_t s_i[] PROGMEM = "i";
static const uint8_t s_u[] PROGMEM = "u";
static const uint8_t s_e[] PROGMEM = "e";  // え
static const uint8_t s_o[] PROGMEM = "o";  // お

static const uint8_t s_xa[] PROGMEM = "xa"; // ぁ
static const uint8_t s_xi[] PROGMEM = "xi"; // ぃ
static const uint8_t s_xe[] PROGMEM = "xe";  // ぇ
static const uint8_t s_xu[] PROGMEM = "xu"; // ぅ
static const uint8_t s_xo[] PROGMEM = "xo";  // ぉ

static const uint8_t s_ka[] PROGMEM = "ka";
static const uint8_t s_ki[] PROGMEM = "ki";
static const uint8_t s_ku[] PROGMEM = "ku";
static const uint8_t s_ke[] PROGMEM = "ke";
static const uint8_t s_ko[] PROGMEM = "ko";

static const uint8_t s_ga[] PROGMEM = "ga"; // が
static const uint8_t s_gi[] PROGMEM = "gi"; // ぎ
static const uint8_t s_gu[] PROGMEM = "gu"; // ぐ
static const uint8_t s_ge[] PROGMEM = "ge"; // げ
static const uint8_t s_go[] PROGMEM = "go"; // ご

static const uint8_t s_sa[] PROGMEM = "sa";
static const uint8_t s_si[] PROGMEM = "si";
static const uint8_t s_su[] PROGMEM = "su";
static const uint8_t s_se[] PROGMEM = "se";
static const uint8_t s_so[] PROGMEM = "so";

static const uint8_t s_za[] PROGMEM = "za"; // ざ
static const uint8_t s_ji[] PROGMEM = "ji"; // じ
static const uint8_t s_zu[] PROGMEM = "zu"; // ず
static const uint8_t s_ze[] PROGMEM = "ze"; // ぜ
static const uint8_t s_zo[] PROGMEM = "zo"; // ぞ


static const uint8_t s_ta[] PROGMEM = "ta";
static const uint8_t s_ti[] PROGMEM = "ti";
static const uint8_t s_tu[] PROGMEM = "tu";
static const uint8_t s_te[] PROGMEM = "te";
static const uint8_t s_to[] PROGMEM = "to";

static const uint8_t s_xtu[] PROGMEM = "xtu"; // っ

static const uint8_t s_da[] PROGMEM = "da"; // だ
static const uint8_t s_di[] PROGMEM = "di"; // ぢ
static const uint8_t s_du[] PROGMEM = "du"; // づ
static const uint8_t s_de[] PROGMEM = "de"; // で
static const uint8_t s_do[] PROGMEM = "do"; // ど

static const uint8_t s_na[] PROGMEM = "na"; // な
static const uint8_t s_ni[] PROGMEM = "ni"; // に
static const uint8_t s_nu[] PROGMEM = "nu";
static const uint8_t s_ne[] PROGMEM = "ne";
static const uint8_t s_no[] PROGMEM = "no";

static const uint8_t s_ha[] PROGMEM = "ha";
static const uint8_t s_hi[] PROGMEM = "hi";
static const uint8_t s_fu[] PROGMEM = "fu";
static const uint8_t s_he[] PROGMEM = "he";
static const uint8_t s_ho[] PROGMEM = "ho";

static const uint8_t s_ba[] PROGMEM = "ba"; // ば
static const uint8_t s_bi[] PROGMEM = "bi"; // び
static const uint8_t s_bu[] PROGMEM = "bu"; // ぶ
static const uint8_t s_be[] PROGMEM = "be"; // べ
static const uint8_t s_bo[] PROGMEM = "bo"; // ぼ

static const uint8_t s_pa[] PROGMEM = "pa";  // パ
static const uint8_t s_pi[] PROGMEM = "pi";
static const uint8_t s_pu[] PROGMEM = "pu";
static const uint8_t s_pe[] PROGMEM = "pe";   
static const uint8_t s_po[] PROGMEM = "po";

static const uint8_t s_ma[] PROGMEM = "ma"; // ま
static const uint8_t s_mi[] PROGMEM = "mi"; // み
static const uint8_t s_mu[] PROGMEM = "mu"; // む
static const uint8_t s_me[] PROGMEM = "me";
static const uint8_t s_mo[] PROGMEM = "mo"; // も

static const uint8_t s_ya[] PROGMEM = "ya"; // や
static const uint8_t s_yu[] PROGMEM = "yu"; // ゆ
static const uint8_t s_yo[] PROGMEM = "yo"; // よ
static const uint8_t s_xya[] PROGMEM = "xya";  // ゃ  
static const uint8_t s_xyu[] PROGMEM = "xyu";  // ゅ
static const uint8_t s_xyo[] PROGMEM = "xyo";  // ょ

static const uint8_t s_ra[] PROGMEM = "ra";
static const uint8_t s_ri[] PROGMEM = "ri"; // り
static const uint8_t s_ru[] PROGMEM = "ru"; // る
static const uint8_t s_re[] PROGMEM = "re"; // れ
static const uint8_t s_ro[] PROGMEM = "ro"; // ろ

static const uint8_t s_wa[] PROGMEM = "wa"; // わ
static const uint8_t s_wo[] PROGMEM = "wo"; // を
static const uint8_t s_vu[] PROGMEM = "vu"; // ヴ
static const uint8_t s_nn[] PROGMEM = "xn"; // ん nn

static const uint8_t s_larrow[] PROGMEM = {HID_DIRECT_PREFIX, HID_L_ARROW, 0};   // D  ←
static const uint8_t s_rarrow[] PROGMEM = {HID_DIRECT_PREFIX, HID_R_ARROW, 0};   // F  →
static const uint8_t s_uarrow[] PROGMEM = {HID_DIRECT_PREFIX, HID_U_ARROW, 0};     // K  ↑
static const uint8_t s_darrow[] PROGMEM = {HID_DIRECT_PREFIX, HID_D_ARROW, 0};      // J  ↓
static const uint8_t s_delete[] PROGMEM = {HID_DIRECT_PREFIX, HID_DELETE, 0};     // N Delete
static const uint8_t s_enter[] PROGMEM = {HID_DIRECT_PREFIX, HID_ENTER, 0};      // M Enter

#endif 	// __OUTPUT_STRINGS_H__
