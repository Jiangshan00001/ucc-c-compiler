struct A
{
	unsigned a:4;
	unsigned d:18;
};

main()
{
	struct A *f();
	f()->d |= 5;
}
