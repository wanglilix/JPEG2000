
#include "mi_includes.h"

/* 
==========================================================
   Tag-tree coder interface
==========================================================
*/

mi_tgt_tree_t *mi_tgt_create(mi_UINT32 numleafsh, mi_UINT32 numleafsv, mi_event_mgr_t *manager) {
        mi_INT32 nplh[32];
        mi_INT32 nplv[32];
        mi_tgt_node_t *node = 00;
        mi_tgt_node_t *l_parent_node = 00;
        mi_tgt_node_t *l_parent_node0 = 00;
        mi_tgt_tree_t *tree = 00;
        mi_UINT32 i;
        mi_INT32  j,k;
        mi_UINT32 numlvls;
        mi_UINT32 n;

        tree = (mi_tgt_tree_t *) mi_calloc(1,sizeof(mi_tgt_tree_t));
        if(!tree) {
                mi_event_msg(manager, EVT_ERROR, "Not enough memory to create Tag-tree\n");
                return 00;
        }

        tree->numleafsh = numleafsh;
        tree->numleafsv = numleafsv;

        numlvls = 0;
        nplh[0] = (mi_INT32)numleafsh;
        nplv[0] = (mi_INT32)numleafsv;
        tree->numnodes = 0;
        do {
                n = (mi_UINT32)(nplh[numlvls] * nplv[numlvls]);
                nplh[numlvls + 1] = (nplh[numlvls] + 1) / 2;
                nplv[numlvls + 1] = (nplv[numlvls] + 1) / 2;
                tree->numnodes += n;
                ++numlvls;
        } while (n > 1);

        /* ADD */
        if (tree->numnodes == 0) {
                mi_free(tree);
                mi_event_msg(manager, EVT_WARNING, "tgt_create tree->numnodes == 0, no tree created.\n");
                return 00;
        }

        tree->nodes = (mi_tgt_node_t*) mi_calloc(tree->numnodes, sizeof(mi_tgt_node_t));
        if(!tree->nodes) {
                mi_event_msg(manager, EVT_ERROR, "Not enough memory to create Tag-tree nodes\n");
                mi_free(tree);
                return 00;
        }
        tree->nodes_size = tree->numnodes * (mi_UINT32)sizeof(mi_tgt_node_t);

        node = tree->nodes;
        l_parent_node = &tree->nodes[tree->numleafsh * tree->numleafsv];
        l_parent_node0 = l_parent_node;

        for (i = 0; i < numlvls - 1; ++i) {
                for (j = 0; j < nplv[i]; ++j) {
                        k = nplh[i];
                        while (--k >= 0) {
                                node->parent = l_parent_node;
                                ++node;
                                if (--k >= 0) {
                                        node->parent = l_parent_node;
                                        ++node;
                                }
                                ++l_parent_node;
                        }
                        if ((j & 1) || j == nplv[i] - 1) {
                                l_parent_node0 = l_parent_node;
                        } else {
                                l_parent_node = l_parent_node0;
                                l_parent_node0 += nplh[i];
                        }
                }
        }
        node->parent = 0;
        mi_tgt_reset(tree);
        return tree;
}

/**
 * Reinitialises a tag-tree from an existing one.
 *
 * @param       p_tree                          the tree to reinitialize.
 * @param       p_num_leafs_h           the width of the array of leafs of the tree
 * @param       p_num_leafs_v           the height of the array of leafs of the tree
 * @return      a new tag-tree if successful, NULL otherwise
*/
mi_tgt_tree_t *mi_tgt_init(mi_tgt_tree_t * p_tree,mi_UINT32 p_num_leafs_h, mi_UINT32 p_num_leafs_v, mi_event_mgr_t *p_manager)
{
        mi_INT32 l_nplh[32];
        mi_INT32 l_nplv[32];
        mi_tgt_node_t *l_node = 00;
        mi_tgt_node_t *l_parent_node = 00;
        mi_tgt_node_t *l_parent_node0 = 00;
        mi_UINT32 i;
        mi_INT32 j,k;
        mi_UINT32 l_num_levels;
        mi_UINT32 n;
        mi_UINT32 l_node_size;

        if (! p_tree){
                return 00;
        }

        if ((p_tree->numleafsh != p_num_leafs_h) || (p_tree->numleafsv != p_num_leafs_v)) {
                p_tree->numleafsh = p_num_leafs_h;
                p_tree->numleafsv = p_num_leafs_v;

                l_num_levels = 0;
                l_nplh[0] = (mi_INT32)p_num_leafs_h;
                l_nplv[0] = (mi_INT32)p_num_leafs_v;
                p_tree->numnodes = 0;
                do
                {
                        n = (mi_UINT32)(l_nplh[l_num_levels] * l_nplv[l_num_levels]);
                        l_nplh[l_num_levels + 1] = (l_nplh[l_num_levels] + 1) / 2;
                        l_nplv[l_num_levels + 1] = (l_nplv[l_num_levels] + 1) / 2;
                        p_tree->numnodes += n;
                        ++l_num_levels;
                }
                while (n > 1);

                /* ADD */
                if (p_tree->numnodes == 0) {
                        mi_tgt_destroy(p_tree);
                        return 00;
                }
                l_node_size = p_tree->numnodes * (mi_UINT32)sizeof(mi_tgt_node_t);
                
                if (l_node_size > p_tree->nodes_size) {
                        mi_tgt_node_t* new_nodes = (mi_tgt_node_t*) mi_realloc(p_tree->nodes, l_node_size);
                        if (! new_nodes) {
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to reinitialize the tag tree\n");
                                mi_tgt_destroy(p_tree);
                                return 00;
                        }
                        p_tree->nodes = new_nodes;
                        memset(((char *) p_tree->nodes) + p_tree->nodes_size, 0 , l_node_size - p_tree->nodes_size);
                        p_tree->nodes_size = l_node_size;
                }
                l_node = p_tree->nodes;
                l_parent_node = &p_tree->nodes[p_tree->numleafsh * p_tree->numleafsv];
                l_parent_node0 = l_parent_node;

                for (i = 0; i < l_num_levels - 1; ++i) {
                        for (j = 0; j < l_nplv[i]; ++j) {
                                k = l_nplh[i];
                                while (--k >= 0) {
                                        l_node->parent = l_parent_node;
                                        ++l_node;
                                        if (--k >= 0) {
                                                l_node->parent = l_parent_node;
                                                ++l_node;
                                        }
                                        ++l_parent_node;
                                        }
                                if ((j & 1) || j == l_nplv[i] - 1)
                                {
                                        l_parent_node0 = l_parent_node;
                                }
                                else
                                {
                                        l_parent_node = l_parent_node0;
                                        l_parent_node0 += l_nplh[i];
                                }
                        }
                }
                l_node->parent = 0;
        }
        mi_tgt_reset(p_tree);

        return p_tree;
}

void mi_tgt_destroy(mi_tgt_tree_t *p_tree)
{
        if (! p_tree) {
                return;
        }

        if (p_tree->nodes) {
                mi_free(p_tree->nodes);
                p_tree->nodes = 00;
        }
        mi_free(p_tree);
}

void mi_tgt_reset(mi_tgt_tree_t *p_tree) {
        mi_UINT32 i;
        mi_tgt_node_t * l_current_node = 00;;

        if (! p_tree) {
                return;
        }

        l_current_node = p_tree->nodes;
        for     (i = 0; i < p_tree->numnodes; ++i)
        {
                l_current_node->value = 999;
                l_current_node->low = 0;
                l_current_node->known = 0;
                ++l_current_node;
        }
}

void mi_tgt_setvalue(mi_tgt_tree_t *tree, mi_UINT32 leafno, mi_INT32 value) {
        mi_tgt_node_t *node;
        node = &tree->nodes[leafno];
        while (node && node->value > value) {
                node->value = value;
                node = node->parent;
        }
}

void mi_tgt_encode(mi_bio_t *bio, mi_tgt_tree_t *tree, mi_UINT32 leafno, mi_INT32 threshold) {
        mi_tgt_node_t *stk[31];
        mi_tgt_node_t **stkptr;
        mi_tgt_node_t *node;
        mi_INT32 low;

        stkptr = stk;
        node = &tree->nodes[leafno];
        while (node->parent) {
                *stkptr++ = node;
                node = node->parent;
        }
        
        low = 0;
        for (;;) {
                if (low > node->low) {
                        node->low = low;
                } else {
                        low = node->low;
                }
                
                while (low < threshold) {
                        if (low >= node->value) {
                                if (!node->known) {
                                        mi_bio_write(bio, 1, 1);
                                        node->known = 1;
                                }
                                break;
                        }
                        mi_bio_write(bio, 0, 1);
                        ++low;
                }
                
                node->low = low;
                if (stkptr == stk)
                        break;
                node = *--stkptr;
        }
}

mi_UINT32 mi_tgt_decode(mi_bio_t *bio, mi_tgt_tree_t *tree, mi_UINT32 leafno, mi_INT32 threshold) {
        mi_tgt_node_t *stk[31];
        mi_tgt_node_t **stkptr;
        mi_tgt_node_t *node;
        mi_INT32 low;

        stkptr = stk;
        node = &tree->nodes[leafno];
        while (node->parent) {
                *stkptr++ = node;
                node = node->parent;
        }
        
        low = 0;
        for (;;) {
                if (low > node->low) {
                        node->low = low;
                } else {
                        low = node->low;
                }
                while (low < threshold && low < node->value) {
                        if (mi_bio_read(bio, 1)) {
                                node->value = low;
                        } else {
                                ++low;
                        }
                }
                node->low = low;
                if (stkptr == stk) {
                        break;
                }
                node = *--stkptr;
        }
        
        return (node->value < threshold) ? 1 : 0;
}
