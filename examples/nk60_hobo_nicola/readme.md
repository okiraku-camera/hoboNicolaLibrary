## Hobo-nicola keyboard implementation for nk60(rev3) and nk61 PCB.

To build this software you will need:
 * nk60 (revision3) or nk61 (revision1) PCB. https://okiraku-camera.tokyo/blog/?p=16074
 * Arduino-pico Version 5.4.2 
 * Adafruit TinyUSB Library for Arduino : Version 3.7.3
 * hoboNicola Library : Version 1.7.7
 
 In Arduino Board configuration : 
  * Board: Generic RP2040
  * USB Stack: Adafruit TinyUSB
  * CPU Speed: 50 MHz is preferable (less current consumption).

When updating the firmware, drop the .uf2 file onto the drive created by the UF2 bootloader, because Arduino's built-in CDC is disabled.
 

