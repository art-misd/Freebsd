/*
 * Copyright (c) 1995 Peter Wemm <peter@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Absolutely no warranty of function or purpose is made by the author
 *    Peter Wemm.
 *
 * $FreeBSD: src/lib/libutil/setproctitle.c,v 1.8.2.2 1999/11/23 00:18:54 brian Exp $
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/exec.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Older FreeBSD 2.0, 2.1 and 2.2 had different ps_strings structures and
 * in different locations.
 * 1: old_ps_strings at the very top of the stack.
 * 2: old_ps_strings at SPARE_USRSPACE below the top of the stack.
 * 3: ps_strings at the very top of the stack.
 * This attempts to support a kernel built in the #2 and #3 era.
 */

struct old_ps_strings {
	char	*old_ps_argvstr;
	int	old_ps_nargvstr;
	char	*old_ps_envstr;
	int	old_ps_nenvstr;
};
#define	OLD_PS_STRINGS ((struct old_ps_strings *) \
	(USRSTACK - SPARE_USRSPACE - sizeof(struct old_ps_strings)))

#if defined(__STDC__)		/* from other parts of sendmail */
#include <stdarg.h>
#else
#include <varargs.h>
#endif


#define SPT_BUFSIZE 2048	/* from other parts of sendmail */
extern char * __progname;	/* is this defined in a .h anywhere? */

void
#if defined(__STDC__)
setproctitle(const char *fmt, ...)
#else
setproctitle(fmt, va_alist)
	const char *fmt;
	va_dcl
#endif
{
	static struct ps_strings *ps_strings;
	static char buf[SPT_BUFSIZE];
	static char obuf[SPT_BUFSIZE];
	static char **oargv;
	static int oargc = -1;
	static char *nargv[2] = { buf, NULL };
	char **nargvp;
	int nargc;
	va_list ap;
	size_t len;
	unsigned long ul_ps_strings;

#if defined(__STDC__)
	va_start(ap, fmt);
#else
	va_start(ap);
#endif

	if (fmt) {
		buf[sizeof(buf) - 1] = '\0';

		/* print program name heading for grep */
		(void) snprintf(buf, sizeof(buf), "%s: ", __progname);

		/*
		 * can't use return from sprintf, as that is the count of how
		 * much it wanted to write, not how much it actually did.
		 */

		len = strlen(buf);

		/* print the argument string */
		(void) vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);

		nargvp = nargv;
		nargc = 1;
	} else if (*obuf != '\0') {
  		/* Idea from NetBSD - reset the title on fmt == NULL */
		nargvp = oargv;
		nargc = oargc;
	} else
		/* Nothing to restore */
		return;

	va_end(ap);

	if (ps_strings == NULL) {
		len = sizeof(ul_ps_strings);
		if (sysctlbyname("kern.ps_strings", &ul_ps_strings, &len, NULL,
		    0) == -1)
			ul_ps_strings = PS_STRINGS;
		ps_strings = (struct ps_strings *)ul_ps_strings;
	}

	/* PS_STRINGS points to zeroed memory on a style #2 kernel */
	if (ps_strings->ps_argvstr) {
		/* style #3 */
		if (oargc == -1) {
			/* Record our original args */
			oargc = ps_strings->ps_nargvstr;
			oargv = ps_strings->ps_argvstr;
			for (nargc = len = 0; nargc < oargc; nargc++) {
				snprintf(obuf + len, sizeof(obuf) - len, "%s%s",
				    len ? " " : "", oargv[nargc]);
				if (len)
					len++;
				len += strlen(oargv[nargc]);
				if (len >= sizeof(obuf))
					break;
			}
		}
		ps_strings->ps_nargvstr = nargc;
		ps_strings->ps_argvstr = nargvp;
	} else {
		/* style #2 - we can only restore our first arg :-( */
		if (*obuf == '\0')
			strncpy(obuf, OLD_PS_STRINGS->old_ps_argvstr,
			    sizeof(obuf) - 1);
		OLD_PS_STRINGS->old_ps_nargvstr = 1;
		OLD_PS_STRINGS->old_ps_argvstr = nargvp[0];
	}
}
