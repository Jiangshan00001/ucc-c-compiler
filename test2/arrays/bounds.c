// RUN: %check %s

struct A
{
	int a[3];
};

main()
{
	int x[2];

	return
		((struct A *)0)->a[3] +             // CHECK: /index 3 out of bounds.*3/

		// fine
		x[1] +

		// oob
		x[5] +                              // CHECK: /index 5 out of bounds.*2/
		*(x + 3) +                          // CHECK: /index 3 out of bounds.*2/

		// negative bounds
		x[-2] +                             // CHECK: /index -2 out of bounds.*2/
		*(x - 1) +                          // CHECK: /index -2 out of bounds.*2/

		// reverse oob
		*(2 + x) +                          // CHECK: /index 2 out of bounds.*2/
		3[x] +                              // CHECK: /index 3 out of bounds.*2/

		// reverse negative oob
		*(-2 + x) +                         // CHECK: /index -2 out of bounds.*2/
		-5[x];                              // CHECK: /index -5 out of bounds.*2/
}
