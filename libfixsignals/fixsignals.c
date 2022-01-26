//we overwrite the sigaction syscall so ROOT does not f^W mess
// with important system handlers
//cf http://stackoverflow.com/questions/69859/how-could-i-intercept-linux-sys-calls

// If a symbol gSystem is defined, we will block attempts to overwrite
// all signals with default behaviour "Core" as per man 7 signal

#define _GNU_SOURCE
#include <dlfcn.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#if 0
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact)
{
  if (dlsym(RTLD_DEFAULT, "gSystem"))
    {
      DEBUG("libfixsignal: ROOT detected. ");
      if (signum == SIGQUIT
          || signum == SIGILL
          || signum == SIGABRT
          || signum == SIGFPE
          || signum == SIGSEGV)
      
      //if (signum != SIGUSR1 && signum != SIGUSR2)
        {
          DEBUG("BLOCKED sigaction for signal %d\n", signum);
          return 0; // nice try, ROOT
        }
      else
        {
          DEBUG("allowed sigaction for signal %d\n", signum);
        }
    }
  // no root libraries loaded, or not a core dump signal, so handle
  // it normally
  
  int (* _sigaction)(int, const struct sigaction *,
                     struct sigaction *)=dlsym(RTLD_NEXT, "sigaction");
  return _sigaction(signum, act, oldact);
}

// unrelated: this function is used at startup of analysis.
// if this lib is present, 
int wait_for_gdb()
{
  while (1)
    sleep(1);
}
