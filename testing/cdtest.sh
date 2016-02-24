#!/bin/bash

#cd testing &> /dev/null
./huff -c $1 zipped.huff
./huff -d zipped.huff file.out
diff -q $1 file.out

#xxd zipped.lzw > zipped.lhex
#xxd $1 > file.hex
#xxd out > out.hex

