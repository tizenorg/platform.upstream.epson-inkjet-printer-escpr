
#ifndef STR_H
#define STR_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "def.h"

BEGIN_C

char * str_clone (char *, size_t);
char * str_delete (char *);

END_C

#endif /* STR_H */
