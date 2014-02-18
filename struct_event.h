#ifndef STRUCT_EVENT_H_
#define STRUCT_EVENT_H_

#include <stdint.h>

static const int event_t_size = 44;

typedef struct event
{
	uint16_t size;
	uint16_t magic_affe;
//---- head -------------	
	uint32_t event_id;
	uint64_t timestamp;
	uint16_t cfd_samples[4];
	uint16_t loverflow;
	uint8_t hoverflow;
	uint8_t self_triggered;
	uint16_t num_pileup;
	uint16_t num_discarded;
//---- energy -----------	
	uint16_t energy;
	uint16_t reserved;
//----- RPID ------------
	uint16_t rpid_size;
	uint16_t magic_babe;
	uint16_t n_f;
	uint16_t n_s;
}
event_t;

static const int trace_head_t_size = 8;

typedef struct trace_head
{
	uint16_t size;
	uint16_t magic_2bad;
	uint16_t wrap_around_addr;
	uint16_t type;
}
trace_head_t;

static const char* OVERFLOW_NAME[] =
{
	"CFD",
	"Baseline",
	"MAU",
	"MWD",
	"PeakSensing",
	"E -> Event Buffer",
	"Trace -> Event Buffer",
	"Derivative",
	"MultExp",
	"MWD 2",
	"RPID Integration",
	"RPID Nf",
	"RPID Ns",
	"RPID Ns_hat",
	"Nf -> Event Buffer",
	"Ns -> Event Buffer",
	"RPID n"
};

#endif /*STRUCT_EVENT_H_*/
