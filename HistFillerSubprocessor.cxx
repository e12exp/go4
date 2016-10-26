#include "HistFillerSubprocessor.h"
#include <tuple>
//#include  <boost/preprocessor/control/while.hpp>
//#include <boost/preprocessor/repetition/repeat.hpp>
//makeHistName<1,1>(h[0].descr, idx)
template<class HistType, int nAxis, int nIdx>
HistFillerSubprocessor<HistType, nAxis, nIdx>::HistFillerSubprocessor(module_index_t idx[nIdx],
								      HistogramAxis h[nAxis],
								      int rebin):
  SingleHistSubprocessor<HistType, nAxis>
  (this->makeHistName(idx, h), h, rebin)
{
  for (int i=0; i<nIdx; i++)
    this->idx[i]=idx[i];
  for (int i=0; i<nAxis; i++)
    this->getVal[i]=h[i].getValue;

  if (nIdx!=1)
    for (int i=0; i<nIdx; i++)
      if (idx[i]==IDX_ANY)
	{
	  fprintf(stderr, "Multiple ANY indices are not allowed!\n");
	  exit(-1);
	}
}

template<class HistType, int nAxis, int nIdx>
void HistFillerSubprocessor<HistType, nAxis, nIdx>::processEvent(CalifaParser* p)
{
  if (this->idx[0]!=IDX_ANY)
    this->processEventIdx(p, this->idx);
  else
    {
      CalifaParser::eventmap_t* evts=p->getCalifaEvents();
      for (auto i=evts->begin(); i!=evts->end(); ++i)
	{
	   auto single_idx=i->first;
	   this->processEventIdx(p, &single_idx);
	}
    }
}

template<class HistType, int nAxis, int nIdx>
void HistFillerSubprocessor<HistType, nAxis, nIdx>::processEventIdx(CalifaParser* p, module_index_t idx[nIdx])
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


template<class HistType, int nAxis, int nIdx>
const char*  HistFillerSubprocessor<HistType, nAxis, nIdx>::makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis])
{
  //    char* buf=(char*)malloc(200);
  char* buf=(char*)malloc(200);
  char tmp[100];

  if (nAxis == 1)
    snprintf(tmp, 100, h[0].descr);
  else
    snprintf(tmp, 100, "%s_vs_%s", h[0].descr, h[1].descr);
  if (nIdx == 1 && idx[0]==IDX_ANY)
    {
      snprintf(buf, 200, "%s", tmp);
    }
  else if (nIdx == 1)
    {
      snprintf(buf, 200, "%s/sfp_%01d/febex_%02d/%s_%01d_%02d_%02d", 
	       tmp,
	       std::get<0>(idx[0]), std::get<1>(idx[0]),
	       tmp,
	       std::get<0>(idx[0]), std::get<1>(idx[0]),
	              std::get<2>(idx[0]));
    }
  else
    {
      snprintf(buf, 200, "%s/sfp_%01d/febex_%02d/%s_%01d_%02d_%02d_vs_%01d_%02d_%02d", 
	       tmp,
	       std::get<0>(idx[0]), std::get<1>(idx[0]),
	       tmp,
	       std::get<0>(idx[0]), std::get<1>(idx[0]),
	       std::get<2>(idx[0]),
	       std::get<0>(idx[1]), std::get<1>(idx[1]),
	       std::get<2>(idx[1]));
    }
  return buf;
}


void writeHistPath(char* out, int n, char* base, CalifaParser::module_index_t& idx)
{
  snprintf(out, n, "%s/sfp_%01d/febex_%02d/", base, std::get<0>(idx), std::get<1>(idx));
}

//old
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
template class HistFillerSubprocessor<TH2I, 2, 1>;
template class HistFillerSubprocessor<TH2I, 2, 2>;
