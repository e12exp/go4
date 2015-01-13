#include "CalEnergySubprocessor.h"
#include <stdio.h>
#define FUN (CalEnergySubprocessor::getCal(idx))
#define EN(bin, def) FUN?FUN->Eval(bin):def

CalEnergySubprocessor::calmap_t* CalEnergySubprocessor::cal=NULL;
CalEnergySubprocessor::CalEnergySubprocessor(std::string name,
				       module_index_t idx,
				       int nbins,
				       double upperLimit, 
				       double lowerLimit)
  : SingleChannelSubprocessor(name, idx, nbins,
			      EN(upperLimit, 1),
			      EN(lowerLimit, 0))
{
  this->chToEnergy=CalEnergySubprocessor::getCal(idx);
}

void CalEnergySubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.evnt && this->chToEnergy)
    this->h->Fill(this->chToEnergy->Eval(ei.evnt->energy));
}
const TF1* CalEnergySubprocessor::getCal(module_index_t idx)
{
  if (!CalEnergySubprocessor::cal)
    {
      int count=0;
      CalEnergySubprocessor::cal=new calmap_t;
      FILE* f=fopen("en_cal.txt", "r");
      if (!f)
	lerror("could not open calibration file en_cal.txt\n");
      else
	{
	  module_index_t idx;
	  char buf[21];
	  while(fscanf(f, " %hhx %hhx %hhx %20s",
			 &(std::get<0>(idx)),
			 &(std::get<1>(idx)),
			 &(std::get<2>(idx)),
				 buf)==4)
	    {
	      char name[30];
	      snprintf(name, 30, "en_cal_%d", count++);
	      (*CalEnergySubprocessor::cal)[idx]=new TF1(name, buf);
	      linfo("read channel calibration %x %x %x: %s\n", 
		    (std::get<0>(idx)),
		    (std::get<1>(idx)),
		    (std::get<2>(idx)),buf);
	    }
	  lerror("read %d calibration lines\n", count);
	  fclose(f);
	}
    }
  if (CalEnergySubprocessor::cal->count(idx))
    return (*CalEnergySubprocessor::cal)[idx];
  else
    return NULL;
}
