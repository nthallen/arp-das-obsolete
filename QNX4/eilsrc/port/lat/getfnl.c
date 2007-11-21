/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		getfnl - get file name list
*
* synopsis	count = getfnl(fnp,fna,fnasiz,attr);
*
*		int count;		number of bytes moved to fna
*		char *fnp;		file name pattern
*		char *fna;		file name array
*		unsigned fnasiz;	size of file name array
*		int attr;		file attribute
*
* description	This function searches a disk directory to find the
*		file names that match the specified pattern.  The pattern
*		can contain ? and * meta-characters.
*
*		If the search is successful, the file name array will
*		contain a sequence of null-terminated strings.  The last
*		string is terminated by 2 nulls.
*
* returns	count = 0 if no matching names found
*		      = -1 if error such as bad pattern or not enough space
*			   in fna
*
*
**/

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#include <lat.h>

int getfnl(fnp,fna,fnasiz,attr)
const char *fnp;
char *fna;
unsigned fnasiz;
int attr;
{
    int count=0, cat=0, num=1,j=1; 
    char dir[FMSIZE];
    char ffile[FMSIZE];
    char pat[FNSIZE];
    char *p;
    DIR *dirp;
    struct dirent *direntp;
    struct stat st;

    /* looking for dos hidden files, system files, volume label files */
    if (attr & 0x0E)
    	return(0);
    /* looking for subdirectories */
    if (attr & 0x10) j=2;
    
    p=fna;
    if (stcgfp(dir,fnp)) cat++;
    else getcwd(dir,FMSIZE);

    if ((dirp=opendir(dir))==NULL) return(0);

    if (cat)  /* there is a directory in the pattern */
	strcpy(pat,strrchr(fnp,(int)'/')+1);
    else strcpy(pat,fnp);

    if ( ffile[strlen(dir)-1]!='/') strcat(dir,"/");

    while ((direntp=readdir(dirp))!=0)
	if (!fnmatch(pat,direntp->d_name,FNM_PATHNAME)) {
	    strcpy(ffile,dir);
	    if (!stat(strcat(ffile,direntp->d_name),&st))
	    if ((S_ISREG(st.st_mode) && j==1) || (S_ISDIR(st.st_mode) && j==2)) {
		if (cat) {
		    num+=strlen(dir);
		    if (num<fnasiz) {
			strcpy(p,dir);
			p+=strlen(dir);
		    }
		}
		num+=strlen(direntp->d_name);
		if (num<fnasiz) {
		    strcpy(p,direntp->d_name);
		    p+=strlen(direntp->d_name)+1;
		}
		if (num<fnasiz) count++;
		else {
		    count=-1;
		    break;
		}
	    }
	}
closedir(dirp);
*p='\0';		
return(count);	
}
