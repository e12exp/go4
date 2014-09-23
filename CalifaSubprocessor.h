#ifndef __CalifaSubprocessor_H
#define __CalifaSubprocessor_H

#include "struct_event.h"
#include "CalifaParser.h"
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#define __GXX_EXPERIMENTAL_CXX0X__ 1
#endif 
#include <list>
#include "TObject.h"
#include "debug.h"
class CalifaSubprocessor
{
 public:
  CalifaSubprocessor();
  virtual void registerSP();
  virtual void processEvent(CalifaParser* p)=0;
  virtual std::list<TObject*> makeHists()
    {
      linfo("CalifaSubprocessor::makeHists was not overridden.\n");
      std::list<TObject*> empty;
      return empty;
    }
  //virtual const char* getPath()=0;
 protected:
};

#endif
