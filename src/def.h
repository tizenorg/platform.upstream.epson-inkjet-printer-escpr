
#ifndef DEF_H
#define DEF_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>

typedef int bool_t;

/* defines */
#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C }
#else
#define BEGIN_C
#define END_C
#endif /* __cplusplus */


enum exbit_set {
	EXBIT_COLOR_SEPARATE = 0x00000001
};

#endif /* DEF_H */
