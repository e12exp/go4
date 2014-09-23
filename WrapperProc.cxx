#include "WrapperProc.h"
#include "CalifaProc.h"

#include "TGo4EventProcessor.h"
WrapperProc::WrapperProc(const char* name)
{
  this->proc=createCalifaProc(name, this);
}
WrapperProc::~WrapperProc()
{
  //delete proc;
}

Bool_t WrapperProc::BuildEvent(TGo4EventElement* target)
{
  return wrapperBuildEvent(this->proc, target);
}
