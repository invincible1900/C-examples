#!/bin/bash

# \xb0\x01 mov    al,0x1
# \xe6\xe9 out    0xe9,al
# \xb0\x02 mov    al,0x2
# \xe6\xea out    0xea,al
# \xf4     hlt
echo -e -n "\xb0\x01\xe6\xe9\xb0\x02\xe6\xea\xf4" > a.bin 
make
./a.out a.bin
rm a.out a.bin

