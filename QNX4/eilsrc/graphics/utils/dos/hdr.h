struct hdr {
	unsigned char rows;
	unsigned char cols;
	unsigned char pos_y;
	unsigned char pos_x;
	unsigned char nattrs;
};
#define HDRSIZE (sizeof(struct scrhdr))
