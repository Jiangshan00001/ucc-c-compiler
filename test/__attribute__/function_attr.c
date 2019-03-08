// RUN: %check --only %s

__attribute__((noreturn)) void
	d0 (void),
	__attribute__((format(printf, 1, 2))) d1 (const char *, ...),
	d2 (void);

// noreturn applies to all
// format applies to d1

_Noreturn void a()
{
	d0();
}

_Noreturn void b()
{
	d2();
}

_Noreturn void c()
{
	d1("hi %s"); // CHECK: warning: too few arguments for format (%s)
}
