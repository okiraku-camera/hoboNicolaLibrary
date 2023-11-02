/**
 * 
  nk_rp_pwmled.cpp RP2040 PWM LED functions.
  Copyright (c) 2023 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	hoboNicola	1.7.2   support RP2040
*/

#if defined(ARDUINO_GENERIC_RP2040)
// select Generic_RP2040 on board config of Arduino-pico 
#include <Arduino.h>
#include "hardware/pwm.h"

static const int16_t pwm_wrap_value = 10000;	
static const int16_t pwm_min_level = 100;
static const int16_t pwm_max_level = pwm_wrap_value - 1;
static const int16_t pwm_upper_thr_level = 7000 ;
static const int16_t pwm_lower_thr_level = 4000 ;
static const int16_t pwm_level_step = 100;
static const int16_t pwm_level_step_slow = 60;
static const int16_t pwm_level_step_fast = 180;

static uint8_t pwm_gpio;
static volatile int pwm_level = pwm_min_level;
static volatile uint8_t pwm_dir = 1;	// 0 : dimmer, 1 : brighter, 2 : force off.

// wrap interrpt occurs on every pwm_wrap_value/1000000 sec.
extern "C" void isr_rp_pwm_wrap() {

	pwm_clear_irq(pwm_gpio_to_slice_num(pwm_gpio));
	pwm_set_gpio_level(pwm_gpio, pwm_level); 
	if (pwm_dir == 2) {
		if (pwm_level > 1)
			pwm_level -= pwm_level_step;
		else
			pwm_level = 1;
		return;
	}

	if (pwm_dir == 1) {
		if (pwm_level < pwm_upper_thr_level)
			pwm_level += pwm_level_step;
		else
			pwm_level += pwm_level_step_fast;
		if (pwm_level >= pwm_max_level) {
			pwm_dir = 0;
			pwm_level = pwm_max_level;
		}
	} else  {
		if (pwm_level > pwm_lower_thr_level)
			pwm_level -= pwm_level_step_fast;
		else
			pwm_level -= pwm_level_step_slow;	
		if (pwm_level <= pwm_min_level) {
			pwm_dir = 1;		
			pwm_level = pwm_min_level;
		}
	}
}


void rp_pwm_init(uint8_t gpio) {
	pwm_gpio = gpio;
	gpio_set_dir(gpio, true);
	gpio_set_function(gpio, GPIO_FUNC_PWM);
	pwm_config pwmconf = pwm_get_default_config();
	pwm_config_set_clkdiv_int(&pwmconf, F_CPU / 1000000);	// divide to 1MHz.
	pwm_config_set_wrap(&pwmconf, pwm_wrap_value);	
	pwm_init(pwm_gpio_to_slice_num(gpio), &pwmconf, false);
// pwm wrap interrupt.
	pwm_clear_irq(pwm_gpio_to_slice_num(gpio));
	irq_set_exclusive_handler(PWM_IRQ_WRAP, isr_rp_pwm_wrap);
	irq_set_enabled(PWM_IRQ_WRAP, true);
	pwm_set_irq_enabled(pwm_gpio_to_slice_num(gpio), true);
	pwm_set_enabled(pwm_gpio_to_slice_num(gpio), true);
}

// nicola_ledは同じ状態でも繰り返して呼び出される。
void rp_start_pwm() {
	if (pwm_dir == 2) {
		pwm_level = 1;
		pwm_dir = 1;
	}
}

void rp_stop_pwm() {
	pwm_dir = 2;
}

#endif