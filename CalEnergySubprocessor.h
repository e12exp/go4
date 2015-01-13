#ifndef CalEnergySubprocessor_h
#define CalEnergySubprocessor_h
#include "SingleChannelSubprocessor.h"
#include "TF1.h"
class CalEnergySubprocessor : public SingleChannelSubprocessor
{
 public:
  CalEnergySubprocessor(std::string name,
			module_index_t idx,
			int nbins,
			double upperLimit, 
			double lowerLimit=0);
  virtual void processSubevent(eventinfo_t ei);
 protected:
  const TF1* chToEnergy;
  typedef std::map<module_index_t, const TF1*> calmap_t;
  static calmap_t *cal;
  static const TF1* getCal(module_index_t);
};
#endif
