/* proxies.h defines structures used for the Proxy statement.
 * $Log$
 */
#define MAX_PROXY_POINTS 6
#define MAX_PROXIES 10

typedef struct {
  char *name;
  int n_proxies;
  int proxy_name[MAX_PROXY_POINTS];
  int proxy_index[MAX_PROXY_POINTS];
  int last_time;
  int last_state;
  int first_state;
} proxy;

extern int n_proxies;
extern proxy proxies[MAX_PROXIES];
