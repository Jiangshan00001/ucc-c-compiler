// RUN: %ocheck 3 %s

f()
{
	return (int [][2]){
		{ 1, 2 },
		{ 3, (int[]){3}[0] }
	}[1][1]; // 3
}

main()
{
	int *p;

	p = (int[]){2, f()};

	return p[1];
}
