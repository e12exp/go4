#include "BaselineHistSubprocessor.h"

BaselineHistSubprocessor::BaselineHistSubprocessor(char* name,
						 module_index_t idx)
  : SingleChannelSubprocessor(name, idx, (1<<15)-3, (1<<15)-2, 1)
{

}

void BaselineHistSubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.trace)
    {
      //auto limit=ei.tracepoints;
      unsigned int limit=5;
      //this->h->Scale(1-1.0/64);
      for (unsigned int i=1; i<=limit; i++)
	{
	  this->h->Fill((double)getTracePoint(ei.trace, i));
	}
    }
}
