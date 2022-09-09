#!/bin/bash
gdb -q plt -ex "set disassembly-flavor intel" \
    -ex "disass main" \
    -ex "b main" \
    -ex "b puts@plt" \
    -ex "r" \
    -ex "display/5i \$pc" 

    
