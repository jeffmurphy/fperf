#include <stdio.h>

int foo1() { printf("foo1\n"); sleep(1); foo2(); return 0;}
int foo2() { printf("foo2\n"); sleep(2); return 0;}

int
main(int ac, char **av)
{
	int i;
	printf("\tt1 arg count = %d\n", ac);
	for(i = 0 ; i < ac ; i++)
		printf("\t   arg[%d] = \"%s\"\n", i, av[i]);

	printf("main\n");
	foo1();
	foo1();
	exit(0);
}


