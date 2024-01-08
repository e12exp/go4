#ifndef BaselineHistSubprocessor_h
#define BaselineHistSubprocessor_h
#include "SingleChannelSubprocessor.h"
class BaselineHistSubprocessor : public SingleChannelSubprocessor
{
protected:
  double downscale;
 public:
  BaselineHistSubprocessor(char* name,
                           module_index_t idx, double downscale=1.0);
  virtual void processSubevent(CalifaParser::module_index_t cidx, eventinfo_t ei) override;
};
#endif
