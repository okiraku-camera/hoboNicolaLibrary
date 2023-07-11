/**
	hobo_board_config.cpp  Board defs and funcs of "Hobo-nicola usb/ble adapter".
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
		AVR    : Pro Micro(+3.3V/8MHz and +5V/16MHz),
		SAMD21 : QTPy-m0, XIAO-m0, 
		nRF52  : SSCI ISP1807 Microboard, XIAO-nRF52840
		RP2040 : XIAO-RP, KB2040

	add CH9350L support. (XIAO type: rst_port=2, ProMicro type: rst_port=3
	add neopixel support.
*/

#include "hobo_board_config.h"

extern void neo_pixel_begin();	// hobo_led.cpp

// MAX3421EEならばtrue, false 
//   Serial1をデバッグで使うなら、true, true
// CH9350Lならば false, true
void hobo_device_setup(bool use_spi, bool use_uart) {
	if (LED1 < 31) {
		pinMode(LED1, OUTPUT);
		digitalWrite(LED1, LED_OFF);
	}
	if (LED2 < 31) {
		pinMode(LED2, OUTPUT);
		digitalWrite(LED2, LED_OFF);
	}
#if defined(BLE_LED)
	pinMode(BLE_LED, OUTPUT);
	digitalWrite(BLE_LED, LED_OFF); 
#endif
#if defined(READY_LED)
	pinMode(READY_LED, OUTPUT);
	digitalWrite(READY_LED, LED_OFF); 
#endif

#if defined(PIN_NEOPIXEL_VCC)	
	pinMode(PIN_NEOPIXEL_VCC, OUTPUT);
	digitalWrite(PIN_NEOPIXEL_VCC, 0);	// NeoPixelオフ。
#endif

#ifdef USE_NEOPIXEL
	neo_pixel_begin();
#endif

//
// QTPy-m0   Serial1 : SERCOM0, SPI : SERCOM2
// XIAO-m0   Serial1 : SERCOM4, SPI : SERCOM0
// samd21
#if defined (ADAFRUIT_QTPY_M0) || defined(ARDUINO_SEEED_XIAO_M0)
	uint32_t pm = PM_APBCMASK_ADC | PM_APBCMASK_AC | PM_APBCMASK_DAC | PM_APBCMASK_I2S | PM_APBCMASK_PTC |
			PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | 
			PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 | PM_APBCMASK_TC6 | PM_APBCMASK_TC7;

	#if defined(ADAFRUIT_QTPY_M0)
		pm |= PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4  | PM_APBCMASK_SERCOM5;
		if (!use_uart)
			pm |= PM_APBCMASK_SERCOM0;	// Serial1を使わない
		if (!use_spi)
			pm |= PM_APBCMASK_SERCOM2;	// SPIを使わない。
	#else // XIAO-m0
		pm |= PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM5;
		if (!use_uart)
			pm |= PM_APBCMASK_SERCOM4;	// Serial1を使わない
		if (!use_spi)
			pm |= PM_APBCMASK_SERCOM0;	// SPIを使わない。
	#endif
	PM->APBCMASK.reg &= ~pm;

#elif defined(NRF52_SERIES)

#elif defined(ARDUINO_ARCH_RP2040)
  
#elif defined(__AVR_ATmega32U4__)
	cli();
	wdt_disable();
	ADCSRA = ADCSRA & 0x7f;
	ACSR = ACSR | 0x80;
	if (use_spi)
		PRR0 = _BV(PRTWI) | _BV(PRTIM1) | _BV(PRADC);	
	else
		PRR0 = _BV(PRTWI) | _BV(PRTIM1) | _BV(PRADC) | _BV(PRSPI);  
	if (use_uart)
		PRR1 = _BV(PRTIM4) | _BV(PRTIM3);
	else
		PRR1 = _BV(PRTIM4) | _BV(PRTIM3) | _BV(PRUSART1); 
	sei();
#else
	#error "Unsupported board."
#endif
}