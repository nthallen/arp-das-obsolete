#include <sys/qnxterm.h>
main() {
	term_load();
	term_clear(TERM_CLS_SCRH);
}