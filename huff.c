#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
	// string.h only used to deal with command line input and output
#include <ctype.h>
//#include <stdint.h>

#include "bst.h"




#if __GNUC__
#if __x86_64__ || __powerpc64__
#define ENV_64bit 1
#else
#if __i386__ || __powerpc__
#define ENV_32bit 1
#endif
#endif
#endif



#if ENV_32bit || ENV_64bit
typedef unsigned long int huff_uint32;
#else
#error system environment not supported
#endif

#define BYTESIZE 256
#define BYTEWIDTH 8
#define BST_PRINT_OFFSET 10
#define TABLETYPE1_HEADERSIZE 1
#define TABLETYPE2_HEADERSIZE 2
#define ENDING_OFFSET 1


//#define DEBUG 1
#include "debug.h"

typedef enum {COMPRESS, DECOMPRESS} Option;
typedef struct ByteEncodingDef {
	unsigned char 	decoded_byte;
	huff_uint32 	encoded_bitarray;
	int 			length;
	} ByteEncoding;
typedef struct BstNodeDataDef {
	unsigned int 	bitvalue;
	huff_uint32		freq;
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
huff_uint32 quick_log2(huff_uint32 i);
huff_uint32 getfilesize(FILE* fileptr);






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

int 	source_filesize;
int 	dest_filesize;
float 	ratio;

if (option == COMPRESS) {
	compression(source_fileptr, dest_fileptr);

	source_filesize = getfilesize(source_fileptr);
	dest_filesize = getfilesize(dest_fileptr);
	ratio = dest_filesize/((float) source_filesize);

	fprintf( 	stdout,
				"original size: \t%d (0x%x)\n",
				source_filesize,
				source_filesize);
	fprintf( 	stdout,
				"new size: \t%d (0x%x)\n",
				dest_filesize,
				dest_filesize);
	fprintf( 	stdout,
				"ratio: \t\t%f (%.2f%%)\n",
				ratio,
				ratio * 100);
	}
else if (option == DECOMPRESS) {
	decompression(source_fileptr, dest_fileptr);
	}

fclose(dest_fileptr);
fclose(source_fileptr);

return 1;
}



/* ----------------------------------------------------------------------
 * function: 	make_leafnodes()
 * description: makes leaf nodes out of the frequency array.
 * input: 		freq_array - frequency array
 * 				leafnodes_count - count of leaf nodes
 * output: 		leafnodes - address of BstLeafNodes structure to be filled out
 */
void make_leafnodes(
	huff_uint32* freq_array,
	int leafnodes_count,
	BstLeafNodes* leafnodes)
{
int x;
BstNode** 		leafnodes_array;
BstNode* 		node_ptr;
BstNodeData* 	data_ptr;
//BstLeafNodes* 	leafnodes;
int 			leafnodes_index;

leafnodes_array = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
leafnodes_index = 0;
for (x = 0; x < BYTESIZE; x++) {
	// only include the bytes with a good occurence
	if (freq_array[x] > 0) {
		data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
		data_ptr->refbyte 	= x;
		data_ptr->freq 		= freq_array[x];

		node_ptr = (BstNode*) malloc(sizeof(BstNode));
		node_ptr->left 		= NULL;
		node_ptr->right 	= NULL;
		node_ptr->parent 	= NULL;
		node_ptr->data 		= data_ptr;
		leafnodes_array[leafnodes_index] = node_ptr;

		leafnodes_index++;
		}
	}
//leafnodes = (BstLeafNodes*) malloc(sizeof(BstLeafNodes));
leafnodes->array = leafnodes_array;
leafnodes->count = leafnodes_count;
}



/* ----------------------------------------------------------------------
 * function: 	make_binarytree()
 * description: takes an array of nodes and builds a tree.
 * input: 		leafnodes_array - array of nodes
 * 				leafnodes_count - size of leafnodes_array array
 * output: 		encoding_array_ret - address
 */
void make_binarytree(BstTree* tree)
{
int 			x;
int 			smallindex_one;
int 			smallindex_two;
long 			smallval_one;
long 			smallval_two;
long 			currentvalue;
int 			nodes_count;
BstNode** 		nodes;
BstNode* 		node_left;
BstNode* 		node_right;
BstNode* 		node_combine;
BstNodeData* 	data_ptr;
BstNode** 		leafnodes_array;
int 			leafnodes_count;

leafnodes_array = tree->leafnodes.array;
leafnodes_count = tree->leafnodes.count;


nodes = (BstNode**) malloc(leafnodes_count * sizeof(BstNode*));
memcpy(nodes, leafnodes_array, leafnodes_count * sizeof(BstNode*));

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
			if ((smallval_one > currentvalue) || (smallval_one == 0)) {
				smallval_two 	= smallval_one;
				smallindex_two 	= smallindex_one;
				smallval_one 	= currentvalue;
				smallindex_one 	= x;
				}
			else if ((smallval_two > currentvalue) || (smallval_two == 0)) {
				smallval_two 	= currentvalue;
				smallindex_two 	= x;
				}
			}
		}

	node_left 				= nodes[smallindex_one];
	node_right 				= nodes[smallindex_two];

	data_ptr = (BstNodeData*) malloc(sizeof(BstNodeData));
	data_ptr->refbyte 		= 0;
	data_ptr->freq 			= smallval_one + smallval_two;

	node_combine = (BstNode*) malloc(sizeof(BstNode));
	node_combine->parent 	= NULL;
	node_combine->left 		= node_left;
	node_combine->right 	= node_right;
	node_combine->data 		= data_ptr;

	node_left->parent 		= node_combine;
	node_right->parent 		= node_combine;

	((BstNodeData*) node_left->data)->bitvalue 	= 0;
	((BstNodeData*) node_right->data)->bitvalue = 1;

	nodes[smallindex_one] 	= node_combine;
	nodes[smallindex_two] 	= NULL;
	nodes_count--;
	}

BstNode* 		root;

root = node_combine;
((BstNodeData*) root->data)->bitvalue = -1;

tree->leafnodes.array 	= leafnodes_array;
tree->leafnodes.count 	= leafnodes_count;
tree->root = root;

free(nodes);
}





/* ----------------------------------------------------------------------
 * function: 	nodearray_encoding()
 * description: takes an array of nodes, builds a tree, and fills in the
 * 				encoding array.
 * input: 		leafnodes_array - array of nodes
 * 				leafnodes_count - size of leafnodes_array array
 * output: 		encoding_array_ret - address
 */

void tree_encoding(
	BstTree* 		tree,
	ByteEncoding** 	encoding_array_ret)
{
// traveling up from leaf node to root

int 			x;
int 		 	encodingbyte;
int 		 	encodingbyte_next;
int 			length;
ByteEncoding* 	encoding_array;
BstNode* 		node_leaf;
BstNode* 		node;
BstNodeData* 	data_ptr;

BstNode** 		leafnodes_array;
int 			leafnodes_count;

leafnodes_array = tree->leafnodes.array;
leafnodes_count = tree->leafnodes.count;

encoding_array = 	(ByteEncoding*)
					malloc(leafnodes_count * sizeof(ByteEncoding));

for (x = 0; x < leafnodes_count; x++) {
	node_leaf 		= leafnodes_array[x];
	node 			= node_leaf;
	encodingbyte 	= 0;
	length 			= 0;
	while (node->parent != NULL) {
		encodingbyte_next = ((BstNodeData*) node->data)->bitvalue;
		encodingbyte = encodingbyte | (encodingbyte_next << length);
		node = node->parent;
		length++;
		}
	data_ptr = (BstNodeData*) node_leaf->data;
	encoding_array[x].decoded_byte 	= data_ptr->refbyte;
	encoding_array[x].encoded_bitarray 	= encodingbyte;
	encoding_array[x].length 		= length;
	}

*encoding_array_ret = encoding_array;

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

huff_uint32 	x;
unsigned char 	byte;
huff_uint32* 	freq_array;
huff_uint32 	source_filesize;
huff_uint32 	freq_entrymaxsize;
int 			leafnodes_count;

source_filesize = getfilesize(source_fileptr);


// initialize freq_array
freq_array = (huff_uint32*) malloc(BYTESIZE * sizeof(huff_uint32));
for (x = 0; x < BYTESIZE; x++) {
	freq_array[x] = 0;
	}

// set freq_array
freq_entrymaxsize = UCHAR_MAX;
leafnodes_count = 0;

for (x = 0; x < source_filesize; x++) {
	byte = fgetc(source_fileptr);
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



// initialize leafnodes_array
BstTree 	tree;

make_leafnodes(freq_array, leafnodes_count, &tree.leafnodes);




// write freq_array to file
int 			tabletype1_size;
int 			tabletype2_size;
int 			freq_entrybytesize;
char* 			freq_ptr;
BstNode** 		leafnodes_array;
BstNodeData* 	data_ptr;

leafnodes_count = tree.leafnodes.count;
leafnodes_array = tree.leafnodes.array;

freq_entrybytesize = quick_log2(freq_entrymaxsize + 1) / BYTEWIDTH;
tabletype1_size = 	BYTESIZE * freq_entrybytesize +
					TABLETYPE1_HEADERSIZE;
tabletype2_size = 	leafnodes_count * freq_entrybytesize +
					TABLETYPE2_HEADERSIZE;

if (tabletype1_size <= tabletype2_size) {
	fputc(freq_entrybytesize, dest_fileptr);
	for (x = 0; x < BYTESIZE; x++) {
		freq_ptr = ((char*) &(freq_array[x]));
		fwrite(freq_ptr, 1, freq_entrybytesize, dest_fileptr);
		}
	}
else {
	fputc(-freq_entrybytesize, dest_fileptr);
	fputc(leafnodes_count, dest_fileptr);
	for (x = 0; x < leafnodes_count; x++) {
		data_ptr = (BstNodeData*) leafnodes_array[x]->data;
		byte = data_ptr->refbyte;
		fputc(byte, dest_fileptr);
		freq_ptr = ((char*) &(data_ptr->freq));
			// this works on little endian systems
		fwrite(freq_ptr, 1, freq_entrybytesize, dest_fileptr);
		}
	}

free(freq_array);


// encoding
ByteEncoding* 	encoding_array;

make_binarytree(&tree);
tree_encoding(&tree, &encoding_array);

#if DEBUG
bst_debugprinttree(&tree, &getdata);
for (x = 0; x < leafnodes_count; x++) {
	debugc(encoding_array[x].decoded_byte)
	debug(encoding_array[x].encoded_bitarray)
	debugb(&(encoding_array[x].encoded_bitarray))
	debug(encoding_array[x].length)
	}
#endif

bst_freenodes(&tree);
free(tree.leafnodes.array);





// read source file and output encodings
unsigned char 	decoded_byte;
unsigned char 	bit_buffer;
huff_uint32 	encoded_bitarray;
int 			index;
int 			bit_width;
int 			length;

fseek(source_fileptr, 0, SEEK_SET);

bit_width 	= BYTEWIDTH;
bit_buffer 	= 0;
length 		= 0;
x 			= 0;
while (1) {
	if (bit_width <= 0) {
		fputc(bit_buffer, dest_fileptr);
		bit_buffer = 0;
		bit_width = BYTEWIDTH;
		}
	if (length <= 0) {
		if (x >= source_filesize) {
			if (bit_width != BYTEWIDTH) {
				bit_buffer = bit_buffer << bit_width;
				fputc(bit_buffer, dest_fileptr);
				}
			fputc(bit_width % BYTEWIDTH, dest_fileptr);
			break;
			}
		decoded_byte = fgetc(source_fileptr);
		index = 0;
		while (encoding_array[index].decoded_byte != decoded_byte) {
			index++;
			}
		encoded_bitarray = encoding_array[index].encoded_bitarray;
		length = encoding_array[index].length;
		x++;
		}
	bit_buffer = bit_buffer << 1;
	bit_buffer |= (char) (encoded_bitarray >> (length - 1)) & 1;
	bit_width--;
	length--;
	}

free(encoding_array);

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
huff_uint32* 	freq_array;
int 			source_filesize;
int 			data_size;
int 			leafnodes_count;
int 			freq_entrybytesize;

int 			buffer;
unsigned char 	byte;

source_filesize = getfilesize(source_fileptr);


// read source file and set freq_array
freq_array = (huff_uint32*) malloc(BYTESIZE * sizeof(huff_uint32));
type = fgetc(source_fileptr);
if (type > 0) {
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
	freq_entrybytesize = -((char) type);
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





// make leaf nodes out of the frequency array
BstTree tree;

make_leafnodes(freq_array, leafnodes_count, &tree.leafnodes);
free(freq_array);




// encoding

make_binarytree(&tree);
free(tree.leafnodes.array);


#if DEBUG
bst_debugprinttree(&tree, &getdata);
debugm("decompression: encoding table")
for (x = 0; x < leafnodes_count; x++) {
	debugc(encoding_array[x].decoded_byte)
	debugb(&(encoding_array[x].encoded_bitarray))
	debug(encoding_array[x].length)
	}
#endif


// trace bits through binary tree and output decoded byte
unsigned char 	encodedbuffer;
unsigned char 	decoded_byte;
unsigned char 	padding;
int 			bitvalue;
int 			bit_width;
BstNode* 		root;
BstNode* 		node;

root 		= tree.root;
node 		= root;
x 			= 0;
bit_width 	= 0;
padding 	= 0;

while (1) {
	if (bit_width <= 0)	{
		if (x >= data_size) {
			break;
			}
		encodedbuffer = fgetc(source_fileptr);
		if (x >= (data_size - 1)) {
			padding = fgetc(source_fileptr);
			}
		x++;
		bit_width = BYTEWIDTH - padding;
		}
	bitvalue = (encodedbuffer >> (bit_width - 1 + padding)) & 1;
	bit_width--;

	if (bitvalue == 1)
		node = node->right;
	else
		node = node->left;

	decoded_byte = ((BstNodeData*) node->data)->refbyte;
	if ((node->left == NULL) || (node->right == NULL)) {
		fputc(decoded_byte, dest_fileptr);
		node = root;
		}
	}

if (node != root)
	fprintf(stdout, "Error at end of file\n");

bst_freenodes(&tree);

return;

}











/* ----------------------------------------------------------------------
 * function: 	bst_freenodes()
 * description: frees all data and nodes in the tree
 * input: 		tree - pointer to a tree
 * notes: 		depends on the recursive bst_freenodes_rec function
 */

void bst_freenodes(BstTree* tree)
{
if (tree == NULL)
	return;
bst_freenodes_rec(tree->root);
return;
}




/* ----------------------------------------------------------------------
 * function: 	bst_freenodes_rec()
 * description: frees all data and nodes in subnodes
 * input: 		node - pointer to a node
 * notes:
 */
void bst_freenodes_rec(BstNode* node)
{
//debug(node)
if (node == NULL)
	return;
bst_freenodes_rec(node->left);
bst_freenodes_rec(node->right);
if (node->data != NULL)
	free(node->data);
free(node);
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
huff_uint32 getfilesize(FILE* fileptr)
{
huff_uint32 filesize;
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

huff_uint32 quick_log2(huff_uint32 i)
{
huff_uint32 l;
l = 0;
while (i >>= 1) {
	l++;
	}
return l;
}


