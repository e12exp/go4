#include "SingleTraceSubprocessor.h"

SingleTraceSubprocessor::SingleTraceSubprocessor(std::string name,
						 module_index_t idx,
						 int nbins,
						 int upperLimit, 
						 int lowerLimit)
  : SingleChannelSubprocessor(name, idx, nbins, upperLimit, lowerLimit)
{
}

void SingleTraceSubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.trace)
    {
      this->h->Reset();
      for (unsigned int i=1; i<=ei.tracepoints; i++)
	{
	  this->h->SetBinContent(i, (double)(ei.trace->points[i]));
	}
    }
}
