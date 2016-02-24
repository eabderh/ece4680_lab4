/* ---- FILE HEADER -----------------------------------------------------
 * project: 	lab4
 * file: 		makefile.txt
 * author: 		Elias Abderhalden / Tyler Fitzgerald
 * date: 		2016-02-22
 * ----------------------------------------------------------------------
 * class: 		ece4680 spring 2016
 * instructor: 	Adam Hoover
 * assignment: 	lab4
 * purpose: 	hoffman compression
 * ----------------------------------------------------------------------
 */



compilation:

	gcc -Wall -g huff.c -o huff


usage:

	huff [option] [file_input] [file_ouptut]

	option:
		"-c" - compression
		"-d" - decompression
	file_input:
		file to be compressed/decompressed
	file_ouptut:
		name of output file

compression format:

	the first byte defines the way the frequency array is written to the file.
	the next few bytes is the frequency array.
	then comes the actual data.
	at the end of the file is a single byte that contains the number of
	padding bits that were added to the data.




