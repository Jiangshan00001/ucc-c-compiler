// RUN: %ocheck 3 %s

main()
{
	/* empty attribute is important - should still parse as decl */
	__attribute(()) x = 3;
	return x;
}
