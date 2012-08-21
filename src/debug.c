
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if DEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "err.h"
#include "debug.h"

static char *debug_path = "/tmp/epson-escpr-wrapper-dump";
static FILE *debug_fp = NULL;

void
debug_init (void)
{
	char path[255];
	pid_t pid;
	
	pid = getpid ();
	sprintf (path, "%s.%d", debug_path, pid);
	
	if (!debug_fp)
	{
		debug_fp = fopen (path, "w");
		if (!debug_fp)
			err_msg (MSGTYPE_WARNING, "Can't open the dump file \"%s\".", path);
	}

	fchmod (fileno (debug_fp), 0644);
	return;
}


void
debug_dump (const char *fmt, ...)
{
	va_list ap;

	if (!debug_fp)
		debug_init ();

	va_start (ap, fmt);
	vfprintf (debug_fp, fmt, ap);

	va_end (ap);
	return;
}


void
debug_end (void)
{
	if (!debug_fp)
		fclose (debug_fp);

	return;
}

#endif /* DEBUG */
