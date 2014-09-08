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
#ifndef SPAR_H
#define SPAR_H

#define __ARRAYSIZE__ 100
#define __MEVX__ 5
#define __MEVY__ 10

#include "TGo4Parameter.h"

class CalifaParam : public TGo4Parameter {
   public:
      CalifaParam();
      CalifaParam(const char* name);
      virtual ~CalifaParam();
      virtual Int_t  PrintParameter(Text_t * n, Int_t);
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      // samples for all supported data types:
      // comments appear in the generic GUI editor
      // dont forget to change the UpdateFrom and PrintParameter
      // functions when changing members
      Bool_t   fill;  // control filling
      Float_t  frP1;  // control frP1
      Float_t  frP2;  // control frP2
      Int_t    fiNum;  // control fiNum
      Bool_t   fbEnab;  // control
      Double_t fdTest;  // control
      Short_t  fsTest;  // control
      Char_t   fcTest;  // control
      Long_t   flTest;  // control
      TString  fxInputfile;
      Int_t    fiDataArray[__ARRAYSIZE__];
      Double_t fdMeV[__MEVX__][__MEVY__];
      UInt_t   fuNum;
      Double_t fdTestarr[__MEVX__];
      UShort_t fuShort;
      UChar_t  fuChar;
      ULong_t  fuLong;

   ClassDef(CalifaParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
