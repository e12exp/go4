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
#include "CalifaProc.h"
#include "CalifaSubprocessor.h"
#include <list>
#include "Riostream.h"

#include "TH1.h"
//#include "TH2.h"
#include "snprintf.h"
#include <TFile.h>
#include <TCanvas.h>
#include "TGo4EventElement.h"
#include "TGo4MbsEvent.h"

#include <cmath>
#include "struct_gosip.h"

#include "debug.h"
#include <string.h>

#include "OnDemandSubprocessor.h"
#include "SingleHistSubprocessor.h"

#define RESET_BACKTRACE
#ifdef RESET_BACKTRACE
#include <signal.h>
int disable_backtrace()
{
  signal(SIGSEGV, SIG_DFL);
  return 0;
}
int no_backtrace=disable_backtrace();
#endif



CalifaProc* CalifaProc::gProc=NULL;


void CalifaProc::RegisterSubprocessor(CalifaSubprocessor* sp)
{
  //linfo("registered %d histograms for subprocessor\n", l.size());
  this->newsubprocessors.push_back(sp);
  //we do not register the histograms right away, because the subprocessor is 
  // probably not constructed yet when it is registered in the base class constructor
}


void CalifaProc::registerObject(TObject* o)
{
  
  linfo("registering a new TObject of type %s\n", o->ClassName());
  if (auto h=dynamic_cast<TH1*>(o))
    {
      char* dir=NULL;
      const char* name=h->GetName();
      if (const char* x=strrchr(name, (int)'/'))
	{
	  int idx=(int)(x-name);
	  char buf[1000];
	  strncpy(buf, h->GetName(), 1000);
	  buf[idx]=0;
	  dir=buf;
	  h->SetName(buf+idx+1);
	  lerror("%s:%s\n", buf, buf+idx+1);
	}
      this->go4ep->AddHistogram(h, dir);
    }
  else if (auto c=dynamic_cast<TCanvas*>(o))
    this->go4ep->AddCanvas(c);
  else if (auto n=dynamic_cast<TNamed*>(o))
    this->go4ep->AddObject(n);
  else
    lerror( "Can not add Object of type %s to CalifaProc, ignored.\n", o->ClassName() );
}

void CalifaProc::registerNewHistograms()
{
  while (!this->newsubprocessors.empty())
    {
      CalifaSubprocessor* sp=this->newsubprocessors.front();
      this->newsubprocessors.pop_front();
      std::list<TObject*> l=sp->makeHists();
      for (auto it=l.begin(); it!=l.end(); ++it)
	this->registerObject(*it);
      this->subprocessors.push_back(sp);
    }
}

CalifaProc::~CalifaProc()
{
  linfo("**** CalifaProc: deleted instance\n");
  delete this->parser;
}

// this one is used in standard factory
CalifaProc::CalifaProc(const char *name, TGo4EventProcessor* go4ep)
{
  linfo("**** CalifaProc: Create instance %s\n", name);
#ifdef RESET_BACKTRACE
  disable_backtrace();
#endif
  CalifaProc::gProc=this;
  this->parser=new CalifaParser();
  this->go4ep=go4ep;
  (new OnDemandSubprocessor())->registerSP();
}

// event function
Bool_t CalifaProc::BuildEvent(TGo4EventElement * target)
{
  this->registerNewHistograms();
  auto fInput = dynamic_cast<TGo4MbsEvent *>(this->go4ep->GetInputEvent());
  if (fInput == 0) {
    linfo("AnlProc: no input event !\n");
    return kFALSE;
  }
  if (fInput->GetTrigger() > 11) {
    linfo("**** CalifaProc: Skip trigger event\n" );
    return kFALSE;
  }
  fInput->ResetIterator();
  while(auto psubevt = fInput->NextSubEvent())
    {
      uint32_t evt_type = psubevt->GetType();
      uint32_t subevt_type = psubevt->GetSubtype();
      uint32_t procid = psubevt->GetProcid();

   
      if(evt_type != FEBEX_EVT_TYPE || subevt_type != FEBEX_SUBEVT_TYPE || procid != FEBEX_PROC_ID)
	continue; //all FAIR/R3B events, febex or otherwise have the same ID set, if it is something else, ignore it. 
      int r=this->parser->parse((uint32_t*)psubevt->GetDataField(), psubevt->GetIntLen());

      if (r)
	{
	  //a bad subevent
	  return kFALSE;
	}
      else
	{
	  /*
	  auto e=this->parser->getCalifaEvents();
	  for (auto ei=e->begin(); ei!=e->end(); ++ei)
	    linfo("found index: %d %d %d\n", 
		  std::get<0>(ei->first), 
		  std::get<1>(ei->first), 
		  std::get<2>(ei->first)
		  );*/
	  ldbg("calling processEvent for %d processors\n", this->subprocessors.size());
	  for (auto sp=this->subprocessors.begin(); sp!=this->subprocessors.end(); ++sp)
	    (*sp)->processEvent(this->parser);
	}
    }
  return kTRUE;
}

Bool_t wrapperBuildEvent(void*p, TGo4EventElement* target)
{
  return ((CalifaProc*)p)->BuildEvent(target);
}

void* createCalifaProc(const char* name, TGo4EventProcessor* w)
{
  return new CalifaProc(name, w);
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
