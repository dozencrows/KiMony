#!/bin/bash
cat $1 | hexdump -v -e '"LONG (0x0" 1/4 "%.8X) " "LONG (0x0" 1/4 "%.8X) " "LONG (0x0" 1/4 "%.8X) " "LONG (0x0" 1/4 "%.8X) " "\n"' > $2

