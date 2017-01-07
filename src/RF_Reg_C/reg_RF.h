/**************************************************************
 * mex interface to Andy Liaw et al.'s C code (used in R package randomForest)
 * Added by Abhishek Jaiantilal ( abhishek.jaiantilal@colorado.edu )
 * License: GPLv2
 * Version: 0.02
 *
 * Supporting file that has some declarations.
 *************************************************************/

#define uint32 unsigned long
#define SMALL_INT char
#define DType double

#ifdef MATLAB
#define SMALL_INT_CLASS mxCHAR_CLASS //will be used to allocate memory t
#endif

#define PRINTF printf

void seedMT(uint32 seed);
uint32 randomMT(void);

void regForest(DType *x, DType *ypred, int *mdim, int *n,
			   int *ntree, int *lDaughter, int *rDaughter,
			   SMALL_INT *nodestatus, int *nrnodes, DType *xsplit,
  DType *avnodes, int *mbest, int *treeSize, int *cat,
			   int maxcat, int *keepPred, DType *allpred, int doProx,
  DType *proxMat, int *nodes, int *nodex) ;