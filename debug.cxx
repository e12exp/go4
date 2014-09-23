#include "debug.h"
int logmsg(int type,  std::string format, ...)
{

  va_list args2;

  if (type<=LOGLEVEL )
    {
      va_start(args2, format);
      vfprintf(stderr, format.c_str(), args2);
      va_end(args2);
    }
  return 0;
}

