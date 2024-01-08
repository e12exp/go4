#include "HistFillerSubprocessor.h"
#include <tuple>
//#include  <boost/preprocessor/control/while.hpp>
//#include <boost/preprocessor/repetition/repeat.hpp>
//makeHistName<1,1>(h[0].descr, idx)
#include <assert.h>
#include <vector>
#include <utility>

template<class HistType, int nAxis, int nIdx, bool hasWeight>
HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::HistFillerSubprocessor(std::array<module_index_t, nIdx> idx,
                                                                                 std::array<HistogramAxis, nAxis> h,
                                                                                 int rebin):
  SingleHistSubprocessor<HistType, nAxis>
  (this->makeHistName(idx, h), h, rebin)
{
  for (int i=0; i<nIdx; i++)
    this->idx[i]=idx[i];
  for (int i=0; i<nAxis+hasWeight; i++)
    this->getVal[i]=h[i].getValue;

  if (nIdx>2)
    for (int i=0; i<nIdx; i++)
      assert(idx[i]!=IDX_ANY && "May not use IDX_ANY for nIdx>2");

  if (nIdx==2)
    assert( (idx[0]==IDX_ANY)==(idx[1]==IDX_ANY) && "Either both indices must be any or none of them");
  
}


template<class T, int nIdx>
struct AnyHelper
{
  static void processEventAny(T, CalifaParser* p)
  {
    assert(0 && "IDX_ANY not supported for nIdx>2");
  }
};

template<class T>
struct AnyHelper<T, 1>
{
  static void processEventAny(T t, CalifaParser* p)
  {
    CalifaParser::eventmap_t* evts=p->getCalifaEvents();
    for (auto i=evts->begin(); i!=evts->end(); ++i)
      {
        auto single_idx=i->first;
        t->processEventIdx(p, &single_idx);
      }
  }
};
  
template<class T>
struct AnyHelper<T, 2>
{
  static void processEventAny(T t, CalifaParser* p)
  {
    CalifaParser::eventmap_t* evts=p->getCalifaEvents();
    for (const auto& i: *evts)
      for (const auto& j: *evts)
        {
          CalifaParser::module_index_t idx[2]={i.first, j.first};
          if (i.first!=j.first)
            t->processEventIdx(p, idx);
        }
  }
};

template<class HistType, int nAxis, int nIdx, bool hasWeight>
void HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::processEvent(CalifaParser* p)
{
  if (this->idx[0]!=IDX_ANY)
    this->processEventIdx(p, this->idx);
  else
    AnyHelper<decltype(this), nIdx>::processEventAny(this, p);  
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
  if (idx==IDX_ANY || idx==IDX_EVENT)
    snprintf(b, len, "%s%s", prefix, postfix);
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxChannelIdx)
    snprintf(b, len, "%s_%01d_%02d_%02d%s", 
	     prefix,
	     GET_SFP(idx), GET_MOD(idx),
	     GET_CH(idx),
	     postfix);
  else if (GET_TYPE(idx)==CalifaParser::subEventIdxType::fbxModuleIdx)
    snprintf(b, len, "%s_%01d_%02d_*%s", 
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
  if (idx==IDX_ANY || idx==IDX_EVENT)
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
const char*  HistFillerSubprocessor<HistType, nAxis, nIdx, hasWeight>::makeHistName(std::array<CalifaParser::module_index_t, nIdx> idx,
                                                                                    std::array<HistogramAxis, nAxis> h)
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

 template class HistFillerSubprocessor<TH(2, I), 2> ;
 template class HistFillerSubprocessor<TH(1, I), 2, 1>; // test only

 template class HistFillerSubprocessor<TH2I, 2, 3, 1> ;

//class foo: public HistFillerSubprocessor<TH(1, I), 1, 0> {};
