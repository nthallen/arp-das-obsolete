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

/* Output definition of each axis */
static void Axes_report( RtgAxis *axes ) {
  for ( ; axes != 0; axes = axes->next ) {
	script_word( "CA" );
	script_word( axes->is_y_axis ? "Y" : "X" );
	script_word( axes->opt.ctname );
	script_word( NULL );
	PropsOutput( axes->opt.ctname, axes->is_y_axis ? "YP" : "XP" );
	script_word( "EA" );
	script_word( NULL );
  }
}

/* Output definition of each graph
  CG name channel x_axis_name y_axis_name
*/
static void Graphs_report( RtgGraph *graphs ) {
  for ( ; graphs != 0; graphs = graphs->next ) {
	script_word( "CG" );
	script_word( graphs->name );
	script_word( graphs->position->channel->name );
	script_word( graphs->X_Axis->opt.ctname );
	script_word( graphs->Y_Axis->opt.ctname );
	script_word( NULL );
	PropsOutput( graphs->name, "GP" );
	script_word( "EG" );
	script_word( NULL );
  }
}

static void BaseWins_report(void) {
  BaseWin *bw;
  
  for (bw = BaseWins; bw != 0; bw = bw->next ) {
	if (bw->pict_id != 0) {
	  assert( bw->title != 0 );
	  script_word( "CW" );
	  script_word( bw->title );
	  script_word( NULL );
	  Basewin_record( bw );
	  PropsOutput( bw->title, "WP" );

	  Axes_report( bw->x_axes );
	  Axes_report( bw->y_axes );
	  Graphs_report( bw->graphs );

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
  PropsOutput( channel->name, "CP" );
}

/* This is the rtg-specific routine to dump the current program state
   to the script file. The script file has already been opened by
   script_create().
*/
void script_dump(void) {
  int i;

  /* Output the global definitions */
  PropsOutput( "", "RP" );

  /* First output the channel definitions */
  for (i = 0; chantypes[i] != 0; i++) {
	if (chantypes[i]->channels_report != 0)
	  chantypes[i]->channels_report();
  }
  CTreeVisit( CT_CHANNEL, chanprop_report );
  
  /* Report configuration of each base window */
  BaseWins_report();
}

/* Create an axis in the specified window. Return non-zero on error
      CA [XY] name
   Only error is if the axis name already exists
*/
static int cmd_CA( const char *bwname ) {
  BaseWin *bw;
  RtgChanNode *CN;
  int is_y;
  
  assert( bwname != 0 );
  CN = ChanTree( CT_FIND, CT_WINDOW, bwname );
  assert( CN != 0 && CN->u.leaf.bw != 0 );
  bw = CN->u.leaf.bw;
  is_y = ( script_argv[1][0] == 'Y' );
  return ( axis_create( bw, script_argv[2], NULL, is_y ) == 0 );
}

/* Create a graph
  CG name channel x_axis_name y_axis_name
  return non-zero if the graph couldn't be created
*/
static int cmd_CG( const char *bwname, const char *filename ) {
  RtgChanNode *CN;
  BaseWin *bw;
  RtgAxis *x_ax, *y_ax;
  chandef *cc;

  /* Get Basewindow */
  assert( bwname != 0 );
  CN = ChanTree( CT_FIND, CT_WINDOW, bwname );
  assert( CN != 0 && CN->u.leaf.bw != 0 );
  bw = CN->u.leaf.bw;
  
  /* Get Channel Definition */
  CN = ChanTree( CT_FIND, CT_CHANNEL, script_argv[2] );
  if ( CN == 0 || CN->u.leaf.channel == 0 ) {
	nl_error( 2, "Channel %s not found for graph %s, script %s",
	  script_argv[2], script_argv[1], filename );
	return 1;
  }
  cc = CN->u.leaf.channel;

  /* Get X Axis Definition */
  if ( script_argv[3][0] == '\0' ) x_ax = NULL;
  else {
	CN = ChanTree( CT_FIND, CT_AXIS, script_argv[3] );
	if ( CN == 0 || CN->u.leaf.axis == 0 ) {
	  nl_error( 2, "Axis %s not found for graph %s, script %s",
		script_argv[3], script_argv[1], filename );
	  return 1;
	}
	x_ax = CN->u.leaf.axis;
  }

  /* Get Y Axis Definition */
  if ( script_argv[4][0] == '\0' ) y_ax = NULL;
  else {
	CN = ChanTree( CT_FIND, CT_AXIS, script_argv[4] );
	if ( CN == 0 || CN->u.leaf.axis == 0 ) {
	  nl_error( 2, "Axis %s not found for graph %s, script %s",
		script_argv[4], script_argv[1], filename );
	  return 1;
	}
	y_ax = CN->u.leaf.axis;
  }

  return graph_crt( bw, script_argv[1], cc, x_ax, y_ax );
}

/* return non-zero on serious error (script should be aborted) */
static int cmd_CC( const char *filename ) {
  int ct;

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
  static dastring bwname = NULL; /* currently open basewindow */

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
	  case 'CA': /* "Create Axis" */
		if ( min_args( 3 ) ) return 1;
		if ( cmd_CA( bwname ) )
		  swallow = 'EA';
		return 0;
	  case 'CG': /* "Create Graph" */
		if ( min_args( 5 ) ) return 1;
		if ( cmd_CG( bwname, filename ) )
		  swallow = 'EG';
		return 0;
	  case 'CC': /* "Create Channel" */
		if ( min_args( 3 ) ) return 1;
		return cmd_CC( filename );
	  case 'CW': /* "Create Window" */
		if ( New_Base_Window( script_argv[1] ) )
		  swallow = 'EW';
		else bwname = dastring_init( script_argv[1] );
		return 0;
	  case 'EA': /* "End of Axis Definition" */
	  case 'EG': /* "End of Graph Definition" */
		return 0;
	  case 'EW': /* "End of Window Definition" */
		dastring_update( &bwname, NULL );
		return 0;
	  case 'PO': /* "Properties Open" */
		if ( min_args( 3 ) ) return 1;
		if ( Properties( script_argv[2], script_argv[1], 0 ) )
		  swallow = 'PA'; /* swallow to the end of the properties */
		else strcpy( plabel, script_argv[1] );
		return 0;
	  case 'PC': /* "Property Change" */
		if ( min_args( 3 ) ) return 1;
		PropChange( plabel, script_argv[1], script_argv[2]);
		return 0;
	  case 'PA': /* "Properties Apply" */
		PropsApply( plabel );
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
