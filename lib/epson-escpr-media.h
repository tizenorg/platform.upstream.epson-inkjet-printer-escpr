/*_______________________________  epson-escpr-media.h   _______________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
 *   Copyright (c) 2009  Seiko Epson Corporation                 All rights reserved.
 *
 *   Copyright protection claimed includes all forms and matters of
 *   copyrightable material and information now allowed by statutory or judicial
 *   law or hereinafter granted, including without limitation, material generated
 *   from the software programs which are displayed on the screen such as icons,
 *   screen display looks, etc.
 *
 */

/*******************************************|********************************************/
/*                                                                                      */
/*                             Epson ESC/PR Media Information                           */
/*                                                                                      */
/*******************************************|********************************************/
#ifndef __EPSON_ESCPR_MEDIA_H__
#define __EPSON_ESCPR_MEDIA_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------- Local Compiler Switch --------------------------------*/
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"

/*------------------------------------- Data Types -------------------------------------*/
/*******************************************|********************************************/

/*----------------------------------  Generic Macros   ---------------------------------*/
/*******************************************|********************************************/


/*-------------  Definition of Media Size ID / Global Media Size Declarations   --------*/
/*******************************************|********************************************/
        /*** Media Size ID                                                              */
        /*** ---------------------------------------------------------------------------*/
typedef enum _tagMEDIA_SIZE_ID_ {
                                /* Dec  Hex  [      mm       ]  */
    EPS_MSID_A4             =0, /*  0  0x00  [210.000,297.000]  */
    EPS_MSID_LETTER           , /*  1  0x01  [215.900,279.400]  */
    EPS_MSID_LEGAL            , /*  2  0x02  [215.900,355.600]  */
    EPS_MSID_A5               , /*  3  0x03  [148.000,210.000]  */
    EPS_MSID_A6               , /*  4  0x04  [105.000,148.000]  */
    EPS_MSID_B5               , /*  5  0x05  [176.000,250.000]  */
    EPS_MSID_EXECUTIVE        , /*  6  0x06  [184.150,266.700]  */
    EPS_MSID_HALFLETTER       , /*  7  0x07  [127.000,215.900]  */
    EPS_MSID_PANORAMIC        , /*  8  0x08  [210.000,594.000]  */
    EPS_MSID_TRIM_4X6         , /*  9  0x09  [113.600,164.400]  */
    EPS_MSID_4X6              , /* 10  0x0A  [101.600,152.400]  */
    EPS_MSID_5X8              , /* 11  0x0B  [127.000,203.200]  */
    EPS_MSID_8X10             , /* 12  0x0C  [203.200,203.200]  */
    EPS_MSID_10X15            , /* 13  0x0D  [254.000,381.000]  */
    EPS_MSID_200X300          , /* 14  0x0E  [200.000,300.000]  */
    EPS_MSID_L                , /* 15  0x0F  [ 88.900,127.000]  */
    EPS_MSID_POSTCARD         , /* 16  0x10  [100.000,148.000]  */
    EPS_MSID_DBLPOSTCARD      , /* 17  0x11  [200.000,148.000]  */
    EPS_MSID_ENV_10_L         , /* 18  0x12  [241.300,104.775]  */
    EPS_MSID_ENV_C6_L         , /* 19  0x13  [162.000,114.000]  */
    EPS_MSID_ENV_DL_L         , /* 20  0x14  [220.000,110.000]  */
    EPS_MSID_NEWEVN_L         , /* 21  0x15  [220.000,132.000]  */
    EPS_MSID_CHOKEI_3         , /* 22  0x16  [120.000,235.000]  */
    EPS_MSID_CHOKEI_4         , /* 23  0x17  [ 90.000,205.000]  */
    EPS_MSID_YOKEI_1          , /* 24  0x18  [120.000,176.000]  */
    EPS_MSID_YOKEI_2          , /* 25  0x19  [114.000,162.000]  */
    EPS_MSID_YOKEI_3          , /* 26  0x1A  [ 98.000,148.000]  */
    EPS_MSID_YOKEI_4          , /* 27  0x1B  [105.000,235.000]  */
    EPS_MSID_2L               , /* 28  0x1C  [127.000,177.800]  */
    EPS_MSID_ENV_10_P         , /* 29  0x1D  [104.775,241.300]  */
    EPS_MSID_ENV_C6_P         , /* 30  0x1E  [114.000,162.000]  */
    EPS_MSID_ENV_DL_P         , /* 31  0x1F  [110.000,220.000]  */
    EPS_MSID_NEWENV_P         , /* 32  0x20  [132.000,220.000]  */
    EPS_MSID_MEISHI           , /* 33  0x21  [ 89.000, 55.000]  */
    EPS_MSID_BUZCARD_89X50    , /* 34  0x22  [ 89.000, 50.000]  */
    EPS_MSID_CARD_54X86       , /* 35  0x23  [ 54.000, 86.000]  */
    EPS_MSID_BUZCARD_55X91    , /* 36  0x24  [ 55.000, 91.000]  */
    EPS_MSID_ALBUM_L          , /* 37  0x25  [127.000,198.000]  */
    EPS_MSID_ALBUM_A5         , /* 38  0x26  [210.000,321.000]  */
    EPS_MSID_PALBUM_L_L       , /* 39  0x27  [127.000  89.000]  */
    EPS_MSID_PALBUM_2L        , /* 40  0x28  [127.000,177.900]  */
    EPS_MSID_PALBUM_A5_L      , /* 41  0x29  [210.000,148.300]  */
    EPS_MSID_PALBUM_A4        , /* 42  0x2A  [210.000,296.300]  */
    EPS_MSID_HIVISION         , /* 43  0x2B  [101.600,180.600]  */
    EPS_MSID_A3NOBI        =61, /* 61  0x3D  [329.000,483.000]  */
    EPS_MSID_A3               , /* 62  0x3E  [297.000,420.000]  */
    EPS_MSID_B4               , /* 63  0x3F  [257.000,364.000]  */
    EPS_MSID_USB              , /* 64  0x40  [279.400,431.800]  */
    EPS_MSID_11X14            , /* 65  0x41  [279.400,355.600]  */
    EPS_MSID_B3               , /* 66  0x42  [364.000,515.000]  */
    EPS_MSID_A2               , /* 67  0x43  [420.000,594.000]  */
    EPS_MSID_USC              , /* 68  0x44  [431.800,558.800]  */
    EPS_MSID_10X12            , /* 69  0x45  [254.000,304.800]  */
     EPS_MSID_F4		      	,
	EPS_MSID_ENVY0					,
	EPS_MSID_ENVY6					,
	EPS_MSID_ENVK2					,
	EPS_MSID_4PC	      		,
	EPS_MSID_CHOKEI_5	      	,
    EPS_MSID_CHOKEI_6	      ,
    EPS_MSID_YOUGATA_1	      ,
    EPS_MSID_YOUGATA_2	      ,
    EPS_MSID_KAKUGATA	      ,
    //EPS_MSID_QPOSTCARD	      ,
    EPS_MSID_LEB	      ,	/*Ledger（B）*/
    EPS_MSID_GLG,
    EPS_MSID_GLT	      , /*Government Letter */ 
    EPS_MSID_HLT	      ,
    EPS_MSID_12X12            , /* 70  0x46  [304.800,304.800]  */
    EPS_MSID_USER          =99, /* 99  0x63  [  0.000,  0.000]  */
    EPS_MSID_UNKNOWN       =0xff/* unknown                      */
} MEDIA_SIZE_ID;

        /*** Media Data Struct                                                          */
        /*** ---------------------------------------------------------------------------*/
typedef struct _tagEPS_MEDIA_INFO_ {
    EPS_INT32 id;
    EPS_INT32 paper_x;
    EPS_INT32 paper_y;
    EPS_INT32 print_area_x_border;
    EPS_INT32 print_area_y_border;
    EPS_INT16 left_margin_borderless;
    EPS_INT16 top_margin_borderless;
    EPS_INT32 print_area_x_borderless;
    EPS_INT32 print_area_y_borderless;
} EPS_MEDIA_INFO;

        /*** Media Size Information (360dpi)                                            */
        /*** ---------------------------------------------------------------------------*/
static const EPS_MEDIA_INFO epsMediaSize[] = {
/*          ID              PaperW/L    3mm W/L     0mm L/T   0mm W/L   */
{   EPS_MSID_A4,            2976, 4209, 2892, 4125, -36, -42, 3048, 4321 },
{   EPS_MSID_LETTER,        3060, 3960, 2976, 3876, -36, -42, 3132, 4072 },
{   EPS_MSID_LEGAL,         3060, 5040, 2976, 4956, -36, -42, 3132, 5152 },
{   EPS_MSID_A5,            2098, 2976, 2014, 2892, -36, -42, 2170, 3088 },
{   EPS_MSID_A6,            1488, 2098, 1404, 2014, -36, -42, 1560, 2210 },
{   EPS_MSID_B5,            2580, 3643, 2495, 3558, -36, -42, 2652, 3755 },
{   EPS_MSID_EXECUTIVE,     2610, 3780, 2526, 3696, -36, -42, 2682, 3892 },
{   EPS_MSID_HALFLETTER,    1980, 3060, 1896, 2976, -36, -42, 2052, 3172 },
{   EPS_MSID_PANORAMIC,     2976, 8419, 2892, 8335, -36, -42, 3048, 8531 },
{   EPS_MSID_TRIM_4X6,      1610, 2330, 1526, 2246, -36, -42, 1682, 2442 },
{   EPS_MSID_4X6,           1440, 2160, 1356, 2076, -36, -42, 1512, 2272 },
{   EPS_MSID_5X8,           1800, 2880, 1716, 2796, -36, -42, 1872, 2992 },
{   EPS_MSID_8X10,          2880, 3600, 2796, 3516, -36, -42, 2952, 3712 },
{   EPS_MSID_F4,            2976, 4677, 2892, 4593, -36, -42, 3048, 4789 },
{   EPS_MSID_ENVY0,         1701, 3331, 1617, 3247, -36, -42, 1773, 3443 },
{   EPS_MSID_ENVY6,         1389, 2693, 1305, 2609, -36, -42, 1461, 2805 },
{   EPS_MSID_ENVK2,         3402, 4706, 3318, 4622, -36, -42, 3474, 4818 },
{   EPS_MSID_4PC,           2835, 4195, 2751, 4111, -36, -42, 2907, 4307 },
{   EPS_MSID_10X15,         1417, 2125, 1333, 2041, -36, -42, 1489, 2237 },
{   EPS_MSID_200X300,       3061, 4790, 2977, 4706, -36, -42, 3133, 4902 },
{   EPS_MSID_L,             1260, 1800, 1176, 1716, -36, -42, 1332, 1912 },
{   EPS_MSID_POSTCARD,      1417, 2098, 1333, 2013, -36, -42, 1489, 2210 },
{   EPS_MSID_DBLPOSTCARD,   2835, 2098, 2751, 2014, -36, -42, 2907, 2210 },
{   EPS_MSID_ENV_10_L,      3420, 1485, 3336, 1401, -36, -42, 3492, 1597 },
{   EPS_MSID_ENV_C6_L,      2296, 1616, 2212, 1532, -36, -42, 2368, 1728 },
{   EPS_MSID_ENV_DL_L,      3118, 1559, 3034, 1475, -36, -42, 3190, 1671 },
{   EPS_MSID_NEWEVN_L,      3118, 1871, 3034, 1787, -36, -42, 3190, 1983 },
{   EPS_MSID_CHOKEI_3,      1701, 3685, 1617, 3247, -36, -42, 1773, 3797 },
{   EPS_MSID_CHOKEI_4,      1276, 3161, 1192, 2822, -36, -42, 1348, 3273 },
{   EPS_MSID_YOKEI_1,       1701, 2494, 1617, 2410, -36, -42, 1773, 2606 },
{   EPS_MSID_YOKEI_2,       1616, 2296, 1532, 2212, -36, -42, 1688, 2408 },
{   EPS_MSID_YOKEI_3,       1389, 2098, 1305, 2014, -36, -42, 1461, 2210 },
{   EPS_MSID_YOKEI_4,       1488, 3331, 1404, 3247, -36, -42, 1560, 3443 },
{   EPS_MSID_2L,            1800, 2522, 1716, 2436, -36, -42, 1872, 2634 },
{   EPS_MSID_ENV_10_P,      1485, 3420, 1401, 3336, -36, -42, 1557, 3532 },
{   EPS_MSID_ENV_C6_P,      1616, 2296, 1532, 2212, -36, -42, 1688, 2408 },
{   EPS_MSID_ENV_DL_P,      1559, 3118, 1475, 3034, -36, -42, 1631, 3230 },
{   EPS_MSID_NEWENV_P,      1871, 3118, 1787, 3034, -36, -42, 1943, 3230 },
{   EPS_MSID_MEISHI,        1261,  779, 1177,  695, -36, -42, 1333,  891 },
{   EPS_MSID_BUZCARD_89X50, 1261,  709, 1177,  625, -36, -42, 1333,  821 },
{   EPS_MSID_CARD_54X86,     765, 1219,  681, 1135, -36, -42,  837, 1331 },
{   EPS_MSID_BUZCARD_55X91,  780, 1290,  696, 1206, -36, -42,  852, 1402 },
{   EPS_MSID_ALBUM_L,       1800, 2607, 1716, 2523, -36, -42, 1872, 2719 },
{   EPS_MSID_ALBUM_A5,      2976, 4294, 2892, 4210, -36, -42, 3048, 4406 },
{   EPS_MSID_PALBUM_L_L,    1800, 1260, 1716, 1176, -36, -42, 1872, 1372 },
{   EPS_MSID_PALBUM_2L,     1800, 2521, 1716, 2437, -36, -42, 1872, 2633 },
{   EPS_MSID_PALBUM_A5_L,   2976, 2101, 2892, 2017, -36, -42, 3048, 2213 },
{   EPS_MSID_PALBUM_A4,     2976, 4203, 2892, 4119, -36, -42, 3048, 4315 },
{   EPS_MSID_HIVISION,      1440, 2560, 1356, 2476, -36, -42, 1512, 2672 },
{   EPS_MSID_A3NOBI,        4663, 6846, 4578, 6761, -48, -42, 4759, 6958 },
{   EPS_MSID_A3,            4209, 5953, 4125, 5868, -48, -42, 4305, 6065 },
{   EPS_MSID_B4,            3643, 5159, 3559, 5075, -36, -42, 3715, 5271 },
{   EPS_MSID_USB,           3960, 6120, 3876, 6036, -36, -42, 4032, 6232 },
{   EPS_MSID_11X14,         3960, 5040, 3876, 4956, -36, -42, 4032, 5152 },
{   EPS_MSID_B3,            5159, 7285, 5075, 7201, -48, -42, 5255, 7397 },
{   EPS_MSID_A2,            5953, 8419, 5869, 8335, -48, -42, 6049, 8531 },
{   EPS_MSID_USC,           6120, 7920, 6036, 7836, -48, -42, 6216, 8032 },
{   EPS_MSID_10X12,         3600, 4320, 3516, 4236, -36, -42, 3672, 4432 },
{   EPS_MSID_12X12,         4320, 4320, 4236, 4236, -36, -42, 4392, 4432 },
{   EPS_MSID_GLT,            2880, 3780, 2796, 3696, -36, -42, 2952, 3892},
{   EPS_MSID_GLG,            3060, 4680, 2976, 4596, -36, -42, 3132, 4792},
{   EPS_MSID_USER,              0,   0,    0,    0, -36, -42,    0,    0 },
{   -1,                         0,   0,    0,    0,   0,   0,    0,    0 }
};

#define EPS_NUM_MEDIA_SIZES     sizeof(epsMediaSize)/sizeof(EPS_MEDIA_INFO)


        /*** Media Size Information (300dpi)                                            */
        /*** ---------------------------------------------------------------------------*/
static const EPS_MEDIA_INFO epsMediaSize300[] = {
/*          ID              PaperW/L    3mm W/L     0mm L/T   0mm W/L   */
{   EPS_MSID_A4,            2480, 3507, 2410, 3437, -30, -35, 2540, 3600 }, 
{   EPS_MSID_LETTER,        2550, 3300, 2480, 3230, -30, -35, 2610, 3393 }, 
{   EPS_MSID_LEGAL,         2550, 4200, 2480, 4130, -30, -35, 2610, 4293 }, 
{   EPS_MSID_A5,            1748, 2480, 1678, 2410, -30, -35, 1808, 2573 }, 
{   EPS_MSID_A6,            1240, 1748, 1170, 1678, -30, -35, 1300, 1841 }, 
{   EPS_MSID_B5,            2149, 3035, 2079, 2965, -30, -35, 2209, 3128 }, 
{   EPS_MSID_EXECUTIVE,     2175, 3150, 2105, 3080, -30, -35, 2235, 3243 }, 
{   EPS_MSID_HALFLETTER,    1650, 2550, 1580, 2480, -30, -35, 1710, 2643 }, 
{   EPS_MSID_PANORAMIC,     2480, 7016, 2410, 6946, -30, -35, 2540, 7109 }, 
{   EPS_MSID_TRIM_4X6,      1342, 1942, 1272, 1872, -30, -35, 1402, 2035 }, 
{   EPS_MSID_4X6,           1200, 1800, 1130, 1730, -30, -35, 1260, 1893 }, 
{   EPS_MSID_5X8,           1500, 2400, 1430, 2330, -30, -35, 1560, 2493 }, 
{   EPS_MSID_8X10,          2400, 3000, 2330, 2930, -30, -35, 2460, 3093 }, 
{   EPS_MSID_10X15,         1181, 1771, 1111, 1701, -30, -35, 1241, 1864 }, 
{	EPS_MSID_F4,    		2480, 3898, 2410, 3828, -30, -35, 2540, 3991 },
{   EPS_MSID_ENVY0,         1417, 2776, 1347, 2706, -36, -42, 1477, 2869 },
{   EPS_MSID_ENVY6,         1157, 2244, 1087, 2174, -36, -42, 1217, 2337 },
{   EPS_MSID_ENVK2,         2835, 3921, 2765, 3851, -36, -42, 2895, 4014 },
{	EPS_MSID_4PC, 		    2362, 3496, 2292, 3426, -30, -35, 2422, 3589 },
{   EPS_MSID_200X300,       2551, 3992, 2481, 3922, -30, -35, 2611, 4085 },
{   EPS_MSID_L,             1050, 1500,  980, 1430, -30, -35, 1110, 1593 }, 
{   EPS_MSID_POSTCARD,      1181, 1748, 1111, 1678, -30, -35, 1241, 1841 }, 
{   EPS_MSID_DBLPOSTCARD,   2363, 1748, 2293, 1678, -30, -35, 2423, 1841 }, 
{   EPS_MSID_ENV_10_L,      2850, 1238, 2780, 1168, -30, -35, 2910, 1331 }, 
{   EPS_MSID_ENV_C6_L,      1913, 1347, 1843, 1277, -30, -35, 1973, 1440 }, 
{   EPS_MSID_ENV_DL_L,      2598, 1299, 2528, 1229, -30, -35, 2658, 1392 }, 
{   EPS_MSID_NEWEVN_L,      2598, 1559, 2528, 1489, -30, -35, 2658, 1652 }, 
{   EPS_MSID_CHOKEI_3,      1418, 3071, 1348, 3001, -30, -35, 1478, 3164 }, 
{   EPS_MSID_CHOKEI_4,      1063, 2634,  993, 2564, -30, -35, 1123, 2727 }, 
{   EPS_MSID_YOKEI_1,       1418, 2078, 1348, 2008, -30, -35, 1478, 2171 }, 
{   EPS_MSID_YOKEI_2,       1347, 1913, 1277, 1843, -30, -35, 1407, 2006 }, 
{   EPS_MSID_YOKEI_3,       1158, 1748, 1088, 1678, -30, -35, 1218, 1841 }, 
{   EPS_MSID_YOKEI_4,       1240, 2776, 1170, 2706, -30, -35, 1300, 2869 }, 
{   EPS_MSID_2L,            1500, 2100, 1430, 2030, -30, -35, 1560, 2193 }, 
{   EPS_MSID_ENV_10_P,      1238, 2850, 1168, 2780, -30, -35, 1298, 2943 }, 
{   EPS_MSID_ENV_C6_P,      1347, 1913, 1277, 1843, -30, -35, 1407, 2006 }, 
{   EPS_MSID_ENV_DL_P,      1299, 2598, 1229, 2528, -30, -35, 1359, 2691 }, 
{   EPS_MSID_NEWENV_P,      1559, 2598, 1489, 2528, -30, -35, 1619, 2691 }, 
{   EPS_MSID_MEISHI,        1051,  649,  981,  579, -30, -35, 1111,  742 }, 
{   EPS_MSID_BUZCARD_89X50, 1051,  591,  981,  521, -30, -35, 1111,  684 }, 
{   EPS_MSID_CARD_54X86,     638, 1016,  568,  946, -30, -35,  698, 1109 }, 
{   EPS_MSID_BUZCARD_55X91,  650, 1075,  580, 1005, -30, -35,  710, 1168 }, 
{   EPS_MSID_ALBUM_L,       1500, 2173, 1430, 2103, -30, -35, 1560, 2266 }, 
{   EPS_MSID_ALBUM_A5,      2480, 3578, 2410, 3508, -30, -35, 2540, 3671 }, 
{   EPS_MSID_PALBUM_L_L,    1500, 1050, 1430,  980, -30, -35, 1560, 1143 }, 
{   EPS_MSID_PALBUM_2L,     1500, 2101, 1430, 2031, -30, -35, 1560, 2194 }, 
{   EPS_MSID_PALBUM_A5_L,   2480, 1751, 2410, 1681, -30, -35, 2540, 1844 }, 
{   EPS_MSID_PALBUM_A4,     2480, 3503, 2410, 3433, -30, -35, 2540, 3596 }, 
{   EPS_MSID_HIVISION,      1200, 2133, 1130, 2063, -30, -35, 1260, 2226 }, 
{   EPS_MSID_A3NOBI,        3886, 5705, 3816, 5635, -40, -35, 3946, 5798 }, 
{   EPS_MSID_A3,            3507, 4960, 3437, 4890, -40, -35, 3567, 5053 }, 
{   EPS_MSID_B4,            3036, 4299, 2966, 4229, -30, -35, 3096, 4392 }, 
{   EPS_MSID_USB,           3300, 5100, 3230, 5030, -30, -35, 3360, 5193 }, 
{   EPS_MSID_11X14,         3300, 4200, 3230, 4130, -30, -35, 3360, 4293 }, 
{   EPS_MSID_B3,            4299, 6071, 4229, 6001, -40, -35, 4359, 6164 }, 
{   EPS_MSID_A2,            4961, 7016, 4891, 6946, -40, -35, 5021, 7109 }, 
{   EPS_MSID_USC,           5100, 6600, 5030, 6530, -40, -35, 5160, 6693 }, 
{   EPS_MSID_10X12,         3000, 3600, 2930, 3530, -30, -35, 3060, 3693 }, 
{   EPS_MSID_12X12,         3600, 3600, 3530, 3530, -30, -35, 3660, 3693 }, 
{   EPS_MSID_GLT,            2400, 3150, 2330, 3080, -30, -35, 2460, 3243 },
{   EPS_MSID_GLG,           2550, 3900, 2480, 3830, -30, -35, 2610, 3993 },
{   EPS_MSID_USER,             0,    0,    0,    0, -30, -35,    0,    0 },
{   -1,                         0,   0,    0,    0,   0,   0,    0,    0 }
};

#define EPS_BORDERS_MARGIN_360	(42)
/*#define EPS_BORDERS_MARGIN_720	(84)*/
#define EPS_BORDERS_MARGIN_300	(35)
/*#define EPS_BORDERS_MARGIN_600	(70)*/


/*-------------  Definition of Media Type ID / Global Media Type Declarations   --------*/
/*******************************************|********************************************/
        /*** Media Type ID                                                              */
        /*** ---------------------------------------------------------------------------*/
typedef enum _tagMEDIA_TYPE_ID_ {
                                /* Dec  Hex                Text Name                        */
    EPS_MTID_PLAIN          =0, /*  0  0x00,  "Plain Paper\0"                               */
    EPS_MTID_360INKJET        , /*  1  0x01,  "360dpi Ink Jet Paper \0"                     */
    EPS_MTID_IRON             , /*  2  0x02,  "Iron-On Cool Peel Transfer Paper\0"          */
    EPS_MTID_PHOTOINKJET      , /*  3  0x03,  "Photo Qaulity Ink Jet Paper #2\0"            */
    EPS_MTID_PHOTOADSHEET     , /*  4  0x04,  "Photo Qaulity Self Adhesive Sheets\0"        */
    EPS_MTID_MATTE            , /*  5  0x05,  "Mattee Paper-HeavyWeight\0"                  */
    EPS_MTID_PHOTO            , /*  6  0x06,  "Photo Paper\0"                               */
    EPS_MTID_PHOTOFILM        , /*  7  0x07,  "Photo Quality Glossy Flim\0"                 */
    EPS_MTID_MINIPHOTO        , /*  8  0x08,  "Photo Stickers 4/6\0"                        */
    EPS_MTID_OHP              , /*  9  0x09,  "Ink Jet Transparencies\0                     */
    EPS_MTID_BACKLIGHT        , /* 10  0x0A,  "Back Light Film\0"                           */
    EPS_MTID_PGPHOTO          , /* 11  0x0B,  "Premium Glossy Photo Paper\0"                */
    EPS_MTID_PSPHOTO          , /* 12  0x0C,  "Premium Semigloss Photo Paper\0"             */
    EPS_MTID_PLPHOTO          , /* 13  0x0D,  "Premimum Luster Photo Paper\0"               */
    EPS_MTID_MCGLOSSY         , /* 14  0x0E,  "Glossy Paper - Photo Weight\0"               */
    EPS_MTID_ARCHMATTE        , /* 15  0xOF,  "Archival Matte Paper\0"                      */
    EPS_MTID_WATERCOLOR       , /* 16  0x10,  "Water Paper-Radiant White\0"                 */
    EPS_MTID_PROGLOSS         , /* 17  0x11,  "Professional Glossy Paper\0"                 */
    EPS_MTID_MATTEBOARD       , /* 18  0x12,  "Matte Board Paper\0"                         */
    EPS_MTID_PHOTOGLOSS       , /* 19  0x13,  "Photo Quality Glossy Paper\0"                */
    EPS_MTID_SEMIPROOF        , /* 20  0x14,  "Dupont/EPSON Semigloss Proofing Paper-A\0"   */
    EPS_MTID_SUPERFINE2       , /* 21  0x15,  "Super Fine Paper -2\0"                       */
    EPS_MTID_DSMATTE          , /* 22  0x16,  "Double Sided Matte Paper\0"                  */
    EPS_MTID_CLPHOTO          , /* 23  0x17,  "ColorLife Photo Paper\0"                     */
    EPS_MTID_ECOPHOTO         , /* 24  0x18,  "Economy Photo Paper\0"                       */
    EPS_MTID_VELVETFINEART    , /* 25  0x19,  "Velvet Fine Art Paper\0"                     */
    EPS_MTID_PROOFSEMI        , /* 26  0x1A,  "EPSON Proofing Paper Simimatte\0"            */
    EPS_MTID_HAGAKIRECL       , /* 27  0x1B,  "Post Card - Recycled Paper\0"                */
    EPS_MTID_HAGAKIINKJET     , /* 28  0x1C,  "Post Card - Ink Jet Paper\0"                 */
    EPS_MTID_PHOTOINKJET2     , /* 29  0x1D,  "Photo Quality Ink Jet Paper\0"               */
    EPS_MTID_DURABRITE        , /* 30  0x1E,  "DURABrite Ink Glossy Photo Paper\0"          */
    EPS_MTID_MATTEMEISHI      , /* 31  0x1F,  "Matte Meishi Card\0"                         */
    EPS_MTID_HAGAKIATENA      , /* 32  0x20,  "Post Card - Address Side\0"                  */
    EPS_MTID_PHOTOALBUM       , /* 33  0x21,  "Photo Album Kit\0"                           */
    EPS_MTID_PHOTOSTAND       , /* 34  0x22,  "Photo Stand Kit\0"                           */
    EPS_MTID_RCB              , /* 35  0x23,  "RC-B\0"                                      */
    EPS_MTID_PGPHOTOEG        , /* 36  0x24,  "Premimum Glossy Photo Paper - EG\0"          */
    EPS_MTID_ENVELOPE         , /* 37  0x25,  "Envelope\0"                                  */
    EPS_MTID_PLATINA          , /* 38  0x26,  "Ultra Glossy Photo Paper\0"                  */
    EPS_MTID_ULTRASMOOTH      , /* 39  0x27,  "Ultra Smooth Fine Art Paper\0"               */
    EPS_MTID_SFHAGAKI         , /* 40  0x28,  "Super Fine Postcard\0"                       */
    EPS_MTID_PHOTOSTD         , /* 41  0x29,  "Premium Glossy Photo Paper (Standard)\0"     */
    EPS_MTID_GLOSSYHAGAKI     , /* 42  0x2A,  "Glossy Postcard\0"                           */
    EPS_MTID_GLOSSYPHOTO      , /* 43  0x2B,  "Glossy Photo Paper\0"                        */
    EPS_MTID_GLOSSYCAST	      , /* 44  0x2C,  "Epson Photo\0"							    */
    EPS_MTID_LETTERHEAD	      , //lam dai hiep them
    EPS_MTID_RECYCLED	      , //lam dai hiep them
    EPS_MTID_COLORPAPER	      , //lam dai hiep them
    EPS_MTID_LABEL	      , //lam dai hiep them
    EPS_MTID_THICK	      , //lam dai hiep them
    EPS_MTID_EXTHICK	      , //lam dai hiep them
    EPS_MTID_COATED	      , //lam dai hiep them
    EPS_MTID_SPECIAL	      , //lam dai hiep them
    EPS_MTID_BUSINESSCOAT     , /* 45  0x2D,  "Business Ink Jet Coat Paper\0"			    */
    EPS_MTID_CDDVD         =91, /* 91  0x5B,  "CD/DVD Label\0"                              */
    EPS_MTID_CDDVDHIGH        , /* 92  0x5C,  "CD/DVD High Quality Label\0"                 */
    EPS_MTID_CDDVDGLOSSY      , /* 93  0x5D,  "CD/DVD Glossy Surface\0"                     */
    EPS_MTID_CLEANING      =99, /* 99  0x63,  "Cleaning Paper\0"                            */
    EPS_MTID_UNKNOWN       =0xff/* unknown                                                  */
} MEDIA_TYPE_ID;


        /*** Media Type Index                                                           */
        /*** ---------------------------------------------------------------------------*/
static const EPS_UINT32 epsMediaTypeIndex[] = {
                                /* Dec  Hex                Text Name                        */
    EPS_MTID_PLAIN            , /*  0  0x00,  "Plain Paper\0"                               */
    EPS_MTID_360INKJET        , /*  1  0x01,  "360dpi Ink Jet Paper \0"                     */
    EPS_MTID_IRON             , /*  2  0x02,  "Iron-On Cool Peel Transfer Paper\0"          */
    EPS_MTID_PHOTOINKJET      , /*  3  0x03,  "Photo Qaulity Ink Jet Paper #2\0"            */
    EPS_MTID_PHOTOADSHEET     , /*  4  0x04,  "Photo Qaulity Self Adhesive Sheets\0"        */
    EPS_MTID_MATTE            , /*  5  0x05,  "Mattee Paper-HeavyWeight\0"                  */
    EPS_MTID_PHOTO            , /*  6  0x06,  "Photo Paper\0"                               */
    EPS_MTID_PHOTOFILM        , /*  7  0x07,  "Photo Quality Glossy Flim\0"                 */
    EPS_MTID_MINIPHOTO        , /*  8  0x08,  "Photo Stickers 4/6\0"                        */
    EPS_MTID_OHP              , /*  9  0x09,  "Ink Jet Transparencies\0                     */
    EPS_MTID_BACKLIGHT        , /* 10  0x0A,  "Back Light Film\0"                           */
    EPS_MTID_PGPHOTO          , /* 11  0x0B,  "Premium Glossy Photo Paper\0"                */
    EPS_MTID_PSPHOTO          , /* 12  0x0C,  "Premium Semigloss Photo Paper\0"             */
    EPS_MTID_PLPHOTO          , /* 13  0x0D,  "Premimum Luster Photo Paper\0"               */
    EPS_MTID_MCGLOSSY         , /* 14  0x0E,  "Glossy Paper - Photo Weight\0"               */
    EPS_MTID_ARCHMATTE        , /* 15  0xOF,  "Archival Matte Paper\0"                      */
    EPS_MTID_WATERCOLOR       , /* 16  0x10,  "Water Paper-Radiant White\0"                 */
    EPS_MTID_PROGLOSS         , /* 17  0x11,  "Professional Glossy Paper\0"                 */
    EPS_MTID_MATTEBOARD       , /* 18  0x12,  "Matte Board Paper\0"                         */
    EPS_MTID_PHOTOGLOSS       , /* 19  0x13,  "Photo Quality Glossy Paper\0"                */
    EPS_MTID_SEMIPROOF        , /* 20  0x14,  "Dupont/EPSON Semigloss Proofing Paper-A\0"   */
    EPS_MTID_SUPERFINE2       , /* 21  0x15,  "Super Fine Paper -2\0"                       */
    EPS_MTID_DSMATTE          , /* 22  0x16,  "Double Sided Matte Paper\0"                  */
    EPS_MTID_CLPHOTO          , /* 23  0x17,  "ColorLife Photo Paper\0"                     */
    EPS_MTID_ECOPHOTO         , /* 24  0x18,  "Economy Photo Paper\0"                       */
    EPS_MTID_VELVETFINEART    , /* 25  0x19,  "Velvet Fine Art Paper\0"                     */
    EPS_MTID_PROOFSEMI        , /* 26  0x1A,  "EPSON Proofing Paper Simimatte\0"            */
    EPS_MTID_HAGAKIRECL       , /* 27  0x1B,  "Post Card - Recycled Paper\0"                */
    EPS_MTID_HAGAKIINKJET     , /* 28  0x1C,  "Post Card - Ink Jet Paper\0"                 */
    EPS_MTID_PHOTOINKJET2     , /* 29  0x1D,  "Photo Quality Ink Jet Paper\0"               */
    EPS_MTID_DURABRITE        , /* 30  0x1E,  "DURABrite Ink Glossy Photo Paper\0"          */
    EPS_MTID_MATTEMEISHI      , /* 31  0x1F,  "Matte Meishi Card\0"                         */
    EPS_MTID_HAGAKIATENA      , /* 32  0x20,  "Post Card - Address Side\0"                  */
    EPS_MTID_PHOTOALBUM       , /* 33  0x21,  "Photo Album Kit\0"                           */
    EPS_MTID_PHOTOSTAND       , /* 34  0x22,  "Photo Stand Kit\0"                           */
    EPS_MTID_RCB              , /* 35  0x23,  "RC-B\0"                                      */
    EPS_MTID_PGPHOTOEG        , /* 36  0x24,  "Premimum Glossy Photo Paper - EG\0"          */
    EPS_MTID_ENVELOPE         , /* 37  0x25,  "Envelope\0"                                  */
    EPS_MTID_PLATINA          , /* 38  0x26,  "Ultra Glossy Photo Paper\0"                  */
    EPS_MTID_ULTRASMOOTH      , /* 39  0x27,  "Ultra Smooth Fine Art Paper\0"               */
    EPS_MTID_SFHAGAKI         , /* 40  0x28,  "Super Fine Postcard\0"                       */
    EPS_MTID_PHOTOSTD         , /* 41  0x29,  "Premium Glossy Photo Paper (Standard)\0"     */
    EPS_MTID_GLOSSYHAGAKI     , /* 42  0x2A,  "Glossy Postcard\0"                           */
    EPS_MTID_GLOSSYPHOTO      , /* 43  0x2B,  "Glossy Photo Paper\0"                        */
    EPS_MTID_GLOSSYCAST	      , /* 44  0x2C,  "Epson Photo\0"							    */
	EPS_MTID_BUSINESSCOAT     , /* 45  0x2D,  "Business Ink Jet Coat Paper\0"				*/
    EPS_MTID_CDDVD            , /* 91  0x5B,  "CD/DVD Label\0"                              */
    EPS_MTID_CDDVDHIGH        , /* 92  0x5C,  "CD/DVD High Quality Label\0"                 */
    EPS_MTID_CDDVDGLOSSY      , /* 93  0x5D,  "CD/DVD Glossy Surface\0"                     */
    EPS_MTID_CLEANING         , /* 99  0x63,  "Cleaning Paper\0"                            */
/*  EPS_MTID_UNKNOWN   not include this list                                                */
};

#define EPS_NUM_MEDIA_TYPES     sizeof(epsMediaTypeIndex)/sizeof(EPS_UINT32)


    /*** LAYOUT                                                                         */
    /*** -------------------------------------------------------------------------------*/
#define	EPS_MLID_CUSTOM 	0x00			/* User-defined margins                     */
#define	EPS_MLID_BORDERLESS	0x01			/* 0mm                                      */
#define	EPS_MLID_BORDERS	0x02			/* 3mm                                      */
#define	EPS_MLID_CDLABEL	0x04			/* CD/DVD Label                             */
#define	EPS_MLID_DIVIDE16	0x08			/* 16 Division (for mini phot sheel)        */


    /*** Print Quality                                                                  */
    /*** -------------------------------------------------------------------------------*/
#define EPS_MQID_UNKNOWN    0x00            /* invalid type                             */
#define	EPS_MQID_DRAFT		0x01
#define	EPS_MQID_NORMAL		0x02
#define	EPS_MQID_HIGH		0x04
#define	EPS_MQID_ALL		(EPS_MQID_DRAFT | EPS_MQID_NORMAL | EPS_MQID_HIGH)


	/*** Paper Source                                                                   */
    /*** -------------------------------------------------------------------------------*/
#define	EPS_MPID_NOT_SPEC	0x00
#define	EPS_MPID_AUTO		EPS_MPID_NOT_SPEC
#define	EPS_MPID_REAR		0x01
#define	EPS_MPID_FRONT1		0x02
#define	EPS_MPID_FRONT2		0x04
#define	EPS_MPID_CDTRAY		0x08
//printer PAGE-S
#define	EPS_MPID_PAGE_S_MP_TRAY				0x00
#define	EPS_MPID_PAGE_S_CASSETTE1				0x01
#define	EPS_MPID_PAGE_S_CASSETTE2				0x02
#define	EPS_MPID_PAGE_S_CASSETTE3				0x03
#define	EPS_MPID_PAGE_S_CASSETTE4				0x04
#define	EPS_MPID_PAGE_S_PARAM_MANUALFEED				0x0A
#define	EPS_MPID_PAGE_S_AUTO_TRAY				0xFF


/* append for page printer */
#define	EPS_MPID_MPTRAY		0x100
#define	EPS_MPID_FRONT3		0x200
#define	EPS_MPID_FRONT4		0x400
#define	EPS_MPID_ALL_ESCPR	(EPS_MPID_REAR | EPS_MPID_FRONT1 | EPS_MPID_FRONT2 | EPS_MPID_CDTRAY)
#define	EPS_MPID_ALL_ESCPAGE (EPS_MPID_MPTRAY | EPS_MPID_FRONT1 | EPS_MPID_FRONT2 | EPS_MPID_FRONT3 | EPS_MPID_FRONT4)


#define	EPS_IS_CDDVD( t )					\
		(  EPS_MTID_CDDVD       == t		\
		|| EPS_MTID_CDDVDHIGH   == t		\
		|| EPS_MTID_CDDVDGLOSSY == t )


	/*** duplex                                                                         */
    /*** -------------------------------------------------------------------------------*/
#define	EPS_DUPLEX_DISABLE	0x00
#define	EPS_DUPLEX_ENABLE	0x01


#ifdef __cplusplus
}
#endif

#endif  /* def __EPSON_ESCPR_MEDIA_H__ */

/*_______________________________  epson-escpr-media.h   _______________________________*/
  
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

