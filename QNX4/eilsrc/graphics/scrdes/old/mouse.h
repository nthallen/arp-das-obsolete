/* mouse.h defines some routines for accessing the mouse.
   Begun July 6, 1990
*/
typedef struct {
  unsigned int mask;
  unsigned int button_status;
  unsigned int h_position;
  unsigned int v_position;
  unsigned int h_counts;
  unsigned int v_counts;
} mouse_event;

int init_mouse(void);
void show_mouse(void);
void hide_mouse(void);
void def_mouse_event(unsigned int mask);
int get_mouse_event(mouse_event *);
int mouse_hit(mouse_event *me);
int get_mouse_hit(mouse_event *me);
void get_mouse_position(int *x, int *y);
