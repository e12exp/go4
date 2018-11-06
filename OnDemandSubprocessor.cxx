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

#define IDX(A, B, C) CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, A, B, C)

void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  //char buf[1000];

  //CalifaParser::module_index_t x=IDX(0, 0, 0);
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts->begin(); i!=evts->end(); ++i)
    {
      auto idx=i->first;
      auto ei=i->second;
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  this->addChannel(p, idx, 0, true);
	}
    }
}


void OnDemandSubprocessor::addChannel(CalifaParser* p, CalifaParser::module_index_t idx, int tracepoints, bool recurse)
{
  if (GET_TYPE(idx)!=CalifaParser::subEventIdxType::fbxChannelIdx)
    return; //other indices are not ready yet
  
  //  static HistogramAxis axis080=	  *createCalEnergyAxis(IDX(0, 8, 0));

  static HistogramAxis qpid_axis[]={axis_full_n_f, axis_full_n_s};
  static HistogramAxis en2_axis[]={axis_full_energy, axis_full_energy};
  static HistogramAxis module_channel[]={axis_fbx_sfp0_module, axis_fbx_channel};
  static HistogramAxis module_mchannel[]={axis_fbx_sfp0_module, axis_mesytec_PA_ch};
  static HistogramAxis module_ts_diff[]={axis_coinc_abs_mod, axis_coinc_ts_diff};
  static HistogramAxis module_pulser[]={axis_fbx_sfp0_module, axis_coinc_pulser};

  static int globals_initialized;
  
  if (!globals_initialized)
    {
      auto any=IDX_ANY;
      // first energy histogram created, create now overview histogram
      new HistFillerSubprocessor<TH(1,I), 1>(&any, &axis_coinc_ts_diff);
      new HistFillerSubprocessor<TH(2,I), 1>(&any, module_channel);
      new HistFillerSubprocessor<TH(2,I), 1>(&any, module_mchannel);
      new HistFillerSubprocessor<TH(2,I), 1>(&any, module_ts_diff);
      new HistFillerSubprocessor<TH(2,I), 1>(&any, module_pulser);
      globals_initialized++;
    }
  
  printf("adding %d %d %d %d\n", GET_TYPE(idx), GET_SFP(idx), GET_MOD(idx), GET_CH(idx));


  auto &evts=*(p->getCalifaEvents());
  
  if (recurse && evts[idx].trace)
    tracepoints=evts[idx].tracepoints;
    
  
  if (this->energy_subprocessors.count(idx)
      && (tracepoints==0 || this->trace_subprocessors.count(idx)))
    return; // everything required was already created

  if (GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx
      || GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx)
    {
      this->energy_subprocessors[idx]=new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_sum_cal_en);
      
      if (GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx)
	{
	  //	  static HistogramAxis en2_axis[]={axis_sum_cal_en, axis080};
	  CalifaParser::module_index_t idxes[2]={idx, IDX(0, 8, 0)};
	  //new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 16);
	  if (GET_PETAL(idx)==0)
	    {
	      	  static HistogramAxis en2_axis[]={axis_sum_cal_en, axis_sum_cal_en};
		  CalifaParser::module_index_t idxes[2]={idx, std::make_tuple(CalifaParser::subEventIdxType::petalIdx, 1, 254, 254)};
		  new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 16);
	    }
	}

      return;
    }


  if (recurse)
    {

      // for (uint8_t sfp=0; sfp<=GET_SFP(idx); sfp++)
      {
	// add one channel for each module before, so the channel dirs get created in right order
	uint8_t sfp=GET_SFP(idx);
	for (uint8_t module=0; module<GET_MOD(idx); module++)
	  {
	    CalifaParser::module_index_t i=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, sfp, module, (uint8_t)0);
	    // petal
	    {
	      auto i2=CalifaParser::toIdxType(CalifaParser::subEventIdxType::petalIdx, i);
	      printf("--->adding %d %d %d %d\n", GET_TYPE(i2), GET_SFP(i2), GET_MOD(i2), GET_CH(i2));
	      this->addChannel(p, i2, tracepoints);
	    }
	    // febex mod
	    {
	      auto i2=CalifaParser::toIdxType(CalifaParser::subEventIdxType::fbxModuleIdx, i);
	      printf("--->adding %d %d %d %d\n", GET_TYPE(i2), GET_SFP(i2), GET_MOD(i2), GET_CH(i2));
	      this->addChannel(p, i2, tracepoints);
	    }
	    this->addChannel(p, i, tracepoints);
	  }
      }
      
      {
	// add previous channels for current module for correct order
	uint8_t sfp= GET_SFP(idx);
	uint8_t module=GET_MOD(idx);
	for (uint8_t ch=0; ch<GET_CH(idx); ch++)
	  {
	    CalifaParser::module_index_t i=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, sfp, module, ch);
	    this->addChannel(p, i, tracepoints);
	  }
      }
    }
    


  std::list<CalifaParser::module_index_t> l={IDX(0, 0, 1)};
  static std::map<CalifaParser::module_index_t, std::list<CalifaParser::module_index_t> > correlations=
    {
      //      {IDX(0, 0, 0), {IDX(0, 0, 1)}},
      // {IDX(0, 0, 1), {IDX(0, 0, 2)}},
      {IDX(0, 8, 0),
       {
	 IDX(0, 4, 0),
	 IDX(0, 4, 1)
       }}, 
      //{IDX(0, 0, 3), {IDX(0, 0, 6)}},
      //{IDX(0, 0, 4), {IDX(0, 0, 5)}},
      //{IDX(0, 0, 5), {IDX(0, 0, 6)}},
      {IDX(0, 6, 4), {IDX(0, 5, 10)}},
      {IDX(0, 0, 11), {IDX(0, 1, 5), IDX(0, 0, 15), IDX(0, 8, 9)}}
    };


  if (!this->energy_subprocessors.count(idx))
    {
  
      //we have found an event without an energy histogram,
      //create one. 
      this->energy_subprocessors[idx]=new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy);
	//	new ChannelCutHistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy, 1,
	//						 &idx, &axis_full_energy);
      new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_lim_energy);
      //new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy);
      ldbg("created a new energy processor for %d:%d:%d.\n", 
	   GET_SFP(idx), GET_MOD(idx), 
	   GET_CH(idx));
      //new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64);
  
      if (HistogramAxis* ha=createCalEnergyAxis(idx))
	{
	  new HistFillerSubprocessor<TH(1,I), 1>(&idx, ha);
	  ldbg(" also created a calibrated energy processor\n");
	  //  new ChannelCutHistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64,
	  //						       &idx, ha, 30.0, 8000.0);
	}
      //new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64);
      if (correlations.count(idx))
	for (auto m=correlations[idx].begin(); m!=correlations[idx].end(); ++m)
	  {
	    CalifaParser::module_index_t idxes[2]={idx, *m};
	    new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 30);
	  }
    }


  if (tracepoints!=0 && ! this->trace_subprocessors.count(idx))
    {
      //we have traces, create trace subprocessors
      //
      //single traces
      {
	SingleTraceSubprocessor* tp=
	  new SingleTraceSubprocessor("trace_last", idx, tracepoints,
				      tracepoints);
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
					idx, tracepoints,
					F_ADC/2);
      }

    }
}
