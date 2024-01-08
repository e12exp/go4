#ifndef OnDemandSubprocessor_h
#define OnDemandSubprocessor_h

#include "CalifaSubprocessor.h"
#include <map>
//this class just adds subprocessors for anything useful found by the parser.
class OnDemandSubprocessor : public CalifaSubprocessor
{
 public:
  static OnDemandSubprocessor* instance()
    {
      if (!OnDemandSubprocessor::inst)
	OnDemandSubprocessor::inst=new OnDemandSubprocessor();
      return OnDemandSubprocessor::inst;
    }
  virtual void processEvent(CalifaParser* p);
 protected:
  static OnDemandSubprocessor* inst;
  OnDemandSubprocessor();

  void addChannel(CalifaParser* p, module_index_t idx, int tracepoints, bool recurse=false);

  typedef std::map<module_index_t, CalifaSubprocessor*> subprocmap;
  subprocmap energy_subprocessors;
  subprocmap trace_subprocessors;


  virtual module_index_t getSensitivity()
  {
    return IDX_ANY; 
  }

};

#endif
