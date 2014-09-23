#include "CalifaSubprocessor.h"
#include "CalifaProc.h"

CalifaSubprocessor::CalifaSubprocessor()
{
  this->registerSP();
}

void CalifaSubprocessor::registerSP()
{
  CalifaProc::GetProc()->RegisterSubprocessor(this);
}
