#ifndef CalifaBaseProc_h
#define CalifaBaseProc_h
#include "TGo4EventElement.h"
#include "TGo4EventProcessor.h"
//see comments in CalifaProc.h

Bool_t wrapperBuildEvent(void*, TGo4EventElement* target); // event processing funct
void* createCalifaProc(const char* name, TGo4EventProcessor* w);

#endif
