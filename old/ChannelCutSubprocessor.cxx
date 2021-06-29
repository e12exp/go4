#include "ChannelCutSubprocessor.h"

ChannelCutSubprocessor::ChannelCutSubprocessor(CalifaParser::module_index_t* cut_idx, HistogramAxis* h, double cutMin, double cutMax)
{
  this->cutMin=cutMin;
  this->cutMax=cutMax;
  this->cut_idx=*cut_idx;
  this->getValue=h->getValue;
}
bool ChannelCutSubprocessor::cut(CalifaParser* p)
{
  double v=this->getValue(p, &(this->cut_idx));
  return (v>=this->cutMin && v<=this->cutMax);
}
