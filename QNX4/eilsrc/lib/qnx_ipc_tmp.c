#include <string.h>
#include <malloc.h>
#include "qnx_ipc.h"

char ipc_tmp[PATH_MAX];

char *qnx_ipc_tmp(pid_t pid, char suffx, char *name) {
  char *s, *buf;
  int i,j;

  if (name==NULL || *name=='\0')
    sprintf(ipc_tmp,"%s%d%c",ipc_dir,pid,suffx);
  else {
    s=(*name=='/') ? name+1 : name;
    buf=malloc(strlen(s)+1);
    for (i=0,j=0;i<strlen(s);i++)
      if (s[i]!='/') buf[j++]=s[i];
    buf[j]='\0';
    sprintf(ipc_tmp,"%s%s",ipc_dir,buf);
    free(buf);
  }
  return(ipc_tmp);
}
