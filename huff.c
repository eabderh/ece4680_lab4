#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
// string.h only used to deal with command line input and output
#include <string.h>

#define byte 256

typedef struct node {
	int freq;
	struct node *left;
	struct node *right;
	struct node *back;
	} node;

typedef struct code {
	int length;
	unsigned char *encoding;
	} code;




int main(int argc, char *argv[]) {
	if ( strcmp(argv[1],"-c") == 0) {
		/* create array to hold frequency of bytes 0-255. Initialize all
		 * counts to 0 (using calloc
		 */
		int *freq = calloc(byte,sizeof(int));
		FILE *fpt = fopen(argv[2],"rb");
		if (fpt == NULL) {
			printf("File read from returned NULL");
			exit(0);
			}
		int c;
		/* loop through file byte by byte and increment byte freq count
		 * accordingly
		 */
		while ( (c = fgetc(fpt)) != EOF) {
			freq[ c ]++;
			}
		// initialize nodeArray
		int x;
		node *nodeArray = calloc(byte,sizeof(node));
		for ( x=0; x < byte; x++) {
			nodeArray[x].freq = freq[x];
			nodeArray[x].left = NULL;
			nodeArray[x].right = NULL;
			nodeArray[x].back = NULL;
			}
		int nodeCount = byte; //256

		node *small1,*small2;
		int small1Val, small2Val;
		int nullBackCount;
		// creates the binary tree via algorithm discussed in class
		while (1) {
			small1 = NULL;
			small2 = NULL;
			small1Val = INT_MAX;
			small2Val = INT_MAX;
			nullBackCount = 0;
			for ( x=0; x < nodeCount; x++) {
				if ( nodeArray[x].back == NULL && nodeArray[x].freq != 0) {
					nullBackCount++;
					if ( nodeArray[x].freq <= small1Val ) {
						small2 = small1;
						if (small2 != NULL) small2Val = small2->freq;
						small1 = &nodeArray[x];
						small1Val = small1->freq;
						}
					else if ( nodeArray[x].freq <= small2Val) {
						small2 = &nodeArray[x];
						small2Val = small2->freq;
						}
					}
				}
			if (nullBackCount == 1) break;
			nodeArray = realloc(nodeArray,(nodeCount+1) * sizeof(node) );
			nodeArray[nodeCount].freq = small1Val +small2Val;
			nodeArray[nodeCount].left = small1;
			nodeArray[nodeCount].right = small2;
			nodeArray[nodeCount].back = NULL;
			small1->back = &nodeArray[nodeCount];
			small2->back = &nodeArray[nodeCount];
			nodeCount++;
			}
		//binary tree completed. Stored and interconnected in nodeArray

		//Initialize codeArray
		code codeArray[256];
		for (x=0; x < byte; x++) {
			codeArray[x].encoding = NULL;
		}

		/* loop through nodeArray[0-255], traversing with the back pointers.
		 * store encoding in codeArray[0-255] (traversing from the leafs to
		 * root leads to backwards code, it is reversed)
		 */
		int charCount,y,y2;
		node *curr,*prev;
		for (x=0; x < byte; x++) {
			if (nodeArray[x].freq == 0) continue;
			curr = &nodeArray[x];
			charCount = 0;
			while (curr->back != NULL) {
				charCount++;
				prev = curr;
				curr = prev->back;
				codeArray[x].encoding = realloc(codeArray[x].encoding, charCount);
				if (prev == curr->left)
					codeArray[x].encoding[charCount - 1] = 0;
				else
					codeArray[x].encoding[charCount - 1] = 1;
				codeArray[x].length = charCount;
				}
			// reverse the codes
			unsigned char codeHold[codeArray[x].length];
			for (y=0; y < codeArray[x].length; y++) {
				codeHold[y] = codeArray[x].encoding[y];
				}
			y2 = 0;
			for (y=codeArray[x].length - 1; y >=0; y--) {
				codeArray[x].encoding[y2] = codeHold[y];
				y2++;
				}
			}
		/* encodings and their respective lengths are stored in
		 * codeArray[0-255]
		 */

		//begin output to file
		FILE *fpt2 = fopen(strcat(argv[2],".huff"),"w+");
		if (fpt2 == NULL) {
			printf("File pointer returned NULL");
			exit(0);
			}
		/* first the 256 int array holding the char-frequencies is written to
		 * file can use the same exact methods as above in decompression to
		 * create the code arrays
		 */
		fwrite(freq,sizeof(int),byte,fpt2);

		/* now rewind the read from file to read through byte by byte again.
		 * Write the proper encoding of each byte
		 */
		rewind(fpt);
		int breakOut = 0;
		int count = 0;
		unsigned char byteOut = 0x00;
		unsigned char *compressed;
		int totalBytes = 0;
		/* extra bits is needed to indicate the amount of padding with 0's on
		 * the very last byte
		 */
		short extraBits = 0;
		c = fgetc(fpt);
		while (1) {
			for (x =0; x < 8; x++) {
				if ( count >= codeArray[c].length) {
					c = fgetc(fpt);
					if ( c == EOF) {
						breakOut = 1;
						extraBits = 8 - (short)x;
						if (extraBits == 8) extraBits = 0;
						byteOut = byteOut << (extraBits - 1);
						break;
						}
					count = 0;
					}
				byteOut = byteOut | codeArray[c].encoding[count];
				if (x != 7) byteOut = byteOut << 1;
				count++;
				}
			totalBytes++;
			compressed = realloc(compressed,totalBytes);
			compressed[totalBytes-1] = byteOut;

			if (breakOut == 1) break;
			byteOut = 0x00;
			}
		/* write one short after the freq array to the file. this short
		 * indicates the amount of padded 0's on the last byte. Range (0-7)
		 */
		fwrite(&extraBits,sizeof(unsigned short),1,fpt2);
		fwrite(compressed,sizeof(unsigned char),totalBytes,fpt2);
		// close files and return
		fclose(fpt2);
		fclose(fpt);
		}
	if ( strcmp(argv[1],"-u") == 0) {
		}
	return 1;
}










