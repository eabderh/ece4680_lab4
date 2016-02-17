#!/bin/bash

cd testing &> /dev/null
./huff -c $1 zipped.huff

xxd zipped.huff > hex/zipped.hex


