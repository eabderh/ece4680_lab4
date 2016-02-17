for Elias

compile with -lm  b/c I included math.h. I honestly forget how to make Makefiles

I used
gcc -g huff.c -o huff -Wall -lm


usage:
	Compression:
		./huff -c [filename to compress]
	
		compressed file will be outputted to previous filename + .huff 

	Decompression:
		...


The compressed file output format:
	first 1024 bytes: The 256 int array that holds the frequency of each char(0-255)
	next 2 bytes: A short that holds the value of the number of padded 0's in the last bytes (0-7 padded zeros)
	remaining bytes: a char array that is packed with encoded bits. The very last char will be padded with 0-bits, up to seven extra 0s. This is because you can only fwrite a byte at a time.


Using the first 1024 bytes of the compressed file you can get the frequency array (int freq[256] ). Then reuse lines 39 to 120 of my code to create the binary tree then create the code array. The code array should be all you need to walk through the bits of the encoded file and write the decompressed file. 

I've tested the compression byte by byte for test.txt and test2.txt to verify it's correct. The compression worked fine for golfcore.ppm and the story.txt file I sent, but were too large to walk through.

I hope you can read my code fine. Let me know if you have any questions
