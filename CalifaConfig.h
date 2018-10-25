#ifndef _CalifaCONFIG_H_
#define _CalifaCONFIG_H_

#define LOGLEVEL 0


#define TIME_ZERO {0,0} //{port id, slave id}

#define RPID_CUTG 0

// Only draw traces every n-th event
#define DRAW_TRACE_EVERY 1 

// Only refresh hists every n-th event
#define REFR_HIST_EVERY 100

#define DIFF_TS 0

#define FBX_DFT 0
#define FBX_FFT 0

#define FBX_HIST_ADC 1
#define FBX_ADC_BITS 14

#define ECUT 0
#define EMIN 19500
#define EMAX 22500

#define ONLY_SELFTRIGGERED 0

#define RPID_CUT 0
#define RPID_CUTG 0

#define X1 -28719
#define Y1 38081
#define X2 -3424
#define Y2 5934
#define XMAX -3100

//RPID on/off
#define RPID 0

#define F_ADC 50

//Time over Threshold on/off
#define TOT 0

#if ! FFT
#define TRACE_SIZE 220         // in samples
#else
//for FFT, trace size has to be a power of two
#define TRACE_SIZE 1<<12
#endif

#define SIS_TRACE_LEN 3000

#define DEBUG 0

#endif

