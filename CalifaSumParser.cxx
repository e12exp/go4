#include "CalifaSumParser.h"
#include "EnergyCal.h"
#include "assert.h"
int CalifaSumParser::parseGo4(TGo4MbsEvent *evt)
{
  int r=this->CalifaParser::parseGo4(evt);
  if (r)
    return r;
  
  for (auto it: *(this->getCalifaEvents()))
    if (it.second.evnt)
      {
	auto idx=it.first;
	auto sfpNo=    idx.sfp;
	auto moduleNo= idx.mod;
	auto cal=EnergyCal::getCal(idx);
	// energies below bin 75 are noise anyways, ignore
	if (!cal || it.second.evnt->energy < 75)
	  continue;
	auto en=cal->Eval(it.second.evnt->energy+((double)rand())/RAND_MAX-0.5);
	this->eventmap[idx].calEnergy=en;
	for (int sumTypeI=1; sumTypeI<3; sumTypeI++)
	  {
	    auto st=static_cast<subEventIdxType>(sumTypeI);
	    auto midx=toIdxType(st, idx);
	    if (this->eventmap.count(midx))
	      this->eventmap[midx].calEnergy+=en;
	    else
	      {
		auto ei=&this->eventmap[midx];
		memset(ei, 0, sizeof(eventinfo_t));
		this->eventmap[midx].calEnergy=en;
	      }
	  } // sumType loop
      } // eventmap loop
  return 0;
}
