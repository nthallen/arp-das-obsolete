#ifdef __QNX__
/* returns the privity level */
int get_priv(void);

#define PRIV_SPECIAL 1
#define PRIV_REGULAR 3
#endif
