#ifndef EnergyCal_h
#define EnergyCal_h
#include "TF1.h"
#include "CalifaParser.h"
#include <map>
class EnergyCal
{
 public:
  typedef std::map<CalifaParser::module_index_t, const TF1*> calmap_t;
  static calmap_t *cal;
  static const TF1* getCal(CalifaParser::module_index_t);
};
#endif
