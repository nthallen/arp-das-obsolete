/* nvram.h Prototypes for ram_utils. Includes code because the code
 * is interpreted differently depending on whether we are internal
 * to the subbus driver or not. (read/write_novram calls are either
 * pointers or direct calls.)
 * $Log$
 */
#define NVRAM_SIZE 8192

void read_dir(unsigned short i, struct nvram_dir_entry *dir) {
  unsigned short o;
  
  o = i*5;
  dir->ID = read_novram(o); o++;
  dir->start = read_novram(o); o++;
  dir->start = (dir->start << 8) + read_novram(o); o++;
  dir->end = read_novram(o); o++;
  dir->end = (dir->end << 8) + read_novram(o);
}

void init_entry(unsigned short i,
				unsigned char id,
				unsigned short start,
				unsigned short end) {
  unsigned short o, p;
  
  o = i*5 + 4;
  write_novram(o, end >> 8); o--;
  write_novram(o, end & 0xFF); o--;
  write_novram(o, start >> 8); o--;
  write_novram(o, start & 0xFF); o--;
  if (id != 0) {
	for (p = start; p <= end; p++)
	  write_novram(p, 0);
  }
  write_novram(o, id);
}
