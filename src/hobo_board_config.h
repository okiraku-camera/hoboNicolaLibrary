/**
	hobo_board_config.h  board defs and funcs of "Hobo-nicola usb/ble adapter".
	Copyright (c) 2022 Takeshi Higasa

	This file is part of "Hobo-nicola keyboard and adapter".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	included in version 1.6.2  July. 1st, 2022.
		support 
		AVR    : Pro Micro(+3.3V/8MHz),
		SAMD21 : QTPy-m0, XIAO-m0, 
		nRF52  : SSCI ISP1807 Microboard, XIAO-nRF52840
		RP2040 : XIAO-RP, KB2040

	add CH9350L support. (XIAO type: rst_port=2, ProMicro type: rst_port=3
	add neopixel support.
*/

#include "Arduino.h"
#ifndef __hobo_board_config_h__
#define __hobo_board_config_h__

#if defined(BLE_LED)
#undef BLE_LED
#endif

// NeoPixelを使うボードでは、#defineすること。
#if defined(USE_NEOPIXEL)
#undef USE_NEOPIXEL
#endif

#if defined(PIN_NEOPIXEL_VCC)
#undef PIN_NEOPIXEL_VCC
#endif

#if defined(CH_RST_PORT)
#undef CH_RST_PORT
#endif

#if defined(ARDUINO_AVR_PROMICRO) || defined(ARDUINO_AVR_LEONARDO)
#include <avr/wdt.h>
	#ifndef PRTIM4
		#define PRTIM4 4
	#endif
	#define LED1 LED_BUILTIN_RX
	#define LED2 LED_BUILTIN_TX
	#define LED_OFF	1
	#define CH_RST_PORT 3
#endif

// QTPy-m0 にはふつうのLEDが載ってないのでNeoPixelを使う。
#if defined(ADAFRUIT_QTPY_M0)
	#define PIN_NEOPIXEL_VCC 12
	#define LED1 31
	#define LED2 31
	#define LED_OFF	1
	#define CH_RST_PORT 2	
	#define USE_NEOPIXEL 1
#endif  

// XIAO-m0 には3つのLEDがある。
// オンボードのLEDでもいいし、自分で用意したものでもよい。
#if defined(ARDUINO_SEEED_XIAO_M0)
	#define LED1 LED_BUILTIN
	#define LED2 PIN_LED2
//	#define LED1 0
//	#define LED2 1
	#define LED_OFF	1
	// D2
	#define CH_RST_PORT 2
#endif

// ISP1807-MB 用のLEDは自分で用意する。
#if defined(ARDUINO_SSCI_ISP1807_MICRO_BOARD)
//	#define LED1 19
	#define LED1 PIN_LED1 // とりあえずオンボードのLEDにしておく。
	#define LED2 20
	#define LED_OFF 1
	#define BLE_LED 18 
	#define CH_RST_PORT 3
#endif

#if defined(ARDUINO_Seeed_XIAO_nRF52840)
	#define LED1 13
	#define LED2 11
	#define BLE_LED 12
	#define LED_OFF 1
	#define CH_RST_PORT 2
#endif  

// adafruit KB2040
// neopixelしか載ってない。常にNeoPixelの電源はオン。
// neopixelのポートピンは、PIN_NEOPIXELに定義済。
#if defined(ARDUINO_ADAFRUIT_KB2040_RP2040)
//	#define LED1 19
	#define LED1 31
	#define LED2 31
	#define LED_OFF 1
	#define CH_RST_PORT 3
	#define USE_NEOPIXEL 1
#endif

// XIAO rp2040はneopixelとふつうのLEDが載っている。
// NeoPixelは使わない。
#if defined(ARDUINO_SEEED_XAIO_RP2040) || defined(ARDUINO_SEEED_XIAO_RP2040)
  #define PIN_NEOPIXEL_VCC 11
  #define LED1 16
 #define LED2 17
//	#define LED1 17 // red
//	#define LED2 16 // green
	#define LED_OFF 1
	#define CH_RST_PORT 28
// BLEではないが、青を消灯するために定義。
	#define BLE_LED	25
	#define READY_LED	25  // blue
#endif  

#if defined(ARDUINO_RASPBERRY_PI_PICO) 
	#define LED1 6
	#define LED2 7
	#define LED_OFF 0
	#define READY_LED	25
#endif  

// nk60などのためのダミーの定義
#if defined(ARDUINO_GENERIC_RP2040) 
	#define LED1 0
	#define LED2 0
	#define LED_OFF 0
	#define READY_LED	0
#endif


#define LED_ON	(1 - LED_OFF)

void hobo_device_setup(bool use_spi = true, bool use_uart = false);

#endif // __hobo_board_config_h__
