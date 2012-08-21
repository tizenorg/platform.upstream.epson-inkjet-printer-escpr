
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "pips.h"

#ifndef N_
#define N_(magic_word)      (magic_word)
#endif

const char *highRanckLabels[] = {
  "hoge hoge",
  N_("I/O"),
  N_("Paper"),
  N_("Layout"),
  N_("Main"),
  N_("Color Management"),
  N_("Utility"),
  NULL
};

const char *ioLabels[] = {
  N_("Input"),
  N_("Output"),
  N_("Select"),
  NULL
};

const char *outputLabels[] = {
  N_("to printer"),
  N_("to file"),
  NULL,
};

const char *outputTypeLabels = N_("export file");

const char *finishButtonLabels[] = {
  N_("Save"),
  N_("Print"),
  N_("Quit"),
  NULL
};

const char *def_margin_label = N_("Default");

OPTION_DATA inkData[] = {
  {     PIPS_OUTPUT_CMYKcm,     "COLOR",        N_("Color")        },
  {     PIPS_OUTPUT_MONO,       "MONO",         N_("Black")        },
  {     END_ARRAY,              "",             ""             }
};

OPTION_DATA mediaSizeData[] = {
    {PM_MSID_A4,            "A4",       N_("A4  210x297 mm")},
    {PM_MSID_LETTER,        "Letter",   N_("Letter  8 1/2x11 in")},
    {PM_MSID_LEGAL,         "Legal",      N_("Legal  8 1/2x14 in")},
    {PM_MSID_A5,            "A5",       N_("A5  148x210 mm")},
    {PM_MSID_A6,            "A6",       N_("A6  105x148 mm")},
    {PM_MSID_B5,            "B5",       N_("B5  182x257 mm")},
    {PM_MSID_EXECUTIVE,     "EXE",      N_("Executive  7 1/4x10 1/2 in")},
    {PM_MSID_HALFLETTER,    "HLT",      N_("Half Letter  5 1/2x8 1/2 in")},
    {PM_MSID_PANORAMIC,     "PANORAM",  N_("Panoramic 210x594 mm")},
    {PM_MSID_TRIM_4X6,      "4X6",      N_("4x6 100x152 mm")},
    {PM_MSID_4X6,           "4X6FULL",  N_("Photo Paper 4x6 in  No Perforations")},
    {PM_MSID_5X8,           "INDEX5",   N_("Index card  5x8 in")                },
    {PM_MSID_8X10,          "8X10",   N_("Index card  8x10 in")               },
    {PM_MSID_10X15,         "PP100",    N_("Photo Paper 100x150 mm")            },
    {PM_MSID_200X300,       "PP200",    N_("Photo Paper 200x300 mm")            },
    {PM_MSID_L,             "L",        N_("3.5x5 in  89x127 mm")               },
    {PM_MSID_POSTCARD,      "PostCard", N_("Post Card  100x148 mm")             },
    {PM_MSID_DBLPOSTCARD,   "DPC",      N_("Double Post Card  200x148 mm")      },
    {PM_MSID_ENV_10_L,      "ENV10_L",  N_("Envelope #10(Landscape) 4 1/8x9 1/2 in")      },
    {PM_MSID_ENV_C6_L,      "ENVC6_L",  N_("Envelope C6(Landscape) 114x162 mm")           },
    {PM_MSID_ENV_DL_L,      "ENVDL_L",  N_("Envelope DL(Landscape) 110x220 mm")           },
    {PM_MSID_NEWEVN_L,      "ENV5X8_L", N_("Envelope(Landscape) 132x220 mm")              },
    {PM_MSID_CHOKEI_3,      "ENVC3",    N_("Envelope  120x235 mm")              },
    {PM_MSID_CHOKEI_4,      "ENVC4",    N_("Envelope  90x205 mm")               },
    {PM_MSID_YOKEI_1,       "ENVY1",    N_("Envelope  120x176 mm")              },
    {PM_MSID_YOKEI_2,       "ENVY2",    N_("Envelope  114x162 mm")              },
    {PM_MSID_YOKEI_3,       "ENVY3",    N_("Envelope  98x148 mm")               },
    {PM_MSID_YOKEI_4,       "ENVY4",    N_("Envelope  105x235 mm")              },
    {PM_MSID_2L,            "2L",       N_("5x7 in  127x178 mm")                },
    {PM_MSID_ENV_10,        "ENV10",    N_("Envelope #10  4 1/8x9 1/2 in")      },
    {PM_MSID_ENV_C6,        "ENVC6",    N_("Envelope C6  114x162 mm")           },
    {PM_MSID_ENV_DL,        "ENVDL",  	N_("Envelope DL  110x220 mm")           },
    {PM_MSID_NEWENV,        "ENV5X8",	N_("Envelope 132x220 mm")       	},
    {PM_MSID_BUZCARD_89X50, "NC",       N_("Business Card 89x50 mm")            },
    {PM_MSID_BUZCARD_89X55, "NC55",     N_("Business Card 89x50 mm")            },
    {PM_MSID_CARD_54X86,    "CARD",     N_("Card 54x86 mm")                       },
    {PM_MSID_BUZCARD_55X91, "NC91",     N_("Business Card 55x91 mm")              },
    {PM_MSID_ALBUM_A5,      "PAA5",     N_("Cover of Photo Album L 127 x 198 mm")     },
    {PM_MSID_ALBUM_L,       "PAL",      N_("Cover of Photo Album A5 210 x 321 mm") },
    {PM_MSID_PALBUM_L_L,    "PALL",     N_("P.Album L (Landscape) 127 x 89 mm") },
    {PM_MSID_PALBUM_2L,     "PA2L",     N_("P.Album 2L 127 x 178 mm")      },
    {PM_MSID_PALBUM_A5_L,   "PAA5L",    N_("P.Album A5 (Landscape) 210 x 148 mm") },
    {PM_MSID_PALBUM_A4,     "PAA4",     N_("P.Album A4 210 x 297 mm")           },
    {PM_MSID_4X7,           "4X7",      N_("16x9 wide size (102 x 181 mm)")},
    {PM_MSID_A3NOBI,        "A3+",      N_("A3+ 329x483 mm")          },
    {PM_MSID_A3,            "A3",       N_("A3  297x420 mm")                    },
    {PM_MSID_B4,            "B4",       N_("B4  257x364 mm")                    },
    {PM_MSID_USB,           "USB",      N_("US B(11x17 in)")},
    {PM_MSID_11X14,         "11X14",    N_("11x14 in")},
    {PM_MSID_B3,            "B3",       N_("B3 364 x 514 mm")},
    {PM_MSID_A2,            "A2",       N_("A2 420 x 594 mm")},
    {PM_MSID_USC,           "USC",      N_("US C 17 x 22 in")},
    {PM_MSID_10X12,	    "10X12",	N_("10x12 in / 254x305 mm")},
    {PM_MSID_12X12,	    "12X12",	N_("30 x 30 cm (12 x 12 in)")},
    {PM_MSID_USER,          "USER",     N_("User Defined")},
    {PM_MSID_UNKNOWN,       "UNKNOWN",  N_("Unknown Paper Size")},
    {END_ARRAY,                "",         ""                                      }
};


OPTION_DATA mediaTypeData[] = {

    { PM_MTID_PLAIN,         "PLAIN",    N_("plain papers")                      },
    { PM_MTID_360INKJET,     "FINE",     N_("360dpi Ink Jet Paper")              },
    { PM_MTID_IRON,          "IRON",     N_("Iron-On Cool Peel Transfer Paper")  },
    { PM_MTID_PHOTOINKJET,   "SFINE",    N_("Photo Quality Ink Jet Paper")       },
    { PM_MTID_PHOTOADSHEET,  "PADSHEET", N_("Photo Quality Self Adhesive Sheets")},
    { PM_MTID_MATTE,         "PMMATT",   N_("EPSON Matte")                       },
    { PM_MTID_PHOTO,         "GPPAPER",  N_("Photo Paper")            	 },  /* chenged Wed Jan 28 2009 */
    { PM_MTID_PHOTOFILM,     "GFILM",    N_("Photo Quality Glossy Film")         },
    { PM_MTID_MINIPHOTO,     "MINIPHOTO",N_("Epson Photo Stickers")		 },
    { PM_MTID_OHP,           "OHP",      N_("Ink Jet Transparencies")            },
    { PM_MTID_BACKLIGHT,     "BLFILM",   N_("Ink Jet Back Light Film")           },
    { PM_MTID_PGPHOTO,       "PMPHOTO",  N_("EPSON Premium Glossy")        },
    { PM_MTID_PSPHOTO,       "PSGLOS",   N_("Epson Premium Semigloss")     },
    { PM_MTID_PLPHOTO,       "MCLP",     N_("Premium Luster Photo Paper")    	 },
    { PM_MTID_MCGLOSSY,      "PHOTOW",   N_("Glossy Paper - Photo Weight")         },
    { PM_MTID_ARCHMATTE,     "ARCMATT",  N_("Archival Matte Paper")              },
    { PM_MTID_WATERCOLOR,    "WATERCOLOR", N_("Watercolor Paper - Radiant White")},
    { PM_MTID_PROGLOSS,      "PROGLS", 	 N_("Professional Glossy Paper")},
    { PM_MTID_MATTEBOARD,    "MBOARD",   N_("Matte Board Paper")},
    { PM_MTID_PHOTOGLOSS,    "PHOTOGLS", N_("Photo Quality Glossy Paper")},
    { PM_MTID_SEMIPROOF,     "SEMIPROOF",N_("Dupont/EPSONSemigloss Proofing Paper-A")},
    { PM_MTID_SUPERFINE2,    "SFINE2",   N_("Photo Quality Ink Jet Paper Plus")},
    { PM_MTID_DSMATTE,       "DBSMATT",  N_("Double-Sided Matte Paper")},
    { PM_MTID_CLPHOTO,       "CLPP",     N_("ColorLife Photo Paper")},
    { PM_MTID_ECOPHOTO,      "EPHOTO",   N_("Economy Photo Paper")},
    { PM_MTID_VELVETFINEART, "VELVET_FINE", N_("Velvet Fine Art Paper")},
    { PM_MTID_PROOFSEMI,     "PROOF_S",  N_("Proofing Paper Semimatte")},
    { PM_MTID_HAGAKIRECL,    "RCPC",     N_("Recycled Paper Postcard")},
    { PM_MTID_HAGAKIINKJET,  "IJPC",     N_("Ink Jet Postcard")},
    { PM_MTID_PHOTOINKJET2,  "PIJ2", 	 N_("Epson Photo Quality Ink Jet")},
    { PM_MTID_DURABRITE,     "EPP_P",    N_("DURABrite Photo Paper")},
    { PM_MTID_MATTEMEISHI,   "MMEISHI",  N_("Matte Business Card")},
    { PM_MTID_HAGAKIATENA,   "ADDRESS",  N_("Post Card Address Side")},
    { PM_MTID_PHOTOALBUM,    "P_ALBUM",  N_("Photo Album")},
    { PM_MTID_PHOTOSTAND,    "P_BOARD",  N_("Photo Board Paper")},
    { PM_MTID_RCB,           "RCB",	 N_("RC-B")},
    { PM_MTID_PGPHOTOEG,     "PGPHOTOEG",N_("Premium Glossy Photo Paper_2")},
    { PM_MTID_ENVELOPE,      "ENV", 	 N_("Envelope")},
    { PM_MTID_PLATINA,       "PLATINA",  N_("Epson Ultra Glossy")},
    { PM_MTID_ULTRASMOOTH,   "USF_ART",  N_("UltraSmooth Fine Art Paper")},
/* add Wed Jan 28 2009 v */
    { PM_MTID_SFHAGAKI,   	"SFHAGAKI",  N_("Super Fine Postcard")},
    { PM_MTID_PHOTOSTD,   	"PHOTOSTD",  N_("Premium Glossy Photo Paper")},
    { PM_MTID_GLOSSYHAGAKI, "GLOSSYHAGAKI", N_("Glossy Postcard")},
    { PM_MTID_GLOSSYPHOTO,  "GLOSSYPHOTO",  N_("Glossy Photo Paper")},
    { PM_MTID_GLOSSYCAST,	"GLOSSYCAST",  N_("Epson Photo")},
    { PM_MTID_BUSINESSCOAT, "BUSINESSCOAT", N_("Business Ink Jet Coat Paper")},
/* add Wed Jan 28 2009 ^ */
/* del Wed Jan 28 2009 { PM_MTID_GPHOTO,        "GPHOTO",   N_("Glossy Photo Paper_2")},*/
/* del Wed Jan 28 2009 { PM_MTID_PHOTOEGCP,     "PHOTOEGCP",N_("Epson Photo")},*/
    { PM_MTID_CDDVD,         "CDR",      N_("CD/DVD")},
    { PM_MTID_CDDVDHIGH,     "CDRH",     N_("CD/DVD Premium Surface")},
    { PM_MTID_CLEANING,      "CLEAN",    N_("Cleaning Sheet")},
    { PM_MTID_UNKNOWN,       "UNKNOWN",  N_("Unknown Paper Type")},
    { END_ARRAY,             "",         ""                       }
};

OPTION_DATA levelData[] = {
  {     PIPS_LV_DRAFT,          "DRAFT",        N_("Draft")           },
  {     PIPS_LV_NORMAL,         "NORMAL",       N_("Normal")          },
  {     PIPS_LV_HIGH,           "HIGH",         N_("High")            },
  {     END_ARRAY,              "",             ""                    }
};

OPTION_DATA fitPageData[] = {
  {     PIPS_FP_OFF,    "OFF",  ""                  },
  {     PIPS_FP_ON,     "ON",   N_("Fit to Page")   },
  {     END_ARRAY,      "",     ""                  }
};

OPTION_DATA multiPageData[] = {
  {     PIPS_MP_1,          "1",        N_("None")        },
  {     PIPS_MP_2,          "2",        N_("2 Pages")     },
  {     PIPS_MP_4,          "4",        N_("4 Pages")     },
  {     END_ARRAY,      "",     ""                  }
};


OPTION_DATA borderlessprintData[] = {
  {     PIPS_BL_OFF,    "OFF",  ""                      },
  {     PIPS_BL_ON,     "ON",   N_("Borderless") },
  {     END_ARRAY,      "",     ""                      }
};

OPTION_ALL opt_all[] = {
  {     P_SCALE,                NULL,           N_("Reduce/Enlarge (%)")    },
  {     P_INK,                  inkData,        N_("Ink")                   },
  {     P_MEDIA_SIZE,           mediaSizeData,  N_("Paper Size")            },
  {     P_MEDIA_TYPE,           mediaTypeData,  N_("Media Type")            },
  {     P_BRIGHTNESS,           NULL,           N_("Brightness")            },
  {     P_CONTRAST,             NULL,           N_("Contrast")              },
  {     P_SATURATION,           NULL,           N_("Saturation")            },
  {     P_QUALITY_LEVEL,        levelData,      N_("Quality")               },
  {     P_MARGIN,               NULL,           N_("Margin (mm)")           },
  {     P_FIT_PAGE,             fitPageData,    N_("Fit to Page")           },
  {     P_MULTI_PAGE,           multiPageData,  N_("Multi Page")            },
  {     P_MARGIN_X,             NULL,           N_("Left")                  },
  {     P_MARGIN_Y,             NULL,           N_("Top")                   },
  {     P_BORDERLESS_PRINT,     borderlessprintData,  N_("Borderless")  },
  {     END_ARRAY,              NULL,           ""                          }
};

const char Status_Monitor[] = N_("Status Monitor");
const char Ink_Level[] = N_("Ink Level");
const char Check[]=N_("Check");
const char Ink_Black[] = N_("Black");
const char Ink_Color[] = N_("Color");
const char Ink_Chenge[] = N_("Replacement");
const char Ink_Volume[] = N_("Ink");
const char No_Connect_Message[] = N_("Cannot communicate with a printer.");

const char Utility_Err_Msg[] = N_("A problem has occurred.");

const char Ink_Change[] = N_("InkChange");
const char Ink_Change_Msg[] = N_("The ink cartridge configuration was changed.\n"
				 "Settings will be reset to the default.\n"
				 "Reconfigure the settings as needed.");

const char Connected_Printer_Chg[] = N_("Printer Changed");
const char Connected_Printer_Chg_Msg[] = N_("The information on a printer was changed.\n"
				      "Setup is returned to a default.\n"
				      "Please set up again.");
const char Printer_Disconnected[] = N_("Printer Disconnected");
const char Printer_Disconnected_Msg[] = N_("Please check connection with a printer and turn ON the power supply of a printer.");
