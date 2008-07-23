/* sb_ram.c
 * $Log$
 * Revision 1.1  1993/04/21  18:20:53  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include "subbus.h"
#include "sb_internal.h"
#include "nvram.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/*  NVRAM Structure {
	  Directory Entries
	  Data Areas
	  Unused Memory
    }
	Directory {
	  Some fixed number of entries
	  Unused entries are all zero
	  First entry points to memory immediately following dir,
	  and subsequent entries are contiguous.
	  struct nvram_dir_entry {
		unsigned char ID;
		unsigned short start;
		unsigned short end;
	  };
	  Since start and end require 2 bytes, partial update is
	  possible, therefore entry creation must be handled carefully.
	  The addresses are written first, end then start, then the
	  data area is zeroed, then the ID is written. Upon initialization,
	  an entry with a zero ID and non-zero addresses is partially
	  initialized. If the addresses make sense (start is contiguous
	  with previous entry, end is greater than or equal to start
	  and less than the limits of the memory) then the designated
	  region need not be tested for pattern in order to verify
	  the integrity of the memory, since the region was probably
	  partial zeroed. If the addresses don't make sense, the
	  entry is cleared, but otherwise the memory is assumed
	  valid.
	}
*/

static unsigned short n_dir_entries;

void nvram_init(void) {
  struct nvram_dir_entry dirent;

  read_dir(0, &dirent);
  if ((dirent.start % sizeof(struct nvram_dir_entry)) != 0) {
	n_dir_entries = 0;
	fprintf(stderr, "SUBBUS: Invalid Directory Structure\n");
  } else n_dir_entries = dirent.start/sizeof(struct nvram_dir_entry);
}

void nvram_dir(pid_t who, unsigned char id, unsigned short n_bytes) {
  struct nvram_reply rep;
  unsigned short first_free, size;
  int i;

  if (n_dir_entries != 0) {
	rep.code = NVRR_NEW_ENTRY;
	first_free = n_dir_entries * sizeof(struct nvram_dir_entry);
	for (i = 0; i < n_dir_entries; i++) {
	  read_dir(i, &rep.dir);
	  if (rep.dir.ID == 0) break;
	  if (rep.dir.ID == id) {
		size = rep.dir.end - rep.dir.start + 1;
		if (size != n_bytes) rep.code = NVRR_MISMATCH;
		else rep.code = NVRR_OLD_ENTRY;
		break;
	  }
	  first_free = rep.dir.end + 1;
	}
	if (i == n_dir_entries) rep.code = NVRR_NO_DIR_SPACE;
	else if (rep.code == NVRR_NEW_ENTRY) {
	  rep.dir.start = first_free;
	  rep.dir.end = first_free + n_bytes - 1;
	  if (rep.dir.end < rep.dir.start || rep.dir.end >= NVRAM_SIZE)
		rep.code = NVRR_NO_SPACE;
	  else {
		rep.dir.ID = id;
		init_entry(i, rep.dir.ID, rep.dir.start, rep.dir.end);
	  }
	}
  } else rep.code = NVRR_NOT_SUPPORTED;
  Reply(who, &rep, sizeof(rep));
}
