/* chan_tree.c implements the channel identifier tree
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"

RtgChanNode *CT_Root;
static char *menubuf;
#define FIRST_MENU_SIZE 128
static int menubufsize = 0;
static int menudrawn = 0;

static int next_word(char *out, char const **inpp) {
  const char *in;

  in = *inpp;
  if (*in == '/') in++;
  if (*in == '\0') return 0;
  while (*in == '/') *out++ = *in++;
  while (*in != '\0' && *in != '/') *out++ = *in++;
  *out = '\0';
  *inpp = in;
  return 1;
}

/*
   act == 0 find
   act == 1 insert
   act == 2 delete
*/
static RtgChanNode *CT_recurse(const char *name, int act,
								RtgChanNode **CNP) {
  RtgChanNode *CN, **CNPsave, *newCN;
  char word[80];
  const char *rest;
  int cmp;

  CN = *CNP;
  rest = name;
  next_word(word, &rest);
  CNPsave = CNP;
  for ( ; CN != 0; CNP = &CN->u.node.sibling, CN = *CNP) {
	cmp = strcmp(word, CN->word == 0 ? "" : CN->word);
	if (cmp <= 0) break;
  }
  if (cmp != 0) {
	if (act == 1) { /* need to insert a node here */
	  newCN = new_memory(sizeof(RtgChanNode));
	  if (word[0] == '\0') newCN->word = NULL;
	  else newCN->word = nl_strdup(word);
	  newCN->u.node.child = NULL;
	  newCN->u.node.sibling = CN;
	  *CNP = newCN;
	  CN = newCN;
	  menudrawn = 0;
	} else if (*CNPsave != 0 && (*CNPsave)->u.node.sibling == 0) {
	  CNP = CNPsave;
	  CN = *CNP;
	  rest = name;
	} else return NULL;
  }
  if (act == 2) {
	if (CN->word != 0)
	  CT_recurse(rest, act, &CN->u.node.child);
	if (CN->word == 0 || CN->u.node.child == 0) {
	  *CNP = CN->u.node.sibling;
	  dastring_update(&CN->word, NULL);
	  free_memory(CN);
	  menudrawn = 0;
	}
	return NULL;
  } else if (CN->word == 0) return CN;
  else return CT_recurse(rest, act, &CN->u.node.child);
}

RtgChanNode *ChanTree_find(const char *name) {
  return CT_recurse(name, 0, &CT_Root);
}

RtgChanNode *ChanTree_insert(const char *name) {
  return CT_recurse(name, 1, &CT_Root);
}  

void ChanTree_delete(const char *name) {
  CT_recurse(name, 2, &CT_Root);
}

static int draw_menu_text(const char *text, int index) {
  for (;;) {
	if (index == menubufsize) {
	  if (menubufsize == 0)
		menubufsize = FIRST_MENU_SIZE;
	  else menubufsize *= 2;
	  menubuf = realloc(menubuf, menubufsize);
	  if (menubuf == 0)
		nl_error(4, "Out of memory in Draw_channel_menu");
	}
	if (*text == '\0') break;
	menubuf[index++] = *text++;
  }
  menubuf[index] = '\0';
  return index;
}

/* draw_menu_recurse
   operate on the assumption that we definitely want to output
   for this node and all of its siblings. As each node is
   processed, therefore, must chase down it's children to
   see if additional nesting is required.
*/
static int draw_menu_recurse(RtgChanNode *CN, int index) {
  RtgChanNode *CNC;

  for (; CN != 0; CN = CN->u.node.sibling) {
	assert(CN->word != 0);
	assert(CN->u.node.child != 0);
	if (CN->word[0] == '/') {
	  index = draw_menu_text("/", index);
	  index = draw_menu_text(CN->word, index);
	  index = draw_menu_text("|", index);
	}
	index = draw_menu_text(CN->word, index);
	for (CNC = CN->u.node.child; ; CNC = CNC->u.node.child) {
	  assert(CNC != 0);
	  if (CNC->word == 0 || CNC->u.node.sibling != 0)
		break;
	}
	if (CNC->word != 0) {
	  index = draw_menu_text("^R@(", index);
	  index = draw_menu_recurse(CNC, index);
	  index = draw_menu_text(")", index);
	}
	if (CN->u.node.sibling != 0)
	  index = draw_menu_text(";", index);
  }
  return index;
}

void Draw_channel_menu( const char *label, const char *title ) {
  if (menudrawn == 0) {
	draw_menu_recurse(CT_Root, 0);
  }
  Menu( label, title, 1, menubuf, "O");
}
