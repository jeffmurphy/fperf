#include <stdio.h>
#include "lock.h"
#include "list.h"
#include "version.h"
#include "start.h"
#include "fptime.h"

int fperfIsSetup = 0;

void
fperfsetup_real(void *mainp, int ac, char **av) 
{
  int           i      = 0;
  funcStats_CG *mainCG = (funcStats_CG *)malloc(sizeof(funcStats_CG));
  funcStats_RT *mainRT = (funcStats_RT *)malloc(sizeof(funcStats_RT));

  printf("This executable was compiled with:\n");
  printf("FPERF Version %2.2f %s\n%s\n", VERSION, ALPHABETA, COPYRIGHT);

#ifndef __sparc__
  printf("\targ count = %d\n", ac);
  for(i = 0 ; i < ac ; i++)
    printf("\t   arg[%d] = \"%s\"\n", i, av[i]);
#endif

  printf("initializing threads ..\n");
  i = pthread_key_create(&CGL_key, NULL);
  if(i != 0) {
	  perror("pthread_key_create");
	  exit(-1);
  }

  i = pthread_key_create(&CGL_cur_key, NULL);
  if (i != 0) {
	  perror("pthread_key_create");
	  exit(-1);
  }

  if(mainCG == NULL) {
	  perror("can't malloc funcStats_CG for main");
	  exit(-1);
  }

  if(mainRT == NULL) {
	  perror("can't malloc funcStats_RT for main");
	  exit(-1);
  }

  printf("initializing stats ..\n");

  memset(mainCG, 0, sizeof(funcStats_CG));
  memset(mainRT, 0, sizeof(funcStats_RT));

  mainCG->rts.addr  = mainRT->addr  = mainp;
  mainCG->rts.ccs   = mainRT->ccs   = __fperf_getFPTimeStamp();
  mainCG->rts.calls = mainRT->calls = 1;
  mainCG->rts.thrId = pthread_self();

  RTFS = mainRT;
  setCG(mainCG);
  setcurCG(mainCG);

  add2FTable(mainp, "main");

  printf("FPERF setup complete.\n\n");

  fperfIsSetup = 1;
}

void
fperfsetup(int ac, char **av)
{
	/* get the return address so we can figure out what the function
	 * that just called us is. in this case, we know it's main.. but
	 * we still need its start address.
         */

        void *pc = (void *)0x4242;

        DEBUG(("fperfsetup() called\n"));
#ifdef i386
        __asm__ ("popl %eax ; movl 0x04(%esp),%eax ; pushl %eax");
#else
        sorry youre on your own;
#endif
        fperfsetup_real(pc, ac, av);
}
