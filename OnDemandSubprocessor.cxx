#include "OnDemandSubprocessor.h"
#include "SingleHistSubprocessor.h"


OnDemandSubprocessor::OnDemandSubprocessor(): energy_subprocessors(), trace_subprocessors()
{

}

virtual void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts.begin(), i!=evts.end(), ++i)
    {
      auto idx=i.first;
      auto ei=i.second;
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  //we have found an event without an energy histogram,
	  //create one. 
	  (new SingleHistSubprocessor(,
			      [idx](CalifaParser* p){return p->getCalifaEvents()[idx];},
			      100000, 100000));


	}
      if (ei.trace && ! this->trace_subprocessors.count(idx))
	{
	  //same for traces
	}
    }
  CalifaProc::getProc()->registerNewHistograms();
}
