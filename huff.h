/* ---- FILE HEADER -----------------------------------------------------
 * project: 	lab4
 * file: 		huff.h
 * author: 		Elias Abderhalden / Tyler Fitzgerald
 * date: 		2016-02-22
 * ----------------------------------------------------------------------
 * class: 		ece4680 spring 2016
 * instructor: 	Adam Hoover
 * assignment: 	lab4
 * purpose: 	hoffman compression
 * ----------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

//#include "bst.h"


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

/* ----------------------------------------------------------------------
 * addition from bst.h
 * added to this file to reduce total number of files
 */


#define BST_PRINT_OFFSET 10

typedef struct BstNodeDef {
	void* 				data;
	struct BstNodeDef* 	parent;
	struct BstNodeDef* 	left;
	struct BstNodeDef* 	right;
	} BstNode;

typedef struct BstLeafNodesDef {
	BstNode** 	array;
	int 		count;
	} BstLeafNodes;

typedef struct BstTreeDef {
	BstNode* 		root;
	int 			tree_size;
	BstLeafNodes 	leafnodes;
	} BstTree;

typedef void (*GetDataFunc)(void*, char*);


void bst_freenodes(BstTree* tree);
void bst_freenodes_rec(BstNode* node);
void bst_debugprinttree(BstTree* tree, GetDataFunc getdata);
void bst_debugprint_rec(
		BstNode* node,
		int level,
		int level_print,
		int height,
		GetDataFunc getdata);
int bst_height(BstTree* tree);
int bst_height_rec(BstNode* node, int level);




/* huff.h */

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


void compression(FILE* source_fileptr, FILE* dest_fileptr);
void decompression(FILE* source_fileptr, FILE* dest_fileptr);
void make_leafnodes(
		huff_uint32* freq_array,
		int leafnodes_count,
		BstLeafNodes* leafnodes);
void make_binarytree(BstTree* tree);
void tree_encoding(
		BstTree* 		tree,
		ByteEncoding** 	encoding_array_ret);
huff_uint32 quick_log2(huff_uint32 i);
huff_uint32 getfilesize(FILE* fileptr);




