#ifndef _HistogramAxis_h
#define _HistogramAxis_h
#include <math.h>
#include "struct_eventinfo.h"
#include "CalifaParser.h"
#include "EnergyCal.h"
#include "TF1.h"


struct HistogramAxis
{
  char descr[50];
  int nBins;
  double min;
  double max;
  double (*getValue)(CalifaParser* parser, CalifaParser::module_index_t* idx);
};

#define GETPARSER if (!parser ) return NAN;
#define GETEVENTINFO GETPARSER if (!idx || !parser->getCalifaEvents()->count(*idx)) return NAN; \
  auto ei=parser->getCalifaEvents()->at(*idx);
#define GETEVNT GETEVENTINFO if (!ei.evnt) return NAN; auto evnt=ei.evnt;
#define DECLARE_EVNT(name) double  HistogramAxisHandlers_evnt_##name(CalifaParser* parser, CalifaParser::module_index_t* idx) EVNT_IMPL(name)
#if NEED_BODIES
#define EVNT_IMPL(name)   { GETEVNT; return evnt->name; }
#define DECLARE_HISTAXIS(name, bins, min, max) HistogramAxis axis_##name = {#name, bins, min, max, HistogramAxisHandlers_evnt_##name};
#else
#define DECLARE_HISTAXIS(name, bins, min, max) extern HistogramAxis axis_##name;
#define EVNT_IMPL(name)
#endif

#define ADD_EVNT_AXIS(name, bins, min, max) DECLARE_EVNT(name); DECLARE_HISTAXIS(name, bins, min, max); 

ADD_EVNT_AXIS(energy, 65536, -32768, 32767);
ADD_EVNT_AXIS(n_f, 65536, -32768, 32767);
ADD_EVNT_AXIS(n_s, 65536, -32768, 32767);
ADD_EVNT_AXIS(tot, 65536, -32768, 32767);
ADD_EVNT_AXIS(num_pileup, 100, 0, 100);
ADD_EVNT_AXIS(num_discarded, 100, 0, 100);

#define GET_TF1(idx, err_ret) const TF1* f=EnergyCal::getCal(idx); if (!f) return err_ret;

double  HistogramAxisHandlers_evnt_cal_en(CalifaParser* parser, CalifaParser::module_index_t* idx)
#if NEED_BODIES
{
  GETEVNT;
  GET_TF1(*idx, NAN);
  return f->Eval(evnt->energy);
}
#endif
;
// calibrated energy is a special case, as the range is dependent on the channel calibration
HistogramAxis* createCalEnergyAxis(CalifaParser::module_index_t idx)
#if NEED_BODIES
{
  GET_TF1(idx, NULL);
  HistogramAxis* h=(HistogramAxis*) malloc(sizeof(HistogramAxis));
  HistogramAxis tmp={"cal_energy", 65536, f->Eval(-32768), f->Eval(32767), HistogramAxisHandlers_evnt_cal_en};
  *h=tmp;
  return h;
}
#endif
;

#endif
