#ifndef FourierSpectrumSubprocessor_h
#define FourierSpectrumSubprocessor_h
#include "SingleChannelSubprocessor.h"
class FourierSpectrumSubprocessor : public SingleChannelSubprocessor
{
 public:
  FourierSpectrumSubprocessor(std::string name,
			      std::string phasename,
		     module_index_t idx,
		     int nbins,
		     int upperLimit, 
		     int lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
 protected:
  TH1* phase_h;
  virtual void reFFT(double* reX, double* imX, unsigned int N);
  virtual void cplxFFT(double* reX, double* imX, unsigned int N);
};
#endif
