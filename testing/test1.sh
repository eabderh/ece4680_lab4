#!/bin/bash

cd testing &> /dev/null
./lab2 -e $1 zipped.rle
echo
./lab2 -d zipped.rle out
diff -q $1 out


