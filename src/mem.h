
#ifndef MEM_H
#define MEM_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "def.h"

/* If it is a success, an address is returned, and if it is failure, a message is outputted and it ends. */
#define mem_new(type, num) \
((type *) mem_malloc (sizeof (type) * (num), 1))
#define mem_new0(type, num) \
((type *) mem_calloc ((num), sizeof (type), 1))
#define mem_renew(type, pointer, num) \
((type *) mem_realloc (pointer, sizeof (type) * (num), 1))

/*
* bool value true Critical
* false Normal operation
*/
BEGIN_C

void * mem_malloc (unsigned int, bool_t);
void * mem_calloc (unsigned int, unsigned int, bool_t);
void * mem_realloc (void *, unsigned int, bool_t);
void mem_free (void *);

END_C

#endif /* MEM_H */
