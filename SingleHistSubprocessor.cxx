#include "SingleHistSubprocessor.h"
#include "debug.h"
#include <assert.h>


template<class T, int nAxis>
struct SingleHistSubprocessorHelper
{
  static T* createHist(std::string name, std::array<HistogramAxis, nAxis> ha, int rebin );
};

template<class T>
struct SingleHistSubprocessorHelper<T,1>
{
  static T* createHist(std::string name, std::array<HistogramAxis, 1> ha, int rebin )
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
  static T* createHist(std::string name, std::array<HistogramAxis, 2> ha, int rebin )
  {
    T* h= new T(name.c_str(), name.c_str(),
		ha[0].nBins/rebin, ha[0].min, ha[0].max,
		ha[1].nBins/rebin, ha[1].min, ha[1].max);
    h->GetXaxis()->SetTitle(ha[0].descr);
    h->GetYaxis()->SetTitle(ha[1].descr);
    return h;
  }
};

#include<map>
class TH1;
std::map<std::string, TH1*> histmap={};

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
  assert(nAxis==1);
  if(histmap.count(name)==0)
    {
      HistogramAxis tmp={"tmp", nbins, lowerLimit, upperLimit, NULL, 0};
      std::array<HistogramAxis, nAxis> ha{tmp};
      for (int i=0; i<nAxis; i++)
        ha[i]=tmp;
      this->h=SingleHistSubprocessorHelper<T, nAxis>::createHist(name, ha, 1);
      histmap[name]=this->h;
    }
  else
    {
      this->h=dynamic_cast<T*>(histmap[name]);
      printf("using existing %s\n",  this->h->GetName());
    }
  this->name=name;
  this->registerObject(histmap[name]);

}




template<class T, int nAxis>
SingleHistSubprocessor<T, nAxis>::SingleHistSubprocessor(std::string name,
                                                         std::array<HistogramAxis, nAxis> ha,                                    
							 int rebin)
{
  if(histmap.count(name)==0)
    {
      this->h=SingleHistSubprocessorHelper<T, nAxis>::createHist(name, ha, rebin);
      histmap[name]=this->h;
    }
  else
    {
      this->h=dynamic_cast<T*>(histmap[name]);
      printf("using existing %s\n",  this->h->GetName());
    }
  this->name=name;
  //printf("created %s\n", this->h->GetName());
  this->registerObject(this->h);
}


template class SingleHistSubprocessor<TH1D, 1>;
template class SingleHistSubprocessor<TH1I, 1>;
template class SingleHistSubprocessor<TH2I, 2>;
