#include "OnDemandSubprocessor.h"
#include "EnergySubprocessor.h"
#include "SingleTraceSubprocessor.h"
#include "FourierSpectrumSubprocessor.h"
#include "CalifaConfig.h"

OnDemandSubprocessor::OnDemandSubprocessor(): energy_subprocessors(), trace_subprocessors()
{

}

void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  char buf[1000];
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts->begin(); i!=evts->end(); ++i)
    {
      auto idx=i->first;
      auto ei=i->second;
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  //we have found an event without an energy histogram,
	  //create one. 
	  snprintf(buf, 1000, "energy/sfp_%01d/febex_%02d/en_%01d_%02d_%02d", 
		   std::get<0>(idx), std::get<1>(idx),
		   std::get<0>(idx), std::get<1>(idx),
		   std::get<2>(idx));
	  this->energy_subprocessors[idx]=
	    new EnergySubprocessor(buf, idx, 100000, 100000);
	  ldbg("created a new energy processor with name %s.\n", buf);
	}
      if (ei.trace && ! this->trace_subprocessors.count(idx))
	{
	  //single traces
	  {
	    snprintf(buf, 1000, 
		     "traces/last/sfp_%01d/febex_%02d/tr_%01d_%02d_%02d",
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<2>(idx));
	    SingleHistSubprocessor* tp=
	      new SingleTraceSubprocessor(buf, idx, ei.tracepoints,
					  ei.tracepoints*1000/F_ADC);
	    tp->getHist()->GetXaxis()->SetTitle("time in ns");
	    snprintf(buf, 1000, 
		     "tr%01d_%02d_%02d - type %d",
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<2>(idx), ei.trace->type);
	    tp->getHist()->SetTitle(buf);
	    this->trace_subprocessors[idx]=tp;
	  }
	  {
	    snprintf(buf, 500, 
		     "traces/fft_amp/sfp_%01d/febex_%02d/fft_amp_%01d_%02d_%02d",
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<2>(idx));
	    snprintf(buf+500, 500, 
		     "traces/fft_phase/sfp_%01d/febex_%02d/fft_phase_%01d_%02d_%02d",
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<0>(idx), std::get<1>(idx), 
		     std::get<2>(idx));
	    FourierSpectrumSubprocessor *fftp=
	      new FourierSpectrumSubprocessor(buf, buf+500, 
					      idx, ei.tracepoints,
					      F_ADC/2);
	  }

	}
    }
}
