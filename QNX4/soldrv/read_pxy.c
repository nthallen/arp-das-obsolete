/* read_pxy.c contains read_proxy().
 * $Log$
 */
#include <string.h>
#include <ctype.h>
#include "tokens.h"
#include "proxies.h"
#include "nortlib.h" /* for nl_error */

int n_proxies = 0, n_prxy_pts = 0;
proxy proxies[MAX_PROXIES];
int prxy_pts[MAX_PRXY_PTS];

void read_proxy(void) {
  int token, i, j;
  
  if (n_proxies == MAX_PROXIES) filerr("Too Many proxies Defined\n");
  token = get_token();
  if (token == TK_PROXY_NAME) filerr("Attempted Redefinition of Proxy\n");
  if (token != TK_UNDEFINED_STRING) filerr("Proxy Name Error\n");
  proxies[n_proxies].name = strdup(gt_input);
  if (get_token() != TK_LBRACE) filerr("Proxy Definition needs '{'\n");
  for (i = 0; i < MAX_PROXY_POINTS; i++) {
	while (isspace(token = gt_getc()));
    if (token == '}') {
      gt_ungetc(token);
      break;
    } else proxies[n_proxies].proxy_name[i] = token;
    if (get_token() != TK_COLON) filerr("Proxy Definition needs ':'\n");
    if (get_token() != TK_NUMBER) filerr("Proxy Value Missing\n");
	if (n_prxy_pts >= MAX_PRXY_PTS) filerr("Too may proxy points\n");
	if (gt_number < 0 || gt_number > 255)
	  filerr("Invalid proxy ID %d\n", gt_number);
	for (j = 0; j < n_prxy_pts; j++)
	  if (prxy_pts[j] == gt_number) {
		nl_error(1, "Reuse of Proxy ID %d", gt_number);
		break;
	  }
	if (j == n_prxy_pts) {
	  prxy_pts[n_prxy_pts] = gt_number;
	  n_prxy_pts++;
	}
	proxies[n_proxies].proxy_index[i] = j;
  }
  proxies[n_proxies].n_proxies = i;
  if (get_token() != TK_RBRACE) filerr("Proxy Definition needs '}'\n");
  n_proxies++;
}
