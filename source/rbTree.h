/* $Id: rbTree.h,v 1.3 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_RBTREE_H_INCLUDED
#define NEDIT_RBTREE_H_INCLUDED

typedef struct rbTreeNode {
    struct rbTreeNode *left;   /* left child */
    struct rbTreeNode *right;  /* right child */
    struct rbTreeNode *parent; /* parent */
    int color;                 /* node color (rbTreeNodeBlack, rbTreeNodeRed) */
} rbTreeNode;

typedef int (*rbTreeCompareNodeCB)(rbTreeNode *left, rbTreeNode *right);
typedef rbTreeNode *(*rbTreeAllocateNodeCB)(rbTreeNode *src);
typedef rbTreeNode *(*rbTreeAllocateEmptyNodeCB)(void);
typedef void (*rbTreeDisposeNodeCB)(rbTreeNode *src);
typedef int (*rbTreeCopyToNodeCB)(rbTreeNode *dst, rbTreeNode *src);

rbTreeNode *rbTreeBegin(rbTreeNode *base);
rbTreeNode *rbTreeReverseBegin(rbTreeNode *base);
rbTreeNode *rbTreeFind(rbTreeNode *base, rbTreeNode *searchNode,
                        rbTreeCompareNodeCB compareRecords);
rbTreeNode *rbTreeInsert(rbTreeNode *base, rbTreeNode *searchNode,
                            rbTreeCompareNodeCB compareRecords,
                            rbTreeAllocateNodeCB allocateNode,
                            rbTreeCopyToNodeCB copyToNode);
rbTreeNode *rbTreeUnlinkNode(rbTreeNode *base, rbTreeNode *z);
void rbTreeDeleteNode(rbTreeNode *base, rbTreeNode *foundNode,
                    rbTreeDisposeNodeCB disposeNode);
int rbTreeDelete(rbTreeNode *base, rbTreeNode *searchNode,
                    rbTreeCompareNodeCB compareRecords,
                    rbTreeDisposeNodeCB disposeNode);
rbTreeNode *rbTreeNext(rbTreeNode *x);
rbTreeNode *rbTreePrevious(rbTreeNode *x);
int rbTreeSize(rbTreeNode *base);
rbTreeNode *rbTreeNew(rbTreeAllocateEmptyNodeCB allocateEmptyNode);
void rbTreeDispose(rbTreeNode *base, rbTreeDisposeNodeCB disposeNode);

#endif /* NEDIT_RBTREE_H_INCLUDED */
