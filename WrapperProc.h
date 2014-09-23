//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#ifndef WRAPPERPROC_H
#define WRAPPERPROC_H
//see comments in CalifaProc.h


#include "CalifaConfig.h"
#include "CalifaBaseProc.h"
#include "TGo4EventProcessor.h"
class CalifaParam;
class TGo4Fitter;
class TClass;
class WrapperProc : public TGo4EventProcessor {
   public:
      WrapperProc(const char* name);
      virtual ~WrapperProc() ;

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function
 protected:
      void* proc;
      ClassDef(WrapperProc,1)
};


#endif //TUNPACKPROCESSOR_H


//----------------------------END OF GO4 SOURCE FILE ---------------------
