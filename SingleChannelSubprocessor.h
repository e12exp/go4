#ifndef _SingleChannelSubprocessor_H
#define _SingleChannelSubprocessor_H
#include "HistogramAxis.h"

#include "SingleHistSubprocessor.h"
class SingleChannelSubprocessor: public SingleHistSubprocessor<TH1D, 1>
{
 public:
  //  typedef CalifaParser::module_index_t module_index_t;
  //old constructor: overload processSubevent
  SingleChannelSubprocessor(char* name,
			    module_index_t idx,
			    int nbins,
			    double upperLimit, 
			    double lowerLimit);


  virtual void processEvent(CalifaParser* p);
  virtual void processSubevent(eventinfo_t ei){};
 protected:
  module_index_t idx;
};
#endif
