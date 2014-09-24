#ifndef _DEBUG_H
#define _DEBUG_H
#include "CalifaConfig.h"
#include <string>
#include <stdarg.h>

int logmsg(int type,  std::string format, ...);


#define lfatal(...) logmsg(0, __VA_ARGS__)
#define lerror(...) logmsg(1, __VA_ARGS__) 
#define linfo(...)  logmsg(2, __VA_ARGS__)
#define ldbg(...)   logmsg(3, __VA_ARGS__)
//ain't C beautiful?

void dumpMem(void* p, size_t len);

#endif
