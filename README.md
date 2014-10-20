amtdoor2
========

This repository tracks files for AMT Door 2, and RFID based lock system in use at the [Ace Monster Toys](http://acemonstertoys.org/) hackspace in Oakland, CA. A complete description of the system these files are part of, [see the description on the AMT Wiki](http://wiki.acemonstertoys.org/Door#AMT_Door_2).

Included files:
* authCode - code to run on a raspberry pi or similar auth server
* housing - 2D drawings for assembly of the door tag reader
* hwTestCode - simplified panel mcu code to test clock settings
* panelMcuCode - firmware to run on the panel mcu that interprets RFID signals and accepts opening requests from the auth server
* physical - Eagle schematic/boards for the panel mcu


## Setting up on a 16u4:

to flash a new atmega16u4 for this code using an ISP (without the bootloader):

```
$ avrdude -p atmega16u4 -P usb -c avrispmkii -u -U lfuse:w:0xfc:m -U hfuse:w:0xd0:m -U efuse:w:0xc3:m
$ make MCU=atmega16u4
$ avrdude -p atmega16u4 -P usb -c avrispmkii -U flash:w:main.hex
```

to load the bootloader first using an ISP:

```
need to set fuses. we will copy from adafruit atmega32u4 dev board.
adafruit sets to: (E:C3, H:D0, L:FC) (avrdude-- has a typo)

Extended (0 is set):
7 1 Reserved
  1 Reserved
  0 Reserved
  0 Reserved
  0 HWBE          Hardware Boot Enable
  0 BODLEVEL2(1)  Brown-out Detector trigger level
  1 BODLEVEL1(1)  Brown-out Detector trigger level
0 1 BODLEVEL0(1)  Brown-out Detector trigger level

High (0 is set):
7 1 OCDEN         Enable OCD
  1 JTAGEN        Enable JTAG
  0 SPIEN         Enable Serial Program and Data Downloading
  1 WDTON         Watchdog Timer always on
  0 EESAVE        EEPROM memory is preserved through the Chip Erase
  0 BOOTSZ1       Select Boot Size (see Table 28-7 for details)
  0 BOOTSZ0       Select Boot Size (see Table 28-7 for details)
0 0 BOOTRST       Select Bootloader Address as Reset Vector

Low (0 is set):
7 1 CKDIV8        Divide clock by 8
  1 CKOUT         Clock output
  1 SUT1          Select start-up time
  1 SUT0          Select start-up time
  1 CKSEL3        Select Clock source
  1 CKSEL2        Select Clock source
  0 CKSEL1        Select Clock source
0 0 CKSEL0        Select Clock source
```


to compile CDC bootloader (from lufa-lib trunk):
for BOOTSZ1 = BOOTSZ0 = 0. see datasheet table 27-8.

```
$ make MCU=atmega16u4 BOOT_START=0x1800 F_CPU=16000000
$ mv BootloaderCDC.hex BootloaderCDC_16u4.hex
$ avrdude -p atmega16u4 -P usb -c avrispmkii -U flash:w:BootloaderCDC_16u4.hex
```
