#include "SimpleSubprocessor.h"
#include "debug.h"

void SimpleSubprocessor::processEvent(CalifaParser* p)
{
  linfo("entering processEvent()\n");
  if (!(p->getCalifaEvents()->count(idx)))
    {
      lerror("no such index\n");
      return;
    }
  eventinfo_t* ei=&(p->getCalifaEvents()->at(idx));
  if (ei && ei->evnt)
    this->hist->Fill(ei->evnt->energy);
  else
    lerror("Element not found in container of size %d\n", 
	   p->getCalifaEvents()->size());
}

std::list<TObject*> SimpleSubprocessor::makeHists()
{
  this->hist=new TH1I("event_count", "event_count", 500, 0.0, 10000.0);
  std::list<TObject*> objs(1, this->hist);
  this->idx=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx,0,0,0);
  return objs;
}
