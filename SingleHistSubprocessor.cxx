#include "SingleHistSubprocessor.h"
#include "debug.h"
SingleHistSubprocessor::SingleHistSubprocessor(std::string name,
					       int nbins,
					       int upperLimit, 
					       int lowerLimit
					       )
{
  //  linfo("creating TH1I(%s, %s, %d, %d, %d)")
  //Yeah, TH1D suck for counting, but until ROOT supports templates 
  // (fat chance!) we will stick to this for simplicity
  this->h=new TH1D(name.c_str(), name.c_str(), nbins, lowerLimit, upperLimit);
  this->registerObject(h);
}


