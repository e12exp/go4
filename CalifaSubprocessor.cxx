#include "CalifaSubprocessor.h"
#include "CalifaProc.h"
#include "debug.h"
CalifaSubprocessor::CalifaSubprocessor()
{
  this->registerSP();
}

void CalifaSubprocessor::registerSP()
{
  CalifaProc::GetProc()->RegisterSubprocessor(this);
}

void CalifaSubprocessor::registerObject(TObject* obj)
{
  CalifaProc::GetProc()->registerObject(obj);
}
