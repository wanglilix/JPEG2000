
#ifndef __TGT_H
#define __TGT_H
/**
@file tgt.h
@brief Implementation of a tag-tree coder (TGT)

The functions in TGT.C have for goal to realize a tag-tree coder. The functions in TGT.C
are used by some function in T2.C.
*/

/** @defgroup TGT TGT - Implementation of a tag-tree coder */
/*@{*/

/**
Tag node
*/
typedef struct mi_tgt_node {
    struct mi_tgt_node *parent;//树节点
    mi_INT32 value;//节点值
    mi_INT32 low;
    mi_UINT32 known;
} mi_tgt_node_t;

/**
Tag tree
*/
typedef struct mi_tgt_tree
{
	mi_UINT32  numleafsh;//水平叶子数
	mi_UINT32  numleafsv;//垂直叶子数
	mi_UINT32 numnodes;//节点数
	mi_tgt_node_t *nodes;
	mi_UINT32  nodes_size;		/* maximum size taken by nodes */
} mi_tgt_tree_t;


/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Create a tag-tree
@param numleafsh Width of the array of leafs of the tree
@param numleafsv Height of the array of leafs of the tree
@return Returns a new tag-tree if successful, returns NULL otherwise
*/
mi_tgt_tree_t *mi_tgt_create(mi_UINT32 numleafsh, mi_UINT32 numleafsv, mi_event_mgr_t *manager);

/**
 * Reinitialises a tag-tree from an exixting one.
 *
 * @param	p_tree				the tree to reinitialize.
 * @param	p_num_leafs_h		the width of the array of leafs of the tree
 * @param	p_num_leafs_v		the height of the array of leafs of the tree
 * @param p_manager       the event manager
 * @return	a new tag-tree if successful, NULL otherwise
*/
mi_tgt_tree_t *mi_tgt_init(mi_tgt_tree_t * p_tree, 
                             mi_UINT32  p_num_leafs_h, 
                             mi_UINT32  p_num_leafs_v, mi_event_mgr_t *p_manager);
/**
Destroy a tag-tree, liberating memory
@param tree Tag-tree to destroy
*/
void mi_tgt_destroy(mi_tgt_tree_t *tree);
/**
Reset a tag-tree (set all leaves to 0)
@param tree Tag-tree to reset
*/
void mi_tgt_reset(mi_tgt_tree_t *tree);
/**
Set the value of a leaf of a tag-tree
@param tree Tag-tree to modify
@param leafno Number that identifies the leaf to modify
@param value New value of the leaf
*/
void mi_tgt_setvalue(mi_tgt_tree_t *tree, 
                      mi_UINT32 leafno, 
                      mi_INT32 value);
/**
Encode the value of a leaf of the tag-tree up to a given threshold
@param bio Pointer to a BIO handle
@param tree Tag-tree to modify
@param leafno Number that identifies the leaf to encode
@param threshold Threshold to use when encoding value of the leaf
*/
void mi_tgt_encode(mi_bio_t *bio, 
                    mi_tgt_tree_t *tree, 
                    mi_UINT32 leafno, 
                    mi_INT32 threshold);
/**
Decode the value of a leaf of the tag-tree up to a given threshold
@param bio Pointer to a BIO handle
@param tree Tag-tree to decode
@param leafno Number that identifies the leaf to decode
@param threshold Threshold to use when decoding value of the leaf
@return Returns 1 if the node's value < threshold, returns 0 otherwise
*/
mi_UINT32 mi_tgt_decode(mi_bio_t *bio, 
                          mi_tgt_tree_t *tree, 
                          mi_UINT32 leafno, 
                          mi_INT32 threshold);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __TGT_H */
