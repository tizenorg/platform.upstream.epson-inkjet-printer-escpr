#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "mem.h"
#include "str.h"

char *
str_clone (char *str, size_t size)
{
	char *new;

	if (!str || !size)
		return NULL;	/* error */
	
	new = mem_new (char, size + 1);
	
	strncpy (new, str, size);
	new[size] = '\0';

	return new;
}

char *
str_delete (char *str)
{
	if (str)
		mem_free (str);

	str = 0;

	return 0;
}
