#include "SingleHistSubprocessor.h"
#include "debug.h"

template<class T, int nAxis>
SingleHistSubprocessor<T, nAxis>::SingleHistSubprocessor(std::string name,
					       int nbins,
					       double upperLimit, 
					       double lowerLimit
					       )
{
  //  linfo("creating TH1I(%s, %s, %d, %d, %d)")
  //Yeah, TH1D suck for counting, but until ROOT supports templates 
  // (fat chance!) we will stick to this for simplicity
  this->h=new TH1D(name.c_str(), name.c_str(), nbins, lowerLimit, upperLimit);
  this->registerObject(h);
}


template<class T, int nAxis>
struct SingleHistSubprocessorHelper
{
  static T* createHist(std::string name, HistogramAxis* ha, int rebin );
};

template<class T>
struct SingleHistSubprocessorHelper<T,1>
{
  static T* createHist(std::string name, HistogramAxis* ha, int rebin )
  {
    T* h= new T(name.c_str(), name.c_str(),
		ha[0].nBins/rebin, ha[0].min, ha[0].max);
    h->GetXaxis()->SetTitle(ha[0].descr);
    return h;
  }
};

template<class T>
struct SingleHistSubprocessorHelper<T,2>
{
  static T* createHist(std::string name, HistogramAxis* ha, int rebin )
  {
    T* h= new T(name.c_str(), name.c_str(),
		ha[0].nBins/rebin, ha[0].min, ha[0].max,
		ha[1].nBins/rebin, ha[1].min, ha[1].max);
    h->GetXaxis()->SetTitle(ha[0].descr);
    h->GetYaxis()->SetTitle(ha[1].descr);
    return h;
  }
};


template<class T, int nAxis>
SingleHistSubprocessor<T, nAxis>::SingleHistSubprocessor(std::string name,
							 HistogramAxis* ha,
							 int rebin)
{
  this->h=SingleHistSubprocessorHelper<T, nAxis>::createHist(name, ha, rebin);
  this->registerObject(this->h);
}


template class SingleHistSubprocessor<TH1D, 1>;
template class SingleHistSubprocessor<TH1I, 1>;
template class SingleHistSubprocessor<TH2I, 2>;
