// RUN: %ucc -c %s

typedef struct A A;

main()
{
	extern A x[];

	//f(x[1]);
}
