/**
 * 
  hid_wrap.cpp  USB and BLE HID wrapper module of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2022 Takeshi Higasa, okiraku-camera.tokyo
  
  This file is part of "Hobo-nicola keyboard and adapter library".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.
  
    Included in...
			hoboNicola 1.6.1.	Feb. 22. 2022	Initial version. BLE and HID support.
			hoboNicola 1.7.0  Mar. 3.  2023 Fix System and Consumer mode support.
*/

#include "hid_wrap.h"

#if defined(USE_TINYUSB)
#include "Adafruit_TinyUSB.h"
static Adafruit_USBD_HID _hid;
static uint8_t usb_led_state = 0;

#if defined(ARDUINO_NRF52_ADAFRUIT)
#include "bluefruit.h"
#endif

#else	// AVR arduino
#include "HID.h"
#endif

static uint8_t hid_output_delay = HID_DELAY_NORMAL;	// default value is modest
#define ble_send_delay_ms 2

static led_callback_t callback_fn = 0;

void set_hid_led_callback(led_callback_t fn) {
	callback_fn = fn;
}

//
// Report Descriptors.
static const uint8_t _hidReportDescriptor[] PROGMEM = {
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x06, // USAGE (Keyboard)
	0xa1, 0x01, // COLLECTION (Application)
	0x85, REPORT_ID_KBD, //   REPORT_ID 

	0x05, 0x07, //   USAGE_PAGE (usage = keyboard page)
	0x19, 0xe0, //   USAGE_MINIMUM  modifiers.
	0x29, 0xe7, //   USAGE_MAXIMUM
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x01, //   LOGICAL_MAXIMUM (1)
	0x95, 0x08, //   REPORT_COUNT (8)
	0x75, 0x01, //   REPORT_SIZE (1) 
	0x81, 0x02, //   INPUT (Data,Var,Abs) 
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x08, //   REPORT_SIZE (8) 
	0x81, 0x01, //   INPUT (Cnst,Var,Abs)

	0x95, 0x05, //   REPORT_COUNT (5)
	0x75, 0x01, //   REPORT_SIZE (1) 
	0x05, 0x08, //   USAGE_PAGE (LEDs)
	0x19, 0x01, //   USAGE_MINIMUM (1) 
	0x29, 0x05, //    USAGE_MAXIMUM (5) 
	0x91, 0x02, //   OUTPUT (Data,Var,Abs) // LED report
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x03, //   REPORT_SIZE (3bit)
	0x91, 0x01, //   OUTPUT (Cnst,Var,Abs) 

	0x95, REPORT_KEY_COUNT, //   
	0x75, 0x08, //   REPORT_SIZE (8bit)
	0x15, 0x00, //   LOGICAL_MINIMUM (0) 
	0x26, 0xdf, 0x00, //   LOGICAL_MAXIMUM (221)
	0x05, 0x07, //   USAGE_PAGE (Keyboard)
	0x19, 0x00, //     USAGE_MINIMUM  keys,
	0x2a, 0xdf, 0x00,//     USAGE_MAXIMUM
	0x81, 0x00, //   INPUT (Data,Ary,Abs)
	0xc0,    // END_COLLECTION

/**
	Consumer section.
 Control Types.....
	OSC : One Shot Control.
	OOC : On Off Control
	RTC : Re-Trigger Control
	MC , LC ...
*/
	0x05, 0x0C,  // USAGE_PAGE (Consumer device)
	0x09, 0x01, // USAGE (Consumer Control)
	0xA1, 0x01, 
	0x85, REPORT_ID_CONSUMER,
	0x09, 0xCD, // Usage PLAY/PAUSE (OSC)
	0x09, 0xCE, // Usage PLAY/SKIP(OSC)
	0x09, 0xB3, // Usage Fast Forward(OOC)
	0x09, 0xB4, // Usage Rewind(OOC)
	0x09, 0xB5, // Usage ScanNext (OSC)
	0x09, 0xB6, // Usage ScanPrev (OSC)
	0x09, 0xB7, // Usage STOP (OSC)
	0x09, 0xE9, // Usage VolUp (RTC)
	0x09, 0xEA, // Usage VolDown (RTC)
	0x09, 0xE2, // Usage Mute (OOC)
	0x75, 0x01, // REPORT_SIZE  1bit.
	0x95, 0x0a, // REPORT_COUNT 10 items.
	0x81, 0x02, //INPUT (Data,Var, Abs)
	0x95, 0x06, // REPORT COUNT = 6bit (filler)
	0x81, 0x01, //INPUT (Data,Const, Abs)
	0xC0,
// System control section.
	0x05, 0x01, // GENERIC Desktop
	0x09, 0x80, // SYSTEM Control.
	0xA1, 0x01, 
	0x85, REPORT_ID_SYSTEM,
	0x09, 0x81, // System PowerDown
	0x09, 0x82, // System Sleep
	0x09, 0x83, // System WakeUp.
	0x15, 0x00, // LOGICAL_MINIMUM (0) 
	0x25, 0x01, // LOGICAL_MAXIMUM (1) 
	0x75, 0x01, // REPORT_SIZE  1bit.
	0x95, 0x03, // REPORT_COUNT 3 items.
	0x81, 0x02, // INPUT (Data,Var,Abs) 
	0x95, 0x0d, // REPORT_COUNT (filler)
	0x81, 0x01, // INPUT (Data,Const, Abs) 
	0xc0 // END_COLLECTION    
};

#if defined(ARDUINO_NRF52_ADAFRUIT)
// Bluefruit BLE
BLEDis bledis;

// BLE LED通知
static void blehid_keyboard_output_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
	if (callback_fn && len > 0)
		callback_fn(data[0]);
}

static void start_advertising();
static void ble_event_callback(ble_evt_t* evt);

class BleHid : public BLEHidGeneric {
	bool _begin;
public:
	BleHid() :BLEHidGeneric(3, 1, 0) {	// num_input, num_output, num_feature
		state = Disconnected;
		_begin = false;
	}	

	err_t begin(void) {
		if (_begin) {
			Bluefruit.Advertising.restartOnDisconnect(true);
			Bluefruit.Advertising.start(0);
			return ERROR_NONE;
		}
		uint16_t input_len [] = { sizeof(key_report_t), 2, 2 };	// key_report, consumerとsystemは２バイト
		uint16_t output_len[] = { 1 };	// LED.
		setReportLen(input_len, output_len, NULL);
		enableKeyboard(true);
		setReportMap(_hidReportDescriptor, sizeof(_hidReportDescriptor));
		setOutputReportCallback(REPORT_ID_KBD, blehid_keyboard_output_cb);
	
		err_t err = BLEHidGeneric::begin();
		Bluefruit.Periph.setConnInterval(9, 12);
		Bluefruit.setEventCallback(ble_event_callback);
		_begin = true;
		start_advertising();
		return err;
	}

	void sendReport(uint8_t id, const void* data, int len) {
		inputReport( Bluefruit.connHandle(), id, data, len);
	}
	
	enum {
		Disconnected = 0,
		Connecting,
		Connected,
		Disconnecting,
	} state;
};

BleHid ble_hid;

// BLE_ADV_INTERVAL_FAST_DFLT間隔で開始してBLE_ADV_FAST_TIMEOUT_DFLT(30sec)の期間に接続しなければ、BLE_ADV_INTERVAL_SLOW_DFLTの間隔を使う。(20～	152.5msec)

static void start_advertising() {
	Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
	Bluefruit.Advertising.addTxPower();
	Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

	Bluefruit.Advertising.addService(ble_hid);
	Bluefruit.Advertising.addName();
	Bluefruit.Advertising.restartOnDisconnect(true);
	Bluefruit.Advertising.setInterval(BLE_ADV_INTERVAL_FAST_DFLT, BLE_ADV_INTERVAL_SLOW_DFLT);
	Bluefruit.Advertising.setFastTimeout(BLE_ADV_FAST_TIMEOUT_DFLT);
	Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

const bool is_ble_connected() { return ble_hid.state == BleHid::Connected; }

//#define BLE_GAP_EVT_BASE       0x10       /**< GAP BLE Event base. */
//#define BLE_GAP_EVT_LAST       0x2F       /**< GAP BLE Event last. */
static void ble_event_callback(ble_evt_t* evt) {
	switch(evt->header.evt_id)  {
	case BLE_GAP_EVT_CONNECTED:
		if (callback_fn)
			callback_fn(0);
		ble_hid.state = BleHid::Connected;
		break;
	case BLE_GAP_EVT_DISCONNECTED:	// この通知がこないことがある。自分で切断するしかない。
		ble_hid.state = BleHid::Disconnected;
		if (callback_fn)
			callback_fn(usb_led_state);
		// セントラルから切られたとき、アドバタイジングやめる。			
		if (Bluefruit.Advertising.isRunning()) {
			Bluefruit.Advertising.restartOnDisconnect(false);
			delay(100);
			Bluefruit.Advertising.stop();
		}
		break;
	};
}

#if defined(ARDUINO_Seeed_XIAO_nRF52840)
	static const char myname[] = "hoboNicola BLE(XIAO)";
#else
	static const char myname[] = "hoboNicola BLE";
#endif

static void init_ble() {
	Bluefruit.begin();
	Bluefruit.Periph.setConnInterval(9, 16);
	Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
	Bluefruit.setName(myname);
	bledis.setManufacturer("unknown");
	bledis.setModel(myname);
	bledis.begin();		  
}

void start_ble() {
	if (ble_hid.state != BleHid::Disconnected) {
		stop_ble();
		delay(200);
	}
	ble_hid.begin();
}

// 接続していれば切断するし、アドバタイジングしていれば止める。
void stop_ble() {
	Bluefruit.Advertising.restartOnDisconnect(false);
	delay(100);
	Bluefruit.Advertising.stop();
	delay(100);
	ble_hid.state = BleHid::Disconnecting;
	uint16_t handle = Bluefruit.connHandle();
	if (Bluefruit.connected(handle))
		Bluefruit.disconnect(handle);
	delay(100);
	if (!is_ble_connected()) {
		ble_hid.state = BleHid::Disconnected;
		return;
	}
}
#else
const bool is_ble_connected() { return false; }
#endif

#if defined(USE_TINYUSB)
static void hid_report_callback_ada(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	if (report_id == REPORT_ID_KBD && report_type == HID_REPORT_TYPE_OUTPUT && buffer && bufsize > 0)
		usb_led_state = buffer[0];	// buffer[0]にLED状態が入っている。(bsp 1.0.0～) 前は違った。
	if (!is_ble_connected() && callback_fn)
		callback_fn(usb_led_state);
}
#else
static void hid_report_callback(uint8_t request, uint8_t data) {
	if (request == HID_SET_REPORT && callback_fn)
		callback_fn(data);
}
#endif

// send_report();
#if defined(USE_TINYUSB)
	#define _send_report(id,data,len)	_hid.sendReport(id, data, len)
#else
	#define _send_report(id,data,len)	HID().SendReport(id, data, len)
#endif
void send_hid_report(uint8_t id, const void* data, int len) {
	static unsigned long last_send = 0;
#if defined(ARDUINO_NRF52_ADAFRUIT)
	if (is_ble_connected()) {
		if (last_send != 0 && (millis() - last_send) < ble_send_delay_ms)
		delay(ble_send_delay_ms);
		ble_hid.sendReport(id, data, len);
		return;
	}
#endif

#if defined(USE_TINYUSB)
	if (!USBDevice.mounted())
		return;
#endif

	if (last_send != 0 && (millis() - last_send) < hid_output_delay)
		delay(hid_output_delay);
	_send_report(id, data, len);
	last_send = millis();
}

// how to set country code in HID Descriptor (bCountryCode) ?
bool hid_begin(const char* name) {
#if defined(ARDUINO_NRF52_ADAFRUIT)
	init_ble();	// valid only nrf52
#endif
#if defined(USE_TINYUSB)
	_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
	_hid.setPollInterval(2);
	_hid.setReportDescriptor(_hidReportDescriptor, sizeof(_hidReportDescriptor));
//	_hid.setStringDescriptor(name);
	_hid.setReportCallback(NULL, hid_report_callback_ada);
	return _hid.begin();

#else
	static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
	HID().setPollingInterval(3);
	HID().AppendDescriptor(&node);
	HID().set_request_callback(hid_report_callback);
	return true;
#endif
}

bool is_usb_suspended() {
#if defined(USE_TINYUSB)
	return USBDevice.suspended();
#else
	return USBDevice.isSuspended();
#endif
}

void usb_wakeup() {
#if defined(USE_TINYUSB)
	if (USBDevice.suspended())
		USBDevice.remoteWakeup();
#else
	if (USBDevice.isSuspended())	
		USBDevice.wakeupHost();
#endif
}

void set_hid_output_delay(uint8_t msec) {
	hid_output_delay = msec;
}
