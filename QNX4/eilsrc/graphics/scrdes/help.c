#include <stdlib.h>
#include <curses.h>
#include "popup.h"
#include "pattr.h"
#include "mnuglobs.h"
#include "etc.h"
#include "help.h"

void help(char *filename) {
char line[60];
if ((popup_file(2,10,filename,HELP_WIN,2,18,60))==0) {
	sprintf(line,"Can't find file %s\nin path or environment",filename);
	ERROR(line);
}
}

void help_menu(int mnu, int c, int draw) {
if (mnu==-1)
	help("scrdes0.hlp");
else if (mnu==genmenu)
	switch (c) {
	case FILES: help("scrdes2.hlp"); break;
	case ATTRIBUTE: help("scrdes3.hlp"); break;
	case DRAW: help("scrdes4.hlp"); break;
	case EXIT: help("scrdes5.hlp"); break;
	}
else if (mnu==attrmenu) help("scrdes3.hlp");
else if (mnu==drawmenu)
	switch (c) {
	case LINE: 
		popup_str(2,8,"Drawing Element: LINE.\nThe region is OUTLINED according to the current option.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case TEXT: 
		popup_str(2,8,"Drawing Element: TEXT.\nEnter text in the region according to the current option.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case FILL: 
		popup_str(2,8,"Drawing Element: FILL.\nThe region is filled according to the current option.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case ERASE:
		popup_str(2,8,"Drawing Element: ERASE.\nErase the foreground, background or attribute\naccording to the current option.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case BUFFER:
		popup_str(2,8,"Drawing Element: BUFFER.\nCapture region into buffer and move/copy it elswhere\naccording to current option.\nInsert the buffer elsewhere by pressing INS when no region exists.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case FIELD: 
		popup_str(2,8,"Drawing Element: FIELD.\nThis exists to interface to Norton's form design system.\nDefine fields and output in the fld format.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	case SCREEN:
		popup_str(2,8,"Drawing Element: SCREEN.\nSet the dimensions and default position of your screen.\nChoosing this element loses previous dimensions.\nPress F6 in the Option Menu for help on options.",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==exitmenu)
	switch (c) {
	case KEYS: help("scrdes1.hlp"); break;
	case SAVE:
		popup_str(2,8,"This item is for output of your screen and/or attribute map.\nPress F6 in the Save Menu for help on output formats.",HELP_WIN,2,-1,0);
	break;
	case PREFERENCE:
		popup_str(2,8,"This item is for setting the attributes\nof the user interface of this program.\nPress F6 in the Preference Menu for help.",HELP_WIN,2,-1,0);
	break;
	case TOGGLESTAT:
		popup_str(2,8,"This item toggles the Status Window on/off.",HELP_WIN,2,-1,0);
	break;
	case HELPKEY: 
		popup_str(2,8,"Havn't you got the hang of it yet?",HELP_WIN,2,-1,0);
	break;
	case ESC: 
		popup_str(2,8,"This item quits the program",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==linemenu)
	switch (draw) {
		case LINE:
			switch (c) {
				case ONE:
					popup_str(2,8,"Option: Single.\nKey: F1.\nThe region is outlined with a single line.",HELP_WIN,2,-1,0);
				break;
				case TWO:
					popup_str(2,8,"Option: Double.\nKey: F2.\nThe region is outlined with a double line.",HELP_WIN,2,-1,0);
				break;
				case THREE:
					popup_str(2,8,"Option: Ascii.\nKey: F3.\nThe region is outlined with the given ascii character.",HELP_WIN,2,-1,0);
				break;
				case FOUR:
					popup_str(2,8,"Option: Color.\nKey: F4.\nThe region is outlined with the current attribute.",HELP_WIN,2,-1,0);
				break;
			}
		break;
		case FILL:
			switch (c) {
				case ONE:
					popup_str(2,8,"Option: Single.\nKey: F1.\nThe region is filled with horizontal/vertical single lines\ndepending on how this option is toggled.",HELP_WIN,2,-1,0);
				break;
				case TWO:
					popup_str(2,8,"Option: Double.\nKey: F2.\nThe region is filled with horizontal/vertial double lines\ndepending on how this option is toggled.",HELP_WIN,2,-1,0);
				break;
				case THREE:
					popup_str(2,8,"Option: Ascii.\nKey: F3.\nThe region is filled with the given ascii character.",HELP_WIN,2,-1,0);
				break;
				case FOUR:
					popup_str(2,8,"Option: Color.\nKey: F4.\nThe region is filled with the current attribute.",HELP_WIN,2,-1,0);
				break;
			}		
		break;
		case SCREEN:
			switch (c) {
				case ONE:
					popup_str(2,8,"Option: Single.\nKey: F1.\nThe screen is outlined with a single line.",HELP_WIN,2,-1,0);
				break;
				case TWO:
					popup_str(2,8,"Option: Double.\nKey: F2.\nThe screen is outlined with a double line.",HELP_WIN,2,-1,0);
				break;
				case THREE:
					popup_str(2,8,"Option: Ascii.\nKey: F3.\nThe screen is outlined with the given ascii character.",HELP_WIN,2,-1,0);
				break;
				case FOUR:
					popup_str(2,8,"Option: Color.\nKey: F4.\nThe screen is outlined with the current attribute.",HELP_WIN,2,-1,0);
				break;
			}		
	}

else if (mnu==textmenu)
	switch (c) {
	case ONE:
		popup_str(2,8,"Option: Left.\nKey: F1.\nText lines ending in CR are left justified.",HELP_WIN,2,-1,0);
	break;
	case TWO:
		popup_str(2,8,"Option: Center.\nKey: F2.\nText lines ending in CR are centered.",HELP_WIN,2,-1,0);
	break;
	case THREE:
		popup_str(2,8,"Option: Right.\nKey: F3.\nText lines ending in CR are right justified.",HELP_WIN,2,-1,0);
	break;	
	case FOUR:
		popup_str(2,8,"Option: Import.\nKey: F4.\nImport text from a file.",HELP_WIN,2,-1,0);
	break;		
	}
else if (mnu==bufmenu)
	switch (c) {
	case ONE:
		popup_str(2,8,"Option: Copy/Overwrite.\nKey: F1.\nA copy of the buffer is overwritten at the new position.",HELP_WIN,2,-1,0);
	break;
	case TWO:
		popup_str(2,8,"Option: Copy/Overlay.\nKey: F2.\nA copy of the buffer is overlaid at the new position,\ni.e. all characters except blanks are copied.",HELP_WIN,2,-1,0);
	break;
	case THREE:
		popup_str(2,8,"Option: Move/Overwrite.\nKey: F3.\nThe buffer is moved to the new position and overwritten.",HELP_WIN,2,-1,0);
	break;
	case FOUR:
		popup_str(2,8,"Option: Move/Overlay.\nKey: F2.\nThe buffer is moved to the new position, and overlaid,\ni.e. all characters except blanks are moved.",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==erasemenu)
	switch (c) {
	case ONE:
		popup_str(2,8,"Option: Foreground.\nKey: F1.\nThe foreground is erased,leaving the background attribute.",HELP_WIN,2,-1,0);
	break;
	case TWO:
		popup_str(2,8,"Option: Background.\nKey: F1.\nThe background is erased,\nor changed to the NORMAL or first attribute.",HELP_WIN,2,-1,0);
	break;
	case THREE:
		popup_str(2,8,"Option: Color.\nKey: F3.\nThe attribute of the region is changed to the current attribute.",HELP_WIN,2,-1,0);
	break;
	case FOUR:
		popup_str(2,8,"Option: All.\nKey: F4.\nThe region erased, leaving only the NORMAL or first attribute.",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==fieldmenu)
	switch (c) {
	case ONE:
		popup_str(2,8,"Option: Create.\nKey: F1.\nCreate a field, a designated area of the screen.",HELP_WIN,2,-1,0);
	break;
	case TWO:
		popup_str(2,8,"Option: Delete.\nKey: F1.\nDelete field(s).",HELP_WIN,2,-1,0);
	break;
	case THREE:
		popup_str(2,8,"Option: Move.\nKey: F3.\nMove the position of a field.",HELP_WIN,2,-1,0);
	break;
	case FOUR:
		popup_str(2,8,"Option: Info.\nKey: F4.\nInformation on field(s).",HELP_WIN,2,-1,0);
	break;
	}
/* fixed to here */
else if (mnu==prefmenu)
	switch (c) {
	case CHOICE1:
		popup_str(2,8,"Instates default color attributes for a color monitor",HELP_WIN,2,-1,0);
	break;
	case CHOICE2:
		popup_str(2,8,"Instates default mono attributes for a monochrome monitor",HELP_WIN,2,-1,0);
	break;
	case CHOICE3:
		popup_str(2,8,"Set preffered attributes for a session or default.\nPress F6 after selecting this item for more help.",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==savemenu)
	switch (c) {
	case CHOICE1:
		popup_str(2,8,"Saves screen definition as a row/col/attributetype bitmap,\nthus field definition are not saved.",HELP_WIN,2,-1,0);
	break;
	case CHOICE2:
		popup_str(2,8,"Saves attribute type/actual map.",HELP_WIN,2,-1,0);
	break;
	case CHOICE4:
		popup_str(2,8,"Saves screen as an ascii definition file,\nfield definitions saved.",HELP_WIN,2,-1,0);
	break;
	case CHOICE5:
		popup_str(2,8,"Saves screen as an ascii file,\nno field or attribute definitions saved.",HELP_WIN,2,-1,0);
	break;	
	}
else if (mnu==fieldmenu)
	switch(c) {
	case CHOICE1:
		popup_str(2,8,"Selecting this option while cursor is in a field,\nwill report the field number.",HELP_WIN,2,-1,0);
	break;
	case CHOICE2:
		popup_str(2,8,"Selecting this option while cursor is in a field,\nwill erase the field definition, but not it's appearance.",HELP_WIN,2,-1,0);
	break;
	}
else if (mnu==pattrmenu) help("scrdes6.hlp");
else ERROR("bugger off");
}

