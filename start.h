

#ifndef __start_h__
#define __start_h__

#include <sys/types.h>
#include <dlfcn.h>

#ifdef NDEBUG
# define DEBUG(X) printf X 
# define DEBUG2(X) ((void)0)
#else
# define DEBUG(X) ((void)0)
# define DEBUG2(X) ((void)0)
#endif

#define SAFEPRT(X) X?X:"NULL"

#ifdef DEMANGLE
# ifdef __GNUC__
  /* this stuff comes from GNU's demangle.h which doesnt appear to be
   * available by default on a system.
   */

  /* Options passed to cplus_demangle (in 2nd parameter). */

#  define DMGL_NO_OPTS    0               /* For readability... */
#  define DMGL_PARAMS     (1 << 0)        /* Include function args */
#  define DMGL_ANSI       (1 << 1)        /* Include const, volatile, etc */
#  define DMGL_JAVA       (1 << 2)        /* Demangle as Java */

#  define DMGL_AUTO       (1 << 8)
#  define DMGL_GNU        (1 << 9)
#  define DMGL_LUCID      (1 << 10)
#  define DMGL_ARM        (1 << 11)

#  define DMGL_DEFAULT (DMGL_PARAMS | DMGL_ANSI)

extern char *cplus_demangle(const char *sym, int flags);
# endif
#endif

	
#endif /* __start_h__ */
