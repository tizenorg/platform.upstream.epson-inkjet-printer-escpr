/*
 * Epson Inkjet Printer Driver (ESC/P-R) for Linux
 * Copyright (C) 2002-2005 AVASYS CORPORATION.
 * Copyright (C) Seiko Epson Corporation 2002-2012.
 *
 *  This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
 */

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
