#include <stdio.h>
#if LINUX > 0
# define __USE_XOPEN
#endif
#include <sys/time.h>
#include <unistd.h>

double
__fperf_getFPTimeStamp(void)
{
  struct timeval tv;
  int n;

  if ((n = gettimeofday(&tv, 0)) != 0)
    return -1;

  return((double) (tv.tv_sec + ((double)tv.tv_usec / (double)1000000.0)));
}
