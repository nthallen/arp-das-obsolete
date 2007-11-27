%{
  /* Copied in from trial.cmd */
%}
%INTERFACE <foo>
%INTERFACE <tm>

&start
	: &commands Quit * {}
	: &commands &&Exit
	;
&&Exit
	: Exit * { cgc_forwarding = 0; }
	;
&commands
	:
	: &commands &command
	;
&command
	: *
	: Telemetry &tm_cmd
	: Log %s ( Enter String to Log to Memo ) * {}
	: foo %s ( Enter String to send to foo ) * {
	    cis_turf(if_foo, "foo_custom %s\n", $2 );
	  }
	;
&tm_cmd
	: Start * { cis_turf( if_tm, "TM Start\n" ); }
	: Logging Suspend * { cis_turf( if_tm, "TM Logging Suspend\n" ); }
	: Logging Resume * { cis_turf( if_tm, "TM Logging Resume\n" ); }
	;

%INTERFACE <bar>
&command
	: bar %s ( Enter String to send to bar ) * {
	    cis_turf(if_bar, "bar_custom %s\n", $2 );
	  }
	;

