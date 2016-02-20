#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
	// string.h only used to deal with command line input and output
#include <ctype.h>
//#include <stdint.h>

#include "bst.h"

typedef unsigned long int uint32_t;


#define BYTESIZE 256
#define BYTEWIDTH 8
#define BST_PRINT_OFFSET 10
#define TABLETYPE1_HEADERSIZE 1
#define TABLETYPE2_HEADERSIZE 2
#define ENDING_OFFSET 1


//#define DEBUG
#ifdef DEBUG
#define test fprintf(stdout, "[TEST %d]\n", __LINE__);
#define debugm(M) fprintf(	stdout, "\n\n[DEBUG %d] %s\n\n", __LINE__, M);
#define debug(M) fprintf(	stdout, "[DEBUG %d] %s: %x %d\n", \
							__LINE__, #M, \
							(unsigned int)M, (unsigned int)M); \
							fflush(stdout);
#define debugs(M) fprintf(stdout, "[DEBUG %d] %s: %s\n", __LINE__, #M, M); \
							fflush(stdout);
#define debugl(M) fprintf(stdout, "[DEBUG %d] %s: %lld\n", __LINE__, #M, M); \
							fflush(stdout);
#define debugc(M) fprintf(stdout, "[DEBUG %d] %s: %c %x\n", \
							__LINE__, #M, M, M); \
							fflush(stdout);
#define debugp(M) fprintf(stdout, "[DEBUG %d] %s: %p\n", \
							__LINE__, #M, M); \
							fflush(stdout);
#define BYTETOBINARYFPRINT "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte) \
	(byte & 0x80 ? 1 : 0), \
	(byte & 0x40 ? 1 : 0), \
	(byte & 0x20 ? 1 : 0), \
	(byte & 0x10 ? 1 : 0), \
	(byte & 0x08 ? 1 : 0), \
	(byte & 0x04 ? 1 : 0), \
	(byte & 0x02 ? 1 : 0), \
	(byte & 0x01 ? 1 : 0)
#define debugb(M) fprintf(stdout, \
	"[DEBUG %d] %s: "BYTETOBINARYFPRINT" "BYTETOBINARYFPRINT"\n", \
	__LINE__, #M, BYTETOBINARY(*(((char*)M)+1)), BYTETOBINARY(*((char*)M))); \
	fflush(stdout);
#else
#define test
#define debugm(M)
#define debug(M)
#define debugs(M)
#define debugc(M)
#define debugp(M)
#define debugb(M)
#define debugl(M)
#endif



typedef enum {COMPRESS, DECOMPRESS} Option;
typedef struct ByteEncodingTag {
	unsigned char 	decodedbyte;
	int 			encodedbyte;
	int 			length;
	} ByteEncoding;
typedef struct BstNodeDataTag {
	unsigned int 	bitvalue;
	long 			freq;
	unsigned char 	refbyte;
	} BstNodeData;



void getdata(void* data_ptr, char* buffer)
{
BstNodeData data;
data = *((BstNodeData*) data_ptr);
if ((data.refbyte != 0) && (isprint(data.refbyte)))
	sprintf(buffer, "%c %ld %d", data.refbyte, data.freq, data.bitvalue);
else
	sprintf(buffer, "0x%x %ld %d", data.refbyte, data.freq, data.bitvalue);
return;
}



void compression(FILE* source_fileptr, FILE* dest_fileptr);
void decompression(FILE* source_fileptr, FILE* dest_fileptr);
uint32_t quick_log2(uint32_t i);
uint32_t getfilesize(FILE* fileptr);






/* ----------------------------------------------------------------------
 * function: 	main()
 * description: main function
 * input: 		argc - number of cmd line arguments
 * 				argv - array of cmd line argument strings
 * output: 		integer corresponding to error level
 * notes:
 */

int main(int argc, char *argv[])
{

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

int source_filesize;
int dest_filesize;


if (option == COMPRESS) {
	compression(source_fileptr, dest_fileptr);

	source_filesize = getfilesize(source_fileptr);
	dest_filesize = getfilesize(dest_fileptr);

	fprintf(stdout, "original size: %d (0x%x)\n", 	source_filesize,
													source_filesize);
	fprintf(stdout, "new size: %d (0x%x)\n", 	dest_filesize,
												dest_filesize);
	fprintf(stdout, "ratio: %f\n", dest_filesize/((float) source_filesize));
	}
else if (option == DECOMPRESS) {
	decompression(source_fileptr, dest_fileptr);
	}

test
fclose(dest_fileptr);
fclose(source_fileptr);
test

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

uint32_t 		x;
unsigned char 	byte;
uint32_t* 		freq_array;
uint32_t 		source_filesize;
uint32_t 		freq_entrymaxsize;
int 			leafnodes_count;

source_filesize = getfilesize(source_fileptr);
freq_array = (uint32_t*) malloc(BYTESIZE * sizeof(uint32_t));
for (x = 0; x < BYTESIZE; x++) {
	freq_array[x] = 0;
	}

debug(source_filesize)

freq_entrymaxsize = UCHAR_MAX;
leafnodes_count = 0;

for (x = 0; x < source_filesize; x++) {
	byte = fgetc(source_fileptr);
	//debugc(byte)

	if (freq_array[byte] == 0) {
		leafnodes_count++;
		}

	freq_array[byte]++;

	if (freq_array[byte] > freq_entrymaxsize) {
		debugl(freq_array[byte])
		debug(byte)
		if (freq_array[byte] < USHRT_MAX) {
			freq_entrymaxsize = USHRT_MAX;
			test
			}
		else if (freq_array[byte] < UINT_MAX) {
			freq_entrymaxsize = UINT_MAX;
			test
			}
		else if (freq_array[byte] < ULONG_MAX) {
			freq_entrymaxsize = ULONG_MAX;
			test
			}
//		else if (freq_array[byte] < ULLONG_MAX)
//			freq_entrymaxsize = ULLONG_MAX;
		else {
			fprintf(stderr, "Error: Frequency too big\n");
			}
		}
	}

debugl(freq_entrymaxsize)
debug(leafnodes_count)

// initialize leafnodes
BstNode** 		leafnodes;
BstNode* 		node_ptr;
BstNodeData* 	data_ptr;
int 			leafnodes_index;


leafnodes = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
leafnodes_index = 0;
for (x = 0; x < BYTESIZE; x++) {
	// only include the bytes with a good occurence
	if (freq_array[x] > 0) {
		data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
		data_ptr->refbyte 		= x;
		data_ptr->freq 			= freq_array[x];

		node_ptr = (BstNode*) malloc(sizeof(BstNode));
		node_ptr->left 		= NULL;
		node_ptr->right 	= NULL;
		node_ptr->parent 	= NULL;
		node_ptr->data 		= data_ptr;
		leafnodes[leafnodes_index] = node_ptr;

		leafnodes_index++;
		}
	}

//debug(freq_entrymaxsize);




int 		tabletype1_size;
int 		tabletype2_size;
char* 		freq_ptr;
uint32_t 	freq_entrybytesize;

freq_entrybytesize = quick_log2(freq_entrymaxsize + 1) / BYTEWIDTH;


tabletype1_size = 	BYTESIZE * freq_entrybytesize +
					TABLETYPE1_HEADERSIZE;
tabletype2_size = 	leafnodes_count * freq_entrybytesize +
					TABLETYPE2_HEADERSIZE;

debugl(freq_entrymaxsize)
debugl(freq_entrybytesize)
debug(leafnodes_count)
debug(tabletype1_size)
debug(tabletype2_size)
//debug
#ifdef DEBUG
for (x = 0; x < leafnodes_count; x++) {
	data_ptr = (BstNodeData*) leafnodes[x]->data;
	debugc(data_ptr->refbyte)
	debug(data_ptr->freq)
	}
#endif

if (tabletype1_size <= tabletype2_size) {
	debugm("compression: tabletype1")
	fputc(freq_entrybytesize, dest_fileptr);
	for (x = 0; x < BYTESIZE; x++) {
		freq_ptr = ((char*) &(freq_array[x]));
		fwrite(freq_ptr, 1, freq_entrybytesize, dest_fileptr);
		}
	}
else {
	debugm("compression: tabletype2")
	debug(-freq_entrybytesize)
	fputc(-freq_entrybytesize, dest_fileptr);
	debug(leafnodes_count)
	fputc(leafnodes_count, dest_fileptr);
	for (x = 0; x < leafnodes_count; x++) {
		data_ptr = (BstNodeData*) leafnodes[x]->data;
		byte = data_ptr->refbyte;
		fputc(byte, dest_fileptr);

		freq_ptr = ((char*) &(data_ptr->freq));
			// this works on little endian systems
		fwrite(freq_ptr, 1, freq_entrybytesize, dest_fileptr);
		}
	}





int 		smallindex_one;
int 		smallindex_two;
long 		smallval_one;
long 		smallval_two;
long 		currentvalue;
int 		nodes_count;
BstNode** 	nodes;
BstNode* 	node_left;
BstNode* 	node_right;
BstNode* 	node_combine;
BstNode* 	root;
BstTree 	tree;
#ifdef DEBUG
#endif


nodes = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
memcpy(nodes, leafnodes, leafnodes_count * sizeof(BstNode*));
//debug
for (x = 0; x < leafnodes_count; x++) {
	data_ptr = (BstNodeData*) nodes[x]->data;
	debugc(data_ptr->refbyte)
	debug(data_ptr->freq)
	}

nodes_count = leafnodes_count;
smallindex_one = 0;
smallindex_two = 0;

while (nodes_count > 1) {
	// initialize
	data_ptr = (BstNodeData*) nodes[smallindex_one]->data;
	smallval_one = 0;
	smallval_two = 0;
	for (x = 0; x < leafnodes_count; x++) {
		if (nodes[x] != NULL) {
			data_ptr = (BstNodeData*) nodes[x]->data;
			currentvalue = data_ptr->freq;
			debug(currentvalue)
			if (	(smallval_one > currentvalue) ||
					(smallval_one == 0)) {
				smallval_two = smallval_one;
				smallindex_two = smallindex_one;
				smallval_one = currentvalue;
				smallindex_one = x;
				}
			else if (	(smallval_two > currentvalue) ||
						(smallval_two == 0)) {
				smallval_two = currentvalue;
				smallindex_two = x;
				}
			}
		}
	debug(smallval_one)
	debug(smallval_two)

	node_left 	= nodes[smallindex_one];
	node_right 	= nodes[smallindex_two];

	data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
	data_ptr->refbyte 	= 0;
	data_ptr->freq 		= smallval_one + smallval_two;

	node_combine = (BstNode*) malloc(sizeof(BstNode));
	node_combine->parent 	= NULL;
	node_combine->left 		= node_left;
	node_combine->right 	= node_right;
	node_combine->data 		= data_ptr;

	node_left->parent 	= node_combine;
	node_right->parent 	= node_combine;

	((BstNodeData*) node_left->data)->bitvalue 	= 0;
	((BstNodeData*) node_right->data)->bitvalue = 1;

	nodes[smallindex_one] = node_combine;
	nodes[smallindex_two] = NULL;
	nodes_count--;
	}

root = node_combine;
((BstNodeData*) root->data)->bitvalue = -1;

tree.root = node_combine;
#ifdef DEBUG
bst_debugprinttree(&tree, &getdata);
#endif






/* traveling up from leaf node to root
 */


ByteEncoding* 	encoding_array;
BstNode* 		node_leaf;
BstNode* 		node;
int 		 	encodingbyte;
int 		 	encodingbyte_next;
int 			length;

encoding_array = 	(ByteEncoding*)
					malloc(leafnodes_count * sizeof(ByteEncoding));

for (x = 0; x < leafnodes_count; x++) {
	node_leaf = leafnodes[x];
	node = node_leaf;
	encodingbyte = 0;
	length = 0;

	while (node->parent != NULL) {
		debug(encodingbyte)
		encodingbyte_next = ((BstNodeData*) node->data)->bitvalue;
		debug(encodingbyte_next)
		encodingbyte = encodingbyte | (encodingbyte_next << length);
		length++;
		node = node->parent;
		}

	encoding_array[x].decodedbyte = ((BstNodeData*) node_leaf->data)->refbyte;
	encoding_array[x].encodedbyte = encodingbyte;
	encoding_array[x].length = length;
	}


//debug
#ifdef DEBUG
debugm("compression: encoding table")
for (x = 0; x < leafnodes_count; x++) {
	debugc(encoding_array[x].decodedbyte)
	debug(encoding_array[x].encodedbyte)
	debugb(&(encoding_array[x].encodedbyte))
	debug(encoding_array[x].length)
	}
#endif




free(freq_array);
fseek(source_fileptr, 0, SEEK_SET);


int 			index;
unsigned char 	decodedbyte;
unsigned int 	encodedbyte;
unsigned char 	bit_buffer;
int 			bit_width;

bit_width 	= BYTEWIDTH;
bit_buffer 	= 0;
length 		= 0;
x 			= 0;
debug(source_filesize)
while (1) {
	if (bit_width <= 0) {
		debugm("output frame")
		debug(*((int*)&bit_buffer))
		debugb(&bit_buffer)
		fputc(bit_buffer, dest_fileptr);
		bit_buffer = 0;
		bit_width = BYTEWIDTH;
		}
	if (length <= 0) {
		if (x >= source_filesize) {
			if (bit_width != BYTEWIDTH) {
				bit_buffer = bit_buffer << bit_width;
				debugm("last frame")
				debugb(&bit_buffer)
				fputc(bit_buffer, dest_fileptr);
				}
			debugm("padding")
			debug(bit_width%BYTEWIDTH)
			fputc(bit_width % BYTEWIDTH, dest_fileptr);
			break;
			}
		decodedbyte = fgetc(source_fileptr);
		index = 0;
		while (encoding_array[index].decodedbyte != decodedbyte) {
			index++;
			}
		encodedbyte = encoding_array[index].encodedbyte;
		length = encoding_array[index].length;
		x++;
		}

//	debug(bit_width)
//	debug(length)
	bit_buffer = bit_buffer << 1;
	bit_buffer |= (char) (encodedbyte >> (length - 1)) & 1;

	bit_width--;
	length--;

	}

//exit(0);
//bst_free(&tree);
//free(leafnodes);

return;

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

int 			x;
char 			type;
long* 			freq_array;
int 			source_filesize;
int 			data_size;
int 			leafnodes_count;
int 			freq_entrybytesize;

int 			buffer;
unsigned char 	byte;

source_filesize = getfilesize(source_fileptr);


debugm("decompression")


freq_array = (long*) malloc(BYTESIZE * sizeof(long));
type = fgetc(source_fileptr);
if (type > 0) {
	debugm("compression: tabletype1")
	freq_entrybytesize = type;
	leafnodes_count = 0;
	for (x = 0; x < BYTESIZE; x++) {
		buffer = 0;
		fread(&buffer, 1, freq_entrybytesize, source_fileptr);
		freq_array[x] = buffer;
		if (buffer != 0)
			leafnodes_count++;
		}
	data_size = source_filesize -
				BYTESIZE * freq_entrybytesize -
				TABLETYPE1_HEADERSIZE -
				ENDING_OFFSET;
	}
else {
	debugm("compression: tabletype2")
	freq_entrybytesize = -((char) type);
	debug(freq_entrybytesize);
	leafnodes_count = fgetc(source_fileptr);
	for (x = 0; x < BYTESIZE; x++) {
		freq_array[x] = 0;
		}
	for (x = 0; x < leafnodes_count; x++) {
		byte = fgetc(source_fileptr);
		buffer = 0;
		fread(&buffer, 1, freq_entrybytesize, source_fileptr);
		freq_array[byte] = buffer;
		}
	data_size = source_filesize -
				leafnodes_count * (freq_entrybytesize + 1) -
				TABLETYPE2_HEADERSIZE -
				ENDING_OFFSET;
	}



debug(leafnodes_count)
debug(data_size)

#ifdef DEBUG2
for (x = 0; x < BYTESIZE; x++) {
	debug(x)
	debug(freq_array[x])
	}

exit(0);
#endif



BstNode** 		leafnodes;
BstNode* 		node_ptr;
BstNodeData* 	data_ptr;
int 			leafnodes_index;

leafnodes = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
leafnodes_index = 0;
for (x = 0; x < BYTESIZE; x++) {
	// only include the bytes with a good occurence
	if (freq_array[x] > 0) {
		data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
		data_ptr->refbyte 		= x;
		data_ptr->freq 			= freq_array[x];

		node_ptr = (BstNode*) malloc(sizeof(BstNode));
		node_ptr->left 		= NULL;
		node_ptr->right 	= NULL;
		node_ptr->parent 	= NULL;
		node_ptr->data 		= data_ptr;
		leafnodes[leafnodes_index] = node_ptr;

		leafnodes_index++;
		}
	}




int 		smallindex_one;
int 		smallindex_two;
int 		smallval_one;
int 		smallval_two;
int 		currentvalue;
int 		nodes_count;
BstNode** 	nodes;
BstNode* 	node_left;
BstNode* 	node_right;
BstNode* 	node_combine;
BstNode* 	root;
BstTree 	tree;
#ifdef DEBUG
#endif


nodes = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
memcpy(nodes, leafnodes, leafnodes_count * sizeof(BstNode*));
//debug
for (x = 0; x < leafnodes_count; x++) {
	data_ptr = (BstNodeData*) nodes[x]->data;
	debugc(data_ptr->refbyte)
	debug(data_ptr->freq)
	}

nodes_count = leafnodes_count;
smallindex_one = 0;
smallindex_two = 0;

while (nodes_count > 1) {
	// initialize
	data_ptr = (BstNodeData*) nodes[smallindex_one]->data;
	smallval_one = 0;
	smallval_two = 0;
	for (x = 0; x < leafnodes_count; x++) {
		if (nodes[x] != NULL) {
			data_ptr = (BstNodeData*) nodes[x]->data;
			currentvalue = data_ptr->freq;
			debug(currentvalue)
			if (	(smallval_one > currentvalue) ||
					(smallval_one == 0)) {
				smallval_two = smallval_one;
				smallindex_two = smallindex_one;
				smallval_one = currentvalue;
				smallindex_one = x;
				}
			else if (	(smallval_two > currentvalue) ||
						(smallval_two == 0)) {
				smallval_two = currentvalue;
				smallindex_two = x;
				}
			}
		}
	debug(smallval_one)
	debug(smallval_two)

	node_left 	= nodes[smallindex_one];
	node_right 	= nodes[smallindex_two];

	data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
	data_ptr->refbyte 	= 0;
	data_ptr->freq 		= smallval_one + smallval_two;

	node_combine = (BstNode*) malloc(sizeof(BstNode));
	node_combine->parent 	= NULL;
	node_combine->left 		= node_left;
	node_combine->right 	= node_right;
	node_combine->data 		= data_ptr;

	node_left->parent 	= node_combine;
	node_right->parent 	= node_combine;

	((BstNodeData*) node_left->data)->bitvalue 	= 0;
	((BstNodeData*) node_right->data)->bitvalue = 1;

	nodes[smallindex_one] = node_combine;
	nodes[smallindex_two] = NULL;
	nodes_count--;
	}

root = node_combine;
((BstNodeData*) root->data)->bitvalue = -1;

tree.root = node_combine;
#ifdef DEBUG
bst_debugprinttree(&tree, &getdata);
#endif




/* traveling up from leaf node to root
 */

ByteEncoding* 	encoding_array;
BstNode* 		node_leaf;
BstNode* 		node;
int 		 	encodingbyte;
int 		 	encodingbyte_next;
int 			length;

encoding_array = 	(ByteEncoding*)
					malloc(leafnodes_count * sizeof(ByteEncoding));

for (x = 0; x < leafnodes_count; x++) {
	node_leaf = leafnodes[x];
	node = node_leaf;
	encodingbyte = 0;
	length = 0;

	while (node->parent != NULL) {
		debug(encodingbyte)
		encodingbyte_next = ((BstNodeData*) node->data)->bitvalue;
		debug(encodingbyte_next)
		encodingbyte = encodingbyte | (encodingbyte_next << length);
		length++;
		node = node->parent;
		}

	encoding_array[x].decodedbyte = ((BstNodeData*) node_leaf->data)->refbyte;
	encoding_array[x].encodedbyte = encodingbyte;
	encoding_array[x].length = length;
	}


//debug
#ifdef DEBUG
debugm("decompression: encoding table")
for (x = 0; x < leafnodes_count; x++) {
	debugc(encoding_array[x].decodedbyte)
	debugb(&(encoding_array[x].encodedbyte))
	debug(encoding_array[x].length)
	}
#endif





unsigned char 	encodedbuffer;
unsigned char 	decodedbyte;
int 			bitvalue;
int 			bit_width;
unsigned char 	padding;


node 		= root;
x 			= 0;
bit_width 	= 0;
padding 	= 0;

debug(data_size)
while (1) {
	debug(bit_width)
	debug(x)

	if (bit_width <= 0)	{
		if (x >= data_size) {
			break;
			}

		encodedbuffer = fgetc(source_fileptr);
		if (x >= (data_size - 1)) {
			padding = fgetc(source_fileptr);
			debugm("padding")
			debug(padding)
			}
		debugm("new frame")
		debugb(&encodedbuffer)
		x++;
		bit_width = BYTEWIDTH - padding;
		}

	//debug(bit_width)
	//debug(bitvalue)

	bitvalue = (encodedbuffer >> (bit_width - 1 + padding)) & 1;
	bit_width--;

	if (bitvalue == 1)
		node = node->right;
	else
		node = node->left;

	decodedbyte = ((BstNodeData*) node->data)->refbyte;
	if ((node->left == NULL) || (node->right == NULL)) {
		debugm("character");
		debugc(decodedbyte)
		fputc(decodedbyte, dest_fileptr);
		node = root;
		}
	}

test
if (node != root)
	fprintf(stdout, "Error at end of file\n");

//bst_free(&tree);
//free(leafnodes);

return;

}











/* ----------------------------------------------------------------------
 * function: 	bst_free()
 * description: frees all data and nodes in the tree
 * input: 		tree - pointer to a tree
 * notes: 		depends on the recursive bst_free_rec function
 */

void bst_free(BstTree* tree)
{
if (tree == NULL)
	return;
bst_free_rec(tree->root);
return;
}




/* ----------------------------------------------------------------------
 * function: 	bst_free_rec()
 * description: frees all data and nodes in subnodes
 * input: 		node - pointer to a node
 * notes:
 */
void bst_free_rec(BstNode* node)
{
//debug(node)
if (node == NULL)
	return;
bst_free_rec(node->left);
bst_free_rec(node->right);
test
if (node->data != NULL)
	free(node->data);
test
free(node);
test
return;
}






/* ----------------------------------------------------------------------
 * function: 	bst_debugprinttree
 * description: prints out a visual representation of the given tree. the
 * 				first line has the root node, the second line its children,
 * 				etc. a border around the tree is also printed
 * input: 		tree - pointer to a tree
 * notes: 		depends on the recursive bst_debugprint_rec function
 */
void bst_debugprinttree(BstTree* tree, GetDataFunc getdata)
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
	bst_debugprint_rec(tree->root, 0, x, height, getdata);
	printf("  |\n");
	}

// printing bottom border
for (x = 0; x < borderwidth; x++) {
	printf("-");
	}
printf("\n");

}









/* ----------------------------------------------------------------------
 * function: 	bst_debugprint_rec
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
void bst_debugprint_rec(	BstNode* node,
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
	bst_debugprint_rec(NULL, level+1, level_print, height, getdata);
	printf("%*c", BST_PRINT_OFFSET, ' ');
	bst_debugprint_rec(NULL, level+1, level_print, height, getdata);
	}
else {
	// if node is not NULL, the output is a key only if its is on the level
	// to be printed
	bst_debugprint_rec(node->left, level+1, level_print, height, getdata);
	if (level == level_print) {
		(*getdata)(node->data, buffer);
		printf("[%*s]", BST_PRINT_OFFSET - 2, buffer);
		}
	else {
		printf("%*c", BST_PRINT_OFFSET, ' ');
		}
	bst_debugprint_rec(node->right, level+1, level_print, height, getdata);
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
uint32_t getfilesize(FILE* fileptr)
{
uint32_t filesize;
fseek(fileptr, 0L, SEEK_END);
filesize = ftell(fileptr);
rewind(fileptr);
return filesize;
}



/* ----------------------------------------------------------------------
 * function: 	quick_log2()
 * description: quick base 2 log
 * input: 		i - int to be converted
 * output: 		log 2 integer
 */

uint32_t quick_log2(uint32_t i)
{
uint32_t l;
l = 0;
while (i >>= 1) {
	l++;
	}
return l;
}


