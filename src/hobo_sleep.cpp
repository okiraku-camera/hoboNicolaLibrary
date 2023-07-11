/**
 * 
  hobo_sleep.cpp sleeping while usb-suspended.
  Copyright (c) 2022 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	hoboNicola	1.6.1   support AVR(atmega32u4), samd21(xiao-m0, QTPy-m0), nrf52840(isp1807 MB).
				1.6.2   change constants for AVR.
*/

#include "hobo_sleep.h"

#if defined(__AVR_ATmega32U4__)
#include <avr/sleep.h>
#include <avr/wdt.h>

ISR(WDT_vect) { 
	wdt_disable();
}
// timeout値に応じてWDTプリスケーラの値を選択する。
void enter_sleep(int16_t timeout_ms) {
	uint8_t wdt_prcl;
	if (timeout_ms != 0) {
		if (timeout_ms <= 16)
			wdt_prcl = 0;	// 16msec.
		else if (timeout_ms <= 32)
			wdt_prcl = _BV(WDP0);	// 32msec.
		else if (timeout_ms <= 64)
			wdt_prcl = _BV(WDP1);	// 64msec.
		else if (timeout_ms <= 125)
			wdt_prcl = (_BV(WDP1) | _BV(WDP0));	// 125msec.
		else if (timeout_ms <= 250)
			wdt_prcl = _BV(WDP2);								// 250msec.
		else if (timeout_ms <= 500)
			wdt_prcl =(_BV(WDP2) | _BV(WDP0));	// 500msec.
		else if (timeout_ms <= 1000)
			wdt_prcl = (_BV(WDP2) | _BV(WDP1));	// 1000msec.
		else if (timeout_ms <= 2000)
			wdt_prcl = (_BV(WDP2) | _BV(WDP1) | _BV(WDP0));	// 2000msec.
		else if (timeout_ms <= 4000)
			wdt_prcl = _BV(WDP3);							// 4000msec.
		else if (timeout_ms <= 8000)
			wdt_prcl = (_BV(WDP3) | _BV(WDP1));	// 8000msec.
		else			
			wdt_prcl = (_BV(WDP2) | _BV(WDP1));	// 1000msec.

#if 0
		UDINT &= ~_BV(WAKEUPI);
		UDIEN  |= _BV(WAKEUPE);
    USBCON |= _BV(FRZCLK);
#endif
		wdt_reset();
		WDTCSR = _BV(WDCE) | _BV(WDE);
		WDTCSR = _BV(WDIE) | wdt_prcl;
		set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
	} else
		set_sleep_mode(SLEEP_MODE_IDLE); 
  sleep_enable(); 
  sei();
  sleep_cpu(); 
  cli();
  sleep_disable();
	if (timeout_ms != 0) {
	  wdt_disable();
#if 0
		UDINT &= ~_BV(WAKEUPI);
		UDIEN &= ~_BV(WAKEUPE);
    USBCON &= ~_BV(FRZCLK);
#endif
	}	
	sei();	
}
#endif //  AVR

#if defined(ARDUINO_ARCH_SAMD)

static volatile bool rtc_overflow = false;
bool rtc_timeup() { return rtc_overflow; }

void RTC_Handler() {
	RTC->MODE1.INTFLAG.reg = RTC_MODE1_INTFLAG_OVF;
	RTC->MODE1.CTRL.reg &= ~RTC_MODE1_CTRL_ENABLE;	// disable RTC
	while (RTC->MODE1.STATUS.reg & RTC_STATUS_SYNCBUSY) ; 
	rtc_overflow = true;
}

// SYSCTRL->XOSC32K.reg の初期化(許可など)は、startup.c で行われているが、SYSCTRL_XOSC32K_RUNSTDBY はセットされていない。
static void start_rtc_count(int16_t count) {
	static bool rtc_ready = false;
	if (!rtc_ready) {
		PM->APBAMASK.reg |= PM_APBAMASK_RTC;
		GCLK->GENDIV.reg = GCLK_GENDIV_ID(2)|GCLK_GENDIV_DIV(4);
		while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) ;
	#ifdef CRYSTALLESS
		GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_RUNSTDBY );
	#else
		GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_RUNSTDBY  );
		SYSCTRL->XOSC32K.reg |= SYSCTRL_XOSC32K_RUNSTDBY;
	#endif
		while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) ;
		GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_GENDIV_ID(RTC_GCLK_ID)));
		while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) ;
		RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_SWRST;
		while (RTC->MODE1.STATUS.reg & RTC_STATUS_SYNCBUSY); 
		RTC->MODE1.CTRL.reg = RTC_MODE1_CTRL_MODE_COUNT16 | RTC_MODE1_CTRL_PRESCALER_DIV1;
		while (RTC->MODE1.STATUS.reg & RTC_STATUS_SYNCBUSY);  // wait sync
		NVIC_SetPriority(RTC_IRQn, 0);
		NVIC_EnableIRQ(RTC_IRQn); // enable RTC interrupt 
		rtc_ready = true;
	}
	rtc_overflow = false;
	RTC->MODE1.COUNT.reg = RTC_MODE1_COUNT_COUNT(0);
	RTC->MODE1.PER.reg = RTC_MODE1_PER_PER(count);
	RTC->MODE1.INTENSET.reg = RTC_MODE1_INTENSET_OVF ;
	RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_ENABLE;
	while (RTC->MODE1.STATUS.reg & RTC_STATUS_SYNCBUSY);  // wait sync
}

void enter_sleep(int16_t timeout_ms) {
	if (timeout_ms == 0) {
		PM->SLEEP.reg = 0;
		SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
		__WFI();
	} else {
		start_rtc_count(timeout_ms);
		SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk; 
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
		__WFI();
		SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; 
	}
}

#elif defined(NRF52_SERIES)
// そのうち
void enter_sleep(int16_t timeout_ms) {
	if (timeout_ms > 0)
		delay(timeout_ms);
	else
		delay(1);		
}
#elif defined(ARDUINO_ARCH_RP2040)
// こっちも
void enter_sleep(int16_t timeout_ms) {
	if (timeout_ms > 0)
		sleep_ms(timeout_ms);
	else
		sleep_ms(1);		
}
#endif





