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
  auto ei=parser->getCalifaEvents()->at(*idx);
#define GETEVNT GETEVENTINFO              \
  if (!ei.evnt)                           \
    return NAN;                           \
  auto evnt=ei.evnt;
#define DECLARE_EVNT(name) double  HistogramAxisHandlers_evnt_##name(CalifaParser* parser, CalifaParser::module_index_t* idx) EVNT_IMPL(name)
#if NEED_BODIES
#define EVNT_IMPL(name)   { GETEVNT; return evnt->name; }
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) HistogramAxis axis_ ## prefix ## _ ## name = {#prefix "_" #name, bins, min, max, HistogramAxisHandlers_evnt_##name, 0};
#define DECLARE_HISTAXIS2(name, ...) HistogramAxis axis_ ## name = {__VA_ARGS__} ;
#define ONBODY(...) __VA_ARGS__ 
#else
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) extern HistogramAxis axis_ ## prefix ## _ ## name;
#define DECLARE_HISTAXIS2(name, ...) extern HistogramAxis axis_ ## name ;
#define EVNT_IMPL(name)
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


DECLARE_EVNT(energy);
DECLARE_EVNT(n_f);
DECLARE_EVNT(n_s);
DECLARE_EVNT(tot);
DECLARE_EVNT(num_pileup);
DECLARE_EVNT(num_discarded);


#if NEED_BODIES //////////////////////////////////////////////////////

double HistogramAxisHandlers_evnt_sfp0_module_dual(CalifaParser* parser,
						     CalifaParser::module_index_t* idx)
{
  GETEVNT;
  uint16_t en=evnt->energy;
  return GET_MOD(*idx)+10*(en>5000 );
}

double HistogramAxisHandlers_evnt_multiplicity(CalifaParser* parser,
					       CalifaParser::module_index_t* idx)
{
  if (!parser)
    return NAN;
  return parser->getMultiplicity();
}



// this would be nicer with lambdas, but they can not be used as template arguments. :-/

// TODO: this is ugly. make cut subprocessors work instead!
#define MAKE_RESTRICTION(name, expr) struct name {  static bool apply(CalifaParser::module_index_t* idx)  { expr; };};

MAKE_RESTRICTION(always, return 1)
MAKE_RESTRICTION(master, return GET_SFP_PURE(*idx)<2  && GET_MOD(*idx)<16)
MAKE_RESTRICTION(slave, return GET_SFP_PURE(*idx)>=2 && GET_MOD(*idx)<16)
MAKE_RESTRICTION(special, return GET_MOD(*idx)>=16)

template<int ref_sys, typename valid_t=always >
double HistogramAxisHandlers_evnt_wrts_diff_ref(CalifaParser* parser,
                                                CalifaParser::module_index_t* idx)
{
    GETEVNT;
    if (!valid_t::apply(idx))
      return NAN;
    int64_t own_ts=parser->getLastTS();
    auto& tsmap=*(parser->getTimestamps());
    if (!tsmap.count(ref_sys) || ! own_ts)
      return NAN;
    int64_t ref_ts=int64_t(tsmap.at(ref_sys).whiterabbit);
    return own_ts-ref_ts;
}
  
  
template<int ref_sys, typename valid_t=always >
double HistogramAxisHandlers_evnt_wrts_diff_ref_comp(CalifaParser* parser,
                                                     CalifaParser::module_index_t* idx)
{
  // compensate slave exploder offset
  double res=HistogramAxisHandlers_evnt_wrts_diff_ref<ref_sys, valid_t>(parser, idx);
  if (GET_SFP_PURE(*idx)>=2)
    res+=245; 
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

WRTSMS(califa, 0x400)
WRTSMS(ams, 0x300)
WRTSMS(main, 0x100)

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

WRTSSKIP(califa, 0x400)
WRTSSKIP(ams, 0x300)
WRTSSKIP(main, 0x100)


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


#define WRTSDIFF(name, pos, neg) ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_ ## name =HistogramAxisHandlers_evnt_wrts_diff<pos,neg>;) DECLARE_HISTAXIS(full, wrts_diff_##name, 10000, -100000, 100000); DECLARE_HISTAXIS(lim, wrts_diff_##name, 4000, 0, 4000);

WRTSDIFF(mes_wix,  WRTS_MES,  WRTS_WIX);
WRTSDIFF(wix_mes,  WRTS_WIX,  WRTS_MES);
WRTSDIFF(main_mes, WRTS_MAIN, WRTS_WIX);
WRTSDIFF(main_wix, WRTS_MAIN, WRTS_WIX);

ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main = HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_master = HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN, master>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_slave = HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN, slave>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_special = HistogramAxisHandlers_evnt_wrts_diff_ref<WRTS_MAIN, special>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_comp = HistogramAxisHandlers_evnt_wrts_diff_ref_comp<WRTS_MAIN>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_comp_master = HistogramAxisHandlers_evnt_wrts_diff_ref_comp<WRTS_MAIN, master>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_comp_slave = HistogramAxisHandlers_evnt_wrts_diff_ref_comp<WRTS_MAIN, slave>;)
ONBODY(auto HistogramAxisHandlers_evnt_wrts_diff_main_comp_special = HistogramAxisHandlers_evnt_wrts_diff_ref_comp<WRTS_MAIN, special>;)

DECLARE_HISTAXIS(lim, wrts_diff_main, 4000, 0, 4000);
DECLARE_HISTAXIS(full, wrts_diff_main, 10000, 0, 100000);
DECLARE_HISTAXIS(lim, wrts_diff_main_master, 4000, 0, 4000);
DECLARE_HISTAXIS(lim, wrts_diff_main_slave,  4000, 0, 4000);
DECLARE_HISTAXIS(lim, wrts_diff_main_special,  4000, 0, 4000);
DECLARE_HISTAXIS(lim, wrts_diff_main_comp, 4000, 0, 4000);
DECLARE_HISTAXIS(full, wrts_diff_main_comp, 10000, 0, 100000);
DECLARE_HISTAXIS(lim, wrts_diff_main_comp_master, 4000, 0, 4000);
DECLARE_HISTAXIS(lim, wrts_diff_main_comp_slave,  4000, 0, 4000);
DECLARE_HISTAXIS(lim, wrts_diff_main_comp_special,  4000, 0, 4000);


//DECLARE_HISTAXIS(full, wrts_diff_califa_ams, 2000, -100000, 100000);
//DECLARE_HISTAXIS(full, wrts_diff_main_ams, 2000, -100000, 100000);
//DECLARE_HISTAXIS(full, wrts_diff_califa_main, 10000, -100000, 100000);
//DECLARE_HISTAXIS(lim, wrts_diff_califa_main, 4000, 0, 4000);

DECLARE_HISTAXIS(full, wrts_ms_califa, 100000, 0, 100000);
DECLARE_HISTAXIS(full, wrts_ms_main, 100000, 0, 100000);
DECLARE_HISTAXIS(full, wrts_ms_ams, 100000, 0, 100000);

DECLARE_HISTAXIS(full, wrts_skip_califa, 10000, 0, 1000000);
DECLARE_HISTAXIS(full, wrts_skip_main,   10000, 0, 1000000);
DECLARE_HISTAXIS(full, wrts_skip_ams,    10000, 0, 1000000);


DECLARE_HISTAXIS(full, energy, 65536, -32768, 32768);
DECLARE_HISTAXIS(lim,  energy, 4000, 0, 4000);
DECLARE_HISTAXIS(lim2,  energy, 8000, 0, 8000);
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
DECLARE_HISTAXIS(full, num_discarded, 100, 0, 100);

DECLARE_HISTAXIS(fbx,sfp0_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp1_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp2_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp3_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,channel,     16, 0, 16);
DECLARE_HISTAXIS(fbx,pc_channel,   36, 0, 36);
DECLARE_HISTAXIS(mesytec,PA_ch,   16, 1, 17);
DECLARE_HISTAXIS(coinc,pulser, 3, 0, 3);
DECLARE_HISTAXIS(coinc,ts_diff, 420*2, -420, 420);
DECLARE_HISTAXIS(coinc,sfp_mod, 80, 0, 80);
DECLARE_HISTAXIS(coinc,pc_sfp_mod, 160, 0, 160);
DECLARE_HISTAXIS(coinc,multiplicity, 5000, 0, 5000);

//DECLARE_HISTAXIS(coinc,abs_ch, 80*16, 0, 80*16);

DECLARE_HISTAXIS(fbx, sfp0_module_dual, 20, 0, 20);
DECLARE_HISTAXIS(sum,cal_en, 16000, 0, 16000);


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
