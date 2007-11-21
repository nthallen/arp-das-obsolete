/*
filesrcs.c: file handling routines and utility routines.
*/

#include "inclsrcs.h"
#include "globsrcs.h"

void error(char *msg, int code, int fatal)
{	char *c=": ", *p="";
	if (msg==0) msg=p;
	if (msg[0]==NULS) c=p;
	switch(code) {
	case DOSERROR: poserr(progname); printf(" errno is %d\n",errno);printf("%s\n",msg); break;
	case NOPROJECT: printf(" %s: %s%sno current project specified\n",progname,msg,c); break;
	case NOSYS: printf(" %s: %s%sthis system not registered or sys.id not found\n",progname,msg,c); break;
	case NOTFOUND:
		if (name[0]!=NULS)
		    printf(" %s: %s%s%s not found or not registered\n", progname,msg,c,name);
		    name[0]=NULS; break;
	case MATCHERROR: printf(" %s: %s%sno matches found for pattern '%s'\n",progname,msg,c,name);
		name[0]=NULS; break;
	case NOTDONE: printf(" %s: %s%scommand not performed due to inconsistencies\n",progname,msg,c);
	break;
	case BUFOVERFLOW: printf(" %s: %s%sinternal buffer overflow; too many file/sys names\n",progname,msg,c);
	break;
	case LISTERROR:
		if (_OSERR) error(0,DOSERROR,0);
		else error(0,BUFOVERFLOW,0);
	break;
	case READERROR: printf("%s: %s%serror reading file\n",progname,msg,c);
	break;
	case BADCOMMAND: printf(" %s: %s%sNo such command\n",progname,msg,c);
	break;
	default: printf(" %s%s%s\n",progname,c,msg);
	}
fflush(stdout);	
if (fatal) exit(1);
}

int isondfs(char *filename) {
#ifdef QNX
	char *p;
	if ((p=strchr(filename,(int)':'))!=NULL) {
		p--;
		if (isalpha(*p))
			return(1);
	}
#endif	
	return(0);
}

int chg_attr(char *filename, char *attr) {
int i=0, len, err=0;
#ifdef QNX
struct dir_entry buffer;
FILE *fp;
if ((fp=fopen(filename,"m"))==NULL) {
	error(filename,DOSERROR,0);
	return(0);
}
if (get_dir_entry(fp,&buffer)) {
	error(filename,DOSERROR,0);
	return(0);
}
#endif
len=strlen(attr);
while (i<=len) {
	switch (attr[i]) {
		case '+': switch (attr[i+1]) {
						case 'r':
#ifdef QNX
							buffer.fattr |= _READ;
							buffer.fgperms |= _READ;
#endif
							break;
						case 'w':
#ifdef QNX
							buffer.fattr |= _WRITE;
							buffer.fgperms |= _WRITE;
#else
							err=chgfa(filename,0);
#endif						
							break;
						default :
							break;
						}
						break;
		case '-': switch (attr[i+1]) {
						case 'r':
#ifdef QNX
							buffer.fattr &= ~_READ;
							buffer.fgperms &= ~_READ;
							buffer.fperms &= ~_READ;
#endif						
							break;
						case 'w':
#ifdef QNX
							buffer.fattr &= ~_WRITE;
							buffer.fgperms &= ~_WRITE;
							buffer.fperms &= ~_WRITE;							
#else
							err=chgfa(filename,1);
#endif						
							break;
						default :
							break;
						}
		}						
		i+=2;
	}
#ifdef QNX
if (set_dir_entry(fp,&buffer))
	err=-1;
fclose(fp);
#endif	
if (err) error(filename,DOSERROR,0);
return(!err);
}

int getconfirm(char change) {
/*
get a confirmation from the user. this function returns and sets the
global variable 'ok' if the significant input character equals 'change'.
*/
char *t;
fflush(stdout);
change=toupper(change);
gets(str);
t=strtok(str,"\t ");
*t=toupper(*t);
if (*t==change) ok=1;
else ok=0;
return(ok);
}

do_copy(char *s1, char *s2, char *s3, char *s4, int ask) {
/* performs a copy with inquiry, returns success status */
char cmd[LINE], dest[FMSIZE]={NULS}, from[FMSIZE]={NULS};
#ifdef QNX
int chk=0;
#endif
int chk2=0, ok=1;
sprintf(cmd,"copy %s%s %s%s",s1,s2,s3,s4);
if (ask) {
	printf(" %s ? y/n\n",cmd);
	ok=getconfirm('Y');
}
else printf(" %s\n",cmd);
if (ok) {
	sprintf(dest,"%s%s",s3,s4);
	sprintf(from,"%s%s",s1,s2);
#ifdef QNX
	if (floppy && ((s3[0]=='A' || s3[0]=='B') && s3[1]==':') ||
			((s4[0]=='A' || s4[0]=='B') && s4[1]==':'))
		if (!CAN_WRITE(from)) {
			chg_attr(from,"+w"); chk++;
		}
#endif	
	if (EXISTS(dest))
		if (!CAN_WRITE(dest)) {
			chg_attr(dest,"+w"); chk2++;
		}
	if (system(cmd)!=0) {
           error(0,DOSERROR,0); ok=0;
           puts("\n retry? y/n");
           if (getconfirm('Y')) {
           	 ok=1; do_copy(s1,s2,s3,s4,0);
           }
           else {
           		ok=0;
           		if (chk2) chg_attr(dest,"-w");
			}
	}
}
else ok=0;
#ifdef QNX
if (chk) chg_attr(from,"-w");
#else
if (ok && floppy && ((s3[0]=='A' || s3[0]=='B') && s3[1]==':') ||
		((s4[0]=='A' || s4[0]=='B') && s4[1]==':'))
		chg_attr(dest,"+w");
#endif
return(ok);
}

int do_delete(char *filename, int ask)
/* performs a delete with inquiry, returns success status */
{
ok=1;
if (ask) {
	printf(" delete %s ? y/n\n",filename);
	getconfirm('Y');
}
else printf(" deleting out of date file %s\n",filename);
if (ok) {
#ifdef QNX
remove(filename);
#else	
	if (!chg_attr(filename,"+w") || remove(filename)) {
       		error(0,DOSERROR,0); ok=0;
       		puts("\n retry? y/n");
		if (getconfirm('Y'))
		    do_delete(filename,0);
       		else return(0);
       	}
#endif
}
return(1);
}

int validate(char *filename) {
/* validates project name, return existence, return true filepath in filename */    
    FILE *fd;
    char path[FMSIZE]={NULS};
    if ((fd=fopene(filename,"r",path))==NULL)
       if (!strpbrk(filename,".")) {
           strcat(filename,".rcs");
           if ((fd=fopene(filename,"r",path))==NULL)
              return(0);
        }
        else return(0);
    if (path) {
         strcpy(projname,path);
         strcat(projname,filename);
        }
    fclose(fd);
    return(1);
}

char *getsysname(void) {
/* returns this systems name */
#ifdef QNX
    if (sysname[0]==NULS) {
		utoa(My_nid,sysname,10);
	}    	
#else	
    FILE *fd; int i;
    if (sysname[0]==NULS) {
       if ((fd=fopene("sys.id","r",NULL))==NULL)
          return(NULL);
       fgets(sysname,FNSIZE,fd);
       if (sysname[i=strlen(sysname)-1]=='\n') sysname[i]=NULS;
       strlwr(sysname);
       fclose(fd);
    }
#endif    
    return(sysname);
}

void getpath( char *path) {
int i=0; char slash;
fflush(stdout);
#ifdef QNX
slash='/';
#else
slash='\\';
#endif
i=strlen(path);
if (i && (path[i-1]!=':' && path[i-1]!=slash)) {
	path[i]=slash;
	path[i+1]=NULS;
}
}

#ifdef QNX
char *getlocation(char *name, char *full_name) {
char *p, temp[FMSIZE]={NULS};
if (full_path(full_name,name)) {
	if ((p=strchr(full_name,']'))!=NULL) {
		strcpy(temp,++p);
		strcpy(full_name,temp);
	}
	p=strrchr(full_name,'/'); p++;
	*p=NULS;
}
else
	error("error in getlocation",99,1);
return(full_name);		
} 
#else
/* written by Norton Allen */
/* modified by eil */
/* modified to return the fullname string or Null if error */
/* modified to always return drive */
/* returns full path only */
char *getlocation(char *name, char *full_name) {
  char def_dir[64], drive[4], *ddp;
  int last_colon, last_slash, i, cdrv;

  cdrv = 0;
  while (isspace(*name)) name++;
  if (name[1] == ':') {
    cdrv = toupper(name[0])-'A'+1;
    stccpy(drive, name, 3);
  } else {
      drive[0]=getdsk()+'A'; drive[1]=':'; drive[2]='\0';
    }
  /* drive[0] = '\0';*/
  strcat(drive, "\\");
  if (getcd(cdrv, full_name)) return(NULL);
  ddp = stpcpy(def_dir, drive);
  strcpy(ddp, full_name);
  last_colon = last_slash = -1;
  for (i = 0; name[i] != '\0'; i++) {
    if (name[i] == '\\') last_slash = i;
    else if (name[i] == ':') last_slash = last_colon = i;
  }
  if (last_slash > last_colon) {
    if (last_slash == last_colon+1) last_colon = last_slash+1;
    else last_colon = last_slash;
    i = name[last_colon];
    name[last_colon] = '\0';
    if (chdir(name)) return(NULL);
    name[last_colon] = i;
    ddp = stpcpy(full_name, drive);
    if (getcd(cdrv, ddp)) return(NULL);
    ddp = strchr(ddp, '\0');
    chdir(def_dir);
  } else ddp = stpcpy(full_name, def_dir);
  if (ddp[-1] != '\\') ddp = stpcpy(ddp, "\\");
  /* ddp = stpcpy(ddp, name+last_slash+1);
  strcpy(ddp, ".SPS");*/
  for (ddp = full_name; *ddp != '\0'; ddp++) *ddp = tolower(*ddp);
  return(full_name);
}
#endif

void getask(void) {
puts(" query before copying? y/n");
asker=!getconfirm('N');
}


long int gettime(char *filename) {
/* returns the date of a file */
    long int ft;
#ifdef QNX
/*	long int fg=0,c1,c2,c3;*/
	struct stat stbuf;
    if (!stat(filename,&stbuf)) {
    	ft=stbuf.st_mtime;
        return(ft);
    }
#else
    int fd;
    if ((fd=open(filename,O_RDONLY,0))>=0) {
        ft=getft(fd);
        close(fd);
        return(ft);
    }
#endif
    else error(0,DOSERROR,0);
}

char *getdate(long tim, char *buf)
{
#ifdef QNX
	struct tm *t;
	t=localtime(&tim);
	strftime(buf,20,"%m/%d/%y %X",t);
#else
	char dat[6], *p;
	ftunpk(tim,dat);
	p=stpdate(buf,2,dat); *p++=' '; p=stptime(p,2,&dat[3]);
#endif
	return(buf);
}

long int getsize(char *filename) {
/* returns the size of a file */
/* correct for qnx, by number of bytes + number of RS's */
 int fd; long size; int rd=0;
#ifdef QNX
 char buf[100];
 FILE *fp;
 int lnecnt=0;
#endif 

if (!CAN_READ(filename)) {
	chg_attr(filename,"+r");
	rd=1;
}

   if ((fd=open(filename,O_RDONLY,0))>=0) {
#ifdef QNX
		if (!isondfs(filename)) {
			fp=fdopen(fd);
		 	while (fgets(buf,100,fp))
				if (buf[strlen(buf)-1]==0x1E)
					lnecnt++;
		}
#endif      
      if ((size=lseek(fd,0,2))<0) error(0,DOSERROR,1);
   }
   else error(0,DOSERROR,1);

   if (rd) chg_attr(filename,"-r");

#ifdef QNX
	   size+=lnecnt;
	   fclose(fp);
#endif   
   close(fd);
   return(size);
}

short int getwrite(char *filename) {
/* returns file's writeability for registering */
	if (!CAN_WRITE(filename)) return(0);
	if (!chg_attr(filename,"-w")) {
		error(0,DOSERROR,0);
		return(1);
	}
	return(0);
}

/* case insensitive match pattern */
int matchpat(char *pat, char *word, char end) {
int i=0,j=0,ast=0,bad=0,done=0;
char hold[FMSIZE];
strcpy(hold,word);
while (!bad && hold[j]!=end && !done)
	switch (pat[i]) {
		case '*':
	        ast=1;
			i++;
			break;
		case '?':
			i++; j++;
			break;
		case NULS:
			if (!ast) bad++;
			else done=1;
			break;
		default :
			if (!ast)
				if (pat[i]==hold[j]) {
					i++; j++;
				}
				else bad++;
			else
				if (pat[i]==hold[j]) {
					ast=0; i++; j++;
				}
				else j++;
	}
return (!bad);
}

int curproj(void) {
/* to be used with command routines that may have a project name as
   an argument */
    if (name[0]==NULS) {
        if (projname[0]==NULS) {
           puts(" Enter your project name"); fflush(stdout);
           gets(name);
           if (name[0]==NULS) return(0);
        }
	else return(1);
    }
    if (!validate(name)) return(0);
    return(1);
}

 /* Calculate, intelligently, the CRC of a dataset incrementally given a 
 * buffer full at a time.
 * Initialize crc to 0 for XMODEM, -1 for CCITT.
 *  * Usage: newcrc = updcrc( oldcrc, bufadr, buflen )
 * 		unsigned int oldcrc, buflen;
 * 		char *bufadr;
 */

    /* the CRC polynomial. This is used by XMODEM (almost CCITT).
     * If you change P, you must change crctab[]'s initial value to what is
     * printed by initcrctab()
     */
#define	P	0x1021

    /* number of bits in CRC: don't change it. */
#define W	16

    /* this the number of bits per char: don't change it. */
#define B	8

static unsigned short crctab[1<<B] = { /* as calculated by initcrctab() */
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
    };

unsigned short updcrc(unsigned short icrc,unsigned char *icp,unsigned int icnt)
{
    unsigned short crc = icrc;
    unsigned char *cp = icp;
    unsigned int cnt = icnt;

    while( cnt-- ) {
#ifdef QNX
		if (*cp==0x1E)
		crc = (crc<<B) ^ crctab[(crc>>(W-B)) ^ 13];		
		*cp=10;
#endif	
		crc = (crc<<B) ^ crctab[(crc>>(W-B)) ^ *cp++];
    }

    return( crc );
}

extern char* progname;
#define MAXBUF	4096

unsigned short int getcrc(char *filename)
{
/* correct for QNX, set RS's to CR/LF, and count RS's */	
    int nr;
    char buf[MAXBUF];
    unsigned short crc;
    int rd=0;
	int fd;

if (!CAN_READ(filename)) {
	chg_attr(filename,"+r");
	rd=1;
}
    crc=0;
    fd=open(filename,O_RDONLY,0);
    while( (nr = read( fd, buf, MAXBUF )) > 0 ) {
        crc = updcrc( crc, buf, nr );
	}
    if( nr != 0 ) poserr(progname);
    close(fd);   	
	if (rd) chg_attr(filename,"-r");
    return(crc);
}

time_t t2dos(time_t time)
{
#ifdef QNX	
	time_t fg=0, c1=0, c2=0, c3=0, c4=0;
	struct tm *tmbuf;
	/* get date */
	tmbuf=localtime(&time);
	/* pack dos long time */
   	fg|=((tmbuf->tm_sec)/2);
   	fg|=(tmbuf->tm_min<<5);
   	c4=tmbuf->tm_hour;
   	c1=tmbuf->tm_mday;
	c2=tmbuf->tm_mon+1;
   	c3=tmbuf->tm_year-80;
   	fg|=(c4<<11);
   	fg|=(c1<<16);
   	fg|=(c2<<21);
   	fg|=(c3<<25);
   	return(fg);
#else
	return(time);
#endif	   	
}

time_t t2qnx(time_t time)
{
#ifdef QNX	
	long sec, min, hr, day, mth, yr, days;
	int i,ad=0;
	sec=(time&0x1F)*2;
	min=(time&0x7E0)>>5;
	hr=(time&0xF800)>>11;
	day=(time&0x1F0000)>>16;
	mth=(time&0x1E00000)>>21;
	yr=(time&0xFE000000)>>25;
	/* figure out how many days since jan 1 1980 */
	days=yr*365+(yr/4);
	for (i=1;i<mth;i++)
		switch(i) {
			case 1: case 3: case 5: case 7: case 8: case 10: case 12: ad+=31; break;
			case 4: case 6: case 9: case 11: ad+=30; break;
			case 2: ad+= (yr%4) ? 28 : 29; break;
		}
	days+=ad; days+=day;
	return(sec+(min*60)+(hr*60*60)+(days*24*60*60));
#else
return(time);
#endif	
}


/* cant seek in directories using dfs for qnx */
int getfloppylist(char fnames[]) {
	FILE *fp;
	char location[FMSIZE];
	char buf[FMSIZE], cmd[FMSIZE], *p;
	int count=0;
	getlocation(projname,location);
/*	if (location[0]==NULS) strcpy(pat,name);
	else strcpy(pat,strrchr(name,'/')+1); */
	sprintf(cmd,"ls %s c=1 -d p=* > floppylist.tmp",location);
	system(cmd);
	if ((fp=fopen("floppylist.tmp","r"))==NULL) return(0);
	p=fnames;
	/* no error checking ! */
	while (fgets(buf,FMSIZE,fp)) {
		buf[strlen(buf)-1]='\0';
		strcpy(p,location);
		p+=strlen(location);
		strcpy(p,buf);
		p+=strlen(buf)+1;
		count++;
	}
	*p='\0';
	fclose(fp);
	remove("floppylist.tmp");
	return(count);
}
