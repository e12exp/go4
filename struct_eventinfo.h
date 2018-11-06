#ifndef __struct_eventinfo_h
#define __struct_eventinfo_h
#include "struct_gosip.h"
#include "struct_event.h"
#include <assert.h>

typedef struct eventinfo
{
  gosip_sub_header_t *gosip;
  event_t *evnt;
  trace_head_t *trace;
  //to be continued
  uint32_t tracepoints;
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



#endif
