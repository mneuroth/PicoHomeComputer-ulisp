# PicoHomeComputer-ulisp

A version of the Lisp programming language for the [PicoHomeComputer](https://github.com/mneuroth/PicoHomeComputer) based [uLisp](http://www.ulisp.com/) and the [chipKIT](http://chipkit.net/) port of uLisp [ulisp-pic32-chipKIT](https://github.com/mneuroth/ulisp-pic32-chipKIT).

This implementation uses source code from other projects:
* Support for [real time clock DS1307](https://github.com/adafruit/RTClib)
* Support for [SRAM chip](https://github.com/dndubins/SRAMsimple)
* Support for [Ethernet chip ENC28J60](https://github.com/njh/EtherCard) or the [port for chipKIT](https://github.com/mneuroth/PicoHomeComputer-EtherCard)
* Support for [SD card](https://github.com/adafruit/SD) or [SdFat](https://github.com/greiman/SdFat)

Reference
---------

All functions of the original uLisp implementation are available, see the [original reference](http://www.ulisp.com/show?3L).

Additional functions to support the hardware of the PicoHomeComputer are:
* now: returns the current date and time, example: `(now)`
* setrtc: sets the date and time of the real time clock, example: `(setrtc 2021 5 22 15 53 05)`
* membread: read a byte from the given address of the RAM chip and returns the value, example: `(membread 1234)`
* membwrite: writes a given byte value into the given address of the RAM chip, example: `(membwrite 1234 42)`
* mem-string-read: read a null terminated string from the given address and with the given size form the RAM chip and returns the string, example: `(mem-string-read 1234 255)`
* mem-string-write: writes a null terminated string to the given address to the RAM chip, example: `(mem-string-write 1234 "some text")`
* info: returns some information about the system, example: `(info)`
* simpleshell: enables and disables the simple shell modus, example: `(simpleshell 1)`
* dir: shows the content of the root direcotry of the SD card, example: `(dir)`
* run: executes a file as lisp code, example: `(run "script.lsp")`
* load-text-file: returns a string which contains the content of the text file with the given name, example: `(load-text-file "text.txt)`
* save-text-file: saves the given text into the given text file name, example: `(save-text-file "text.txt" "content for the text file")`
* edi: starts the editor to modify the given string and returns the new value of the string, example: `(edi "some text to edit")`
