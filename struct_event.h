#ifndef STRUCT_EVENT_H_
#define STRUCT_EVENT_H_

#include <stdint.h>
#ifndef _PACKED
#define _PACKED  __attribute__ ((packed))
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
	uint16_t energy;
//----- QPID ------------
	uint16_t n_f;
	uint16_t n_s;
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
  uint16_t points[0] _PACKED;
}
trace_head_t;
} //extern "C"

#if 0
static_assert(sizeof(trace_head_t)==6, "trace_head_t alignment bad");
//"memory alignment of event_t is wrong");
#endif 

#endif /*STRUCT_EVENT_H_*/
