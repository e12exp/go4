#include "SingleTraceSubprocessor.h"

SingleTraceSubprocessor::SingleTraceSubprocessor(char* name,
						 module_index_t idx,
						 int nbins,
						 double upperLimit, 
						 double lowerLimit)
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
	  this->h->SetBinContent(i, getTracePoint(ei.trace, i));
	}
      char buf[100];
      snprintf(buf, 100, "%s en=%d, q_f=%d, q_s=%d, type %d", h->GetName(), ei.evnt->energy, ei.evnt->n_f, ei.evnt->n_s, ei.trace->type);
      this->h->SetTitle(buf);
    }
}
