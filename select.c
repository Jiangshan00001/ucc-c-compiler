extern long __syscall(int, ...);

int select(int nfds, int readfds, int writefds, int errorfds)
{
	return __syscall(23, nfds, readfds, writefds, errorfds);
}
