#include "BaselineHistSubprocessor.h"

BaselineHistSubprocessor::BaselineHistSubprocessor(char* name,
						 module_index_t idx)
  : SingleChannelSubprocessor(name, idx, 1<<16, (1<<15)-1, -(1<<15))
{

}

void BaselineHistSubprocessor::processSubevent(CalifaParser::module_index_t cidx, eventinfo_t ei)
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
