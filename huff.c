#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
	// string.h only used to deal with command line input and output
#include <ctype.h>

#include "bst.h"




#define BYTESIZE 256
#define BST_PRINT_OFFSET 10

//#define DEBUG
#ifdef DEBUG
#define test fprintf(stdout, "[TEST %d]\n", __LINE__);
#define debug(M) fprintf(	stdout, "[DEBUG %d] %s: %x %d\n", \
							__LINE__, #M, \
							(unsigned int)M, (unsigned int)M); \
							fflush(stdout);
#define debugs(M) fprintf(stdout, "[DEBUG %d] %s: %s\n", __LINE__, #M, M); \
							fflush(stdout);
#define debugc(M) fprintf(stdout, "[DEBUG %d] %s: %c %x\n", \
							__LINE__, #M, M, M); \
							fflush(stdout);
#else
#define test
#define debug(M)
#define debugs(M)
#define debugc(M)
#endif



typedef enum {COMPRESS, DECOMPRESS} Option;
typedef struct BstNodeDataTag {
	int 	freq;
	char 	refbyte;
	} BstNodeData;



void getdata(void* data_ptr, char* buffer)
{
BstNodeData data;
data = *((BstNodeData*) data_ptr);
if ((data.refbyte != 0) && (isprint(data.refbyte)))
	sprintf(buffer, "%c %d", data.refbyte, data.freq);
else
	sprintf(buffer, "0x%x %d", data.refbyte, data.freq);
return;
}



void compression(FILE* source_fileptr, FILE* dest_fileptr);
void decompression(FILE* source_fileptr, FILE* dest_fileptr);
int getfilesize(FILE* fileptr);






/* ----------------------------------------------------------------------
 * function: 	main()
 * description: main function
 * input: 		argc - number of cmd line arguments
 * 				argv - array of cmd line argument strings
 * output: 		integer corresponding to error level
 * notes:
 */

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

debugs(option_str)
debugs(filename_src)
debugs(filename_dest)

if (strcmp(option_str,"-c") == 0)
	option = COMPRESS;
else if (strcmp(option_str,"-d") == 0)
	option = DECOMPRESS;
else
	exit(1);


/* opening files */

FILE* source_fileptr;
FILE* dest_fileptr;


source_fileptr = fopen(filename_src, "rb");
if (source_fileptr == NULL) {
	fprintf(stderr, "Error: File could not be opened\n");
	exit(-1);
	}
dest_fileptr = fopen(filename_dest, "w+");
if (dest_fileptr == NULL) {
	fprintf(stderr, "Error: File could not be opened\n");
	exit(-1);
	}



if (option == COMPRESS) {
	compression(source_fileptr, dest_fileptr);
	}
else if (option == DECOMPRESS) {
	decompression(source_fileptr, dest_fileptr);
	}


fclose(dest_fileptr);
fclose(source_fileptr);

return 1;
}







/* ----------------------------------------------------------------------
 * function: 	compression()
 * description: encrypts via huffman encoding
 * input: 		source_fileptr - input file pointer
 * 				dest_fileptr - output file pointer
 * notes:
 */

void compression(FILE* source_fileptr, FILE* dest_fileptr)
{

int* 			freq_array;
unsigned char 	byte;
int 			x;

//printf("%x\n", *((unsigned int*) source_fileptr));
//debug(source_fileptr)
//debug(dest_fileptr)


int source_filesize = getfilesize(source_fileptr);
debug(source_filesize)
freq_array = (int*) calloc(BYTESIZE, sizeof(int));
//exit(0);


for (x = 0; x < source_filesize; x++) {
	byte = fgetc(source_fileptr);
	debugc(byte)
	freq_array[(int) byte]++;
	}



// initialize leafnodes
BstNode* 		leafnodes;
int 			leafnodes_count;
BstNodeData* 	data;


leafnodes = (BstNode*) calloc(BYTESIZE, sizeof(BstNode));
leafnodes_count = 0;
for (x = 0; x < BYTESIZE-1; x++) {

	// only include the bytes with a good occurence
	if (freq_array[x] > 0) {
		// possible coding of leafnodes
		//BstNode* leafnodes[BYTESIZE];
		//leafnodes[x] = (BstNode*) malloc(sizeof(BstNode);
		data = (BstNodeData*) malloc(sizeof(BstNodeData));
		data->refbyte 	= x;
		data->freq 		= freq_array[x];
		leafnodes[leafnodes_count].data 	= data;
		leafnodes[leafnodes_count].left 	= NULL;
		leafnodes[leafnodes_count].right 	= NULL;
		leafnodes[leafnodes_count].parent 	= NULL;
		leafnodes_count++;
		}
	}
BstNode** 	nodes;
int 		nodes_size;

// mathematically, the number of internal nodes will always be less than the
// number of leaf nodes, therefore the total size can be max double the number
// of leaf nodes
nodes_size = leafnodes_count;
nodes = (BstNode**) malloc(nodes_size * sizeof(BstNode*));

// putting pointers to the entries in the leafnodes array into the nodes
// array of pointers

//char refbyte;
//int freq;
debug(leafnodes_count)
for (x = 0; x < leafnodes_count; x++) {
	nodes[x] = &leafnodes[x];
	//freq = ((BstNodeData*)nodes[x]->data)->freq;
	//refbyte = ((BstNodeData*)nodes[x]->data)->refbyte;
	//debug(x)
	//debug(freq)
	//debugc(refbyte)
	}


int 		index[2];
int 		smallestvalue[2];
int 		currentvalue;
int 		nodes_count;
BstNode* 	node_combine;

nodes_count = nodes_size;
index[0] = 0;

while (nodes_count > 1) {
	// initialize
	data = (BstNodeData*) nodes[index[0]]->data;
	smallestvalue[0] = 0;
	smallestvalue[1] = 0;
	for (x = 0; x < nodes_size; x++) {
		if (nodes[x] != NULL) {
			data = (BstNodeData*) nodes[x]->data;
			currentvalue = data->freq;
			debug(currentvalue)
			if (	(smallestvalue[0] > currentvalue) ||
					(smallestvalue[0] == 0)) {
				smallestvalue[1] = smallestvalue[0];
				index[1] = index[0];
				smallestvalue[0] = currentvalue;
				index[0] = x;
				}
			else if (	(smallestvalue[1] > currentvalue) ||
						(smallestvalue[1] == 0)) {
				smallestvalue[1] = currentvalue;
				index[1] = x;
				}
			}
		}
	debug(smallestvalue[0])
	debug(smallestvalue[1])
	test
	data = (BstNodeData*) malloc(sizeof(BstNodeData));
	data->refbyte 	= 0;
	data->freq 		= smallestvalue[0] + smallestvalue[1];
	debug(data->freq)

	node_combine = (BstNode*) malloc(sizeof(BstNode));
	node_combine->parent 	= NULL;
	node_combine->left 		= nodes[index[0]];
	node_combine->right 	= nodes[index[1]];
	node_combine->data 		= data;

	nodes[index[0]] = node_combine;
	nodes[index[1]] = NULL;
	debug(nodes_count)
	nodes_count--;
	}


BstTree tree;
tree.root = node_combine;

bst_debug_print_tree(&tree, &getdata);




////Initialize codeArray
//code codeArray[256];
//for (x = 0; x < BYTESIZE; x++) {
//	codeArray[x].encoding = NULL;
//}
//
//// loop through leafnodes[0-255], traversing with the back pointers.
//// store encoding in codeArray[0-255] (traversing from the leafs to
//// root leads to backwards code, it is reversed)
//
//int charCount,y,y2;
//node *curr,*prev;
//for (x = 0; x < BYTESIZE; x++) {
//	if (leafnodes[x].freq == 0) continue;
//	curr = &leafnodes[x];
//	charCount = 0;
//	while (curr->back != NULL) {
//		charCount++;
//		prev = curr;
//		curr = prev->back;
//		codeArray[x].encoding = realloc(codeArray[x].encoding, charCount);
//		if (prev == curr->left)
//			codeArray[x].encoding[charCount - 1] = 0;
//		else
//			codeArray[x].encoding[charCount - 1] = 1;
//		codeArray[x].length = charCount;
//		}
//	// reverse the codes
//	unsigned char codeHold[codeArray[x].length];
//	for (y=0; y < codeArray[x].length; y++) {
//		codeHold[y] = codeArray[x].encoding[y];
//		}
//	y2 = 0;
//	for (y=codeArray[x].length - 1; y >=0; y--) {
//		codeArray[x].encoding[y2] = codeHold[y];
//		y2++;
//		}
//	}
//// encodings and their respective lengths are stored in
//// codeArray[0-255]
//
//// begin output to file
//
//// first the 256 int array holding the char-frequencies is written to
//// file can use the same exact methods as above in decompression to
//// create the code arrays
//
//fwrite(freq_array,sizeof(int),BYTESIZE,dest_fileptr);
//
//// now rewind the read from file to read through BYTESIZE by BYTESIZE again.
//// Write the proper encoding of each BYTESIZE
//
//rewind(source_fileptr);
//int breakOut = 0;
//int count = 0;
//unsigned char byteOut = 0x00;
//unsigned char *compressed;
//int totalBytes = 0;
//// extra bits is needed to indicate the amount of padding with 0's on
//// the very last BYTESIZE
//
//short extraBits = 0;
//byte = fgetc(source_fileptr);
//while (1) {
//	for (x =0; x < 8; x++) {
//		if ( count >= codeArray[byte].length) {
//			byte = fgetc(source_fileptr);
//			if ( byte == EOF) {
//				breakOut = 1;
//				extraBits = 8 - (short)x;
//				if (extraBits == 8) extraBits = 0;
//				byteOut = byteOut << (extraBits - 1);
//				break;
//				}
//			count = 0;
//			}
//		byteOut = byteOut | codeArray[byte].encoding[count];
//		if (x != 7) byteOut = byteOut << 1;
//		count++;
//		}
//	totalBytes++;
//	compressed = realloc(compressed,totalBytes);
//	compressed[totalBytes-1] = byteOut;
//
//	if (breakOut == 1) break;
//	byteOut = 0x00;
//	}
//// write one short after the freq array to the file. this short
//// indicates the amount of padded 0's on the last BYTESIZE. Range (0-7)
//
//fwrite(&extraBits,sizeof(unsigned short),1,dest_fileptr);
//fwrite(compressed,sizeof(unsigned char),totalBytes,dest_fileptr);
//// close files and return
//



}








/* ----------------------------------------------------------------------
 * function: 	decompression()
 * description: encrypts via huffman encoding
 * input: 		source_fileptr - input file pointer
 * 				dest_fileptr - output file pointer
 * notes:
 */

void decompression(FILE* source_fileptr, FILE* dest_fileptr)
{



}








/* ----------------------------------------------------------------------
 * function: 	bst_debug_print_tree
 * description: prints out a visual representation of the given tree. the
 * 				first line has the root node, the second line its children,
 * 				etc. a border around the tree is also printed
 * input: 		tree - pointer to a tree
 * notes: 		depends on the recursive bst_debug_pt_rec function
 */
void bst_debug_print_tree(BstTree* tree, GetDataFunc getdata)
{
int x, borderwidth, height;

height = bst_height(tree);
	// setting height
borderwidth = BST_PRINT_OFFSET * ((1 << (height + 1)) - 1) + 6;
	// calculating the border width

printf("Binary Tree\n");
// printing top border
for (x = 0; x < borderwidth; x++) {
	printf("-");
	}
printf("\n");

// printing each level
for (x = 0; x <= height; x++) {
	printf("|  ");
	bst_debug_pt_rec(tree->root, 0, x, height, getdata);
	printf("  |\n");
	}

// printing bottom border
for (x = 0; x < borderwidth; x++) {
	printf("-");
	}
printf("\n");

}









/* ----------------------------------------------------------------------
 * function: 	bst_debug_pt_rec
 * description: recursive function that prints all the keys of one level of a
 * 				tree
 * input:		node - pointer to a node in the tree
 * 				level - current level
 * 				level_print - level to be printed
 * 				height - maximum path length
 * notes: 		all nodes up to the last level (some may have to be filled in
 * 				to archieve a perfect tree) are printed. for those that arent
 * 				on level to be printed a blank line is outputed instead.
 */
void bst_debug_pt_rec(	BstNode* node,
						int level,
						int level_print,
						int height,
						GetDataFunc getdata)
{
// checking if the nodes current level is valid
char buffer[80];
if (level > height) {
	return;
	}

if (node == NULL) {
	// if node is NULL, its children will also be NULL nodes
	bst_debug_pt_rec(NULL, level+1, level_print, height, getdata);
	printf("%*c", BST_PRINT_OFFSET, ' ');
	bst_debug_pt_rec(NULL, level+1, level_print, height, getdata);
	}
else {
	// if node is not NULL, the output is a key only if its is on the level
	// to be printed
	bst_debug_pt_rec(node->left, level+1, level_print, height, getdata);
	if (level == level_print) {
		(*getdata)(node->data, buffer);
		printf("[%*s]", BST_PRINT_OFFSET - 2, buffer);
		}
	else {
		printf("%*c", BST_PRINT_OFFSET, ' ');
		}
	bst_debug_pt_rec(node->right, level+1, level_print, height, getdata);
	}
return;
}



/* ----------------------------------------------------------------------
 * function: 	bst_height
 * description: finds the height of the tree, as in the level of the lowest
 * 				node notes: depends on the recursive bst_height_rec function
 * input: 		tree - pointer to a tree
 */
int bst_height(BstTree* tree)
{
return bst_height_rec(tree->root, 0);
}





/* ----------------------------------------------------------------------
 * function: 	bst_height_rec
 * description: recursive function that returns the maximum path length
 * 				of all nodes following the current one
 * input: 		node - pointer to a node in a tree
 * 				level - level the node is on in the tree
 * output: 		the maximum path length
 */
int bst_height_rec(BstNode* node, int level)
{
int a, b;

if (node == NULL) {
	// returning the path length of the parent node, therefore the level is
	// decremented
	return level-1;
	}

// finding the max path lengths of each child
a = bst_height_rec(node->left, level+1);
b = bst_height_rec(node->right, level+1);

// comparison of the path lengths and returning the larger one
if (b > a) {
	return b;
	}
else {
	return a;
	}
}





/* ----------------------------------------------------------------------
 * function: 	getfilesize()
 * description: returns the number of bytes in a file
 * arguments: 	fileptr - input file
 * notes:
 */
int getfilesize(FILE* fileptr)
{
int filesize;
fseek(fileptr, 0L, SEEK_END);
filesize = ftell(fileptr);
rewind(fileptr);
return filesize;
}




