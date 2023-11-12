/**
 * 
	nicola_table.cpp   support tables for NICOLA-state-machine of "Hobo-nicola keyboard and adapter library".
	Copyright (c) 2018-2023 Takeshi Higasa, okiraku-camera.tokyo

	This file is part of "Hobo-nicola keyboard and adapter library".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

		hoboNicola 1.7.0. March.3. 2023
			from nicola_state.cpp, separating nicola table (JP and US).
*/
#include "hobo_nicola.h"
#include "hobo_settings.h"

// 入力テーブル HIDキーボードの入力コードから内部コードを得る。
// hid usage id(hid) --> internal code(nid)

// 日本語キーボード用
//	1 2 3 4 5 6 7 8 9 0 - ^ \... (行末が\ではいけません） 
//  q w e r t y u i o p @ [
//  a s d f g h j k l ; : ]
//  z x c v b n m , . / _
static const uint8_t hid_to_nid_jp[] = {
	HID_1, HID_2, HID_3, HID_4, HID_5, HID_6, HID_7, HID_8,     HID_9,      HID_0,       HID_MINUS,   HID_EQUALS,  HID_J_BSLASH,
	HID_Q, HID_W, HID_E, HID_R, HID_T, HID_Y, HID_U, HID_I,     HID_O,      HID_P,       HID_J_AT,    HID_J_LBRACK,
	HID_A, HID_S, HID_D, HID_F, HID_G, HID_H, HID_J, HID_K,     HID_L,      HID_SEMICOL, HID_J_COLON, HID_J_RBR_32,
	HID_Z, HID_X, HID_C, HID_V, HID_B, HID_N, HID_M, HID_COMMA, HID_PERIOD, HID_SLASH
};

// us layout.
//	1 2 3 4 5 6 7 8 9 0 - = \... // \は次の行の右端のキー
//  q w e r t y u i o p [ ]
//  a s d f g h j k l ; ' 
//  z x c v b n m , . /
// 存在しないキーは0にしておく。
static const uint8_t hid_to_nid_us[] = {
	HID_1, HID_2, HID_3, HID_4, HID_5, HID_6, HID_7, HID_8,     HID_9,      HID_0,       HID_MINUS,  HID_EQUALS,  HID_BSLASH,
	HID_Q, HID_W, HID_E, HID_R, HID_T, HID_Y, HID_U, HID_I,     HID_O,      HID_P,       HID_LBRACK, HID_RBRACK,
	HID_A, HID_S, HID_D, HID_F, HID_G, HID_H, HID_J, HID_K,     HID_L,      HID_SEMICOL, HID_QUOTE,  0,
	HID_Z, HID_X, HID_C, HID_V, HID_B, HID_N, HID_M, HID_COMMA, HID_PERIOD, HID_SLASH
};

// 出力文字列テーブル
// 同時打鍵ステートマシンの出力(内部コード)をホストに送信するためのローマ字文字列の配列
// hid_to_nid[]と同じ要素構成であること
#include "output_string.h"
#define s_nul	empty_str

// 1.7.0 １行目の記号の並びを変更。
static const uint8_t* const output_single[] PROGMEM = {	// 単独打鍵
//static const uint8_t* const output_single[] = {	// 単独打鍵
//1       2        3       4         5          6          7         8         9         10       11       12        13
	s_1,    s_2,     s_3,    s_4,      s_5,       s_6,       s_7,      s_8,      s_9,      s_0,     s_minus, s_carret, s_bslas,
	s_peri, s_ka,    s_ta,   s_ko,     s_sa,      s_ra,      s_ti,     s_ku,     s_tu,     s_comm,  s_comm,  s_nul,
	s_u,    s_si,    s_te,   s_ke,     s_se,      s_ha,      s_to,     s_ki,     s_i,      s_nn,    s_nul,   s_nul,
	s_peri, s_hi,    s_su,   s_fu,     s_he,      s_me,      s_so,     s_ne,     s_ho,     s_slash
};

static const uint8_t* const output_left[] PROGMEM = {		// 左親指同時
//1        2        3       4         5         6          7         8         9         10        11        12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_l_pren, s_r_pren, s_nul,    s_equ,    s_tild,   s_pipe,
	s_xa, 	 s_e,     s_ri,   s_xya,    s_re,     s_pa,      s_di,     s_gu,     s_du,     s_pi,     s_l_cbrac,s_r_cbrac,
	s_wo,    s_a,     s_na,   s_xyu,    s_mo,     s_ba,      s_do,     s_gi,     s_po,     s_nul,    s_nul,    s_nul,
	s_xu,    s_minus, s_ro,   s_ya,     s_xi,     s_pu,      s_zo,     s_pe,     s_bo,     s_nul
};

static const uint8_t* const output_right[] PROGMEM = {		// 右親指同時
//1        2        3       4         5         6          7         8         9         10        11        12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_l_pren, s_r_pren, s_nul,    s_equ,    s_tild,   s_pipe,
	s_nul,   s_ga,    s_da,   s_go,     s_za,     s_yo,      s_ni,     s_ru,     s_ma,     s_xe,     s_l_cbrac,s_r_cbrac,
	s_vu,    s_ji,    s_de,   s_ge,     s_ze,     s_mi,      s_o,      s_no,     s_xyo,    s_xtu,    s_nul,    s_nul, 
	s_nul,   s_bi,    s_zu,   s_bu,     s_be,     s_nu,      s_yu,     s_mu,     s_wa,     s_xo
};

static const uint8_t* const output_shift[] PROGMEM = {		// Shiftキー押下中
//1        2        3         4         5        6         7         8         9         10       11         12        13
	s_excl,  s_dquot, s_hash,   s_doll,   s_perct, s_amp,    s_quot,   s_l_pren, s_r_pren, s_nul,   s_equ,     s_tild,   s_pipe,
	s_nul,   s_nul,   s_nul,    s_nul,    s_nul,   s_nul,    s_nul,    s_nul,    s_nul,    s_nul,   s_l_cbrac, s_r_cbrac,
	s_nul,   s_nul,   s_larrow, s_rarrow, s_nul,   s_pa,     s_darrow, s_uarrow, s_plus,   s_nul,  s_nul,     s_nul,
	s_nul,   s_pi,    s_nul,    s_pu,     s_pe,    s_delete, s_enter,  s_nul,    s_po,     s_quest
};

// シングル親指キーモード。親指キー同時打鍵時
static const uint8_t* const output_sigle_simul[] PROGMEM = {		
//1        2        3       4         5         6          7         8         9         10        11        12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_l_pren, s_r_pren, s_nul,    s_equ,    s_tild,   s_pipe,
	s_xa,    s_e,     s_ri,   s_xya,    s_re,     s_yo,      s_ni,     s_ru,     s_ma,     s_xe,     s_l_cbrac,s_r_cbrac,
	s_wo,    s_a,     s_na,   s_xyu,    s_mo,     s_mi,      s_o,      s_no,     s_xyo,    s_xtu,    s_nul,    s_nul,
	s_xu,    s_minus, s_ro,   s_ya,     s_xi,     s_nu,      s_yu,     s_mu,     s_wa,     s_xo
};

// シングル親指モード 長押し  *濁音にならないキーは何も出力しない。
static const uint8_t* const output_single_long[] PROGMEM = {	
//1        2        3       4         5         6          7         8         9         10        11      12      13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_l_pren, s_r_pren, s_nul,    s_equ,  s_tild, s_pipe,
	s_nul,   s_ga,    s_da,    s_go,     s_za,     s_nul,    s_di,     s_gu,     s_du,     s_nul,    s_nul,  s_nul,
	s_vu,    s_ji,    s_de,    s_ge,     s_ze,     s_ba,     s_do,     s_gi,     s_nul,    s_nul,    s_nul,  s_nul,
	s_nul,   s_bi,    s_zu,    s_bu,     s_be,     s_nul,    s_zo,     s_nul,    s_bo,     s_nul
};

/** US Layout tables. */
static const uint8_t* const output_single_us[] PROGMEM = {	// 単独打鍵
//1       2        3       4         5         6        7       8      9        10       11           12        13
	s_1,    s_2,     s_3,    s_4,      s_5,      s_6,     s_7,    s_8,   s_9,     s_0,     s_minus,     s_equ,    s_bslas,
	s_peri, s_ka,    s_ta,   s_ko,     s_sa,     s_ra,    s_ti,   s_ku,  s_tu,    s_comm,  s_comm,      s_nul,
	s_u,    s_si,    s_te,   s_ke,     s_se,     s_ha,    s_to,   s_ki,  s_i,     s_nn,    s_nul,       s_nul,
	s_peri, s_hi,    s_su,   s_fu,     s_he,     s_me,    s_so,   s_ne,  s_ho,    s_slash
};

static const uint8_t* const output_left_us[] PROGMEM = {		// 左親指同時
//1        2        3       4         5         6          7         8       9         10          11         12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_aste, s_l_pren, s_r_pren,   s_uline,   s_plus,   s_pipe,
	s_xa, 	 s_e,     s_ri,   s_xya,    s_re,     s_pa,      s_di,     s_gu,   s_du,     s_pi,       s_l_cbrac, s_r_cbrac,
	s_wo,    s_a,     s_na,   s_xyu,    s_mo,     s_ba,      s_do,     s_gi,   s_po,     s_nul,      s_nul,     s_nul,
	s_xu,    s_minus, s_ro,   s_ya,     s_xi,     s_pu,      s_zo,     s_pe,   s_bo,     s_nul
};

static const uint8_t* const output_right_us[] PROGMEM = {		// 右親指同時
//1        2        3       4         5         6          7         8        9         10          11         12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_aste,  s_l_pren, s_r_pren,   s_uline,   s_plus,   s_pipe,
	s_nul,   s_ga,    s_da,   s_go,     s_za,     s_yo,      s_ni,     s_ru,    s_ma,     s_xe,       s_l_cbrac, s_r_cbrac,
	s_vu,    s_ji,    s_de,   s_ge,     s_ze,     s_mi,      s_o,      s_no,    s_xyo,    s_xtu,      s_nul,     s_nul, 
	s_nul,   s_bi,    s_zu,   s_bu,     s_be,     s_nu,      s_yu,     s_mu,    s_wa,     s_xo
};

static const uint8_t* const output_shift_us[] PROGMEM = {		// US Layout with Shift
//1        2        3         4         5        6         7         8         9         10        11         12        13
	s_excl,  s_at,    s_hash,   s_doll,   s_perct, s_carret, s_amp,    s_aste,   s_l_pren, s_r_pren, s_uline,   s_plus,   s_pipe,
	s_nul,   s_nul,   s_nul,    s_nul,    s_nul,   s_nul,    s_nul,    s_nul,    s_nul,    s_nul,    s_l_cbrac, s_r_cbrac,
	s_nul,   s_nul,   s_larrow, s_rarrow, s_nul,   s_pa,     s_darrow, s_uarrow, s_colon,  s_nul,    s_nul,     s_nul,
	s_nul,   s_pi,    s_nul,    s_pu,     s_pe,    s_delete, s_enter,  s_nul,    s_po,     s_quest
};

// シングル親指キーモード。親指キー同時打鍵時
static const uint8_t* const output_sigle_simul_us[] PROGMEM = {		
//1        2        3       4         5         6          7         8        9         10          11         12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_aste,  s_l_pren, s_r_pren,   s_uline,   s_plus,   s_pipe,
	s_xa,    s_e,     s_ri,   s_xya,    s_re,     s_yo,      s_ni,     s_ru,    s_ma,     s_xe,       s_l_cbrac, s_r_cbrac,
	s_wo,    s_a,     s_na,   s_xyu,    s_mo,     s_mi,      s_o,      s_no,    s_xyo,    s_xtu,      s_nul,     s_nul,
	s_xu,    s_minus, s_ro,   s_ya,     s_xi,     s_nu,      s_yu,     s_mu,    s_wa,     s_xo
};

// シングル親指キーモード 長押し時  *濁音にならないキーは何も出力しない。
static const uint8_t* const output_single_long_us[] PROGMEM = {	
//1        2        3       4         5         6          7         8        9         10          11         12        13
	s_quest, s_slash, s_tild, s_l_brac, s_r_brac, s_less,    s_gt,     s_aste,  s_l_pren, s_r_pren,   s_uline,   s_plus,   s_pipe,
	s_nul,   s_ga,    s_da,    s_go,     s_za,    s_nul,     s_di,     s_gu,    s_du,     s_nul,      s_nul,     s_nul,
	s_vu,    s_ji,    s_de,    s_ge,     s_ze,    s_ba,      s_do,     s_gi,    s_nul,    s_nul,      s_nul,     s_nul,
	s_nul,   s_bi,    s_zu,    s_bu,     s_be,    s_nul,     s_zo,     s_nul,   s_bo,     s_nul
};

static const uint8_t* hid_to_nid = hid_to_nid_jp;
static uint8_t nicola_key_count = sizeof(hid_to_nid_jp);

static uint8_t** single_arr = (uint8_t**)output_single;	// 単独打鍵時

static uint8_t** left_arr = (uint8_t**)output_left;			// 親指左同時
static uint8_t** right_arr = (uint8_t**)output_right;			// 親指右同時
static uint8_t** shift_arr = (uint8_t**)output_shift;		// Shiftオン時

static uint8_t** single_simul_arr = (uint8_t**)output_sigle_simul;	// シングル親指キー同時打鍵時
static uint8_t** single_long_arr = (uint8_t**)output_single_long;	// シングル親指キー長押し時

// 出力テーブルの配列も切り替えること。
void HoboNicola::set_nid_table(bool us) {
	if (us) {
		hid_to_nid = hid_to_nid_us;
		nicola_key_count = sizeof(hid_to_nid_us);
		single_arr = (uint8_t**)output_single_us;
		left_arr = (uint8_t**)output_left_us;
		right_arr = (uint8_t**)output_right_us;
		shift_arr = (uint8_t**)output_shift_us;
		single_simul_arr = (uint8_t**)output_sigle_simul_us;	
		single_long_arr = (uint8_t**)output_single_long_us;
	} else {
		hid_to_nid = hid_to_nid_jp;
		nicola_key_count = sizeof(hid_to_nid_jp);
		single_arr = (uint8_t**)output_single;
		left_arr = (uint8_t**)output_left;
		right_arr = (uint8_t**)output_right;
		shift_arr = (uint8_t**)output_shift;
		single_simul_arr = (uint8_t**)output_sigle_simul;	
		single_long_arr = (uint8_t**)output_single_long;
	}
}

uint16_t HoboNicola::get_nid(uint8_t& k) {
	uint16_t m = 0;
	switch(k) {
	case HID_J_COLON:         // C11 : は後退キーに
		k =  HID_BACKSP;
		break;
	case HID_J_RBR_32:	       // C12 [ は取消キーに
		k = HID_ESCAPE;
		break;
	case HID_SPACE:
		if (!dedicated_oyakeys) {
			if (Settings().is_single_oyayubi_mode())	// シングル親指のとき、空白キーは右親指キーとみなす。
				m = MKWORD(k, NID_RIGHT_OYAYUBI);
			if (Settings().is_spc_to_left())
				m = MKWORD(k, NID_LEFT_OYAYUBI);
			else if (Settings().is_spc_to_right())
				m = MKWORD(k, NID_RIGHT_OYAYUBI);
		}
		break;
	case HID_F14:
	case HID_MUHENKAN:
		if (!dedicated_oyakeys)
			m = MKWORD(k, NID_LEFT_OYAYUBI);
		break;
	case HID_F15:
	case HID_HENKAN:
		if (!dedicated_oyakeys)
			m = MKWORD(k, NID_RIGHT_OYAYUBI);
		break;
#if 0
	case HID_PAUSE:
	case HID_DELETE:
		if (!is_initial_state())
			m = MKWORD(k, NID_SETUP_KEY);	// Oyayubi_Stateのみだろう。
		break;			
#endif
	default:
		if (dedicated_oyakeys) {	// 専用の親指キーがある
			if (k == left_oyayubi_code)
				return MKWORD(k, NID_LEFT_OYAYUBI);
			else if (k == right_oyayubi_code)
				return MKWORD(k, NID_RIGHT_OYAYUBI);
		}
		for(uint8_t i = 0; i < nicola_key_count	; i++) {
			// if (pgm_read_byte(&hid_to_nid[i] == k)
			if (hid_to_nid[i] == k)
				return MKWORD(k, (i + 1));
		}
	}
	return m;  
}

// output support
// 出力すべき文字列のアドレスを返す。ATmegaならば文字列はPGM内にある。
// 
const uint8_t*  HoboNicola::get_output_data(uint16_t moji, uint16_t oyayubi) {
	uint8_t n = LOWBYTE(moji) ;
	if (n < 1 || n > nicola_key_count)
		return 0;
	n--;
	uint8_t* address = 0;
	if (oyayubi == 0)
		address = (uint8_t*)PGM_READ_ADDR(single_arr[n]);
	else if (LOWBYTE(oyayubi) == NID_LEFT_OYAYUBI)
		address = (uint8_t*)PGM_READ_ADDR(left_arr[n]);
	else if (LOWBYTE(oyayubi) == NID_RIGHT_OYAYUBI)
		if (Settings().is_single_oyayubi_mode())
			address = (uint8_t*)PGM_READ_ADDR(single_simul_arr[n]);
		else
			address = (uint8_t*)PGM_READ_ADDR(right_arr[n]);
	else if (LOWBYTE(oyayubi) == NID_SHIFT_KEY)
		address = (uint8_t*)PGM_READ_ADDR(shift_arr[n]);
	else if (LOWBYTE(oyayubi) == NID_LONG_PRESSED)
		address = (uint8_t*)PGM_READ_ADDR(single_long_arr[n]);
			
	if (address && PGM_READ_BYTE(address) == 0)
		return 0;
	return (const uint8_t*)address;    
}