#include "OnDemandSubprocessor.h"
//#include "EnergySubprocessor.h"
//#include "CalEnergySubprocessor.h"
#include "SingleTraceSubprocessor.h"
#include "FourierSpectrumSubprocessor.h"
#include "CalifaConfig.h"
#include "BaselineHistSubprocessor.h"
#include "HistogramAxis.h"
#include "HistFillerSubprocessor.h"
//#include "ChannelCutHistFillerSubprocessor.h"
OnDemandSubprocessor* OnDemandSubprocessor::inst=nullptr;

OnDemandSubprocessor::OnDemandSubprocessor(): energy_subprocessors(), trace_subprocessors()
{

  printf("Created OnDemandSubprocessor.*********************\n\n\n");
}


void OnDemandSubprocessor::processEvent(CalifaParser* p)
{
  //char buf[1000];

  //CalifaParser::module_index_t x=IDX(0, 0, 0);
  CalifaParser::eventmap_t* evts=p->getCalifaEvents();
  for (auto i=evts->begin(); i!=evts->end(); ++i)
    {
      auto idx=i->first;
      auto ei=i->second;

      int tracepoints=0;
      if (ei.trace)
        tracepoints=ei.tracepoints;
  
      if (ei.evnt && ! this->energy_subprocessors.count(idx))
	{
	  this->addChannel(p, idx, tracepoints, true);
	}
    }
}


void OnDemandSubprocessor::addChannel(CalifaParser* p, CalifaParser::module_index_t idx, int tracepoints, bool recurse)
{

  if (this->energy_subprocessors.count(idx)
      )//&& (tracepoints==0 || this->trace_subprocessors.count(idx)))
    return; // everything required was already created
  
  static int added=0;
  printf("%s: Adding Channel %d.%d.%d. (No %d)\n", __PRETTY_FUNCTION__, GET_SFP(idx), GET_MOD(idx), GET_CH(idx), added++);
  //if (GET_TYPE(idx)!=CalifaParser::subEventIdxType::fbxChannelIdx)
  //  return; //other indices are not ready yet

  if (idx==IDX_INVALID)
    return;
  if (GET_TYPE(idx)!=CalifaParser::subEventIdxType::fbxChannelIdx && recurse)
    return;
  //  static HistogramAxis axis080=	  *createCalEnergyAxis(IDX(0, 8, 0));

  //  static HistogramAxis qpid_axis[]={axis_lim_n_f, axis_lim_n_s};
  //static HistogramAxis module_channel[]={axis_coinc_sfp_mod, axis_fbx_pc_channel};
  //static HistogramAxis module_mchannel[]={axis_fbx_sfp0_module, axis_mesytec_PA_ch};
  //static HistogramAxis module_ts_diff[]={axis_coinc_sfp_mod, axis_coinc_ts_diff};
  //static HistogramAxis module_pulser[]={axis_fbx_sfp0_module, axis_coinc_pulser};
  

  static int globals_initialized;
  
  if (!globals_initialized)
    {
      auto any=IDX_ANY;
      auto evt=IDX_EVENT;

      
      // first energy histogram created, create now overview histogram
      //      new HistFillerSubprocessor<TH(1,I), 1>(&any, &axis_coinc_ts_diff);

      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_sfp_mod, axis_fbx_pc_channel});
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_weird_sfp_mod, axis_fbx_channel});
     new SingleTraceSubprocessor("trace_any", any, tracepoints, tracepoints);


      
      // TIMESTAMP HISTOGRAMS
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_mes_nonspecial);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_wix_nonspecial);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_mes_nonspecial_he);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_wix_nonspecial_he);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_master_he);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_slave_he);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_mes_special);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_lim_wrts_diff_main_wix_special);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_full_wrts_diff_main_mes_special);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_full_wrts_diff_main_mes_nonspecial_he);
      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_full_wrts_diff_main_wix_nonspecial_he);
      //new HistFillerSubprocessor<TH(2,I), 1>(&any, module_mchannel);
      //new HistFillerSubprocessor<TH(2,I), 1>(&any, module_ts_diff);
      //new HistFillerSubprocessor<TH(2,I), 1>(&any, module_pulser);
      //      new HistFillerSubprocessor<TH(1,I), 1>(&evt, &axis_full_wrts_diff_main_ams);
      //      new HistFillerSubprocessor<TH(1,I), 1>(&evt, &axis_full_wrts_diff_califa_ams);
      /*new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_diff_main_mes);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_diff_main_wix);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_diff_mes_wix);*/
      //new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_diff_t3_mes_wix);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_diff_wix_mes);
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim_wrts_diff_main});
      //new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim_wrts_diff_main_master});
      //new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim_wrts_diff_main_slave});
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_full_wrts_diff_main}, 10);
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_lim_wrts_diff_main_nonspecial, axis_lim2_energy}, 10);
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_lim_wrts_diff_main_mes_nonspecial, axis_lim2_energy}, 10);
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_lim_wrts_diff_main_wix_nonspecial, axis_lim2_energy}, 10);
       new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_lim_wrts_diff_main_nonspecial, axis_lim2_energy}, 10);
     new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim2_energy}, 10);
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_fbx_overflow_worst});
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim_trace_sample});
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_full2_energy, axis_lim2_dr_gamma});
      new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_coinc_pc_sfp_mod, axis_lim2_dr_gamma});
      //new HistFillerSubprocessor<TH(2,I), 1>(any, {axis_lim2_dr_proton, axis_full2_energy});

      //static HistogramAxis wrts_diff_febex[]={axis_lim_wrts_diff_califa_main, axis_coinc_sfp_mod};
      //new HistFillerSubprocessor<TH(2,I), 1>(&any, wrts_diff_febex);
      auto ch=IDX(0, 1, 0);
      //      new HistFillerSubprocessor<TH(1,I), 1>(&ch, &axis_full_wrts_diff_califa_main);


      if (0)
        {
          new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_ms_mes);
          //      new HistFillerSubprocessor<TH(1,I), 1>(&evt, &axis_full_wrts_ms_ams);
          new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_ms_main);
        }
      
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_skip_mes);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_skip_wix);
      //      new HistFillerSubprocessor<TH(1,I), 1>(&evt, &axis_full_wrts_skip_ams);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_full_wrts_skip_main);
      new HistFillerSubprocessor<TH(1,I), 1>(evt, axis_coinc_multiplicity);

      static auto idx_006=IDX(0, 0, 6);
      //new HistFillerSubprocessor<TH(1,I), 1>(&idx_006, &axis_hack_psp_sum);
      //new HistFillerSubprocessor<TH(1,I), 1>(&idx_006, &axis_hack_psp_diff);


      new HistFillerSubprocessor<TH(1,I), 1>(any, axis_full_delay);

      globals_initialized++;

      
    }
  
  linfo("adding %d %d %d %d (recurse=%d, tl=%d)\n", GET_TYPE(idx), GET_SFP(idx), GET_MOD(idx), GET_CH(idx),
        recurse, tracepoints);


  auto &evts=*(p->getCalifaEvents());
  
  if (recurse && evts[idx].trace)
    tracepoints=evts[idx].tracepoints;
    


  
  if ((GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx
       || GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx))
    {
      this->energy_subprocessors[idx]=new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_sum_cal_en);
      
      if (GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx)
	{
	  //	  static HistogramAxis en2_axis[]={axis_sum_cal_en, axis080};
	  CalifaParser::module_index_t idxes[2]={idx, IDX(0, 8, 0)};
	  //new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 16);
          /*	  if (GET_PETAL(idx)==0)
	    {
	      	  static HistogramAxis en2_axis[]={axis_sum_cal_en, axis_sum_cal_en};
		  CalifaParser::module_index_t idxes[2]={idx, std::make_tuple(CalifaParser::subEventIdxType::petalIdx, 1, 254, 254)};
		  new HistFillerSubprocessor<TH(2,I), 2>(idxes, en2_axis, 16);
                  }*/
	}

      return;
    }


  if (recurse && GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxChannelIdx)
    {

      // for (uint8_t sfp=0; sfp<=GET_SFP(idx); sfp++)
      {
	// add one channel for each module before, so the channel dirs get created in right order
	for (uint8_t sfp=10*( GET_SFP(idx)/10); sfp<=GET_SFP(idx); sfp++)
	  for (uint8_t module=0; module<=GET_MOD(idx); module++)
	    {
	      CalifaParser::module_index_t i=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx,
							     sfp, module,	(uint8_t)0);
	      // petal
	      if (0)
	      {
		auto i2=CalifaParser::toIdxType(CalifaParser::subEventIdxType::petalIdx, i);
		printf("--->adding %d %d %d %d\n", GET_TYPE(i2), GET_SFP(i2), GET_MOD(i2), GET_CH(i2));
		this->addChannel(p, i2, tracepoints);
	      }
	      // febex mod
	      if (0)
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
     {MIDX(0,0), {MIDX(0,1)}},
      {IDX(0, 6, 0),
       {
	 IDX(0, 6, 15),
	 IDX(0, 7, 0)
       }},//, 
      {IDX(0, 7, 0),
       {
	 IDX(0, 6, 0)
       }}//, 
      //{IDX(0, 0, 3), {IDX(0, 0, 6)}},
      //{IDX(0, 0, 4), {IDX(0, 0, 5)}},
      //{IDX(0, 0, 5), {IDX(0, 0, 6)}},
      //  {IDX(0, 6, 4), {IDX(0, 5, 10)}},
      //{IDX(0, 0, 11), {IDX(0, 1, 5), IDX(0, 0, 15), IDX(0, 8, 9)}}
    };


  if (!this->energy_subprocessors.count(idx))
    {
  
      //we have found an event without an energy histogram,
      //create one. 
      this->energy_subprocessors[idx]=new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_energy);
      new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_num_pileup);
	//	new ChannelCutHistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy, 1,
	//						 &idx, &axis_full_energy);
      new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_lim_energy);
      //new HistFillerSubprocessor<TH(1,I), 1>(&idx, &axis_full_energy);
      ldbg("created a new energy processor for %d:%d:%d.\n", 
	   GET_SFP(idx), GET_MOD(idx), 
	   GET_CH(idx));
      //new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 2);
  
      /*      if (HistogramAxis* ha=createCalEnergyAxis(idx))
	{
	  new HistFillerSubprocessor<TH(1,I), 1>(&idx, ha);
	  ldbg(" also created a calibrated energy processor\n");
	  //  new ChannelCutHistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64,
	  //						       &idx, ha, 30.0, 8000.0);
	}
      else 
      */if (0)//;GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx)
	{
	  new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_lim_cal_en);

	}
      //new HistFillerSubprocessor<TH(2,I), 1>(&idx, qpid_axis, 64);
      if (0 && correlations.count(idx))
	for (auto m=correlations[idx].begin(); m!=correlations[idx].end(); ++m)
	  {
	    new HistFillerSubprocessor<TH(2,I), 2>({idx, *m}, {axis_lim_cal_en, axis_lim_cal_en}, 10);
	  }
    }


  if (tracepoints!=0 && ! this->trace_subprocessors.count(idx) && GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxChannelIdx )
    {
      assert(tracepoints>0);
      //printf("tracepoints=%d\n", tracepoints);
      //we have traces, create trace subprocessors
      //
      // baseline hist processor
      {
	//	    BaselineHistSubprocessor* bhp=
	new BaselineHistSubprocessor("baseline", idx);
      }
      
      //single traces
      if (tracepoints>100)
      {
	SingleTraceSubprocessor* tp=
	  new SingleTraceSubprocessor("trace_last", idx, tracepoints,
				      tracepoints);
	tp->getHist()->GetXaxis()->SetTitle("trace point index");
	this->trace_subprocessors[idx]=tp;
      }
      //FFT
      if (0)
      {
	//	    FourierSpectrumSubprocessor *fftp=
	new FourierSpectrumSubprocessor("fft_amp", "fft_phase", 
					idx, tracepoints,
					F_ADC/2); // for decimated traces. 
      }
      // trace analysis histograms
      if (0)
      {
        std::array<HistogramAxis,2> trace_vs_en_axis={axis_full_energy, axis_full_trace_en};

        new HistFillerSubprocessor<TH(2,I), 1>(idx, trace_vs_en_axis, 10);
        std::array<HistogramAxis,2> bl_vs_err={axis_full_trace_en_diff, axis_full_trace0};
        new HistFillerSubprocessor<TH(2,I), 1>(idx, bl_vs_err, 8);

        std::array<HistogramAxis,2> bl={axis_full_bl_slope, axis_full_trace0};
        new HistFillerSubprocessor<TH(2,I), 1>(idx, bl, 8);

        new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_trace_en);
        new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_trace_en_diff);

        new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_trace_start);
        new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_max_slope);
        new HistFillerSubprocessor<TH(1,I), 1>(idx, axis_full_bl_slope);
      }


    }

}
