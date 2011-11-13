#include "unistd.h"
#include "stdio.h"
#include "string.h"

static int _stdin  = 0;
static int _stdout = 1;
static int _stderr = 2;

FILE *stdin  = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

static const char *nums = "0123456789abcdef";

/* Private */
static void printd_rec(int fd, int n, int base)
{
	int d;
	d = n / base;
	if(d)
		printd_rec(fd, d, base);
	write(fd, nums + n % base, 1);
}

static void printn(int fd, int n, int base)
{
	if(n < 0){
		write(fd, "-", 1);
		n = -n;
	}

	printd_rec(fd, n, base);
}

static void printd(int fd, int n)
{
	printn(fd, n, 10);
}

static void printx(int fd, int n)
{
	write(fd, "0x", 2);
	printn(fd, n, 16);
}


/* Public */

int vfprintf(FILE *file, const char *fmt, va_list ap)
{
	int fd = *file;

	while(*fmt){
		if(*fmt == '%'){
			fmt++;

			if(*fmt == 's')
				write(fd, *(char **)ap, strlen(*(char **)ap));
			else if(*fmt == 'c')
				write(fd, (char *)ap, 1);
			else if(*fmt == 'd')
				printd(fd, *(int *)ap);
			else if(*fmt == 'p' || *fmt == 'x')
				printx(fd, *(int *)ap);
			else
				write(fd, fmt, 1); /* default to just printing the char */

			ap++;
		}else{
			write(fd, fmt, 1);
		}
		fmt++;
	}
}

int dprintf(int fd, const char *fmt, ...)
{
	return vfprintf(&fd, fmt, (void *)(&fmt + 1));
}

int fprintf(FILE *file, const char *fmt, ...)
{
	return vfprintf(file, fmt, (void *)(&fmt + 1));
}

int printf(const char *fmt, ...)
{
	return vfprintf(stdout, fmt, (void *)(&fmt + 1));
}

int puts(const char *s)
{
	return printf("%s\n", s) > 0 ? 1 : -1;
}
