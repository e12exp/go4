#ifndef __CalifaSubprocessor_H
#define __CalifaSubprocessor_H

#include "struct_event.h"
#include "CalifaParser.h"
#include <list>
#include "TObject.h"
#include "debug.h"
class CalifaSubprocessor
{
 public:
  CalifaSubprocessor();
  virtual void registerSP();
  virtual void processEvent(CalifaParser* p)=0;
  //returns a list of objects to be registered
  virtual std::list<TObject*> makeHists()
    {
      //ldbg("CalifaSubprocessor::makeHists was not overridden.\n");
      std::list<TObject*> empty;
      return empty;
    }
  //virtual const char* getPath()=0;
  //empty means any
  //virtual std::list<CalifaParser::module_index_t> getSensitivityList();

 protected:
  //alternativly, call this to register a processor
  virtual void registerObject(TObject * obj);
};

#endif
