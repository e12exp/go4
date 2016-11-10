#ifndef _HistogramAxis_h
#define _HistogramAxis_h
#include <math.h>
#include "struct_eventinfo.h"
#include "CalifaParser.h"
#include "EnergyCal.h"
#include "TF1.h"
#include "convert_idx.h"

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

DECLARE_HISTAXIS(full, energy, 65536, -32768, 32768);
DECLARE_HISTAXIS(lim, energy, 2000, 0, 2000);

DECLARE_HISTAXIS(full, n_f, 65536, -32768, 32768);
DECLARE_HISTAXIS(rebinned512, n_f, 65536/512, -32768, 32768);

DECLARE_HISTAXIS(full, n_s, 65536, -32768, 32767);
DECLARE_HISTAXIS(rebinned512, n_s, 65536/512, -32768, 32768);

DECLARE_HISTAXIS(full, tot, 65536, -32768, 32768);
DECLARE_HISTAXIS(full, num_pileup, 100, 0, 100);
DECLARE_HISTAXIS(full, num_discarded, 100, 0, 100);

#if NEED_BODIES //////////////////////////////////////////////////////

template <int sfpNo>
double HistogramAxisHandlers_evnt_sfp_module(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  if (std::get<0>(*idx)!=sfpNo)
    return NAN;
  return std::get<1>(*idx);
}
;

#define SFP_MODULE_ALIAS(n) double (*HistogramAxisHandlers_evnt_sfp ## n ##  _module)(CalifaParser*, CalifaParser::module_index_t*) =\
    HistogramAxisHandlers_evnt_sfp_module<n>

SFP_MODULE_ALIAS(0);
SFP_MODULE_ALIAS(1);
SFP_MODULE_ALIAS(2);
SFP_MODULE_ALIAS(3);

double HistogramAxisHandlers_evnt_channel(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  return std::get<2>(*idx);
}


/*double HistogramAxisHandlers_evnt_one(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  return 1;
  }*/




double HistogramAxisHandlers_evnt_PA_ch(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  return febex2preamp(std::get<2>(*idx));
}

#define GET_TF1(idx, err_ret) const TF1* f=EnergyCal::getCal(idx); if (!f) return err_ret;

double  HistogramAxisHandlers_evnt_cal_en(CalifaParser* parser, CalifaParser::module_index_t* idx)
{
  GETEVNT;
  GET_TF1(*idx, NAN);
  return f->Eval(evnt->energy);
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

DECLARE_HISTAXIS(fbx,sfp0_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp1_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp2_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,sfp3_module, 19, 0, 19);
DECLARE_HISTAXIS(fbx,channel,     16, 0, 16);
DECLARE_HISTAXIS(mesytec,PA_ch,   16, 1, 17);
//DECLARE_HISTAXIS2(weight_one, "weight_one",1,0,1,HistogramAxisHandlers_evnt_one, 1) ;

// calibrated energy is a special case, as the range is dependent on the channel calibration
HistogramAxis* createCalEnergyAxis(CalifaParser::module_index_t idx)
#if NEED_BODIES
{
  GET_TF1(idx, NULL);
  HistogramAxis* h=(HistogramAxis*) malloc(sizeof(HistogramAxis));
  HistogramAxis tmp={"cal_energy", 65536, f->Eval(-32768), f->Eval(32768), HistogramAxisHandlers_evnt_cal_en};
  *h=tmp;
  return h;
}
#endif
;

#endif
