#include "rbTree.h"

#include <stdlib.h>
#include <string.h>
/*#define RBTREE_TEST_CODE*/
#ifdef RBTREE_TEST_CODE
#include <stdio.h>
#endif

#define rbTreeNodeRed       0
#define rbTreeNodeBlack     1

static void rotateLeft(rbTreeNode *x, rbTreeNode **root)
{
    rbTreeNode *y = x->right;
    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;

    if (x == *root) {
        *root = y;
    }
    else if (x == x->parent->left) {
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

static void rotateRight(rbTreeNode *x, rbTreeNode **root)
{
    rbTreeNode *y = x->left;
    x->left = y->right;
    if (y->right != NULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;

    if (x == *root) {
        *root = y;
    }
    else if (x == x->parent->right) {
        x->parent->right = y;
    }
    else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

static void insertBalance(rbTreeNode *x, rbTreeNode **root)
{
  x->color = rbTreeNodeRed;
  while (x != *root && x->parent->color == rbTreeNodeRed) {
    if (x->parent == x->parent->parent->left) {
      rbTreeNode *y = x->parent->parent->right;
      if (y && y->color == rbTreeNodeRed) {
        x->parent->color = rbTreeNodeBlack;
        y->color = rbTreeNodeBlack;
        x->parent->parent->color = rbTreeNodeRed;
        x = x->parent->parent;
      }
      else {
        if (x == x->parent->right) {
          x = x->parent;
          rotateLeft(x, root);
        }
        x->parent->color = rbTreeNodeBlack;
        x->parent->parent->color = rbTreeNodeRed;
        rotateRight(x->parent->parent, root);
      }
    }
    else {
      rbTreeNode *y = x->parent->parent->left;
      if (y && y->color == rbTreeNodeRed) {
        x->parent->color = rbTreeNodeBlack;
        y->color = rbTreeNodeBlack;
        x->parent->parent->color = rbTreeNodeRed;
        x = x->parent->parent;
      }
      else {
        if (x == x->parent->left) {
          x = x->parent;
          rotateRight(x, root);
        }
        x->parent->color = rbTreeNodeBlack;
        x->parent->parent->color = rbTreeNodeRed;
        rotateLeft(x->parent->parent, root);
      }
    }
  }
  (*root)->color = rbTreeNodeBlack;
}

rbTreeNode *rbTreeBegin(rbTreeNode *base)
{
    return(base->left);
}

rbTreeNode *rbTreeReverseBegin(rbTreeNode *base)
{
    return(base->right);
}

rbTreeNode *rbTreeFind(rbTreeNode *base, rbTreeNode *searchNode,
                        rbTreeCompareNodeCB compareRecords)
{
    rbTreeNode *foundNode = NULL;
    rbTreeNode *current = base->parent;
    while(current != NULL) {
        int compareResult = compareRecords(searchNode, current);

        if (compareResult < 0) {
            current = current->left;
        }
        else if (compareResult > 0) {
            current = current->right;
        }
        else {
            foundNode = current;
            current = NULL;
        }
    }
    return(foundNode);
}

rbTreeNode *rbTreeInsert(rbTreeNode *base, rbTreeNode *searchNode,
                            rbTreeCompareNodeCB compareRecords,
                            rbTreeAllocateNodeCB allocateNode,
                            rbTreeCopyToNodeCB copyToNode)
{
    rbTreeNode *current, *parent, *x;
    int fromLeft = 0, foundMatch = 0;

    current = base->parent;
    parent = NULL;
    x = NULL;
    while(current != NULL) {
        int compareResult = compareRecords(searchNode, current);

        if (compareResult < 0) {
            parent = current;
            current = current->left;
            fromLeft = 1;
        }
        else if (compareResult > 0) {
            parent = current;
            current = current->right;
            fromLeft = 0;
        }
        else {
            x = current;
            if (!copyToNode(x, searchNode)) {
                x = NULL;
            }
            current = NULL;
            foundMatch = 1;
        }
    }

    if (!foundMatch) {
        x = allocateNode(searchNode);
        if (x) {
            x->parent = parent;
            x->left = NULL;
            x->right = NULL;
            x->color = rbTreeNodeRed;

            if (parent) {
                if (fromLeft) {
                    parent->left = x;
                }
                else {
                    parent->right = x;
                }
            }
            else {
                base->parent = x;
            }
            ++(base->color);
            if (x->parent == base->left && (x->parent == NULL || x->parent->left == x)) {
                base->left = x;
            }
            if (x->parent == base->right && (x->parent == NULL || x->parent->right == x)) {
                base->right = x;
            }
            insertBalance(x, &base->parent);
        }
    }
    return(x);
}

rbTreeNode *rbTreeUnlinkNode(rbTreeNode *base, rbTreeNode *z)
{
    int swapColor;
    rbTreeNode *x, *y, *x_parent;

    y = z;
    if (y->left == NULL) {
        x = y->right;
        if (y == base->left) {
            base->left = rbTreeNext(y);
        }
    }
    else {
        if (y->right == NULL) {
            x = y->left;
            if (y == base->right) {
                base->right = rbTreePrevious(y);
            }
        }
        else {
            y = y->right;
            while (y->left != NULL) {
                y = y->left;
            }
            x = y->right;
        }
    }
    if (y != z) {
        z->left->parent = y;
        y->left = z->left;
        if (y != z->right) {
            x_parent = y->parent;
            if (x != NULL) {
                x->parent = y->parent;
            }
            y->parent->left = x;
            y->right = z->right;
            z->right->parent = y;
        }
        else {
            x_parent = y;
        }
        if (base->parent == z) {
            base->parent = y;
        }
        else if (z->parent->left == z) {
            z->parent->left = y;
        }
        else {
            z->parent->right = y;
        }
        y->parent = z->parent;

        swapColor = y->color;
        y->color = z->color;
        z->color = swapColor;

        y = z;
    }
    else {
        x_parent = y->parent;
        if (x != NULL) {
            x->parent = y->parent;
        }
        if (base->parent == z) {
            base->parent = x;
        }
        else {
            if (z->parent->left == z) {
                z->parent->left = x;
            }
            else {
                z->parent->right = x;
            }
        }
    }

    --(base->color);

    if (y->color != rbTreeNodeRed) { 
        while (x != base->parent && (x == NULL || x->color == rbTreeNodeBlack)) {
            if (x == x_parent->left) {
                rbTreeNode *w = x_parent->right;
                if (w->color == rbTreeNodeRed) {
                    w->color = rbTreeNodeBlack;
                    x_parent->color = rbTreeNodeRed;
                    rotateLeft(x_parent, &base->parent);
                    w = x_parent->right;
                }
                if ((w->left == NULL || 
                w->left->color == rbTreeNodeBlack) &&
                (w->right == NULL || 
                w->right->color == rbTreeNodeBlack)) {

                    w->color = rbTreeNodeRed;
                    x = x_parent;
                    x_parent = x_parent->parent;
                } else {
                    if (w->right == NULL || 
                    w->right->color == rbTreeNodeBlack) {

                        if (w->left) {
                            w->left->color = rbTreeNodeBlack;
                        }
                        w->color = rbTreeNodeRed;
                        rotateRight(w, &base->parent);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = rbTreeNodeBlack;
                    if (w->right) {
                        w->right->color = rbTreeNodeBlack;
                    }
                    rotateLeft(x_parent, &base->parent);
                    break;
                }
            }
            else {
                rbTreeNode *w = x_parent->left;
                if (w->color == rbTreeNodeRed) {
                    w->color = rbTreeNodeBlack;
                    x_parent->color = rbTreeNodeRed;
                    rotateRight(x_parent, &base->parent);
                    w = x_parent->left;
                }
                if ((w->right == NULL || 
                    w->right->color == rbTreeNodeBlack) &&
                    (w->left == NULL || 
                    w->left->color == rbTreeNodeBlack)) {

                    w->color = rbTreeNodeRed;
                    x = x_parent;
                    x_parent = x_parent->parent;
                }
                else {
                    if (w->left == NULL || 
                        w->left->color == rbTreeNodeBlack) {

                        if (w->right) {
                            w->right->color = rbTreeNodeBlack;
                        }
                        w->color = rbTreeNodeRed;
                        rotateLeft(w, &base->parent);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = rbTreeNodeBlack;
                    if (w->left) {
                        w->left->color = rbTreeNodeBlack;
                    }
                    rotateRight(x_parent, &base->parent);
                    break;
                }
            }
        }
        if (x) {
            x->color = rbTreeNodeBlack;
        }
    }
    return(y);
}

void rbTreeDeleteNode(rbTreeNode *base, rbTreeNode *foundNode,
                    rbTreeDisposeNodeCB disposeNode)
{
    disposeNode(rbTreeUnlinkNode(base, foundNode));
}

int rbTreeDelete(rbTreeNode *base, rbTreeNode *searchNode,
                    rbTreeCompareNodeCB compareRecords,
                    rbTreeDisposeNodeCB disposeNode)
{
    int foundNode = 0;
    rbTreeNode *z;

    z = rbTreeFind(base, searchNode, compareRecords);
    if (z != NULL) {
        rbTreeDeleteNode(base, z, disposeNode);
        foundNode = 1;
    }
    return(foundNode);
}

rbTreeNode *rbTreeNext(rbTreeNode *x)
{
    if (x->right != NULL) {
        x = x->right;
        while (x->left != NULL) {
            x = x->left;
        }
    }
    else {
        rbTreeNode *fromX;
        do {
            fromX = x;
            x = x->parent;
        } while (x && fromX == x->right);
    }
    return(x);
}

rbTreeNode *rbTreePrevious(rbTreeNode *x)
{
    if (x->left != NULL) {
        x = x->left;
        while (x->right != NULL) {
            x = x->right;
        }
    }
    else {
        rbTreeNode *fromX;
        do {
            fromX = x;
            x = x->parent;
        } while (x && fromX == x->left);
    }
    return(x);
}

int rbTreeSize(rbTreeNode *base)
{
    return(base->color);
}

rbTreeNode *rbTreeNew(rbTreeAllocateEmptyNodeCB allocateEmptyNode)
{
    rbTreeNode *rootStorage = allocateEmptyNode();
    if (rootStorage) {
        rootStorage->left = NULL;   /* leftmost node */
        rootStorage->right = NULL;  /* rightmost node */
        rootStorage->parent = NULL; /* root node */
        rootStorage->color = 0;     /* node count */
    }
    return(rootStorage);
}

void rbTreeDispose(rbTreeNode *base, rbTreeDisposeNodeCB disposeNode)
{
    rbTreeNode *iter = rbTreeBegin(base);
    while (iter != NULL) {
        rbTreeNode *nextIter = rbTreeNext(iter);

        if (iter->parent) {
            if (iter->parent->left == iter) {
                iter->parent->left = iter->right;
            }
            else {
                iter->parent->right = iter->right;
            }
        }
        if (iter->right != NULL) {
            iter->right->parent = iter->parent;
        }
        base->left = nextIter;
        if (base->right == iter) {
            base->right = NULL;
        }
        --(base->color);
        if (base->parent == iter) {
            base->parent = nextIter;
        }
        disposeNode(iter);

        iter = nextIter;
    }
    disposeNode(base);
}

#ifdef RBTREE_TEST_CODE
/* ================================================================== */

typedef struct TestNode {
    rbTreeNode      nodePointers; /* MUST BE FIRST MEMBER */
    char *str;
    char *key;
} TestNode;


static int rbTreeCompareNode_TestNode(rbTreeNode *left, rbTreeNode *right)
{
    return(strcmp(((TestNode *)left)->key, ((TestNode *)right)->key));
}

static rbTreeNode *rbTreeAllocateNode_TestNode(rbTreeNode *src)
{
    TestNode *newNode = malloc(sizeof(TestNode));
    if (newNode) {
        newNode->str = malloc(strlen(((TestNode *)src)->str) + 1);
        if (newNode->str) {
            strcpy(newNode->str, ((TestNode *)src)->str);
            
            newNode->key = malloc(strlen(((TestNode *)src)->key) + 1);
            if (newNode->key) {
                strcpy(newNode->key, ((TestNode *)src)->key);
            }
            else {
                free(newNode->str);
                newNode->str = NULL;

                free(newNode);
                newNode = NULL;
            }
        }
        else {
            free(newNode);
            newNode = NULL;
        }
    }
    return((rbTreeNode *)newNode);
}

rbTreeNode *rbTreeAllocateEmptyNodeCB_TestNode(void)
{
    TestNode *newNode = malloc(sizeof(TestNode));
    if (newNode) {
        newNode->str = NULL;
        newNode->key = NULL;
    }
    return((rbTreeNode *)newNode);
}

static void rbTreeDisposeNode_TestNode(rbTreeNode *src)
{
    if (src) {
        if (((TestNode *)src)->str) {
            free(((TestNode *)src)->str);
            ((TestNode *)src)->str = NULL;
        }
        if (((TestNode *)src)->key) {
            free(((TestNode *)src)->key);
            ((TestNode *)src)->key = NULL;
        }
        src->left = (void *)-1;
        src->right = (void *)-1;
        src->parent = (void *)-1;
        src->color = rbTreeNodeBlack;

        free(src);
    }
}

static int rbTreeCopyToNode_TestNode(rbTreeNode *dst, rbTreeNode *src)
{
    TestNode newValues;
    int copiedOK = 0;
    
    newValues.str = malloc(strlen(((TestNode *)src)->str) + 1);
    if (newValues.str) {
        strcpy(newValues.str, ((TestNode *)src)->str);
        
        newValues.key = malloc(strlen(((TestNode *)src)->key) + 1);
        if (newValues.key) {
            strcpy(newValues.key, ((TestNode *)src)->key);
            
            ((TestNode *)dst)->str = newValues.str;
            ((TestNode *)dst)->key = newValues.key;
            copiedOK = 1;
        }
        else {
            free(newValues.str);
            newValues.str = NULL;
        }
    }
    return(copiedOK);
}

static void DumpTree(rbTreeNode *base)
{
    rbTreeNode *newNode;
    
    newNode = rbTreeBegin(base);
    while (newNode != NULL) {
        rbTreeNode *nextNode = rbTreeNext(newNode);
        
        printf("[%s] = \"%s\"\n", ((TestNode *)newNode)->key, ((TestNode *)newNode)->str);
        printf("[%x] l[%x] r[%x] p[%x] <%s>\n", (int)newNode, (int)newNode->left, (int)newNode->right, (int)newNode->parent, ((newNode->color == rbTreeNodeBlack)?"Black":"Red"));
        
        newNode = nextNode;
    }
}

int main(int argc, char **argv)
{
    rbTreeNode *base, *newNode;
    TestNode searchNode;
    char tmpkey[20], tmpValue[40];
    int i;
    
    searchNode.key = tmpkey;
    searchNode.str = tmpValue;

    base = rbTreeNew(rbTreeAllocateEmptyNodeCB_TestNode);
    if (!base) {
        printf("Failed New!!!\n");
        exit(1);
    }
    for (i = 0; i < 100; ++i) {
        sprintf(tmpkey, "%d", i);
        sprintf(tmpValue, "<%d>", i * i);
        
        newNode = rbTreeInsert(base, (rbTreeNode *)&searchNode,
                            rbTreeCompareNode_TestNode,
                            rbTreeAllocateNode_TestNode,
                            rbTreeCopyToNode_TestNode);
        if (!newNode) {
            printf("Failed!!!\n");
            exit(1);
        }
    }
    
    newNode = rbTreeBegin(base);
    while (newNode != NULL) {
        rbTreeNode *nextNode = rbTreeNext(newNode);
        
        printf("[%s] = \"%s\"\n", ((TestNode *)newNode)->key, ((TestNode *)newNode)->str);
        
        if (strlen(((TestNode *)newNode)->str) < 7) {
            int didDelete;
            
            printf("Deleting [%s]\n", ((TestNode *)newNode)->key);
            didDelete = rbTreeDelete(base, newNode,
                    rbTreeCompareNode_TestNode, rbTreeDisposeNode_TestNode);
            printf("delete result = %d\n", didDelete);
        }

        newNode = nextNode;
    }

    printf("Tree Size = %d\n", rbTreeSize(base));
    printf("\n++++++++++++++++\n");
    DumpTree(base);
    printf("\n++++++++++++++++\n");

    rbTreeDispose(base, rbTreeDisposeNode_TestNode);

    printf("\nDone.\n");
    return(0);
}
#endif
