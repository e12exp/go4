#ifndef _HistogramAxis_h
#define _HistogramAxis_h
#include <math.h>
#include "struct_eventinfo.h"
#include "CalifaParser.h"
#include "CalifaSumParser.h"
#include "EnergyCal.h"
#include "TF1.h"
#include "convert_idx.h"
#include <stdlib.h>
#include <assert.h>

#define GETPARSER \
  if (!parser )\
    return NAN;
#define GETEVENTINFO\
  GETPARSER\
  if (!idx || !parser->getCalifaEvents()->count(*idx))\
    return NAN;                                       \
  auto& ei=parser->getCalifaEvents()->at(*idx);
#define GETEVNT GETEVENTINFO              \
  if (!ei.evnt)                           \
    return NAN;                           \
  auto evnt=ei.evnt;
#define DECLARE_EVNTINFO(name) double  HistogramAxisHandlers_evnt_##name(CalifaParser* parser, CalifaParser::module_index_t* idx) EVNTINFO_IMPL(name)
// ^-- read a field from an eventinfo_t
#define DECLARE_EVNT(name) double  HistogramAxisHandlers_evnt_##name(CalifaParser* parser, CalifaParser::module_index_t* idx) EVNT_IMPL(name)
// ^-- read a field from a struct_event

#if NEED_BODIES
#define EVNTINFO_IMPL(name) { GETEVENTINFO; return ei.name ; }
#define EVNT_IMPL(name)   { GETEVNT; return evnt->name; }
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) HistogramAxis axis_ ## prefix ## _ ## name = {#prefix "_" #name, bins, min, max, HistogramAxisHandlers_evnt_##name, 0};
#define DECLARE_HISTAXIS2(name, ...) HistogramAxis axis_ ## name = {__VA_ARGS__} ;
#define ONBODY(...) __VA_ARGS__ 
#else
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) extern HistogramAxis axis_ ## prefix ## _ ## name;
#define DECLARE_HISTAXIS2(name, ...) extern HistogramAxis axis_ ## name ;
#define EVNTINFO_IMPL(name) ;
#define EVNT_IMPL(name) ;
#define ONBODY(...)
#endif

template<class T, int nAxis>
class SingleHistSubprocessor;

struct HistogramAxis
{
  char descr[50];
  int nBins;
  double min;
  double max;
  double (*getValue)(CalifaParser* parser, CalifaParser::module_index_t* idx);
  int is_one;
protected:
  ONBODY(public:) // hack only accessible within HistogramAxis.cxx
  HistogramAxis(char descr[50], int nBins, double min, double max,
                double (*getValue)(CalifaParser* parser, CalifaParser::module_index_t* idx), int is_one=0):
  nBins(nBins), min(min), max(max), getValue(getValue), is_one(is_one)
  {
    strncpy(this->descr, descr, 50);
    this->descr[49]='\0';
  }
  HistogramAxis()
  {
    memset(this, 0, sizeof(*this));
  }
  template<class T, int nAxis>
  friend class SingleHistSubprocessor;
};

DECLARE_EVNTINFO(max_slope);
DECLARE_EVNTINFO(discr_amp);
DECLARE_EVNTINFO(discr_max);
DECLARE_EVNTINFO(trace_start);
DECLARE_EVNTINFO(trace_en);
DECLARE_EVNTINFO(trace0);
DECLARE_EVNTINFO(trace_en_diff);
DECLARE_EVNTINFO(bl_slope);

DECLARE_EVNT(energy);
DECLARE_EVNT(n_f);
DECLARE_EVNT(n_s);
DECLARE_EVNT(tot);
DECLARE_EVNT(num_pileup);
DECLARE_EVNT(num_discarded);
DECLARE_EVNT(self_triggered);

#if NEED_BODIES //////////////////////////////////////////////////////

double HistogramAxisHandlers_evnt_sfp0_module_dual(CalifaParser* parser,
						     CalifaParser::module_index_t* idx)
{
  GETEVNT;
  uint16_t en=evnt->energy;
  return GET_MOD(*idx)+10*(en>5000 );
}

double HistogramAxisHandlers_evnt_trace_sample(CalifaParser* parser,
                                               CalifaParser::module_index_t* idx)
{
  GETEVNT;
  return (evnt->cfd_samples[0]);
}

template <int offset, int modulus>
double HistogramAxisHandlers_evnt_dr(CalifaParser* parser,
                                     CalifaParser::module_index_t* idx)
{
  GETEVNT;
  if (evnt->energy<200) // low energy stuff, might not be real
    return NAN;
  if (GET_SFP_PURE(*idx)==0
      ||GET_SFP_PURE(*idx)==3)
    return NAN; // single range barrel section
  if (GET_MOD(*idx)%2 != modulus)
    return NAN;
  uint8_t newmod= GET_MOD(*idx)+offset;
  
  CalifaParser::module_index_t shifted=std::make_tuple(GET_TYPE(*idx), GET_SFP(*idx), newmod, GET_CH(*idx));
  if (!parser->getCalifaEvents()->count(shifted)
      || !parser->getCalifaEvents()->at(shifted).evnt)
    return 0.0; // not triggered: energy 0
  return parser->getCalifaEvents()->at(shifted).evnt->energy;
}



double HistogramAxisHandlers_evnt_multiplicity(CalifaParser* parser,
					       CalifaParser::module_index_t* idx)
{
  if (!parser)
    return NAN;
  return parser->getMultiplicity();
}

template<class T>
uint8_t bitcount(T x)
{
  uint8_t res=0;
  T current=1;
  while(current)
    {
      res+=bool(x & current);
      current<<=1;
    }
  return res;
}

double HistogramAxisHandlers_evnt_overflow_cnt(CalifaParser* parser,
					       CalifaParser::module_index_t* idx)
{
  GETEVNT;
  
  return bitcount(evnt->overflow);
}

double HistogramAxisHandlers_evnt_overflow_worst(CalifaParser* parser,
                                                CalifaParser::module_index_t* idx)
{
  // per f_user, max's firmware uses the following overflow bits:
  /*
    0: "CFD",
    1: "Baseline",
    2: "MAU",
    3: "MWD",

    4: "PeakSensing",
    5: "E -> Event Buffer",
    6: "Trace -> Event Buffer",
    7: "Nf -> Event Buffer",

    8: "Ns -> Event Buffer",
    9: "ADC",
   10: "ADC Underflow",
   11: "QPID Nf",

   12: "QPID Ns"
   13: "TOT Input",
   14: "TOT",
   15: "Unknown?!" // lets make sure we have 16 accessible elements here --pklenze
  */
  GETEVNT;
  // strategy: return just an id for the earliest overflow in the foodchain.
  // E beats QPID beats ToT beats CFD beats trace
  if (!evnt->overflow)          // no overflows, default case
    return 0;
  if (evnt->overflow & (1<<9))  // ADC overflow
    return 1;
  if (evnt->overflow & (1<<10)) // ADC underflow
    return 2;
  if (evnt->overflow & 0xe)     // baseline, mau, mwd
    return 3;
  if (evnt->overflow & 0x30)    // PeakSensing, E -> Event Buffer
    return 4;
  if (evnt->overflow & 0x1f80)  // QPID, Nf, Ns
    return 5;
  if (evnt->overflow & 0x6000)  // TOT
    return 6;
  return 7;    
}



// this would be nicer with lambdas, but they can not be used as template arguments. :-/
// (without a different C++ compiler)

// TODO: this is ugly. make cut subprocessors work instead!
#define MAKE_RESTRICTION_IMPL(name, expr) struct name {  static bool apply(CalifaParser* parser, CalifaParser::module_index_t* idx)  { expr; };};
#define MAKE_RESTRICTION(name, expr) MAKE_RESTRICTION_IMPL(name, expr) MAKE_RESTRICTION_IMPL(name##_he, GETEVNT; if (evnt->energy<100) return 0; expr)

MAKE_RESTRICTION(always, return 1)
MAKE_RESTRICTION(master, return GET_SFP_PURE(*idx)<2  && GET_MOD(*idx)<16)
MAKE_RESTRICTION(slave, return GET_SFP_PURE(*idx)>=2 && GET_MOD(*idx)<16)
MAKE_RESTRICTION(special, return GET_MOD(*idx)>=16)
MAKE_RESTRICTION(nonspecial, return GET_MOD(*idx)<16)

MAKE_RESTRICTION(mes_nonspecial, return GET_SFP(*idx)/10==1 && GET_MOD(*idx)<16)
MAKE_RESTRICTION(wix_nonspecial, return GET_SFP(*idx)/10==2 && GET_MOD(*idx)<16)


//TODO
//MAKE_RESTRICTION(mes_high_nonspecial, return GET_SFP(*idx)/10==1 && GET_MOD(*idx)<16 )
//MAKE_RESTRICTION(wix_nonspecial, return GET_SFP(*idx)/10==2 && GET_MOD(*idx)<16)


MAKE_RESTRICTION(mes_special, return GET_SFP(*idx)/10==1 && GET_MOD(*idx)>=16)
MAKE_RESTRICTION(wix_special, return GET_SFP(*idx)/10==2 && GET_MOD(*idx)>=16)

template<int ref_sys, typename valid_t=always >
double HistogramAxisHandlers_evnt_wrts_diff_ref(CalifaParser* parser,
                                                CalifaParser::module_index_t* idx)
{
    GETEVNT;
    if (!valid_t::apply(parser, idx))
      return NAN;
    int64_t own_ts=ei.wrts;
    auto& tsmap=*(parser->getTimestamps());
    if (!tsmap.count(ref_sys) || ! own_ts)
      return NAN;
    int64_t ref_ts=int64_t(tsmap.at(ref_sys).whiterabbit);
    return own_ts-ref_ts;
}

double HistogramAxisHandlers_evnt_delay(CalifaParser* parser,
                                 CalifaParser::module_index_t* idx)
{
#define LEAPSECONDS 37
  
  return double(time(NULL)+LEAPSECONDS) - parser->getLastTS()/1e9;
}
  
template<int ref_sys, typename valid_t=always >
double HistogramAxisHandlers_evnt_wrts_diff_ref_comp(CalifaParser* parser,
                                                     CalifaParser::module_index_t* idx)
{
  // compensate slave exploder offset
  double res=HistogramAxisHandlers_evnt_wrts_diff_ref<ref_sys, valid_t>(parser, idx);
#if 0 // should be fixed now!
  if (GET_SFP_PURE(*idx)>=2)
    res+=245;
#endif
  return res;
}

template<int sys1, int sys2>
double HistogramAxisHandlers_evnt_wrts_diff(CalifaParser* parser,
					    CalifaParser::module_index_t* idx)
{
  static int64_t last1=0;
  static int64_t last2=0;
  
  auto& tsmap=*(parser->getTimestamps());
  if (!tsmap.count(sys1) || !tsmap.count(sys2))
    return NAN;

  int64_t cur1=int64_t(tsmap.at(sys1).whiterabbit);
  int64_t cur2=int64_t(tsmap.at(sys2).whiterabbit);

#if 0 
  if (cur1==last1 && cur2==last2) //no new event concerning us
    return NAN;

  if (last1<last2 && cur1 < last2) //no leapfrog
    return NAN;

  if (last2<last1 && cur2 < last1) //no leapfrog
    return NAN;
#endif 
  last1=cur1;
  last2=cur2;
  return (double)(cur1-cur2);
}

template<int sys1>
double HistogramAxisHandlers_evnt_wrts_ms(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  static int64_t last1=0;
  auto& tsmap=*(parser->getTimestamps());
  if (!tsmap.count(sys1))
    return NAN;
   int64_t cur1=int64_t(tsmap.at(sys1).whiterabbit);
   if (cur1==last1 ) //no new event concerning us
    return NAN;
   last1=cur1;
   return (double)((cur1/1000000)%100000);
}

#define WRTSMS(name, sys) auto HistogramAxisHandlers_evnt_wrts_ms_ ## name =HistogramAxisHandlers_evnt_wrts_ms<sys>;

WRTSMS(mes, 0xa00)
WRTSMS(wix, 0xb00)
WRTSMS(main, 0x1000)

template<int sys1>
double HistogramAxisHandlers_evnt_wrts_skip(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  static int64_t last1=0;
  auto& tsmap=*(parser->getTimestamps());
  if (!tsmap.count(sys1))
    return NAN;
   int64_t cur1=int64_t(tsmap.at(sys1).whiterabbit);
   int64_t prev1=int64_t(tsmap.at(sys1).whiterabbit_prev);
   if (cur1==last1 ) //no new event concerning us
    return NAN;
   last1=cur1;
   return (double)(cur1-prev1);
}

#define WRTSSKIP(name, sys) auto HistogramAxisHandlers_evnt_wrts_skip_ ## name =HistogramAxisHandlers_evnt_wrts_skip<sys>;

WRTSSKIP(mes, 0xa00)
WRTSSKIP(wix, 0xb00)
WRTSSKIP(main, 0x1000)


template <int sfpNo>
double HistogramAxisHandlers_evnt_sfp_module(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  if (GET_SFP(*idx)!=sfpNo)
    return NAN;
  return GET_MOD(*idx);
}
;

double  HistogramAxisHandlers_evnt_xenergy(CalifaParser* parser, CalifaParser::module_index_t* idx) //energy: missing -> NAN. xenergy: missing -> 0
{
  GETPARSER
    if (!idx || !parser->getCalifaEvents()->count(*idx))
      return 0.0; 
  auto ei=parser->getCalifaEvents()->at(*idx);
  if (!ei.evnt)
    return 0.0;
  return ei.evnt->energy;
}


double  HistogramAxisHandlers_evnt_yenergy(CalifaParser* parser, CalifaParser::module_index_t* idx) //energy: overflow -> NAN
{
  GETPARSER
    if (!idx || !parser->getCalifaEvents()->count(*idx))
      return NAN; 
  auto ei=parser->getCalifaEvents()->at(*idx);
  if (!ei.evnt)
    return NAN;
  if (ei.evnt->overflow)
    return NAN;
  return ei.evnt->energy;
}


#define SFP_MODULE_ALIAS(n) double (*HistogramAxisHandlers_evnt_sfp ## n ##  _module)(CalifaParser*, CalifaParser::module_index_t*) =\
    HistogramAxisHandlers_evnt_sfp_module<n>

SFP_MODULE_ALIAS(0);
SFP_MODULE_ALIAS(1);
SFP_MODULE_ALIAS(2);
SFP_MODULE_ALIAS(3);

double HistogramAxisHandlers_evnt_channel(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  return GET_CH(*idx);
}

double HistogramAxisHandlers_evnt_pc_channel(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  //printf("%d.x.%d -> %d\n", GET_SFP(*idx), GET_CH(*idx),  (GET_SFP(*idx)/10-1)*20 + GET_CH(*idx));
  return (GET_SFP(*idx)/10-1)*20 + GET_CH(*idx);
}

/*double HistogramAxisHandlers_evnt_one(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  return 1;
  }*/




double HistogramAxisHandlers_evnt_PA_ch(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  return febex2preamp(GET_CH(*idx));
}

#define GET_TF1(idx, err_ret) const TF1* f=EnergyCal::getCal(idx); if (!f) return err_ret;

/*double  HistogramAxisHandlers_evnt_cal_en(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  GET_TF1(*idx, NAN);
  return f->Eval(evnt->energy+((double)rand())/RAND_MAX-0.5);
  }*/

double HistogramAxisHandlers_evnt_pulser(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  static const CalifaParser::module_index_t idx0=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 8, 4);
  if (!parser->getCalifaEvents()->count(idx0))
    return NAN;
  // no pulser

  {
    auto ei=parser->getCalifaEvents()->at(idx0);
    if (!ei.evnt)
      return NAN;
  }

  //  if (GET_CH(*idx)==0) // only count channel 0 of each febex
  //  return NAN;

  if (!idx || !parser->getCalifaEvents()->count(*idx))
    return 0;
  auto ei=parser->getCalifaEvents()->at(*idx);
  if (!ei.evnt) 
    return 0;
  auto evnt=ei.evnt;
  uint16_t en=evnt->energy;
  return 1+(5000<en) ;
}

double HistogramAxisHandlers_evnt_sfp_mod(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  if (!idx)
    return NAN;
  return (GET_SFP(*idx)%10)*20+GET_MOD(*idx);
}

double HistogramAxisHandlers_evnt_weird_sfp_mod(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  if (!idx || GET_SFP(*idx)/10!=7)
    return NAN;
  
  return (GET_SFP(*idx)%10)*20+GET_MOD(*idx);
}

double HistogramAxisHandlers_evnt_pc_sfp_mod(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  if (!idx)
    return NAN;
  return (GET_SFP(*idx)>=20)*80+(GET_SFP(*idx)%10)*20+GET_MOD(*idx);
}

double HistogramAxisHandlers_evnt_ts_diff(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  static const CalifaParser::module_index_t idx0=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 0, 0);
  if (!idx || !parser->getCalifaEvents()->count(*idx)   || !parser->getCalifaEvents()->count(idx0))
    return NAN;
  static uint64_t ts0;
  uint64_t ts1;
  {
    auto ei=parser->getCalifaEvents()->at(*idx);
    if (!ei.evnt)
      return NAN;
    //    if (ei.evnt->energy < 5000)
    //  return NAN;
    ts1= ei.evnt->timestamp;
  }
  {
    auto ei=parser->getCalifaEvents()->at(idx0);
    if (!ei.evnt)
      return NAN;
    //if (ei.evnt->energy < 5000)
    //  return NAN;

    ts0= ei.evnt->timestamp;
  }
  return (int64_t)ts1-(int64_t)ts0;
}


double HistogramAxisHandlers_evnt_cal_en(CalifaParser* parser, CalifaParser::module_index_t* idx)
{

  //assert(GET_CH(*idx)==IDX_CHANNEL_WILDCARD);
  if (!parser->getCalifaEvents()->count(*idx))
      return NAN;
  return (*(parser->getCalifaEvents()))[*idx].calEnergy;
}

#endif  // NEED_BODIES //////////////////////////////////////////////////////


#define WRTSDIFF(name, pos, neg) ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_ ## name =HistogramAxisHandlers_evnt_wrts_diff<pos,neg>;) DECLARE_HISTAXIS(full, wrts_diff_##name, 10000, -100000, 100000); DECLARE_HISTAXIS(lim, wrts_diff_##name, 8000, 0, 8000);

WRTSDIFF(mes_wix,  WRTS_MES,  WRTS_WIX);
WRTSDIFF(wix_mes,  WRTS_WIX,  WRTS_MES);
WRTSDIFF(main_mes, WRTS_MAIN, WRTS_WIX);
WRTSDIFF(main_wix, WRTS_MAIN, WRTS_WIX);
WRTSDIFF(mes_sync,  0xf0d0, 0xf0d4);

ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main = HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN>;)
ONBODY(auto HistogramAxisHandlers_evnt_dr_gamma = HistogramAxisHandlers_evnt_dr<+1, 0>;)

#define LIM_WRTS_DIFF_MAX 8000

#define WRTS_DIFF_MAIN_IMPL(cond) ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_##cond=HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN, cond>;)  ; DECLARE_HISTAXIS(lim, wrts_diff_main_##cond, LIM_WRTS_DIFF_MAX, 0, LIM_WRTS_DIFF_MAX) ; DECLARE_HISTAXIS(full, wrts_diff_main_##cond, 10000, -100000, 100000)
#define WRTS_DIFF_MAIN(cond) WRTS_DIFF_MAIN_IMPL(cond) ; WRTS_DIFF_MAIN_IMPL(cond##_he) 
WRTS_DIFF_MAIN(nonspecial)
WRTS_DIFF_MAIN(mes_nonspecial)
WRTS_DIFF_MAIN(wix_nonspecial)
WRTS_DIFF_MAIN(special)
WRTS_DIFF_MAIN(mes_special)
WRTS_DIFF_MAIN(wix_special)
WRTS_DIFF_MAIN(master)
WRTS_DIFF_MAIN(slave)

DECLARE_HISTAXIS(lim, wrts_diff_main, LIM_WRTS_DIFF_MAX,  0, LIM_WRTS_DIFF_MAX);
DECLARE_HISTAXIS(full, wrts_diff_main, 10000, 0, 100000);
//DECLARE_HISTAXIS(lim, wrts_diff_main_master, 4000, 0, 4000);
//DECLARE_HISTAXIS(lim, wrts_diff_main_slave,  4000, 0, 4000);

//DECLARE_HISTAXIS(full, wrts_diff_califa_ams, 2000, -100000, 100000);
//DECLARE_HISTAXIS(full, wrts_diff_main_ams, 2000, -100000, 100000);
//DECLARE_HISTAXIS(full, wrts_diff_califa_main, 10000, -100000, 100000);
//DECLARE_HISTAXIS(lim, wrts_diff_califa_main, 4000, 0, 4000);

DECLARE_HISTAXIS(full, wrts_ms_mes, 100000, 0, 100000);
DECLARE_HISTAXIS(full, wrts_ms_wix, 100000, 0, 100000);
DECLARE_HISTAXIS(full, wrts_ms_main, 100000, 0, 100000);
//DECLARE_HISTAXIS(full, wrts_ms_ams, 100000, 0, 100000);

DECLARE_HISTAXIS(full, wrts_skip_mes, 10000, 0, 1000000);
DECLARE_HISTAXIS(full, wrts_skip_wix, 10000, 0, 1000000);
DECLARE_HISTAXIS(full, wrts_skip_main,   10000, 0, 1000000);
//DECLARE_HISTAXIS(full, wrts_skip_ams,    10000, 0, 1000000);


DECLARE_HISTAXIS(full, energy, 65536, -32768, 32768);

DECLARE_HISTAXIS(full, trace_en, 1<<13, 0, (1<<15)-1);
DECLARE_HISTAXIS(full, trace_en_diff, 8000, -16000, 16000);
DECLARE_HISTAXIS(full, trace0, 1<<13, 0, 1<<14);
DECLARE_HISTAXIS(full, bl_slope, 200, -100, 100);
DECLARE_HISTAXIS(full, discr_amp, 4096, 0, 4096);
DECLARE_HISTAXIS(full, discr_max, 1000, 0, 1000);

DECLARE_HISTAXIS(full, trace_start, 1000, 0, 1000);
DECLARE_HISTAXIS(full, max_slope, 1<<13, 0, (1<<15)-1);


DECLARE_HISTAXIS(full2, energy, 1<<9, 0, 32768);
DECLARE_HISTAXIS(lim,  energy, 3000, 0, 6000);
DECLARE_HISTAXIS(lim2,  energy, 8000, 0, 8000);
DECLARE_HISTAXIS(lim2,  dr_gamma, 1<<9, 0, 32768);
//DECLARE_HISTAXIS(lim2,  energy, 4096, 0, 32768);
DECLARE_HISTAXIS(lim2,  yenergy, 4096, 0, 32768);

DECLARE_HISTAXIS(lim, xenergy, 4000, 0, 4000);

DECLARE_HISTAXIS(full, n_f, 65536, -32768, 32768);
DECLARE_HISTAXIS(rebinned512, n_f, 65536/512, -32768, 32768);

DECLARE_HISTAXIS(lim, n_f, 2500, 0, 10000);
DECLARE_HISTAXIS(lim, n_s, 2500, 0, 10000);

DECLARE_HISTAXIS(full, n_s, 65536, -32768, 32767);
DECLARE_HISTAXIS(rebinned512, n_s, 65536/512, -32768, 32768);

DECLARE_HISTAXIS(full, tot, 65536, -32768, 32768);
DECLARE_HISTAXIS(full, num_pileup, 100, 0, 100);
DECLARE_HISTAXIS(full, self_triggered, 256, 0, 256);
DECLARE_HISTAXIS(full, num_discarded, 100, 0, 100);

DECLARE_HISTAXIS(full,trace_sample,  1<<14, 0, 1<<14);
DECLARE_HISTAXIS(lim,trace_sample,  1<<8, 0, 1<<14);

DECLARE_HISTAXIS(fbx,sfp0_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp1_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp2_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp3_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,channel,     16, 0, 16);
DECLARE_HISTAXIS(fbx,pc_channel,   36, 0, 36);
DECLARE_HISTAXIS(fbx,overflow_worst, 8, 0, 8);
DECLARE_HISTAXIS(mesytec,PA_ch,   16, 1, 17);
DECLARE_HISTAXIS(coinc,pulser, 3, 0, 3);
DECLARE_HISTAXIS(coinc,ts_diff, 420*2, -420, 420);
DECLARE_HISTAXIS(coinc,sfp_mod, 80, 0, 80);
DECLARE_HISTAXIS(coinc,pc_sfp_mod, 160, 0, 160);
DECLARE_HISTAXIS(coinc,weird_sfp_mod, 80, 0, 80);

DECLARE_HISTAXIS(coinc,multiplicity, 5000, 0, 5000);

//DECLARE_HISTAXIS(coinc,abs_ch, 80*16, 0, 80*16);

DECLARE_HISTAXIS(fbx, sfp0_module_dual, 20, 0, 20);
DECLARE_HISTAXIS(sum,cal_en, 16000, 0, 16000);


DECLARE_HISTAXIS(full, delay, 5000, 0, 5000);


// calibrated energy is a special case, as the range is dependent on the channel calibration
HistogramAxis* createCalEnergyAxis(CalifaParser::module_index_t idx)
#if NEED_BODIES
{
  GET_TF1(idx, NULL);
  static const CalifaParser::module_index_t idx0=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 8, 0);
  HistogramAxis* h=(HistogramAxis*) malloc(sizeof(HistogramAxis));
  double slope=f->Eval(1) - f->Eval(0);
  double offset=f->Eval(0);
  double minRawBin=0; //(idx==idx0)?0:100;
  double maxEnergy0=3e3;//16e3;
  int rescale=1; //(idx==idx0)?1:5;
  double maxRawBin=floor(maxEnergy0/slope/rescale)*rescale;
  double minEnBin=f->Eval(minRawBin);
  double maxEnBin=floor(f->Eval(maxRawBin));
  int nBins=(maxRawBin-minRawBin)/rescale;
  //HistogramAxis tmp={"cal_energy", 65536, f->Eval(-32768), f->Eval(32768), HistogramAxisHandlers_evnt_cal_en};
  // HistogramAxis tmp={"cal_energy", 1000, 100, 6100, HistogramAxisHandlers_evnt_cal_en};
  HistogramAxis tmp={"cal_energy", nBins, minEnBin, maxEnBin, HistogramAxisHandlers_evnt_cal_en};
  *h=tmp;
  return h;
}
#endif
;

// alternately, a version with a fixed 
DECLARE_HISTAXIS(lim,cal_en, 4000, 0, 2.0);

#endif
