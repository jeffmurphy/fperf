#ifndef __list_h__ 
#define __list_h__

typedef struct _funcTableEntry funcTableEntry;
struct _funcTableEntry { 
	char           *name;
	void           *addr;
	funcTableEntry *next;
};

/* 'runtime' stats - tell us how many times a function was
 * called and how long, on avg, did it take to run.
 */

typedef struct _funcStats_RT funcStats_RT;
struct _funcStats_RT {
	void           *addr;
	int             recurCount;
	double          secs;    /* previous call run time     */
	double          ccs;     /* current call start time    */
	double          secsSum; /* tot run time for all calls */
	unsigned int    calls;   /* number of calls            */
	funcStats_RT   *next;
};

void          fperffs();
funcStats_RT *lookupRTS(void *pc);


/* 'call graph' stats. we track the path of each call sequence
 * and record the path and associated run time stats at each
 * path segment. each thread has a separate call graph, so we
 * use a thr specific key to hold the call graph. now this means
 * that we either have to dump the CG for each thread as each thread
 * exits, or we need to keep track of these in a master list so the
 * primary thread can dump all CGs when the process exits. we'll be
 * doing the latter. 
 */

typedef struct _funcStats_CG  funcStats_CG;

struct _funcStats_CG {
	funcStats_RT  rts;
	funcStats_CG *next;
	funcStats_CG *prev;
	funcStats_CG *child;
	funcStats_CG *parent;
};

funcStats_CG *getcurCG();
void          setcurCG(funcStats_CG *p);
funcStats_CG *getCG();
void          setCG(funcStats_CG *p);

typedef struct _funcStats_CGL funcStats_CGL;

struct _funcStats_CGL {
	funcStats_CG  *CG;
	funcStats_CGL *next;
};

#include <pthread.h>

#ifndef __LIST_C__
extern funcStats_RT       *RTFS;
extern funcTableEntry     *funcTable;
extern funcStats_CGL      *CGL;
extern pthread_key_t       CGL_key;
extern pthread_key_t       CGL_cur_key;
#endif

#endif /* __list_h__ */
