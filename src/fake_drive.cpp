/**
  fake_drive.cpp  Fake small FAT12 Disk Drive Impl. of "Hobo-nicola keyboard and adapter library".
  Copyright (c) 2021 Takeshi Higasa

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php

 This program is based on "msc_ramdisk.cpp", example of TinyUSB. 
 Original copyright notices are below.
 *****************
 * msc_ramdisk.cpp
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "Arduino.h"
#include "fake_drive.h"

#if defined(USE_TINYUSB)
#include "Adafruit_TinyUSB.h"
Adafruit_USBD_MSC usb_msc;

static fd_notify_cb_t __attribute__((__aligned__(4))) fd_notify_cb = 0;
void set_fd_notify_cb(fd_notify_cb_t fn) {
	fd_notify_cb = fn;
}

// drive specs.
static const uint16_t phy_sector_size = 512;
static const uint8_t phy_sector_count = 16;
static const uint8_t file_contents[] = {  // size < 256 bytes.
  'h', 'o', 'b', 'o', 'n', 'i', 'c', 'o', 'l', 'a'  
};

static const uint8_t boot_sector[] = {  
    0xEB,0x3C,0x90, // JMP to offset 0x3e(boot code) and NOP. 
    'M', 'S', 'W', 'I', 'N', '4', '.', '1',  // OEMName
    0x00, (uint8_t)(phy_sector_size >> 8),
    0x01,   // Sector per Cluster = 1
    0x01,0x00, // Reserved Sector = 1
    0x01,   // Num Fats = 1.
    0x10,0x00, // Root entry count = 16 (32 * 16 = 512bytes)/
    phy_sector_count,0x00, // Total Sectors = phy_sector_count.
    0xf8,   // BPB media type = 0xf8.
    0x01,0x00, // Fat-size in sectors = 1.
    0x01,0x00, // Sectors per track = 1.
    0x01,0x00, // Numbers of head = 1.
    0,0,0,0,0,0,0,0, // Hidden Sectors and Total Sectors == 0.
    0x80,   // Drive Number = 0x80.
    0x00,   // reserved = 0.
    0x29,   // Boot signature = 0x29.
	0x89,0x67,0x45,0x23, // Volume Serial = 2345-6789
    'h', 'o', 'b', 'o', 'N', 'i', 'c', 'o', 'l', 'a', ' ',  // Volume Label = "hoboNicola "
    'F', 'A', 'T', '1', '2', ' ', ' ', ' ' // File System Type = "FAT12  "
};
static const uint8_t boot_signature[] = { 0x55, 0xaa};  // boot signature = 0xaa55
static const uint8_t fat_table[] = { 0xF8, 0xFF, 0xFF, 0xFF, 0x0F};
static const uint8_t root_directory[] = {
// Volume label
    'h', 'o', 'b', 'o', 'N', 'i', 'c', 'o', 'l', 'a', ' ',  // Name 
    0x08,    // Attribute = 0x08 (Volume ID)
    0x00,   // NTRes = 0.
    0x00,   // create time sub-seconds = 0
    0xcb,0xb2,0x56,0x54, // create time (0xb2cb) and date(0x5456)
    0x00,0x00, // Last access date = 0x0000 (not support).
    0x00,0x00, // First cluster High = 0x0000 (not support).
    0xcb,0xb2,0x56,0x54, // Last write time (0xb2cb) and date(0x5456)
    0x00,0x00, // First cluster Low = 0x0000.
    0,0,0,0, // File size in bytes.
// HOBONICOLA.BIN
    'H', 'O', 'B', 'O', 'N', 'I', 'C', 'O', 'B', 'I', 'N',  // Name 
    0x20,    // Attribute = 0x20 (archive)
    0x00,   // NTRes = 0.
    0x00,   // create time sub-seconds = 0
    0xcb,0xb2,0x56,0x54, // create time (0xb2cb) and date(0x5456)
    0x56,0x54, // Last access date = 0x5456
    0x00,0x00, // First cluster High = 0x0000 (not support).
    0xcb,0xb2,0x56,0x54, // Last write time (0xb2cb) and date(0x5456)
    0x02,0x00, // First cluster Low = 0x0002.
    sizeof(file_contents),0,0,0    // File size in bytes
};
static const uint8_t LBA_HOBONICO = 3;
static void debug_f(const char *format, ...) {
#if DEBUG
  char buf[100];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buf, sizeof(buf), format, ap);
  Serial1.print(buf);
  va_end(ap);
#endif
}

#if DEBUG
static void dump(uint8_t* buffer, uint32_t bufsize) {
  char tmp[20];
  uint8_t* p = buffer;
  for(uint32_t i = 0; i < bufsize; i += 16) {
    if (i % 16 == 0) {
      debug_f("%04lx  ", i);
      memset(tmp, 0, sizeof(tmp));
    }
    for(uint32_t c = 0; c < 16; c++) {
      if ( i + c > bufsize)
        break;
      debug_f("%02x ", *p);
      if (*p > 0x1f && *p < 0x80)
        tmp[c] = *p;
      else
        tmp[c] = '.';
      p++;
    }
    debug_f(" %s\n", tmp);
  }  
}
#else
#define dump(a,b)
#endif
// buffer size is defined at ....
//  ...Adafruit_TinyUSB_Arduino\src\arduino\ports\samd\tusb_config_samd.h
//  MSC Buffer size of Device Mass storage
//  #define CFG_TUD_MSC_EP_BUFSIZE 512
// buffer (array) defined at ...
// 	src/class/msc/msc_device.c
// 	CFG_TUSB_MEM_SECTION CFG_TUSB_MEM_ALIGN static uint8_t _mscd_buf[CFG_TUD_MSC_EP_BUFSIZE];

int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize) {
  const uint8_t * src = 0;
  int count = 0;
  memset(buffer, 0, bufsize);
  switch(lba) {
  case 0: // boot-sector.
    memcpy(&((uint8_t*)buffer)[510], boot_signature, sizeof(boot_signature));
    src = boot_sector;
    count = sizeof(boot_sector);
    break;  
  case 1:
    src = fat_table;
    count = sizeof(fat_table);
    break;  
  case 2:
    src = root_directory;
    count = sizeof(root_directory);
    break;  
  case LBA_HOBONICO:	// HOBONICO.BINのアドレス。
    src = file_contents;
    count = sizeof(file_contents);
    break;  
  default:
    break;    
  }  
  if (src) memcpy(buffer, src, count);
  
  debug_f("msc_read_cb(lba=%d, bufsize=%d)\n", lba, bufsize);
  dump((uint8_t*)buffer, bufsize);
  return bufsize;
}

int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
	if (!buffer || bufsize < 1)
		return bufsize;
	if (lba == LBA_HOBONICO && fd_notify_cb) {
		if (*buffer == 'h')				// none-NICOLA mode.	
			fd_notify_cb(false);
		else if (*buffer == 'H')	// NICOLA mode.
			fd_notify_cb(true);
	}
	debug_f("msc_write_cb(lba=%d, bufsize=%d)\n", lba, bufsize);
	dump((uint8_t*)buffer, bufsize);
	return bufsize;
}

void fake_drive_init() {
#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
  TinyUSB_Device_Init(0);
#endif
// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "Mass Storage", "1.0");  
  usb_msc.setCapacity(phy_sector_count, phy_sector_size);  // Set disk size
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, 0);   // Set callback
  usb_msc.setUnitReady(true);  // Set Lun ready (RAM disk is always ready)
  usb_msc.begin(); 
}

#else // defined(USE_TINYUSB)

void fake_drive_init() {}
void set_fd_notify_cb(fd_notify_cb_t fn) {}

#endif
