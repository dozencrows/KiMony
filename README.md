KiMony
======
This is a smart, programmable remote firmware built on the Freedom KL26Z evaluation board, sporting
a Freescale Kinetis KL26Z microcontroller.

It's a 'bare metal' Kinetis Design Studio 2 project; i.e. no Processor Expert or RTOS code, just some system headers.

The hardware required consists of custom PCBs used to route the KL26Z IO pins to various components and to
tactile switches for input.

It uses my I2C-IR module via I2C, an 8080 8-bit parallel LCD module with ILI9341 controller, a key matrix via
an MCP23008 (via I2C), a touch screen via SPI on the LCD module, various GPIO pins connecting to other switches
and two pins that double up as UART send/receive (for debugging and downloading data to the remote).

There are some Python scripts that are used to build the binary data for the remote, and to download it over
serial via the UART pins.

The linker script is customised to embed a copy of the binary data so that the remote doesn't start 'empty' -
this is mostly for development purposes.

Important Note
--------------
I am providing code inthis repository to you under an open source license.
Because this is my personal repository, the license you receive to my code is from me and not from my employer (Facebook).
