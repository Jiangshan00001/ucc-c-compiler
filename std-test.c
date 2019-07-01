int printf(const char *, ...);

struct A { int i; };

void g(struct A *);

void f()
{
	g(&(struct A){ 3 });
}

main()
{
	f();
}

void g(struct A *p)
{
	printf("{ %d } // %p\n", p->i, p);
}
