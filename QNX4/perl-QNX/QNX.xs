#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/psinfo.h>
#include <sys/seginfo.h>
#include <sys/vc.h>
#include <errno.h>

static int qnx_errno;
static char *qnx_recvbuf;
static int qnx_recvbufsize;
static int grow_recvbuf( int newsize ) {
  if ( newsize > qnx_recvbufsize ) {
	char *new_buf = (char *) realloc( qnx_recvbuf, newsize );
	if ( new_buf == 0 ) {
	  qnx_errno = ENOMEM;
      return -1;
	} else {
	  qnx_recvbuf = new_buf;
	  qnx_recvbufsize = newsize;
	}
  }
  return 0;
}

MODULE = QNX		PACKAGE = QNX		

SV*
errno()
	CODE:
	RETVAL = newSViv( qnx_errno );
	OUTPUT:
	RETVAL

SV*
Send(sv_pid, smsg, rmsg, sv_size_send, sv_size_reply)
	SV* sv_pid
	SV* smsg
	SV* rmsg 
	SV* sv_size_send
	SV* sv_size_reply
	PREINIT:
	unsigned size_send = SvUV( sv_size_send );
	unsigned size_reply = SvUV( sv_size_reply );
	pid_t pid = SvIV( sv_pid );
	char* send_msg = SvPV_nolen( smsg );
	int r = 0;
	CODE:
	r = grow_recvbuf( size_reply );
	if ( r == 0 ) {
	  r = Send( pid , send_msg, qnx_recvbuf, size_send, size_reply);
	  if( r == 0 )
		  sv_setpvn( rmsg, qnx_recvbuf, size_reply );
	  else qnx_errno = errno;
    }
	RETVAL = newSViv( r );
	OUTPUT:
	rmsg
	RETVAL

SV*
Receive(sv_pid, rvmsg, sv_size_receive)
	SV* sv_pid
	SV* rvmsg
	SV* sv_size_receive
	PREINIT:
	unsigned size_receive = SvNV( sv_size_receive );
	pid_t pid = SvIV( sv_pid );
	int r;
	CODE:
	r = grow_recvbuf( size_receive );
	if ( r == 0 ) {
	  r = Receive( pid, qnx_recvbuf, size_receive );
	  if( r != -1 )
		  sv_setpvn( rvmsg, qnx_recvbuf, size_receive );
	  else qnx_errno = errno;
	}
	RETVAL = newSViv( r );
	OUTPUT:
	rvmsg
	RETVAL

SV*
Reply(sv_pid, rpmsg, sv_size_reply)
	SV* sv_pid
	SV* rpmsg
	SV* sv_size_reply
	PREINIT:
	unsigned size_reply = SvNV( sv_size_reply );
	pid_t pid = SvIV( sv_pid );
	char* reply_msg = SvPV_nolen( rpmsg );
	CODE:
	{ pid_t who;
	  who = Reply( pid, reply_msg, size_reply );
	  if ( who == -1 ) qnx_errno = errno;
	  RETVAL = newSViv( who );
	}
	OUTPUT:
	RETVAL

int
name_attach(sv_nid, sv_name)
	SV* sv_nid
	SV* sv_name
	PREINIT:
	nid_t nid = SvIV( sv_nid );
	char* name = SvPV_nolen( sv_name );
	CODE:
	{ int name_id;
	  name_id = qnx_name_attach( nid, name );
	  if ( name_id == -1 ) qnx_errno = errno;
	  RETVAL = name_id;
	}
	OUTPUT:
	RETVAL

int
name_detach(sv_nid, sv_name_id)
	SV* sv_nid
	SV* sv_name_id
	PREINIT:
	nid_t nid = SvIV( sv_nid );
	int name_id = SvIV( sv_name_id );
	CODE:
	{ int rv;
	  rv = qnx_name_detach( nid, name_id );
	  if ( rv == -1 ) qnx_errno = errno;
	  RETVAL = rv;
	}
	OUTPUT:
	RETVAL

SV*
name_locate(sv_nid, sv_name, sv_size, sv_copies)
	SV* sv_nid
	SV* sv_name
	SV* sv_size
	SV* sv_copies
	PREINIT:
	nid_t nid = SvIV( sv_nid );
	char* name = SvPV_nolen( sv_name );
	unsigned size = SvNV( sv_size );
	unsigned copies = SvIV( sv_copies );
	unsigned* cptr = &copies;
	int r;
	CODE:
	r = qnx_name_locate( nid, name, size, cptr );
	if( r != -1 )	
		sv_setiv( sv_copies, copies );
	else qnx_errno = errno;
	RETVAL = newSViv( r );
	OUTPUT:
	sv_copies
	RETVAL 

SV* 
proxy_attach(sv_pid, sv_data, sv_size, sv_priority)
	  SV* sv_pid
	  SV* sv_data
	  SV* sv_size
	  SV* sv_priority
	PREINIT:
	  pid_t pid = SvIV( sv_pid );
	  char* data = SvPV_nolen( sv_data );
	  int size = SvIV( sv_size );
	  int priority = SvIV( sv_priority );
	CODE:
	  { pid_t proxy;
		proxy = qnx_proxy_attach( pid, data, size, priority );
		if ( proxy == -1 ) qnx_errno = errno;
		RETVAL = newSViv( proxy );
	  }
	OUTPUT:
	  RETVAL

int
proxy_detach(sv_pid)
	  SV* sv_pid
	PREINIT:
	  pid_t pid = SvIV( sv_pid );
	CODE:
	  { int rv;
		rv = qnx_proxy_detach( pid );
		if ( rv == -1 ) qnx_errno = errno;
		RETVAL = rv;
	  }
	OUTPUT:
	  RETVAL

SV*
Trigger(sv_proxy)
	  SV* sv_proxy
	PREINIT:
	  pid_t proxy = SvIV( sv_proxy );
	CODE:
	  { pid_t pid = Trigger( proxy );
		if ( pid == -1 ) qnx_errno = errno;
		RETVAL = newSViv( pid );
	  }
	OUTPUT:
	  RETVAL

SV*
proxy_rem_attach(sv_nid, sv_proxy)
	  SV* sv_nid
	  SV* sv_proxy
	PREINIT:
	  nid_t nid = SvIV( sv_nid );
	  pid_t proxy = SvIV( sv_proxy );
	CODE:
	  { pid_t pid = qnx_proxy_rem_attach( nid, proxy );
		if ( pid == -1 ) qnx_errno = errno;
		RETVAL = newSViv( pid );
	  }
	OUTPUT:
	  RETVAL 

int
proxy_rem_detach(sv_nid, sv_vproxy)
	  SV* sv_nid
	  SV* sv_vproxy
	PREINIT:
	  nid_t nid = SvIV( sv_nid );
	  pid_t vproxy = SvIV( sv_vproxy );
	CODE:
	  { int rv;
		rv = qnx_proxy_rem_detach( nid, vproxy );
		if ( rv == -1 ) qnx_errno = errno;
		RETVAL = rv;
	  }
	OUTPUT:
	  RETVAL

SV*
getnid()
	CODE:
	  RETVAL = newSViv( getnid() );
	OUTPUT:
	  RETVAL


SV*
psinfo1(sv_proc_pid, sv_pid) 
      SV* sv_proc_pid
      SV* sv_pid
    PREINIT:
      unsigned proc_pid = SvIV( sv_proc_pid );
      unsigned pid      = SvIV( sv_pid );
      unsigned retpid   = 0;
      struct _psinfo psdata;
      HV * myhash  = NULL;
      SV * tmpsv   = NULL;
      char * stype = NULL;
      char * tmpstr = NULL;
    CODE:
      retpid = qnx_psinfo( proc_pid, pid, &psdata, 0, 0);
      if( retpid == -1 ) {
		  qnx_errno = errno;
          RETVAL = &PL_sv_undef;
      }
      else {
          myhash = newHV();

          tmpsv  = newSViv( retpid );
          hv_store( myhash, "pid",  3, tmpsv, 0 );

          tmpsv  = newSViv( psdata.priority );
          hv_store( myhash, "priority",  8, tmpsv, 0 );

          switch (psdata.state) {
              case STATE_DEAD:          tmpstr="DEAD"; break;
              case STATE_READY:         tmpstr="READY"; break;
              case STATE_SEND_BLOCKED:  tmpstr="SEND"; break;
              case STATE_RECEIVE_BLOCKED:   tmpstr="RECEIVE"; break;
              case STATE_REPLY_BLOCKED: tmpstr="REPLY"; break;
              case STATE_HELD:          tmpstr="HELD"; break;
              case STATE_SIGNAL_BLOCKED:    tmpstr="SIGNAL"; break;
              case STATE_WAIT_BLOCKED:  tmpstr="WAIT"; break;
              case STATE_SEM_BLOCKED:       tmpstr="SEM"; break;
              default:
                  tmpstr="?"; break;
          }
          tmpsv  = newSVpv( tmpstr, strlen( tmpstr ) );
          hv_store( myhash, "state", 5, tmpsv, 0 );

          if ((psdata.flags & ( _PPF_MID | _PPF_VID )) == 0) {
              tmpsv  = newSVpv( psdata.un.proc.name, strlen( psdata.un.proc.name ));
              hv_store( myhash, "name", 4, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.proc.father );
              hv_store( myhash, "father",  6, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.proc.son );
              hv_store( myhash, "son",     3, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.proc.brother );
              hv_store( myhash, "brother", 7, tmpsv, 0 );

              stype = "process";
          }
          else if (psdata.flags & _PPF_VID) {
              tmpsv  = newSViv( psdata.un.vproc.local_pid );
              hv_store( myhash, "local_pid", 9, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.vproc.remote_pid );
              hv_store( myhash, "remote_pid", 10, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.vproc.remote_vid );
              hv_store( myhash, "remote_vid", 10, tmpsv, 0 );

              tmpsv  = newSViv( psdata.un.vproc.remote_nid );
              hv_store( myhash, "remote_nid", 10, tmpsv, 0 );

              stype = "vc";
          }
          else {
              tmpsv  = newSViv( psdata.un.mproc.count );
              hv_store( myhash, "count",  5, tmpsv, 0 );

              stype = "proxy" ;
          }

          tmpsv = newSVpv( stype, strlen( stype ) );
          hv_store( myhash, "type", 4, tmpsv, 0 );

          # return the hash as a reference

          RETVAL = newRV_noinc( myhash );
      }
    OUTPUT:
      RETVAL 


SV*
vc_attach(sv_nid, sv_pid, sv_length, sv_flags)
      SV* sv_nid
      SV* sv_pid
      SV* sv_length
      SV* sv_flags
    PREINIT:
      unsigned nid    = SvIV( sv_nid );
      unsigned pid    = SvIV( sv_pid );
      unsigned length = SvIV( sv_length );
      unsigned flags  = SvIV( sv_flags );
    CODE:
	  { pid_t pid = qnx_vc_attach( nid, pid, length, flags );
		if ( pid == -1 ) qnx_errno = errno;
		RETVAL = newSViv( pid );
	  }
    OUTPUT:
      RETVAL 


int
vc_detach(sv_vid)
      SV* sv_vid
    PREINIT:
      unsigned vid    = SvIV( sv_vid );
    CODE:
	  { int rv;
		rv = qnx_vc_detach( vid );
		if ( rv == -1 ) qnx_errno = errno;
		RETVAL = rv;
	  }
    OUTPUT:
      RETVAL 


SV*
net_alive()
    PREINIT:
      char* buf = (char *) malloc( 500 * sizeof(char) );
      int   nid;
      int   nbnodes;
      AV *  myarray = NULL;
      SV *  tmpsv   = NULL;
      int   pid;
    CODE:
      pid = qnx_name_locate( 0, "qnx/net", 0, NULL );
      if (pid == -1) {
          RETVAL = &PL_sv_undef;
      }
      else {
          nbnodes = qnx_net_alive( buf, 500 );
          if (nbnodes == -1) {
              RETVAL = &PL_sv_undef;
          }
          else {
              myarray = newAV();
              av_unshift( myarray, nbnodes );
              for (nid=1; nid<=nbnodes; nid++) {
                  tmpsv  = newSViv( buf[nid] ? 1 : 0 );
                  av_store( myarray, nid, tmpsv );
              }
              RETVAL = newRV_noinc( myarray );
          }
      }
      free( buf );
    OUTPUT:
      RETVAL
