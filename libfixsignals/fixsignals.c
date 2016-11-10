//#include <signal.h>
//we overwrite the sigaction syscall so ROOT does not f^W mess
// with important system handlers
//cf http://stackoverflow.com/questions/69859/how-could-i-intercept-linux-sys-calls

//as root cleverly uses sigusr, we have to allow that signal to be overwritten. 


#define _GNU_SOURCE
#include <dlfcn.h>
//#define _FCNTL_H
//#include <bits/fcntl.h>
#define _SIGNAL_H
typedef int __pid_t;
typedef unsigned int __uid_t;
typedef long int __clock_t;
#include <bits/sigset.h>
#include <bits/siginfo.h>
#include <asm-generic/signal-defs.h>
#include <bits/sigaction.h>
#include <bits/signum.h>

int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact)
{
  if (signum != SIGUSR1 && signum != SIGUSR2)
    return 0;

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
