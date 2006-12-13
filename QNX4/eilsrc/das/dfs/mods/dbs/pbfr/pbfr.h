#define MAX_STAR_CLIENTS 12
#define DEF_BFR_SIZE 5120

/* should always be less than SHRT_MAX */
#define MAX_BFR_SIZE 32000
/*
integrity check: verify each registered client for existance
when the total number of stamps and cmds being held in the
buffer for guarentee of delivery, reaches this level.
*/
#define CHECK_CLIENTS 50
