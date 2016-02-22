#!/bin/bash

#cd testing &> /dev/null
echo compression testing > log
valgrind --leak-check=full ./huff -c $1 zipped.lzw 2>> log
echo decompression testing >> log
valgrind --leak-check=full ./huff -d zipped.lzw out 2>> log
diff -q $1 out >> log

vim log

