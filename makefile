# ---- FILE HEADER ----------------------------------------------------------
# project: lab3
# file: makefile
# student/author: elias abderhalden
# date: 2016-02-11
# ---------------------------------------------------------------------------
# class: ece4380 spring 2016
# instructor: Adam Hoover
# assignment: lab3
# purpose: lzw compression
# ---------------------------------------------------------------------------
# notes:	commands
# 			make - build programs
# 			make clean - delete object files, executable, and core
# ---------------------------------------------------------------------------


c = gcc

cnf = -g -Wall
cn = $(c) $(cnf)
co = -O2 -DNDEBUG

sl = $(c) -fPIC -shared

a = ar
af = -cvq
an = $(a) $(af)


lab3 : huff.o
	$(c) huff.o -o huff -lm

%.o : %.c
	$(cn) -c -o $@ $<



t : $(wildcard t*)

clearscreen :
	clear

t0 :
t1 :
t2 :
t3 :
t4 :
t5 :
t6 :
t7 :
t8 :
t9 :


cl : clo clt clsl cla
	mkdir trash
	rm -f core
clo :
	mv *.o trash
clt :
	mv t? trash
clsl :
	mv *.so trash
cla :
	mv *.a trash




