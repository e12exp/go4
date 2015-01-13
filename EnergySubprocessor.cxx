#include "EnergySubprocessor.h"


EnergySubprocessor::EnergySubprocessor(std::string name,
				       module_index_t idx,
				       int nbins,
				       double upperLimit, 
				       double lowerLimit)
  : SingleChannelSubprocessor(name, idx, nbins, upperLimit, lowerLimit)
{
  //nothing to see here, move along. 
}

void EnergySubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.evnt)
    this->h->Fill(ei.evnt->energy);
}
