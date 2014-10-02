#ifndef OnDemandSubprocessor_h
#define OnDemandSubprocessor_h

#include "CalifaSubprocessor.h"
#include <map>
//this class just adds subprocessors for anything useful found by the parser.
class OnDemandSubprocessor : public CalifaSubprocessor
{
 public:
  OnDemandSubprocessor();
  virtual void processEvent(CalifaParser* p);
 protected:
  typedef std::map<CalifaParser::module_index_t, CalifaSubprocessor*> subprocmap;
  subprocmap energy_subprocessors;
  subprocmap trace_subprocessors;
};

#endif
