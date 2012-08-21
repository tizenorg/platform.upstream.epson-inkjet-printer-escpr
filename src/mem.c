
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include "mem.h"
#include "err.h"


void *
mem_malloc (unsigned int size, bool_t crit)
{
	void *m;

	m = malloc (size);
	if (crit && m == NULL)
		err_system ("mem_malloc");

	return m;
}

void *
mem_calloc (unsigned int num, unsigned int size, bool_t crit)
{
	void *m;

	m = calloc (num, size);
	if (crit && m == NULL)
		err_system ("mem_calloc");

	return m;
}

void *
mem_realloc (void *m, u_int size, bool_t crit)
{
	m = realloc (m, size);
	if (crit && m == NULL)
		err_system ("mem_realloc");

	return m;
}

void
mem_free (void *m)
{
	if (m) free (m);

	return;
}
