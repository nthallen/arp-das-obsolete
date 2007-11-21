#define SCRMAGIC 3

/* input .scr file to a window, returns number of attributes */
/* negative on posx and posy will default */
extern int scr_win_in(char *filename, WINDOW *win, char **attrtypes, int posy, int posx, struct hdr *shdr);

/* output .scr file from a window, returns success status */
int scr_win_out(char *filename, WINDOW *win, char **attrtypes, int nattrs, int posy, int posx, int lines, int cols);
