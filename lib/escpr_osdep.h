/***********************************************************************
 *
 * Copyright (c) 2005-2008 Seiko Epson Corporation.
 * 
 * This program is free software; you can redistribute it and/or modify
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * File Name:   escpr_osdep.h
 *
 ***********************************************************************/

#ifndef __EPSON_ESCPR_OSDEP_H__
#define __EPSON_ESCPR_OSDEP_H__

#include <stddef.h>

/*=======================================================================================*/
/* typedef Area                                                                          */
/*=======================================================================================*/
typedef unsigned char      ESCPR_UBYTE1;
typedef unsigned short     ESCPR_UBYTE2;
typedef unsigned int       ESCPR_UBYTE4;

typedef char               ESCPR_BYTE1;
typedef short              ESCPR_BYTE2;
typedef int                ESCPR_BYTE4;

/* Local time */
typedef struct _tagESCPR_LOCAL_TIME_ {
    ESCPR_UBYTE2        year;
    ESCPR_UBYTE1        mon;
    ESCPR_UBYTE1        day;
    ESCPR_UBYTE1        hour;
    ESCPR_UBYTE1        min;
    ESCPR_UBYTE1        sec;
}ESCPR_LOCAL_TIME;

/*=======================================================================================*/
/* Define Area                                                                           */
/*=======================================================================================*/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL int
#endif

/*=======================================================================================*/
/* Global Functions                                                                           */
/*=======================================================================================*/
void        ESCPR_Mem_Set(void* Dest, ESCPR_BYTE4 c, size_t Size);
void        ESCPR_Mem_Copy(void* Dest, const void* Src, size_t Size);
void*       ESCPR_Mem_Alloc(size_t MemSize);
void        ESCPR_Mem_Free(void* MemPtr);
BOOL        ESCPR_Mem_Compare(const void *s1, const void *s2, size_t n);
void        ESCPR_GetLocalTime(ESCPR_LOCAL_TIME *epsTime);

#endif
