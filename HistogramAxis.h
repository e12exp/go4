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
struct HistogramAxis
{
  char descr[50];
  int nBins;
  double min;
  double max;
  double (*getValue)(CalifaParser* parser, CalifaParser::module_index_t* idx);
  int is_one;
};

#define GETPARSER if (!parser ) return NAN;
#define GETEVENTINFO GETPARSER if (!idx || !parser->getCalifaEvents()->count(*idx)) return NAN; \
  auto ei=parser->getCalifaEvents()->at(*idx);
#define GETEVNT GETEVENTINFO if (!ei.evnt) return NAN; auto evnt=ei.evnt;
#define DECLARE_EVNT(name) double  HistogramAxisHandlers_evnt_##name(CalifaParser* parser, CalifaParser::module_index_t* idx) EVNT_IMPL(name)
#if NEED_BODIES
#define EVNT_IMPL(name)   { GETEVNT; return evnt->name; }
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) HistogramAxis axis_ ## prefix ## _ ## name = {#prefix "_" #name, bins, min, max, HistogramAxisHandlers_evnt_##name, 0};
#define DECLARE_HISTAXIS2(name, ...) HistogramAxis axis_ ## name = {__VA_ARGS__} ;
#else
#define DECLARE_HISTAXIS(prefix, name, bins, min, max) extern HistogramAxis axis_ ## prefix ## _ ## name;
#define DECLARE_HISTAXIS2(name, ...) extern HistogramAxis axis_ ## name ;
#define EVNT_IMPL(name)
#endif



DECLARE_EVNT(energy);
DECLARE_EVNT(n_f);
DECLARE_EVNT(n_s);
DECLARE_EVNT(tot);
DECLARE_EVNT(num_pileup);
DECLARE_EVNT(num_discarded);

//DECLARE_HISTAXIS(short, wrts, 0, 0x10000)


//DECLARE_HISTAXIS2(weight_one, "weight_one",1,0,1,HistogramAxisHandlers_evnt_one, 1) ;


/*template<HistogramAxis CutAxis, HistogramAxis >
constexpr HistogramAxis cut(HistogramAxis cutaxis, std::function<bool(double)> cutfun,
			    HistogramAxis inner)
{
  axis_ ## prefix ## _ ## name = {#prefix "_" #name, bins, min, max, HistogramAxisHandlers_evnt_##name, 0};
  }*/


#if NEED_BODIES //////////////////////////////////////////////////////
/*
double HistogramAxisHandlers_evnt_wrts)
{
  
}
*/
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


double HistogramAxisHandlers_evnt_psp_sum(CalifaParser* parser,
					  CalifaParser::module_index_t* idx)
{
  static const CalifaParser::module_index_t idx1=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 0, 6);
  static const CalifaParser::module_index_t idx2=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 3, 6);

  GETPARSER
  if ( !parser->getCalifaEvents()->count(idx1)) return NAN;
  auto ei1=parser->getCalifaEvents()->at(idx1);
  if (!ei1.evnt) return NAN; auto evnt1=ei1.evnt;

  if (!parser->getCalifaEvents()->count(idx2)) return NAN;
  auto ei2=parser->getCalifaEvents()->at(idx2);
  if (!ei2.evnt) return NAN; auto evnt2=ei2.evnt;
  return evnt1->energy+evnt2->energy;
}

double HistogramAxisHandlers_evnt_psp_diff(CalifaParser* parser,
					  CalifaParser::module_index_t* idx)
{
  static const CalifaParser::module_index_t idx1=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 0, 6);
  static const CalifaParser::module_index_t idx2=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 3, 6);
  static const CalifaParser::module_index_t idx3=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 4, 4);
  static const CalifaParser::module_index_t idx4=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, 0, 7, 4);

    
  GETPARSER
  if ( !parser->getCalifaEvents()->count(idx1)) return NAN;
  auto ei1=parser->getCalifaEvents()->at(idx1);
  if (!ei1.evnt) return NAN; auto evnt1=ei1.evnt;

  if (!parser->getCalifaEvents()->count(idx2)) return NAN;
  auto ei2=parser->getCalifaEvents()->at(idx2);
  if (!ei2.evnt) return NAN; auto evnt2=ei2.evnt;

  if (!parser->getCalifaEvents()->count(idx3)) return NAN;
  if (!parser->getCalifaEvents()->count(idx4)) return NAN;
  return evnt1->energy-evnt2->energy;
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

#define WRTSDIFF(name, pos, neg) auto HistogramAxisHandlers_evnt_wrts_diff_ ## name =HistogramAxisHandlers_evnt_wrts_diff<pos,neg>; DECLARE_HISTAXIS(full, wrts_diff_##name, 10000, -100000, 100000); DECLARE_HISTAXIS(lim, wrts_diff_##name, 4000, 0, 4000);

// for trig 1:
#define WRTS_MES      0x0a00
#define WRTS_WIX      0x0b00
#define WRTS_MAIN     0x1000
#define TRIG3_OFFSET 0x30000
WRTSDIFF(mes_wix,  WRTS_MES,  WRTS_WIX);
WRTSDIFF(main_mes, WRTS_MAIN, WRTS_WIX);
WRTSDIFF(main_wix, WRTS_MAIN, WRTS_WIX);
WRTSDIFF(t3_mes_wix,  WRTS_MES+TRIG3_OFFSET,  WRTS_WIX+TRIG3_OFFSET);
WRTSDIFF(t3_main_mes, WRTS_MAIN+TRIG3_OFFSET, WRTS_WIX+TRIG3_OFFSET);
WRTSDIFF(t3_main_wix, WRTS_MAIN+TRIG3_OFFSET, WRTS_WIX+TRIG3_OFFSET);

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

double HistogramAxisHandlers_evnt_abs_mod(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  if (!idx)
    return NAN;
  return (GET_SFP(*idx)%10)*20+GET_MOD(*idx);
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
/*
template<int petalNo>
double HistogramAxisHandlers_evnt_petal_(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  
}

#define TODO(n) double (*HistogramAxisHandlers_evnt_petal ## n ##  _)(CalifaParser*, CalifaParser::module_index_t*) =\
    HistogramAxisHandlers_evnt_sfp_module<n>
*/

#endif  // NEED_BODIES //////////////////////////////////////////////////////

// axis_fbx_sfp0_module filters for sfp0 and returns the module number.
// 

DECLARE_HISTAXIS(hack, psp_sum, 30000, 0, 30000);
DECLARE_HISTAXIS(hack, psp_diff, 30000, -15000, 15000);

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
DECLARE_HISTAXIS(lim2,  energy, 4096, 0, 32768);
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
DECLARE_HISTAXIS(coinc,abs_mod, 80, 0, 80);
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
