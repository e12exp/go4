#ifndef _ChannelCutHistFillerSubprocessor_h
#define _ChannelCutHistFillerSubprocessor_h

#include "ChannelCutSubprocessor.h"
#include "HistFillerSubprocessor.h"
//OBSOLETE

template<class HistType, int nAxis, int nIdx>
class ChannelCutHistFillerSubprocessor:
  public HistFillerSubprocessor<HistType, nAxis, nIdx>,
  public ChannelCutSubprocessor
{
 public:
  ChannelCutHistFillerSubprocessor( CalifaParser::module_index_t idx[nIdx],
				    HistogramAxis h[nAxis],
				    int rebin,
				    CalifaParser::module_index_t* cut_idx,
				    HistogramAxis* cut_axis,
				    double cutMin=-INFINITY,
  				    double cutMax=INFINITY );
  
};

#endif
