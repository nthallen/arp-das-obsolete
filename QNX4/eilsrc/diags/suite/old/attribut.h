/* attribut.h defines the attributes */
extern unsigned char attributes[];
void init_attrs(char *name);
#define CFG_FILE "scdiag.cfg"
#define MAX_ATTRS 10
#define ATTR_MAIN attributes[0]
#define ATTR_LOGO attributes[1]
#define ATTR_NAME attributes[2]
#define ATTR_PASS attributes[3]
#define ATTR_WARN attributes[4]
#define ATTR_FAIL attributes[5]
#define ATTR_HILT attributes[6]
#define ATTR_EXEC attributes[7]
