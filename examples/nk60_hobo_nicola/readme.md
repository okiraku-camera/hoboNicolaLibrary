## Hobo-nicola keyboard implementation for nk60 PCB rev 3 / nk61 PCBs using RP2040.

To build this software you will need:
 * nk60 (revision3) or nk61 PCBs. https://okiraku-camera.tokyo/blog/?p=16074
 * Arduino-pico : Version 3.9.5 
 * Adafruit TinyUSB Library for Arduino : Version 3.2.0
 * hoboNicola Library : Version 1.7.9
 
 In Arduino Board configuration : 
  * Board: Generic RP2040
  * USB Stack: Adafruit TinyUSB
  * CPU Speed: 50 MHz is preferable (less current consumption).

When updating the firmware, drop the .uf2 file onto the drive created by the UF2 bootloader.
The serial-based firmware update used in Arduino is not possible because built-in CDC is disabled in prog.


