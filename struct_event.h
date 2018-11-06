#ifndef STRUCT_EVENT_H_
#define STRUCT_EVENT_H_

#include <stdint.h>
#ifndef _PACKED
#define _PACKED  //__attribute__ ((packed))
#endif

extern "C"
{
static const int event_t_size = 44;
static const int event_115a_t_size = 40;

typedef struct event
{
	uint16_t size;
	uint16_t magic;
//---- head -------------	
	uint32_t event_id;
	uint64_t timestamp;
	uint16_t cfd_samples[4];
	uint32_t overflow;
	uint8_t self_triggered;
	uint16_t num_pileup;
	uint16_t num_discarded;
//---- energy -----------	
	int16_t energy;
//----- QPID ------------
	int16_t n_f;
	int16_t n_s;
//----- Time Over Threshold ----
	uint16_t tot;
	int16_t tot_samples[4];
}
event_t;

static const int trace_head_t_size = 8;

typedef struct trace_head
{
  uint16_t size _PACKED;
  uint16_t magic_2bad _PACKED;
  uint16_t type _PACKED;
  union {
    int16_t s[1];
    uint16_t u[1];
  } points _PACKED;

}
trace_head_t;
} //extern "C"

#include <stdio.h>
#define SIGNED_TRACES 1
inline double getTracePoint(struct trace_head* h, int n)
{
  //printf("%d\n", h->type);
  //if (h->type==3 || h->type==4)
  if (SIGNED_TRACES)
    return h->points.s[n];
  else
    return h->points.u[n];
}; 

#if 0
static_assert(sizeof(trace_head_t)==6, "trace_head_t alignment bad");
//"memory alignment of event_t is wrong");
#endif 

#endif /*STRUCT_EVENT_H_*/
