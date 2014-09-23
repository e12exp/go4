#include "SingleHistSubprocessor.h"
#include "debug.h"
SingleHistSubprocessor::SingleHistSubprocessor(std::string name,
					       update_t f,
					       int nbins,
					       int upperLimit, 
					       int lowerLimit,
					       filter_t filter)
{
  //  linfo("creating TH1I(%s, %s, %d, %d, %d)")
  this->h=new TH1I(name.c_str(), name.c_str(), nbins, lowerLimit, upperLimit);
  this->f=f;
}

std::list<TObject*> SingleHistSubprocessor::makeHists()
{
  std::list<TObject*> objs(1, this->h);
  return objs;
}

void SingleHistSubprocessor::processEvent(CalifaParser* p)
{
  //  if (this->filter(p))
    this->h->Fill(this->f(p));
}
