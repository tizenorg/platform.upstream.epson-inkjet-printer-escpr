/***********************************************************************
 *
 * Copyright (c) 2005 Seiko Epson Corporation.
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
 * File Name:   escpr_def.h
 *
 ***********************************************************************/

#ifndef __EPSON_ESCPR_DEF_H__
#define __EPSON_ESCPR_DEF_H__

#include "escpr_osdep.h"

/*=======================================================================================*/
/* typedef                                                                               */
/*=======================================================================================*/

typedef ESCPR_BYTE4 (* ESCPR_FPSPOOLFUNC)(void* hParam, 
        const ESCPR_UBYTE1* pBuf, ESCPR_UBYTE4 cbBuf);

typedef ESCPR_BYTE4 ESCPR_ERR_CODE;

typedef enum _ESCPR_ENDIAN {
    ESCPR_ENDIAN_NOT_TESTED = 0,
    ESCPR_ENDIAN_BIG,
    ESCPR_ENDIAN_LITTLE
} ESCPR_ENDIAN;

/*=======================================================================================*/
/* struct                                                                                */
/*=======================================================================================*/

typedef struct tagESCPR_RECT{
    ESCPR_BYTE4 left;
    ESCPR_BYTE4 top;
    ESCPR_BYTE4 right;
    ESCPR_BYTE4 bottom;
}ESCPR_RECT;

typedef struct tagESCPR_BANDBMP{
    ESCPR_UBYTE4  WidthBytes;      /* number of bytes in each scan line            */
    ESCPR_UBYTE1* Bits;            /* pointer to the the bit values for the bitmap */
}ESCPR_BANDBMP;

typedef struct tagESCPR_OPT{
    ESCPR_BYTE4          nPrnID;         /* Printer ID                */
    ESCPR_FPSPOOLFUNC    fpspoolfunc;    /* pointer to spool function */
}ESCPR_OPT;


/*----- Set Print Quality -----*/
typedef struct tagESCPR_PRINT_QUALITY{
    ESCPR_UBYTE1     MediaTypeID;
    ESCPR_UBYTE1     PrintQuality;
    ESCPR_UBYTE1     ColorMono;
    ESCPR_BYTE1      Brightness;
    ESCPR_BYTE1      Contrast;
    ESCPR_BYTE1      Saturation;
    ESCPR_UBYTE1     ColorPlane;
    ESCPR_UBYTE2     PaletteSize;
    ESCPR_UBYTE1*    PaletteData;
}ESCPR_PRINT_QUALITY;

/*----- Set Print Job -----*/
typedef struct tagESCPR_PRINT_JOB{
    ESCPR_UBYTE4     PaperWidth;
    ESCPR_UBYTE4     PaperLength;
    ESCPR_BYTE2      TopMargin;
    ESCPR_BYTE2      LeftMargin;
    ESCPR_UBYTE4     PrintableAreaWidth;
    ESCPR_UBYTE4     PrintableAreaLength;
    ESCPR_UBYTE1     InResolution;
    ESCPR_UBYTE1     PrintDirection;
}ESCPR_PRINT_JOB;

typedef enum tagESCPR_STATUS{
    ESCPR_STATUS_NOT_INITIALIZED,
    ESCPR_STATUS_INITIALIZED
}ESCPR_STATUS;

typedef struct {
    ESCPR_FPSPOOLFUNC       gfpSendData;  /* A function pointer for send data function */
} ESCPR_GLOBALFUNC;


/*=======================================================================================*/
/* define                                                                                */
/*=======================================================================================*/

/*=======================================================================================*/
/* Media Information of Control Command "MI"                                             */
/*=======================================================================================*/
/* Media Types */
#define ESCPR_MTID_PLAIN              0
#define ESCPR_MTID_360INKJET          1
#define ESCPR_MTID_IRON               2
#define ESCPR_MTID_PHOTOINKJET        3
#define ESCPR_MTID_PHOTOADSHEET       4
#define ESCPR_MTID_MATTE              5
#define ESCPR_MTID_PHOTO              6
#define ESCPR_MTID_PHOTOFILM          7
#define ESCPR_MTID_MINIPHOTO          8
#define ESCPR_MTID_OHP                9   
#define ESCPR_MTID_BACKLIGHT          10  
#define ESCPR_MTID_PGPHOTO            11  
#define ESCPR_MTID_PSPHOTO            12  
#define ESCPR_MTID_PLPHOTO            13  
#define ESCPR_MTID_MCGLOSSY           14  
#define ESCPR_MTID_ARCHMATTE          15  
#define ESCPR_MTID_WATERCOLOR         16  
#define ESCPR_MTID_PROGLOSS           17  
#define ESCPR_MTID_MATTEBOARD         18  
#define ESCPR_MTID_PHOTOGLOSS         19  
#define ESCPR_MTID_SEMIPROOF          20  
#define ESCPR_MTID_SUPERFINE2         21  
#define ESCPR_MTID_DSMATTE            22  
#define ESCPR_MTID_CLPHOTO            23  
#define ESCPR_MTID_ECOPHOTO           24  
#define ESCPR_MTID_VELVETFINEART      25  
#define ESCPR_MTID_PROOFSEMI          26  
#define ESCPR_MTID_HAGAKIRECL         27  
#define ESCPR_MTID_HAGAKIINKJET       28  
#define ESCPR_MTID_PHOTOINKJET2       29  
#define ESCPR_MTID_DURABRITE          30  
#define ESCPR_MTID_MATTEMEISHI        31  
#define ESCPR_MTID_HAGAKIATENA        32  
#define ESCPR_MTID_PHOTOALBUM         33  
#define ESCPR_MTID_PHOTOSTAND         34  
#define ESCPR_MTID_RCB                35  
#define ESCPR_MTID_PGPHOTOEG          36  
#define ESCPR_MTID_ENVELOPE           37  
#define ESCPR_MTID_PLATINA            38  
#define ESCPR_MTID_ULTRASMOOTH        39
#define ESCPR_MTID_CDDVD              91  
#define ESCPR_MTID_CDDVDHIGH          92  
#define ESCPR_MTID_CLEANING           99  

/*=======================================================================================*/
/* Definition of Parameter for Print Quality                                             */
/*=======================================================================================*/
/* PrintQuality */
#define ESCPR_PQ_DRAFT                  0   /* Draft                    */
#define ESCPR_PQ_NORMAL                 1   /* Normal                   */
#define ESCPR_PQ_HIGH                   2   /* High                     */

/* ColorMono */
#define ESCPR_CM_COLOR                  0   /* Color                    */
#define ESCPR_CM_MONOCHROME             1   /* Monochrome               */

/* Brightness */
#define ESCPR_BRIGHT_DEF                0   /* Default                  */

/* Contrast */
#define ESCPR_CONT_DEF                  0   /* Default                  */

/* Saturation */
#define ESCPR_SATU_DEF                  0   /* Default                  */

/* ColorPlane */
#define ESCPR_CP_FULLCOLOR              0   /* Full Color               */
#define ESCPR_CP_256COLOR               1   /* 256 Color                */

/*=======================================================================================*/
/* Definition of Parameter for Print Job                                                 */
/*=======================================================================================*/
/* InResolution */
#define ESCPR_IR_3636                   0   /* 360 x 360 dpi            */
#define ESCPR_IR_7272                   1   /* 720 x 720 dpi            */
#define ESCPR_IR_3030                   2   /* 300 x 300 dpi            */
#define ESCPR_IR_6060                   3   /* 600 x 600 dpi            */

/* PrintDirection */
#define ESCPR_PD_BIDIREC                0   /* Bi-direction             */
#define ESCPR_PD_UNIDIREC               1   /* Uni-direction            */

/*=======================================================================================*/
/* Definition of Send Data                                                               */
/*=======================================================================================*/
/* CompMode */
#define ESCPR_COMP_NO                   0   /* Non compression          */
#define ESCPR_COMP_DONE                 1   /* Runlength compression    */

/*=======================================================================================*/
/* Definition of End Page                                                                */
/*=======================================================================================*/
#define ESCPR_END_PAGE                  0   /* There is no next page    */
#define ESCPR_NEXT_PAGE                 1   /* There is a next page     */

/*=======================================================================================*/
/* Definition of Error Code                                                              */
/*=======================================================================================*/
#define ESCPR_ERR_NOERROR               0   /* terminated normally                     */
#define ESCPR_ERR_NOT_SUPPORT           -1  /* parameter aren't be supported.          */
#define ESCPR_ERR_DON_T_PROCESS         -2  /* on don't process this situation.        */
#define ESCPR_ERR_CAN_T_FREE_HANDLE     -3  /* can't free handle.                      */
#define ESCPR_ERR_CAN_T_ALLOC_MEMORY    -4  /* can't allocate needed memory.           */
#define ESCPR_ERR_HAPPEN_PROBLEM        -10 /* happened any error.                     */
#define ESCPR_ERR_NOINBMP               -20 /* doesn't get an input bitmap.            */
#define ESCPR_ERR_NOATBMP               -21 /* doesn't get an attribute bitmap.        */
#define ESCPR_ERR_CAN_T_LOAD_DLL        -30 /* can't load DLL                          */
#define ESCPR_ERR_SPOOL_IO              -40 /* error with spooling i/o function        */

#endif

