#ifndef __struct_eventinfo_h
#define __struct_eventinfo_h
#include "struct_gosip.h"
#include "struct_event.h"
typedef struct eventinfo
{
  gosip_sub_header_t *gosip;
  event_t *evnt;
  trace_head_t *trace;
  //to be continued
  uint32_t tracepoints;
} eventinfo_t;


#endif
