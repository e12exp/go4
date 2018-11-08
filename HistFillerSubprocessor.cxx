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

static void appendIdxName(char* buf, size_t totlen, const char* prefix, CalifaParser::module_index_t idx, const char* postfix)
{
  auto start=strlen(buf);
  auto b=buf+start;
  auto len=totlen-start;
  if (idx==IDX_ANY)
    snprintf(b, len, "%s%s", prefix, postfix);
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxChannelIdx)
    snprintf(b, len, "%s_%01d_%02d_%02d%s", 
	     prefix,
	     GET_SFP(idx), GET_MOD(idx),
	     GET_CH(idx),
	     postfix);
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx)
    snprintf(b, len, "%s_%01d_%02d*%s", 
	     prefix,
	     GET_SFP(idx), GET_MOD(idx),
	     postfix);
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx)
    snprintf(b, len, "%s_petal_%01d%s", 
	     prefix,
	     GET_PETAL(idx),
	     postfix);
  return;
}

static void appendIdxPath(char* buf, size_t totlen, const char* prefix, CalifaParser::module_index_t idx)
{
  auto start=strlen(buf);
  auto b=buf+start;
  auto len=totlen-start;
  if (idx==IDX_ANY)
    ; // nothing
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxChannelIdx)
      snprintf(b, len, "%s/sfp_%01d/febex_%02d/", 
	       prefix,
	       GET_SFP(idx), GET_MOD(idx));
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx)
    snprintf(b, len, "%s/sfp_%01d/", 
	     prefix,
	     GET_SFP(idx)
	     );
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::petalIdx)
    snprintf(b, len, "%s/petals/", 
	     prefix//, GET_PETAL(idx)
	     );
  return;
}


template<class HistType, int nAxis, int nIdx, bool hasWeight>
const char*  HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis])
{
  //    char* buf=(char*)malloc(200);
  char* buf=(char*)malloc(200); //we are dealing with ROOT, a bit of memory will always leak
  buf[0]=0;
  char tmp[100];
  tmp[0]=0;
  for (int n=0; n<nAxis+hasWeight; n++)
    {
      size_t l=strlen(tmp);
      snprintf(tmp+l, 100-l, "%s%s", h[n].descr,
	       (n+1==nAxis+hasWeight)?"":(n+2==nAxis+hasWeight && hasWeight)?"_w_":"_vs_");
    }
  

  if (nIdx == 1)
    {
      appendIdxPath(buf, 200, tmp, idx[0]);
      appendIdxName(buf, 200, tmp, idx[0], "");
    }
  else
    {
      appendIdxPath(buf, 200, tmp, idx[0]);
      for (int n=0; n<nAxis+hasWeight; n++)
	{
	  size_t l=strlen(buf);
	  appendIdxName(buf, 200, h[n].descr, idx[n],
			(n+1==nAxis+hasWeight)?"":(n+2==nAxis+hasWeight && hasWeight)?"_w_":"_vs_");
	}      
    }
  //printf("created hist: %s\n", buf);
  return buf;
}

// also old?
void writeHistPath(char* out, int n, char* base, CalifaParser::module_index_t& idx)
{
  snprintf(out, n, "%s/sfp_%01d/febex_%02d/", base, GET_SFP(idx), GET_MOD(idx));
}

//old
const char* makeHistName(char* base, CalifaParser::module_index_t  *idx)
{
  char* buf=(char*)malloc(200);
  snprintf(buf, 200, "%s/sfp_%01d/febex_%02d/%s_%01d_%02d_%02d", 
	   base,
	   GET_SFP(idx[0]), GET_MOD(idx[0]),
	   base,
	   GET_SFP(idx[0]), GET_MOD(idx[0]),
	   GET_CH(idx[0]));
  return buf;
}

#include <TH1I.h>

template class HistFillerSubprocessor<TH(1, I), 1>;
template class HistFillerSubprocessor<TH(2, I), 1>;
template class HistFillerSubprocessor<TH(2, I), 2>;
template class HistFillerSubprocessor<TH(1, I), 2, 1>; // test only

template class HistFillerSubprocessor<TH2I, 2, 3, 1>;
