#ifndef _SingleHistSubprocessor_H
#define _SingleHistSubprocessor_H

#include "CalifaSubprocessor.h"
#include <functional>
#include "TH1.h"
class SingleHistSubprocessor: public CalifaSubprocessor
{
 public:
  typedef std::function<double(CalifaParser*)> update_t;
  typedef std::function<int(CalifaParser*)> filter_t;
  SingleHistSubprocessor(std::string name,
			 update_t f,
			 int nbins,
			 int upperLimit, 
			 int lowerLimit=0,
			 filter_t filter=[](CalifaParser* p){return 1; }
			 );
  virtual std::list<TObject*> makeHists();
  virtual void processEvent(CalifaParser* p);
 protected:
  TH1* h;
  update_t f;
  filter_t filter;
};
#endif
