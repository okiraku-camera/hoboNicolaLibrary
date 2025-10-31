## Hobo-nicola adapter implementation for rp2040 using Pico-pio-usb library

To build this software you will need:
 * Arduino-pico Version 5.4.2 
 * Adafruit TinyUSB Library for Arduino : Version 3.7.3
 * hoboNicola Library : Version 1.7.7
 
 In Arduino Board configuration : 
  * Board: your board, ex. Seeed xiao rp2040 
  * USB Stack: Adafruit TinyUSB
  * CPU Speed: 120 MHz.

To update firmware
 * UF2 method: Put your board in bootloader mode, then drag and drop the .uf2 file onto the drive that appears.
 * IDE method: Use Arduino IDE or VSCode to upload via the board’s USB serial port.
Both methods work—choose the one that fits your setup.
 

