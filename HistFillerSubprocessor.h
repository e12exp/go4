#ifndef _HistFillerSubprocessor_h
#define _HistFillerSubprocessor_h
#include "SingleHistSubprocessor.h"

template<class HistType, int nAxis, int nIdx>
  class HistFillerSubprocessor:
  public SingleHistSubprocessor<HistType, nAxis>
{
 public:
  typedef CalifaParser::module_index_t module_index_t;
  //  typename SingleHistSubprocessor<HistType, nAxis>::module_index_t module_index_t;
  //new constructor: provide HistogramAxis
  HistFillerSubprocessor( module_index_t idx[nIdx],
			  HistogramAxis h[nAxis],
			  int rebin=1 );
  
  const char* makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis]);

  virtual void processEvent(CalifaParser* p);

 protected:
  module_index_t idx[nIdx];
  typedef double (*eval_t)(CalifaParser* parser, CalifaParser::module_index_t* idx) ;
  //function pointer array to the function which extracts our data
  eval_t getVal[nAxis];
  
};



const char* makeHistName(char* base, CalifaParser::module_index_t* idx);

#endif
