#ifndef EnergySubprocessor_h
#define EnergySubprocessor_h
#include "SingleChannelSubprocessor.h"
class EnergySubprocessor : public SingleChannelSubprocessor
{
 public:
  EnergySubprocessor(std::string name,
		     module_index_t idx,
		     int nbins,
		     int upperLimit, 
		     int lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
};
#endif
