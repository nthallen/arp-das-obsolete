unsigned long int check_crc(array,sz)
unsigned char *array;           /* Array of bytes to CRC */
unsigned long int sz;           /* Number of bytes to CRC */
{
   unsigned long int rem,       /* CRC accumulator */
       mask;                    /* Bit extractor */

   rem=0xffff;
   for(;sz--;array++) {         /* For each entry */
      mask=0x80;
      do {                      /* For each bit */
         if(mask&*array)        /* If bit set in current byte */
            rem^=0x8000;        /* Flip bit 15 of accumulator */
         rem+=rem;              /* Shift accumulator left once */
         if(rem>0xffff) {       /* If overflow to bit 16 occurred */
            rem&=0xffff;        /* Mask to 16 bits */
            rem^=0x1021;        /* Flip bits */
         }
      } while((mask>>=1)!=0);
   }
   rem=~rem;                     /* Invert CRC */
   rem&=0xffff;                  /* Mask to 2 bytes */
   return(rem);
}

