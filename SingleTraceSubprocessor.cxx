#include "SingleTraceSubprocessor.h"

SingleTraceSubprocessor::SingleTraceSubprocessor(char* name,
						 module_index_t idx,
						 int nbins,
						 double upperLimit, 
						 double lowerLimit)
  : SingleChannelSubprocessor(name, idx, nbins, upperLimit, lowerLimit)
{
}

void SingleTraceSubprocessor::processSubevent(CalifaParser::module_index_t cidx, eventinfo_t ei)
{
  if (ei.trace )
    {
      this->h->Reset();
      for (unsigned int i=1; i<=ei.tracepoints; i++)
	{
	  this->h->SetBinContent(i, getTracePoint(ei.trace, i));
	}
      this->h->GetXaxis()->SetRangeUser(0, ei.tracepoints);
      char buf[100];
      snprintf(buf, 100, "trace %d.%d.%d en=%d, q_f=%d, q_s=%d, type %d @0x%lx, pileup=%d, discr_amp=%f",
               GET_SFP(cidx), GET_MOD(cidx), GET_CH(cidx),
               ei.evnt->energy, ei.evnt->n_f, ei.evnt->n_s, ei.trace->type, ei.evnt->timestamp, ei.evnt->num_pileup, ei.discr_amp);
      this->h->SetTitle(buf);
    }
}
