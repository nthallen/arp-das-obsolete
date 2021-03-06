extern int getconfirm(char change);
extern int chg_attr(char *filename, char *attr);
extern void error(char *msg, int code, int fatal);
extern int isondfs(char *filename);
extern do_copy(char *s1, char *s2, char *s3, char *s4, int ask);
extern int do_delete(char *filename, int ask);
extern int validate(char *filename);
extern char *getsysname(void);
extern char *getlocation(char *name, char *full_name);
extern void getask(void);
extern void getpath( char *path);
extern long int gettime(char *filename);
extern char *getdate(long tim, char *buf);
extern long int getsize(char *filename);
extern short int getwrite(char *filename);
extern int matchpat(char *pat, char *word, char end);
extern int curproj(void);
extern unsigned short int getcrc(char *filename);
extern time_t t2dos(time_t time);
extern time_t t2qnx(time_t time);
int getfloppylist(char fnames[]);
