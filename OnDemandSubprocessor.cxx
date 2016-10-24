#include "OnDemandSubprocessor.h"
//#include "EnergySubprocessor.h"
//#include "CalEnergySubprocessor.h"
#include "SingleTraceSubprocessor.h"
#include "FourierSpectrumSubprocessor.h"
#include "CalifaConfig.h"
#include "BaselineHistSubprocessor.h"
#include "HistogramAxis.h"
#include "ChannelCutHistFillerSubprocessor.h"

OnDemandSubprocessor::OnDemandSubprocessor(): energy_subprocessors(), trace_subprocessors()
{

}

#define IDX(A, B, C) CalifaParser::module_index_t(A, B, C)

void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  //char buf[1000];
  static HistogramAxis qpid_axis[]={axis_full_n_f, axis_full_n_s};
  static HistogramAxis en2_axis[]={axis_lim_energy, axis_lim_energy};
  std::list<CalifaParser::module_index_t> l={IDX(0, 0, 1)};
  static std::map<CalifaParser::module_index_t, std::list<CalifaParser::module_index_t> > correlations=
    {
      {IDX(0, 0, 0), {IDX(0, 0, 1)}},
      {IDX(0, 0, 1), {IDX(0, 0, 2)}},
      {IDX(0, 0, 2), {IDX(0, 0, 3)}}, 
      {IDX(0, 0, 3), {IDX(0, 0, 4)}},
      {IDX(0, 0, 4), {IDX(0, 0, 5)}},
      {IDX(0, 0, 5), {IDX(0, 0, 6)}},
      {IDX(0, 0, 6), {IDX(0, 0, 7)}}
    };
  CalifaParser::module_index_t x=IDX(0, 0, 0);
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts->begin(); i!=evts->end(); ++i)
    {
      auto idx=i->first;
      auto ei=i->second;
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  //we have found an event without an energy histogram,
	  //create one. 
	  this->energy_subprocessors[idx]=
	    new ChannelCutHistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy, 1,
							     &idx, &axis_full_energy);

	  //new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy);
	  ldbg("created a new energy processor for %d:%d:%d.\n", 
	       std::get<0>(idx), std::get<1>(idx), 
	       std::get<2>(idx));
	  //new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64);
	  if (HistogramAxis* ha=createCalEnergyAxis(idx))
	    {
	      new HistFillerSubprocessor<TH(1,I), 1>(&idx, ha);
	      ldbg(" also created a calibrated energy processor\n");
	      //  new ChannelCutHistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64,
	      //						       &idx, ha, 30.0, 8000.0);
	    }
	  new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64);
	  if (correlations.count(idx))
	    for (auto m=correlations[idx].begin(); m!=correlations[idx].end(); ++m)
	    {
	      CalifaParser::module_index_t idxes[2]={idx, *m};
	      new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 10);
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
