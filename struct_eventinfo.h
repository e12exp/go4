#ifndef __struct_eventinfo_h
#define __struct_eventinfo_h
#include "struct_gosip.h"
#include "struct_event.h"
#include <assert.h>

typedef struct eventinfo
{
  uint64_t wrts;
  gosip_sub_header_t *gosip;
  event_t *evnt;
  trace_head_t *trace;
  //to be continued
  uint32_t tracepoints;
  uint32_t trace_start; // which sample of the trace has the highest slope?
  double max_slope;     // what is that slope
  double trace_en;      // what is the difference between the maximum of the trace and the baseline before?
  double trace_en_diff; 
  double trace0, bl_slope; 

  double calEnergy;
  eventinfo& operator+=(event_t* rhs)
  {
    assert(evnt);
    assert(rhs);
    evnt->n_f+=rhs->n_f;
    evnt->n_s+=rhs->n_s;
    evnt->energy += rhs->energy;
    evnt->magic++; // hack for multiplicity
    //    calEnergy+=rhs->calEnergy;
    return *this;
  }

} eventinfo_t;

inline double getTracePoint(eventinfo_t* ei, int n, bool sign=SIGNED_TRACES)
{
  return getTracePoint(ei->trace, n, sign);
}



#endif
