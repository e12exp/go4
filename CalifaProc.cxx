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
#include <map>
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
      for (auto it: l)
	this->registerObject(it);

      auto sens=sp->getSensitivity();
      this->subprocessors[sens].push_back(sp);
      if (sens==IDX_NONE && !l.empty())
	{
	  TNamed* n=dynamic_cast<TNamed*>(l.front());
	  if (n)
	    lerror( "The subprocessor which added TNamed %s did not set a channel sensitivity,"
		    " this histogram will never get filled. ", n->GetName() );
	}
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
  this->parser=new CalifaSumParser();
  this->go4ep=go4ep;
  (new OnDemandSubprocessor())->registerSP();
}

// event function
Bool_t CalifaProc::BuildEvent(TGo4EventElement * target)
{
  ldbg("**** CalifaProc: new event\n");
  this->registerNewHistograms();
  auto fInput = dynamic_cast<TGo4MbsEvent *>(this->go4ep->GetInputEvent());

  if (this->parser->parseGo4(fInput))
    return kFALSE;

  for (auto it:*(this->parser->getCalifaEvents()))
    if (this->subprocessors.count(it.first))
      for (auto sp : this->subprocessors[it.first])
	(sp)->applyCut(this->parser);

  if (this->subprocessors.count(IDX_ANY))
      for (auto sp : this->subprocessors[IDX_ANY])
	(sp)->applyCut(this->parser);

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
