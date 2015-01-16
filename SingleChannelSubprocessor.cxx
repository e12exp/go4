#include "SingleChannelSubprocessor.h"
#include <HistFillerSubprocessor.h>
SingleChannelSubprocessor::SingleChannelSubprocessor(char* name,
						     module_index_t idx,
						     int nbins,
						     double upperLimit, 
						     double lowerLimit
						     )
  : SingleHistSubprocessor<TH1D, 1>(makeHistName(name, &idx), nbins, upperLimit, lowerLimit)
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
