#!/bin/bash

#cd testing &> /dev/null
echo compression testing > log
valgrind --leak-check=full ./huff -c $1 zipped.huff 2>> log
echo decompression testing >> log
valgrind --leak-check=full ./huff -d zipped.huff file.out 2>> log
diff -q $1 file.out >> log

vim log

