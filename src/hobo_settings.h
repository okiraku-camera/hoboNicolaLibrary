// settings.h 
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// basic settings in EEPROM
	static const uint32_t SPC_TO_LEFT	= 1;		// 1-2のうちのいずれかだけオンにできること。
	static const uint32_t SPC_TO_RIGHT = 2;
	static const uint32_t SPC_TO_OYAYUBI = (SPC_TO_LEFT | SPC_TO_RIGHT);	// 0x11 空白は親指キーとして使っている
	static const uint32_t OUTPUT_IMMEDIATE_ON = 4;
	static const uint32_t EISU_TO_NICOLA_OFF = 8;		// 英数(Caps)キーならばNICOLA OFF。
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
	static const uint32_t MUHENKAN_TO_NICOLA_ON	= 0x20000L;	// 無変換／F14が押されたらNICOLAにする。
	static const uint32_t SPC_TO_MUHENKAN				= 0x40000L;	// 一方通行 空白キーを無変換にする。
	static const uint32_t CAPS_TO_IMEOFF				= 0x80000L;	// CapsLockをImeOffとする CapsLockはFn+CapsLockをFnキーテーブルに設定しておけばよい。
	static const uint32_t MUHENKAN_TO_IMEOFF		= 0x100000L;	// 一方通行 無変換／F14をImeOffとする。MUHENKAN_TO_NICOLA_ONやSPC_TO_MUHENKANより先に処理する。
	static const uint32_t DISABLE_NICOLA				= 0x200000L;	// 通知やキー操作でNICOLAモードにしない。NICOLA関連の処理はホスト側でやる。
	static const uint32_t KANA_TO_IMEON					= 0x400000L;	// ひらがなキーを日本語キーボードでも常にIMEONにする。Mac用
	static const uint32_t HENKAN_TO_NICOLA_ON		= 0x800000L;	// 変換／F15が押されたらNICOLAにする。
	static const uint32_t REDUCE_DELAY					= 0x1000000L;	// HIDコード送信時の遅延を短縮する
	static const uint32_t HENKAN_MUHENKAN_FK		= 0x2000000L;	// 変換無変換は無条件にF15,F14にする
	static const uint32_t MUHENKAN_F14_TO_LEFT	= 0x4000000L;	// 無変換やF14を無条件にEnterとする。

#define _SPC_TO_LEFT(a) (bool)(a & SPC_TO_LEFT)
#define _SPC_TO_RIGHT(a) (bool)(a & SPC_TO_RIGHT)
#define _SPC_TO_NONE(a) (bool)((a & SPC_TO_OYAYUBI) == 0)
#define _SINGLE_OYAYUBI_MODE(a) (bool)(a & SINGLE_OYAYUBI_MODE)
#define _SELF_REPEAT(a) (bool)(a & SELF_REPEAT)
#define _OUTPUT_IMMEDIATE_ON(a) (bool)(a & OUTPUT_IMMEDIATE_ON)

#define _SWAP_CAPS_CTRL(a) (bool)(a & SWAP_CAPS_CTRL)
#define _RALT_TO_HIRAGANA(a) (bool)(a & RALT_TO_HIRAGANA)
#define _KANJI_TO_NICOLA_OFF(a) (bool)(a & KANJI_TO_NICOLA_OFF)
#define _KANJI_TOGGLE_NICOLA(a) (bool)(a & KANJI_TOGGLE_NICOLA)

#define _EISU_TO_NICOLA_OFF(a) (bool)(a & EISU_TO_NICOLA_OFF)
#define _CAPS_TO_IMEOFF(a) (bool)(a & CAPS_TO_IMEOFF)
#define _KANA_TO_NICOLA_ON(a) (bool)(a & KANA_TO_NICOLA_ON)
#define _KANA_TO_IMEON(a) (bool)(a & KANA_TO_IMEON)
#define _HENKAN_TO_SPC(a) (bool)(a & HENKAN_TO_SPC)
#define _HENKAN_TO_NICOLA_ON(a) (bool)(a & HENKAN_TO_NICOLA_ON)
#define _MUHENKAN_TO_IMEOFF(a) (bool)(a & MUHENKAN_TO_IMEOFF)
#define _MUHENKAN_TO_NICOLA_ON(a) (bool)(a & MUHENKAN_TO_NICOLA_ON)
#define _SPC_TO_MUHENKAN(a) (bool)(a & SPC_TO_MUHENKAN)

#define _HENKAN_MUHENKAN_FK(a) (bool)(a & HENKAN_MUHENKAN_FK)
#define _MUHENKAN_F14_TO_LEFT(a) (bool)(a & MUHENKAN_F14_TO_LEFT)

#define _REDUCE_DELAY(a) (bool)(a & REDUCE_DELAY)
#define _US_LAYOUT(a) (bool)(a & US_LAYOUT)
#define _DISABLE_NICOLA(a) (bool)(a & DISABLE_NICOLA)
#define _REDUCE_DELAY(a) (bool)(a & REDUCE_DELAY)

#define _SCR_AS_NICOLA(a) (bool)((a & SCR_AS_NICOLA) == 0)
#define _NUML_AS_NICOLA(a) (bool)(a & NUML_AS_NICOLA)
#define _USE_KBD_SUSPEND(a) (bool)(a & USE_KBD_SUSPEND)

#if defined(USE_TINYUSB)
#define _USE_MSC_NOTIFY(a) (bool)(a & USE_MSC_NOTIFY)
#else
#define _USE_MSC_NOTIFY(a) 0
#endif

class _Settings  {
	uint32_t settings;
	uint32_t extra_settings;
	uint32_t flush_count;


	static _Settings* pInstance;

	_Settings() : settings(0), extra_settings(0), flush_count(0) { }
	void set_at(uint16_t  addr, uint32_t data);
	uint32_t get_at(uint16_t addr);
	uint32_t _size();
	void _write(uint16_t addr, uint32_t value);
	void begin();
	void load();
	void flush();

public:
	static _Settings* Create() {
		if (!pInstance) {
			pInstance = new _Settings;
			pInstance->begin();
			pInstance->load();
		}
		return pInstance;
	}

	const uint32_t get_data() { return settings; }
	const uint32_t get_extra() { return extra_settings; }
	const uint32_t get_flush_count() { return flush_count; }
	const uint32_t get_size() { return _size(); }
	const uint8_t get_xd_rgb_value() { return extra_settings & 0xff ; }
	const int16_t get_rp_pwm_max_value() { return extra_settings & 0xffff ; }

	void save(uint32_t new_set);
	void save_extra(uint32_t new_extra);
	void save_xd_rgb_value(uint8_t val);
	void save_rp_pwm_max_value(int16_t val);

	void set_data(uint32_t data) { settings = data; }

	void save_set(int8_t index);
	uint32_t load_set(int8_t index);

#if 0
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
	bool is_spc_to_muhenkan() const { return (bool)(settings & SPC_TO_MUHENKAN); }	
	bool is_caps_to_imeoff() const { return (bool)(settings & CAPS_TO_IMEOFF); }
	bool is_muhenkan_to_nicola_on() const { return (bool)(settings & MUHENKAN_TO_NICOLA_ON); }
	bool is_muhenkan_to_imeoff() const { return (bool)(settings & MUHENKAN_TO_IMEOFF); }
	bool is_kana_to_imeon() const { return (bool)(settings & KANA_TO_IMEON); }
	bool is_disable_nicola() const { return (bool)(settings & DISABLE_NICOLA); }
	bool is_henkan_to_nicola_on() const { return (bool)(settings & HENKAN_TO_NICOLA_ON); }
	bool is_hid_reduce_delay() const { return (bool)(settings & REDUCE_DELAY); }
	bool is_setting_z() const { return (bool)(settings & HENKAN_MUHENKAN_FK);}
	bool is_setting_y() const { return (bool)(settings & MUHENKAN_F14_TO_LEFT);}

	bool is_use_msc_notify() const { 
#if defined(USE_TINYUSB)
		return (bool)(settings & USE_MSC_NOTIFY); 
#else
		return false;
#endif
	}
#endif
};

#endif //#ifndef __SETTINGS_H__