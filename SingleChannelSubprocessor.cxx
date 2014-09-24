#include "SingleChannelSubprocessor.h"

SingleChannelSubprocessor::SingleChannelSubprocessor(std::string name,
						     module_index_t idx,
						     int nbins,
						     int upperLimit, 
						     int lowerLimit
						     )
  : SingleHistSubprocessor(name, nbins, upperLimit, lowerLimit)
{
  this->idx=idx;
}


void SingleChannelSubprocessor::processEvent(CalifaParser* p)
{
  auto evmap=p->getCalifaEvents();
  CalifaParser::eventmap_t::iterator evit;
  if (( evit=evmap->find(this->idx)) != evmap->end())
    {
      this->processSubevent(evit->second);
    }
}
