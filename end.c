#include <stdio.h>
#include "setup.h"
#include "start.h"
#include "fptime.h"
#include "list.h"

void
real_fperffe(void *pc) 
{
	void         *fstart = pc;
	Dl_info       info   = { NULL, NULL, NULL, NULL };
	funcStats_RT *rts    = NULL;
	double        st     = __fperf_getFPTimeStamp();
	funcStats_CG *CG_cur = NULL;

	if(fperfIsSetup == 0) return;

	/* this first section looks up the function name based upon the
	 * pc and demangles it if possible
	 */

	DEBUG2(("returning from function pc=%x\n", pc));
	DEBUG2(("trying to determine function name..\n"));
	if(dladdr(pc, &info) != 0) {
		DEBUG2(("\tdli_fname = %s\n", SAFEPRT(info.dli_fname)));
		DEBUG2(("\tdli_sname = %s\n", SAFEPRT(info.dli_sname)));
#if defined(DEMANGLE) && defined(__GNUC__)
		{
			char *dem = cplus_demangle(info.dli_sname, 
						   DMGL_DEFAULT);
			DEBUG2(("\tdemangled = %s\n", SAFEPRT(dem)));
			if(dem) 
				add2FTable(info.dli_saddr, dem);
			else
				add2FTable(info.dli_saddr, info.dli_sname);
		}
#else
		add2FTable(info.dli_saddr, info.dli_sname);
#endif
		DEBUG2(("\tdli_saddr = %x\n", info.dli_saddr));
		fstart = info.dli_saddr;
	} else {
		perror("dladdr");
		return;
	}

	/* this next section will look up the function's RT stats
	 * record and figure out the run time for this function [since
	 * fperffs() was called]. 
	 */

	rts = lookupRTS(fstart);
	if(rts) {
		if(rts->ccs == 0.0) {
			printf("error: rts->ccs == 0\n");
			return;
		}
		rts->secs     = st - rts->ccs;
		rts->ccs      = 0.0;
		rts->secsSum += rts->secs;
	} else {
		printf("failed to lookupRTS for %s\n",
		       info.dli_sname);
	}

	/* finally, we want to back off CG_cur to point to our
	 * parent. but first we'll clean up the stats.
	 */

	CG_cur = getcurCG();

	if(CG_cur->rts.ccs == 0.0)
		printf("error: CG_cur->ccs == 0\n");
	else {
		CG_cur->rts.secs     = st - CG_cur->rts.ccs;
		CG_cur->rts.ccs      = 0.0;
		CG_cur->rts.secsSum += CG_cur->rts.secs;
	}

	if(CG_cur->parent == NULL) {
		printf("error: CG_cur->parent == NULL\n");
		return;
	}

	setcurCG(CG_cur->parent);
}

void
fperffe()
{
	/* get the return address so we can figure out what the function
	 * that just called us is
	 */
	void *pc = (void *)0x4242;

	DEBUG(("fperffe() called\n"));
#ifdef i386
	__asm__ ("popl %eax ; movl 0x04(%esp),%eax ; pushl %eax");
#else
	sorry youre on your own;
#endif
	
	real_fperffe(pc);
}

