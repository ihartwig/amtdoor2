amtdoor2
========


Using a 16u4:

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

avrdude -p atmega16u4 -P usb -c avrispmkii -u -U lfuse:w:0xfc:m -U hfuse:w:0xd0:m -U efuse:w:0xc3:m
avrdude -p atmega16u4 -P usb -c avrispmkii -U flash:w:BootloaderCDC_16u4.hex
avrdude -p atmega16u4 -P usb -c avrispmkii -U flash:w:main.hex




actually set to: -u -U lfuse:w:0xfc:m -U hfuse:w:0xf3:m -U efuse:w:0xf8:m

avrdude -p atmega16u4 -P usb -c avrispmkii -U flash:w:main.hex -U lfuse:w:0xfc:m -U hfuse:w:0xf3:m -U efuse:w:0xf8:m

to compile CDC bootloader (from lufa-lib trunk):
for BOOTSZ1 = BOOTSZ0 = 0. see datasheet table 27-8.
make MCU=atmega16u4 BOOT_START=0x1800 F_CPU=16000000

