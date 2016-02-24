/* ---- FILE HEADER -----------------------------------------------------
 * project: 	lab4
 * file: 		bst.h
 * author: 		Elias Abderhalden / Tyler Fitzgerald
 * date: 		2016-02-22
 * ----------------------------------------------------------------------
 * class: 		ece4680 spring 2016
 * instructor: 	Adam Hoover
 * assignment: 	lab4
 * purpose: 	hoffman compression
 * ----------------------------------------------------------------------
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





