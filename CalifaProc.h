#ifndef CalifaProc_H
#define CalifaProc_H

#include "CalifaConfig.h"

#include "TGo4EventProcessor.h"
#include <stdint.h>
#include "struct_event.h"
#include <list>
#include <tuple>
#include <map>
#include "CalifaBaseProc.h"
class CalifaSubprocessor;
class CalifaParam;
class TGo4Fitter;
class TClass;
class CalifaParser;

//Note: as Go4 makes extensive use of ROOT stuff, it has to use use ROOT
// to build class defintions. 
// Sadly, ROOT tends to fail even on moderatly sophisticated things like 
// tuples, so we can never let it see any serious C++.
// As a workaround, there exists a fully ROOT-compatible (aka dumb) class named 
// WrapperProc, and a C-compatible interface to CalifaProc named CalifaBaseProc.
// Yes, this is ugly. 

#include "CalifaParser.h"
class CalifaProc
{
   public:
  //CalifaProc() ;
  CalifaProc(const char* name, TGo4EventProcessor* go4ep);
  virtual ~CalifaProc() ;

  Bool_t BuildEvent(TGo4EventElement* target); // event processing function
  void RegisterSubprocessor(CalifaSubprocessor* sp);
  static CalifaProc* GetProc()
  {
    return CalifaProc::gProc;
  }

  /*  static const uint32_t FEBEX_PROC_ID=1;
  static const uint32_t FEBEX_EVT_TYPE=10;
  static const uint32_t FEBEX_SUBEVT_TYPE=1;
  */


  void registerObject(TObject* o);
 protected:
  typedef CalifaParser::module_index_t module_index_t;

  static CalifaProc* gProc;
  void registerNewHistograms();
  std::map<module_index_t, std::list<CalifaSubprocessor*> > subprocessors;
  std::list<CalifaSubprocessor*> newsubprocessors;

  CalifaParser* parser;
  TGo4EventProcessor* go4ep;
};

extern CalifaProc* gCalifaProc;


#endif //TUNPACKPROCESSOR_H


//----------------------------END OF GO4 SOURCE FILE ---------------------
