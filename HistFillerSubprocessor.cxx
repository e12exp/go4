#include "HistFillerSubprocessor.h"
#include <tuple>
//#include  <boost/preprocessor/control/while.hpp>
//#include <boost/preprocessor/repetition/repeat.hpp>
//makeHistName<1,1>(h[0].descr, idx)
#include <assert.h>
#include <vector>
#include <utility>

template<class HistType, int nAxis, int nIdx, bool hasWeight>
HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::HistFillerSubprocessor(module_index_t idx[nIdx],
								      HistogramAxis h[nAxis],
								      int rebin):
  SingleHistSubprocessor<HistType, nAxis>
  (this->makeHistName(idx, h), h, rebin)
{
  for (int i=0; i<nIdx; i++)
    this->idx[i]=idx[i];
  for (int i=0; i<nAxis+hasWeight; i++)
    this->getVal[i]=h[i].getValue;

  if (nIdx!=1)
    for (int i=0; i<nIdx; i++)
      assert(idx[i]!=IDX_ANY);
}


template<class HistType, int nAxis, int nIdx, bool hasWeight>
void HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::processEvent(CalifaParser* p)
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

// this is bad, but still better than std::integer_sequence
template<class HistType, int n>
struct HistFill
{
  static void foo(HistType* h, double *t);
};

template<class HistType> struct HistFill<HistType, 1>{ static void foo(HistType* h, double *t) { h->Fill(t[0]); }};
template<class HistType> struct HistFill<HistType, 2>{ static void foo(HistType* h, double *t) { h->Fill(t[0], t[1]); }};
template<class HistType> struct HistFill<HistType, 3>{ static void foo(HistType* h, double *t) { h->Fill(t[0], t[1], t[2]); }};


template<class HistType, int nAxis, int nIdx, bool hasWeight>
void HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::processEventIdx(CalifaParser* p, module_index_t idx[nIdx])
{
  double r[nAxis+hasWeight];
  for (int i=0; i<nAxis+hasWeight; i++)
    {
      r[i]=this->getVal[i](p, &(idx[i%nIdx])); // see static assert in header
      if (isnan(r[i]))
	return;
    }
  HistFill<HistType, nAxis+hasWeight>::foo(this->h, r);

  // ROOT abuses http://stackoverflow.com/questions/2986891/ to restrict TH2::Fill(double). Workaround.
  /*  if (nAxis+hasWeight==1)
    static_cast<TH1*>(this->h)->Fill(r[0]);
  else if (nAxis+hasWeight==1)
    this->h->Fill(r[0], r[1]); // TH2 or TH1+weight
  else
    this->h->Fill(r[0], r[1], r[2]); //  TH2+weight
    return;*/
  
}


template<class HistType, int nAxis, int nIdx, bool hasWeight>
const char*  HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis])
{
  //    char* buf=(char*)malloc(200);
  char* buf=(char*)malloc(200); //we are dealing with ROOT, a bit of memory will always leak
  char tmp[100];
  tmp[0]=0;
  for (int n=0; n<nAxis+hasWeight; n++)
    {
      size_t l=strlen(tmp);
      snprintf(tmp+l, 100-l, "%s%s", h[n].descr,
	       (n+1==nAxis+hasWeight)?"":(n+2==nAxis+hasWeight && hasWeight)?"_w_":"_vs_");
    }
  
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
      snprintf(buf, 200, "%s/sfp_%01d/febex_%02d/", 
	       tmp,
	       std::get<0>(idx[0]), std::get<1>(idx[0]));
      for (int n=0; n<nAxis+hasWeight; n++)
	{
	  size_t l=strlen(buf);
	  snprintf(buf+l, 200-l, "%s_%01d_%02d_%02d%s", h[n].descr,
		   std::get<0>(idx[n]), std::get<1>(idx[n]),
		   std::get<2>(idx[n]),
		   (n+1==nAxis+hasWeight)?"":(n+2==nAxis+hasWeight && hasWeight)?"_w_":"_vs_");
	}      
    }
  printf("created hist: %s\n", buf);
  return buf;
}

// also old?
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

template class HistFillerSubprocessor<TH(1, I), 1>;
template class HistFillerSubprocessor<TH(2, I), 1>;
template class HistFillerSubprocessor<TH(2, I), 2>;
template class HistFillerSubprocessor<TH(1, I), 2, 1>; // test only

template class HistFillerSubprocessor<TH2I, 2, 3, 1>;
