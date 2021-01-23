// RUN: %ucc -c -o %t %s -w
// RUN: objdump -d %t || true
// RUN: %ocheck 0 %s -w

void exit(int) __attribute__((noreturn));

syntax(int n)
{
	int ar[n];
	int (*p)[n] = &ar;

	syntax(p);
}

assert(_Bool b, int lno)
{
	if(!b){
		printf("%s:%d: assert failed\n", __FILE__, lno);
		exit(1);
	}
}
#define assert(x) assert((x), __LINE__)

f(int n)
{
	short (*p)[n] = 0;

	printf("p = %#x\n", p);
	assert(p == 0);

	__auto_type a = (int)(p + 1);
	__auto_type b = 3 * sizeof(short);
	if(a != b){
		printf("a=%#x\nb=%#x\n", a, b);
		fflush(0);
		exit(5);
	}
}

main()
{
	f(3);

	return 0;
}
