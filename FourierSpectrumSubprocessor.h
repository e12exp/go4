#ifndef FourierSpectrumSubprocessor_h
#define FourierSpectrumSubprocessor_h
#include "SingleChannelSubprocessor.h"
class FourierSpectrumSubprocessor : public SingleChannelSubprocessor
{
 public:
  FourierSpectrumSubprocessor(char* name,
			      char* phasename,
		     module_index_t idx,
		     int nbins,
		     double upperLimit, 
		     double lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
  static uint32_t getMSB(uint32_t i);
 protected:
  TH1* phase_h;
  virtual void reFFT(double* reX, double* imX, unsigned int N);
  virtual void cplxFFT(double* reX, double* imX, unsigned int N);
};
#endif
