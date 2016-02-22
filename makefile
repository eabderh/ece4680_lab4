# ---- FILE HEADER ----------------------------------------------------------
# project: 		lab4
# file: 		makefile
# author: 		elias abderhalden
# date: 		2016-02-11
# ---------------------------------------------------------------------------
# class: 		ece4680 spring 2016
# instructor: 	Adam Hoover
# assignment: 	lab4
# purpose: 		hoffman compression
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


lab3 : huff.o huff.h debug.h bst.h
	$(c) huff.o -o huff

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


#cl : clo clt clsl cla
#	rm -f core
cl : clo
	rm -f core
clo :
	mv *.o trash/. 	2>/dev/null; true
clt :
	mv t? trash/. 	2>/dev/null; true
clsl :
	mv *.so trash/. 2>/dev/null; true
cla :
	mv *.a trash/. 	2>/dev/null; true




