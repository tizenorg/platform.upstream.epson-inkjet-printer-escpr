
#ifndef LIB_PRT_X_H
#define LIB_PRT_X_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "pipsDef.h"
#include "escpr_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DLL_PRT_INIT  "escprInitJob"
#define DLL_PAGE_INIT "escprInitPage"
#define DLL_BAND_OUT  "escprBandOut"
#define DLL_PAGE_END  "escprTerminatePage"
#define DLL_PRT_END   "escprDestroyJob"
#define DLL_FILTER_PMREPLY "escprFilterPMReply"
	
	typedef void *handle_t;

	typedef struct point_struct
	{
		long x;
		long y;
	} point_t;

	typedef int (*INIT_FUNC) (const ESCPR_OPT *, const ESCPR_PRINT_QUALITY *, const ESCPR_PRINT_JOB*);
	typedef int (*PINIT_FUNC) (void);
	typedef int (*OUT_FUNC) (const ESCPR_BANDBMP *, ESCPR_RECT *);
	typedef int (*PEND_FUNC) (const ESCPR_UBYTE1);
	typedef int (*END_FUNC) (void);  
	typedef int (*FILTER_PMPEPLY_FUNC) (ESCPR_UBYTE1 *);
	
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIB_PRT_X_H */
