#ifndef _ChannelCutSubprocessor_h
#define _ChannelCutSubprocessor_h
#include "HistogramAxis.h"
#include <math.h>

#include "CalifaSubprocessor.h"
class ChannelCutSubprocessor:
public virtual CalifaSubprocessor
{
 public:
  ChannelCutSubprocessor(CalifaParser::module_index_t* cut_idx, HistogramAxis* h, double cutMin=-INFINITY, double cutMax=INFINITY);
  virtual bool cut(CalifaParser* p);
  double (*getValue)(CalifaParser* parser, CalifaParser::module_index_t* idx);
 protected:
  double cutMin;
  double cutMax;
  CalifaParser::module_index_t cut_idx;

  virtual CalifaParser::module_index_t getSensitivity()
  {
    return cut_idx; 
  }

};

#endif
