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
  static uint32_t getMSB(uint32_t i);
  virtual void processSubevent(module_index_t cidx, eventinfo_t ei) override;
 protected:
  TH1* phase_h;
  virtual void reFFT(double* reX, double* imX, unsigned int N);
  virtual void cplxFFT(double* reX, double* imX, unsigned int N);
};
#endif
