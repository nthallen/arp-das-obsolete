/* nvram.c A Program to verify and/or initialize a Non-Volatile RAM
   Directory Structure.
   $Log$
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/kernel.h>
#include "nortlib.h"
#include "subbus.h"
#include "nvram.h"
static char rcsid[] = "$Id$";
#define MAX_HEX_COLS 20

static unsigned char pattern(unsigned short addr) {
  return(addr & 0xFF);
}

/* returns TRUE if NVRAM from 'from' to NVRAM_SIZE-1 is properly
   patterned. */
static int check_pattern(unsigned short from) {
  for (; from < NVRAM_SIZE; from++)
	if (read_novram(from) != pattern(from)) return(0);
  return(1);
}

static void fill_pattern(unsigned short from) {
  for (; from < NVRAM_SIZE; from++)
	write_novram(from, pattern(from));
}

/* return TRUE if an existing directory is in place at least as
   large as n_entries */
int existing_dir(unsigned short n_entries) {
  struct nvram_dir_entry dirent;
  unsigned short first_free, first_pat, n_dir_entries;
  int ent_no;

  /* Check Existing Directory Structure */
  read_dir(0, &dirent);
  
  /* Do we have an existing directory structure? */
  if ((dirent.ID == 0) ||
	  (dirent.start == 0) ||
	  (dirent.start >= NVRAM_SIZE) ||
	  (dirent.end < dirent.start) ||
	  (dirent.end >= NVRAM_SIZE) ||
	  (dirent.start % sizeof(struct nvram_dir_entry)) != 0)
	return(0);

  /* Is it big enough? */
  n_dir_entries = dirent.start/sizeof(struct nvram_dir_entry);
  if (n_dir_entries < n_entries) return(0);
  for (ent_no = 1; ent_no < n_dir_entries; ent_no++) {
	first_free = dirent.end + 1;
	read_dir(ent_no, &dirent);
	if (dirent.ID == 0) break;
	if (dirent.start != first_free ||
		dirent.end < dirent.start ||
		dirent.end >= NVRAM_SIZE)
	  return(0);
  }
  
  /* was the last entry partially entered? */
  first_pat = first_free;
  if (ent_no < n_dir_entries &&
	  dirent.start == first_free &&
	  dirent.end >= dirent.start &&
	  dirent.end < NVRAM_SIZE)
	first_pat = dirent.end + 1;

  /* Check that the remainder of memory is patterened */
  if (!check_pattern(first_pat)) return(0);
  if (first_pat != first_free)
	fill_pattern(first_free);
  if (ent_no < n_dir_entries) {
	while (ent_no < n_dir_entries)
	  init_entry(ent_no++, 0, 0, 0);
  }
  return(1);
}

static void initialize_ram(unsigned short n_entries) {
  unsigned short first_free;
  int i;
 
  printf("NVRAM: Initializing NVRAM Directory\n");
  first_free = n_entries * sizeof(struct nvram_dir_entry);
  init_entry(0, 0, first_free, 0);
  for (i = 1; i < n_entries; i++)
	init_entry(i, 0, 0, 0);
  fill_pattern(first_free);
}

/* Prints the contents of NVRAM */
static void dumpit(void) {
  struct nvram_dir_entry dirent;
  unsigned short n_entries, ent_no, col;

  read_dir(0, &dirent);
  n_entries = dirent.start / sizeof(dirent);
  for (ent_no = 0; ent_no < n_entries; ent_no++) {
	read_dir(ent_no, &dirent);
	if (dirent.ID == 0) break;
	printf("NVRAM ID: %3d Start: %4d End: %4d\n", dirent.ID,
			dirent.start, dirent.end);
	for (col = 0; dirent.start <= dirent.end; dirent.start++, col++) {
	  if (col == MAX_HEX_COLS) {
		putchar('\n');
		col = 0;
	  }
	  printf(" %02X", read_novram(dirent.start));
	}
	putchar('\n');
  }
}

#ifdef __USAGE
%C	[-n <number of directory entries>] [-i] [-v]
	Initialize Non-Volatile RAM directory structure or validate
	an existing directory structure.
	-n <m> Specify the minimum number of directory entries [10]
	-i     Force reinitialization, regardless of NVRAM contents
	-v     If directory structure exists, dump it's contents
#endif

void main(int argc, char **argv) {
  unsigned char n_dir_entries = 10, chk_ram = 1, dump_ram = 0;
  unsigned char subbus_msg = SBMSG_NVR_INIT;
  int c;

  optind = 0;
  opterr = 0;
  do {
	c = getopt(argc, argv, "n:iv");
	switch (c) {
	  case 'n': n_dir_entries = atoi(optarg); break;
	  case 'i': chk_ram = 0; break;
	  case 'v': dump_ram = 1; break;
	  case '?':
		nl_error(3, "Unknown option -%c", optopt);
	}
  } while (c != -1);
  
  if (load_subbus() == 0)
	nl_error(3, "Subbus Not Resident");
  if (!(subbus_features & SBF_LG_RAM))
	nl_error(3, "Resident Subbus does not support large NVRAM");
  if (chk_ram && existing_dir(n_dir_entries)) {
	if (dump_ram) dumpit();
  } else initialize_ram(n_dir_entries);
  if (Send(sb_pid, &subbus_msg, NULL, 1, 0) != 0)
	nl_error(3, "Error sending to subbus");
}
