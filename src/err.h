
#ifndef ERR_H
#define ERR_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdarg.h>

#include "def.h"

#define DEBUG_PATH "/tmp/eps_debug.txt"

enum msgtype {
	MSGTYPE_ERROR = 0,
	MSGTYPE_WARNING,
	MSGTYPE_INFO,
	MSGTYPE_MESSAGE
};



void err_init (const char *);
void err_msg (enum msgtype, const char *, ...);
void err_fatal (const char *, ...);
void err_system (const char *, ...);



#endif /* ERR_H */
