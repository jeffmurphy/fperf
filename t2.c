/* simple test2: threaded */

#include <stdio.h>
#include <pthread.h>

void *thrmain1(void *p) { foo1(); foo1(); return NULL; }

int foo1() { printf("foo1\n"); sleep(1); foo2(); return 0;}
int foo2() { printf("foo2\n"); sleep(2); return 0;}

int
main(int ac, char **av)
{
	int       i;
	pthread_t t;

	printf("[%d] main\n", pthread_self());

	i = pthread_create(&t, NULL, thrmain1, NULL);
	if(i != 0) {
		perror("pthread_create");
		exit(-1);
	}

	printf("thread created id=%u\n", t);

	foo1();
	foo1();
	foo2();

	i = pthread_join(t, NULL);
	if(i != 0) {
		perror("pthread_join");
		exit(-1);
	}
	exit(0);
}


