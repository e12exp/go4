#ifndef _HistFillerSubprocessor_h
#define _HistFillerSubprocessor_h
#include "SingleHistSubprocessor.h"
#include <cstdbool>

template<class HistType, int nAxis, int nIdx, bool hasWeight=0>
  class HistFillerSubprocessor:
  public SingleHistSubprocessor<HistType, nAxis>
{
  // either you provide one index, which will be used for any axes, or
  // provide one for each axis (including the weight axis). 
  static_assert( nIdx==1                         // one index for everything
		 || ( nIdx == nAxis+hasWeight),   // different index for everything, 
		 "either you provide one index, which will be used for any axes, or "
		 "provide one for each axis (including the weight axis, if hasWeight)"); 

 public:
  typedef CalifaParser::module_index_t module_index_t;
  //  typename SingleHistSubprocessor<HistType, nAxis>::module_index_t module_index_t;
  //new constructor: provide HistogramAxis
  HistFillerSubprocessor( module_index_t idx[nIdx],
			  HistogramAxis h[nAxis],
			  int rebin=1);
  
  const char* makeHistName(CalifaParser::module_index_t idx[nAxis], HistogramAxis h[nAxis]);

  virtual void processEvent(CalifaParser* p);
 protected:
  virtual void processEventIdx(CalifaParser* p, module_index_t idx[nIdx]);

  virtual CalifaParser::module_index_t getSensitivity()
  {
    return idx[0]; 
  }

  module_index_t idx[nIdx];
  typedef double (*eval_t)(CalifaParser* parser, CalifaParser::module_index_t* idx) ;
  //function pointer array to the function which extracts our data
  eval_t getVal[nAxis];
  
};

const char* makeHistName(char* base, CalifaParser::module_index_t* idx);

#endif
