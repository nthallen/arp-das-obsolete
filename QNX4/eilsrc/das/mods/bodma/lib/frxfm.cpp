/* FRXFM.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 FRXFM.CPP 23-Jun-95,20:04:02,`TBUIJS'
     C version of FRXFM that can do up to 1000000 points.
1:1 FRXFM.CPP 11-Jul-95,14:07:40,`TBUIJS'
     Changed FRXFM to call the assembler version fastfrx when the vector is
     16k points or less. Otherwise it calls a special version of FRXFM that
     breaks the arrays down into 4 quarters in order to avoid huge indexing with
     arrays up to 64k, if the array is bigger than 64K a generalized but slow
     version is called.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*extern "C" far fastfrxfm (unsigned short n2pow, float huge *x, float huge *y);*/
static void frxfm_small (unsigned short n2pow, float  *x, float  *y);
static void frxfm_big (unsigned short n2pow, float  *x, float  *y);
static void sorter (unsigned short, unsigned long, float  *, float  *);

float *c_cos, *c_sin, *f_cos, *f_sin;

void frxfm (unsigned short n2pow, float  *x, float  *y)
{
	static short init = 1;
	static float trig_table[4096];
	const double pi = 3.14159265359;

	if (init)
		{
		c_cos = trig_table;
		c_sin = c_cos + 1024;
		f_cos = c_cos + 2048;
		f_sin = c_cos + 3072;

		for (short n = 0; n < 1024; n++)
			{
			c_cos[n] = (float)cos (2.0*pi/1024.0*n);
			c_sin[n] = (float)sin (2.0*pi/1024.0*n);
			f_cos[n] = (float)cos (2.0*pi/1048576.0*n);
			f_sin[n] = (float)sin (2.0*pi/1048576.0*n);
			}
		init = 0;
		}

/*	if (n2pow < 15)
  		{
		fastfrxfm (n2pow, x, y);
	  	}
	else */ if (n2pow < 17)
		{
		frxfm_small (n2pow, x, y);
		}
	else
	  	{
		frxfm_big (n2pow, x, y);
		}
}

void frxfm_small (unsigned short n2pow, float  *x, float  *y)
{
	unsigned long m, nthpow;
	unsigned short length, seqloc, nxtlth;
	unsigned short nth4pow, n4pow, pass;
	unsigned short j, j1, j2, j3, j4;
	unsigned short m1, m2;
	unsigned char scale;
	float c1,s1,c2,s2,c3,s3;
	float i, r;
	float r1,r2,r3,r4,i1,i2,i3,i4,a1,a2,a3,a4,a5,a6;
	float *x1, *x2, *x3, *x4, *y1, *y2, *y3, *y4;

	nthpow = 1UL << n2pow;
	nth4pow = (unsigned short)(nthpow >> 2);
	n4pow = n2pow >> 1;

	// divide up the array into quarters so that we can do a 64K point
	// FFT without huge pointers.
	x1 = (float *)x;
	x2 = (float *)(x+nth4pow);
	x3 = (float *)(x+nth4pow+nth4pow);
	x4 = (float *)(x+nth4pow+nth4pow+nth4pow);
	y1 = (float *)y;
	y2 = (float *)(y+nth4pow);
	y3 = (float *)(y+nth4pow+nth4pow);
	y4 = (float *)(y+nth4pow+nth4pow+nth4pow);

	if (n4pow)
		{
		/* first pass */
		scale = n2pow - 2;
		nxtlth = 1U << scale;

		r1 = x1[0] + x3[0];
		r2 = x1[0] - x3[0];
		r3 = x2[0] + x4[0];
		r4 = x2[0] - x4[0];
		i1 = y1[0] + y3[0];
		i2 = y1[0] - y3[0];
		i3 = y2[0] + y4[0];
		i4 = y2[0] - y4[0];
		x3[0] = r2 - i4;
		x4[0] = r2 + i4;
		y3[0] = i2 + r4;
		y4[0] = i2 - r4;
		x2[0] = r1 - r3;
		x1[0] = r1 + r3;
		y2[0] = i1 - i3;
		y1[0] = i1 + i3;

		scale = 18 - scale;
		for (j = 1; j < nxtlth; j++)
			{
			m = (unsigned long)j << scale;
			m1 = (unsigned short)(m >> 10);
			m2 = (unsigned short)(m & 1023);

			c1 = c_cos[m1]*f_cos[m2] - c_sin[m1]*f_sin[m2];
			s1 = c_cos[m1]*f_sin[m2] + c_sin[m1]*f_cos[m2];
			c2 = (c1*c1) - (s1*s1);
			s2 = (c1*s1) + (c1*s1);
			c3 = (c1*c2) - (s1*s2);
			s3 = (c2*s1) + (s2*c1);

			r1 = x1[j] + x3[j];
			r2 = x1[j] - x3[j];
			r3 = x2[j] + x4[j];
			r4 = x2[j] - x4[j];
			i1 = y1[j] + y3[j];
			i2 = y1[j] - y3[j];
			i3 = y2[j] + y4[j];
			i4 = y2[j] - y4[j];
			a1 = r2 - i4;
			a5 = r2 + i4;
			a2 = i2 + r4;
			a6 = i2 - r4;
			x1[j] = r1 + r3;
			a3 = r1 - r3;
			y1[j] = i1 + i3;
			a4 = i1 - i3;
			x2[j] = (c2*a3) - (s2*a4);
			y2[j] = (s2*a3) + (c2*a4);
			x3[j] = (c1*a1) - (s1*a2);
			y3[j] = (s1*a1) + (c1*a2);
			x4[j] = (c3*a5) - (s3*a6);
			y4[j] = (s3*a5) + (c3*a6);
			}

		/* other passes */
		for (pass = 2; pass <= n4pow; pass++)
			{
			scale = n2pow - (pass<<1);
			nxtlth = 1U << scale;
			length = nxtlth << 2;
			for (seqloc = length; seqloc <= nth4pow; seqloc += length)
				{
				j1 = seqloc - length;
				j2 = j1 + nxtlth;
				j3 = j2 + nxtlth;
				j4 = j3 + nxtlth;

				r1 = x1[j1] + x1[j3];
				r2 = x1[j1] - x1[j3];
				r3 = x1[j2] + x1[j4];
				r4 = x1[j2] - x1[j4];
				i1 = y1[j1] + y1[j3];
				i2 = y1[j1] - y1[j3];
				i3 = y1[j2] + y1[j4];
				i4 = y1[j2] - y1[j4];
				x1[j3] = r2 - i4;
				x1[j4] = r2 + i4;
				y1[j3] = i2 + r4;
				y1[j4] = i2 - r4;
				x1[j2] = r1 - r3;
				x1[j1] = r1 + r3;
				y1[j2] = i1 - i3;
				y1[j1] = i1 + i3;

				r1 = x2[j1] + x2[j3];
				r2 = x2[j1] - x2[j3];
				r3 = x2[j2] + x2[j4];
				r4 = x2[j2] - x2[j4];
				i1 = y2[j1] + y2[j3];
				i2 = y2[j1] - y2[j3];
				i3 = y2[j2] + y2[j4];
				i4 = y2[j2] - y2[j4];
				x2[j3] = r2 - i4;
				x2[j4] = r2 + i4;
				y2[j3] = i2 + r4;
				y2[j4] = i2 - r4;
				x2[j2] = r1 - r3;
				x2[j1] = r1 + r3;
				y2[j2] = i1 - i3;
				y2[j1] = i1 + i3;

				r1 = x3[j1] + x3[j3];
				r2 = x3[j1] - x3[j3];
				r3 = x3[j2] + x3[j4];
				r4 = x3[j2] - x3[j4];
				i1 = y3[j1] + y3[j3];
				i2 = y3[j1] - y3[j3];
				i3 = y3[j2] + y3[j4];
				i4 = y3[j2] - y3[j4];
				x3[j3] = r2 - i4;
				x3[j4] = r2 + i4;
				y3[j3] = i2 + r4;
				y3[j4] = i2 - r4;
				x3[j2] = r1 - r3;
				x3[j1] = r1 + r3;
				y3[j2] = i1 - i3;
				y3[j1] = i1 + i3;

				r1 = x4[j1] + x4[j3];
				r2 = x4[j1] - x4[j3];
				r3 = x4[j2] + x4[j4];
				r4 = x4[j2] - x4[j4];
				i1 = y4[j1] + y4[j3];
				i2 = y4[j1] - y4[j3];
				i3 = y4[j2] + y4[j4];
				i4 = y4[j2] - y4[j4];
				x4[j3] = r2 - i4;
				x4[j4] = r2 + i4;
				y4[j3] = i2 + r4;
				y4[j4] = i2 - r4;
				x4[j2] = r1 - r3;
				x4[j1] = r1 + r3;
				y4[j2] = i1 - i3;
				y4[j1] = i1 + i3;
				}

			scale = 18-scale;
			for (j = 1; j < nxtlth; j++)
				{
				m = (unsigned long)j << scale;
				m1 = (unsigned short)(m >> 10);
				m2 = (unsigned short)(m & 1023);

				c1 = c_cos[m1]*f_cos[m2] - c_sin[m1]*f_sin[m2];
				s1 = c_cos[m1]*f_sin[m2] + c_sin[m1]*f_cos[m2];
				c2 = (c1*c1) - (s1*s1);
				s2 = (c1*s1) + (c1*s1);
				c3 = (c1*c2) - (s1*s2);
				s3 = (c2*s1) + (s2*c1);

				for (seqloc = length; seqloc <= nth4pow; seqloc += length)
					{
					j1 = seqloc - length + j;
					j2 = j1 + nxtlth;
					j3 = j2 + nxtlth;
					j4 = j3 + nxtlth;

					r1 = x1[j1] + x1[j3];
					r2 = x1[j1] - x1[j3];
					r3 = x1[j2] + x1[j4];
					r4 = x1[j2] - x1[j4];
					i1 = y1[j1] + y1[j3];
					i2 = y1[j1] - y1[j3];
					i3 = y1[j2] + y1[j4];
					i4 = y1[j2] - y1[j4];
					a1 = r2 - i4;
					a5 = r2 + i4;
					a2 = i2 + r4;
					a6 = i2 - r4;
					x1[j1] = r1 + r3;
					a3 = r1 - r3;
					y1[j1] = i1 + i3;
					a4 = i1 - i3;
					x1[j2] = (c2*a3) - (s2*a4);
					y1[j2] = (s2*a3) + (c2*a4);
					x1[j3] = (c1*a1) - (s1*a2);
					y1[j3] = (s1*a1) + (c1*a2);
					x1[j4] = (c3*a5) - (s3*a6);
					y1[j4] = (s3*a5) + (c3*a6);

					r1 = x2[j1] + x2[j3];
					r2 = x2[j1] - x2[j3];
					r3 = x2[j2] + x2[j4];
					r4 = x2[j2] - x2[j4];
					i1 = y2[j1] + y2[j3];
					i2 = y2[j1] - y2[j3];
					i3 = y2[j2] + y2[j4];
					i4 = y2[j2] - y2[j4];
					a1 = r2 - i4;
					a5 = r2 + i4;
					a2 = i2 + r4;
					a6 = i2 - r4;
					x2[j1] = r1 + r3;
					a3 = r1 - r3;
					y2[j1] = i1 + i3;
					a4 = i1 - i3;
					x2[j2] = (c2*a3) - (s2*a4);
					y2[j2] = (s2*a3) + (c2*a4);
					x2[j3] = (c1*a1) - (s1*a2);
					y2[j3] = (s1*a1) + (c1*a2);
					x2[j4] = (c3*a5) - (s3*a6);
					y2[j4] = (s3*a5) + (c3*a6);

					r1 = x3[j1] + x3[j3];
					r2 = x3[j1] - x3[j3];
					r3 = x3[j2] + x3[j4];
					r4 = x3[j2] - x3[j4];
					i1 = y3[j1] + y3[j3];
					i2 = y3[j1] - y3[j3];
					i3 = y3[j2] + y3[j4];
					i4 = y3[j2] - y3[j4];
					a1 = r2 - i4;
					a5 = r2 + i4;
					a2 = i2 + r4;
					a6 = i2 - r4;
					x3[j1] = r1 + r3;
					a3 = r1 - r3;
					y3[j1] = i1 + i3;
					a4 = i1 - i3;
					x3[j2] = (c2*a3) - (s2*a4);
					y3[j2] = (s2*a3) + (c2*a4);
					x3[j3] = (c1*a1) - (s1*a2);
					y3[j3] = (s1*a1) + (c1*a2);
					x3[j4] = (c3*a5) - (s3*a6);
					y3[j4] = (s3*a5) + (c3*a6);

					r1 = x4[j1] + x4[j3];
					r2 = x4[j1] - x4[j3];
					r3 = x4[j2] + x4[j4];
					r4 = x4[j2] - x4[j4];
					i1 = y4[j1] + y4[j3];
					i2 = y4[j1] - y4[j3];
					i3 = y4[j2] + y4[j4];
					i4 = y4[j2] - y4[j4];
					a1 = r2 - i4;
					a5 = r2 + i4;
					a2 = i2 + r4;
					a6 = i2 - r4;
					x4[j1] = r1 + r3;
					a3 = r1 - r3;
					y4[j1] = i1 + i3;
					a4 = i1 - i3;
					x4[j2] = (c2*a3) - (s2*a4);
					y4[j2] = (s2*a3) + (c2*a4);
					x4[j3] = (c1*a1) - (s1*a2);
					y4[j3] = (s1*a1) + (c1*a2);
					x4[j4] = (c3*a5) - (s3*a6);
					y4[j4] = (s3*a5) + (c3*a6);
					}
				}
			}
		}
	if (n2pow != 2*n4pow)
		{
		for (j = 0; j < nth4pow; j += 2)
			{
			r = x1[j] + x1[j+1];
			x1[j+1] = x1[j] - x1[j+1];
			x1[j] = r;
			i = y1[j] + y1[j+1];
			y1[j+1] = y1[j] - y1[j+1];
			y1[j] = i;

			r = x2[j] + x2[j+1];
			x2[j+1] = x2[j] - x2[j+1];
			x2[j] = r;
			i = y2[j] + y2[j+1];
			y2[j+1] = y2[j] - y2[j+1];
			y2[j] = i;

			r = x3[j] + x3[j+1];
			x3[j+1] = x3[j] - x3[j+1];
			x3[j] = r;
			i = y3[j] + y3[j+1];
			y3[j+1] = y3[j] - y3[j+1];
			y3[j] = i;

			r = x4[j] + x4[j+1];
			x4[j+1] = x4[j] - x4[j+1];
			x4[j] = r;
			i = y4[j] + y4[j+1];
			y4[j+1] = y4[j] - y4[j+1];
			y4[j] = i;
			}
		}
	sorter (n2pow, nthpow, x, y);
}

void frxfm_big (unsigned short n2pow, float  *x, float  *y)
{
	unsigned long nthpow, length, seqloc;
	unsigned long m, nxtlth;
	unsigned short n4pow, pass;
	unsigned long j, j1, j2, j3, j4;
	unsigned short m1, m2;
	unsigned char scale;
	float c1,s1,c2,s2,c3,s3;
	float i, r;
	float r1,r2,r3,r4,i1,i2,i3,i4,a1,a2,a3,a4,a5,a6;

	nthpow = 1UL << n2pow;
	n4pow = n2pow >> 1;
	if (n4pow)
		{
		/* first pass */
		scale = n2pow - 2;
		nxtlth = 1UL << scale;
		j3 = nxtlth + nxtlth;
		j4 = j3 + nxtlth;

		r1 = x[0] + x[j3];
		r2 = x[0] - x[j3];
		r3 = x[nxtlth] + x[j4];
		r4 = x[nxtlth] - x[j4];
		i1 = y[0] + y[j3];
		i2 = y[0] - y[j3];
		i3 = y[nxtlth] + y[j4];
		i4 = y[nxtlth] - y[j4];
		x[j3] = r2 - i4;
		x[j4] = r2 + i4;
		y[j3] = i2 + r4;
		y[j4] = i2 - r4;
		x[nxtlth] = r1 - r3;
		x[0] = r1 + r3;
		y[nxtlth] = i1 - i3;
		y[0] = i1 + i3;

		scale = 18 - scale;
		for (j = 1; j < nxtlth; j++)
			{
			m = j << scale;
			m1 = (unsigned short)(m >> 10);
			m2 = (unsigned short)(m & 1023);

			c1 = c_cos[m1]*f_cos[m2] - c_sin[m1]*f_sin[m2];
			s1 = c_cos[m1]*f_sin[m2] + c_sin[m1]*f_cos[m2];
			c2 = (c1*c1) - (s1*s1);
			s2 = (c1*s1) + (c1*s1);
			c3 = (c1*c2) - (s1*s2);
			s3 = (c2*s1) + (s2*c1);

			j2 = j + nxtlth;
			j3 = j2 + nxtlth;
			j4 = j3 + nxtlth;

			r1 = x[j] + x[j3];
			r2 = x[j] - x[j3];
			r3 = x[j2] + x[j4];
			r4 = x[j2] - x[j4];
			i1 = y[j] + y[j3];
			i2 = y[j] - y[j3];
			i3 = y[j2] + y[j4];
			i4 = y[j2] - y[j4];
			a1 = r2 - i4;
			a5 = r2 + i4;
			a2 = i2 + r4;
			a6 = i2 - r4;
			x[j] = r1 + r3;
			a3 = r1 - r3;
			y[j] = i1 + i3;
			a4 = i1 - i3;
			x[j2] = (c2*a3) - (s2*a4);
			y[j2] = (s2*a3) + (c2*a4);
			x[j3] = (c1*a1) - (s1*a2);
			y[j3] = (s1*a1) + (c1*a2);
			x[j4] = (c3*a5) - (s3*a6);
			y[j4] = (s3*a5) + (c3*a6);
			}


		/* other passes */
		for (pass = 2; pass <= n4pow; pass++)
			{
			scale = n2pow - (pass<<1);
			nxtlth = 1UL << scale;
			length = nxtlth << 2;
			for (seqloc = length; seqloc <= nthpow; seqloc += length)
				{
				j1 = seqloc - length;
				j2 = j1 + nxtlth;
				j3 = j2 + nxtlth;
				j4 = j3 + nxtlth;

				r1 = x[j1] + x[j3];
				r2 = x[j1] - x[j3];
				r3 = x[j2] + x[j4];
				r4 = x[j2] - x[j4];
				i1 = y[j1] + y[j3];
				i2 = y[j1] - y[j3];
				i3 = y[j2] + y[j4];
				i4 = y[j2] - y[j4];
				x[j3] = r2 - i4;
				x[j4] = r2 + i4;
				y[j3] = i2 + r4;
				y[j4] = i2 - r4;
				x[j2] = r1 - r3;
				x[j1] = r1 + r3;
				y[j2] = i1 - i3;
				y[j1] = i1 + i3;
				}

			scale = 18-scale;
			for (j = 1; j < nxtlth; j++)
				{
				m = j << scale;
				m1 = (unsigned short)(m >> 10);
				m2 = (unsigned short)(m & 1023);

				c1 = c_cos[m1]*f_cos[m2] - c_sin[m1]*f_sin[m2];
				s1 = c_cos[m1]*f_sin[m2] + c_sin[m1]*f_cos[m2];
				c2 = (c1*c1) - (s1*s1);
				s2 = (c1*s1) + (c1*s1);
				c3 = (c1*c2) - (s1*s2);
				s3 = (c2*s1) + (s2*c1);

				for (seqloc = length; seqloc <= nthpow; seqloc += length)
					{
					j1 = seqloc - length + j;
					j2 = j1 + nxtlth;
					j3 = j2 + nxtlth;
					j4 = j3 + nxtlth;

					r1 = x[j1] + x[j3];
					r2 = x[j1] - x[j3];
					r3 = x[j2] + x[j4];
					r4 = x[j2] - x[j4];
					i1 = y[j1] + y[j3];
					i2 = y[j1] - y[j3];
					i3 = y[j2] + y[j4];
					i4 = y[j2] - y[j4];
					a1 = r2 - i4;
					a5 = r2 + i4;
					a2 = i2 + r4;
					a6 = i2 - r4;
					x[j1] = r1 + r3;
					a3 = r1 - r3;
					y[j1] = i1 + i3;
					a4 = i1 - i3;
					x[j2] = (c2*a3) - (s2*a4);
					y[j2] = (s2*a3) + (c2*a4);
					x[j3] = (c1*a1) - (s1*a2);
					y[j3] = (s1*a1) + (c1*a2);
					x[j4] = (c3*a5) - (s3*a6);
					y[j4] = (s3*a5) + (c3*a6);
					}
				}
			}
		}
	if (n2pow != 2*n4pow)
		{
		for (j = 0; j<nthpow; j += 2)
			{
			r = x[j] + x[j+1];
			x[j+1] = x[j] - x[j+1];
			x[j] = r;
			i = y[j] + y[j+1];
			y[j+1] = y[j] - y[j+1];
			y[j] = i;
			}
		}
	sorter (n2pow, nthpow, x, y);
}

static void sorter (unsigned short n2pow, unsigned long nthpow,
						float  *x, float   *y)
{
	float r;
	static unsigned char blt[] =
		{
		0,128,64,192,32,160,96,224,16,144,80,208,48,176,112,240,
		8,136,72,200,40,168,104,232,24,152,88,216,56,184,120,248,
		4,132,68,196,36,164,100,228,20,148,84,212,52,180,116,244,
		12,140,76,204,44,172,108,236,28,156,92,220,60,188,124,252,
		2,130,66,194,34,162,98,226,18,146,82,210,50,178,114,242,10,
		138,74,202,42,170,106,234,26,154,90,218,58,186,122,250,6,
		134,70,198,38,166,102,230,22,150,86,214,54,182,118,246,14,
		142,78,206,46,174,110,238,30,158,94,222,62,190,126,254,1,
		129,65,193,33,161,97,225,17,145,81,209,49,177,113,241,9,137,
		73,201,41,169,105,233,25,153,89,217,57,185,121,249,5,133,69,
		197,37,165,101,229,21,149,85,213,53,181,117,245,13,141,77,
		205,45,173,109,237,29,157,93,221,61,189,125,253,3,131,67,195,
		35,163,99,227,19,147,83,211,51,179,115,243,11,139,75,203,43,
		171,107,235,27,155,91,219,59,187,123,251,7,135,71,199,39,167,
		103,231,23,151,87,215,55,183,119,247,15,143,79,207,47,175,
		111,239,31,159,95,223,63,191,127,255
		};

	unsigned long n0, n5;

	for (n0 = nthpow, n2pow = 24-n2pow; --n0;)
		{

		n5 = (((unsigned long)blt[(short)(n0 & 0xffUL)] << 16) +
			((unsigned short)blt[(short)((n0 & 0xff00UL) >> 8)] << 8) +
			blt[(short)((n0 & 0xff0000UL) >> 16)]) >> n2pow;

		if (n0 > n5)
			{
			r = x[n0];
			x[n0] = x[n5];
			x[n5] = r;
			r = y[n0];
			y[n0] = y[n5];
			y[n5] = r;
			}
		}
}

