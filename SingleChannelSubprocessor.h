#ifndef _SingleChannelSubprocessor_H
#define _SingleChannelSubprocessor_H

#include "SingleHistSubprocessor.h"
class SingleChannelSubprocessor: public SingleHistSubprocessor
{
 public:
  typedef CalifaParser::module_index_t module_index_t;
  SingleChannelSubprocessor(std::string name,
			    module_index_t idx,
			    int nbins,
			    int upperLimit, 
			    int lowerLimit
			 );
  virtual void processEvent(CalifaParser* p);
  virtual void processSubevent(eventinfo_t ei)=0;
 protected:
  module_index_t idx;
};
#endif
