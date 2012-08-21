
#ifndef PIPS_H
#define PIPS_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "pipsDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	
#define END_ARRAY -1			
#define ENTRY_MAX 48	/* 30 -> 48 用紙種類の増加にともない48に更新 */
	
#define N_OPT 12
#define EX_N_OPT 15
	
	/* option's ID */
#define P_SCALE            0
#define P_INK              1
#define P_MEDIA_SIZE       2
#define P_MEDIA_TYPE       3
#define P_BRIGHTNESS       4
#define P_CONTRAST         5
#define P_SATURATION       6
#define P_QUALITY_LEVEL    7
#define P_MARGIN           8   /* add Mon Aug 28 2000 sk */
#define P_FIT_PAGE         9   /* add Tue Oct  2 2001 sk */
#define P_MULTI_PAGE       10   /* add Tue Oct  2 2001 sk */
#define P_BORDERLESS_PRINT 11

#define P_MARGIN_X         12
#define P_MARGIN_Y         13

#define P_OUTPUT_EX        14


/* Action modes */
#define ACTION_MODE_GUI 0
#define ACTION_MODE_CUI 1
#define ACTION_MODE_GET_PRINTER_ID 2


	/* label call type    -chenged Mon Aug 28 2000 sk */
	enum nBASE_CALL_TYPE
	{
		CALL_FOR_RSC_NAME = 0,
		CALL_FOR_X_NAME,
		CALL_FOR_VALUE
	};

#define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)

	typedef struct tagPIPS_INIT_PARAM
	{
		int action_mode;
	} PIPS_INIT_PARAM;

	typedef struct tagSEP_INIT_PARAM
	{
		short paper_id;
		int prt_format;
		int media_type;
		int brightness;
		int contrast;
		int saturation;
		int mag;
		int qlevel;
#ifdef CONV_VERSION2
		POINT margin;		/* add Mon Aug 28 2000 sk */
#endif /* CONV_VERSION2 */
		int fit_page;
		int multi_page;
		int borderless_print;
	} SEP_INIT_PARAM;
  
	typedef struct _OPTION_DATA
	{
		int value;
		char *rsc_name;
		char *x_name;
		char *x_name_jp;
	} OPTION_DATA;

	typedef struct _OPTION_ALL
	{
		int id;
		OPTION_DATA *data;
		char *x_name;
		char *x_name_jp;    
	} OPTION_ALL;
	
	int xMode (int, char*[], char*, SEP_INIT_PARAM*, PIPS_INIT_PARAM*);
	int cMode (char *, SEP_INIT_PARAM *, PIPS_INIT_PARAM *);
	int printLoop (char*, FILE*, SEP_INIT_PARAM*);
	OPTION_DATA* databasesCall (int, int, void*);
	int printModeCheck (const short*, int, SEP_INIT_PARAM*);
	int getValueOfDatabases (int, char*);
	char* getRscName (int, int);
	OPTION_DATA* getOptData(int,int);
  
	int getRsc (SEP_INIT_PARAM*, const char*);
	void saveRsc (void*[], const char*);
	int getOpts (int, char*[], char **, SEP_INIT_PARAM *, PIPS_INIT_PARAM *pip);
	int getOptsForX (void*[], SEP_INIT_PARAM*);
	int fileLoad (FILE*, FILE*, POINT*, UCHAR**);
	void confWinInit (int*, char**[], char*, SEP_INIT_PARAM*, PIPS_INIT_PARAM*);
	void confWinFinalize (void);
	int confWinShow (SEP_INIT_PARAM*, const char*);

	int load_printer_info(char*);
	int update_resource (char*);
	int free_resource ();
	int load_printer_option (char*);
	int load_dummy_resource ();
	int borderless_support(char*);

	int load_default_printer_info();
	int get_default_printer_id();
#ifdef CONV_VERSION2
	int
	graphics_converter_version2 (SEP_INIT_PARAM *, char*, char*);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PIPS_H */
