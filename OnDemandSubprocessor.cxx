#include "OnDemandSubprocessor.h"
//#include "EnergySubprocessor.h"
//#include "CalEnergySubprocessor.h"
#include "SingleTraceSubprocessor.h"
#include "FourierSpectrumSubprocessor.h"
#include "CalifaConfig.h"
#include "BaselineHistSubprocessor.h"
#include "HistogramAxis.h"
#include "HistFillerSubprocessor.h"

OnDemandSubprocessor::OnDemandSubprocessor(): energy_subprocessors(), trace_subprocessors()
{

}

void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  //char buf[1000];
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts->begin(); i!=evts->end(); ++i)
    {
      auto idx=i->first;
      auto ei=i->second;
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  //we have found an event without an energy histogram,
	  //create one. 
	  this->energy_subprocessors[idx]=new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_energy);
	  ldbg("created a new energy processor for %d:%d:%d.\n", 
	       std::get<0>(idx), std::get<1>(idx), 
	       std::get<2>(idx));

	  if (HistogramAxis* ha=createCalEnergyAxis(idx))
	    {
	      new HistFillerSubprocessor<TH(1,I), 1>(&idx, ha);
	      ldbg(" also created a calibrated energy processor\n");
	    }
	}
      if (ei.trace && ! this->trace_subprocessors.count(idx))
	{
	  //we have traces, create trace subprocessors
	  //
	  //single traces
	  {
	    SingleTraceSubprocessor* tp=
	      new SingleTraceSubprocessor("trace_last", idx, ei.tracepoints,
					  ei.tracepoints);
	    tp->getHist()->GetXaxis()->SetTitle("trace point index");
	    this->trace_subprocessors[idx]=tp;
	  }
	  // baseline hist processor
	  {
	    //	    BaselineHistSubprocessor* bhp=
	      new BaselineHistSubprocessor("baseline", idx);
	  }
	  //FFT
	  {
	    //	    FourierSpectrumSubprocessor *fftp=
	    new FourierSpectrumSubprocessor("fft_amp", "fft_phase", 
					      idx, ei.tracepoints,
					      F_ADC/2);
	  }

	}
    }
}
