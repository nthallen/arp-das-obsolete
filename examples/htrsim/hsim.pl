#! /usr/local/bin/perl
require "box.pl";
require "fld.pl";

&define_attr( "BOX" );
&define_attr( "FIELD" );
&define_attr( "HEADER" );
&define_attr( "LABEL" );
&define_attr( "TITLE" );
&define_attr( "UNITS" );

&tblbegin( "" );
&tbltitle( "Heater Simulation" );
&tblsensor( "Thtr", "Thtr", 6, "C" );
&tblsensor( "Tamb", "Tamb", 6, "C" );
&tblsensor( "I", "I", 6, "A" );
&tblsensor( "R", "R", 6, "Ohms" );
( $row, $col ) = &tblend( 0, 0, 0, 2 );
&tblbegin( "" );
&tblsensor( "Rt", "Rt", 6, "C/W" );
&tblsensor( "Ct", "Ct", 6, "J/C" );
&tblsensor( "MFCtr", "MFCtr", 5, "" );
( $mrow, $col ) = &tblend( 0, $col-1, 0, 2 );
&field( $row, 0, 79, 2, "FIELD", "%TMA:hsimalgo" );
&field( $row+2, 0, 79, 2, "FIELD", "%CLIENT" );
&fldend;
