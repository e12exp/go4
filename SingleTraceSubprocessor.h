#ifndef SingleTraceSubprocessor_h
#define SingleTraceSubprocessor_h
#include "SingleChannelSubprocessor.h"
class SingleTraceSubprocessor : public SingleChannelSubprocessor
{
 public:
  SingleTraceSubprocessor(char* name,
		     module_index_t idx,
		     int nbins,
		     double upperLimit, 
		     double lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
};
#endif
