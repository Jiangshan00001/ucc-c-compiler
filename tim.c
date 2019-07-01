#include <stdio.h>

int global;
static int global_static;

void f(int *p)
{
	printf("%d %d %d\n", global, global_static, *p);
}

int main()
{
	printf("sizeof(void*) = %d\n", sizeof(void*));
	f(&(int){3});
	return 3;
}
