/* read_mode.c reads in the mode commands.
   Written March 24, 1987
   Modified July 1991 for QNX.
   Modified 4/17/92 for QNX 4.
*/
#include <stdio.h>
#include <malloc.h>
#include "tokens.h"
#include "modes.h"
#include "solenoid.h"
#include "routines.h"
#include "dtoa.h"
#include "solfmt.h"

void add_change(change *nc, int mn);

mode modes[MAX_MODES];
int n_solenoids = 0;
int n_dtoas = 0;
solenoid solenoids[MAX_SOLENOIDS];
dtoa dtoas[MAX_DTOAS];
int res_num = 1, res_den = 1;

void init_modes(void) {
  int i;

  for (i = 0; i < MAX_MODES; i++) {
    modes[i].init = modes[i].first = modes[i].last = NULL;
    modes[i].next_mode = -1;
  }
}

#define new_change() (change *)malloc(sizeof(change))

void read_mode(void) {
  int mn, i, sn, time, token, in_routine, flin;
  long int fpos;
  change *nc;

  in_routine = 0;
  if (get_token() != TK_NUMBER) filerr("Mode requires number\n");
  mn = gt_number;
  if (mn < 0 || mn > MAX_MODES) filerr("Mode number out of range\n");
  if (modes[mn].init != NULL || modes[mn].first != NULL ||
        modes[mn].next_mode >= 0)
    filerr("Attempted Redefinition of mode %d\n", mn);
  if (get_token() != TK_LBRACE) filerr("Mode requires Left Brace\n");
  for (i = 0; i < n_solenoids; i++) solenoids[i].last_time =
    solenoids[i].last_state = solenoids[i].first_state = -1;
  for (i = 0; i < n_dtoas; i++)
    dtoas[i].last_time = dtoas[i].last_state = dtoas[i].first_state = -1;
  for (;;) {
    token = get_token();
    switch (token) {
      case TK_INITIALIZE:
        token = get_token();
        if ((token != TK_SOLENOID_NAME) && (token != TK_DTOA_NAME))
          filerr("Initialize requires solenoid or DtoA name\n");
        sn = gt_number;
        if (get_token() != TK_COLON) filerr("Initialize <name>: - ':' needed\n");
        if ((i = get_change_code(token, sn)) < 0)
          filerr("Initialize <name>:<valving char> - No <valving char>\n");
        nc = new_change();
        nc->type = token;
        nc->t_index = sn;
        nc->state = i;
        nc->next = modes[mn].init;
        modes[mn].init = nc;
        continue;
      case TK_SOLENOID_NAME:
      case TK_DTOA_NAME:
        sn = gt_number;
        if (get_token() != TK_COLON) filerr("<name>: - Missing the ':'\n");
        if (token == TK_SOLENOID_NAME) time = solenoids[sn].last_time;
        else time = dtoas[sn].last_time;
        for (i = get_change_code(token, sn); ;i = get_change_code(token, sn)) {
          if (i == -1) break;
          if (time == -1) time = 0;
          else if (i == MODE_SWITCH_OK) ;
          else time++;
          if ((i != MODE_SWITCH_OK) &&
              (((token == TK_DTOA_NAME) && (i == dtoas[sn].last_state)) ||
             ((token == TK_SOLENOID_NAME) && (i == solenoids[sn].last_state))))
            continue;
          nc = new_change();
          nc->type = token;
          nc->state = i;
          if (i == MODE_SWITCH_OK) {
            nc->t_index = 0;
            nc->time = time+1;
          } else {
            nc->t_index = sn;
            nc->time = time;
            if (token == TK_SOLENOID_NAME) {
              if (solenoids[sn].first_state == -1)
                solenoids[sn].first_state = i;
              solenoids[sn].last_state = i;
            } else {
              if (dtoas[sn].first_state == -1) dtoas[sn].first_state = i;
              dtoas[sn].last_state = i;
            }
          }
          add_change(nc, mn);
        }
        if (token == TK_SOLENOID_NAME) solenoids[sn].last_time = time;
        else dtoas[sn].last_time = time;
        continue;
      case TK_ROUTINE_NAME:
        if (in_routine) filerr("Nested routines are not supported\n");
        fpos = gt_fpos(&flin);
        gt_spos(routines[gt_number].fpos, routines[gt_number].flin);
        in_routine = 1;
        continue;
      case TK_SELECT:
        if (in_routine) filerr("Select within routine is illegal\n");
        if (get_token() != TK_NUMBER) filerr("Select expects mode number\n");
        modes[mn].next_mode = gt_number;
        if (get_token() != TK_RBRACE)
          filerr("Select must be last command in mode\n");
        break;
      case TK_RBRACE:
        if (in_routine) {
          gt_spos(fpos, flin);
          in_routine = 0;
          continue;
        } else break;
      default: filerr("Unexpected token in read_mode\n");
    }
    break; /* only for RBRACE */
  }
  time = -1;
  for (i = 0; i < n_solenoids; i++)
    if (solenoids[i].last_time != -1)
      if (time == -1) time = solenoids[i].last_time;
      else if (solenoids[i].last_time != time)
        filerr("Mode %d has ambiguous cycle lengths (sol. %s)\n", mn,
                solenoids[i].name);
  for (i = 0; i < n_dtoas; i++)
    if (dtoas[i].last_time != -1)
      if (time == -1) time = dtoas[i].last_time;
      else if (dtoas[i].last_time != time)
        filerr("Mode %d has ambiguous cycle lengths (dtoa %s)\n", mn,
                dtoas[i].name);
  modes[mn].length = time+1;    /* length of the cycle */
  modes[mn].res_num = res_num;
  modes[mn].res_den = res_den;
  optimize(mn);
}

void add_change(change *nc, int mn) {
  change *oc;

  if (modes[mn].first == NULL) {
    modes[mn].first = modes[mn].last = nc;
    nc->next = NULL;            /*  First change        */
  } else if (nc->time < modes[mn].first->time) {  /* chg prior to first chg */
    nc->next = modes[mn].first;
    modes[mn].first = nc;
  } else if (nc->state == MODE_SWITCH_OK) {
    oc = modes[mn].first;
    if (oc->time == nc->time) {
      if (oc->state == MODE_SWITCH_OK) free(nc);
      else {
        nc->next = modes[mn].first;
        modes[mn].first = nc;
      }
    } else {
      for ( ; oc->next != NULL && oc->next->time < nc->time;
           oc = oc->next);
      if (oc->next == NULL || oc->next->time > nc->time ||
          oc->next->state != MODE_SWITCH_OK) {
        nc->next = oc->next;
        oc->next = nc;
        if (nc->next == NULL) modes[mn].last = nc;
      } else free(nc);
    }
  } else {
    for (oc = modes[mn].first;
      oc->next != NULL && oc->next->time <= nc->time;
      oc = oc->next);
    nc->next = oc->next;
    oc->next = nc;
    if (nc->next == NULL) modes[mn].last = nc;
  }
}
