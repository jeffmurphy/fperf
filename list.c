#include <stdio.h>
#include "start.h"
#include "fptime.h"
#include "lock.h"

#define __LIST_C__
#include "list.h"

funcStats_RT       *RTFS      = NULL;

funcStats_RT *
lookupRTS(void *pc) 
{
        funcStats_RT *f;
	for(f = RTFS ; f && (f->addr != pc) ; f = f->next);
	return f;
}

void
insertRTS(funcStats_RT *r)
{
	funcStats_RT *f;
	r->next = NULL; /* just in case */
	for(f = RTFS ; f && f->next ; f = f->next);
	if(f)
		f->next = r;
	else
		RTFS = r;
}


funcTableEntry     *funcTable = NULL;

void
add2FTable(void *pc, const char *fname)
{
	int             len = 0;
	funcTableEntry *f, *n;
	
	if(!fname || !*fname) {
		printf("add2FTable: fname == NULL\n");
		return;
	}
	
	for(f = funcTable ; f && (f->addr != pc) && f->next ; f = f->next);
	if(f && (f->addr == pc)) return; /* already on the list */
	
	n = (funcTableEntry *)malloc(sizeof(funcTableEntry));
	if(!n) {
		perror("can't malloc new funcTableEntry");
		return;
	}
	
	n->addr = pc;
	len     = strlen(fname);
	n->name = (char *)malloc(len+1);
	*(n->name + len) = 0;
	if(! n->name) {
		perror("can't malloc space for funcTableEntry->fname");
		free(n);
		return;
	}
	strncpy(n->name, fname, len);
	n->next = 0;
	
	if(f)
		f->next = n;
	else 
		funcTable = n;
}

funcStats_CGL      *CGL;
pthread_key_t       CGL_key;
pthread_key_t       CGL_cur_key;

funcStats_CG *
getCG()
{
	void *p = pthread_getspecific(CGL_key);
	return (funcStats_CG *)p;
}

void
setCG(funcStats_CG *p)
{
	funcStats_CGL *c;
	int            i;

	/* first, make sure we track us on the master list */

	for(c = CGL ; 
	    c && (c->CG != p) && c->next ; 
	    c = c->next);

	if(c == NULL || (c->next == NULL && c->CG != p)) {
		funcStats_CGL *n = 
			(funcStats_CGL *)malloc(sizeof(funcStats_CGL));

		if(n == NULL) {
			perror("couldn't malloc space for funcStats_CGL");
			return;
		}
		n->CG   = p;
		n->next = NULL;
		if(CGL == NULL) {
			DEBUG(("CGL == NULL .. making CGL\n"));
			CGL = n;
		} else {
			DEBUG(("CGL != NULL .. adding to end of CGL\n"));
			c->next = n;
		}
	}

	i = pthread_setspecific(CGL_key, (void *)p);
	if(i != 0)
		perror("pthread_setspecific");
}

funcStats_CG *
getcurCG()
{
	void *p = pthread_getspecific(CGL_cur_key);
	return (funcStats_CG *)p;
}

void
setcurCG(funcStats_CG *p)
{
	int i = pthread_setspecific(CGL_cur_key, (void *)p);
	if(i != 0)
		perror("pthread_setspecific");
}

