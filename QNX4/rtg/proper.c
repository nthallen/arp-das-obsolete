/* proper.c */
#include <windows/Qwindows.h>
#include <assert.h>
#include <string.h>
#include "nortlib.h"
#include "rtg.h"

extern RtgPropDefA winpropdef;

static RtgPropDefB PropDefBs[] = {
  &winpropdef, 0, NULL, NULL, 0, 0,
  NULL, 0, NULL, NULL, 0, 0
};

/* Standard Property Dialog buttons: */
static char *bars[4] = { NULL, "Apply|Y;Reset|CANCEL", NULL, NULL };

static RtgPropDefB *label2PDB(const char *prop_label) {
  int i;
  RtgPropDefB *PDB;
  
  assert(prop_label != 0);
  for (i = 0; ; i++) {
	PDB = &PropDefBs[i];
	if (PDB->def == 0) {
	  nl_error(2, "Unknown property label: %s", prop_label);
	  return NULL;
	}
	if (strcmp(PDB->def->di_label+1, prop_label) == 0)
	  return PDB;
  }
}

/* find_props() locates the property structure for the named object
   using the Property definition
*/
static void *find_props(const char *name, RtgPropDefB *PDB) {
  RtgPropDefA *pd;
  void *prop_ptr;
  
  assert(PDB != 0 && PDB->def != 0);
  assert(name != 0);
  pd = PDB->def;
  if (pd->find_prop != 0)
	prop_ptr = pd->find_prop(name, PDB);
  else {
	RtgChanNode *CN;
	CN = ChanTree(CT_FIND, pd->tree, name);
	if (CN != 0)
	  prop_ptr = CN->u.leaf.voidptr;
	else prop_ptr = NULL;
  }
  if (prop_ptr == 0)
	nl_error(2, "No properties for \"%s\" of type \"%s\"",
					name, PDB->def->di_label);
  return prop_ptr;
}

/* Initializes the newvals array and n_elements */
static void init_newvals(RtgPropDefB *PDB) {
  RtgPropEltDef *pe;
  int i;
  
  pe = PDB->def->elements;
  assert(pe != 0);
  for (i = 0; pe[i].tag != 0; i++);
  assert(i > 0);
  PDB->newvals = new_memory(sizeof(RtgPropValDef) * i);
  memset(PDB->newvals, 0, sizeof(RtgPropValDef) * i);
  PDB->n_elements = i;
  PDB->last_element = 0;
}

static int tag2eltno(RtgPropDefB *PDB, const char *tag) {
  RtgPropEltDef *pe;
  int i;

  pe = PDB->def->elements;
  for (i = 0; i < PDB->n_elements; i++) {
	assert(pe[i].tag != 0);
	if (strcmp(tag, pe[i].tag) == 0) break;
  }
  if (i >= PDB->n_elements) {
	nl_error(2, "Unknown element tag %s in tag2eltno", tag);
	i = -1;
  }
  return i;
}

static void element2newval(const char *tag, RtgPropDefB *PDB) {
  int i;
  
  i = tag2eltno(PDB, tag);
  if (i >= 0)
	PDB->def->elements[i].type->elt2val(&PDB->newvals[i].val);
}

/* Copies properties from actual structure to newvals */
static void elements_assign(RtgPropDefB *PDB, int props2newvals) {
  RtgPropEltDef *pe, *edef;
  void *pptr;
  int i;
  
  pe = PDB->def->elements;
  for (i = 0; i < PDB->n_elements; i++) {
	edef = &pe[i];
	pptr = ((char *)PDB->prop_ptr) + edef->offset;
	if (props2newvals)
	  edef->type->assign( &PDB->newvals[i].val, (RtgPropValue *)pptr );
	else
	  edef->type->assign( (RtgPropValue *)pptr, &PDB->newvals[i].val );
	PDB->newvals[i].changed = 0;
  }
}

/* Updates picture based on newvals */
static void newvals2pict(RtgPropDefB *PDB) {
  RtgPropEltDef *pe, *edef;
  int i;
  
  pe = PDB->def->elements;
  for (i = 0; i < PDB->n_elements; i++) {
	edef = &pe[i];
	edef->type->val2pict( edef->tag, &PDB->newvals[i].val );
  }
}

static int compare_element(RtgPropDefB *PDB, int elt_no) {
  RtgPropEltDef *pe;
  
  pe = &PDB->def->elements[elt_no];
  return PDB->newvals[elt_no].changed =
	pe->type->compare( (RtgPropValue *)((char *)PDB->prop_ptr + pe->offset), 
		  &PDB->newvals[elt_no].val) != 0;
}

/* Checks element i for sanity. Returns 1 if insane, 0 otherwise */
static int sanity_check(RtgPropDefB *PDB, int elt_no) {
  RtgPropEltDef *pe;
  
  pe = &PDB->def->elements[elt_no];
  return pe->type->check != 0 && pe->type->check( PDB,
		  (RtgPropValue *)((char *)PDB->prop_ptr + pe->offset), 
		  &PDB->newvals[elt_no].val );
}

/* Recover from sanity_check changes if necessary for specified element */
static void sanity_recover(RtgPropDefB *PDB, int elt_no) {
  RtgPropEltDef *pe;
  
  if (PDB->newvals[elt_no].changed) {
	pe = &PDB->def->elements[elt_no];
	if (pe->type->recover != 0)
	  pe->type->recover( PDB,
			(RtgPropValue *)((char *)PDB->prop_ptr + pe->offset),
			&PDB->newvals[elt_no].val );
  }
}

#define ELTBUF_SIZE 4096

static int prop_handler(QW_EVENT_MSG *msg, char *label) {
  RtgPropDefB *PDB;
  RtgPropDefA *pd;
  
  if (msg->hdr.action == QW_DISMISS) {
	/* on QW_DISMISS, the label is placed into the key field */
    PDB = label2PDB(msg->hdr.key+1);
	if (PDB != 0)
	  PropCancel_(NULL, msg->hdr.key+1);
	else
	  EventNotice("Prop Handler Dismiss", msg);
	return 1;
  }
  PDB = label2PDB(label+1);
  if (PDB == 0) {
	nl_error(2, "Unknown label %s in prop_handler", label);
	return 0;
  }
  pd = PDB->def;
  if (msg->hdr.key[0] == 'Y') { /* Apply */
	void *eltbuf, *elt;

	eltbuf = new_memory(ELTBUF_SIZE);
	PictureCurrent(PDB->pict_id);
	CopyElements("AP*", eltbuf, ELTBUF_SIZE, NULL);
	for (elt = ElementFirst(eltbuf); elt != 0; elt = ElementNext()) {
	  char *tag;
	
	  tag = ElementTag();
	  assert(tag[0] == 'A' && tag[1] == 'P');
	  element2newval(tag, PDB);
	}
	free_memory(eltbuf);
	if (PropsApply_(label+1))
	  DialogCancel(label, NULL);
  } else if (pd->handler == 0 || !pd->handler(msg, PDB)) {
	switch (msg->hdr.action) {
	  case QW_CLOSED:
	  case QW_CANCELLED:
		/* If dialog is still active, should restore old
		   property values here */
	    if (DialogCurrent(pd->di_label)) {
		  PictureCurrent(PDB->pict_id);
		  elements_assign(PDB, 1);
		  newvals2pict(PDB);
		}
		break;
	  default:
		EventNotice("PropHandler", msg);
	}
  }
  return 1;
}

void Properties_(const char *name, const char *plabel, int open_dialog) {
  RtgPropDefB *PDB;
  RtgPropDefA *pd;

  PDB = label2PDB(plabel);
  if (PDB == 0) return;
  pd = PDB->def;

  if (!open_dialog) {
	/* If dialog is open, close it */
	PropCancel_(name, plabel);
  }

  /* locate the properties structure */
  PDB->prop_ptr = find_props(name, PDB);
  if (PDB->prop_ptr == 0)
	return;

  /* Initialize newvals if necessary */
  if (PDB->newvals == 0)
	init_newvals(PDB);

  /* Copy properties to newvals */
  elements_assign(PDB, 1);
  
  if (open_dialog) {
	/* Load the picture if necessary */
	if (PDB->pict_id == 0) {
	  PDB->pict_id = Picture(pd->pict_name+1, pd->pict_file);
	  if (PDB->pict_id == 0) {
		nl_error(2, "Unable to open dialog picture %s", pd->pict_file);
		return;
	  }
	} else PictureCurrent(PDB->pict_id);

	/* update picture */
	newvals2pict(PDB);

	/* Call type-specific routine to customize the picture */
	if (pd->dial_update != 0)
	  pd->dial_update(PDB);

	if (DialogCurrent(pd->di_label) != YES)
	  Dialog(pd->di_label, pd->di_title, NULL, bars, NULL, "cp-b");

	{ static int handler_set = 0;
  
	  if (!handler_set) {
		set_key_handler('r', prop_handler);
		handler_set = 1;
	  }
	}
  }
}

/* cancel the associated dialog
   Depending on the dialog type, the name may be ignored.
   If name == NULL, cancel no matter what.
*/
void PropCancel_(const char *name, const char *plabel) {
  RtgPropDefB *PDB;
  RtgPropDefA *pd;
  void *prop_ptr;
  
  PDB = label2PDB(plabel);
  if (PDB == 0) return;
  pd = PDB->def;
  if (name != 0) {
	prop_ptr = find_props(name, PDB);
	if (prop_ptr == 0 || prop_ptr != PDB->prop_ptr)
	  return;
  }
  if (pd->cancel != 0 && pd->cancel(PDB) == 0)
	return;
  if (PDB->pict_id != 0 && DialogCurrent(pd->di_label))
	DialogCancel(pd->di_label, "P");
  PDB->prop_ptr = NULL;
}

/* Switch to the new object only if the dialog is active */
void PropUpdate_(const char *name, const char *plabel) {
  RtgPropDefB *PDB;

  PDB = label2PDB(plabel);
  if (PDB == 0) return;
  if (DialogCurrent(plabel))
	Properties_(name, plabel, 1);
}

/* Changes the specified value... Assumes dialog is not open */
void PropChange_(const char *plabel, const char *tag, const char *value) {
  RtgPropDefB *PDB;
  int i;

  PDB = label2PDB(plabel);
  if (PDB == 0) return;
  i = tag2eltno(PDB, tag);
  if (i >= 0)
	PDB->def->elements[i].type->ascii2val( &PDB->newvals[i].val, value);
}

/*	Applies the new properties as if the Apply button
	had been selected in the GUI. Returns 0 if there
	is any error, 1 otherwise.
*/
int PropsApply_(const char *prop_label) {
  RtgPropDefB *PDB;
  RtgPropDefA *pd;
  int i;

  PDB = label2PDB(prop_label);
  if (PDB == 0) return 0;
  pd = PDB->def;

  /* Perform sanity checks (if any) on each element */
  for (i = 0; i < PDB->n_elements; i++) {
	if (compare_element(PDB, i) && sanity_check(PDB, i))
	  break;
  }
  
  /* Invoke custom sanity check (apply) if any */
  if (i < PDB->n_elements || (pd->apply != 0 && pd->apply(PDB) == 0)) {
	/* Perform sanity recovery if any check failed */
	while (--i >= 0)
	  sanity_recover(PDB, i);
	return 0;
  }

  /* Assign all newvals to current settings */
  elements_assign(PDB, 0);
  
  return 1;
}

void PropsOutput_(FILE *fp, const char *name, const char *plabel) {
  nl_error(1, "PropsOutput_ not ready yet!");
  fp = fp; name = name; plabel = plabel;
}

/*find_prop: optional function to locate the property structure
	     when it can't be found by a basic ChanTree(CT_FIND).
		 This is true for graph axis properties
  dial_update: modifies the current picture as might be
		 necessary. Property Elements have already been updated.
		 May assume the picture is current. NOT Invoked if
		 open_dialog == 0
		 Returns 0 on success, 1 on failure. May call nl_error.
		 (At the moment, I don't imagine failures here)
  handler: Auxilliary handler routine. QW_DISMISS and key 'Y'
		 are handled first (so not passed to handler)
		 Returns 1 if the message is handled, 0 otherwise.
		 QW_CLOSED and QW_CANCELLED are handled afterward
		 if handler doesn't. May be NULL.
  apply: calling this function indicates that all the elements
		 have been processed and the new values are ready
		 to be copied to the actual structure. A non-zero
		 result indicates success. Zero indicates an error.
		 nl_error can be used to report the nature of the
		 error. The actual copying of values is performed
		 after this.
  cancel: Called when the dialog is to be cancelled, allowing
		 other actions to be taken (such as cancelling nested
		 dialogs). May return 0 if the dialog should not
		 be closed for some reason (though I can't imagine
		 why not at the moment). The general processing
		 prior to this will deal with the case where the
		 specific invocation doesn't match the PropClose()
		 request.
		 The dialog will be closed by the subsequent code.

  In element definitions:
	Tag is the tag of picture element {
	  Begins with "AP"
	  Followed by 1-letter type code as listed below
	  Followed by letters to make tag unique
	  Type of element and property {
		Sa: dastring
		Sb: dastring/treename
		D: double
		i: short int
		u: unsigned short int
		B: state (aka boolean) (unsigned char)
	   Notes:
		strings have trailing spaces elminated
		treenames are strings which are keys in a ChanTree structure {
			assumes treetype listed with proptype
			handled specially before sanity check on apply
			error if name cannot be changed
		  }
	  }
	}
	Offset within prop structure of the original
	(Index into new values is implicit based on index here)
	Last element in the list should have a NULL tag

  A complete minimal definition of a property dialog consists of
  A dialog picture created in iEdit
  Definition of an array of elements (RtgPropEltDef)
  Definition of the dialog (RtgPropDefA)
*/
