/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "stdio.h"
#include "paths.h"

#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)

extern  char *
mktemp(char  *path);

int 
mkstemp(char *path);

#include "sys/types.h"
#include "signal.h"
#include "unistd.h"
#include "string.h"
#include "errno.h"

FILE *
tmpfile()
{
#if LW_CFG_SIGNAL_EN > 0
	sigset_t set, oset;
#endif
	
	FILE *fp;
	int fd, sverrno;
#define	TRAILER	"tmp.XXXXXXXXXX"
	char buf[sizeof(_PATH_TMP) + sizeof(TRAILER)];

	(void)memcpy(buf, _PATH_TMP, sizeof(_PATH_TMP) - 1);
	(void)memcpy(buf + sizeof(_PATH_TMP) - 1, TRAILER, sizeof(TRAILER));

#if LW_CFG_SIGNAL_EN > 0
	sigfillset(&set);
	(void)sigprocmask(SIG_BLOCK, &set, &oset);
#endif

	fd = mkstemp(buf);
	if (fd != -1)
		(void)unlink(buf);

#if LW_CFG_SIGNAL_EN > 0
	(void)sigprocmask(SIG_SETMASK, &oset, NULL);
#endif

	if (fd == -1)
		return (NULL);

	if ((fp = fdopen(fd, "w+")) == NULL) {
		sverrno = errno;
		(void)close(fd);
		errno = sverrno;
		return (NULL);
	}
	return (fp);
}

#endif  /*  (LW_CFG_DEVICE_EN > 0)      */
        /*  (LW_CFG_FIO_LIB_EN > 0)     */
