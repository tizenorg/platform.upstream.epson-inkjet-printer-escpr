
#ifndef DEBUG_H
#define DEBUG_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdarg.h>

#include "def.h"

#if !DEBUG   /* not DEBUG */
#define DEBUG_START
#define DEDBUG_END

#define DEBUG_QUALITY_STRUCT(str)
#define DEBUG_JOB_STRUCT(str)

#else /* DEBUG */
#define DEBUG_START debug_init ()
#define DEDBUG_END debug_end ()

#define DEBUG_QUALITY_STRUCT(str) \
debug_dump ("--- ESCPR_PRINT_QUALITY ---\n\n" \
	    "mediaTypeID  = %d\n" \
	    "quality      = %d\n" \
	    "colormode    = %d\n" \
	    "brightness   = %d\n" \
	    "contrast     = %d\n" \
	    "saturation   = %d\n" \
	    "colorPlane   = %d\n" \
	    "paltetteSize = %d\n" \
	    "--------------------------\n\n" \
	    ,str.MediaTypeID \
	    ,str.PrintQuality \
            ,str.ColorMono \
	    ,str.Brightness \
	    ,str.Contrast \
	    ,str.Saturation \
	    ,str.ColorPlane \
	    ,str.PaletteSize)

#define DEBUG_JOB_STRUCT(str) \
debug_dump ("--- ESCPR_PRINT_JOB  ---\n\n" \
	    "paperWidth     = %ld\n" \
	    "paperLength    = %ld\n" \
	    "topMargin      = %d\n" \
	    "leftMargin     = %d\n" \
	    "areaWidth      = %ld\n" \
	    "areaLength     = %ld\n" \
	    "inResolution   = %d\n" \
	    "printDirection = %d\n" \
	    "--------------------------\n\n" \
	    ,str.PaperWidth \
	    ,str.PaperLength \
	    ,str.TopMargin \
	    ,str.LeftMargin \
	    ,str.PrintableAreaWidth \
	    ,str.PrintableAreaLength \
	    ,str.InResolution \
	    ,str.PrintDirection)

BEGIN_C

void debug_init (void);
void debug_dump (const char *, ...);
void debug_end (void);

END_C

#endif /* DEBUG */

#endif /* DEBUG_H */
