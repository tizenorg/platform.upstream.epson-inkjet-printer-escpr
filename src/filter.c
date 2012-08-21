
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#define HAVE_PPM (0)

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>



#include "epson-escpr-def.h"
#include "epson-escpr-media.h"
#include "epson-protocol.h"
#include "epson-escpr-pvt.h"
#include "epson-escpr-api.h"
#include "epson-escpr-services.h"
#include "epson-escpr-mem.h"
#include <signal.h>

#include "err.h"
#include "mem.h"
#include "str.h"
#include "csv.h"
#include "debug.h"
#include "libprtX.h"
#include "optBase.h"
#include "linux_cmn.h"



#define PATH_MAX 256
#define NAME_MAX 41

#define WIDTH_BYTES(bits) (((bits) + 31) / 32 * 4)

#define PIPSLITE_FILTER_VERSION "* epson-escpr is a part of " PACKAGE_STRING

#define PIPSLITE_FILTER_USAGE "Usage: $ epson-escpr model width_pixel height_pixel Ink PageSize Quality"

typedef struct rtp_filter_option {
	char model[NAME_MAX];
	char model_low[NAME_MAX];
	char ink[NAME_MAX];
	char media[NAME_MAX];
	char quality[NAME_MAX];
	char duplex[NAME_MAX];
} filter_option_t;


FILE* outfp = NULL;

/* static functions */
static int set_pips_parameter (filter_option_t *, ESCPR_OPT *, ESCPR_PRINT_QUALITY *);
static int get_page_size (const char *, const char *);
static int getMediaTypeID(char *);
static ESCPR_BYTE4 print_spool_fnc(void* , const ESCPR_UBYTE1*, ESCPR_UBYTE4);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
const int band_line = 1;
extern EPS_INT32    libStatus;                  /*  Library (epsInitDriver) status      */
extern EPS_PRINT_JOB   printJob;
extern EPS_UINT32   sendDataBufSize;
extern EPS_UINT8*   sendDataBuf;    /* buffer of SendCommand(save) input                */
extern EPS_UINT32   tmpLineBufSize;
extern EPS_UINT8*   tmpLineBuf;

extern EPS_CMN_FUNC epsCmnFnc;
EPS_JOB_FUNCS		jobFnc;

extern EPS_INT32 tonerSave;
extern EPS_INT32 back_type;
extern EPS_INT32 lWidth;
extern EPS_INT32 lHeight;
extern EPS_INT32 areaWidth;
extern EPS_INT32 areaHeight;

#ifndef ESCPR_HEADER_LENGTH	
#define ESCPR_HEADER_LENGTH            10    /* ESC + CLASS + ParamLen + CmdName */	
#endif

#ifndef ESCPR_SEND_DATA_LENGTH
#define ESCPR_SEND_DATA_LENGTH          7
#endif

EPS_JOB_ATTRIB     jobAttr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////////////////
EPS_ERR_CODE epsInitLib(){
	EPS_CMN_FUNC cmnFuncPtrs;
	memset(&cmnFuncPtrs, 0, sizeof(EPS_CMN_FUNC));
	
	cmnFuncPtrs.version = EPS_CMNFUNC_VER_CUR;
	cmnFuncPtrs.findCallback = NULL;
	cmnFuncPtrs.memAlloc = &epsmpMemAlloc;
	cmnFuncPtrs.memFree = &epsmpMemFree;
	cmnFuncPtrs.sleep = &epsmpSleep;
	cmnFuncPtrs.getTime = &epsmpGetTime;
	cmnFuncPtrs.getLocalTime = &epsmpGetLocalTime;
	cmnFuncPtrs.lockSync = &epsmpLockSync;
	cmnFuncPtrs.unlockSync = &epsmpUnlockSync;
	cmnFuncPtrs.stateCallback = NULL; /* current version unused */

	memcpy((void*)(&epsCmnFnc), (void*)&cmnFuncPtrs, sizeof(EPS_CMN_FUNC));
}

EPS_ERR_CODE epsInitJob(){
	
	memset(&printJob, 0, sizeof(EPS_PRINT_JOB));
	printJob.printer = (EPS_PRINTER_INN*) malloc(sizeof(EPS_PRINTER_INN));
	memset(printJob.printer, 0, sizeof(EPS_PRINTER_INN));
	
	printJob.jobStatus  = EPS_STATUS_NOT_INITIALIZED;
	printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.findStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.bComm		= TRUE;	
	printJob.platform   = 0x04; /* '0x04 = linux' is default */ 

	libStatus   = EPS_STATUS_NOT_INITIALIZED;
	sendDataBufSize = 0;
	sendDataBuf = NULL;
	tmpLineBufSize = 0;
	tmpLineBuf  = NULL;

/*** Initialize continue buffer                                                         */
	printJob.contData.sendData = NULL;
	printJob.contData.sendDataSize = 0;

	/* DEL printJob.additional = EPS_ADDDATA_NONE;
	printJob.qrcode.bits = NULL;
	printJob.qrcode.cellNum = 0;
	printJob.qrcode.dpc = 0; */
	obsClear();

/*** Set "Endian-ness" for the current cpu                                              */
	memInspectEndian();
	
/*** Set Communication Mode                                                             */
	//printJob.commMode = commMode;
	
/*** Change ESC/P-R Lib Status                                                          */
	libStatus = EPS_STATUS_INITIALIZED;
	EPS_PRINTER_INN*    printer = printJob.printer;
	printer->pmData.state = EPS_PM_STATE_NOT_FILTERED;
	printer->language = EPS_LANG_ESCPR;
	return EPS_ERR_NONE;
}


EPS_ERR_CODE epsInitVariable(){
	sendDataBufSize = (EPS_INT32)(ESCPR_HEADER_LENGTH    +
									ESCPR_SEND_DATA_LENGTH +
									(printJob.printableAreaWidth * printJob.bpp));
	sendDataBuf = (EPS_UINT8*)EPS_ALLOC(sendDataBufSize);
	if(sendDataBuf == NULL){
		sendDataBufSize = 0;
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	
	memset(sendDataBuf, 0xFF, (EPS_UINT32)sendDataBufSize);
	/*** Allocate buffer for RLE complession                                        */
	tmpLineBufSize = (EPS_INT32)(printJob.printableAreaWidth * printJob.bpp) + 256; /* 256 is temp buffer */
	tmpLineBuf = (EPS_UINT8*)EPS_ALLOC(tmpLineBufSize);
	if(tmpLineBuf == NULL){
		tmpLineBufSize = 0;
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	memset(tmpLineBuf, 0xFF, (EPS_UINT32)tmpLineBufSize);
	return EPS_ERR_NONE;
}


EPS_ERR_CODE epsFilterEndPage(EPS_BOOL bNextPage){
	EPS_ERR_CODE    retStatus= EPS_ERR_NONE;
	
#ifdef GCOMSW_CMD_ESCPAGE_S
	debug_msg("GCOMSW_CMD_ESCPAGE defined\n");
		retStatus = pageEndPage();
#else
		retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif

	return retStatus;

}

void eps_toupper(char *str){
	int i = 0;
	int len;
	if(str == NULL || strlen(str) == 0){
		return;
	}
	len = strlen(str);
	for(i = 0; i < len; i++){
		str[i] = toupper(str[i]);
	}
	return;
}

static int  getMediaSizeID(char *media_name){
  int j;
	debug_msg("media name = %s\n", media_name);
  for(j = 0; mediaSizeData[j].value != END_ARRAY; j++){
    if((strlen(mediaSizeData[j].rsc_name) == strlen(media_name)) && strncmp(mediaSizeData[j].rsc_name,media_name, strlen(mediaSizeData[j].rsc_name)) == 0){
    	debug_msg("mediaSizeData[%d].rsc_name: %s, x_name = %s\n", j, mediaSizeData[j].rsc_name, mediaSizeData[j].x_name);
      return mediaSizeData[j].value;
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Useage: epson-escpr model width_pixel height_pixel Ink PageSize Quality */
int
main (int argc, char *argv[])
{
	filter_option_t fopt;
	char libfile[PATH_MAX + 1]; 

	long width_pixel, height_pixel;
	long HWResolution;
	long bytes_per_line;
	int byte_par_pixel;
	double x_mag, y_mag;
	double print_area_x, print_area_y;
	char *image_raw;
	unsigned char *band;

	long read_size = 0;
	int band_line;

	int err = 0;
	int i, j;

	/* 2004.04.15 added for 'cancel page' */
	int cancel = 0;
	
	/* 2005.11.28 added  */
	char *paper;
	char *bin_id;
	char *point;

	/* library options */
	ESCPR_OPT printOpt;
	ESCPR_PRINT_QUALITY printQuality;
	//ESCPR_PRINT_JOB printJob;
	ESCPR_BANDBMP bandBmp;
	ESCPR_RECT bandRect;
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
  struct sigaction action;      /* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */

#ifdef HAVE_SIGSET
  sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);
#else
  signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGSET */
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	memset (&jobAttr, 0, sizeof(jobAttr));
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/* attach point */
#ifdef USE_DEBUGGER
	int flag = 1;
	while (flag) sleep (3);
#endif /* USE_DEBUGGER */

	DEBUG_START;
	err_init (argv[0]);

	if (argc != 9)
	{
		for ( i = 1; i < argc; i++ ) {
			if ( (0 == strncmp(argv[i], "-v", (strlen("-v")+1)) )
				|| (0 == strncmp(argv[i], "--version", (strlen("--version")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_FILTER_VERSION);
				return 0;
			} else if ( (0 == strncmp(argv[i], "-u", (strlen("-u")+1)) )
				|| (0 == strncmp(argv[i], "--usage", (strlen("--usage")+1)) )
				) {
				fprintf(stderr, "%s\n", PIPSLITE_FILTER_USAGE);
				return 0;
			} else if ( (0 == strncmp(argv[i], "-h", (strlen("-h")+1)) )
				|| (0 == strncmp(argv[i], "--help", (strlen("--help")+1)) )
				) {
				fprintf(stderr, "%s\n%s\n", PIPSLITE_FILTER_VERSION, PIPSLITE_FILTER_USAGE);
				return 0;
			}
		}
		fprintf (stderr, "%s\n", PIPSLITE_FILTER_USAGE);
		return 1;
	}

	/* set filter options */
	memset (&fopt, 0, sizeof (filter_option_t));
	memset (&printOpt, 0, sizeof (ESCPR_OPT));
	memset (&printQuality, 0, sizeof(ESCPR_PRINT_QUALITY));
	//memset (&printJob, 0, sizeof(ESCPR_PRINT_JOB));
	memset (&bandBmp, 0, sizeof(ESCPR_BANDBMP));
	memset (&bandRect, 0, sizeof(ESCPR_RECT));

	strncpy (fopt.model, argv[1], NAME_MAX);
	for (i = 0; fopt.model[i] != '\0' && i < NAME_MAX - 1; i ++)
		fopt.model_low[i] = tolower (fopt.model[i]);
	fopt.model_low[i] = '\0';

	width_pixel = atol (argv[2]);
	height_pixel = atol (argv[3]);
	HWResolution = atol (argv[4]);

	strncpy (fopt.ink, argv[5], NAME_MAX);
	strncpy (fopt.media, argv[6], NAME_MAX);
	strncpy (fopt.quality, argv[7], NAME_MAX);
	strncpy (fopt.duplex, argv[8], NAME_MAX);
	
	debug_msg("all para\n");
	for(i = 0; i< argc; i++){
		debug_msg("argv[%d] = %s\n", i, argv[i]);
	}

	outfp = stdout;
	printOpt.fpspoolfunc = (ESCPR_FPSPOOLFUNC)print_spool_fnc;
	
	if (set_pips_parameter (&fopt, &printOpt, &printQuality))
		err_fatal ("Cannot get option of PIPS."); /* exit */
	debug_msg("after setpip, duplex = %d\n", jobAttr.duplex);
	point = fopt.media;
	if(point[0]=='T')
	{
		paper=str_clone(++point,strlen(fopt.media)-1);
		bin_id="BORDERLESS";
		jobAttr.printLayout = EPS_MLID_BORDERLESS;
	}
	else
	{
		paper=str_clone(point,strlen(fopt.media));
		bin_id="BORDER";
		jobAttr.printLayout = EPS_MLID_BORDERS;
	}
	
	eps_toupper(fopt.media);
	jobAttr.mediaSizeIdx = getMediaSizeID(fopt.media);

	switch(HWResolution){
	case 360:
		//printJob.InResolution = ESCPR_IR_3636;
		jobAttr.inputResolution = EPS_IR_360X360;
		break;
	case 720:
		//printJob.InResolution = ESCPR_IR_7272;
		jobAttr.inputResolution = EPS_IR_720X720;
		break;
	case 300:
		//printJob.InResolution = ESCPR_IR_3030;
		jobAttr.inputResolution = EPS_IR_300X300;
		break;
	case 600:
		//printJob.InResolution = ESCPR_IR_6060;
		jobAttr.inputResolution = EPS_IR_600X600;
		break;							
	}

	
	//if (get_page_size (&printJob, paper, bin_id))
	if (get_page_size (paper, bin_id))
		err_fatal ("Cannot get page size of PIPS.");

	band_line = 1;

	if (strcmp (fopt.ink, "COLOR") == 0)
		byte_par_pixel = 3;
	else
		byte_par_pixel = 1;
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	epsInitLib();
	epsInitJob();
	debug_msg("before setupjobattr, duplex = %d\n", jobAttr.duplex);
	debug_msg("mediasizeIDx = %d\n", 	jobAttr.mediaSizeIdx);
	err = SetupJobAttrib(&jobAttr);
	if (err){
		debug_msg("Error occurred in \"SetupJobAttrib\": %d\n", err);
		err_fatal ("Error occurred in \"SetupJobAttrib\".");	/* exit */
	}
	//printJob.paperWidth = width_pixel;
	//printJob.paperHeight = height_pixel;

	epsInitVariable();
	
	debug_msg("begin pageAllocBuffer()\n");
	err = pageAllocBuffer();
	if(err){
		debug_msg ("Error occurred in \"pageAllocBuffer\".");	/* exit */
		err_fatal ("Error occurred in \"pageAllocBuffer\".");	/* exit */
	}else{
		debug_msg ("pageAllocBuffer() success\n");
	}


	EPS_PRINTER_INN curPrinter;
	memset(&curPrinter, 0, sizeof(curPrinter));
	curPrinter.language = EPS_LANG_ESCPR;
	curPrinter.protocol = EPS_PROTOCOL_LPR;

	printJob.printer = &curPrinter;

	prtSetupJobFunctions(printJob.printer, &jobFnc);
	debug_msg("call SendStartJob function\n");

	err = SendStartJob(FALSE);
	if (err){
		debug_msg("Error occurred in \"pageStartJob\": %d\n", err);
		err_fatal ("Error occurred in \"pageStartJob\".");	/* exit */
	}else{
		debug_msg("pageStartJob() success");
	}
	
	printJob.jobStatus = EPS_STATUS_ESTABLISHED;
	int printHeight = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////	




	print_area_x = printJob.printableAreaWidth;
	print_area_y = printJob.printableAreaHeight;

	/* setup band struct */
	bandBmp.WidthBytes = WIDTH_BYTES (print_area_x * 3 * 8 );

	band = (unsigned char *)mem_new (char, bandBmp.WidthBytes * band_line);
	memset (band, 0xff, bandBmp.WidthBytes * band_line);
	bandBmp.Bits = band;

	bandRect.left = 0;
	bandRect.right = printJob.printableAreaWidth;
	bandRect.top = 0;
	bandRect.bottom = 0;

	/* debug */
	DEBUG_JOB_STRUCT (printJob);
	DEBUG_QUALITY_STRUCT (printQuality);
#if 0
	err = escprInitJob (&printOpt, &printQuality, &printJob);
	if (err)
		err_fatal ("Error occurred in \"INIT_FUNC\".");	/* exit */
#endif	

	x_mag = (double)print_area_x / width_pixel;
	y_mag = (double)print_area_y / height_pixel;
	band_line = 1;
 	bytes_per_line = WIDTH_BYTES(width_pixel * byte_par_pixel * 8 );
	debug_msg("bytes_per_line = %d\n", bytes_per_line);
	image_raw = mem_new0 (char, bytes_per_line);
	//epsStartJob(&jobAttr);
	int page_count = 0;
	while ((read_size = read (STDIN_FILENO, image_raw, bytes_per_line)) > 0)
	{
		debug_msg("step 100.0\n");
		long x_count, y_count;
		int band_line_count;
#if (HAVE_PPM)
		char ppmfilename[30];
		FILE *fp;
#endif
		y_count = 0;
		band_line_count = 0;
		bandRect.top = 0;

		//err = escprInitPage ();
		err = epsStartPage(NULL);
#if (HAVE_PPM)
		sprintf(ppmfilename, "/tmp/px-b750f-page%d.ppm", page_count);
		fp = fopen(ppmfilename, "w");
		fprintf(fp, "P3\n");
		fprintf(fp, "%d\n",bytes_per_line/byte_par_pixel);
		fprintf(fp, "%d\n",(int)print_area_y);
		fprintf(fp, "255\n");
		fclose(fp);
#endif
		page_count++;
		if (err)
			err_fatal ("Error occurred in \"PINIT_FUNC\"."); /* exit */
		debug_msg("step 100.1\n");
		debug_msg("ppm image width = %d; height = %d\n", bytes_per_line/3, (int)print_area_y);
		printJob.verticalOffset = 0;
		debug_msg("start page %d ======================\n", page_count);
		if (jobAttr.duplex ==  EPS_DUPLEX_SHORT && ((page_count % 2) == 0))
		{
			char* pagebuf = malloc((int)(print_area_y) * (bandBmp.WidthBytes));
			char* startpage = pagebuf;
			long int posbuf = 0;
			//debug_msg("height pixel = %d\n", height_pixel);
			//debug_msg("height pixel [%d]* byte_per_line [%d]=%d// Widthbyte = %d\n", (int)height_pixel, bytes_per_line, (int)(height_pixel)*(bytes_per_line), bandBmp.WidthBytes);
			for (i = 0; i < print_area_y; i ++)
			{
				char *line;
				//debug_msg("step 10\n");
				line = (char *)(band + (bandBmp.WidthBytes * band_line_count));
				while ((0 == y_count) || ((i > (y_mag * y_count) - 1) && (y_count < height_pixel))) {
					while ((0 == err) && (read_size < bytes_per_line)) {
						size_t rsize;
						rsize = read(STDIN_FILENO, image_raw + read_size, bytes_per_line - read_size);
						//debug_msg("step 11\n");
						if (rsize <= 0) {
							if ((errno != EINTR) && (errno != EAGAIN) && (errno != EIO)) {
								err = -1;
								cancel = 1;
								goto quit;
								/* not reached		*/
								break;
							}
							usleep(50000);
						} else {
							read_size += rsize;
						}
					}
					if( read_size == bytes_per_line ){
						y_count++;
						read_size = 0;
					}
				}
				read_size = 0;
				/* convert input raster data to output byte data. */
				int copybyte = 1;
				if ( 1 == byte_par_pixel )
					copybyte = 3;
				if (x_mag == 1)
				{
					//debug_msg("step 12\n");
					if ( 1 == byte_par_pixel )
					{
						int k;
						for ( j = 0; j < print_area_x; j++ )
						{
							for ( k = 0; k < copybyte ; k++ )
								line[j * 3 + k] = image_raw[j];
						}
					}
					else
						memcpy (line, image_raw, bytes_per_line);
				}
				else
				{
					//debug_msg("step 13\n");
					x_count = 0;
					for (j = 0; j < print_area_x; j ++)
					{
						int k;
						while ( x_count == 0 || j > x_mag * x_count)
							x_count ++;
						for ( k = 0; k < copybyte ; k++ )
							memcpy (line + (j * 3 ) + k,
							image_raw + ((x_count - 1) * byte_par_pixel),
							byte_par_pixel);
					}
				}
				//debug_msg("step 14\n");
				band_line_count ++;
				if (band_line_count >= band_line)
				{
					bandRect.bottom = bandRect.top + band_line_count;
					printHeight = band_line_count;
					printHeight = 1;
					memcpy(pagebuf, bandBmp.Bits, bandBmp.WidthBytes);
#if (HAVE_PPM)
					fp = fopen(ppmfilename, "a+");
					int i=0;	
					for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
						fprintf(fp, "%u ", (unsigned char)pagebuf[i]);
					}
					fprintf(fp, "\n");
					fclose(fp);
#endif
					pagebuf+= bandBmp.WidthBytes;
					posbuf+=bandBmp.WidthBytes;
					//D_PrintBand (bandBmp.Bits, bandBmp.WidthBytes, &printHeight);
					band_line_count -= printHeight;
					bandBmp.Bits += band_line_count;
					
					bandRect.top = bandRect.bottom;
				}
			}
			//debug_msg("step 15\n");
			if (band_line_count > 0)
			{
				bandRect.bottom = bandRect.top + band_line_count;
				printHeight = 1;
				memcpy(pagebuf, bandBmp.Bits, bandBmp.WidthBytes);
#if (HAVE_PPM)
				fp = fopen(ppmfilename, "a+");
				int i=0;	
				for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
					fprintf(fp, "%u ", (unsigned char)pagebuf[i]);
				}
				fprintf(fp, "\n");
				fclose(fp);
#endif
				pagebuf+= bandBmp.WidthBytes;
				posbuf+= bandBmp.WidthBytes;
				band_line_count -= printHeight;
				bandBmp.Bits += band_line_count;
			}
			//debug_msg("band line count = %d\n", band_line_count);
			//debug_msg("width byte = %d\n", bandBmp.WidthBytes);
			//debug_msg("posbuf = %d\n", posbuf);
		
			int revert = 0;
			int pos = posbuf - bandBmp.WidthBytes ;
			int revert_line;
			char *rever_buf = malloc(bandBmp.WidthBytes);
			//debug_msg("byte_par_pixel = %d\n", byte_par_pixel);
			for (revert = print_area_y; revert > 0; revert--)
			{
				if (3 != byte_par_pixel)
				{
					int k = 0;
					for (k = bandBmp.WidthBytes/3; k >= 0; k--)
					{
						memcpy(rever_buf + k*3, startpage + pos + (bandBmp.WidthBytes) - k*3, 3);
					}
				}
				else
				{
					int k = 0;
					for (k = bandBmp.WidthBytes/3; k >= 0; k--)
					{
						memcpy(rever_buf + k*3, startpage + pos + (bandBmp.WidthBytes - 6) - k*3, 3);
// 						if (k == 0)
// 						{
// 							memcpy(rever_buf + k*3, startpage + pos + bandBmp.WidthBytes + 3, 3);
// 						}
					}
					//debug_msg("k = %d\n", k);
				}
				//debug_msg("step 100\n");
				PrintBand (rever_buf, bandBmp.WidthBytes, &printHeight);
				pos -= bandBmp.WidthBytes;
				//debug_msg("printHeight = %d\n", printHeight);
				//debug_msg("WidthByte = %d\n", bandBmp.WidthBytes);
			}
			debug_msg("free rever\n");
			if (rever_buf != NULL)
			{
				free(rever_buf);
				rever_buf = NULL;
			}
			debug_msg("free page\n");
			if (pagebuf != NULL)
			{
				pagebuf = startpage;
				free (pagebuf);
				pagebuf = NULL;
			}
			debug_msg("free page sucessfull\n");
			err = epsEndPage(FALSE);

			if(err)
				err_fatal ("Error occurred in \"PEND_FUNC\".");	/* exit */
		}
		else
		{
			for (i = 0; i < print_area_y; i ++)
			{
				char *line;
	
				line = (char *)(band + (bandBmp.WidthBytes * band_line_count));
				while ((0 == y_count) || ((i > (y_mag * y_count) - 1) && (y_count < height_pixel))) {
					while ((0 == err) && (read_size < bytes_per_line)) {
						size_t rsize;
						
						rsize = read(STDIN_FILENO, image_raw + read_size, bytes_per_line - read_size);
						/* 2009.03.17 epson-escpr-1.0.0 */
						/* if user cancels job from CUPS Web Admin, */
						/* epson-escpr-wrapper exit normally, */
						/* and read() return rsize = 0 */
						if (rsize <= 0) {
							if ((errno != EINTR) && (errno != EAGAIN) && (errno != EIO)) {
								err = -1;
								/* 2004.04.15		*/
								/* error then quit	*/ 
								/* don't care err = -1  */
								cancel = 1;
								goto quit;
								/* not reached		*/
								break;
							}
							usleep(50000);
						} else {
							read_size += rsize;
						}
					}
	
					/* 2004.04.01                                              */
					/* for "skip reading"                                      */
					/* modified y_count count up condition and clear read_size */
					if( read_size == bytes_per_line ){
						y_count++;
						/* need to clear read_size               */
						/* not clear, data still remains on pipe */
						read_size = 0;
					}
				}
				read_size = 0;
				
				/* convert input raster data to output byte data. */
				{
					int copybyte = 1;
	
					if ( 1 == byte_par_pixel )
						copybyte = 3;
	
					if (x_mag == 1)
					{
	
						if ( 1 == byte_par_pixel )
						{
							int k;
	
							for ( j = 0; j < print_area_x; j++ )
							{
								for ( k = 0; k < copybyte ; k++ )
									line[j * 3 + k] = image_raw[j];
							}
						}
						else
							memcpy (line, image_raw, bytes_per_line);
						
					}
					else
					{
						x_count = 0;
						
						for (j = 0; j < print_area_x; j ++)
						{
							int k;
	
							while ( x_count == 0 || j > x_mag * x_count)
								x_count ++;
							
							for ( k = 0; k < copybyte ; k++ )
								memcpy (line + (j * 3 ) + k,
								image_raw + ((x_count - 1) * byte_par_pixel),
								byte_par_pixel);
						}
					}
				}
				
				band_line_count ++;
				
				if (band_line_count >= band_line)
				{
					bandRect.bottom = bandRect.top + band_line_count;
					#if 0
					if (escprBandOut (&bandBmp, &bandRect))
						err_fatal ("Error occurred in \"OUT_FUNC\"."); /* exit */
					#endif
					printHeight = band_line_count;
					PrintBand (bandBmp.Bits, bandBmp.WidthBytes, &printHeight);
#if (HAVE_PPM)
					fp = fopen(ppmfilename, "a+");
					int i=0;	
					for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
						fprintf(fp, "%u ", (unsigned char)bandBmp.Bits[i]);
					}
					fprintf(fp, "\n");
					fclose(fp);
#endif
					debug_msg("printHeight = %d\n", printHeight);
					debug_msg("WidthByte = %d\n", bandBmp.WidthBytes);
					band_line_count -= printHeight;
					bandBmp.Bits += band_line_count;
					bandRect.top = bandRect.bottom;
				}
				
			}
			
			if (band_line_count > 0)
			{
				bandRect.bottom = bandRect.top + band_line_count;
				
				//if (PrintBand (&bandBmp, bandBmp.WidthBytes))
				err = PrintBand (bandBmp.Bits, bandBmp.WidthBytes, &printHeight);
				debug_msg("printHeight = %d\n", printHeight);
				if(err)
					err_fatal ("Error occurred in \"OUT_FUNC\"."); /* exit */
#if (HAVE_PPM)
				fp = fopen(ppmfilename, "a+");
				int i=0;	
				for(i=0; i<bytes_per_line/byte_par_pixel*3; i++){		
					fprintf(fp, "%u ", (unsigned char)bandBmp.Bits[i]);
				}
				fprintf(fp, "\n");
				fclose(fp);
#endif
				band_line_count -= printHeight;
				bandBmp.Bits += band_line_count;
			}
			
			//if (escprTerminatePage (1ESCPR_END_PAGE))
			err = epsEndPage(FALSE);
			if(err)
				err_fatal ("Error occurred in \"PEND_FUNC\".");	/* exit */
		}
	}
	
	DEDBUG_END;

/* 2004.04.15 for 'error' */	
quit:;
	if( cancel ){
		//escprTerminatePage (ESCPR_END_PAGE);
		err = epsEndPage(FALSE);
	}
     
	//if (escprDestroyJob ())
	err = epsEndJob();
	if(err)
		err_fatal ("Error occurred in \"END_FUNC\"."); /* exit */

	/* free alloced memory */
	mem_free(image_raw);
	mem_free(band);
	mem_free(paper);
	debug_msg("PRINT SUCCESS\n");
	return 0;
}


static int
set_pips_parameter (filter_option_t *filter_opt_p, ESCPR_OPT *printOpt, ESCPR_PRINT_QUALITY *printQuality)
{
	char *mediaType;
	char *quality;
	char *ink;
	char *duplex;

	if (strlen (filter_opt_p->media) == 0
	    || strlen (filter_opt_p->ink) == 0
	    || strlen (filter_opt_p->quality) == 0)
		return 1;

	/* pickup MediaType & Quality from input */
	quality = strrchr(filter_opt_p->quality, '_');

	if(strlen(quality) == 0)
		return 1;

	mediaType = str_clone (filter_opt_p->quality, strlen(filter_opt_p->quality) - strlen(quality));

	/* Media Type ID */
	printQuality->MediaTypeID = getMediaTypeID(mediaType); 
	jobAttr.mediaTypeIdx = getMediaTypeID(mediaType);
	debug_msg(" mediaType = %s\n", mediaType);

	/* Print Quality */
	if(strcmp(quality, "_DRAFT") == 0){
		printQuality->PrintQuality = ESCPR_PQ_DRAFT;
		jobAttr.printQuality = EPS_MQID_DRAFT; 
	}else if(strcmp(quality, "_NORMAL") == 0){
		printQuality->PrintQuality = ESCPR_PQ_NORMAL;
		jobAttr.printQuality = EPS_MQID_NORMAL; 
	}else{
		printQuality->PrintQuality = ESCPR_PQ_HIGH;		  
		jobAttr.printQuality = EPS_MQID_DRAFT; 
	}

	/* Ink */
	ink = str_clone (filter_opt_p->ink, strlen (filter_opt_p->ink));
 	if (strcmp (ink, "COLOR") == 0){ 
 		printQuality->ColorMono = ESCPR_CM_COLOR; 
 		jobAttr.colorMode =  EPS_CM_COLOR;
 	}
 	else{ 
 		printQuality->ColorMono = ESCPR_CM_MONOCHROME;
 		jobAttr.colorMode =  EPS_CM_MONOCHROME; 
 	}

	/* Duplex */
	duplex = str_clone (filter_opt_p->duplex, strlen (filter_opt_p->duplex));
 	if (strcmp (duplex, "None") == 0){ 
		debug_msg("DUPLEX NONE\n");
 		jobAttr.duplex =  EPS_DUPLEX_NONE;
 	}
	if (strcmp (duplex, "DuplexNoTumble") == 0){ 
		debug_msg("DUPLEX LONG\n");
 		jobAttr.duplex =  EPS_DUPLEX_LONG;
 	}
 	if (strcmp (duplex, "DuplexTumble") == 0){ 
		debug_msg("DUPLEX SHORT\n");
 		jobAttr.duplex =  EPS_DUPLEX_SHORT;
 	}
	/* Others */
	printQuality->Brightness = 0;
	printQuality->Contrast = 0;
	printQuality->Saturation = 0;
	printQuality->ColorPlane = ESCPR_CP_FULLCOLOR; /* default for epson-escpr */
	printQuality->PaletteSize = 0;                 /* cause ColorPlane is FULLCOLOR */
	printQuality->PaletteData = NULL;
	
	jobAttr.brightness = 0;
	jobAttr.contrast = 0;
	jobAttr.saturation = 0;
	jobAttr.apfAutoCorrect = EPS_APF_ACT_STANDARD;
	jobAttr.sharpness = 0;
	jobAttr.colorPlane = ESCPR_CP_FULLCOLOR;
	jobAttr.paletteSize = 0;
	jobAttr.paletteData = NULL;
	jobAttr.copies = 1;
	jobAttr.feedDirection = EPS_FEEDDIR_PORTRAIT;      /* paper feed direction  hardcode */
	jobAttr.paperSource = EPS_MPID_AUTO;
	jobAttr.printDirection = 0;

	/* free alloced memory */
	mem_free(mediaType);
	mem_free(ink);

	return 0;
}


/* Get PageSize for PIPS */
static int
get_page_size (const char *paper, const char *bin_id)
{
#if 0
	csvlist_t *csv_p;
	const char *path = PAPER_PATH;
	int pos;

	long l_margin, r_margin, t_margin, b_margin;
	
	csv_p = csvlist_open (path);
	if (!csv_p)
		err_fatal ("%s : List file is broken.", path);

	pos = csvlist_search_keyword (csv_p, 0, paper);
	if (pos < 0)
		err_fatal ("%s : Unknown PageSize.", paper);


	//if(printJob->InResolution == ESCPR_IR_3636 || printJob->InResolution == ESCPR_IR_7272){
	if(jobAttr.inputResolution == EPS_IR_360X360 || jobAttr.inputResolution == EPS_IR_720X720){
		printJob->PaperWidth = atol (csvlist_get_word (csv_p, pos + 2));
		printJob->PaperLength = atol (csvlist_get_word (csv_p, pos + 3));

		l_margin = atol (csvlist_get_word (csv_p, pos + 4));
		r_margin = atol (csvlist_get_word (csv_p, pos + 5));
		t_margin = atol (csvlist_get_word (csv_p, pos + 6));
		b_margin = atol (csvlist_get_word (csv_p, pos + 7));
	}else{
		printJob->PaperWidth = atol (csvlist_get_word (csv_p, pos + 8));
		printJob->PaperLength = atol (csvlist_get_word (csv_p, pos + 9));

		l_margin = atol (csvlist_get_word (csv_p, pos + 10));
		r_margin = atol (csvlist_get_word (csv_p, pos + 11));
		t_margin = atol (csvlist_get_word (csv_p, pos + 12));
		b_margin = atol (csvlist_get_word (csv_p, pos + 13));

	}


	if (!strcmp (bin_id, "BORDER"))
	{
 		printJob->PrintableAreaWidth = printJob->PaperWidth - l_margin - r_margin;
		printJob->PrintableAreaLength = printJob->PaperLength - t_margin - b_margin;
		printJob->TopMargin = t_margin;
		printJob->LeftMargin = l_margin;
	}

	else if (!strcmp (bin_id, "BORDERLESS"))
	{
		if(printJob->InResolution == ESCPR_IR_3636 || printJob->InResolution == ESCPR_IR_7272){
	 		printJob->PrintableAreaWidth = printJob->PaperWidth + 72;
			printJob->PrintableAreaLength = printJob->PaperLength + 112;
			printJob->TopMargin = -42;
			if ( printJob->PaperWidth < 4209 ) 
				printJob->LeftMargin = -36;
			else
				printJob->LeftMargin = -48;
		}else{
	 		printJob->PrintableAreaWidth = printJob->PaperWidth + 60;
			printJob->PrintableAreaLength = printJob->PaperLength + 93;
			printJob->TopMargin = -35;
			if ( printJob->PaperWidth < 3507 ) 
				printJob->LeftMargin = -30;
			else
				printJob->LeftMargin = -40;

		}

	}

	else
	{
		err_fatal ("%s : This sheet feeder is not supported.");
	}

	printJob->PrintDirection =  0; 

	/* free alloced memory */
	csvlist_close(csv_p);
#endif
	return 0;
}

static int  getMediaTypeID(char *rsc_name)
{
  int j;
	debug_msg("mediatype name = %s\n", rsc_name);
  for(j = 0; mediaTypeData[j].value != END_ARRAY; j++)
    if(strcmp(mediaTypeData[j].rsc_name,rsc_name) == 0)
      return mediaTypeData[j].value;
  return 0;
}

ESCPR_BYTE4 print_spool_fnc(void* hParam, const ESCPR_UBYTE1* pBuf, ESCPR_UBYTE4 cbBuf) 
{
	long int i;
	for (i = 0; i < cbBuf; i++)
		putc(*(pBuf + i), outfp);
	return 1;
}
