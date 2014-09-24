#include "debug.h"
#include <stdio.h>
#include <string>
#include <stdint.h>
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

void dumpMem(void* p, size_t len)
{
  uint8_t* ip=(uint8_t*)p;
  fprintf(stderr, "contents of memory at %p (%d bytes)", ip, len);
  for (size_t i=0; i<len; i++)
    {
      if (!(i%16))
	fprintf(stderr, "\n %p: ", &(ip[i]));
      else if (!(i%2))
	fprintf(stderr, " ");
      fprintf(stderr, "%02x", (unsigned int)ip[i]);
    }
  fprintf(stderr, "\n");
}
#ifdef DEBUGDEBUG
int main(int argc, char**argv)
{
  dumpMem(argv, 20);
  return 0;
}
#endif

