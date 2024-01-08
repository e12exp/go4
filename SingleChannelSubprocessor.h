#ifndef _SingleChannelSubprocessor_H
#define _SingleChannelSubprocessor_H
#include "HistogramAxis.h"

#include "SingleHistSubprocessor.h"
class SingleChannelSubprocessor: public SingleHistSubprocessor<TH1D, 1>
{
 public:
  //  typedef module_index_t module_index_t;
  //old constructor: overload processSubevent
  SingleChannelSubprocessor(char* name,
			    module_index_t idx,
			    int nbins,
			    double upperLimit, 
			    double lowerLimit);


  virtual void processEvent(CalifaParser* p);
  virtual void processSubevent(module_index_t cidx, eventinfo_t ei) = 0;
 protected:

  virtual module_index_t getSensitivity()
  {
    return idx; 
  }

  module_index_t idx;
};

#endif
