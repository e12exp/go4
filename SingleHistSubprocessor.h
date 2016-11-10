#ifndef _SingleHistSubprocessor_H
#define _SingleHistSubprocessor_H
#include "HistogramAxis.h"
#include "CalifaSubprocessor.h"
#include "TH2.h"
//a subprocessor which registers a single histogram
// you will have to overload this class to fill it. 

#define TH(x, y) TH##x##y, x

template<class T, int nAxis>
class SingleHistSubprocessor: public virtual CalifaSubprocessor
{
 public:
  typedef CalifaParser::module_index_t module_index_t;
  SingleHistSubprocessor(std::string name,
			 int nbins,
			 double upperLimit, 
			 double lowerLimit=0.0
			 );
  SingleHistSubprocessor(std::string name,
			 HistogramAxis* h,
			 int rebin=1);

  //overload to fill histogram
  virtual void processEvent(CalifaParser* p)
  {
  };
  virtual T* getHist()
  {
    return this->h;
  }
 protected:
  T* h=0;
  std::string name="";
};
#endif
