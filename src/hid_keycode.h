#ifndef __HID_KEYCODE_H__
#define __HID_KEYCODE_H__

/**
	hid_keycode.h some definitions(HID keycode, Modifiers masks, LED, etc..) of "Hobo-nicola keyboard and adapter library".
  Copyright (c) Takeshi Higasa, okiraku-camera.tokyo
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
   
  HID Usage ID values for hoboNicola keyboard and adapter library.
  0x00 - 0x03 : Undefined or Error code.
  0x04 - 0x9f : basic key codes area.
  0xa0 - 0xaf : consumer control codes (media keys)
  0xb0 - 0xb7 : system control codes (power, sleep, wakeup)
  0xb8 - 0xbf : extended function keys (e.g. Fn keys) 
  0xc0 - 0xcf : extra function keys (e.g. Fn+modifier keys) 
  0xd0 - 0xdf : internal macro keys (not sent to host, used for internal processing)
  0xe0 - 0xe7 : modifier keys (Ctrl, Shift, Alt, GUI) 
  0xe8 - 0xef : reserved
  0xf0 - 0xff : vendor defined


*/

#define HID_UNDEF 0
#define HID_KBD_START	0x04
#define HID_A		0x04 
#define HID_B		0x05 
#define HID_C		0x06 
#define HID_D		0x07 
#define HID_E		0x08 
#define HID_F		0x09 
#define HID_G		0x0a 
#define HID_H		0x0b 
#define HID_I		0x0c 
#define HID_J		0x0d 
#define HID_K		0x0e 
#define HID_L		0x0f 
#define HID_M		0x10 
#define HID_N		0x11 
#define HID_O		0x12 
#define HID_P		0x13 
#define HID_Q		0x14 
#define HID_R		0x15 
#define HID_S		0x16 
#define HID_T		0x17 
#define HID_U		0x18 
#define HID_V		0x19 
#define HID_W		0x1a 
#define HID_X		0x1b 
#define HID_Y		0x1c 
#define HID_Z		0x1d 

#define HID_1		0x1e 
#define HID_2		0x1f 
#define HID_3		0x20 
#define HID_4		0x21 
#define HID_5		0x22 
#define HID_6		0x23 
#define HID_7		0x24 
#define HID_8		0x25 
#define HID_9		0x26
#define HID_0		0x27 

#define HID_ENTER	0x28 
#define HID_ESCAPE	0x29 
#define HID_BACKSP	0x2a 
#define HID_TAB		0x2b 
#define HID_SPACE	0x2c 
#define HID_SPC HID_SPACE 

#define HID_MINUS		0x2d 
#define HID_EQUALS		0x2e  
#define HID_EQL HID_EQUALS
#define HID_J_CFLEX   HID_EQUALS  // ^ ~
#define HID_LBRACK	0x2f
#define HID_J_AT  HID_LBRACK  // @ `
#define HID_RBRACK	0x30    // [ {
#define HID_J_LBRACK  HID_RBRACK    // [ {
#define HID_BSLASH		0x31	// us keyboard.
#define HID_J_RBRACK  HID_BSLASH    // ] }
#define HID_J_RBR_32	0x32	// TORIKESHI key.
#define HID_SEMICOLON	0x33  // ; +
#define HID_SEMICOL HID_SEMICOLON
#define HID_QUOTE		  0x34  // : *
#define HID_J_COLON     HID_QUOTE
#define HID_GRAVE_AC	0x35		// ` / ~ 
#define HID_ZENHAN  HID_GRAVE_AC  // 半角全角

#define HID_COMMA		0x36 
#define HID_PERIOD		0x37 
#define HID_SLASH		0x38 

#define HID_CAPS    0x39
#define HID_F1			0x3a
#define HID_F2			0x3b
#define HID_F3			0x3c
#define HID_F4			0x3d
#define HID_F5			0x3e
#define HID_F6			0x3f
#define HID_F7			0x40
#define HID_F8			0x41
#define HID_F9			0x42
#define HID_F10			0x43
#define HID_F11			0x44
#define HID_F12			0x45
#define HID_PRNTSCRN	0x46
#define HID_SCRLOCK		0x47
#define HID_PAUSE		0x48

#define HID_INSERT		0x49
#define HID_HOME			0x4a
#define HID_PGUP			0x4b
#define HID_DELETE		0x4c
#define HID_END				0x4d
#define HID_PGDOWN		0x4e
#define HID_R_ARROW		0x4f
#define HID_L_ARROW		0x50
#define HID_D_ARROW		0x51
#define HID_U_ARROW		0x52
// Keypad or Numpad 0x53 - 0x63
#define HID_KEYPAD_NUMLOCK	0x53
#define HID_KEYPAD_SLASH		0x54
#define HID_KEYPAD_ASTERISK	0x55
#define HID_KEYPAD_MINUS		0x56
#define HID_KEYPAD_PLUS			0x57
#define HID_KEYPAD_ENTER		0x58
#define HID_KEYPAD_1				0x59
#define HID_KEYPAD_2				0x5a
#define HID_KEYPAD_3				0x5b
#define HID_KEYPAD_4				0x5c
#define HID_KEYPAD_5				0x5d
#define HID_KEYPAD_6				0x5e
#define HID_KEYPAD_7				0x5f
#define HID_KEYPAD_8				0x60
#define HID_KEYPAD_9				0x61
#define HID_KEYPAD_0				0x62
#define HID_KEYPAD_PERIOD		0x63

// 0x64 : Non-US \ / | 
#define HID_APP     0x65
#define HID_POWER		0x66
// 0x67 Keypad EQUALS
// extended Function keys.
#define HID_F13			0x68
#define HID_F14			0x69
#define HID_F15			0x6a
#define HID_F16			0x6b
#define HID_F17			0x6c
#define HID_F18			0x6d
#define HID_F19			0x6e
#define HID_F20			0x6f
#define HID_F21			0x70
#define HID_F22			0x71
#define HID_F23			0x72
#define HID_F24			0x73

//
#define HID_MUTE	0x7f
#define HID_VOL_UP	0x80
#define HID_VOL_DN	0x81
// International keys
#define HID_INT1	0x87
#define HID_INT2	0x88
#define HID_INT3	0x89
#define HID_INT4	0x8a
#define HID_INT5	0x8b
#define HID_INT6	0x8c
#define HID_INT7	0x8d
#define HID_INT8	0x8e
#define HID_INT9	0x8f

#define HID_J_UL		HID_INT1
#define HID_HIRAGANA	  HID_INT2
#define HID_HIRA			  HID_INT2
#define HID_J_BSLASH	  HID_INT3
#define HID_HENKAN		  HID_INT4
#define HID_MUHENKAN	  HID_INT5
#define HID_MUHEN			  HID_INT5

#define HID_LANG1	0x90
#define HID_LANG2	0x91
#define HID_LANG3	0x92
#define HID_LANG4	0x93
#define HID_LANG5	0x94
#define HID_LANG6	0x95
#define HID_LANG7	0x96
#define HID_LANG8	0x97
#define HID_LANG9	0x98

// for MS-IME
#define HID_IME_ON	HID_LANG1
#define HID_IME_OFF	HID_LANG2
#define HID_KBD_END	0x9f

#define IS_BASIC_KEYCODE(a) ((a) >= HID_KBD_START && (a) <= HID_KBD_END)

// standard modifiers
#define HID_MODIFIERS   0xe0
#define HID_MODIFIERS_MAX   0xe8
#define HID_MODIFIERS_COUNT (HID_MODIFIERS_MAX - HID_MODIFIERS)
#define HID_L_CTRL	0xe0
#define HID_L_SHIFT	0xe1
#define HID_L_ALT	0xe2
#define HID_L_GUI	0xe3
#define HID_R_CTRL	0xe4
#define HID_R_SHIFT	0xe5
#define HID_R_ALT	0xe6
#define HID_R_GUI	0xe7

#define IS_MODIFIER_KEYCODE(a) ((a) >= HID_MODIFIERS && (a) <= HID_R_GUI) 

#define HID_L_CTRL_MASK 1
#define HID_L_SHIFT_MASK 2
#define HID_L_ALT_MASK 4
#define HID_L_GUI_MASK 8
#define HID_R_CTRL_MASK 0x10
#define HID_R_SHIFT_MASK 0x20
#define HID_R_ALT_MASK 0x40
#define HID_R_GUI_MASK 0x80

#define HID_CTRL_MASK 0x11
#define HID_SHIFT_MASK 0x22
#define HID_ALT_MASK 0x44
#define HID_GUI_MASK 0x88

// hoboNicola extended key codes.
// メディアおよびシステム制御コードは hid_wrap.cpp内のレポートディスクリプタと整合していること。
#define FN_MEDIA_CODE_START 0xa0

#define FN_MEDIA_PLAY_PAUSE FN_MEDIA_CODE_START
#define FN_MEDIA_PLAY_SKIP (FN_MEDIA_CODE_START + 1)
#define FN_MEDIA_FF (FN_MEDIA_CODE_START + 2)
#define FN_MEDIA_REW (FN_MEDIA_CODE_START + 3)
#define FN_MEDIA_SCAN_NEXT (FN_MEDIA_CODE_START + 4)
#define FN_MEDIA_SCAN_PREV (FN_MEDIA_CODE_START + 5)
#define FN_MEDIA_STOP (FN_MEDIA_CODE_START + 6)
#define FN_MEDIA_VOL_UP (FN_MEDIA_CODE_START + 7)
#define FN_MEDIA_V_UP FN_MEDIA_VOL_UP
#define FN_M_V_UP FN_MEDIA_VOL_UP
#define FN_MEDIA_VOL_DOWN (FN_MEDIA_CODE_START + 8)
#define FN_MEDIA_V_DN FN_MEDIA_VOL_DOWN
#define FN_M_V_DN FN_MEDIA_VOL_DOWN
#define FN_MEDIA_MUTE (FN_MEDIA_CODE_START + 9)
#define FN_M_MUTE FN_MEDIA_MUTE
#define FN_MEDIA_CODE_END 0xb0
#define IS_CONSUMER_KEYCODE(a) ((a) >= FN_MEDIA_CODE_START && (a) < FN_SYSTEM_CODE_START)

#define FN_SYSTEM_CODE_START FN_MEDIA_CODE_END
#define FN_SYSTEM_POWER_DOWN 0xb0
#define FN_SYSTEM_SLEEP 0xb1
#define FN_SYSTEM_WAKEUP 0xb2
#define FN_SYSTEM_CODE_END 0xb8

#define IS_SYSTEM_KEYCODE(a) ((a) >= FN_SYSTEM_CODE_START && (a) < FN_SYSTEM_CODE_END)

#define FN_SETUP_MODE 0xb8	// Fnキーを使って設定を開始する (hoboNicola)
#define FN_SELECT_STORED_1 0xb9	// StoredSettings1 を選択する
#define FN_SELECT_STORED_2 0xba	// StoredSettings2 を選択する
#define FN_SELECT_STORED_3 0xbb	// StoredSettings3 を選択する

#define HID_X_FN1 0xbc
#define HID_X_FN2 0xbd
#define HID_X_FN3 0xbe

// speical function keys (not sent to host, used for keyboard implementation)
#define FN_EXTRA_START 0xc0
#define FN_EXTRA_END (FN_EXTRA_START + 4) // 0xc3

#define IS_MOUSE_CODE(a) ((a) >= FN_MS_BTN1 && (a) <= FN_MS_WH_DOWN)
#define FN_MS_BTN1 0xc4
#define FN_MS_BTN2 0xc5
#define FN_MS_BTN3 0xc6
#define FN_MS_BTN_END FN_MS_BTN3

#define FN_MS_LEFT 0xc7 
#define FN_MS_RIGHT 0xc8
#define FN_MS_UP 0xc9
#define FN_MS_DOWN 0xca
#define FN_MS_WH_UP 0xcb
#define FN_MS_WH_DOWN 0xcc


#define IS_MACRO_KEYCODE(a) ((a) >= HID_M_0 && (a) <= HID_M_9) 
#define MACRO_KEYCODE_BASE HID_M_0
#define MACRO_KEY_COUNT 10
#define HID_M_0 0xd0
#define HID_M_1 0xd1
#define HID_M_2 0xd2
#define HID_M_3 0xd3
#define HID_M_4 0xd4
#define HID_M_5 0xd5
#define HID_M_6 0xd6
#define HID_M_7 0xd7
#define HID_M_8 0xd8 
#define HID_M_9 0xd9

#define IS_MACRO_DELAY_CODE(a) ((a) >= HID_M_DELAY100 && (a) <= HID_M_DELAY1000)
#define HID_M_DELAY100 0xda	// delay 100ms
#define HID_M_DELAY500 0xdb	// delay 500ms
#define HID_M_DELAY1000 0xdc	// delay 1000ms


// キー+ 修飾キーを指定する場合。
#define WITH_L_CTRL 0x0100
#define WITH_L_SHIFT 0x0200
#define WITH_L_ALT 0x0400
#define WITH_L_GUI 0x0800
#define WITH_R_CTRL 0x1000
#define WITH_R_SHIFT 0x2000
#define WITH_R_ALT 0x4000
#define WITH_R_GUI 0x8000


// LEDs
#define HID_LED_NUMLOCK   1
#define HID_LED_CAPSLOCK  2
#define HID_LED_SCRLOCK   4
#define HID_LED_COMPOSE   8
#define HID_LED_KANA      0x10

#endif //  __HID_KEYCODE_H__
