#ifndef OnDemandSubprocessor_h
#define OnDemandSubprocessor_h

#include "CalifaSubprocessor.h"
#include <map>
#include "TH1I.h"
//this class just adds subprocessors for anything useful found by the parser.
class OnDemandSubprocessor : public CalifaSubprocessor
{
 public:
  OnDemandSubprocessor();
  virtual void processEvent(CalifaParser* p);
 protected:
  map<  CalifaParser::module_index_t, CalifaSubprocessor* > energy_subprocessors;
  map<  CalifaParser::module_index_t, CalifaSubprocessor* > trace_subprocessors;
}

#endif
