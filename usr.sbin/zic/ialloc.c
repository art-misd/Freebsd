#ifndef lint
#ifndef NOID
static char	elsieid[] = "@(#)ialloc.c	8.28";
#endif /* !defined NOID */
#endif /* !defined lint */

#ifndef lint
static const char rcsid[] =
  "$FreeBSD: src/usr.sbin/zic/ialloc.c,v 1.3.2.1 1999/08/29 15:50:04 peter Exp $";
#endif /* not lint */

/*LINTLIBRARY*/

#include "private.h"

#define nonzero(n)	(((n) == 0) ? 1 : (n))

char *	icalloc P((int nelem, int elsize));
char *	icatalloc P((char * old, const char * new));
char *	icpyalloc P((const char * string));
char *	imalloc P((int n));
void *	irealloc P((void * pointer, int size));
void	ifree P((char * pointer));

char *
imalloc(n)
const int	n;
{
	return malloc((size_t) nonzero(n));
}

char *
icalloc(nelem, elsize)
int	nelem;
int	elsize;
{
	if (nelem == 0 || elsize == 0)
		nelem = elsize = 1;
	return calloc((size_t) nelem, (size_t) elsize);
}

void *
irealloc(pointer, size)
void * const	pointer;
const int	size;
{
	if (pointer == NULL)
		return imalloc(size);
	return realloc((void *) pointer, (size_t) nonzero(size));
}

char *
icatalloc(old, new)
char * const		old;
const char * const	new;
{
	register char *	result;
	register int	oldsize, newsize;

	newsize = (new == NULL) ? 0 : strlen(new);
	if (old == NULL)
		oldsize = 0;
	else if (newsize == 0)
		return old;
	else	oldsize = strlen(old);
	if ((result = irealloc(old, oldsize + newsize + 1)) != NULL)
		if (new != NULL)
			(void) strcpy(result + oldsize, new);
	return result;
}

char *
icpyalloc(string)
const char * const	string;
{
	return icatalloc((char *) NULL, string);
}

void
ifree(p)
char * const	p;
{
	if (p != NULL)
		(void) free(p);
}

void
icfree(p)
char * const	p;
{
	if (p != NULL)
		(void) free(p);
}
