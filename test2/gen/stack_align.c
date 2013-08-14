// RUN: %ucc -o %t %s && %t

#define NULL (void *)0

p(char *p
#ifdef WITH_ALIGN
		, char *align
#endif
		)
{
	printf("hello %s %s\n", p,
#ifdef WITH_ALIGN
			align
#else
			"n/a"
#endif
			);
}

main()
{
	p("abc"
#ifdef WITH_ALIGN
			, "def"
#endif
			);
}
