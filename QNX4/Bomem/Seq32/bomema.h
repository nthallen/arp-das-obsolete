/* bomema.h Defines analysis interface.
 * $Log$
 */
/*
 The basic requirements for analysis of DSP data is as follows:
 
 The TM stream is monitored to observe changes in the BmSeq
 datum. A new value indicates that a new scan has been written
 to disk. We must determine which blackbody was being observed
 using the mirror position, MirPs. From the blackbody selection,
 we can determine which temperature channel is attached to that
 blackbody. We must convert that voltage to a temperature using
 the appropriate calibration formula. Finally, the sequence
 number, the blackbody code and the temperature can be passed
 to an analysis routine or process.

 I expect the analysis to be moderately to extremely time-consuming.
 Hence if it is done with the TM client process, the client had
 better be off the buffer in realtime. This means that the client
 is definitely not a server under those conditions. If the analysis
 process needs to be a server in order to receive commands from other
 sources (e.g. a windows program) then it must be separated from the
 TM client.
*/
#define BO_ANALYSIS 'ba'
#define BA_DESC_SIZE 40

typedef struct {
  unsigned short id; /* 'ba'aka BO_ANALYSIS */
  unsigned short sequence;
  unsigned short bb_code;
  double temperature;
  double time;
  char description[BA_DESC_SIZE];
} bomem_analysis;
