/* chan_tree.c implements the channel identifier tree
 * $Log$
 * Revision 1.3  1995/01/27  20:34:36  nort
 * *** empty log message ***
 *
 * Revision 1.2  1994/12/19  16:41:00  nort
 * *** empty log message ***
 *
 * Revision 1.1  1994/12/07  16:32:56  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"

#define FIRST_MENU_SIZE 128
typedef struct {
  RtgChanNode *CT_Root;
  char *menubuf;
  int menubufsize;
  unsigned char menudrawn:1;
} treedef;

static treedef tree_defs[CT_N_TREES];

static void next_word(char *out, char const **inpp) {
  const char *in;

  in = *inpp;
  if (*in == '/') in++;
  if (*in != '\0') {
	while (*in == '/') *out++ = *in++;
	while (*in != '\0' && *in != '/') *out++ = *in++;
  }
  *out = '\0';
  *inpp = in;
}

/*
   act == 0 find
   act == 1 insert
   act == 2 delete
*/
static RtgChanNode *CT_recurse(treedef *tree, const char *name, int act,
								RtgChanNode **CNP) {
  RtgChanNode *CN, **CNPsave, *newCN;
  char word[80];
  const char *rest;
  int cmp;

  CN = *CNP;
  rest = name;
  next_word(word, &rest);
  CNPsave = CNP;
  for (cmp = -1; CN != 0; CNP = &CN->u.node.sibling, CN = *CNP) {
	cmp = strcmp(word, CN->word == 0 ? "" : CN->word);
	if (cmp <= 0) break;
  }
  if (cmp != 0) {
	if (act == 1) { /* need to insert a node here */
	  newCN = new_memory(sizeof(RtgChanNode));
	  newCN->word = dastring_init( word );
	  /*  if (word[0] == '\0') newCN->word = NULL;
		  else newCN->word = nl_strdup(word); */
	  newCN->u.node.child = NULL;
	  newCN->u.node.sibling = CN;
	  *CNP = newCN;
	  CN = newCN;
	  tree->menudrawn = 0;
	} else if (*CNPsave != 0 && (*CNPsave)->u.node.sibling == 0) {
	  CNP = CNPsave;
	  CN = *CNP;
	  rest = name;
	} else return NULL;
  } else if (act == 1 && CN->word == 0)
	return NULL;
  if (act == 2) {
	if (CN->word != 0)
	  CT_recurse(tree, rest, act, &CN->u.node.child);
	if (CN->word == 0 || CN->u.node.child == 0) {
	  *CNP = CN->u.node.sibling;
	  dastring_update(&CN->word, NULL);
	  free_memory(CN);
	  tree->menudrawn = 0;
	}
	return NULL;
  } else if (CN->word == 0) return CN;
  else return CT_recurse(tree, rest, act, &CN->u.node.child);
}

static int draw_menu_text(treedef *tree, const char *text, int index) {
  for (;;) {
	if (index == tree->menubufsize) {
	  if (tree->menubufsize == 0)
		tree->menubufsize = FIRST_MENU_SIZE;
	  else tree->menubufsize *= 2;
	  tree->menubuf = realloc(tree->menubuf, tree->menubufsize);
	  if (tree->menubuf == 0)
		nl_error(4, "Out of memory in Draw_channel_menu");
	}
	if (*text == '\0') break;
	tree->menubuf[index++] = *text++;
  }
  tree->menubuf[index] = '\0';
  return index;
}

/* draw_menu_recurse
   operate on the assumption that we definitely want to output
   for this node and all of its siblings. As each node is
   processed, therefore, must chase down it's children to
   see if additional nesting is required.
*/
static int draw_menu_recurse(treedef *tree, RtgChanNode *CN, int index) {
  RtgChanNode *CNC;

  for (; CN != 0; CN = CN->u.node.sibling) {
	assert(CN->word != 0);
	assert(CN->u.node.child != 0);
	if (CN->word[0] == '/') {
	  index = draw_menu_text(tree, "/", index);
	  index = draw_menu_text(tree, CN->word, index);
	  index = draw_menu_text(tree, "|", index);
	}
	index = draw_menu_text(tree, CN->word, index);
	for (CNC = CN->u.node.child; ; CNC = CNC->u.node.child) {
	  assert(CNC != 0);
	  if (CNC->word == 0 || CNC->u.node.sibling != 0)
		break;
	}
	if (CNC->word != 0) {
	  index = draw_menu_text(tree, "^R@(", index);
	  index = draw_menu_recurse(tree, CNC, index);
	  index = draw_menu_text(tree, ")", index);
	}
	if (CN->u.node.sibling != 0)
	  index = draw_menu_text(tree, ";", index);
  }
  return index;
}

RtgChanNode *ChanTree(int act, treetype tree, const char *name) {
  assert(act >= 0 && act <= 2);
  assert(tree >= 0 && tree < CT_N_TREES);
  return CT_recurse(&tree_defs[tree], name, act, &tree_defs[tree].CT_Root);
}

int ChanTree_defined(treetype tree) {
  assert(tree >= 0 && tree < CT_N_TREES);
  return tree_defs[tree].CT_Root != 0;
}

void Draw_ChanTree_Menu(treetype tree, const char *label, const char *title) {
  assert(tree >= 0 && tree < CT_N_TREES);
  if (tree_defs[tree].menudrawn == 0) {
	draw_menu_recurse(&tree_defs[tree], tree_defs[tree].CT_Root, 0);
  }
  Menu( label, title, 1, tree_defs[tree].menubuf, "O");
}

/* Returns non-zero on success */
int ChanTree_Rename(treetype tree, const char *oldname, const char *newname) {
  RtgChanNode *CNP, CN;
  
  CNP = ChanTree(CT_FIND, tree, oldname);
  if (CNP == 0) return 0; /* unable to find old name */
  assert(CNP->word == 0);
  CN = *CNP;
  ChanTree(CT_DELETE, tree, oldname);
  CNP = ChanTree(CT_INSERT, tree, newname);
  if (CNP == 0) {
	CNP = ChanTree(CT_INSERT, tree, oldname);
	assert(CNP != 0 && CNP->word == 0);
	*CNP = CN;
	return 0;
  }
  assert(CNP->word == 0);
  *CNP = CN;
  return 1;
}

/* Inserts a new node using the specified format. The approach
   here is brute force, but it could be optimized later if
   that was important. I don't think it will matter, since we
   will usually be dealing with only a handful of entries.
   Returns the resulting name, rather than the node, since the
   node can be derived from the name later.
*/
char *ChanTreeWild(treetype tree, const char *format) {
  RtgChanNode *CN;
  int name_no = 0;
  static char name[20];
	
  do {
	sprintf(name, format, ++name_no);
	CN = ChanTree(CT_INSERT, tree, name);
  } while (CN == 0);
  CN->u.leaf.voidptr = NULL;
  return name;
}

static CTVisitRec( RtgChanNode *CN, void (* func)( RtgChanNode *CN) ) {
  for ( ; CN != 0; CN = CN->u.node.sibling ) {
	if ( CN->word == 0 )
	  func( CN );
	else
	  CTVisitRec( CN->u.node.child, func );
  }
}

/* Visit all the nodes of the specified tree, and call func on all the 
  leaves */
void CTreeVisit( treetype tree, void (* func)( RtgChanNode *CN) ) {
  assert(tree >= 0 && tree < CT_N_TREES);
  CTVisitRec( tree_defs[tree].CT_Root, func );
}
