#include "HistFillerSubprocessor.h"
#include <tuple>
#include  <boost/preprocessor/control/while.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
//makeHistName<1,1>(h[0].descr, idx)
template<class HistType, int nAxis, int nIdx>
HistFillerSubprocessor<HistType, nAxis, nIdx>::HistFillerSubprocessor(module_index_t idx[nIdx],
								      HistogramAxis h[nAxis]):
  SingleHistSubprocessor<HistType, nAxis>(
					  makeHistName(h[0].descr, &(idx[0]))
					  , h)
{
  for (int i=0; i<nIdx; i++)
    this->idx[i]=idx[i];
  for (int i=0; i<nAxis; i++)
    this->getVal[i]=h[i].getValue;
}

template<class HistType, int nAxis, int nIdx>
void HistFillerSubprocessor<HistType, nAxis, nIdx>::processEvent(CalifaParser* p)
{
  
  double r[nAxis];
  for (int i=0; i<nAxis; i++)
    {
      r[i]=this->getVal[i](p, &(idx[i%nIdx]));
      if (isnan(r[i]))
	return;
    }
  if (nAxis==1)
    this->h->Fill(r[0]);
  else
    this->h->Fill(r[0], r[1]);
  return;
}

#if 0
template<class HistType, int nAxis, int nIdx>
const char*  HistFillerSubprocessor<HistType, nAxis, nIdx>::makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis])
{
  //    char* buf=(char*)malloc(200);
    char tmp[200];
    if (nAxis==1 && nIdx == 1)
      return
}


void writeHistPath(char* out, int n, char* base, CalifaParser::module_index_t& idx)
{
  snprintf(out, n, "%s/sfp_%01d/febex_%02d/", base, std::get<0>(idx), std::get<1>(idx));
}

#endif
//template<int nAxis, int nIdx>
const char* makeHistName(char* base, CalifaParser::module_index_t  *idx)
{
  char* buf=(char*)malloc(200);
  snprintf(buf, 200, "%s/sfp_%01d/febex_%02d/%s_%01d_%02d_%02d", 
	   base,
	   std::get<0>(idx[0]), std::get<1>(idx[0]),
	   base,
	   std::get<0>(idx[0]), std::get<1>(idx[0]),
	   std::get<2>(idx[0]));
  return buf;
}

#include <TH1I.h>

template class HistFillerSubprocessor<TH1I, 1, 1>;
