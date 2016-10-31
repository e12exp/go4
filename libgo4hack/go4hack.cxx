#ifndef  _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#define ROOT_TVersionCheck //avoid instantiating TVersionCheck via include guard
#include <TH1.h>
#include <THistPainter.h>
#include <TVirtualPad.h>
//static TGo4Script* TGo4Script::ScriptInstance() is protected. no problem.
#define protected public
#include <TGo4Script.h>
#undef protected

#define NAME_LEN 256
#include <functional>
#include <map>
#include <string>

#include <Buttons.h>
#define LC kButton1Down


static void getHistname(char out[NAME_LEN], const char* basename, uint8_t sfp, uint8_t module, uint8_t channel)
{
  snprintf(out, 256, "Analysis/Histograms/%s/sfp_%01d/febex_%02d/%s_%01d_%02d_%02d", 
	   basename,
	   sfp, module,
	   basename,
	   sfp, module, channel);
  return;
}

typedef std::map<std::pair<std::string, Int_t>,
		 std::function<void(char[NAME_LEN], uint8_t, uint8_t)> > onClickMap_t;

static onClickMap_t onClickMap={
  {{"fbx_sfp0_module_vs_fbx_channel", LC}, [](auto out, auto m, auto c){getHistname(out, "lim_energy", 0, m, c);} }
  
};


typedef void (*ee_t)(THistPainter*, Int_t, Int_t, Int_t);

void THistPainter::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
  // do not spam console with mouseover events
  if (event<51 || event>55)
    {
      printf("calling THistPainter::ExecuteEvent( %d, %d, %d)\n", event, px,py);
      printf("histname=%s\n", this->fH->GetName());
    }

  auto key=std::make_pair(std::string(this->fH->GetName()), event);
  if (onClickMap.count(key) )
    {
      int mod=floor(gPad->AbsPixeltoX(px));
      int ch= floor(gPad->AbsPixeltoY(py));
      
      char item[NAME_LEN];
      onClickMap[key](item, mod, ch);
      bool ret=TGo4Script::ScriptInstance()->DrawItem(item);
      printf("DrawItem(%s) returned %d\n", item, ret);
      return;
    }
  void* ee_void=(dlsym(RTLD_NEXT, "_ZN12THistPainter12ExecuteEventEiii"));
  ee_t* ee_real=reinterpret_cast<ee_t*>(&ee_void);
  if (*ee_real)
    (**ee_real)(this, event, px, py);
  else
    printf("could not find original THistPainter::ExecuteEvent method. name wrangling wrong?\n");
}


