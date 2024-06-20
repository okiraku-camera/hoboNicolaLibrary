// hid_wrap.h

#ifndef __HID_WRAP_H__
#define __HID_WRAP_H__

#include "Arduino.h"

#define REPORT_ID_KBD  1
#define REPORT_ID_CONSUMER  2
#define REPORT_ID_SYSTEM  3

const bool is_ble_connected();
void start_ble();
void stop_ble();

static const uint8_t REPORT_KEY_COUNT = 6;
typedef struct {
	uint8_t modifiers;
	uint8_t reserved;
	uint8_t keys[REPORT_KEY_COUNT];
} key_report_t;


typedef void (*led_callback_t)(uint8_t);

void send_hid_report(uint8_t id, const void* data, int len);
void set_hid_led_callback(led_callback_t fn);
bool hid_begin(const char* name = 0);


bool is_usb_suspended();
void usb_wakeup();

#define HID_DELAY_NORMAL	6
#define HID_DELAY_SHORT		3

void set_hid_output_delay(uint8_t msec);

#endif
