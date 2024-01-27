## Hobo-nicola keyboard implementation for nk60 PCB revision 3.

To build this software you will need:
 * nk80 PCB (revision2). https://okiraku-camera.tokyo/blog/?p=16074
 * Arduino-pico Version 3.6.0 
 * Adafruit TinyUSB Library for Arduino : Version 2.2.5
 * hoboNicola Library : Version 1.7.4
 
 In Arduino Board configuration : 
  * Board: Generic RP2040
  * USB Stack: Adafruit TinyUSB
  * CPU Speed: 50 MHz is preferable (less current consumption).

When updating the firmware, drop the .uf2 file onto the drive created by the UF2 bootloader, because Arduino's built-in CDC is disabled.
 

