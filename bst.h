




typedef struct BstNodeTag {
	void* 				data;
	struct BstNodeTag* 	parent;
	struct BstNodeTag* 	left;
	struct BstNodeTag* 	right;
	} BstNode;

typedef struct BstTreeTag {
	BstNode* 	root;
	int 		tree_size;
	} BstTree;


typedef void (*GetDataFunc)(void*, char*);

void bst_free(BstTree* tree);
void bst_free_rec(BstNode* node);
void bst_debugprinttree(BstTree* tree, GetDataFunc getdata);
void bst_debugprint_rec(	BstNode* node,
							int level,
							int level_print,
							int height,
							GetDataFunc getdata);
int bst_height(BstTree* tree);
int bst_height_rec(BstNode* node, int level);





