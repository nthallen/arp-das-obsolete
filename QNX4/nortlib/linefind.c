#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "nortlib.h"
#include "rtgapi.h"
#include "ssp.h"
#include "linefind.h"
char rcsid_linefind_c[] =
  "$Header$";

/* Ns holds the array size. The number of samples in a scan is 
   2*Ns. The 2*Ns points are packed into Ns complex values for an
   Ns-point FFT. This saves time by avoiding the redundant 
   calculation of the negative frequencies, which are simply the 
   complex conjugate of the positive frequency values.
   The filter, H, being real and symmetric, requires only Ns/2
   points.
   
   When loading vectors, no more than 2*Ns points will fit in the 
   Raw array, so take note.
*/
int Ns;
static RawScan Base;
static FFT_T *H;
static char *base_file;

static int graphics;

#define fft(x,y,z) realft(x,2*(y),z)

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

static void four1(float data[], unsigned long nn, int isign)
{
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	float tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=isign*(6.28318530717959/mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}
#undef SWAP
/* (C) Copr. 1986-92 Numerical Recipes Software +@(90L+V3. */

static void realft(float data[], unsigned long n, int isign)
{
	void four1(float data[], unsigned long nn, int isign);
	unsigned long i,i1,i2,i3,i4,np3;
	float c1=0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta;

	theta=3.141592653589793/(double) (n>>1);
	if (isign == 1) {
		c2 = -0.5;
		four1(data,n>>1,1);
	} else {
		c2=0.5;
		theta = -theta;
	}
	wtemp=sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi=sin(theta);
	wr=1.0+wpr;
	wi=wpi;
	np3=n+3;
	for (i=2;i<=(n>>2);i++) {
		i4=1+(i3=np3-(i2=1+(i1=i+i-1)));
		h1r=c1*(data[i1]+data[i3]);
		h1i=c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i=c2*(data[i1]-data[i3]);
		data[i1]=h1r+wr*h2r-wi*h2i;
		data[i2]=h1i+wr*h2i+wi*h2r;
		data[i3]=h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wr=(wtemp=wr)*wpr-wi*wpi+wr;
		wi=wi*wpr+wtemp*wpi+wi;
	}
	if (isign == 1) {
		data[1] = (h1r=data[1])+data[2];
		data[2] = h1r-data[2];
	} else {
		data[1]=c1*((h1r=data[1])+data[2]);
		data[2]=c1*(h1r-data[2]);
		four1(data,n>>1,-1);
	}
}
/* (C) Copr. 1986-92 Numerical Recipes Software +@(90L+V3. */

static rtg_t *A_raw_rtg, *B_raw_rtg, *AHi_rtg, *BHi_rtg, *C_rtg;

/* AllocateVector allocates size+1 complex values and dies if it 
   fails */
static FFT_T *AllocateVector( int size ) {
  FFT_T *V;
  V = malloc((2 * size + 1) * sizeof(FFT_T));
  if ( V == 0 ) nl_error( 4, "Out of memory in AllocateVector" );
  return V;
}

/* The units for PFmin and PFmax are feature size in steps.
   Rate is the number of steps per sample and Npts is the total 
   number of samples.
   Npts is the total number of samples expected (2*Ns), but
   GenerateFilter produces Npts/2 filter values (just the
   positive frequencies) since the filter is symmetric.
   Features smaller than PFmin steps will be passed (gain 1),
   features larger than PFmax will be eliminated (gain 0) and
   features inbetween will be filtered by a function linear in
   frequency.
*/
static void
GenerateFilter( FFT_T *H, int Npts, int Rate, int PFmin, int PFmax ) {
  int n, m, i;
  FFT_T slope;
  
  if ( PFmax < PFmin ) PFmax = PFmin;
  n = (((long)Npts)*Rate)/PFmax;
  m = (((long)Npts)*Rate+PFmin-1)/PFmin;
  Npts /= 2;
  if ( n > Npts || m > Npts )
	nl_error( 3, "Filter params out of joint" );
  for ( i = 1; i <= n; i++ ) H[i] = 0;
  if ( m-n > 1 ) {
	slope = 1.0/(m-n);
	for ( i = n+1; i < m; i++ )
	  H[i] = (i-n)*slope;
  }
  for ( i = m; i <= Npts; i++ ) H[i] = 1;
}

static void
VectorMult( FFT_T *A, FFT_T *B, int Npts ) {
  FFT_T temp;
  int i;

  for ( i = 1; i <= Npts; i++ ) {
	temp = A[1] * B[1] - A[2] * B[2];
	A[2] = A[1] * B[2] + A[2] * B[1];
	A[1] = temp;
	A += 2; B += 2;
  }
}

static void
VectorRMult( FFT_T *A, FFT_T *R, int Npts ) {
  int i;
  
  A++; R++;
  for ( i = 1; i <= Npts; i++ ) {
	*A++ *= *R;
	*A++ *= *R++;
  }
}

static void VectorCopy( FFT_T *A, FFT_T *B, int Npts ) {
  memcpy( A, B, (2*Npts+1)*sizeof(FFT_T) );
}

static void VectorConj( FFT_T *A, int Npts ) {
  A += 2;
  for ( A += 2; Npts-- > 0; A += 2 ) *A = -(*A);
}

void LoadVector( RawScan *scan, char *fname ) {
  FILE *fp;
  int i, npts;
  FFT_T *A;
  
  if ( Ns == 0 ) {
	nl_error( 2, "LoadVector called before InitLineFind()" );
	return;
  }
  npts = Ns*2;
  scan->Npts = 0;
  fp = fopen( fname, "r" );
  if ( fp == 0 ) {
	nl_error( 1, "Unable to open scan file %s", fname );
	return;
  }
  fscanf( fp, "%u %u %u", &scan->Rate, &scan->Pos0, &scan->PeakPos );
  A = scan->Raw;
  for ( i = 1; i <= npts; i++ ) {
	if ( fscanf( fp, "%f", &A[i] ) != 1 ) break;
  }
  fclose(fp);
  scan->Npts = i-1;
}

/* detrends the real vector A[1..npts] and zero pads out to 
   maxpts. detrending subtracts the line between the first
   and last points (A[1] and A[npts])
*/
static void Detrend( FFT_T *A, int maxpts, int npts ) {
  double m, b;
  int x;
  
  b = A[1];
  m = (A[npts] - A[1])/(npts-1);
  for ( x = 0; x < npts; x++ )
	A[x+1] -= m*x + b;
  for ( x = npts+1; x <= maxpts; x++ )
	A[x] = 0;
}

/* Swaps the first npts/2 complex values with the second */
static void VectorWrap( FFT_T *A, int npts ) {
  int i;
  FFT_T temp;
  
  for ( i = 1; i <= npts; i++ ) {
	temp = A[i];
	A[i] = A[i+npts];
	A[i+npts] = temp;
  }
}

/* returns the index of the maximum element in the real array 
   A[1..npts]
*/
static int MaxIndex( FFT_T *A, int npts ) {
  FFT_T maxVal;
  int maxIdx, i;
  
  maxVal = A[1]; maxIdx = 1;
  for ( i = 2; i <= npts; i++ ) {
	if ( A[i] > maxVal ) {
	  maxVal = A[i];
	  maxIdx = i;
	}
  }
  return maxIdx;
}

static void ProcessBase( void ) {
  { static unsigned short Filter_Rate = 0;
	if ( Base.Rate != Filter_Rate ) {
	  GenerateFilter( H, 2*Ns, Base.Rate, 1200, 3000);
	  Filter_Rate = Base.Rate;
	}
  }

  VectorCopy( Base.Data, Base.Raw, Ns );
  Detrend( Base.Data, Ns*2, Base.Npts );
  fft( Base.Data, Ns, 1 );
  VectorRMult( Base.Data, H, Ns );
  if ( graphics ) {
	double X0 = Base.Pos0 - (long)Base.PeakPos;
	VectorCopy( Base.Hi, Base.Data, Ns );
	fft( Base.Hi, Ns, -1 );
	rtg_report( B_raw_rtg, non_number, 0 );
	rtg_report( BHi_rtg, non_number, 0 );
	rtg_report( A_raw_rtg, non_number, 0 );
	rtg_report( AHi_rtg, non_number, 0 );
	rtg_report( C_rtg, non_number, 0 );
	rtg_sequence( B_raw_rtg, X0, Base.Rate, Base.Npts, &Base.Raw[1] );
	rtg_sequence( BHi_rtg, X0, Base.Rate, Ns*2, &Base.Hi[1] );
  }
  VectorConj( Base.Data, Ns );
}

/* If the Scan is empty or there is no base scan, Scan->PeakPos 
will be set to zero. Otherwise it will be set to the new Peak 
Position.
*/
void ProcessScan( RawScan *Scan ) {
  int lag;

  Scan->PeakPos = 0;
  if ( Base.Npts == 0 || Scan->Npts == 0 ) {
	nl_error( 1, "Process Scan: Empty Scan or No Base Scan" );
	return;
  }
  if ( graphics ) {
	rtg_report( A_raw_rtg, non_number, 0 );
	rtg_report( AHi_rtg, non_number, 0 );
	rtg_report( C_rtg, non_number, 0 );
  }
  if ( Scan->Rate != Base.Rate ) {
	nl_error( 1, "Scan Rate (%d) differs from Base Scan (%d)",
				Scan->Rate, Base.Rate );
	if ( graphics ) {
	  double X0 = Scan->Pos0 - (long)Base.PeakPos;
	  rtg_sequence( A_raw_rtg, X0, Scan->Rate, Scan->Npts, &Scan->Raw[1] );
	}
	return;
  }
  VectorCopy( Scan->Data, Scan->Raw, Ns );
  Detrend( Scan->Data, Ns*2, Scan->Npts );
  fft( Scan->Data, Ns, 1 );
  if ( graphics ) VectorCopy( Scan->Hi, Scan->Data, Ns );
  VectorMult( Scan->Data, Base.Data, Ns );
  fft( Scan->Data, Ns, -1 );
  VectorWrap( Scan->Data, Ns );
  lag = MaxIndex( Scan->Data, Ns*2 ) - Ns - 1;
  Scan->PeakPos = Scan->Pos0 + ( Base.PeakPos - (long)Base.Pos0 )
				  + lag * Base.Rate;
  if ( graphics ) {
	double X0 = Base.Pos0 - (long)Base.PeakPos;
	X0 -= lag * Base.Rate;
	/* plot A_raw w/lag */
	rtg_sequence( A_raw_rtg, X0, Base.Rate, Scan->Npts, &Scan->Raw[1] );
	VectorRMult( Scan->Hi, H, Ns );
	fft( Scan->Hi, Ns, -1 );
	rtg_sequence( AHi_rtg, X0, Base.Rate, Ns*2, &Scan->Hi[1] );
	rtg_sequence( C_rtg, -(lag+Ns)*(double)Base.Rate, Base.Rate, Ns*2,
									  &Scan->Data[1] );
  }
}

void WriteScan( RawScan *Scan ) {
  static int ScanCount = 1;
  char fname[80];
  FILE *fp;
  int i;

  sprintf( fname, "%s.%02d", base_file, ScanCount++ );
  fp = fopen( fname, "w" );
  if ( fp != 0 ) {
	fprintf( fp, "%d %u %u\n", Scan->Rate, Scan->Pos0, Scan->PeakPos );
	for ( i = 1; i <= Scan->Npts; i++ )
	  fprintf( fp, "%14.7e\n", Scan->Raw[i] );
	fclose( fp );
	nl_error( 0, "Base Spectrum written to %s", fname );
  } else nl_error( 1, "Unable to open output file %s", fname );
}

void NewBaseScan( RawScan *Scan ) {
  if ( Scan->Npts == 0 ) {
	nl_error( 1, "Empty scan in NewBaseScan!" );
	return;
  }
  VectorCopy( Base.Raw, Scan->Raw, Ns );
  Base.Rate = Scan->Rate;
  Base.Pos0 = Scan->Pos0;
  Base.PeakPos = Scan->PeakPos;
  Base.Npts = Scan->Npts;
  ProcessBase( );
}

void InitLineFind( RawScan *Scan, int Ns_set, char *bfile ) {
  if ( Base.Raw != 0 || Ns != 0 )
	nl_error( 3, "InitLineFind called more than once!" );
  Ns = Ns_set;
  base_file = bfile;
  A_raw_rtg = rtg_init( "LineFind/Scan/Raw" );
  B_raw_rtg = rtg_init( "LineFind/Base/Raw" );
  AHi_rtg = rtg_init( "LineFind/Scan/Filtered" );
  BHi_rtg = rtg_init( "LineFind/Base/Filtered" );
  C_rtg = rtg_init( "LineFind/Corr" );
  graphics = ( A_raw_rtg->pid != -1 );
  Scan->Raw = AllocateVector( Ns );
  Scan->Data = AllocateVector( Ns );
  Base.Raw = AllocateVector( Ns );
  Base.Data = AllocateVector( Ns );
  H = AllocateVector( Ns/2 );

  /* These are needed for graphics only */
  if ( graphics ) {
	Scan->Hi = AllocateVector( Ns );
	Base.Hi = AllocateVector( Ns );
  }

  /* Load Base Vector and process */
  LoadVector( &Base, base_file );
  ProcessBase( );
}
