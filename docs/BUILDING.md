Building
========

The repo comes with a number of goodies to make developer life more
fun:

- docker definition for required toolchain
- docker/dev.sh to quickly and seamlessly issue commands in the toolchain container
- gdb-regview and .gdbinit to load pheriperal names into gdb
- use of blackmagic esp8266 to flash firmware via wifi

How to build the firmware
-------------------------

    git clone --recursive 5x5x2.git
    cd 5x5x2/docker
    docker build -t stm32-gcc .
    cd ..
    ./docker/dev.sh "make -C libopencm3 && make"


How to debug the firmware
-------------------------

I use an [esp8266](https://github.com/walmis/blackmagic-espidf)
programmed as a
[blackmagic](https://github.com/blacksphere/blackmagic) probe to
connect gdb via wifi to the target keyboard. The board has a 4-pin
SWIO/SWCLK .1'' header on the right lower side where the probe can be
attached.



```console
[dijkstra@mcu 5x5x2]$ ./docker/dev.sh 
root@df176a49d098:/mnt# make debug 
arm-none-eabi-gdb-py --command=.gdb_config
GNU gdb (GNU Arm Embedded Toolchain 10-2020-q4-major) 10.1.90.20201028-git
Copyright (C) 2020 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-linux-gnu --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
spi_disable_tx_dma (spi=1073819648) at ../common/spi_common_all.c:674
674		SPI_CR2(spi) &= ~SPI_CR2_TXDMAEN;
Black Magic Probeesp8266v1.7.1-252-g863a41d, Hardware Version 0
Copyright (C) 2015  Black Sphere Technologies Ltd.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>

Target voltage: 2965mV
Available Targets:
No. Att Driver
 1      STM32F1 medium density M3
A program is being debugged already.  Kill it? (y or n) [answered Y; input not from terminal]
[Switching to Thread 1]
0x0800218e in col_read () at matrix.c:85
85	{
Breakpoint 1 at 0x80001d2: file 5x5x2.c, line 81.
Note: automatically using hardware breakpoints for read-only addresses.
Loaded gdb-regview/gdb-regview.py
Type "regview <tab>" to see available commands.
Loaded register definitions: gdb-regview/defs/STM32F10X_CL.xml
```
```gdb
(gdb) load 5x5x2.elf 
Loading section .text, size 0x84a4 lma 0x8000000
Loading section .data, size 0x4a4 lma 0x80084a4
Loading section .userflash, size 0x1000 lma 0x800f000
Start address 0x08006f38, load size 39240
Transfer rate: 4 KB/sec, 934 bytes/write.
(gdb) r
The program being debugged has been started already.
Start it from the beginning? (y or n) y
Starting program: /mnt/5x5x2.elf 
[Switching to Thread 1]

Breakpoint 1, main () at 5x5x2.c:81
81	    mcu_init();
(gdb) 
```

Example use of [regview](https://github.com/fnoble/gdb-regview):
```gdb
(gdb) regview show SPI1_CR1 
SPI1_CR1 (*0x40013000) = 0x00000000

CPHA	0x0		Clock Phase
CPOL	0x0		Clock Polarity
MSTR	0x0		Master Selection
BR	0x0		BR[2:0] bits (Baud Rate Control)
SPE	0x0		SPI Enable
LSBFIRST	0x0		Frame Format
SSI	0x0		Internal slave select
SSM	0x0		Software slave management
RXONLY	0x0		Receive only
DFF	0x0		Data Frame Format
CRCNEXT	0x0		Transmit CRC next
CRCEN	0x0		Hardware CRC calculation enable
BIDIOE	0x0		Output enable in bidirectional mode
BIDIMODE	0x0		Bidirectional data mode enable
(gdb) 
```
