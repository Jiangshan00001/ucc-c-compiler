ATTR __thread int i = 3;

int *f(void)
{
	return &i;
}
