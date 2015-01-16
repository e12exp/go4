#ifndef BaselineHistSubprocessor_h
#define BaselineHistSubprocessor_h
#include "SingleChannelSubprocessor.h"
class BaselineHistSubprocessor : public SingleChannelSubprocessor
{
 public:
  BaselineHistSubprocessor(char* name,
		     module_index_t idx);
  virtual void processSubevent(eventinfo_t ei);
};
#endif
