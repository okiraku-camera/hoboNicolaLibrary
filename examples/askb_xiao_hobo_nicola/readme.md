## Hobo-nicola keyboard implementation for ASkeyboard (Seeed Seeeduino SAMD21).

*askb_xiao_hobo_nicola*

Copyright (c) Takeshi Higasa. Licensed under the GPL3.0.

version 1.7.9 Jan. 18 2026 

* To build this software you will need :
  * ASkeyboard (Seeed xiaoduino SAMD21 version). https://okiraku-camera.tokyo/blog/?p=14991
  * core package : Seeed samd 1.8.5 (https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json)
  * Adafruit TinyUSB Library for Arduino : Version 3.2.0
  * hoboNicola Library : Version 1.7.9
 
* In Arduino Board configuration : 
  * Board: Seeeduino XIAO
  * USB Stack: Adafruit TinyUSB

* Firmware Update : 
  * Arduino CDC method.

## Current layout (HID base) : 

  [Pause] [Han/Zen] F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 <br/>
  [Esc] 1 2 3 4 5 6 7 8 9 0 - ^ \ [BackSpace] <br/>
  [Tab]  Q W E R T Y U I O P @ [ <br/>
  [L-Ctrl] A S D F G H J K L ; : ] [Enter]<br/>
  [L-Shift] Z X C V B N M , . / _ [R-Shift]<br/>
  [Caps][Gui][Alt][Hira][L-Oya][R-Oya][Space][R-Ctrl][Fn]<br/>
  &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[Caps][Hira] <br/>

  L-Oya = Muhenkan
  R-Oya = Henkan