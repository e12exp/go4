#include "debug.h"
#include <stdio.h>
#include <string>
#include <stdint.h>
#include <unistd.h>
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
  fprintf(stderr, "contents of memory at %p (%ld bytes)", ip, len);
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

//#define DBG_GDB

int __attribute__((weak)) wait_for_gdb()
{
  //by default, do not wait
#ifdef DBG_GDB
  while (1)
    sleep(1);
#endif
  return 0;
}

int my_dummy_global=wait_for_gdb();


#ifdef DEBUGDEBUG
int main(int argc, char**argv)
{
  dumpMem(argv, 20);
  return 0;
}
#endif

