#define ESC 27
#define CR 10
#define MAXMENUS 20


struct mdesc {
	WINDOW *m;		    /* the pad */
	short flag;		    /* slot being used? */
	int xtra;		    /* extra space for items */
	int *codes;		    /* return codes */
	char *title;		    /* menu title */
	char **items;		    /* menu items */
	unsigned char  normattr;    /* normal attribute */
	unsigned char  hiattr;	    /* highlight attribute */
	unsigned char  titattr;	    /* menu title attribute */
	int *selchrs;		    /* selection characters */
	int selnum;		    /* number of selection characters */
	int *escchrs;		    /* escape characters */
	int escnum;		    /* number of esc characters */
	int lines;		    /* number of items */
	int cols;		    /* width of items */
	int last;		    /* last selection */
	int vert;		    /* horiizontal or vertical? */
	int bx;			    /* 0=no box, 1=single line box, 2=double line box */
};

extern struct mdesc mtbl[MAXMENUS];


/*
Initialise a Menu.
*/

extern int menu_init(char *,char *,unsigned char,unsigned char,unsigned char,
	int *,int,int *,int,int,int);

/*
Display Menu.
*/

extern int menu(int,int,int,char *,int *,int *,int);

/*
Dynamically add an item to an existing menu.
*/

extern int menu_add_item(int,char *,int,int);

/*
Dynamically delete a menu item from an existing menu.
*/

extern int menu_del_item(int,char *,int);


/*
Destroy a menu or all menus.
*/

extern int menu_end(int);


/*
This sets the default choice for a menu.
*/

extern int menu_set_default(int,char *,int);

/*
Resets the attributes for a menu.
*/

extern int menu_set_attrs(int md, unsigned char *menu_norm, 
	unsigned char *menu_hi, unsigned char *menu_title);

/*
Resets a code return for an item.
*/

extern int menu_set_code(int md, char *item, int code);

/*
Gets last selection code.
*/

#define menu_get_last(M) (mtbl[M].last)

/*
Get the number of menu items.
*/

#define menu_get_num_items(M) (mtbl[M].items)

/*
Get the width of an item.
*/

#define menu_get_width_items(M) ( mtbl[M].vert ? mtbl[M].cols : mtbl[M].cols+1)

/*
Get the width of menu items.
*/

int menu_get_width(int menu);

/*
Get the total width of a menu.
*/

/*
Get the total length of a menu.
*/

int menu_get_length(int menu);
