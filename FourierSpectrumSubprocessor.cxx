#include "FourierSpectrumSubprocessor.h"
#include "HistFillerSubprocessor.h"
#include <math.h>
#include <iostream>
FourierSpectrumSubprocessor::FourierSpectrumSubprocessor(char* name,
							 char* phasename,
						 module_index_t idx,
						 int nbins,
						 double upperLimit, 
						 double lowerLimit)
  : SingleChannelSubprocessor(name, idx, 
			      FourierSpectrumSubprocessor::getMSB((unsigned int)nbins)/2,
			      upperLimit, lowerLimit)
{  
  this->phase_h=new TH1D(makeHistName(phasename, &idx),
			 phasename,
			 FourierSpectrumSubprocessor::getMSB((unsigned int)nbins)/2,
			 lowerLimit, upperLimit);
  this->registerObject(this->phase_h);
}

uint32_t FourierSpectrumSubprocessor::getMSB(uint32_t i)
{
  //calculate the highest power of 2 smaller than ei.tracepoints
  //int n=1<<((int)floor(log2(n)));
  uint32_t n=1;
  while (n<<1 < i)
    n<<=1;
  return n;
}


void FourierSpectrumSubprocessor::processSubevent(eventinfo_t ei)
{
  if (ei.trace)
    {
      uint32_t n=FourierSpectrumSubprocessor::getMSB(ei.tracepoints);
      double *reX=(double*)malloc(sizeof(double)*n);
      double *imX=(double*)malloc(sizeof(double)*n);
      for (uint32_t i=0; i<n; i++)
	{
	  reX[i]=getTracePoint(ei.trace, i+1);
	  imX[i]=0.0;
	}
      this->reFFT(reX, imX, n);
      for (uint32_t i=0; i<n/2; i++)
	{
	  this->h->Fill(i*F_ADC/(1.0*n), sqrt(reX[i]*reX[i]+imX[i]*imX[i]));
	  //std::cout << i << ":" << i*F_ADC/(2.0*n) << " " << sqrt(reX[i]*reX[i]+imX[i]*imX[i]) << std::endl;
	  this->phase_h->Fill(i*F_ADC/(1.0*n), atan2(imX[i], reX[i]));
	}
      this->h->SetBinContent(1, 0.0);
      free(reX);
      free(imX);
    }
}

void FourierSpectrumSubprocessor::reFFT(double *ReX, double *ImX, unsigned int N)
{
  unsigned int i, im, ip2, ipm, ip;
  double TR, TI;

  memset(ImX, 0, sizeof(double)*N);

  // Transform real values to complex values
  for(i = 0; i < N/2; i++)
  {
    ReX[i] = ReX[2*i];
    ImX[i] = ReX[2*i + 1];
  }

  // Perfom complex fast fourier transform
  this->cplxFFT(ReX, ImX, N/2);

  // Even/odd frequency decomposition
  for(i = 1; i < N/4; i++)
  {
    im = N/2 - i;
    ip2 = i + N/2;
    ipm = im + N/2;
    ReX[ip2] = (ImX[i] + ImX[im])/2;
    ReX[ipm] = ReX[ip2];
    ImX[ip2] = -(ReX[i] - ReX[im])/2;
    ImX[ipm] = -ImX[ip2];
    ReX[i] = (ReX[i] + ReX[im])/2;
    ReX[im] = ReX[i];
    ImX[i] = (ImX[i] - ImX[im])/2;
    ImX[im] = -ImX[i];
  }

  ReX[N*3/4] = ImX[N/4];
  ReX[N/2] = ImX[0];
  ImX[N*3/4] = 0;
  ImX[N/2] = 0;
  ImX[N/4] = 0;
  ImX[0] = 0;

  unsigned int l = (unsigned int)log2(N);
  unsigned int le = 1 << l;
  unsigned int le2 = 1 << (l-1);
  double ur = 1;
  double ui = 0;
  double sr = cos(M_PI/(double)le2);
  double si = -sin(M_PI/(double)le2);

  for(unsigned j = 1; j <= le2; j++)
  {
    for(i = j - 1; i < N; i += le)
    {
      ip = i + le2;
      TR = ReX[ip]*ur - ImX[ip]*ui;
      TI = ReX[ip]*ui + ImX[ip]*ur;
      ReX[ip] = ReX[i] - TR;
      ImX[ip] = ImX[i] - TI;
      ReX[i] += TR;
      ImX[i] += TI;
    }
    TR = ur;
    ur = TR*sr - ui*si;
    ui = TR*si + ui*sr;
  }
}


void FourierSpectrumSubprocessor::cplxFFT(double *ReX, double *ImX, unsigned int N)
{
  double TR, TI;

  unsigned int M = (int)log2(N);
  unsigned int j = N/2;
  unsigned int k;

  unsigned int i, l, le, le2, ip;

  double sr, si, ur, ui;

  // Bit reversal sorting
  for(i = 1; i < N-1; i++)
  {
    if(i < j)
    {
      TR = ReX[j];
      TI = ImX[j];
      ReX[j] = ReX[i];
      ImX[j] = ImX[i];
      ReX[i] = TR;
      ImX[i] = TI;
    }
    k = N/2;
    while(k <= j)
    {
      j -= k;
      k /= 2;
    }
    j += k;
  }

  // Loop over assembling stages
  for(l = 1; l <= M; l++)
  {
    le = 1 << l;
    le2 = 1 << (l-1);

    ur = 1;
    ui = 0;

    sr = cos(M_PI / (double)le2);
    si = -sin(M_PI / (double)le2);

    // Loop over sub DFTs
    for(j = 1; j <= le2; j++)
    {
      // Loop over butterflies
      for(i = j-1; i < N; i += le)
      {
        ip = i + le2;
        // Butterfly
        TR = ReX[ip]*ur - ImX[ip]*ui;
        TI = ReX[ip]*ui + ImX[ip]*ur;
        ReX[ip] = ReX[i]-TR;
        ImX[ip] = ImX[i]-TI;
        ReX[i] += TR;
        ImX[i] += TI;
      }
      TR = ur;
      ur = TR*sr - ui*si;
      ui = TR*si + ui*sr;
    }
  }
}
