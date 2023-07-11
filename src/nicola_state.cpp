/**
 * 
	nicola_state.cpp   NICOLA-state-machine implements simultaneous typing method of "Hobo-nicola keyboard and adapter library".
	Copyright (c) 2018 Takeshi Higasa, okiraku-camera.tokyo

	This file is part of "Hobo-nicola keyboard and adapter library".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

		Included in hoboNicola 1.5.0.		July. 31. 2021.
			based on nicola_keyboard.cpp
		hoboNicola 1.7.0. March.3. 2023
			Separating nicola tables to nicola_table.cpp
*/

#include "hobo_nicola.h"
#include "hobo_settings.h"

void HoboNicola::timer_tick(unsigned long now) {
	if (event_time != 0 && (now > event_time)) {
		event_time = 0;
		nicola_state(Time_out, 0);
	}
	if (Settings().is_self_repeat() && (repeat_time != 0) && (now > repeat_time)) {
		nicola_state(Key_repeat, 0);
		repeat_time = now + repeat_interval;
	}
}

void HoboNicola::moji_set(uint16_t param) {
	moji = param;
	moji_time = millis();
	event_time = moji_time + e_charTime;
	repeat_moji = repeat_oyayubi = 0;
	immediate_output(moji);
}

void HoboNicola::oyayubi_set(uint16_t param) {
	oyayubi = param;
	oyayubi_time = millis();
	event_time = oyayubi_time + e_oyaTime;
	repeat_moji = repeat_oyayubi = 0;
}

// 即時出力用の前文字キャンセル。
#define sendBS() stroke(HID_BACKSP, 0, true)	

static bool immediate = false;

void HoboNicola::nicola_state(nicola_event_t e, uint16_t param) {
	unsigned long now = millis();
	if (e == All_off) {
		repeat_time = 0;
	}
	switch(state) {
	case Initial_State:
		event_time = moji_time = oyayubi_time = 0;
		oyayubi = moji = 0;
		repeat_moji = repeat_oyayubi = 0;
		repeat_time = 0;
		immediate = false;
		switch(e) {
		case Moji_pressed:
			moji_set(param);
			state = Character_State;
			break;
		case Oyayubi_pressed:
			oyayubi_set(param);
			state = Oyayubi_State;
			break;
		default:
			break;
		}
		break;
	case Character_State:
		switch(e) {
		case Init:
			state = Initial_State;
			break;
		case All_off:
		case Key_released:
			output();
			state = Initial_State;
			break;
		case Moji_pressed:
			output();
			moji_set(param);
			break;
		case Oyayubi_pressed:
			oyayubi_set(param);
			state = Char_Oya_State;
			break;
		case Time_out:
			repeat_moji = moji;
			output();
			if (Settings().is_single_oyayubi_mode()) {	// 長押し待ちへ
				event_time = now + e_longpressTime;
				state = Release_Wait_State;
			} else {
				state = Repeat_State;
				repeat_time = millis() + repeat_delay;
			}
			break;
		default:
			break;			
		}
		break;

	case Release_Wait_State:	// 長押しタイムアウト待ち
		switch(e) {
		case Moji_pressed:
			moji_set(param);
			state = Character_State;
			break;
		case Oyayubi_pressed:
			oyayubi_set(param);
			state = Oyayubi_State;
			break;
		case Time_out:	// 長押し時間経過
			repeat_oyayubi = NID_LONG_PRESSED;
			output();	
			state = Repeat_State;
			repeat_time = millis() + repeat_delay;
			break;
		default:
			state = Initial_State;
			break;			
		}
		break;
	case Oyayubi_State:
		switch(e) {
		case Init:
			state = Initial_State;
			break;
		case Moji_pressed:
			repeat_oyayubi = oyayubi;
			repeat_moji = param;
			output();
			state = Repeat_State;
			repeat_time = millis() + repeat_delay;
			break;
		case All_off:
			output();
			state = Initial_State;
			break;
		case Key_released:
			if (param == oyayubi) {
				output();
				state = Initial_State;
			}
			break;
		case Oyayubi_pressed:
			output();
			oyayubi_set(param);
			break;
		case Time_out:
			if (!dedicated_oyakeys) {
				repeat_oyayubi = oyayubi;
				output();
				state = Repeat_State;
				repeat_time = millis() + repeat_delay;
			}
			break;
		default:
			break;			
		}
		break;
	case Char_Oya_State:
		switch(e) {
		case Init:
			state = Initial_State;
			break;
		case All_off:
			output();
			state = Initial_State;
			break;
		case Oyayubi_pressed:
			output();
			oyayubi_set(param);
			state = Oyayubi_State;
			break;
		case Moji_pressed:
			if ((oyayubi_time - moji_time) < (now - oyayubi_time)) {
				output();   // 先行の文字と親指
				moji_set(param);
				state = Character_State;
			} else {
				uint16_t oya = oyayubi;
				oyayubi = 0;
				if (!immediate)
					output();   // 先行の文字だけ出す
				immediate = false;
				repeat_oyayubi = oya;
				repeat_moji = param;
				output();   // 後追いの文字と親指
				state = Repeat_State;
				repeat_time = millis() + repeat_delay;
			}
			break;
		case Key_released:  // 文字キーが離されたタイミングに応じて単独打鍵とする。
			if ((moji == param) && (now - oyayubi_time < e_nicolaTime) && (oyayubi_time - moji_time) > (now - oyayubi_time)) {
				uint16_t oya = oyayubi;
				oyayubi = 0;
				if (!immediate)
					output();  
				moji = 0;
				immediate = false;
				oyayubi = oya;
				state = Oyayubi_State;
				break;
			}
			output();
			state = Initial_State;
			break;
		case Time_out:
			repeat_oyayubi = oyayubi;
			repeat_moji = moji;
			output();
			state = Repeat_State;
			repeat_time = millis() + repeat_delay;
			break;
		default:
			break;		
		}
		break;
	case Repeat_State:
		switch(e) {
		case Key_repeat:
			output();
			break;
		case Oyayubi_pressed:
			oyayubi_set(param);
			moji = 0;
			state = Oyayubi_State;
			break;		
		case Moji_pressed:
			moji_set(param);
			state = Character_State;
			break;
		case Time_out:
			break;	// ignore time-out event.	
		default:
			state = Initial_State;
			break;
		}	
		break;
	}
}

#define isShiftPressed() (bool) ((modifiers & HID_SHIFT_MASK) != 0)

void HoboNicola::output() {
	if (repeat_moji || repeat_oyayubi) {	// 長押しのときもrepeat_mojiに入っている。
		moji = repeat_moji;
		oyayubi = repeat_oyayubi;
	}
	if (moji == 0 && oyayubi) {
		stroke(HIGHBYTE(oyayubi), modifiers);
		oyayubi = 0;
		return;
	}
	if (immediate) {
		immediate = false;
		if (oyayubi)
			sendBS();
		else
			return;
	}
	if (setup_mode) {
		setup_options(LOWBYTE(moji));
		moji = 0;
		oyayubi = 0;
		return;
	}

	if (isShiftPressed() && oyayubi == 0)
		oyayubi = NID_SHIFT_KEY;

	const uint8_t* p = get_output_data(moji, oyayubi);
	oyayubi = 0;
	moji = 0;
	if (p) {
		if (state == Release_Wait_State) {
			sendBS();	// タイムアウトで出した文字をキャンセル。
			delay(2);
		}
		send_PGM_string(p);
	}
}

void HoboNicola::immediate_output(uint16_t moji) {
	if (!Settings().is_immediate_output() || setup_mode || LOWBYTE(moji) == NID_SETUP_KEY)
		return;
	const uint8_t* p = get_output_data(moji, isShiftPressed() ? NID_SHIFT_KEY : 0);
	if (p) {
		send_PGM_string(p);
		immediate = true;
	}
}
