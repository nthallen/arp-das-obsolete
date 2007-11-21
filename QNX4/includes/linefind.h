typedef float FFT_T;
typedef struct {
  int Rate;
  int Npts;
  unsigned short PeakPos; /* Peak pos in steps from first sample */
  unsigned short Pos0; /* Etalon Pos of first step */
  FFT_T *Raw;
  FFT_T *Data;
  FFT_T *Hi;
} RawScan;
extern int Ns;
void ProcessScan( RawScan *Scan );
void NewBaseScan( RawScan *Scan );
void WriteScan( RawScan *Scan );
void InitLineFind( RawScan *Scan, int Ns_set, char *bfile );
void LoadVector( RawScan *scan, char *fname );
