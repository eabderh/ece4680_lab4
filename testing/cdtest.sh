#!/bin/bash

cd testing &> /dev/null
./huff -c $1 zipped.lzw
./huff -d zipped.lzw out
diff -q $1 out

xxd zipped.lzw > zipped.lhex
xxd $1 > file.hex
xxd out > out.hex

