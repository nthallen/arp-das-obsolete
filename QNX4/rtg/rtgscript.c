/* rtgscript.c is the rtg-specific portion of script.c
  create each channel and define it's properties
  create each window and define it's properties {
	create each axis and define it's properties
	create each graph and define it's properties
  }
  
  We will do a couple custom things.
*/
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"

void BaseWins_report(void) {
  BaseWin *bw;
  
  for (bw = BaseWins; bw != 0; bw = bw->next ) {
	if (bw->pict_id != 0) {
	  assert( bw->title != 0 );
	  script_word( "CW" );
	  script_word( bw->title );
	  script_word( NULL );
	  Basewin_record( bw );
	  PropsOutput_( bw->title, "WP" );
	  /* Output definition of each axis */
	  /* Output definition of each graph */
	  script_word( "EW" );
	  script_word( NULL );
	}
  }
}

static chantype *chantypes[] = {
  &ss_chan_type,
  &cdb_type,
  &dummy_type,
  NULL
};

static void chanprop_report( RtgChanNode *CN ) {
  chandef *channel;
  
  assert( CN != 0 );
  channel = CN->u.leaf.channel;
  PropsOutput_( channel->name, "CP" );
}

/* This is the rtg-specific routine to dump the current program state
   to the script file. The script file has already been opened by
   script_create().
*/
void script_dump(void) {
  int i;
  
  /* First output the channel definitions */
  for (i = 0; chantypes[i] != 0; i++) {
	if (chantypes[i]->channels_report != 0)
	  chantypes[i]->channels_report();
  }
  CTreeVisit( CT_CHANNEL, chanprop_report );
  
  /* Report configuration of each base window */
  BaseWins_report();
}

/* return non-zero on serious error (script should be aborted) */
static int cmd_CC( const char *filename ) {
  int ct;

  if ( script_argc < 3 ) {
	nl_error( 2, "Insufficient arguments for CC in script file %s",
								filename );
	return 1;
  }
  /* search list of channel types for abbr matching argv[1] */
  for ( ct = 0; chantypes[ ct ] != 0; ct++ ) {
	if ( strcmp( chantypes[ ct ]->abbr,
				 script_argv[ 1 ] ) == 0 ) break;
  }
  if ( chantypes[ ct ] != 0 ) {
	if ( chantypes[ ct ]->channel_create != 0 )
	  chantypes[ ct ]->channel_create( script_argv[ 2 ] );
	return 0;
  } else {
	nl_error( 2, "Unknown channel type %s for CC in script file %s", 
			  script_argv[1], filename );
	return 1;
  }
}

static int min_args( int n_args ) {
  if ( script_argc < n_args ) {
	nl_error( 2, "Insufficient arguments for script command %s", 
			  script_argv[0] );
	return 1;
  }
  return 0;
}

/* script_cmd returns non-zero on a serious error requiring early abort */
int script_cmd( const char *filename ) {
  static unsigned int swallow = 0;
  static char plabel[8] = ""; /* currently open properties */

  if ( script_argc > 0 ) {
	unsigned char *cmd = script_argv[ 0 ];
	unsigned int cmdcode;

	cmdcode = (cmd[0] << 8) + cmd[1];
	if ( swallow != 0 ) {
	  if ( swallow == cmdcode )
		swallow = 0;
	  return 0;
	}
	switch ( cmdcode ) {
	  case 'CC': /* "Create Channel" */
		return cmd_CC( filename );
	  case 'CW': /* "Create Window" */
		if ( New_Base_Window( script_argv[1] ) )
		  swallow = 'EW';
		return 0;
	  case 'EW': /* "End of Window Definition" */
		return 0;
	  case 'PO': /* "Properties Open" */
		if ( min_args( 3 ) ) return 1;
		if ( strcmp( script_argv[1], "AP" ) != 0 ) {
		  if ( Properties_( script_argv[2], script_argv[1], 0 ) )
			swallow = 'PA'; /* swallow to the end of the properties */
		  else strcpy( plabel, script_argv[1] );
		}
		return 0;
	  case 'PC': /* "Property Change" */
		if ( min_args( 3 ) ) return 1;
		PropChange_( plabel, script_argv[1], script_argv[2]);
		return 0;
	  case 'PA': /* "Properties Apply" */
		PropsApply_( plabel );
		plabel[0] = '\0';
		return 0;
	  default:
		break;
	}
	nl_error( 2, "Unrecognized command %s in script file %s",
	  cmd, filename );
  }
  return 0;
}
