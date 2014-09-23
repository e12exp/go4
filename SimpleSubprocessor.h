#include "CalifaSubprocessor.h"
#include "TH1I.h"
class SimpleSubprocessor : public CalifaSubprocessor
{
  virtual void processEvent(CalifaParser* p);
  virtual std::list<TObject*> makeHists();
  virtual const char* getPath()
  {
    return "foo";
  }
  TH1I* hist;
  CalifaParser::module_index_t idx;
};
