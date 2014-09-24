#ifndef _SingleHistSubprocessor_H
#define _SingleHistSubprocessor_H

#include "CalifaSubprocessor.h"
#include "TH1.h"
//a subprocessor which registers a single histogram
// you will have to overload this class to fill it. 
class SingleHistSubprocessor: public CalifaSubprocessor
{
 public:
  typedef CalifaParser::module_index_t module_index_t;
  SingleHistSubprocessor(std::string name,
			 int nbins,
			 int upperLimit, 
			 int lowerLimit=0
			 );
  //overload to fill histogram
  virtual void processEvent(CalifaParser* p)
  {
  };
  virtual TH1* getHist()
  {
    return this->h;
  }
 protected:
  TH1* h;
};
#endif
