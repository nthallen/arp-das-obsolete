#ifndef DG_TMR_H
#define DG_TMR_H

class DG_tmr {
  public:
    DG_tmr(int coid, int pulse_code );
    ~DG_tmr();
    void settime( int per_sec, int per_nsec );
  private:
    int timerid;
};

#endif

