# edf2ext.awk Converts .edf files to .ext for TMC input.
# $Log$
# Revision 1.1  1993/05/28  20:06:11  nort
# Initial revision
#
# spreadsheet deleteme 6
#  1 O3Ref %6.0lf Ct24_Double
#
/^ *spreadsheet/ {
  if (written == 1) nsps++
  else {
	print "%{ /* edf2ext.awk reading " FILENAME " */"
	print "  #include <stdlib.h>"
	print "  #include \"ssp.h\""
	print "  #include \"tmctime.h\""
	printf "\n"
	print "  #define Ct24_Long(x) (0xFFFFFF & *(TMLONG *)&x)"
	print "  #define Ct24_Double(x) (double)Ct24_Long(x)"
	print "  #define To_Double(x) (double)(x)"
	print "  #define EXTRACTION_INIT initialize()"
	print "  #define EXTRACTION_TERM terminate()"
	print "  #define ALL_SPSS"
	printf "\n"
	print "  static double ext_delta = 0.;"
	printf "\n"
	written = 1;
	nsps = 0;
  }
  sps[nsps] = $2
  ncols[nsps] = $3
  if (NF > 3 && $4 == "separate") sep[nsps] = "y"
}
/^[ \t]*[0-9]/ {
  datum[nsps,$1] = $2
  if (NF >= 3) datfmt[nsps,$1] = $3
  else datfmt[nsps,$1] = "%9.2e"
  if (NF >= 4) datcnv[nsps,$1] = $4
  else datcnv[nsps,$1] = "convert"
}
/init_only/ { init_only = "yes" }
END {
  # print the spreadsheet declarations
  for (i = 0; i <= nsps; i++)
    print "  sps_ptr " sps[i] ";"

  # print the initializations
  print "  void initialize(void) {"
  print "\t{"
  print "\t  char *s;"
  print "\t  s = getenv(\"EXT_DELTA\");"
  print "\t  if (s != NULL) {"
  print "\t\text_delta=atof(s);"
  print "\t\tmsg(MSG, \"Using Time Delta of %lf\", ext_delta);"
  print "\t  }"
  print "\t}"
  for (i = 0; i <= nsps; i++) {
	print "\t" sps[i] " = ss_create(\"" sps[i] "\", 1, " ncols[i] ", 1);"
	print "\tif (" sps[i] " < 0)"
	print "\t  msg(MSG_EXIT_ABNORM, \"Unable to open spreadsheet " sps[i] "\");"
	print "\tss_set_column(" sps[i] ", 0, \"%14.11lt\", \"Time\");"
	for (j = 1; j < ncols[i]; j++) {
	  if (datfmt[i,j] == "") datfmt[i,j] = "%9.2e"
	  printf "\tss_set_column(" sps[i] ", " j ", "
	  print "\"" datfmt[i,j] "\", \"" datum[i,j] "\");"
	}
  }
  print "  }"
  
  # print the terminations
  print "  void terminate(void) {"
  for (i = 0; i <= nsps; i++) {
	print "\tss_close(" sps[i] ");"
  }
  print "  }"
  print "%}"

  # print the extraction statements
  if (init_only != "yes") {
	for (i = 0; i <= nsps; i++) {
	  k = 0;
	  for (j = 1; j < ncols[i]; j++) {
		if (datum[i,j] != "") {
		  if (k > 0 && sep[nsps] == "y") print "}"
		  if (k == 0 || seq[nsps] == "y") {
			print "{"
			print "  ss_insert_value(" sps[i] ", dtime()+ext_delta, 0);"
		  }
		  printf "  ss_set(" sps[i] ", " j ", "
		  print datcnv[i,j] "(", datum[i,j] "));"
		  k++;
		}
	  }
	  print "}"
	}
  }
}
