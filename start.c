#include <stdio.h>
#include "setup.h"
#include "start.h"
#include "fptime.h"
#include "list.h"

void
real_fperffs(void *pc)
{
	void              *fstart  = pc;
	Dl_info            info    = { NULL, NULL, NULL, NULL };
	funcStats_RT      *rts     = NULL;
	double             st      = __fperf_getFPTimeStamp();
	funcStats_CG      *CGL_cur = NULL;

	if(fperfIsSetup == 0) return;

	/* this first section looks up the function name based upon the
	 * pc and demangles it if possible
	 */
	
	DEBUG(("function called with pc=%x\n", pc));
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
	
	/* this next section will look up the functions RT stats
	 * record and insert a current call start time (ccs). when
	 * fperffe() is called, it will use the ccs value to determine
	 * the total function run time for this particular execution
	 * cycle. it will then adjust the running average. note that
	 * this is general stats for all threads - we record thread
	 * specific stats in the call graph section.
	 */

	rts = lookupRTS(fstart);
	if(rts) {
		rts->ccs = st;
		rts->calls++;
	} else {
		rts = (funcStats_RT *)malloc(sizeof(funcStats_RT));
		if(!rts) {
			perror("failed to malloc new funcStats_RT");
			return;
		}
		rts->thrId = pthread_self();
		rts->secsSum = 0.0;
		rts->secs  = 0.0;
		rts->ccs   = st;
		rts->calls = 1;
		rts->next  = NULL;
		rts->addr  = fstart;

		insertRTS(rts);
	}

	/* this section records the 'call graph'. we do this by first
	 * determining "where we are" in terms of the graph we are 
	 * constructing. 
	 *
	 * since fperffs() is called at the beginning of each function,
	 * we need to figure out what node in the graph corresponds to
	 * out previous function (the one that called out current function)
	 * and then chain a call graph node off of the call graph node for
	 * out previous function.
	 */

	/* call graph recording: since we have two thread specific
	 * variables: the CG (call graph) and curCG (a pointer into
	 * the call graph to track where we currently are) we need
	 * to do the following:
	 *
	 * if CG == NULL then 
	 *   this is a new thread.
	 *   t = new CG
	 *   setCG(t)
	 *   setcurCG(t)
	 * else
	 *   this is an existing thread.
	 * 
	 *   
	 */

	CGL_cur = getcurCG();

	if(CGL_cur == NULL) {
		/* new thread. */
		funcStats_CG *new = 
			(funcStats_CG *)malloc(sizeof(funcStats_CG));
		if(new == NULL) {
			perror("failed to malloc new funcStats_CG");
			return;
		}
		memset(new, 0, sizeof(funcStats_CG));

		new->rts.thrId = pthread_self();
		new->rts.calls++;
		new->rts.ccs   = st;
		new->rts.addr  = fstart;
		
		setCG(new);
		setcurCG(new);

	} else {
 
		/* if the current CGL is us then this function
		 * has been called recursively. we increment recurCount.
		 * fperffe() will decrement recurCount and when it hits
		 * zero, the stats will be calculated and stored for
		 * the aggregate amount of time the func took to run.
		 */
		
		if(CGL_cur->rts.addr == fstart) {
			DEBUG(("fperffs: recursion detected.\n"));
			CGL_cur->rts.recurCount++;
			CGL_cur->rts.calls++;
		}

		/* otherwise: we've arrived in a new function. 
		 * this is a child of our previous function (CGL_cur)
		 * so we'll add it as a child (unless it is already there).
		 */
	
		else {
			funcStats_CG *fs, *new;

			/* check to see if this function is already listed as
			 * one of our children
			 */

			for(new = CGL_cur->child ; 
			    new && new->rts.addr != fstart ; 
			    new = new->next);

			if(new == NULL) {
				new = (funcStats_CG *) malloc(sizeof(funcStats_CG));
				if(new != NULL)
					memset(new, 0, sizeof(funcStats_CG));
			}
			
			if(new == NULL) {
				perror("failed to malloc space for funcStats_CG");
				return;
			}

			/* now, start stats for this function */

			new->rts.thrId = pthread_self();
			new->rts.calls++;
			new->rts.ccs  = st;
			new->rts.addr = fstart;
			
			/* point back to our parent */
			
			if(new->parent == NULL)
				new->parent = CGL_cur;
			
			/* point parent to us only if we are the first child */
			
			if(CGL_cur->child == NULL) {
				DEBUG(("we are the first child.\n"));
				CGL_cur->child = new;
			}
			
			/* stick us onto our parent's children list (unless we
			 * are already there)
			 */
			
			for(fs = CGL_cur->child ; 
			    fs && (fs->rts.addr != fstart) && fs->next ;
			    fs = fs->next);
			
			if(fs->rts.addr != fstart) 
				fs->next = new;
			
			/* finally, update CGL_cur to point to us */
			
			setcurCG(new);
		}
	}

}

/* ROUTINE
 *   fperffs()
 *
 * DESCRIPTION
 *   this routine takes the current PC and records it.
 *   it then attaches a 'start time' to the PC record.
 *   eventually, we hope that fperffe() will be called when
 *   the function exits and the PC record will be updated with
 *   a total run time. 
 *
 *   finally, we can analyze the PC, cross reference it to a symbol
 *   name (function name) and produce a listing of function name and
 *   time spent in the function.
 *
 *   call graph generation:
 *  
 *   in order to generate a call graph, we need to figure out where
 *   we currently are (this will change based on thread id for MT
 *   programs) and then keep a list showing the path that we are 
 *   taking thru each function call. as fperffs() is called, we'll
 *   add a node to the list. when fperffe() is called, it should 
 *   pop back one node and then record some stats. 
 */

void
fperffs()
{
	/* get the return address so we can figure out what the function
	 * that just called us is
	 */
	void *pc = (void *)0x4242;

	DEBUG(("fperffs() called\n"));
#ifdef i386
	__asm__ ("popl %eax ; movl 0x04(%esp),%eax ; pushl %eax");
#else
	sorry youre on your own;
#endif
	
	real_fperffs(pc);
}

