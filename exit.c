#include <stdio.h>
#include <unistd.h>
#include "setup.h"
#include "start.h"
#include "list.h"
#include "lock.h"
#include "fptime.h"

static void printChildren(funcStats_CG *c, FILE *fp);

static char *
lookupFuncName(const void *addr)
{
	funcTableEntry *f;
	for(f = funcTable ; f && (f->addr != addr); f = f->next);
	if(f) return f->name;
	return NULL;
}

void 
dumpSecCG(FILE *fp)
{
	funcStats_CGL *cgl;

	fprintf(fp, "SECTION-CG:\n");

	if(CGL == NULL) 
		printf("error: CGL == NULL\n");

	for(cgl = CGL ; cgl ; cgl = cgl->next) {
		funcStats_CG  *cg = cgl->CG;
		char          *n;

		if(cg->next != NULL)
			printf("warning: CGL[%x]->next != NULL (has peer)", 
			       cg->rts.addr);
		
		n = lookupFuncName(cg->rts.addr);
		fprintf(fp, "ROOT\tf=%s\ta=%x\tts=%f\tcalls=%u\tavg=%f\n",
			SAFEPRT(n),
			cg->rts.addr,
			cg->rts.secsSum,
			cg->rts.calls,
			(double)(cg->rts.secsSum / cg->rts.calls));

		printChildren(cg->child, fp);

	}
}


void
fperfexit()
{
	funcTableEntry *f;
	funcStats_RT   *rt;
	funcStats_CG   *cg, *cg2;
	FILE           *fp;
	pid_t           pid = getpid();
	char            fn[256];
	int             i;
	double          now = __fperf_getFPTimeStamp();
	char           *funcname;

	if(fperfIsSetup == 0) return;

	DEBUG(("fperf exit function.\n"));

	/* calculate "main" stats */

	RTFS->secs     = now - RTFS->ccs;
	RTFS->secsSum += RTFS->secs;
	RTFS->ccs      = 0.0;

#if 0
	CGL->rts.secs      = now - CGL->rts.ccs;
	CGL->rts.secsSum   = CGL->rts.secs;
	CGL->rts.ccs       = 0.0;
#endif

	sprintf(fn, "fperf.dat.%u", pid);
	sprintf(fn, "fperf.dat", pid);
	fp = fopen(fn, "w");
	if(fp == NULL) {
		perror("failed to open fperf.dat for writing");
		return;
	}

	/* dump the "function table" section.
	 * this section is simply a maping of function start
	 * address to function name. it may or may not be
	 * useful (since we reproduce this in later sections
	 * anyway) but is included for convenience.
	 */

	fprintf(fp, "SECTION-FT:\n");
	for(f = funcTable ; f ; f = f->next)
		fprintf(fp, "0x%x=%s\n", f->addr, 
			f->name ? f->name : "unknown");
	
	/* dump the "run time" section.
	 * this section includes stats on number of calls,
	 * total amount of time spent (for all calls) in a function 
	 * and average amount of time (per call) spent.
	 * note that these times include the time spent in child
	 * functions. if you want to break out the child time and
	 * get only the amount of time really spent in a particular
	 * function you need to examine the call graph section and
	 * do some calculations.
	 */

	fprintf(fp, "SECTION-RT:\n");

	for(rt = RTFS ; rt ; rt = rt->next) {
		funcname = lookupFuncName(rt->addr);
		fprintf(fp, "f=%s\ta=%x\t", SAFEPRT(funcname), rt->addr);
		fprintf(fp, "ts=%f\tcalls=%u\tavg=%f\n", 
			rt->secsSum,
			rt->calls,
			(double)(rt->secsSum / rt->calls));
#if 0
		printf("\n\taddr=%x\n\tsecs=%f\n\tccs=%f\n\tsecsSum=%f\n",
		       rt->addr, rt->secs, rt->ccs, rt->secsSum);
		printf("\tcalls=%u\n", rt->calls);
#endif
	}

	/* dump the 'call graph' section.
	 * this section helps us see how function calls flow
	 * thru the execution of an application. we can graphically
	 * construct a call graph and even calculate the actual
	 * time spent in a function (versus total) by subtracting
	 * out the time spent in child functions
	 */


	dumpSecCG(fp);

	fclose(fp);

	printf("FPERF statistics written to %s\n", fn);

	i = pthread_key_delete(CGL_key);
	if(i != 0) 
		perror("pthread_key_delete");

	i = pthread_key_delete(CGL_cur_key);
	if(i != 0) 
		perror("pthread_key_delete");

}

static void
printChildren(funcStats_CG *c, FILE *fp)
{
	funcStats_CG *t;
	char         *funcname;

	for(t = c ; t ; t = t->next) {
		funcname = lookupFuncName(t->parent->rts.addr);
		fprintf(fp, "CHILDOF\tpf=%s\tpa=%x\t",
			SAFEPRT(funcname), t->parent->rts.addr);

		funcname = lookupFuncName(t->rts.addr);
		fprintf(fp, "f=%s\ta=%x\tts=%f\tcalls=%u\tavg=%f\n",
			SAFEPRT(funcname), 
			t->rts.addr,
			t->rts.secsSum, 
			t->rts.calls,
			(double)(t->rts.secsSum / t->rts.calls));

		/* if we have any children, print those out recursively
		 * so we can get this overwith as simply as possible 
		 */

		if(t->child)
			printChildren(t->child, fp);
	}
}
