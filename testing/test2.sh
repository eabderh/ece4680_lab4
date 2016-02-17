#!/bin/bash

cd testing &> /dev/null
./lab3 -c $1 zipped.rle
read -n1 -r key
xxd zipped.rle > zipped.hex
cat zipped.hex
#./lab3 -d zipped.rle out
#diff -q $1 out


