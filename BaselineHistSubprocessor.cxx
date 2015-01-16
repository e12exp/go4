#include "BaselineHistSubprocessor.h"

BaselineHistSubprocessor::BaselineHistSubprocessor(char* name,
						 module_index_t idx)
  : SingleChannelSubprocessor(name, idx, (1<<15)+1+512, (1<<15)+1, -512.0)
{

}

void BaselineHistSubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.trace)
    {
      this->h->Scale(1-1.0/64);
      for (unsigned int i=1; i<=ei.tracepoints; i++)
	{
	  this->h->Fill((double)getTracePoint(ei.trace, i));
	}
    }
}
