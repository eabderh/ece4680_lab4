

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
void bst_debugprint_rec(	BstNode* node,
							int level,
							int level_print,
							int height,
							GetDataFunc getdata);
int bst_height(BstTree* tree);
int bst_height_rec(BstNode* node, int level);





