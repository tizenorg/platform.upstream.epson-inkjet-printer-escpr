
#ifndef CSV_H
#define CSV_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "def.h"

typedef struct csvlist_str
{
	char *path;
	char *top_p;
	long size;

	char **list;
	int lnum;

} csvlist_t;

BEGIN_C

csvlist_t *csvlist_open (const char *);
void csvlist_close (csvlist_t *);
int csvlist_search_keyword (csvlist_t *, int, const char *);
char * csvlist_get_word (csvlist_t *, int);
int csvlist_get_max (csvlist_t *);

END_C

#endif /* CSV_H */
