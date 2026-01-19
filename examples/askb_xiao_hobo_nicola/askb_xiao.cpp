/**
  askb_xiao.cpp Hardware support program for "ASKeyboard (askb_xiao_rev01)" using hoboNicola Library.
  Copyright (c) 2022 Takeshi Higasa

  This file is part of "ASKeyboard (askb_xiao_rev01)".

  "ASKeyboard (askb_xiao_rev01)" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "ASKeyboard (askb_xiao_rev01)" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "ASKeyboard (askb_xiao_rev01)".  If not, see <http://www.gnu.org/licenses/>.

  SAMD21 version 1.0.0  August. 20, 2022.
*/

#include "arduino.h"
#include <SPI.h>
//#define SPI_CLOCK 8000000
#define SPI_CLOCK 10000000
// クロックを10MHzに指定すると実際は12MHzになっちゃうが、動いてるんで。

#define PORTA(pin)	(pin)
#define PORTB(pin)	(pin + 32)
class spi_ss_port {
	uint8_t group;
  uint8_t port;
  uint8_t pin;
public:
  spi_ss_port(uint8_t _pin) {
		group = 0;	// 0 : PA00～31, 1 : PB00～31
		if (_pin > 31) {
			group = 1;
			_pin -= 32;
		}
    pin = _pin;
  }
  void dir_write() {
    PORT->Group[group].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);
    PORT->Group[group].DIRSET.reg = 1ul << pin;
  }
  void dir_read(){
    PORT->Group[group].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);
    PORT->Group[group].DIRCLR.reg = (1ul << pin);
  }
  void Set() {
    PORT->Group[group].OUTSET.reg = (1ul << pin);
  }
  void Clear() {
    PORT->Group[group].OUTCLR.reg = (1ul << pin);
  }
};

#if defined(ADAFRUIT_QTPY_M0)
	#define SS_ROW	PORTA(6) // D6 / PA06
	#define SS_COL	PORTA(7) // D7 / PA07
#elif defined(SEEED_XIAO_M0)
	#define SS_ROW	PORTB(8) // D6 / PB08
	#define SS_COL	PORTB(9) // D7 / PB09
#endif

spi_ss_port ss_row(SS_ROW);
spi_ss_port ss_col(SS_COL);

static const uint8_t rst_pin = 5;
static const uint8_t spi_address = 0x40;

static const uint8_t IOCON = 0x05;
static const uint8_t IODIRA = 0x00;
static const uint8_t IODIR = IODIRA;
static const uint8_t IODIRB = 0x10;
static const uint8_t IPOLA = 0x01;
static const uint8_t IPOL = IPOLA;
static const uint8_t IPOLB = 0x11;
static const uint8_t GPIOA = 0x09;
static const uint8_t GPIO = GPIOA;
static const uint8_t GPIOB = 0x19;
static const uint8_t OLATA = 0x0A;
static const uint8_t OLAT = OLATA;
static const uint8_t OLATB = 0x1A;
static const uint8_t GPPU = 0x06; // S08のみ

void spi_write(spi_ss_port& device_select, uint8_t reg_addr, uint8_t data) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  device_select.Clear();
  uint8_t c[3]; // spi_addr, reg_addr, data
  c[0] = spi_address; // 書込み時はLSBをクリア
  c[1] = reg_addr; 
  c[2] = data; 
  SPI.transfer(c, 3); 
  device_select.Set();
  SPI.endTransaction();
}

uint8_t spi_read(spi_ss_port& device_select, uint8_t reg_addr) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  device_select.Clear();
  uint8_t c[2]; // spi_addr, reg_addr
  c[0] = spi_address + 1; //  読出し時はLSBをセット。
  c[1] = reg_addr;
  SPI.transfer(c, 2); 
  uint8_t data = SPI.transfer(0); 
  device_select.Set();
  SPI.endTransaction();
  return data;
}

// 1COL選択とROWデータ読み出し
uint8_t spi_read_row(uint8_t reg_addr, uint8_t data) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  ss_col.Clear();
  uint8_t c[3]; // spi_addr, reg_addr, data
  c[0] = spi_address;
  c[1] = reg_addr;
  c[2] = data;
  SPI.transfer(c, 3); 
  ss_col.Set();
//
  ss_row.Clear();
  c[0] = spi_address + 1; 
  c[1] = GPIO;
  SPI.transfer(c, 2); 
  uint8_t value = SPI.transfer(0); 
  ss_row.Set();
//  
  ss_col.Clear();
  c[0] = spi_address;
  c[1] = reg_addr;
  c[2] = 0xff;
  SPI.transfer(c, 3); 
  ss_col.Set();
  SPI.endTransaction();
  return value;
}

void init_mcp23() {
  pinMode(rst_pin, OUTPUT);
  delay(1);
  digitalWrite(rst_pin, HIGH);
  ss_col.dir_write();
  ss_col.Set();
  ss_row.dir_write();
  ss_row.Set();
	
	SPI.begin();

// 23S17は全ピン出力
  spi_write(ss_col, IOCON, B10110000); // BANK=1, SEQOP=1, DISSLW=1, HAEN=0
  spi_write(ss_col, IODIRA, 0); // write
  spi_write(ss_col, IODIRB, 0); // write
  spi_write(ss_col, IPOLA, 0); // 
  spi_write(ss_col, IPOLB, 0); // 
  spi_write(ss_col, OLATA, 0xff); // 
  spi_write(ss_col, OLATB, 0xff); // 

// 23S08は全ピン入力
  spi_write(ss_row, IOCON, B00110000); // SEQOP=1, DISSLW=1, HAEN=0
  spi_write(ss_row, IODIR, 0xff); // read
  spi_write(ss_row, IPOL, 0xff); // reverse polarity.
  spi_write(ss_row, GPIO, 0); // 
  spi_write(ss_row, GPPU, 0); // プルアップ全無効
}

// scancode --> HID UsageID
// usageid = hid_table[][ scancode - 1 ];
static const uint8_t HID_TABLE_BASE = 0;
static const uint8_t HID_TABLE_FN1 = 1;
#define SW_COUNT 92

#include "hid_keycode.h"

static uint8_t hid_table_index = HID_TABLE_BASE;
void table_change(uint8_t key, bool pressed) {
	if (key == HID_X_FN1 && pressed)
		hid_table_index = HID_TABLE_FN1;
	else
		hid_table_index = HID_TABLE_BASE;
}


// scancode --> HID UsageID
// usageid = scan_to_hid_table[ scancode - 1 ];
//static const uint8_t scan_to_hid_table[] PROGMEM = {     
static const uint8_t scan_to_hid_table[2][SW_COUNT] = {
//	
// HID Usage ID,	scancode	Keyname
{	HID_PAUSE,		// 01		STOP --> Pause
	HID_ZENHAN,		// 02		COPY --> Han/Zen.
	HID_F1,			// 03		F1
	HID_F2,			// 04
	HID_F3,			// 05
	HID_F4,			// 06
	HID_F5,			// 07
	HID_F6,			// 08
	HID_F7,			// 09
	HID_F8,			// 10	F8
	HID_F9,			// 11	F9
	HID_F10,		// 12	F10
	HID_ESCAPE,		// 13	ESC --> Esc
	HID_1,			// 14	1
	HID_2,			// 15
	HID_3,			// 16
	HID_4,			// 17
	HID_5,			// 18
	HID_6,			// 19
	HID_7,			// 20
	HID_8,			// 21
	HID_9,			// 22
	HID_0,			// 23
	HID_MINUS,		// 24		- / =
	HID_EQUALS,		// 25		^ / `
	HID_J_BSLASH,	// 26		\ / |
	HID_Q,			// 27		Q
	HID_W,			// 28
	HID_E,			// 29
	HID_R,			// 30
	HID_T,			// 31
	HID_Y,			// 32
	HID_U,			// 33
	HID_I,			// 34
	HID_O,			// 35
	HID_P,			// 36
	HID_LBRACK,		// 37		@ / ~
	HID_RBRACK,		// 38		[ / {
	HID_UNDEF,		// 39		(not used)
	HID_A,			// 40
	HID_S,			// 41
	HID_D,			// 42
	HID_F,			// 43
	HID_G,			// 44
	HID_H,			// 45
	HID_J,			// 46
	HID_K,			// 47
	HID_L,			// 48
	HID_SEMICOLON,	// 49
	HID_Z,			// 50
	HID_X,			// 51
	HID_C,			// 52
	HID_V,			// 53
	HID_B,			// 54
	HID_N,			// 55
	HID_M,			// 56
	HID_COMMA,		// 57
	HID_PERIOD,		// 58
	HID_SLASH,		// 59
	HID_J_UL,		// 60		_ / \. 
	HID_BACKSP,		// 61		BS --> Backspace.
	HID_TAB,		// 62		Tab
	HID_ENTER,		// 63		RETURN --> Enter
	HID_L_CTRL,		// 64		CTRL --> Left Ctrl
	HID_QUOTE,		// 65		: / *
	HID_J_RBR_32,	// 66		] / }
	HID_L_SHIFT,	// 67		Left SHIFT.
	HID_R_SHIFT,	// 68		Right SHIFT.
	HID_CAPS,		// 69		CAPS --> Caps (Eisu)
	HID_L_GUI,		// 70		GRPH --> Left GUI
	HID_L_ALT,		// 71		KANA --> Left Alt.
	HID_HIRAGANA,	// 72		TAB(NFER) --> Hiragana.
	HID_MUHENKAN,	// 73		Left OYAYUBI  MuHenkan
	HID_HENKAN,		// 74		Right OYAYUBI	Henkan.
	HID_SPACE,		// 75		Space.
	HID_R_CTRL,		// 76		CTRL(XFER) --> Right Ctrl.
//	HID_APP,		// 77		ALT --> App
	HID_X_FN1,		// 77		ALT --> Fn
//	HID_IME_OFF,		// 78		NFER
//	HID_IME_ON,			// 79		XFER  IME_ONにすると前回オン時の文字種に戻るのでNG
	HID_CAPS,		// 78		NFER
	HID_HIRAGANA,	// 79		XFER
	HID_PGUP,		// 80		ROLL UP
	HID_PGDOWN,		// 81		ROLL DOWN
	HID_INSERT,		// 82		INS --> Insert	
	HID_HOME,		// 83		HOME --> Home
	HID_DELETE,		// 84		DEL --> Delete
	HID_END,		// 85		HELP --> End
	HID_UNDEF,		// 86		(not used)
	HID_UNDEF,		// 87		(not used)
	HID_UNDEF,		// 88		(not used)
	HID_U_ARROW,	// 89		↑
	HID_L_ARROW,	// 90		←
	HID_R_ARROW,	// 91		→
	HID_D_ARROW		// 92		↓
},
{	HID_PAUSE,		// 01		STOP --> Pause
	HID_ZENHAN,		// 02		COPY --> Han/Zen.
	HID_F11,		// 03		F1 --> F11
	HID_F12,		// 04
	HID_F13,		// 05
	HID_F14,		// 06
	HID_F15,		// 07
	HID_F16,		// 08
	HID_F17,		// 09
	HID_PRNTSCRN,	// 10	F8
	HID_SCRLOCK,	// 11	F9
	HID_PAUSE,		// 12	F10
	HID_ESCAPE,		// 13	ESC --> Esc
	HID_1,			// 14	1
	HID_2,			// 15
	HID_3,			// 16
	HID_4,			// 17
	HID_5,			// 18
	HID_6,			// 19
	HID_7,			// 20
	HID_8,			// 21
	HID_9,			// 22
	HID_0,			// 23
	HID_MINUS,		// 24		- / =
	HID_EQUALS,		// 25		^ / `
	HID_J_BSLASH,	// 26		\ / |
	HID_Q,			// 27		Q
	HID_W,			// 28
	HID_E,			// 29
	HID_R,			// 30
	HID_T,			// 31
	HID_Y,			// 32
	HID_U,			// 33
	HID_I,			// 34
	HID_O,			// 35
	HID_P,			// 36
	HID_LBRACK,		// 37		@ / ~
	HID_RBRACK,		// 38		[ / {
	HID_UNDEF,		// 39		(not used)
	HID_A,			// 40
	HID_S,			// 41
	HID_D,			// 42
	HID_F,			// 43
	HID_G,			// 44
	HID_H,			// 45
	HID_J,			// 46
	HID_K,			// 47
	HID_L,			// 48
	HID_SEMICOLON,	// 49
	HID_Z,			// 50
	HID_X,			// 51
	HID_C,			// 52
	HID_V,			// 53
	HID_B,			// 54
	HID_N,			// 55
	FN_MEDIA_MUTE,	// HID_M,	// 56
	FN_MEDIA_V_DN,	// HID_COMMA, // 57
	FN_MEDIA_V_UP,	// HID_PERIOD,	// 58
	HID_SLASH,		// 59
	HID_J_UL,		// 60		_ / \. 
	HID_BACKSP,		// 61		BS --> Backspace.
	HID_TAB,		// 62		Tab
	HID_ENTER,		// 63		RETURN --> Enter
	HID_L_CTRL,		// 64		CTRL --> Left Ctrl
	HID_QUOTE,		// 65		: / *
	HID_J_RBR_32,	// 66		] / }
	HID_L_SHIFT,	// 67		Left SHIFT.
	HID_R_SHIFT,	// 68		Right SHIFT.
	HID_CAPS,		// 69		CAPS --> Caps (Eisu)
	HID_L_GUI,		// 70		GRPH --> Left GUI
	HID_L_ALT,		// 71		KANA --> Left Alt.
	HID_HIRAGANA,	// 72		TAB(NFER) --> Hiragana.
	HID_MUHENKAN,	// 73		Left OYAYUBI  MuHenkan
	HID_HENKAN,		// 74		Right OYAYUBI	Henkan.
	HID_SPACE,		// 75		Space.
	HID_R_CTRL,		// 76		CTRL(XFER) --> Right Ctrl.
//	HID_APP,		// 77		ALT --> App
	HID_X_FN1,		// 77		ALT --> Fn
	HID_IME_OFF,		// 78		NFER
//	HID_IME_ON,			// 79		XFER  IME_ONにすると前回オン時の文字種に戻るのでNG
	HID_HIRAGANA,	// 79		XFER
	HID_PGUP,		// 80		ROLL UP
	HID_PGDOWN,		// 81		ROLL DOWN
	HID_INSERT,		// 82		INS --> Insert	
	HID_HOME,		// 83		HOME --> Home
	HID_DELETE,		// 84		DEL --> Delete
	HID_END,		// 85		HELP --> End
	HID_UNDEF,		// 86		(not used)
	HID_UNDEF,		// 87		(not used)
	HID_UNDEF,		// 88		(not used)
	HID_U_ARROW,	// 89		↑
	HID_L_ARROW,	// 90		←
	HID_R_ARROW,	// 91		→
	HID_D_ARROW		// 92		↓
}
};

static const uint8_t max_scan_code = 92;
static const uint8_t scan_lines = 12;
static const uint8_t DIP_SW_JIS = 1;
static const uint8_t DIP_SW_OYA1 = 0;
static const uint8_t DIP_SW_OYA2 = 2;
static uint8_t dip_sw = 0;;
bool kana_locked = false;
uint8_t current_scan[scan_lines] = {0};
uint8_t last_scan[scan_lines] = {0};

const bool is_dipsw_jis() {
	return (dip_sw == DIP_SW_JIS);
}

void clear_data() {
	memset(current_scan, 0, sizeof(current_scan));
	memset(last_scan, 0, sizeof(last_scan));
}

extern void askb_key_event(uint8_t key, bool keyon);

void matrix_scan() {
	uint8_t tmp_scan[scan_lines];
	uint8_t col_index = 0;
// new scan.
	for(uint8_t i = 0; i < 8; i++)
		tmp_scan[col_index++] = spi_read_row(OLATA, 0xff & ~(1 << i));
	for(uint8_t i = 0; i < 4; i++)
		tmp_scan[col_index++] = spi_read_row(OLATB, 0xff & ~(1 << i));
	int n = memcmp(tmp_scan, current_scan, scan_lines);
	memcpy(current_scan, tmp_scan, scan_lines);

	if (n == 0) {	// 前回と一致した。
// generate scan-code.
		for(uint8_t i = 0; i < scan_lines; i++) {
			uint8_t cur = current_scan[i];
			uint8_t old = last_scan[i];
			uint8_t changed = cur ^ old;
			if (!changed)
			  continue;
			uint8_t mask = 1;
			for(int8_t r = 0; r < 8; r++, mask <<= 1) {
				if (changed & mask) {
					int8_t key = (i << 3) + r + 1;
					if (key > 0 && key <= max_scan_code)
						askb_key_event(scan_to_hid_table[hid_table_index][key - 1], (bool)((cur & mask) != 0));
				}
			}
		}
		memcpy(last_scan, current_scan, scan_lines);
	}

}

// XIAO-m0専用
void init_askb_xiao() {
// PM_APBCMASK_SERCOM0がSPI

// 通常動作時の消費電流が1mAほど減る。ディープスリープ時には影響しない。
  uint32_t pm = PM_APBCMASK_ADC | PM_APBCMASK_AC | PM_APBCMASK_DAC | PM_APBCMASK_I2S | PM_APBCMASK_PTC |
    PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | 
    PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 | PM_APBCMASK_TC6 | PM_APBCMASK_TC7 | 
    PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM5;
  PM->APBCMASK.reg &= ~pm;

// XIAOのオンボードLEDをINPUT_PULLUPにしておく。
	pinMode(LED_BUILTIN, INPUT_PULLUP);
	pinMode(PIN_LED2, INPUT_PULLUP);
	pinMode(PIN_LED3, INPUT_PULLUP);

	init_mcp23();
}
