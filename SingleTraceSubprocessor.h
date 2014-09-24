#ifndef SingleTraceSubprocessor_h
#define SingleTraceSubprocessor_h
#include "SingleChannelSubprocessor.h"
class SingleTraceSubprocessor : public SingleChannelSubprocessor
{
 public:
  SingleTraceSubprocessor(std::string name,
		     module_index_t idx,
		     int nbins,
		     int upperLimit, 
		     int lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
};
#endif
