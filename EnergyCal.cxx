#include "EnergyCal.h"
#include "TRandom.h"
#include <stdio.h>
#include "debug.h"
#define FUN (EnergyCal::getCal(idx))
#define EN(bin, def) FUN?FUN->Eval(bin):def

EnergyCal::calmap_t* EnergyCal::cal=NULL;

const TF1* EnergyCal::getCal(module_index_t idx)
{
  //assert(0 && "GetCal should be rewritten to use pc ids.");
  /*
  if (!EnergyCal::cal)
    {
      int count=0;
      EnergyCal::cal=new calmap_t;
      FILE* f=fopen("en_cal.txt", "r");
      if (!f)
	lerror("could not open calibration file en_cal.txt\n");
      else
	{
	  module_index_t idx;
	  char buf[51];
	  while(fscanf(f, " %hhd %hhd %hhd %50s",
			 &(idx.sfp),
			 &(idx.mod),
			 &(idx.ch),
				 buf)==4)
	    {
	      char name[30];
	      snprintf(name, 30, "en_cal_%d", count++);
	      (*EnergyCal::cal)[idx]=new TF1(name, buf);
	      linfo("read channel calibration 0x%x 0x%x 0x%x: %s\n", 
		    (idx.sfp),
		    (idx.mod),
		    (idx.ch),buf);
	    }
	  lerror("read %d calibration lines\n", count);
	  fclose(f);
	}
    }
  if (EnergyCal::cal->count(idx))
    return (*EnergyCal::cal)[idx];
  else
  */
    return NULL;
}
