struct fldinfo {
	unsigned char num;	/* field number */
	char *txt;		/* string */
	unsigned char line;	/* line */
	unsigned char col;	/* column */
	unsigned char width;	/* width of field */
	unsigned char length;	/* length of field */
	int attrcode;		/* attribute code */
	struct fldinfo *fldnext;
};

#define FLDINFOSZ (sizeof(struct fldinfo))

extern int fld_win_out(char * filename,WINDOW *codescr,struct fldinfo *top, char **attrtypes, int numattr, int lines, int cols, int posy, int posx);
extern int fld_win_in(char *filename, WINDOW *win, struct fldinfo **top, int *numflds, char **attrtypes, int posy, int posx, struct hdr *fhdr);
extern void fld_free(struct fldinfo *);
extern int fld_num(struct fldinfo *,int, int, int, int, struct fldinfo **);
extern int fld_del(struct fldinfo **,int);
extern int fld_add(struct fldinfo **f,int num,char *txt,int line, int col, int width, int length, int attrcode);
