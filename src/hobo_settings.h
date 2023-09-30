// settings.h 
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// basic settings in EEPROM
	static const uint32_t SPC_TO_LEFT	= 1;		// 1-2のうちのいずれかだけオンにできること。
	static const uint32_t SPC_TO_RIGHT = 2;
	static const uint32_t SPC_TO_OYAYUBI = 3;	// 0x11 1-2の設定をクリアする。
	static const uint32_t OUTPUT_IMMEDIATE_ON = 4;
	static const uint32_t EISU_TO_NICOLA_OFF = 8;		// 英数(Caps)キーならばNICOLA OFF。USのときは使わないこと。
	static const uint32_t KANA_TO_NICOLA_ON	= 0x10;	// ひらがな または ImeOnならばNICOLA ON
	static const uint32_t SINGLE_OYAYUBI_MODE = 0x40;
	static const uint32_t SELF_REPEAT = 0x80;
	static const uint32_t SCR_AS_NICOLA					= 0x100;		// Scroll Lock LEDとNICOLAモードを同期する
	static const uint32_t KANJI_TO_NICOLA_OFF		= 0x200;		// 全半キーでNICOLAオフ	
	static const uint32_t KANJI_TOGGLE_NICOLA		= 0x20;			// どちらか一方だけオンにできること。
	static const uint32_t RALT_TO_HIRAGANA			= 0x400;		// 一方通行
	static const uint32_t SWAP_CAPS_CTRL				= 0x800;		// 入れ替え
	static const uint32_t HENKAN_TO_SPC					= 0x1000;		// 一方通行 変換キーを空白にする。
	static const uint32_t US_LAYOUT 						= 0x2000;		// WindowsをUSレイアウトで使うときにオン
	static const uint32_t USE_MSC_NOTIFY 				= 0x4000;		// TINYUSB専用
	static const uint32_t USE_KBD_SUSPEND 			= 0x8000;		// キーボード疑似サスペンド
	static const uint32_t NUML_AS_NICOLA 				= 0x10000L;	// NumLock LEDとNICOLAモードを同期する
	//static const uint32_t KANJI_TO_SHIFT_SPACE	= 0x20000L;	// 全半キーで Shift + Spaceを出す。
	static const uint32_t SPC_TO_MUHENKAN				= 0x40000L;	// 一方通行 空白キーを無変換にする。
	static const uint32_t CAPS_TO_IMEOFF_US			= 0x80000L;	// USのときCapsLockをImeOffとする CapsLockはFn+CapsLockをFnキーテーブルに設定しておけばよい。

class _Settings  {
	uint32_t settings;
	uint32_t extra_settings;
	uint32_t flush_count;

	static const uint16_t  SETTINGS_ADDR =	0;
	static const uint16_t  EXTRA_ADDR	= 8;
	static const uint16_t  COUNTER_ADDR	= 16;

	_Settings() : settings(0), extra_settings(0), flush_count(0) { }
	void set_at(uint16_t  addr, uint32_t data);
	uint32_t get_at(uint16_t addr);
	uint32_t _size();
	void _write(uint16_t addr, uint32_t value);
	void begin();
	void load();
	void flush();

public:
	static _Settings& instance() {
		static _Settings* p = 0;
		if (!p) {
			p = new _Settings;
			p->begin();
			p->load();
		}
		return *p;
	}

	const uint32_t get_data() { return settings; }
	const uint32_t get_extra() { return extra_settings; }
	const uint32_t get_flush_count() { return flush_count; }
	const uint32_t get_size() { return _size(); }
	const uint8_t get_xd_rgb_value() { return extra_settings & 0xff ; }

	void save(uint32_t new_set);
	void save_extra(uint32_t new_extra);
	void save_xd_rgb_value(uint8_t val);

	bool is_immediate_output() const { return (bool)(settings & OUTPUT_IMMEDIATE_ON); }
	bool is_spc_to_left() const { return (bool)(settings & SPC_TO_LEFT); }
	bool is_spc_to_right() const { return (bool)(settings & SPC_TO_RIGHT); }
	bool is_spc_to_none() const { return (bool) (settings & SPC_TO_OYAYUBI) == 0; }
	bool is_self_repeat() const { return (bool)(settings & SELF_REPEAT); }

	bool is_single_oyayubi_mode() const { return (bool)(settings & SINGLE_OYAYUBI_MODE); }

	bool is_eisu_to_nicola_off() const { return (bool)(settings & EISU_TO_NICOLA_OFF); }
	bool is_kana_to_nicola_on() const { return (bool)(settings & KANA_TO_NICOLA_ON); }

	bool is_scrlock_as_nicola() const { return (bool)(settings & SCR_AS_NICOLA) == 0; }
	bool is_numlock_as_nicola() const { return (bool)(settings & NUML_AS_NICOLA); }

	bool is_kanji_toggle_nicola() const { return (bool)(settings & KANJI_TOGGLE_NICOLA); }
	bool is_kanji_to_nicola_off() const { return (bool)(settings & KANJI_TO_NICOLA_OFF); }
	bool is_ralt_to_hiragana() const { return (bool)(settings & RALT_TO_HIRAGANA); }
	bool is_swap_caps_ctrl() const { return (bool)(settings & SWAP_CAPS_CTRL); }
	bool is_henkan_to_spc() const { return (bool)(settings & HENKAN_TO_SPC); }	
	bool is_us_layout() const { return (bool)(settings & US_LAYOUT); }
	bool is_use_kbd_suspend() const { return (bool)(settings & USE_KBD_SUSPEND); }
	
//	bool is_kanji_shift_space() const { return (bool)(settings & KANJI_TO_SHIFT_SPACE); }

	bool is_spc_to_muhenkan() const { return (bool)(settings & SPC_TO_MUHENKAN); }	
	// bool is_imeoff_to_nicola_off() const { return (bool)(settings & IMEOFF_TO_NICOLA_OFF); }
	bool is_caps_to_imeoff_us() const { return (bool)(settings & CAPS_TO_IMEOFF_US); }

	bool is_use_msc_notify() const { 
#if defined(USE_TINYUSB)
		return (bool)(settings & USE_MSC_NOTIFY); 
#else
		return false;
#endif
	}
};

#endif //#ifndef __SETTINGS_H__