# Hobo-nicola keyboard and adapter library for Arduino environments.

## What's hobo-nicola 

The hobo-Nicola library is firmware for hardware adapters that allow general Japanese USB keyboard to be used as "almost NICOLA keyboard". By using this library, you can get (almost) NICOLA layout kayboard. And at the same time, you can use simultaneous typing method with Thumb-shift keys according to the NICOLA standard. Adapters (USB-USB, PS/2-USB) that incorporate the hobo-Nicola library are called hobo-Nicola adapters, and their circuits are publicly published.

The hoboNicola adapter behaves as a USB HID keyboard when connected to a host such as PC. The adapter uses the HID Usage ID (keycode) received from the physical keyboard to change the layout and perform simultaneous keystroke processing, etc., and then sends the keycode to the host.

This library can be used not only for Japanese USB keyboards, but also as adapters for PS/2 interface keyboards and US layout keyboards. In addition, it is also possible to make the firmware of the keyboard itself. You can build the almost NICOLA keyboard by incorporating this library into the micro controller of the keyboard PCB.

## What's NICOLA keyboard

The NICOLA keyboard is one of the keyboard standards for typing Japanese using PCs and WSs, established at the end of the 20th century. This standard inherits the input method of a dedicated word processor called ”OASYS", which had been manufactured by Fujitsu.

It features a unique key layout for typing the many Japanese kana and symbols with fewer keys. And it has a simultaneous typing method with the two Thumb-shift keys to select kanas, numbers and symbols from multiplexed keys.

For details, please refer to the following site (written in Japanese). http://nicola.sunicom.co.jp/spec/jisdraft.htm

## About Build

The hoboNicolaLibrary may be installed  as a library in Arduino IDE. Implementations using various arduino-boards and USB-Host modules are in the examples directory. Select an appropriate example sketch according to the haardware, modify (if needed), build, and write to the board.

My build environment is the traditional Arduino IDE (1.8.19 recommended) or VSCode with arduino-cli enabled Arduino-extension.

## Arduino boards

The Arduino boards I've used to build the adapter until now  are:
### AVR
Arduino Leonardo (ATmega32u4 +5V/16MHz)

SparkFun Pro Micro (ATmega32u4 +3.3V/8MHz and +5V/16MHz)

### SAMD21
Adafruit QT Py M0 (SAMD21E18)

SeeedStudio XIAO SAMD21 (SAMD21G1)

### nRF52840
SeeedStudio XIAO nRF52840

SwitchScience ISP1807 Micro Board

### RP2040
SeeedStudio XIAO RP2040

Adafruit KB2040

Raspberry Pi Pico

## Examples
The three implementations for the adapter shown below work with four different microcontrollers(ATMega32U4, ATSAMD21, nRF52840 and RP2040). Requires installation of the appropriate Arduino core-package or Board-support-package. TinyUSB library is required when choosing non-AVR controllers.

### ch9350_hobo_nicola
It targets adapter implementations using the CH9350L as a USB host controller. The code for interfacing with the CH9350L is included in this library.

### usb_hobo_nicola
It targets adapter implementations using the MAX3421EE (USB Host Shield) as a USB Host Controller.  Using [felis/USB_Host_Shield_2.0](https://github.com/felis/USB_Host_Shield_2.0) (slightly modified and included in this library).

### rp_hobo_nicola
It targets the implementation of an adapter that enables USB device (PC interface) and USB host (keyboard interface) on single RP2040 only.
Using [sekigon-gonnoc/Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) and [Adafruit_TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino) as USBHost. By now, dedicated to RP2040.

### ps2_hobo_nicola
This is an early implementation of an adapter, intended to take advantage of PS/2 keyboards. The only applicable microcontroller is the ATMega32U4.

### xd87
This is an implementation example of applying the hoboNicola library to the XD87 keyboard PCB (https://kprepublic.com/). The target is the ATMega32U4 (+5V/16MHz) on the PCB and I built it  as Arduino Leonardo or Pro Micro.  Short description of xd87 hoboNicola keyboard is [here](./xd87/xd87.md).

### xd64
This is also  applying the hoboNicola library to the XD64(rev3) keyboard PCB (https://kprepublic.com/). This PCB is for making a GH60 compatible 60% keyboard. Short description of xd64 hoboNicola keyboard is [here](./assets/xd64/xd64.md).


## Key layout

This library provides the following keyboard layouts similar to the NICOLA standard, using Windows Japanese edition.

### japanese keyboard
![](./assets/hobonicola-basic-layout.png)

### us layout keyboard
![](./assets/hobonicola-basic-layout-(us).png)

###

