#pragma once

#include "CalifaParser.h"
#include <array>
#include <math.h> //NAN


class CalifaSumParser: public CalifaParser
{
public:
 CalifaSumParser(): CalifaParser()
    {
    }
  virtual int parseGo4(TGo4MbsEvent *);
 
protected:
};
