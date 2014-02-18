// N.Kurz, EE, GSI, 15-Jan-2010

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
#include "TXXXProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "snprintf.h"
#include <TFile.h>

#include "TGo4MbsEvent.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CondArray.h"
#include "TGo4Picture.h"
#include "TXXXParam.h"
#include "TGo4Fitter.h"

#include <cmath>
#include "struct_gosip.h"
#include "struct_event.h"

#define ECUT 0
#define EMIN 19500
#define EMAX 22500

#define ONLY_SELFTRIGGERED 0

#define RPID_CUT 0
#define RPID_CUTG 0

#define X1 -28719
#define Y1 38081
#define X2 -3424
#define Y2 5934
#define XMAX -3100

//RPID on/off
#define RPID 0

#if RPID
#define M ((Y2 - Y1) / (X2 - X1))
#define N (Y2 - X2*M)
#define Y(x) (M*(x) + N)
#endif

#define F_ADC 50

//Time over Threshold on/off
#define TOT 0


#define TRACE_SIZE 4096         // in samples
#define SIS_TRACE_LEN 3000

//#define N_CHA  1                // no. of ADC channels per febex card

#define DEBUG 0

#if DEBUG
#define DBG(msg) std::cerr << msg;
#else
#define DBG(msg)
#endif

static Int_t l_sfp_slaves[MAX_SFP] = NR_SLAVES;

static Int_t id_time_zero[2] = TIME_ZERO;

//***********************************************************
TXXXProc::TXXXProc():TGo4EventProcessor("Proc")
{
  cout << "**** TXXXProc: Create instance " << endl;
}

//***********************************************************
TXXXProc::~TXXXProc()
{
  cout << "**** TXXXProc: Delete instance " << endl;
}

//***********************************************************
// this one is used in standard factory
TXXXProc::TXXXProc(const char *name):TGo4EventProcessor(name)
{
  cout << "**** TXXXProc: Create instance " << name << endl;

  Text_t chis[128];
  Text_t chead[128];
  Int_t l_i, l_j, l_k;

  for (l_i = 0; l_i < MAX_SFP; l_i++) {
    if (l_sfp_slaves[l_i] != 0) {
      for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++) {

        for (l_k = 0; l_k < N_CHA; l_k++) {
          sprintf(chead, "Febex Trace %2d %2d", l_j,
                  l_k);
          sprintf(chis, "febex_trace_%02d_%02d", l_j, l_k);
          h_trace[l_i][l_j][l_k] =
            MakeTH1('I', chis, chead, TRACE_SIZE-1, 0, TRACE_SIZE-1);
        }
       for (l_k = 0; l_k < N_CHA; l_k++) {
          sprintf(chead, "Febex Energy %2d %2d", l_j, l_k);
          sprintf(chis, "febex_energy_%02d_%02d", l_j, l_k);
          h_energy_fpga[l_i][l_j][l_k] =
            MakeTH1('I', chis, chead, 1000, 0, 0xFFFF);
        }

       for (l_k = 0; l_k < N_CHA; l_k++) {
	  h_evse[l_i][l_j][l_k] = MakeTH2('I', Form("febex_e_%02d_%02d-%02d", l_i, l_k, (l_k + 1) % N_CHA), Form("Febex %d - %d vs %d",
		l_i, l_k, (l_k + 1) % N_CHA),
	      500,0,0xffff,500,0,0xffff);
        }


        for (l_k = 0; l_k < N_CHA; l_k++) {
          sprintf(chead, "Febex QPID %2d %2d", l_j, l_k);
          sprintf(chis, "febex_qpid_%02d_%02d", l_j, l_k);
          h_rpid_fpga[l_i][l_j][l_k] =
            MakeTH2('I', chis, chead, 1000, 0, 66000, 1000, 0, 66000);
        }

#if (FBX_FFT || FBX_DFT)
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_dft_ampl[l_i][l_j][l_k] = MakeTH1('D', Form("dft_amplitude_%02d_%02d", l_j, l_k),
	      Form("DFT Amplitude %02d %02d", l_j, l_k), TRACE_SIZE/2-1, 0, F_ADC/2);
	  h_dft_phase[l_i][l_j][l_k] = MakeTH1('D', Form("dft_phase_%02d_%02d", l_j, l_k),
	      Form("DFT Phase %02d %02d", l_j, l_k), TRACE_SIZE/2-1, 0, F_ADC/2);
	}
#endif

#if FBX_HIST_ADC
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_hist_adc[l_i][l_j][l_k] = MakeTH1('I', Form("adc_hist_%02d_%02d", l_j, l_k),
	      Form("ADC Histogram %02d %02d", l_j, l_k), (1 << FBX_ADC_BITS), 0, (1 << FBX_ADC_BITS));
	}
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_hist_adc_dev[l_i][l_j][l_k] = MakeTH1('D', Form("adc_hist_dev_%02d_%02d", l_j, l_k),
	      Form("ADC Histogram Deviation%02d %02d", l_j, l_k), (1 << FBX_ADC_BITS) - 1, 0, (1 << FBX_ADC_BITS) - 1);
	}
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_trace_adc_diff[l_i][l_j][l_k] = MakeTH1('I', Form("adc_diff_%02d_%02d", l_j, l_k),
	      Form("ADC Differential %02d %02d", l_j, l_k), TRACE_SIZE-2, 0, TRACE_SIZE-2);
	}
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_hist_adc_diff[l_i][l_j][l_k] = MakeTH1('I', Form("adc_diff_hist_%02d_%02d", l_j, l_k),
	      Form("ADC Differential Histogram %02d %02d", l_j, l_k), (1 << FBX_ADC_BITS) - 1, -(1 << (FBX_ADC_BITS-1)), (1 << (FBX_ADC_BITS-1)) - 1);
	}
	for(l_k = 0; l_k < N_CHA; l_k++)
	{
	  h_adc_diff_vs_adc[l_i][l_j][l_k] = MakeTH2('I', Form("adc_diff_vs_adc_%02d_%02d", l_j, l_k),
	      Form("ADC Differential vs ADC %02d %02d", l_j, l_k), (1 << FBX_ADC_BITS)/5-1, 0, (1 << FBX_ADC_BITS) - 1, (1 << FBX_ADC_BITS)/5 - 1, -(1 << (FBX_ADC_BITS-1)), (1 << (FBX_ADC_BITS-1)) - 1);
	}
#endif
      }
    }
  }

  // CAEN ADC
  for(l_k = 0; l_k < 32; l_k++)
  {
    h_energy_caen[l_k] = MakeTH1('I', Form("v785_energy_%02d", l_k), Form("CAEN v785 Energy %d", l_k), 0xfff, 0, 0xfff);
  }

  // SIS ADC
  for(l_k = 0; l_k < 8; l_k++)
  {

    h_energy_sis[l_k] = MakeTH1('I', Form("sis3302_energy_%d", l_k), Form("SIS3302 Energy %d", l_k), 0xffff, 0, 0xffff);
    h_trace_sis[l_k] = MakeTH1('I', Form("sis3302_trace_%d", l_k), Form("SIS3302 Trace %d", l_k), SIS_TRACE_LEN, 0, SIS_TRACE_LEN); 
  }
 
  sprintf(chead, "TRIGGER TYPE");
  sprintf(chis, "Trig");
  h_trig_type = MakeTH1('I', chis, chead, 0x20, 0, 0x10);

  /*
  sprintf(chis, "HIT PATTERN");
  sprintf(chead, "Hit");
  h_hit_pattern = MakeTH1('I', chis, chead, 0x1000, 0, 0x1000);


  sprintf(chis, "NO. HIT ");
  sprintf(chead, "Hit");
  h_num_hit = MakeTH1('I', chis, chead, 0x20, 0, 0x20);


  sprintf(chis, "DATA SIZE ");
  sprintf(chead, "Trig");
  h_data_size = MakeTH1('I', chis, chead, 0x1000, 0, 0x1000);
  */
  
  num_events = 0;

  printf("Histograms created \n");
  fflush(stdout);

#if RPID_CUTG
  TFile *f1 = new TFile("rpid-cut.root");
  rpid_cut = (TCutG*)f1->Get("CUTG");
#endif
}

//-----------------------------------------------------------
// event function
Bool_t TXXXProc::BuildEvent(TGo4EventElement * target)
{                               
  // called by framework. We dont fill any outindent: Standard input:253: Warning:old style assignment ambiguity in "=*".  Assuming "= *" put event here at all

  Int_t l_i, l_j, l_k, l_l;
  UInt_t *pl_se_dat;
  UInt_t *pl_se_end;
  UInt_t *pl_tmp;
  UInt_t *ptr_next_submemory;

  UInt_t l_padd;

  gosip_header_t *gosip = new gosip_header_t;
  gosip_sub_header_t *gosip_sub = new gosip_sub_header_t;
  event_t *evnt = new event_t;
  trace_head_t *data = new trace_head_t;

  UInt_t l_cha;

#if FBX_FFT
  double aReX[TRACE_SIZE], aImX[TRACE_SIZE];
  double ReX, ImX;
#endif

  uint64_t l_time, l_time_zero;

  static UInt_t l_evt_ct = 0;

  TGo4MbsSubEvent *psubevt;

  fInput = (TGo4MbsEvent *) GetInputEvent();
  if (fInput == 0) {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  if (fInput->GetTrigger() > 11) {
    cout << "**** TXXXProc: Skip trigger event" << endl;
    return kFALSE;
  }
  // first we fill the arrays fCrate1,2 with data from MBS source
  // we have up to two subevents, crate 1 and 2
  // Note that one has to loop over all subevents and select them by
  // crate number:   psubevt->GetSubcrate(),
  // procid:         psubevt->GetProcid(),
  // and/or control: psubevt->GetControl()
  // here we use only crate number

  l_evt_ct++;
  fInput->ResetIterator();
  //while((psubevt = fInput->NextSubEvent()) != 0) // loop over subevents
  //{


  int crate_nr;

  while((psubevt = fInput->NextSubEvent()))
  {
    crate_nr = psubevt->GetSubcrate();
    
    pl_se_dat = (UInt_t *) psubevt->GetDataField();
    pl_se_end = pl_se_dat + psubevt->GetIntLen();

    pl_tmp = pl_se_dat;

  switch(crate_nr)
  {
    case 0:
     // Febex 

  if ((*pl_tmp) == 0xbad00bad) {
    printf("found bad event \n");
    return kFALSE;
  }
  for (l_i = 0; l_i < MAX_SFP; l_i++)   // loop over SFPs
    {
      if (l_sfp_slaves[l_i] != 0) {
        // first jump over pci express dma padding words
        for (l_j = 0; l_j < 100; l_j++) {
          l_padd = *pl_tmp++;
          //printf ("l_padd: 0x%x \n", l_padd); 
          if ((l_padd & 0xfff00000) != 0xadd00000) {
            //printf ("%d padding words \n", l_j);
            pl_tmp--;
            break;
          }
        }

	      uint32_t energy[N_CHA];

//              if (gosip->num_submemories > 0) {
		// loop over ADC channels 
//                for (l_k = 0; l_k < (Int_t)gosip->num_submemories; l_k++)
		  while(pl_tmp < pl_se_end)
                  {
		    gosip_sub->header_size = *pl_tmp & 0xff;
		    gosip_sub->trigger = (*pl_tmp >> 8) & 0xff;
		    gosip_sub->module_id = (*pl_tmp >> 16) & 0xff;
		    gosip_sub->submemory_id = (*pl_tmp++ >> 24) & 0xff;
		    gosip_sub->data_size = *pl_tmp++;
                    l_cha = gosip_sub->submemory_id;

		    l_j = gosip_sub->module_id;

		    DBG("l_cha: " << l_cha << endl)

                    //h_hit_pattern->Fill(l_cha + 0x100 
		    //			* (l_j + l_i * 0x1000));

                    if (gosip_sub->header_size != 0x34) {
                      printf("ERROR>> header is wrong = %x \n", 
			     gosip_sub->header_size);
                    }

		    // data reduction: end of stream marked with submemory
		    // size = 0 ???
		    if (gosip_sub->data_size == 0)
		      continue;
                    //h_data_size->Fill(gosip_sub->data_size);
		    
		    //Pointer to next submemory
		    ptr_next_submemory = pl_tmp + gosip_sub->data_size / 4;

		    //Special channel
		    if(gosip_sub->submemory_id == 0xff)
		    {
		      pl_tmp = ptr_next_submemory;
		      continue;
		    }

		    //Fill Trigger Type histogram
		    h_trig_type->Fill(gosip_sub->trigger);

		    //Deactivated channel
		    if(gosip_sub->submemory_id >= N_CHA)
		    {
		      pl_tmp = ptr_next_submemory;
		      continue;
		    }

		    //Data reduction: Empty channel
		    if(gosip_sub->data_size == 4 && *pl_tmp == 0xdeadda7a)
		    {
		      pl_tmp = ptr_next_submemory;
		      continue;
		    }

		    // *********start event data*************//
		    
		    evnt->size = *pl_tmp & 0xffff;
		    evnt->magic_affe = *pl_tmp++ >> 16;
		    evnt->event_id = *pl_tmp++;
		    evnt->timestamp = *pl_tmp++;
		    evnt->timestamp |= ((uint64_t)*pl_tmp++) << 32;
		    memcpy(evnt->cfd_samples, pl_tmp, 8); pl_tmp+=2;
		    evnt->loverflow = *pl_tmp && 0xffff;
		    evnt->hoverflow = (*pl_tmp >> 16) & 0xff;
		    evnt->self_triggered = *pl_tmp++ >> 24;
		    evnt->num_pileup = *pl_tmp & 0xffff;
		    evnt->num_discarded = *pl_tmp++ >> 16;
		    evnt->energy = *pl_tmp & 0xffff;
		    evnt->reserved = *pl_tmp++ >> 16;
		    evnt->rpid_size = *pl_tmp & 0xffff;
		    evnt->magic_babe = *pl_tmp++ >> 16;
		    evnt->n_f = *pl_tmp & 0xffff;
		    evnt->n_s = *pl_tmp++ >> 16;

		    //printf("Nf: %d, Ns: %d\n", evnt->n_f, evnt->n_s);

		    //Check Event Buffer ident 0xAFFE
		    if (evnt->magic_affe != 0xAFFE) {
		      printf("ERROR>> header size is not marked with AFFE:"
			     " %x\n", evnt->magic_affe);
		      pl_tmp = ptr_next_submemory;
		      continue;
		    }

#if RPID_CUT
		   if(!(evnt->n_s <= Y(evnt->n_f) && evnt->n_f <= XMAX))
		   {
		      pl_tmp = ptr_next_submemory;
		      continue;
		   }
#endif

#if RPID_CUTG
		  if(!rpid_cut->IsInside(evnt->n_f, evnt->n_s))
		  {
		    pl_tmp = ptr_next_submemory;
		    continue;
		  }
#endif

#if ECUT
		  if(evnt->energy < EMIN || evnt->energy > EMAX)
		  {
		    pl_tmp = ptr_next_submemory;
		    continue;
		  }
#endif

//	    if(num_events % DRAW_TRACE_EVERY == 0)
//	    {
//              for (l_k = 0; l_k < N_CHA; l_k++) {
//                h_trace[l_i][l_j][l_k]->Reset("");
//              }
//	    }
 
		    //Fill Energy FPGA histogram
#if ONLY_SELFTRIGGERED
		    if(evnt->self_triggered )
		    {
#endif
			    h_energy_fpga[l_i][l_j][l_cha]->Fill(evnt->energy);
			    energy[l_cha] = evnt->energy;

//			    if(!(evnt->loverflow | evnt->hoverflow))
			    if(!(evnt->n_f == 0 && evnt->n_s == 0))
	  		    h_rpid_fpga[l_i][l_j][l_cha]->Fill(evnt->n_f, evnt->n_s);
#if ONLY_SELFTRIGGERED
		    }
#endif

		    //If data samples are available
		    if (evnt->size > event_t_size) {
		      data->size = *pl_tmp & 0xffff;
		      data->magic_2bad = *pl_tmp++ >> 16;
		      data->wrap_around_addr = *pl_tmp & 0xffff;
		      data->type = *pl_tmp++ >> 16;
		      //Check Data Samples ident
 		      if (data->magic_2bad != 0x2bad) {
			printf("ERROR>> wrong data header ident: 0x%x "
			       "instead of 0x2bad\n", data->magic_2bad);
		      } else {
			//Pointer to first trace position
			pl_tmp += sizeof(trace_head_t) / 4;

//			if(num_events % DRAW_TRACE_EVERY == 0)
//			{
				h_trace[l_i][l_j][l_cha]->Reset();
				// loop over traces 
  				for (l_l = 0; l_l < (data->size / 4 - 2); l_l++) {
  				  // disentangle data
  				  h_trace[l_i][l_j][l_cha]->
  				    Fill(l_l * 2, pl_tmp[l_l] & 0xFFFF);
  				  h_trace[l_i][l_j][l_cha]->
  				    Fill(l_l * 2 + 1, pl_tmp[l_l] >> 16);
  				}

#if FBX_HIST_ADC
		    uint16_t last_sample = 0;

		  for(l_l = 0; l_l < TRACE_SIZE/2; l_l++)
		  {
		    h_hist_adc[l_i][l_j][l_cha]->Fill(pl_tmp[l_l] & 0xFFFF);
		    h_hist_adc[l_i][l_j][l_cha]->Fill((pl_tmp[l_l] >> 16) & 0xFFFF);

		    if(l_l > 0)
		    {
		      h_trace_adc_diff[l_i][l_j][l_cha]->Fill(2*l_l-1, (int32_t)(pl_tmp[l_l] & 0xFFFF) - last_sample);
		      h_hist_adc_diff[l_i][l_j][l_cha]->Fill((int32_t)(pl_tmp[l_l] & 0xFFFF) - last_sample);
		      h_adc_diff_vs_adc[l_i][l_j][l_cha]->Fill(pl_tmp[l_l] & 0xFFFF, (int32_t)(pl_tmp[l_l] & 0xFFFF) - last_sample);
		    }
		
		    h_trace_adc_diff[l_i][l_j][l_cha]->Fill(2*l_l, (int32_t)(pl_tmp[l_l] >> 16) - (int32_t)(pl_tmp[l_l] & 0xFFFF));
		    h_hist_adc_diff[l_i][l_j][l_cha]->Fill((int32_t)(pl_tmp[l_l] >> 16) - (int32_t)(pl_tmp[l_l] & 0xFFFF));
		    h_adc_diff_vs_adc[l_i][l_j][l_cha]->Fill(pl_tmp[l_l] >> 16, (int32_t)(pl_tmp[l_l] & 0xFFFF) - last_sample);

		    last_sample = pl_tmp[l_l] >> 16;
		  }

		  h_hist_adc_dev[l_i][l_j][l_cha]->Reset();
		  Int_t n = h_hist_adc[l_i][l_j][l_cha]->GetNbinsX();
		  
		  Double_t m = 0; //h_hist_adc[l_i][l_j][l_cha]->GetMean(1);
		  for(l_l = 10; l_l < n-10; l_l++)
		    m += h_hist_adc[l_i][l_j][l_cha]->GetBinContent(l_l);
		  m /= (n-20);

		  Double_t s = 0; //h_hist_adc[l_i][l_j][l_cha]->GetRMS(1);
		  for(l_l = 10; l_l < n-10; l_l++)
		    s += pow(h_hist_adc[l_i][l_j][l_cha]->GetBinContent(l_l) - m, 2);
		  s = sqrt(s/(n-20));

		  if(s != 0)
		  {
		    for(l_l = 0; l_l < n; l_l++)
		      h_hist_adc_dev[l_i][l_j][l_cha]->SetBinContent(l_l, fabs(h_hist_adc[l_i][l_j][l_cha]->GetBinContent(l_l) - m) / s);
		  }
#endif

#if FBX_DFT				
				for(int _k = 0; _k < TRACE_SIZE/2; _k++)
				{
				  double ReX = 0;
				  double ImX = 0;

				  for(int _i = 0; _i < TRACE_SIZE/2; _i++)
				  {
				    ReX += (Double_t)(pl_tmp[_i] & 0xffff) * cos(2 * M_PI * _k * (2 * _i) / TRACE_SIZE);
				    ImX += (Double_t)(pl_tmp[_i] & 0xffff) * sin(2 * M_PI * _k * (2 * _i) / TRACE_SIZE);
				    
				    ReX += (Double_t)(pl_tmp[_i] >> 16) * cos(2 * M_PI * _k * (2 * _i + 1) / TRACE_SIZE);
				    ImX += (Double_t)(pl_tmp[_i] >> 16) * sin(2 * M_PI * _k * (2 * _i + 1) / TRACE_SIZE);
				  }
				  h_dft_ampl[l_i][l_j][l_cha]->AddBinContent(_k, sqrt(ReX*ReX + ImX*ImX));
				  if(ReX == 0 && ImX >= 0)
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, M_PI/2);
				  else if(ReX == 0 && ImX < 0)
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, -M_PI/2);
				  else
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, atan(ImX/ReX));
				}
#endif
#if FBX_FFT
				for(unsigned int _i = 0; _i < TRACE_SIZE/2; _i++)
				{
				  aReX[2*_i] = pl_tmp[_i] & 0xffff;
				  aReX[2*_i + 1] = pl_tmp[_i] >> 16;
				}
				ReFFT(aReX, aImX, TRACE_SIZE);
				for(unsigned int _k = 0; _k < TRACE_SIZE/2; _k++)
				{
				  ReX = aReX[_k];
				  ImX = aImX[_k];
				  h_dft_ampl[l_i][l_j][l_cha]->AddBinContent(_k, sqrt(ReX*ReX + ImX*ImX));
				  if(ReX == 0 && ImX >= 0)
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, M_PI/2);
				  else if(ReX == 0 && ImX < 0)
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, -M_PI/2);
				  else
				    h_dft_phase[l_i][l_j][l_cha]->AddBinContent(_k, atan(ImX/ReX));
				}
#endif				

//			}
			pl_tmp = ptr_next_submemory;
			
		      }
		    }
		    
                  }

		  for(int z = 0; z < N_CHA; z++)
		   h_evse[l_i][l_j][z]->Fill(energy[(z + 1) % N_CHA], energy[z]);

	  break;	  
	case 1:
	  // VME crate
	  if(*pl_tmp != 0)
	  {
	    cout << "Broken VME event. Header should be 0 (0x" << hex << *pl_tmp << ")" << endl;
	    continue;
	  }
	  pl_tmp++;

	  // CAEN ADC
	  // - Header
	  if((*pl_tmp & 0xff000000) != 0xfa000000)
	  {
	    cout << "Broken v785 event. Header should be 0xfa... (0x" << hex << *pl_tmp << ")" << endl;
	    continue;
	  }
	  pl_tmp++;

	  for(l_i = 0; l_i < 32; l_i++)
	  {
	    if((*pl_tmp & 0xff000000) != 0xf8000000)
	    {
	      cout << "Broken v785 channel. Data should be 0xf8.... (0x" << hex << *pl_tmp << ")" << endl;
	      pl_tmp++;
	      continue;
	    }
	    l_k = (*pl_tmp & 0x00ff0000) >> 16;
	    if(l_k > 31)
	    {
	      cout << "Invalid v785 channel number: " << dec << (int)l_k << endl;
	      pl_tmp++;
	      continue;
	    }
	    h_energy_caen[l_k]->Fill(*pl_tmp & 0xfff);
	    pl_tmp++;
	  }

      } //switch(crate_nr)
      } //while(...)
      }
    }
  num_events++;
  return kTRUE;
}

#if FBX_FFT
void TXXXProc::ReFFT(double *ReX, double *ImX, unsigned int N)
{
  unsigned int i, im, ip2, ipm, ip;
  double TR, TI;

  memset(ImX, 0, sizeof(double)*N);

  // Transform real values to complex values
  for(i = 0; i < N/2; i++)
  {
    ReX[i] = ReX[2*i];
    ImX[i] = ReX[2*i + 1];
  }

  // Perfom complex fast fourier transform
  CplxFFT(ReX, ImX, N/2);

  // Even/odd frequency decomposition
  for(i = 1; i < N/4; i++)
  {
    im = N/2 - i;
    ip2 = i + N/2;
    ipm = im + N/2;
    ReX[ip2] = (ImX[i] + ImX[im])/2;
    ReX[ipm] = ReX[ip2];
    ImX[ip2] = -(ReX[i] - ReX[im])/2;
    ImX[ipm] = -ImX[ip2];
    ReX[i] = (ReX[i] + ReX[im])/2;
    ReX[im] = ReX[i];
    ImX[i] = (ImX[i] - ImX[im])/2;
    ImX[im] = -ImX[i];
  }

  ReX[N*3/4] = ImX[N/4];
  ReX[N/2] = ImX[0];
  ImX[N*3/4] = 0;
  ImX[N/2] = 0;
  ImX[N/4] = 0;
  ImX[0] = 0;

  unsigned int l = (unsigned int)log2(N);
  unsigned int le = 1 << l;
  unsigned int le2 = 1 << (l-1);
  double ur = 1;
  double ui = 0;
  double sr = cos(M_PI/(double)le2);
  double si = -sin(M_PI/(double)le2);

  for(unsigned j = 1; j <= le2; j++)
  {
    for(i = j - 1; i < N; i += le)
    {
      ip = i + le2;
      TR = ReX[ip]*ur - ImX[ip]*ui;
      TI = ReX[ip]*ui + ImX[ip]*ur;
      ReX[ip] = ReX[i] - TR;
      ImX[ip] = ImX[i] - TI;
      ReX[i] += TR;
      ImX[i] += TI;
    }
    TR = ur;
    ur = TR*sr - ui*si;
    ui = TR*si + ui*sr;
  }
}

void TXXXProc::CplxFFT(double *ReX, double *ImX, unsigned int N)
{
  double TR, TI;

  unsigned int M = (int)log2(N);
  unsigned int j = N/2;
  unsigned int k;

  unsigned int i, l, le, le2, ip;

  double sr, si, ur, ui;

  // Bit reversal sorting
  for(i = 1; i < N-1; i++)
  {
    if(i < j)
    {
      TR = ReX[j];
      TI = ImX[j];
      ReX[j] = ReX[i];
      ImX[j] = ImX[i];
      ReX[i] = TR;
      ImX[i] = TI;
    }
    k = N/2;
    while(k <= j)
    {
      j -= k;
      k /= 2;
    }
    j += k;
  }

  // Loop over assembling stages
  for(l = 1; l <= M; l++)
  {
    le = 1 << l;
    le2 = 1 << (l-1);

    ur = 1;
    ui = 0;

    sr = cos(M_PI / (double)le2);
    si = -sin(M_PI / (double)le2);

    // Loop over sub DFTs
    for(j = 1; j <= le2; j++)
    {
      // Loop over butterflies
      for(i = j-1; i < N; i += le)
      {
	ip = i + le2;
	// Butterfly
	TR = ReX[ip]*ur - ImX[ip]*ui;
	TI = ReX[ip]*ui + ImX[ip]*ur;
	ReX[ip] = ReX[i]-TR;
	ImX[ip] = ImX[i]-TI;
	ReX[i] += TR;
	ImX[i] += TI;
      }
      TR = ur;
      ur = TR*sr - ui*si;
      ui = TR*si + ui*sr;
    }
  }
}
#endif
//----------------------------END OF GO4 SOURCE FILE ---------------------
