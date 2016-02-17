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



typedef enum {COMPRESS, DECOMPRESS} Option;



int main(int argc, char *argv[]) {

	Option option;
	char* option_str;
	char* filename_src;
	char* filename_dest;

	if (argc != 4) {
		fprintf(stderr, "Error: Argument count\n");
		exit(-1);
		}

	option_str = 	argv[1];
	filename_src = 	argv[2];
	filename_dest = argv[3];

	if (strcmp(option_str,"-c") == 0)
		option = COMPRESS;
	else if (strcmp(option_str,"-d") == 0)
		option = DECOMPRESS;
	else
		exit(1);


	/* opening files */

	FILE* source_fileptr;
	FILE* dest_fileptr;
	//int filesize_src;
	//int filesize_dest;


	source_fileptr = fopen(filename_src, "rb");
	if (source_fileptr == NULL) {
		fprintf(stderr, "Error: File could not be opened\n");
		exit(-1);
		}
	dest_fileptr = fopen(filename_dest, "wb");
	if (dest_fileptr == NULL) {
		fprintf(stderr, "Error: File could not be opened\n");
		exit(-1);
		}





	if (option == COMPRESS) {
		// create array to hold frequency of bytes 0-255. Initialize all
		// counts to 0 (using calloc

		int *freq = calloc(byte,sizeof(int));
		int c;
		// loop through file byte by byte and increment byte freq count
		// accordingly

		while ( (c = fgetc(source_fileptr)) != EOF) {
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

		// loop through nodeArray[0-255], traversing with the back pointers.
		// store encoding in codeArray[0-255] (traversing from the leafs to
		// root leads to backwards code, it is reversed)

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
		// encodings and their respective lengths are stored in
		// codeArray[0-255]

		// begin output to file

		// first the 256 int array holding the char-frequencies is written to
		// file can use the same exact methods as above in decompression to
		// create the code arrays

		fwrite(freq,sizeof(int),byte,dest_fileptr);

		// now rewind the read from file to read through byte by byte again.
		// Write the proper encoding of each byte

		rewind(source_fileptr);
		int breakOut = 0;
		int count = 0;
		unsigned char byteOut = 0x00;
		unsigned char *compressed;
		int totalBytes = 0;
		// extra bits is needed to indicate the amount of padding with 0's on
		// the very last byte

		short extraBits = 0;
		c = fgetc(source_fileptr);
		while (1) {
			for (x =0; x < 8; x++) {
				if ( count >= codeArray[c].length) {
					c = fgetc(source_fileptr);
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
		// write one short after the freq array to the file. this short
		// indicates the amount of padded 0's on the last byte. Range (0-7)

		fwrite(&extraBits,sizeof(unsigned short),1,dest_fileptr);
		fwrite(compressed,sizeof(unsigned char),totalBytes,dest_fileptr);
		// close files and return
		fclose(dest_fileptr);
		fclose(source_fileptr);
		}
	else if (option == DECOMPRESS) {
		}

	return 1;
}










/*---------------------------------------------------------------------------
function: bst_debug_print_tree
description: prints out a visual representation of the given tree. the first
	line has the root node, the second line its children, etc. a border
	around the tree is also printed
notes: depends on the recursive bst_debug_pt_rec function
input: 	tree - pointer to a tree
output: none

void bst_debug_print_tree(bst_t* tree)
{
int x, borderwidth, height;

height = bst_height(tree);
	// setting height
borderwidth = 3 * ((1 << (height + 1)) + 1);
	// calculating the border width

printf("Binary Tree\n");
// printing top border
for (x=0; x<borderwidth; x++)
	{
	printf("-");
	}
printf("\n");

// printing each level
for (x=0; x<=height; x++)
	{
	printf("|  ");
	bst_debug_pt_rec(tree->root, 0, x, height);
	printf("  |\n");
	}

// printing bottom border
for (x=0; x<borderwidth; x++)
	{
	printf("-");
	}
printf("\n");

}
*/




/*---------------------------------------------------------------------------
function: bst_debug_pt_rec
description: recursive function that prints all the keys of one level of a
	tree
notes: all nodes up to the last level (some may have to be filled in to
	archieve a perfect tree) are printed. for those that arent on level to be
	printed a blank line is outputed instead.
input:	node - pointer to a node in the tree
		level - current level
		level_print - level to be printed
		height - maximum path length
output: none

void bst_debug_pt_rec(	bst_node_t* node,
						int level,
						int level_print,
						int height)
{

// checking if the nodes current level is valid
if (level > height)
	{
	return;
	}

if (node == NULL)
	{
	// if node is NULL, its children will also be NULL nodes
	bst_debug_pt_rec(NULL, level+1, level_print, height);
	printf("   ");
	bst_debug_pt_rec(NULL, level+1, level_print, height);
	}
else
	{
	// if node is not NULL, the output is a key only if its is on the level
	// to be printed
	bst_debug_pt_rec(node->left, level+1, level_print, height);
	if (level == level_print)
		{
		printf("%3d",(int) node->key);
		}
	else
		{
		printf("   ");
		}
	bst_debug_pt_rec(node->right, level+1, level_print, height);
	}
return;
}

*/
