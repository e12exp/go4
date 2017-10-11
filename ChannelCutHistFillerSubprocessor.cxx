#include "ChannelCutHistFillerSubprocessor.h"

template class ChannelCutHistFillerSubprocessor<TH(1,I),1>;
template class ChannelCutHistFillerSubprocessor<TH(2,I),1>;

template<class HistType, int nAxis, int nIdx>
ChannelCutHistFillerSubprocessor<HistType, nAxis, nIdx>::
ChannelCutHistFillerSubprocessor( CalifaParser::module_index_t idx[nIdx],
				  HistogramAxis h[nAxis],
				  int rebin,
				  CalifaParser::module_index_t* cut_idx,
				  HistogramAxis* cut_axis,						   
				  double cutMin,
				  double cutMax)
  :HistFillerSubprocessor<HistType, nAxis, nIdx>(idx, h, rebin),
   ChannelCutSubprocessor(cut_idx, cut_axis, cutMin, cutMax)
{
  
}

