/* proxies.h defines structures used for the Proxy statement.
 * $Log$
 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
 */
#define MAX_PROXY_POINTS 6
#define MAX_PROXIES 10
#define MAX_PRXY_PTS 15

typedef struct {
  char *name;
  int n_proxies;
  int proxy_name[MAX_PROXY_POINTS];
  int proxy_index[MAX_PROXY_POINTS];
  int last_time;
  int last_state;
  int first_state;
} proxy;

extern int n_proxies, n_prxy_pts;
extern proxy proxies[MAX_PROXIES];
extern int prxy_pts[MAX_PRXY_PTS];
