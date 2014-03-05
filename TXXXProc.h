//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#ifndef TUNPACKPROCESSOR_H
#define TUNPACKPROCESSOR_H

#include <TCutG.h>
#include "TXXXConfig.h"

#include "TGo4EventProcessor.h"

class TXXXParam;
class TGo4Fitter;

class TXXXProc : public TGo4EventProcessor {
   public:
      TXXXProc() ;
      TXXXProc(const char* name);
      virtual ~TXXXProc() ;

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function

 private:
      TGo4MbsEvent  *fInput;  //!

      TH1          *h_trace[MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_energy_fpga [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH2          *h_rpid_fpga [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH2          *h_evse [MAX_SFP][MAX_SLAVE][N_CHA];  //!
#if FBX_DFT || FBX_FFT
      TH1	   *h_dft_ampl [MAX_SFP][MAX_SLAVE][N_CHA];
      TH1	   *h_dft_phase [MAX_SFP][MAX_SLAVE][N_CHA];
#endif

#if FBX_HIST_ADC
      TH1	    *h_hist_adc [MAX_SFP][MAX_SLAVE][N_CHA];
      TH1	    *h_hist_adc_dev [MAX_SFP][MAX_SLAVE][N_CHA];
      TH1	    *h_trace_adc_diff [MAX_SFP][MAX_SLAVE][N_CHA];
      TH1	    *h_hist_adc_diff [MAX_SFP][MAX_SLAVE][N_CHA];
      TH2	    *h_adc_diff_vs_adc [MAX_SFP][MAX_SLAVE][N_CHA];
#endif

#if DIFF_TS
      TH1	    *h_diff_ts[MAX_SFP][MAX_SLAVE][N_CHA];
#endif      

      // CAEN v785 ADC
      TH1	   *h_energy_caen[32];
      // SIS3302 ADC
      TH1	   *h_energy_sis[8];
      TH1	   *h_trace_sis[8];

      TH1          *h_time_stamp [MAX_SFP][MAX_SLAVE];  
      TH1          *h_time_diff  [MAX_SFP][MAX_SLAVE];  

      TH1          *h_trig_type;  
      TH1          *h_hit_pattern;  
      TH1          *h_num_hit;  
      TH1          *h_data_size; 

      int	    num_events; 

#if RPID_CUTG
      TCutG	  *rpid_cut;
#endif

#if FBX_FFT
      void	  ReFFT(double *ReX, double *ImX, unsigned int N);
      void	  CplxFFT(double *ReX, double *ImX, unsigned int N);
#endif

   ClassDef(TXXXProc,1)
};
#endif //TUNPACKPROCESSOR_H


//----------------------------END OF GO4 SOURCE FILE ---------------------
