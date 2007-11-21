/*
datasrcs.c: builds, manipulates, outputs the data structure that holds the
database in memory.
*/

#include "inclsrcs.h"
#include "filesrcs.h"
#include "globsrcs.h"

/* code insertions */
#define SCROLL(z) if ((++z)>NUMLINES) \
{ puts(" ---------------Press <Return> for More--------------"); \
fflush(stdout); gets(0); z=0; }
#define UP2DATE(s,c,ptr) (s==ptr->sizest && c==ptr->crcest)

char *badfiles[]={"BAK","MAP","LNK","RCS","OBJ","EXE","DBG"};
#define NUM_BADFILES (sizeof(badfiles))

/* data definitions */
/* The data structure is a hash table of pointers to lists of filenodes */
/* The filenodes have pointers to lists of sysnodes. The sysnodes have  */
/* pointers to lists of locnodes.                                       */

struct filenode {
    char name[FNSIZE];
    long int latest, sizest;   /* info on the most up to date version of file */
    unsigned short int crcest; /* time,size and crc */
    short int numwrite;        /* number of writeable copies */
    short int numcopy;         /* number of up to date copies */
    struct locnode *ulptr;     /* location of most up to date copy on hard disk */
    struct sysnode *usptr;     /* system of most up to date copy */
    struct sysnode *sptr;      /* pointer to system list */
    struct filenode *next;     /* pointer to next filenode */
};

struct sysnode {
    char sysid[FNSIZE];        /* system identifier */
    struct sysnode *next;      /* pointer to next sysnode */
    struct locnode *lptr;      /* pointer to location list */
};

struct locnode {
    long int time;             /* time, size and crc of file at this location */
    long int size;
    unsigned short int crc;
    short int writ;            /* if file at this location is attribute -r or not */
    char where[FMSIZE];        /* full pathname */
    struct locnode *next;      /* pointer to next locnode */
};

/* traversal pointers */
struct filenode *ftptr; struct sysnode *stptr; struct locnode *ltptr;

/* hash definitions */
#define HASHSIZE 50
static struct filenode *htable[HASHSIZE];

int hashfctn(char *filename) {
/* returns a files hash calculation */
    int sum=0;
    for (;*filename!=NULS;sum+=*filename++);
    sum %= HASHSIZE;
    return(sum);
}

struct locnode *chg_locnode(struct locnode *lptr,
struct sysnode *sptr, struct filenode *fptr, char *place,
long time, long sze, unsigned short crc, short wr) {
/* if a negative value, keeps the old value, except crc. */
dirty++;
if (time<0) time=lptr->time;
if (sze<0) sze=lptr->size;
/* if (crc<0) crc=lptr->crc; crc is unsigned! */
if (wr<0) wr=lptr->writ;
if (fptr) {
	if (lptr->size != sze || lptr->crc !=crc)
		if (UP2DATE(sze,crc,fptr))
			fptr->numcopy++;
        if (time>fptr->latest) {
        	fptr->latest=time;
        	fptr->crcest=crc; fptr->sizest=sze;
        	fptr->usptr=sptr; fptr->ulptr=lptr;
		fptr->numcopy=1;
        }
	if (lptr->writ>wr) fptr->numwrite--;
	else if (lptr->writ<wr) fptr->numwrite++;
}
lptr->time=time; lptr->size=sze; lptr->crc=crc; lptr->writ=wr; 
if (place!=0 && place[0]!=NULS) strcpy(lptr->where,place);
return(lptr);
}

struct locnode *add_locnode(struct locnode *lptr,
struct sysnode *sptr, struct filenode *fptr, char *place,
long time, long sze, unsigned short crc, short wr) {
struct locnode *ptr;
/* returns a pointer to a location node that is added to data structure */
dirty++;
if (lptr)
   ptr=lptr->next=(struct locnode *)malloc(sizeof(struct locnode));
else
   ptr=sptr->lptr=(struct locnode *)malloc(sizeof(struct locnode));
ptr->next=NULL;
ptr->writ=0; ptr->size=0; ptr->crc=0;
chg_locnode(ptr,sptr,fptr,place,time,sze,crc,wr);
return(ptr);
}

struct sysnode *add_sysnode(struct sysnode *sptr, struct filenode *fptr, char *sysid) {
/* returns pointer to system node that is added to data structure */
    struct sysnode *ptr;
    dirty++;
    if (sptr)
       ptr=sptr->next=(struct sysnode *)malloc(sizeof(struct sysnode));
    else
       ptr=fptr->sptr=(struct sysnode *)malloc(sizeof(struct sysnode));
    ptr->next=NULL;
    strcpy(ptr->sysid,strlwr(sysid));
    ptr->lptr=NULL;
    return(ptr);
}

struct filenode *add_filenode(short flag, struct filenode *fptr, 
char *node, long time, long sze, unsigned short crc) {
/* returns pointer to a file node location that is added to data structure */	
    struct filenode *ptr;
    dirty++;
    if (flag) {
       if (fptr)
          ptr=fptr->next=(struct filenode *)malloc(sizeof(struct filenode));
       else
          ptr=htable[hashfctn(node)]=(struct filenode *)malloc(sizeof(struct filenode));
       strcpy(ptr->name,node);
       ptr->numwrite=0; ptr->numcopy=0;
       ptr->next=NULL; ptr->sptr=NULL;
       ptr->usptr=NULL; ptr->ulptr=NULL;
     }
    else ptr=fptr;
    ptr->crcest=crc; ptr->latest=time;
    ptr->sizest=sze; return(ptr);
}

int searchfile(char *node, struct filenode **fileptr) {
/* searches for the node in the hashtable. Zero if not found. */
/* if found, returns pointer to node in fileptr */
/* if not found, returns pointer to aid in insertion or NULL if the */
/* list doesnt exist */
    struct filenode *cursor=NULL; int hash;
    if (!htable[hash=hashfctn(node)]) {
        *fileptr=NULL; return(0);
    }
    else
        do {
           if (!cursor) cursor=htable[hash];
           else cursor=cursor->next;
           *fileptr=cursor;
           if (!strcmp(cursor->name,node)) 
           return(1);
           }
        while (cursor->next);
    return(0);
 }

int searchsys(char *sys, struct sysnode *top, struct sysnode **sysptr) {
/* searches for system node given a pointer to the top of a system list
   if found, returns pointer to node in sysptr
   if not found, returns pointer to aid in insertion or NULL if the
   list doesnt exist, and returns zero. */
   
   struct sysnode *cursor=NULL;
   if (!top) {
       *sysptr=NULL; return(0);
   }
   else
      do {
               if (!cursor) cursor=top;
               else cursor=cursor->next;
               *sysptr=cursor;
               if (!stricmp(sys,cursor->sysid))
                  return(1);
      }
      while (cursor->next);
return(0);
}

int searchloc(char *location,struct locnode *top, struct locnode **locptr) {
/* searches for location node given a pointer to the top of a loc list */
/* if found, returns pointer to node in locptr */
/* if not found, returns pointer to aid in insertion or NULL if the */
/* list doesnt exist, and returns zero. */
    struct locnode *cursor=NULL;
    if (!top) {
        *locptr=NULL; return(0);
    }
    else
    do {
           if (!cursor) cursor=top;
           else cursor=cursor->next;
           *locptr=cursor;
#ifdef DOS           
           if (!stricmp(location,cursor->where))
#else           
           if (!strcmp(location,cursor->where))
#endif           
              return(1);
        }
        while(cursor->next);
    return(0);
}

struct filenode *ftraverse(struct filenode *ftptr) {
/* returns pointer to next file node during a traversal */
static int ht;
    if (!ftptr) {
       ht=-1;
       while (htable[++ht]==NULL && ht < (HASHSIZE-1));
       ftptr=htable[ht];
    }
    else
       if ((ftptr=ftptr->next)==NULL && ht<(HASHSIZE-1)) {
          while (htable[++ht]==NULL && ht < (HASHSIZE-1));
          ftptr=htable[ht];
        }
   return(ftptr);
}

struct sysnode *straverse(struct sysnode *stptr, struct filenode *ftptr) {
/* returns pointer to next system node during a traversal */
    if (!stptr)
       stptr=ftptr->sptr;
    else
       stptr=stptr->next;
    return(stptr);
}

struct locnode *ltraverse(struct locnode *ltptr, struct sysnode *stptr) {
/* returns pointer to next location node during a traversal */    
    if (!ltptr)
       ltptr=stptr->lptr;
    else
       ltptr=ltptr->next;
    return(ltptr);
}

struct locnode *ldelete(struct locnode *lptr, struct sysnode *sptr, struct filenode *fptr) {
/* deletes a location node, returns pointer to previous node */
    struct locnode *ptr;
    dirty++;
    ptr=sptr->lptr;
    if (lptr->writ) fptr->numwrite--;
    if (UP2DATE(lptr->size,lptr->crc,fptr)) fptr->numcopy--;
    if (lptr==fptr->ulptr) { fptr->ulptr=NULL; fptr->usptr=NULL; }
    if (ptr!=lptr) {
       while (ptr->next != lptr)
          ptr=ptr->next;
       ptr->next=lptr->next;
    }
    else {
        sptr->lptr=lptr->next;
        ptr=sptr->lptr;
    }
    free(lptr); lptr=NULL;
    return(ptr);
}

struct sysnode *sdelete(struct sysnode *sptr, struct filenode *fptr) {
/* deletes system node, returns pointer to provious node */
    struct sysnode *ptr;
    dirty++;
    ptr=fptr->sptr;
    if (ptr!=sptr) {
       while (ptr->next !=sptr)
          ptr=ptr->next;
       ptr->next=sptr->next;
    }
    else  {
        fptr->sptr=sptr->next;
        ptr=fptr->sptr;
    }
    free(sptr);
    return(ptr);
}

struct filenode *fdelete(struct filenode *fptr) {
/* deletes file node, returns pointer to previous node */
    int hash;
    struct filenode *ptr;
    dirty++;
    ptr=htable[(hash=hashfctn(fptr->name))];
    if (ptr!=fptr) {
       while (ptr->next != fptr)
          ptr=ptr->next;
       ptr->next=fptr->next;
    }
    else {
        htable[hash]=fptr->next;
        ptr=htable[hash];
    }
    free(fptr);
    return(ptr);
}

int getlist(char fnames[], char *ptrs[], int diskflag, int sysflag) {
/* general purpose file/sys names list maker. The pattern for list
   inclusion is in global variable 'name'.
   diskflag: whether nor not the file must live on disk or be
   in the hashtable to be included.
   sysflag: whether we are looking for file or system names.
*/

   int i, j, count=0; char *ptr;
   ftptr=NULL;

   if (name[0]==NULS) {
	  if (sysflag)
              puts (" Enter system name pattern (*?)");
	  else
	     if (diskflag)
                 puts (" Enter file name pattern of files on disk (*?)");
	     else
                 puts (" Enter file name pattern (*?)");
	  fflush(stdout);
      gets(name);
      if (name[0]==NULS) return(0);
    }

   /* get an array of pointers to the file/sys names */
	if (diskflag) {
#ifdef QNX		
		if (diskflag==2 && floppy==2)
			count=getfloppylist(fnames);
		else			
#endif		
		count=getfnl(name,fnames,BUFSIZE,0);
	}
	else if (!sysflag) {
	    while ((ftptr=ftraverse(ftptr))!=NULL)
		if (matchpat(name,ftptr->name,NULS)) {
        		strcat(fnames,ftptr->name);
	        	strcat(fnames," ");
        		count++;
                  }
	}
	else
	    while ((ftptr=ftraverse(ftptr))!=NULL) {
	    	stptr=NULL;
		while ((stptr=straverse(stptr,ftptr))!=NULL) {
			if (matchpat(name,stptr->sysid,NULS)) {
				stcpm(fnames,stptr->sysid,&ptr);
				if (!ptr || (ptr && !matchpat(name,ptr,' '))) {
					strcat(fnames,stptr->sysid);
					strcat(fnames," ");
					count++;
				}
			}
		}
	    }

	switch (count) {
		case -1: error(0,LISTERROR,0); break;
		case 0: error(0,MATCHERROR,0); break;
		default:
		if (diskflag) {
			if (strbpl(ptrs,NPTRS,fnames) !=count) {
				error(0,LISTERROR,0); count=0;
			}
		}
		else {
			i=strlen(fnames);
			if ((i+1)>=BUFSIZE) error(0,BUFOVERFLOW,0);
			else  {
				for (j=1;j<i;j++)
				if (fnames[j]==' ') fnames[j]=NULS;
				fnames[i+2]=NULS;
				if (strbpl(ptrs,NPTRS,fnames) !=count) {
					error(0,LISTERROR,0); count=0;
				}
				else strsrt(ptrs,count);
			}
		}
	}
#ifdef DOS
	if (count)
		for(i=0;i<count;i++) strlwr(ptrs[i]);
#endif
	return(count);
}

void freememory(void) {
/* frees up memory, used by hashtable */
   int i=0;
   struct filenode *fptr;
   for (i=0;i<HASHSIZE;i++)
      while ((fptr=htable[i])!=NULL) {
         while (ldelete(fptr->sptr->lptr,fptr->sptr,fptr));
         if (!sdelete(fptr->sptr,fptr)) fdelete(fptr);
      }
   inmemory[0]=NULS;
}

int getuptodate(struct filenode *fptr) {
/* finds the most recent file, returns success status */
    struct sysnode *stptr2; struct locnode *ltptr2;

    /* check if current pointers are valid */
    if (fptr->ulptr)
       if (fptr->ulptr->writ&&fptr->ulptr->time==fptr->latest&&fptr->ulptr->crc==fptr->crcest&&fptr->ulptr->size==fptr->sizest)
	  if (fptr->numwrite==1)
             return(1);

    fptr->ulptr=NULL; fptr->usptr=NULL;

    /* favors a local up to date file */
    getsysname();
    stptr2=NULL; ltptr2=NULL;
    while((stptr2=straverse(stptr2,fptr))!=NULL)
       while ((ltptr2=ltraverse(ltptr2,stptr2))!=NULL)
          if (((ltptr2->writ && fptr->numwrite >= 1) ||
          (!ltptr2->writ && !fptr->numwrite)) &&
          ltptr2->time==fptr->latest && ltptr2->crc==fptr->crcest &&
          ltptr2->size==fptr->sizest) {
	  if (!fptr->ulptr || !stricmp(stptr2->sysid,sysname))
              fptr->ulptr=ltptr2; fptr->usptr=stptr2;
          }
    if (fptr->ulptr)
       return(1);
    printf(" WARNING:-%s: inconsistent most recent file\n",fptr->name);
    fflush(stdout); return(0);
}

void processfile(struct filenode *fptr, struct locnode *lptr) {
    short warn=0;
    char loc[FMSIZE]={NULS}, to[FMSIZE]={NULS}, from[FMSIZE]={NULS};
    struct sysnode *stptr2=NULL; struct locnode *ltptr2=NULL;
    /* assume database reflects true state.                */
    /* if fptrs-> != lptrs-> then need an update.          */
    /* check all locations for a date greater than lptrs-> */
    /* but less than fptrs->, warn if there are            */
    /* else update (check floppy).                         */
    if (!UP2DATE(lptr->size,lptr->crc,fptr)) {
        while ((stptr2=straverse(stptr2,fptr))!=NULL)
           while ((ltptr2=ltraverse(ltptr2,stptr2))!=NULL)
              if ((ltptr2->time > lptr->time)&&(ltptr2->time < fptr->latest))
                 if ((ltptr2->crc != lptr->crc && ltptr2->size != lptr->size)
    	              && (ltptr2->crc != fptr->crcest && ltptr2->size != fptr->sizest)) {
                 warn++;
                 printf(" WARNING:-for update of %s%s\n",lptr->where,fptr->name);
                 printf(" intermediate file at %s %s\n",stptr2->sysid,ltptr2->where);
                 puts(" update anyway? y/n");
                 if (getconfirm('Y')) warn--;
                }
        if (!warn) {
		sprintf(to,"%s%s",lptr->where,fptr->name);
        	if (floppy) {
        		sprintf(from,"%s%s",getlocation(projname,loc),fptr->name);
        		if (!EXISTS(from) || getcrc(from)!=fptr->crcest || getsize(from)!=fptr->sizest)
				from[0]=NULS;
        	}
		if (from[0]==NULS)
			if (!stricmp(fptr->usptr->sysid,getsysname()))
				sprintf(from,"%s%s",fptr->ulptr->where,fptr->name);
		if (from[0]!=NULS) {
		   if (!qu++) getask();
                   if (do_copy(from,"",to,"",asker))
                      chg_locnode(lptr,stptr,fptr,0,fptr->latest,fptr->sizest,fptr->crcest,lptr->writ);
                }
                else
                   printf(" update not completed for %s%s: the latest file is at %s %s\n", \
                   lptr->where,fptr->name,fptr->usptr->sysid,fptr->ulptr->where);
        }
    }
}

int consistent(void) {
    char hold[FMSIZE];
    int warn=0;
    ftptr=NULL;
    /* ltraverse and resolve inconsistencies */
    while ((ftptr=ftraverse(ftptr))!=NULL) {
       ftptr->numcopy=0;
       if (ftptr->numwrite>1)
          printf(" WARNING-%s: more than one writeable files (or update database)\n",ftptr->name);
       stptr=NULL;
       if (getuptodate(ftptr))
          while ((stptr=straverse(stptr,ftptr))!=NULL) {
            ltptr=NULL;
            while ((ltptr=ltraverse(ltptr,stptr))!=NULL) {
                strcat(strcpy(hold,ltptr->where),ftptr->name);
                if (UP2DATE(ltptr->size,ltptr->crc,ftptr))
                   ftptr->numcopy++;
                if (ltptr->writ && ftptr->ulptr!=ltptr) {
                   printf(" WARNING-%s:%s%s: invalid writeable version\n",stptr->sysid,ltptr->where,ftptr->name);
                   if (ltptr->crc!=ftptr->crcest || ltptr->size!=ftptr->sizest)
                      printf(" INCONSISTENCY:-%s:%s and %s:%s%s\n",stptr->sysid,hold,ftptr->usptr->sysid,ftptr->ulptr->where,ftptr->name);
                   warn++;
                }
            }
         }
         else warn++;
    }
   return(!warn);
}

void commitfile(char *filename, int flag, struct filenode *fptr,
    struct sysnode *sptr, struct locnode *lptr) {
    long int tme;
    short  wr;
	if (!EXISTS(filename)) {
		printf(" %s: no longer exists; deleted from database\n",filename);
		if ((lptr=ldelete(lptr,sptr,fptr))==NULL)
		if ((sptr=sdelete(sptr,fptr))==NULL)
		fptr=fdelete(fptr);
	}
	else {
		wr=(access(filename,2)) ? 0 : 1;
		tme=gettime(filename);
		/* if something has changed, then update */
        if ((tme!=lptr->time && tme!=lptr->time+1) || wr != lptr->writ)
           	if (!flag)
           	     printf(" WARNING-changes to %s not committed to database\n",filename);
			else {
				chg_locnode(lptr,sptr,fptr,0,tme,getsize(filename),getcrc(filename),wr);
				printf("\t%s%s\n",lptr->where,fptr->name);
			}
	}
fflush(stdout);	
}

void beg_resolve(int flag) {
/* check out all references in database for this system */
    char hold[FMSIZE];
    struct sysnode *sptr=NULL;
    ftptr=NULL;
    /* ltraverse and update locations */
    if (getsysname()) {
    while ((ftptr=ftraverse(ftptr))!=NULL)
        if (searchsys(sysname,ftptr->sptr,&sptr)) {
           ltptr=NULL;
           while ((ltptr=ltraverse(ltptr,sptr))!=NULL) {
               strcat(strcpy(hold,ltptr->where),ftptr->name);
               commitfile(hold,flag,ftptr,sptr,ltptr);
           }
        }
    }
    else error("WARNING-",NOSYS,0);
}

void end_resolve(void) {
/* copies files to floppy if other systems need to be updated */
/* or if only one copy exists for latest version of file */
    char loc[FMSIZE]={NULS}, hold[FMSIZE];
    char fnames[BUFSIZE]={NULS}, node[FNSIZE]={NULS}, *ptrs[NPTRS];
    register int i;
    struct filenode *fileptr;
    int count=0;
    ftptr=NULL; stptr=NULL; ltptr=NULL;
    if (floppy) {
       strcat(strcpy(name,getlocation(projname,loc)),"*.*");
       /* check updateness of files on floppy */
       if ((count=getlist(fnames,ptrs,2,0))>0)
          for (i=0;i<count;i++)
             if (stcgfn(node,ptrs[i]))
                if (searchfile(node,&fileptr))
                   if (getcrc(ptrs[i])!=fileptr->crcest || getsize(ptrs[i])!=fileptr->sizest)
		      do_delete(ptrs[i],0);

       qu=0;
       while((ftptr=ftraverse(ftptr))!=NULL)
          if (ftptr->usptr)
             if (!stricmp(ftptr->usptr->sysid,getsysname()))
                if (!EXISTS(strcat(strcpy(hold,loc),ftptr->name)))
                   if (ftptr->numcopy<2) {
				      if (!qu++) getask();
                      printf(" only one copy of file %s\n",ftptr->name);
                      do_copy(ftptr->ulptr->where,ftptr->name,getlocation(projname,loc),ftptr->name,asker);
                   }
                   else {
                      stptr=NULL;
                      while ((stptr=straverse(stptr,ftptr))!=NULL)
                         if (stricmp(stptr->sysid,ftptr->usptr->sysid)) {
                            ltptr=NULL;
                            while ((ltptr=ltraverse(ltptr,stptr))!=NULL)
                               if (!UP2DATE(ltptr->size,ltptr->crc,ftptr)) {
                                  if (!qu++) getask();
                                  do_copy(ftptr->ulptr->where,ftptr->name,loc,ftptr->name,asker);
                               }
                          }
                   }
    }
}


int readfile(char *filename) {
/* assume dos files names are lower case */	
    FILE *fd; register int i;
    char lne[LINE+1], item1[FMSIZE]; long int item2, item3;
    short int item5;
    unsigned short item4;
    struct filenode *fptr=0; struct sysnode *sptr; struct locnode *lptr;
    if ((fd=fopen(filename,"r"))!=NULL) {
       for (i=0;i<HASHSIZE;i++) htable[i]=NULL;
       while (fgets(lne,sizeof(lne),fd))
             switch (strspn(lne,"\t")) {
             	case 0: break;
                case 1: /* read items */
	               sscanf(lne,"%s %ld %ld %u",item1, &item2, &item3, &item4);
                   if (fix) strlwr(item1);	               
                   if (searchfile(item1,&fptr)) { error(filename,READERROR,0); return(0); }
                   fptr=add_filenode(1,fptr,item1,t2qnx(item2),item3,item4); sptr=NULL;
                   break;
                case 2: sscanf(lne,"%s",item1);
                   if (searchsys(item1,fptr->sptr,&sptr)) { error(filename,READERROR,0); return(0); }
                   sptr=add_sysnode(sptr,fptr,item1); lptr=NULL;
                   break;
                case 3: sscanf(lne,"%s %ld %ld %u %d",item1,&item2,&item3,&item4,&item5);
				   if (fix) strlwr(item1);                
                   if (searchloc(item1,sptr->lptr,&lptr)) { error(filename,READERROR,0); return(0); }
                   else lptr=add_locnode(lptr,sptr,fptr,item1,t2qnx(item2),item3,item4,item5);
                   break;
                default: error(filename,READERROR,0); return(0);
              }
              strcpy(inmemory,projname);
              dirty=0;
              fclose(fd);
              return(1);
        }
        else { error(0,DOSERROR,0); return(0); }
}


int writefile(char *filename) {
    FILE *fd;
    if (filename[0]==NULS) return(1);
    end_resolve();
    ftptr=NULL;
    if (!CAN_WRITE(filename)) {
    	printf(" %s is read-only, overwrite anyway? y/n\n",filename);
	if (getconfirm('Y'))
    	chg_attr(filename,"+w");
    }
    if ((fd=fopen(filename,"w"))!=NULL)
       while ((ftptr=ftraverse(ftptr))!=NULL) {
           /* write file record */
/*           fprintf(fd,"\t%s %ld %ld %u\r\l",ftptr->name,
           t2dos(ftptr->latest),ftptr->sizest,ftptr->crcest);*/
           fprintf(fd,"\t%s %ld %ld %u\n",ftptr->name,
           t2dos(ftptr->latest),ftptr->sizest,ftptr->crcest);          
           stptr=NULL;
           while ((stptr=straverse(stptr,ftptr))!=NULL) {
                /* write sys record */
/*                fprintf(fd,"\t\t%s\r\l",stptr->sysid);*/  /**/
		         fprintf(fd,"\t\t%s\n",stptr->sysid);  /**/
                ltptr=NULL;
                /* write loc list */
                while ((ltptr=ltraverse(ltptr,stptr))!=NULL) {
                   /* write loc record */
/*                   fprintf(fd,"\t\t\t%s %ld %ld %u %d\r\l",ltptr->where,
                   t2dos(ltptr->time),ltptr->size,ltptr->crc,ltptr->writ);*/
                   fprintf(fd,"\t\t\t%s %ld %ld %u %d\n",ltptr->where,
                   t2dos(ltptr->time),ltptr->size,ltptr->crc,ltptr->writ);
                }
             }
          }
    else { error(filename,DOSERROR,0); return(0); }
    fclose(fd);
    dirty=0;
    return(1);
}

int curmemory(void) {
    char loc[FMSIZE]={NULS};
    if (!strcmp(inmemory,projname))
       return(1);
    if (!dirty || (dirty && writefile(inmemory))) {
       if (inmemory[0]!=NULS) freememory();
       if (readfile(projname)) {
       	  	if (fix) { 
       	  		writefile(inmemory); 
	       	  	exit(0);
			}
       		if (isondfs(projname)) floppy=2;
	   	  	else {
		          getlocation(projname,loc); floppy=0;
	        	  if (loc[1]==':')
		          switch (loc[0]) {
				case 'a': case 'A':
				case 'b': case 'B':
		          	case '1': case '2': floppy=1; break;
		          }
			}
       		if (command!=VERSION) beg_resolve(0);
			return(1);
        }
	}
	return(0);
}

void up_file(void) {
    char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS], node[FNSIZE];
    register int i;
    int count=0;
    struct filenode *fileptr;
    /* get list of files to update */
    ltptr=NULL; qu=1;
    if (consistent()) {
       puts("\n updating ...");
       while ((count=getlist(fnames,ptrs,1,0))>0) {
          for (i=0;i<count; i++) {
             if (count>2) getask();
             if (stcgfn(node,ptrs[i]))
                if (searchfile(node,&fileptr))
                   if (searchsys(sysname,fileptr->sptr,&stptr))
                      while((ltptr=ltraverse(ltptr,stptr))!=NULL)
                         processfile(fileptr,ltptr);
                    else
                       printf(" %s: %s not on this system\n",progname,node);
                else
                   printf(" %s: %s not registered\n",progname,node);
             else
                printf(" %s: %s not a filename\n",progname, node);
           }
	name[0]=NULS;
	fnames[0]=NULS;
       }
    }
    else error(0,NOTDONE,0);
}

void com_file(void) {
    char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS], loc[FMSIZE]={NULS}, node[FNSIZE];
    register int i;
    int count=0;
    struct filenode *fptr;
    struct sysnode *sptr;
    struct locnode *lptr;
    /* get list of files to commit */
    lptr=NULL;
    puts("\n committing ...");
    while ((count=getlist(fnames,ptrs,1,0))>0) {
          for (i=0;i<count; i++) {
       		stcgfn(node,ptrs[i]);
                if (searchfile(node,&fptr))
                   if (searchsys(getsysname(),fptr->sptr,&sptr))
						if (searchloc(getlocation(ptrs[i],loc),sptr->lptr,&lptr))
         	              commitfile(ptrs[i],1,fptr,sptr,lptr);
          	        else
                            printf(" %s: %s not registered\n",progname,ptrs[i]);
          	   else
          	      printf(" %s: %s not registered\n",progname,ptrs[i]);
          }
	name[0]=NULS;
	fnames[0]=NULS;
    }
}

/* just adds files */
void add_file(void) {
    char fnames[BUFSIZE]={NULS}, node[FNSIZE], *ptrs[NPTRS], location[FMSIZE];
    char  ext[FESIZE], path[FMSIZE], projnode[FNSIZE];
    register int i,j; int count=1;
    struct locnode *locptr;
    struct sysnode *sysptr,*sp;
    struct filenode *fileptr, *fp;
if (getsysname()) {
  if (command==COPY)
     if (floppy) {
        if (count=getlist(fnames,ptrs,2,0)) {
        	puts(" Enter location to store these files");
        	gets(path); getpath(path);
		if (count>2) getask();
        	puts("\n copying ...");
		stcgfn(projnode,projname);
        	for (i=0; i<count; i++) {
        		stcgfn(node,ptrs[i]);
			if (stricmp(node,projnode))
        		    do_copy("",ptrs[i],path,node,asker);
        	}
		if (option!='F')
        	   strcat(strcpy(name,path),"*.*");
        }
     }
     else count=0;

   if (count)
    while ((count=getlist(fnames,ptrs,1,0))>0) {
       puts("\n registering ...");
       for (i=0; i<count; i++) {
	  ok=1;
          locptr=NULL; sysptr=NULL; fileptr=NULL; location[0]=NULS;
          if (stcgfn(node,ptrs[i])) {
	      if (command==COPY)
		  if (!EXISTS(strcat(getlocation(projname,location),node)) ||
	               (!stricmp(projnode,node))) ok=0;
	  if (ok)
          	if (stcgfe(ext,node))
          	/* check file extension for BADFILES */
          	for (j=0;j<NUM_BADFILES;j++)
          	if (!stricmp(badfiles[j],ext)) {
          		printf(" really want to register %s? y/n\n",ptrs[i]);
			getconfirm('Y');
          	}
          location[0]=NULS;
	  getlocation(ptrs[i],location);
          if (ok) {
     		printf("\t%s\n",ptrs[i]);
                if (searchfile(node,&fileptr))
                   if (searchsys(sysname,fileptr->sptr,&sysptr))
                      if (searchloc(getlocation(ptrs[i],location),sysptr->lptr,&locptr))
                          printf(" %s: %s is already registered\n",\
                          progname,ptrs[i]);
                      else
                         /* insert newlocation at locptr.next */
                         add_locnode(locptr,sysptr,fileptr,location,
                         gettime(ptrs[i]),getsize(ptrs[i]),getcrc(ptrs[i]),
                         getwrite(ptrs[i]));
                   else
                      /* insert new sysnode and new location */
                      add_locnode(locptr,
                      add_sysnode(sysptr,fileptr,sysname),fileptr,location,
                      gettime(ptrs[i]),getsize(ptrs[i]),getcrc(ptrs[i]),
                      getwrite(ptrs[i]));
               else {
                /* insert new filenode, sysnode and locnode */
       		fp=add_filenode(1,fileptr,node,gettime(ptrs[i]),getsize(ptrs[i]),getcrc(ptrs[i]));
       		sp=add_sysnode(sysptr,fp,sysname);
       		add_locnode(locptr,sp,fp,location,gettime(ptrs[i]),getsize(ptrs[i]),getcrc(ptrs[i]),getwrite(ptrs[i]));
               }
             }
          }
       }
       name[0]=NULS;
       fnames[0]=NULS;
       if (command==COPY) break;
   }
   consistent();
}
else { error(0,NOSYS,0); error(0,NOTDONE,0); }
}

void del_file(void) {
    int count=0,i;
    char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS], location[FMSIZE]={NULS}, node[FNSIZE];
    struct filenode *fptr;
    while ((count=getlist(fnames,ptrs,0,0))>0) {
       for (i=0; i<count; i++)
          if (stcgfn(node,ptrs[i]))
              if (searchfile(node,&fptr)) {
		/* ask */
                 while((fptr->sptr->lptr=ldelete(fptr->sptr->lptr,fptr->sptr,fptr))!=NULL);
                 while((fptr->sptr=sdelete(fptr->sptr,fptr))!=NULL);
                 fdelete(fptr);
                 printf(" %s deleted from database\n",node);
                 /* delete from floppy */
				if (floppy)
              	   if (EXISTS(strcat(getlocation(projname,location),node)))
				do_delete(location,asker);
               }
           else printf(" %s: %s not found or not registered\n",progname,node);
	name[0]=NULS;
	fnames[0]=NULS;
    }
}

void cop_sys(void) {
char location[FMSIZE];
if (projname[0]!=NULS) {
	if (floppy) {
		strcpy(name,strcat(getlocation(projname,location),"*.*"));
		add_file();
	}
	else {
		sprintf(location,"%s: not a floppy project file",projname);
		error(location,99,0);
	}
}
else error(0,NOPROJECT,0);
}

void add_sys(void) {
    char path[FMSIZE];
    FILE *fp;
    if (name[0] == NULS) {
       puts(" Enter your system identifier"); gets(name);
    }
    if (name[0] != NULS) {
       puts(" Enter location to store identifier");
       gets(path); getpath(path);
       strcat(path,"sys.id");
       if (fputs(name,fp=fopen(path,"w"))) error(0,DOSERROR,0);
       else (fclose(fp));
    }
name[0]=NULS;
}

void up_sys(void) {
/* traverse and for each file, go to this system list and update */
qu=0;
if (getsysname())
 if (projname[0]!=NULS) {
   ftptr=NULL; stptr=NULL;
   if (consistent()) {
      puts("\n updating ...");
      while ((ftptr=ftraverse(ftptr))!=NULL)
   	if (searchsys(sysname,ftptr->sptr,&stptr)) {
            ltptr=NULL;
             while ((ltptr=ltraverse(ltptr,stptr))!=NULL)
               processfile(ftptr,ltptr);
         }
   }
   else error(0,NOTDONE,0);
 }
 else error(0,NOPROJECT,0);
else error(0,NOSYS,0);
}

void com_sys(void) {
/* traverse and for each file, update the database */
if (projname[0]!=NULS) {
   puts("\n committing ...");
   beg_resolve(1);
}
else error(0,NOPROJECT,0);
}

void del_sys(void) { 
int count=0, i;
char snames[BUFSIZE], *ptrs[NPTRS];
struct sysnode *sptr;
if (projname[0]!=NULS) {
ftptr=NULL;
count=getlist(snames,ptrs,0,1);
for (i=0;i<count;i++)
	if (curmemory()) {
		while ((ftptr=ftraverse(ftptr))!=NULL)
		     if (searchsys(name,ftptr->sptr,&sptr)) {
		     	while (ldelete(sptr->lptr,sptr,ftptr));
		     	if (!sdelete(sptr,ftptr)) fdelete(ftptr); /* delete from floppy here */
		     }
		printf(" system %s files deleted from database %s\n",name,projname);      
	}
}
else error(0,NOPROJECT,0);
}


int getproject(void) {
    if (name[0]==NULS)
       if (projname[0]==NULS)
              error("",NOPROJECT,0);
       else {
 	if (command==PROJECT)
        printf(" current project is %s\n",projname);
       }
    else
       if (validate(name)) {
          printf(" current project is %s\n",projname);
          puts( "\n resolving ..."); fflush(stdout);
          curmemory();
          consistent();
       }
       else
       	error(0,NOTFOUND,0);

name[0]=NULS;
if (projname[0]==NULS) return(0); else return(1);
}

void cop_project(void)
{
	struct sysnode *sptr;
	char snames[BUFSIZE]={NULS}, *sptrs[NPTRS];
	int scount=0, first=1, i, l=0;
	if (getproject()) {
		cop_sys(); strcpy(name,"*");
		scount=getlist(snames,sptrs,0,1);
		if (scount) {
			for (i=0;i<scount;i++) {
				first=1; ftptr=NULL;
				while ((ftptr=ftraverse(ftptr))!=NULL)
				if (!searchsys(sptrs[i],ftptr->sptr,&sptr)) {
					if (first) { printf(" system %s does not have the following files:\n",sptrs[i]); SCROLL(l); }
					printf("\t%s\n",ftptr->name); fflush(stdout);
					first=0; SCROLL(l);
				}
			}
		}
	}
}

void up_project(void) {
    ftptr=NULL;
    if (getproject()) {
          up_sys();
          /* notify of other systems that need to be updated */
          while ((ftptr=ftraverse(ftptr))!=NULL) {
             stptr=NULL;
             while ((stptr=straverse(stptr,ftptr))!=NULL) {
                if (stricmp(stptr->sysid,getsysname())) {
                   ltptr=NULL;
                   while ((ltptr=ltraverse(ltptr,stptr))!=NULL)
                      if (ltptr->size!=ftptr->sizest || ltptr->crc!=ftptr->crcest) {
                         printf("\n update needed for file %s%s on system %s\n",ltptr->where,ftptr->name,stptr->sysid);
                         fflush(stdout); break;
                      }
                  }
              }
          }
    }
}

void com_project(void) {
char  *ptrs[NPTRS];
char snames[BUFSIZE]={NULS};
int j, count;
ftptr=NULL; stptr=NULL; ltptr=NULL;
if (getproject()) {
	com_sys();
	strcpy(name,"*"); count=getlist(snames,ptrs,0,1);
	if (!count)
   	   puts(" no files on any other system registered");
	else
	   if (count>1) {
		puts(" files on the following systems may need to be committed:");
		for (j=0;j<count;j++)
		    if (stricmp(sysname,ptrs[j]))
	   	        printf("\t%s\n",ptrs[j]);
	   }
    }
}

/* just gets projname */
void add_project(void) {
    FILE *fd;  char oldprojname[FMSIZE];
    strcpy(oldprojname,projname);
    projname[0]=NULS;
    if (curproj())
		printf(" %s: %s already registered\n",progname,projname);
    else  {
       if (name[0]!=NULS)
    	if ((fd=fopen(name,"w"))!=NULL) {
    		strcpy(projname,name);
    		name[0]=NULS;
#ifdef QNX
			putc(' ',fd);
#endif    		
    		fclose(fd);
    		if (curmemory()) add_file();
    	}
    	else
    	   error(0,DOSERROR,0);
       else
	strcpy(projname,oldprojname);
    }
}

void del_project(void) {
    char oldprojname[FMSIZE]={NULS};
    if (name[0]!=NULS) strcpy(oldprojname,projname);
    if (curproj()) {
       if (stricmp(projname,oldprojname)) oldprojname[0]=NULS;
       printf(" delete %s y/n?\n",projname);
       if (getconfirm('Y'))
          if (remove(projname)) error(0,DOSERROR,0);
          else
             if (!strcmp(inmemory,projname))
                freememory();
     }
     else error(0,NOTFOUND,0);
     if (inmemory[0]!=NULS)
        strcpy(projname,inmemory);
     else strcpy(projname,oldprojname);
}

void listafile(struct filenode *ftptr,int i) {
char rw, b[20];
static int lc;
if (!i) lc=1;
printf("File: %-12s  Latest: %17s; %7ld bytes; %7u crc;\n",ftptr->name,getdate(ftptr->latest,b),ftptr->sizest,ftptr->crcest);
SCROLL(lc); stptr=NULL;
while ((stptr=straverse(stptr,ftptr))!=NULL) {
	printf("      System: %s\n",stptr->sysid);
	SCROLL(lc); ltptr=NULL;
	while ((ltptr=ltraverse(ltptr,stptr))!=NULL) {
		rw=(ltptr->writ) ? 'W' : 'R';
		printf("              Location: %s\n",ltptr->where); SCROLL(lc);
		printf("                            %17s; %7ld bytes; %7u crc; %c\n",getdate(ltptr->time,b),ltptr->size,ltptr->crc,rw);
		SCROLL(lc);
	}
   }
fflush(stdout);   
}

void list_file(void)
{
int count=0,i;
char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS];
	count=getlist(fnames,ptrs,0,0);
	for (i=0;i<count;i++) {
		searchfile(ptrs[i],&ftptr);
		listafile(ftptr,i);
	}
}

void list_project(void) {
char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS];
int count=0, i;
ftptr=NULL;
if (getproject()) {
	printf("Project: %s\n",projname);
	strcpy(name,"*"); count=getlist(fnames,ptrs,0,0);
	for (i=0; i<count;i++) {
		searchfile(ptrs[i],&ftptr);
		listafile(ftptr,i);
	}
   }
}

void list_sys(void) {
char snames[BUFSIZE]={NULS}, *ptrs[NPTRS];
int count=0,i,lc=0;
char rw, b[20];
if (projname[0]!=NULS) {
stptr=NULL;
count=getlist(snames,ptrs,0,1);
for (i=0;i<count;i++) {
	printf("System: %s\n",ptrs[i]); ftptr=NULL; SCROLL(lc);
	while ((ftptr=ftraverse(ftptr))!=NULL)
		if (searchsys(ptrs[i],ftptr->sptr,&stptr)) {
			printf("\tFile: %s\n",ftptr->name); ltptr=NULL; SCROLL(lc);
			while (ltptr=ltraverse(ltptr,stptr)) {
				if (ltptr->writ) rw='W';
				else rw='R';
				printf("\t\tLocation: %s\n",ltptr->where); SCROLL(lc);
				printf("\t\t\t%17s; %7ld bytes; %7u crc;   %c\n",getdate(ltptr->time,b),ltptr->size,ltptr->crc,rw); SCROLL(lc);
			}
		}
    }
    fflush(stdout);
}
else error(0,NOPROJECT,0);
}

void request(void) {
/* request write permission for a file */
int count=0,i;
char fnames[BUFSIZE]={NULS}, *ptrs[NPTRS], location[FMSIZE]={NULS};
char hold[FMSIZE], node[FNSIZE];
struct filenode *fptr; struct sysnode *sptr; struct locnode *lptr;
ok=0;
if (getsysname())
   if (projname[0]!=NULS)
      if (consistent()) {
         while ((count=getlist(fnames,ptrs,1,0))>0) {
            for (i=0; i<count; i++)
	        if (stcgfn(node,ptrs[i]))
               if (searchfile(node,&fptr))
                  if (searchsys(sysname,fptr->sptr,&sptr))
                     if (searchloc(getlocation(ptrs[i],location),sptr->lptr,&lptr))
                        if (UP2DATE(lptr->size,lptr->crc,fptr)) {
                           /* requested file is up to date */
                           ok=1;
                           if ((fptr->ulptr!=lptr&&fptr->numwrite==1)||(fptr->numwrite==0)) {
                              if (stricmp(sysname,fptr->usptr->sysid)) {
                                 if (fptr->numwrite) {
                                    printf(" the current writeable version of %s is on system %s\n",fptr->name,fptr->usptr->sysid);
                                    printf(" is %s up to date concerning %s:%s%s? y/n\n",projname,fptr->usptr->sysid,fptr->ulptr->where,fptr->name);
                                    if (!getconfirm('Y'))
                                       printf(" resolve %s for system %s, (VERSION) and then request write\n",projname,fptr->usptr->sysid);
                                 }
                              }
                              if (ok) {
                                 if (chg_attr(ptrs[i],"+w")) {
                                    printf(" write permission request for %s granted\n",ptrs[i]);
                                    lptr->writ=1; dirty++;
                                    if (fptr->numwrite==0)
                                       fptr->numwrite++;
                                    else /* number of writeables is one */
                                       if (!stricmp(fptr->usptr->sysid,sysname))
                                          if (chg_attr(strcat(strcpy(hold,fptr->ulptr->where),fptr->name),"-w")) {
                                             fptr->ulptr->writ=0;
                                             fptr->ulptr=lptr; fptr->usptr=sptr;
                                          }
                                          else fptr->numwrite=2;
                                       else { /* warn user to make other one read-only */
                                          fptr->numwrite=2;
                                          printf("\n INSTRUCTION:-make %s%s on system %s read-only NOW and update database\n",fptr->ulptr->where,fptr->name,fptr->usptr->sysid);
                                          fptr->ulptr=lptr; fptr->usptr=sptr;
                                       }
                                    }
                                    else error(0,DOSERROR,0);
                                 }
                              }
                              else printf(" write permission already exists for %s\n",ptrs[i]);   
                           }
                           else printf(" write permission request for %s denied; file not up to date\n",ptrs[i]);
                        else printf(" %s not registered\n",ptrs[i]);
                     else printf(" no file locations for %s registered on system %s\n",fptr->name,sysname);
                  else printf(" %s not registered\n",node);
               name[0]=NULS;
               fnames[0]=NULS;
            }
         }
         else error(0,NOTDONE,0);
     else error(0,NOPROJECT,0);
  else error(0,NOSYS,0);
}

