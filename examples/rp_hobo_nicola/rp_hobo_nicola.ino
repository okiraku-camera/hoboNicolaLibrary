/**
	rp_hobo_nicola.ino Main sketch of "Hobo-nicola usb/ble adapter for RP2040 using Pico_Pio_USB".
	Copyright (c) 2022-2023 Takeshi Higasa

	This file is part of "Hobo-nicola keyboard and adapter".

	"Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	"Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	version 1.7.0  Jul. 1, 2023.
		(Pico-pio-usb 0.5.2, Arduino-pico 3.3.0, Adafruit Tinyusb 2.2.1) 
*/

#include "Adafruit_USBH_Host.h"
#include "hobo_nicola.h"
#include "hobo_board_config.h"
#include "pio_usb.h"

static const uint8_t usbh_dp_gpio = 2;
Adafruit_USBH_Host USBHost;

typedef struct {
	uint8_t flag;
	uint8_t code;
} queue_key_t;
static queue_t key_event_queue;

typedef struct {
	uint8_t report_id;       // 0xff
	uint8_t dev_addr;  // = 0xff;
	uint8_t instance;  // = 0xff;
	bool mounted; // = false;
	uint16_t report_length; // = 0;
} device_info_t;

device_info_t device_info;

void init_device_info() {
	device_info = { 0xff, 0xff, 0xff, false, 0 };
}

static const uint8_t prev_report_size = 16;
uint8_t prev_report[prev_report_size];
// led state value on connected keyboard.
uint8_t kbd_led_state = 0;
uint8_t prev_kbd_led_state = 0xff;

// core1 setup.
void setup1() {
	init_device_info();
	delay(20);  // wait a while for core0 setup() done.
	pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
	pio_cfg.pin_dp = usbh_dp_gpio;
	USBHost.configure_pio_usb(1, &pio_cfg);
	USBHost.begin(1);
}

// core1's loop
unsigned long led_sync_time = 0;
void loop1() {
	USBHost.task();
	// sync keyboard leds.
	if (led_sync_time == 0)
		led_sync_time = millis();
	else {
		if (millis() - led_sync_time > 150) { // every 150ms.
			if (device_info.report_id == 0xff) {
				if (device_info.mounted) led_toggle_ready();
			} else if ((kbd_led_state != prev_kbd_led_state)) {
				static uint8_t rep[2];
				rep[0] = kbd_led_state;
				tuh_hid_set_report(device_info.dev_addr, device_info.instance, device_info.report_id, HID_REPORT_TYPE_OUTPUT, &rep, 1);
				prev_kbd_led_state = kbd_led_state;
				led_ready(true);	// 点いてないことがあるので。
			}
			led_sync_time = millis();            
		}
	}
}

// hid device mounted.
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
	memset(prev_report, 0, prev_report_size);
	device_info.mounted = true;
	static const uint8_t arr_count = 5;
	tuh_hid_report_info_t report_info[arr_count] ;
	uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, arr_count, desc_report, desc_len);
	for ( uint8_t i = 0; i < report_count; i++ ) {  // is keyboard ? 
		if (device_info.report_id == 0xff && report_info[i].usage_page == 0x0001 && report_info[i].usage == 0x06) {
			led_ready(true);
			device_info.report_id = report_info[i].report_id;
			device_info.dev_addr = dev_addr;
			device_info.instance = instance;
			tuh_hid_receive_report(dev_addr, instance); // start receive input reports.
			break;
		}
	}
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
	led_ready(false);
	init_device_info();
}

void send_key_event(uint8_t code, uint8_t flag) {
	queue_key_t key;
	key.flag = flag;
	key.code = code;
	queue_try_add(&key_event_queue, &key); // キューがフルなら入らないだけ。 
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
	if (!report || len < 1) return;
	const uint8_t* rep = report;    
	if (device_info.report_id != 0) {
		if (device_info.report_id == 0xff)  // not a keyboard.
			return;    
		if (*report != device_info.report_id) { // other device.
			tuh_hid_receive_report(dev_addr, instance);
			return;
		}
		rep++;
		len--;
	}
	if (rep[2] == 0x01) {  // roll-over error.
		tuh_hid_receive_report(dev_addr, instance);
		return;
	}
	if (device_info.report_length == 0)
		device_info.report_length = min(prev_report_size, len);

	// check modifiers.
	uint8_t change = rep[0] ^ prev_report[0];
	if (change != 0) {
		uint8_t mask = 1;
		for(uint8_t i = 0; i < 8; i++, mask <<= 1)
			if (change & mask) send_key_event((uint8_t)( HID_MODIFIERS + i), (rep[0] & mask) != 0);
	}
	// check normal keys
	for(uint8_t i = 2; i < device_info.report_length; i++) { 
		bool cur_match = false;
		bool prev_match = false;
		for(uint8_t n = 2; n < device_info.report_length; n++) {
			if (rep[i] == 0 || rep[i] == prev_report[n]) cur_match = true;
			if (prev_report[i] == 0 || prev_report[i] == rep[n]) prev_match = true;
		}
		if (!cur_match)		send_key_event(rep[i], 1);
		if (!prev_match)	send_key_event(prev_report[i], 0);
	}
	memcpy(prev_report, rep, device_info.report_length);
	tuh_hid_receive_report(dev_addr, instance);
}

/** core0 hoboNicola **/
// Function keys with Fn-key pressed.
static const uint16_t fn_keys[] PROGMEM = {
	HID_S | WITH_R_CTRL,	FN_SETUP_MODE,
	HID_M,        				FN_MEDIA_MUTE,
	HID_COMMA,						FN_MEDIA_VOL_DOWN,
	HID_PERIOD,						FN_MEDIA_VOL_UP,
	HID_U_ARROW,					HID_PGUP,
	HID_D_ARROW,					HID_PGDOWN,
	HID_R_ARROW,					HID_END,
	HID_L_ARROW,					HID_HOME,
	HID_ENTER,						FN_MEDIA_PLAY_PAUSE,
	HID_IME_OFF,					HID_CAPS,								// Fn + ImeOff (Caps) = CapsLock
	HID_ESCAPE | WITH_R_CTRL,  FN_SYSTEM_SLEEP,   // Ctrl + App + Esc  
	0, 0
};

class rp2040_hobo_nicola : public HoboNicola {
	public:
		rp2040_hobo_nicola() {  }
		const uint16_t* fn_keys_table() { return fn_keys; }
};

rp2040_hobo_nicola hobo_nicola;

void setup() { 
	hobo_device_setup();
	queue_init(&key_event_queue, sizeof(queue_key_t), 16);
	HoboNicola::init_hobo_nicola(&hobo_nicola, "rp_hobo_nicola");
	delay(100);
	hobo_nicola.releaseAll();
}

bool suspended = false;
void loop() {
	if (is_usb_suspended()) {
		if (!suspended) {
			kbd_led_state = 0;
			delay(200);
			all_led_off();
			if (Settings().is_use_kbd_suspend())
				multicore_reset_core1();  // stop core1
				suspended = true;
		}
		delay(1000);
	} else {
		if ( suspended ) {
			suspended = false;
			delay(10);
			if (Settings().is_use_kbd_suspend())
				watchdog_reboot(0, 0, 10);  // reboot after a while. 
			prev_kbd_led_state = 0xff;  
		}
		hobo_nicola.idle();
		kbd_led_state = hobo_nicola.get_hid_led_state();
		queue_key_t key;
		while (queue_try_remove(&key_event_queue, &key))
			hobo_nicola.key_event(key.code, key.flag);
		delay(2);
	}
}
