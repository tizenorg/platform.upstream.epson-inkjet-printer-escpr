/*_______________________________   epson-escpr-api.c   ________________________________*/

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
/*                             Epson ESC/P-R Library APIs                               */
/*                                                                                      */
/*                                Core Function Calls                                   */
/*                            --------------------------                                */
/*		API-0001   EPS_ERR_CODE epsInitDriver        (commMode, usbFuncPtrs,            */
/*                                                    netFuncPtrs, cmnFuncPtrs);        */
/*      API-0002   EPS_ERR_CODE epsReleaseDriver     ();                                */
/*      API-0003   EPS_ERR_CODE epsFindPrinter       (protocol, timeout);               */
/*      API-0004   EPS_ERR_CODE epsProbePrinter      (probeParam);                      */
/*      API-0005   EPS_ERR_CODE epsCancelFindPrinter ();                                */
/*      API-0006   EPS_ERR_CODE epsSetPrinter        (printer);                         */
/*      API-0007   EPS_ERR_CODE epsStartJob          (jobAttrib);                       */
/*      API-0008   EPS_ERR_CODE epsStartPage         (pageAttrib);                      */
/*      API-0009   EPS_ERR_CODE epsPrintBand         (imageData,width,,height);         */
/*      API-0010   EPS_ERR_CODE epsSendData          (prnData);                         */
/*      API-0011   EPS_ERR_CODE epsEndPage           ();                                */
/*      API-0012   EPS_ERR_CODE epsEndJob            ();                                */
/*      API-0013   EPS_ERR_CODE epsCancelJob         (reserve);                         */
/*      API-0014   EPS_ERR_CODE epsContinueJob       ();                                */
/*      API-0015   EPS_ERR_CODE epsGetStatus         (status);                          */
/*      API-0016   EPS_ERR_CODE epsGetSupportedMedia (media);                           */
/*      API-0017   EPS_ERR_CODE epsGetPrintableArea  (pageAttrib,                       */
/*                                                      printableWidth,                 */
/*                                                      printableHeight);               */
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------  Local Compiler Switch  -------------------------------*/
/*******************************************|********************************************/

    /*** 4KB Packet Data                                                                */
    /***--------------------------------------------------------------------------------*/
#define LCOMSW_PACKET_4KB           0       /* 0: Make any size packet                  */
                                            /* 1: Make 4KB packet                       */
#define LCOMSW_CANCEL_JOB           0

#define LCOMSW_USE_720DPI           0		/* Not support 720dpi                       */       

#define ESCPR_DEBUG_IMAGE_LOG       0       /* 0: OFF    1: ON                          */

#define LCOMSW_DUMMY_SEND			0		/* 1: Enable 0byte data sending             */

/*#define LCOMSW_JOBPARAM_CEHCK_OFF*/

/* #define LCOMSW_CMDDMP */

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-pvt.h"
#include "epson-escpr-services.h"
#include "epson-escpr-mem.h"
#include "epson-protocol.h"
#include "epson-layout.h"
#ifdef GCOMSW_CMD_ESCPAGE
#include "epson-escpage.h"
	#ifdef GCOMSW_CMD_ESCPAGE_S
	#include "epson-escpage-s.h"
	#endif
#endif
#include "epson-escpr-api.h"


/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/
DECRALE_DMP_FILE

    /*** Extern Function                                                                */
    /*** -------------------------------------------------------------------------------*/
EPS_USB_FUNC    epsUsbFnc;
EPS_NET_FUNC    epsNetFnc;
EPS_CMN_FUNC    epsCmnFnc;

    /*** Print Job Structure                                                            */
    /*** -------------------------------------------------------------------------------*/
EPS_PRINT_JOB   printJob;
EPS_INT32 tonerSave;
EPS_INT32 back_type;

EPS_INT32 lWidth;
EPS_INT32 lHeight;

EPS_INT32 areaWidth;
EPS_INT32 areaHeight;


/*-------------------------  Module "Local Global" Variables  --------------------------*/
/*******************************************|********************************************/
    /*** internal stock                                                                 */
    /*** -------------------------------------------------------------------------------*/
static EPS_SUPPORTED_MEDIA	g_supportedMedia;				/* Supported Media          */

    /*** Job function                                                                   */
    /*** -------------------------------------------------------------------------------*/
EPS_JOB_FUNCS		jobFnc;							

/*--------------------------------  Local Definition   ---------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_API
#define EPS_LOG_MODULE EPS_LOG_MODULE_API
#else
#define EPS_LOG_MODULE 0
#endif

    /*** Roop Count                                                                     */
    /*** -------------------------------------------------------------------------------*/
#define EPS_ROOP_NUM                   40       /* Send the data for "EPS_ROOP_NUM"     */
                                                /* times and get printer status         */

#define EPS_TIMEOUT_NUM                20       /* retry Send the data "EPS_TIMEOUT_NUM"*/
#define EPS_TIMEOUT_SEC                1000     /* retry Send the data 1 sec */

    /*** Packet Size (4KB)                                                              */
    /*** -------------------------------------------------------------------------------*/
#if LCOMSW_PACKET_4KB
#define ESCPR_PACKET_SIZE_4KB        4090
#endif

    /*** ESC/PR Attributes not settable by developer                                    */
    /*** -------------------------------------------------------------------------------*/
#define EPS_COMP_NON                    0       /* Non compression                      */
#define EPS_COMP_RLE                    1       /* Runlength compression                */

#define EPS_END_PAGE                    0       /* There is no next page                */
#define EPS_NEXT_PAGE                   1       /* There is a next page                 */

    /*** RLE Compression States                                                         */
    /*** -------------------------------------------------------------------------------*/
#define EPS_RLE_COMPRESS_NOT_DONE       0
#define EPS_RLE_COMPRESS_DONE           1

    /*** Borderless Mode                                                                */
    /*** -------------------------------------------------------------------------------*/
#define EPS_BORDER_3MM_MARGINE          0
#define EPS_BORDER_CUSTOM               1
#define EPS_BORDERLESS_NORMAL           2
#define EPS_BORDERLESS_ZERO_MARGINE     3


/*---------------------------------  ESC/P-R Commands  ---------------------------------*/
/*******************************************|********************************************/
    /*** ESC/P-R Command Length                                                         */
    /*** -------------------------------------------------------------------------------*/
#define REMOTE_HEADER_LENGTH            5
#define ESCPR_CLASS_LENGTH              2    /* ESC + CLASS */
#define ESCPR_HEADER_LENGTH            10    /* ESC + CLASS + ParamLen + CmdName */
#define ESCPR_PRINT_QUALITY_LENGTH      9
#define ESCPR_SEND_DATA_LENGTH          7
#define ESCPR_SEND_JPGDATA_LENGTH       2
#define ESCPR_JPGHEAD_LENGTH            (ESCPR_HEADER_LENGTH + ESCPR_SEND_JPGDATA_LENGTH)

    /*** Escape Commands for Initialize/Colse Printer                                   */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 ExitPacketMode[]= {
                            0x00, 0x00, 0x00, 0x1B, 0x01, 0x40, 0x45, 0x4A, 0x4C, 0x20,
                            0x31, 0x32, 0x38, 0x34, 0x2E, 0x34, 0x0A, 0x40, 0x45, 0x4A,
                            0x4C, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0A                    };

static const EPS_UINT8 ESCPRMode[] = {
/*                            0x1B, 0x28, 0x52, 0x06, 0x00, 0x00, 0x45, 0x53, 0x43, 0x50, 0x52, };*/
                            0x1B, '(', 'R', 0x06, 0x00, 0x00, 'E', 'S', 'C', 'P', 'R' };
static const EPS_UINT8 ESCPRModeJpg[] = {
                            0x1B, '(', 'R', 0x07, 0x00, 0x00, 'E', 'S', 'C', 'P', 'R', 'J' };

static const EPS_UINT8 InitPrinter[] = {
                            0x1B, 0x40,                                                 };

static const EPS_UINT8 EnterRemoteMode[] = {
                            0x1B, 0x28, 0x52, 0x08, 0x00, 0x00, 'R', 'E', 'M', 'O', 'T', 'E', '1',  };

static const EPS_UINT8 RemoteJS[] = {
                            'J', 'S', 0x04, 0x00, 0x00, 
							0x00, 0x00, 0x00 };

static const EPS_UINT8 RemoteTI[] = {
		                    'T', 'I', 0x08, 0x00, 0x00,
							0x00, 0x00, /* YYYY */
							0x00,       /* MM */
							0x00,       /* DD */
							0x00,       /* hh */
							0x00,       /* mm */
							0x00,       /* ss */	};
							
static const EPS_UINT8 RemoteHD[] = {
                            'H', 'D', 0x03, 0x00, 0x00, 
							0x03, 0xFF };

static const EPS_UINT8 RemoteJH[] = {
                             'J',  'H', 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x45, 0x53, 0x43, 0x50, 0x52, 0x4c, 0x69, 0x62              };

static const EPS_UINT8 RemotePP[] = {
                            'P', 'P', 0x03, 0x00, 0x00, 0x00, 0x00                      };

static const EPS_UINT8 RemoteDP[] = {
                            'D', 'P', 0x02, 0x00, 0x00, 0x02                            };

static const EPS_UINT8 RemoteLD[] = {
                            'L', 'D', 0x00, 0x00,										};

static const EPS_UINT8 RemoteJE[] = {
                            'J', 'E', 0x01, 0x00, 0x00,                                 };

static const EPS_UINT8 ExitRemoteMode[] = {
                            0x1B, 0x00, 0x00, 0x00,                                     };
#ifdef GCOMSW_EF_MAINTE
static const EPS_UINT8 RemoteCH[] = {
                            'C', 'H', 0x02, 0x00, 0x00, 0x00				};
static const EPS_UINT8 RemoteNC[] = {
                            'N', 'C', 0x02, 0x00, 0x00, 0x00				};
static const EPS_UINT8 RemoteVI[] = {
                            'V', 'I', 0x02, 0x00, 0x00, 0x00				};

static const EPS_UINT8 DataCR[] = {0x0D			};
static const EPS_UINT8 DataLF[] = {0x0A			};
static const EPS_UINT8 DataFF[] = {0x0C			};
#endif

    /*** ESC/P-R Commands (Print Quality)                                               */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 PrintQualityCmd[] = {
	                        0x1B, 'q', 0x09, 0x00, 0x00, 0x00,
                            's', 'e', 't', 'q',
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /*** ESC/P-R Commands (APF setting)                                                 */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 APFSettingCmd[]   = {
	                        0x1B, 'a', 0x04, 0x00, 0x00, 0x00,
                            's', 'e', 't', 'a',
                            0x00, 0x00, 0x00, 0x00};

    /*** ESC/P-R Commands (Print Job)                                                   */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 JobCmd[]          = {
	                        0x1B, 'j', 0x16, 0x00, 0x00, 0x00,
                            's', 'e', 't', 'j',
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const EPS_UINT8 JobCmdJpg[]       = {
	                        0x1B, 'j', 0x05, 0x00, 0x00, 0x00,
                            's', 'e', 't', 's',
                            0x00, 0x00, 0x00, 0x00, 0x00 };

    /*** ESC/P-R Command (Start Page)                                                   */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 StartPage[]       = {
	                        0x1B, 'p', 0x00, 0x00, 0x00, 0x00,  
                            's',  't',  't',  'p' };

    /*** ESC/P-R Commands (Send Data)                                                   */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 SendDataCmd[2]       = {0x1B, 'd'            };
static const EPS_UINT8 SendDataName[4]      = {'d', 's', 'n', 'd'   };
static const EPS_UINT8 SendJpegDataName[4]  = {'j', 's', 'n', 'd'   };

    /*** ESC/P-R Command (End Page)                                                     */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 EndPage[]		= {
	                        0x1B, 'p', 0x01, 0x00, 0x00, 0x00,  
                            'e',  'n',  'd',  'p', 
                            0x00};

    /*** ESC/P-R Command (End Job)                                                      */
    /*** -------------------------------------------------------------------------------*/
static const EPS_UINT8 EndJob[] = {
                            0x1B,  'j', 0x00, 0x00, 0x00, 0x00,
							'e',  'n',  'd',  'j' };

/*-------------------------------  Definition of Macro  --------------------------------*/
/*******************************************|********************************************/


/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/

    /*** Generic Image Object                                                           */
    /*** -------------------------------------------------------------------------------*/
typedef struct _tagEPS_IMAGE_ {
    const EPS_UINT8*  data;                 /* Pointer to image/raster data             */
    EPS_RECT    rect;                       /* Rect define position of image            */
    EPS_UINT32  bytesPerLine;               /* Bytes-Per-Pixel for image/raster         */
} EPS_IMAGE;

/*------------------------------  Local Global Variables  ------------------------------*/
/*******************************************|********************************************/
    /*** ESC/P-R LIB Status State                                                       */
    /*** -------------------------------------------------------------------------------*/
EPS_INT32    libStatus;                  /*  Library (epsInitDriver) status      */

    /*** Status counter                                                                 */
    /*** -------------------------------------------------------------------------------*/
static EPS_INT32    gStatusCount;   /* Variable for register the number of getting      */
                                    /* printer status                                   */

    /*** Buffer for Print Band                                                          */
    /*** -------------------------------------------------------------------------------*/
EPS_UINT32   sendDataBufSize = 0;
EPS_UINT8*   sendDataBuf;    /* buffer of SendCommand(save) input                */
EPS_UINT32   tmpLineBufSize;
EPS_UINT8*   tmpLineBuf;

/*--------------------------------  Local Functions   ----------------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE MonitorStatus           (EPS_STATUS_INFO *                          );
static EPS_ERR_CODE SendLeftovers           (void                                       );
static EPS_ERR_CODE SendBlankBand           (void                                       );
EPS_ERR_CODE SetupJobAttrib          (const EPS_JOB_ATTRIB*                      );
static EPS_ERR_CODE SetupRGBAttrib          (void                                       );
static EPS_ERR_CODE SetupJPGAttrib          (void                                       );
EPS_ERR_CODE SendStartJob			(EPS_BOOL                                   );
EPS_ERR_CODE SendEndJob  			(EPS_BOOL                                   );

EPS_ERR_CODE PrintBand               (const EPS_UINT8*, EPS_UINT32, EPS_UINT32*  );
static EPS_ERR_CODE PrintChunk              (const EPS_UINT8*, EPS_UINT32*              );
static void         AdjustBasePoint         (void                                       );
static EPS_ERR_CODE PrintLine               (EPS_IMAGE*                                 );
static EPS_ERR_CODE SendLine                (const EPS_BANDBMP*, EPS_RECT*              );
static EPS_UINT16   RunLengthEncode         (const EPS_UINT8*, EPS_UINT8*, EPS_UINT16, EPS_UINT8, EPS_UINT8*);

static void         MakeRemoteTICmd         (EPS_UINT8*                                 );
static void         MakeQualityCmd          (EPS_UINT8*                                 );
static void         MakeAPFCmd              (EPS_UINT8*                                 );
static void         MakeJobCmd              (EPS_UINT8*                                 );
static EPS_ERR_CODE AddCmdBuff              (EPS_UINT8 **, EPS_UINT8 **, EPS_UINT32 *, 
											const EPS_UINT8 *, EPS_UINT32               );
static EPS_ERR_CODE CreateMediaInfo			(EPS_PRINTER_INN*, EPS_UINT8*, EPS_INT32	);
static EPS_ERR_CODE	DuplSupportedMedia      (EPS_PRINTER_INN*, EPS_SUPPORTED_MEDIA*     );
static EPS_ERR_CODE	GetPaperSource          (EPS_PRINTER_INN*                           );
static EPS_ERR_CODE GetJpgMax               (EPS_PRINTER_INN*                          );
static void     	ClearSupportedMedia     (void                                      );



/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------             ESC/P-R Library API             ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsInitDriver()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* commMode     EPS_INT8            I: Communication Mode                               */
/*                                     (Bi-Directional or Uni-Directional)              */
/* usbFuncPtrs  EPS_USB_FUNC*       I: Data structure containing function pointers to   */
/*                                     external USB I/O functions.                      */
/* netFuncPtrs  EPS_NET_FUNC*       I: Data structure containing function pointers to   */
/*                                     external network socket I/O functions.           */
/* cmnFuncPtrs  EPS_CMN_FUNC*       I: Data structure containing function pointers to   */
/*                                     external memory management functions.            */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                     - Success                                      */
/*      << Error >>                                                                     */
/*      EPS_ERR_LIB_INTIALIZED           - Lib already initialized                      */
/*      EPS_ERR_INV_ARG_COMMMODE         - Invalid argument "commMode"                  */
/*      EPS_ERR_INV_FNCP_NULL            - Invalid function pointer structure           */
/*      EPS_ERR_INV_FNCP_FINDCALLBACK    - Invalid function pointer "findCallback"      */
/*      EPS_ERR_INV_FNCP_MEMALLOC        - Invalid function pointer "memAlloc"          */
/*      EPS_ERR_INV_FNCP_MEMFREE         - Invalid function pointer "memFree"           */
/*      EPS_ERR_INV_FNCP_SLEEP           - Invalid function pointer "sleep"             */
/*      EPS_ERR_INV_FNCP_OPENPORTAL      - Invalid function pointer "openPortal"        */
/*      EPS_ERR_INV_FNCP_CLOSEPORTAL     - Invalid function pointer "closePortal"       */
/*      EPS_ERR_INV_FNCP_READPORTAL      - Invalid function pointer "readPortal"        */
/*      EPS_ERR_INV_FNCP_WRITEPORTAL     - Invalid function pointer "writePortal"       */
/*      EPS_ERR_INV_FNCP_FINDFIRST       - Invalid function pointer "findFirst"         */
/*      EPS_ERR_INV_FNCP_FINDNEXT        - Invalid function pointer "findNext"          */
/*      EPS_ERR_INV_FNCP_FINDCLOSE       - Invalid function pointer "findClose"         */
/*      EPS_ERR_INV_FNCP_NETSOCKET       - Invalid function pointer "socket"            */
/*      EPS_ERR_INV_FNCP_NETCLOSE        - Invalid function pointer "close"             */
/*      EPS_ERR_INV_FNCP_NETCONNECT      - Invalid function pointer "connect"           */
/*      EPS_ERR_INV_FNCP_NETSHUTDOWN     - Invalid function pointer "shutdown"          */
/*      EPS_ERR_INV_FNCP_NETBIND         - Invalid function pointer "bind"              */
/*      EPS_ERR_INV_FNCP_NETLISTEN       - Invalid function pointer "listen"            */
/*      EPS_ERR_INV_FNCP_NETACCEPT       - Invalid function pointer "accept"            */
/*      EPS_ERR_INV_FNCP_NETSEND         - Invalid function pointer "send"              */
/*      EPS_ERR_INV_FNCP_NETSENDTO       - Invalid function pointer "sendTo"            */
/*      EPS_ERR_INV_FNCP_NETRECEIVE      - Invalid function pointer "receive"           */
/*      EPS_ERR_INV_FNCP_NETRECEIVEFROM  - Invalid function pointer "receiveFrom"       */
/*      EPS_ERR_INV_FNCP_NETGETSOCKNAME  - Invalid function pointer "getsockname"       */
/*      EPS_ERR_INV_FNCP_NETSETBROADCAST - Invalid function pointer "setBroadcast"      */
/*      EPS_ERR_MEMORY_ALLOCATION        - Failed to allocate memory                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Registers the external functions with the device driver.                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsInitDriver (

        EPS_INT32               commMode, 
		const EPS_USB_FUNC*     usbFuncPtrs,
        const EPS_NET_FUNC*     netFuncPtrs,
        const EPS_CMN_FUNC*	    cmnFuncPtrs

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;       /* Return status of internal calls      */

EPS_LOG_FUNCIN

#ifdef GCOMSW_EPSON_SLEEP
EPS_INT32       idx, tdx;                       /* General loop/index varaible          */
EPS_INT32       sleepTime; 
struct timeb    sleepS, sleepE;
#endif /* GCOMSW_EPSON_SLEEP */

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_INTIALIZED );
    }

/*** Validate input parameters                                                          */
	if( EPS_ERR_NONE != (retStatus = prtFunctionCheck(commMode, usbFuncPtrs, netFuncPtrs, cmnFuncPtrs)) ){
		EPS_RETURN( retStatus );
	}

/*** Register Functions                                                                 */
	if( commMode & EPS_PROTOCOL_USB ){
        memcpy((void*)(&epsUsbFnc), (void*)usbFuncPtrs, sizeof(EPS_USB_FUNC));
	} else{
	    memset((void*)(&epsUsbFnc), 0, sizeof(EPS_USB_FUNC));
	}
	if( commMode & EPS_PROTOCOL_NET ){
        memcpy((void*)(&epsNetFnc), (void*)netFuncPtrs, sizeof(EPS_NET_FUNC));
	} else{
	    memset((void*)(&epsNetFnc), 0, sizeof(EPS_NET_FUNC));
	}
    memcpy((void*)(&epsCmnFnc), (void*)cmnFuncPtrs, sizeof(EPS_CMN_FUNC));

#ifdef GCOMSW_EPSON_SLEEP
    if (epsCmnFnc.sleep == NULL)
        epsCmnFnc.sleep = serSleep;
#endif /* GCOMSW_EPSON_SLEEP */


/*** Initialize ESC/P-R Lib and Local Global Variables                                  */
    gStatusCount = 0;

    memset(&printJob, 0, sizeof(EPS_PRINT_JOB));
    
    printJob.jobStatus  = EPS_STATUS_NOT_INITIALIZED;
    printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.findStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.printer    = NULL;
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
    printJob.commMode = commMode;

/*** Change ESC/P-R Lib Status                                                          */
    libStatus = EPS_STATUS_INITIALIZED;

/*** Return to Caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsReleaseDriver()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* N/A              void                                                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                     - Success                                      */
/*      << Error >>                                                                     */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - ESC/P-R Lib is NOT initialized                */
/*                                                                                      */
/* Description:                                                                         */
/*      Creanup Driver.                                                                 */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsReleaseDriver (
	
		void

){
	EPS_LOG_FUNCIN;

/*** Has a Lib been initialized                                                         */
    if (libStatus == EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

	epsEndJob();

/*** Clear inside supported media list                                                  */
	ClearSupportedMedia();

/*** Clear inside printer list                                                          */
	prtClearPrinterList();
	printJob.printer = NULL;
	obsClear();

/*** Clear inside additional data buffer                                                */
	/* DEL EPS_SAFE_RELEASE( printJob.qrcode.bits ) */

/*** Change ESC/P-R Lib Status                                                          */
	libStatus = EPS_STATUS_NOT_INITIALIZED;

	EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsFindPrinter()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* protocol			EPS_INT32               I: Protocol to be retrieved                 */
/* timeout          EPS_UINT32              I: Network pirinter find timeout            */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (printer found)                       */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - not initialized                               */
/*      EPS_ERR_JOB_NOT_CLOSED          - JOB is NOT finished                           */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      find USB and Network printer.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsFindPrinter (

        EPS_INT32           protocol, 
        EPS_UINT32          timeout

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus = EPS_ERR_NONE;       /* Return status of internal calls  */

	EPS_LOG_FUNCIN;

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

/*** Is a Job already open                                                              */
    if (printJob.jobStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_JOB_NOT_CLOSED );
    }

/*** Clear inside printer list                                                          */
	prtClearPrinterList();
	printJob.printer = NULL;
	obsClear();

/*** find                                                                               */
	printJob.findStatus = EPS_STATUS_INITIALIZED;
	
	retStatus = prtFindPrinter( protocol, timeout );

	printJob.findStatus = EPS_STATUS_NOT_INITIALIZED;

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsProbePrinter()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* probeParam       EPS_PROBE*              I: prober parameter structure               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success (printer found)                       */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - not initialized                               */
/*      EPS_ERR_JOB_NOT_CLOSED          - JOB is NOT finished                           */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_INV_ARG_PROBEINFO       - Invalid argument probe infomation             */
/*      EPS_ERR_INV_ARG_UNK_METHOD      - Invalid value "method"                        */
/*      EPS_ERR_INV_ARG_PRINTER_ID      - Invalid format "identify"                     */
/*      EPS_ERR_INV_ARG_PRINTER_ADDR    - Invalid format "address"                      */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_PRINTER_NOT_USEFUL      - received but not usefl                        */
/*                                                                                      */
/* Description:                                                                         */
/*      printer specified by printerID or IP Address is retrieved.                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsProbePrinter(

		const EPS_PROBE*   probeParam
		
){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus = EPS_ERR_NONE;       /* Return status of internal calls  */

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
	if (NULL == probeParam) {
		EPS_RETURN( EPS_ERR_INV_ARG_PROBEINFO );
	}

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

/*** Is a Job already open                                                              */
    if (printJob.jobStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_JOB_NOT_CLOSED );
    }

/*** Clear inside printer list                                                          */
	prtClearPrinterList();
	printJob.printer = NULL;
	obsClear();

/*** probe                                                                              */
	printJob.findStatus = EPS_STATUS_INITIALIZED;
	switch( probeParam->method ){
		case EPS_PRB_BYID:
			retStatus = prtProbePrinterByID( probeParam );
			break;

		case EPS_PRB_BYADDR:
			retStatus = prtProbePrinterByAddr( probeParam );
			break;

		default:
			retStatus = EPS_ERR_INV_ARG_UNK_METHOD;
	}
	printJob.findStatus = EPS_STATUS_NOT_INITIALIZED;

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsCancelFindPrinter()                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* N/A              void                                                                */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - not initialized                               */
/*      EPS_ERR_FIND_NOT_STARTED        - find not started                              */
/*      EPS_ERR_INVALID_CALL            - invalid called                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Cancel the epsFindPrinter(), epsProbePrinter() process.                         */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsCancelFindPrinter (

	void

){
	EPS_LOG_FUNCIN;

 /*** Has a Lib been initialized                                                        */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

/*** Has a Find been started                                                            */
    if (printJob.findStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_FIND_NOT_STARTED );
    }
	
	EPS_RETURN( prtCancelFindPrinter() );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsSetPrinter()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* printer          EPS_PRINTER*            O: Pointer to a target printer              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - ESC/P-R Lib is NOT initialized                */
/*      EPS_ERR_JOB_NOT_CLOSED          - JOB is NOT finished                           */
/*      EPS_ERR_INV_ARG_PRINTER         - Illegal printer was specified                 */
/*      EPS_ERR_INV_ARG_COMMMODE        - Invalid argument "printer.protocol"           */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      EPS_ERR_INV_ARG_PRINTER_ADDR    - Invalid format " printer.location"            */
/*      EPS_ERR_INV_PRINT_LANGUAGE      - Invalid argument "printer.language"           */
/*                                                                                      */
/* Description:                                                                         */
/*      Selects a printer to use from detected printers.                                */
/*      Register a user specified printer.				                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsSetPrinter (

        const EPS_PRINTER*      printer

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE		ret = EPS_ERR_NONE;			/* Return status of internal calls  */
	EPS_PRINTER_INN*	innerPrinter = NULL;

EPS_LOG_FUNCIN;

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

/*** Is a Job already open                                                              */
    if (printJob.jobStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_JOB_NOT_CLOSED );
    }

/*** Validate input parameters                                                          */
	if (NULL == printer){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER );
	}

/*** convert internal structure                                                         */
	innerPrinter = prtGetInnerPrinter(printer);
	if( NULL == innerPrinter ){
		ret = prtAddUsrPrinter(printer, &innerPrinter);
		if( EPS_ERR_NONE != ret ){
			EPS_RETURN( ret );
		}
	}

/*** Set Job target printer                                                             */
	printJob.printer	= innerPrinter;
	printJob.bComm		= TRUE;
	obsSetPrinter(innerPrinter);
	prtSetupJobFunctions(printJob.printer, &jobFnc);	/* Set Job functions */

	EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsStartJob()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* jobAttr          const EPS_JOB_ATTRIB*   I: Print Job Attribute                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_ARG_JOB_ATTRIB              - Invalid argument "jobAttr"           */
/*      EPS_ERR_INV_COLOR_PLANE                 - Invalid Color Plane                   */
/*      EPS_ERR_INV_PALETTE_SIZE                - Invalid Palette Size                  */
/*      EPS_ERR_INV_PALETTE_DATA                - Invalid Palette Data                  */
/*      EPS_ERR_INV_MEDIA_SIZE                  - Invalid Media Size                    */
/*      EPS_ERR_INV_MEDIA_TYPE                  - Invalid Media Type                    */
/*      EPS_ERR_INV_BORDER_MODE                 - Invalid Border Mode                   */
/*      EPS_ERR_INV_PRINT_QUALITY               - Invalid Print Quality                 */
/*      EPS_ERR_INV_COLOR_MODE                  - Invalid Color Mode                    */
/*      EPS_ERR_INV_INPUT_RESOLUTION            - Invalid Input Resolution              */
/*      EPS_ERR_INV_PRINT_DIRECTION             - Invalid Print Direction               */
/*      EPS_ERR_INV_BRIGHTNESS                  - Invalid Brightness                    */
/*      EPS_ERR_INV_CONTRAST                    - Invalid Contrast                      */
/*      EPS_ERR_INV_SATURATION                  - Invalid Saturation                    */
/*      EPS_ERR_INV_TOP_MARGIN                  - Invalid Top Magirn                    */
/*      EPS_ERR_INV_LEFT_MARGIN                 - Invalid Left Margin                   */
/*      EPS_ERR_INV_BOTTOM_MARGIN               - Invalid Bottom Margin                 */
/*      EPS_ERR_INV_RIGHT_MARGIN                - Invalid Right Margin                  */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH     - Invalid Magin Setting (Width)         */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT    - Invalid Magin Setting (Height)        */
/*      EPS_ERR_INV_PAPER_SOURCE                - Invalid Paper source                  */
/*      EPS_ERR_INV_DUPLEX                      - Invalid duplex                        */
/*      EPS_ERR_INV_FEED_DIRECTION              - Invalid feed direction                */
/*      EPS_ERR_LANGUAGE_NOT_SUPPORTED          - Unsupported function Error (language) */
/*                                                                                      */
/*      EPS_ERR_INV_APF_FLT                     - Invalid APF Filter                    */
/*      EPS_ERR_INV_APF_ACT                     - Invalid APF Scene                     */
/*      EPS_ERR_INV_APF_SHP                     - Invalid APF Sharpness                 */
/*      EPS_ERR_INV_APF_RDE                     - Invalid APF Red Eye                   */
/*                                                                                      */
/*      (Uni/Bi-Directional)                                                            */
/*      EPS_ERR_LIB_NOT_INITIALIZED     - ESC/P-R Lib is NOT initialized                */
/*      EPS_ERR_JOB_NOT_CLOSED          - JOB is NOT finished                           */
/*      EPS_ERR_PRINTER_NOT_SET         - Target printer is not specified               */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Fail to memory allocation                     */
/*      (Bi-Directional Only)                                                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */

/*                                                                                      */
/* Description:                                                                         */
/*      Creates a new print job.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsStartJob (

        const EPS_JOB_ATTRIB*     jobAttr  /* Print Attributes for this Job            */

){

/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;           /* Return status of internal calls  */
EPS_STATUS_INFO stInfo;

EPS_LOG_FUNCIN;

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
    }

/*** Is a Job already open                                                              */
    if (printJob.jobStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_JOB_NOT_CLOSED );
    }
	
/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SET );
	}

	if(EPS_CP_JPEG == jobAttr->colorPlane ){
		if( !(EPS_SPF_JPGPRINT & printJob.printer->supportFunc) ){
			EPS_RETURN( EPS_ERR_INV_COLOR_PLANE );
		}
	}

/*======================================================================================*/
/*** Setup Page Attribute                                                               */
/*======================================================================================*/
	if (jobAttr == NULL){
        EPS_RETURN( EPS_ERR_INV_ARG_JOB_ATTRIB )
	}

	if(EPS_LANG_ESCPR == printJob.printer->language ){
		/*** ESC/P-R ***/
		retStatus = SetupJobAttrib(jobAttr);
	} else{
		/*** ESC/Page ***/
#ifdef GCOMSW_CMD_ESCPAGE
		retStatus = pageInitJob(jobAttr);
#else
		retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
	} 
	if (EPS_ERR_NONE != retStatus){
		EPS_RETURN( retStatus );
	}

/*** Change Job Status                                                                  */
    printJob.jobStatus = EPS_STATUS_INITIALIZED;
	obsSetColorPlane( printJob.attr.colorPlane );

/*======================================================================================*/
/*** Check the printer status. (before session open)                                    */
/*======================================================================================*/
	memset(&stInfo, 0, sizeof(stInfo));
	if( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		retStatus = PRT_INVOKE_JOBFNC(jobFnc.GetStatus, (&stInfo, NULL, NULL));
		if( EPS_ERR_NONE != retStatus ){
			goto epsStartJob_END;
		} else if(EPS_ST_IDLE != stInfo.nState){
			retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
			goto epsStartJob_END;
		} else if( EPS_PRNWARN_DISABLE_CLEAN & stInfo.nWarn && 
			      EPS_MNT_CLEANING == printJob.attr.cmdType ){
			retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
			goto epsStartJob_END;
		}
		/* ignore color-ink-end warning 
		   if color print done, will occuer EPS_PRNERR_INKOUT(0x47).
		else if( EPS_PRNWARN_COLOR_INKOUT & stInfo.nWarn ){
			if( !((EPS_CP_FULLCOLOR == printJob.attr.colorPlane || EPS_CP_256COLOR == printJob.attr.colorPlane)
				&& EPS_CM_MONOCHROME == printJob.attr.colorMode
				&& EPS_MTID_PLAIN == printJob.attr.mediaTypeIdx
				&& (EPS_MLID_BORDERS == printJob.attr.printLayout || EPS_MLID_CUSTOM == printJob.attr.printLayout)) ){
				 color printing is impossible. (Mono/Plain/Border is possible) 
				retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
				goto epsStartJob_END;
			}
		}*/
	}

/*======================================================================================*/
/*** Prepar RGB buffer                                                                  */
/*======================================================================================*/
	if(EPS_LANG_ESCPR == printJob.printer->language ){
		/*** ESC/P-R ***/
		if( EPS_CP_FULLCOLOR == printJob.attr.colorPlane ||
			EPS_CP_256COLOR  == printJob.attr.colorPlane){
			/*** Allocate buffer for "Send Data" command                                    */
			EPS_SAFE_RELEASE(sendDataBuf );
			EPS_SAFE_RELEASE(tmpLineBuf  );
			
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

		} else if(EPS_CP_JPEG == printJob.attr.colorPlane){
			if(0 == printJob.printer->JpgMax){
				/*** get jpeg limit */
				retStatus = GetJpgMax(printJob.printer);
				if (retStatus != EPS_ERR_NONE) {
					goto epsStartJob_END;
				}
			}
		}
	} else{
#ifdef GCOMSW_CMD_ESCPAGE
		/*** ESC/Page ***/
		retStatus = pageAllocBuffer();
#else
		retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
		if (retStatus != EPS_ERR_NONE) {
			goto epsStartJob_END;
		}
	}

/*======================================================================================*/
/*** protocol depend StartJob                                                           */
/*======================================================================================*/
	retStatus = jobFnc.StartJob();
	if (retStatus != EPS_ERR_NONE) {
		goto epsStartJob_END;
	}

/*======================================================================================*/
/*** Init flags                                                                         */
/*======================================================================================*/
	printJob.transmittable = TRUE;
	printJob.resetSent = EPS_RESET_NOT_SENT;
	printJob.resetReq = FALSE;
	printJob.bComm = TRUE;
	printJob.sendJS = FALSE;
	printJob.pageCount = 0;
	gStatusCount = 0;

	printJob.contData.savePoint = EPS_SAVEP_NONE;
	printJob.contData.saveStep = 0;
	printJob.contData.sendData = NULL;
	printJob.contData.sendDataSize = 0;

/*======================================================================================*/
/*** Send StartJob Commands                                                             */
/*======================================================================================*/
#ifdef LCOMSW_CMDDMP
{
	EPS_INT8 fname[256];
	sprintf(fname, "%s-%s-%s.prn", (printJob.printer->language == EPS_LANG_ESCPR)?"escpr":"page",
		(printJob.attr.colorMode == EPS_CM_COLOR)?"color":"mono",
		(printJob.attr.colorPlane == EPS_CP_FULLCOLOR)?"24":"8");
	EPS_DF_OPEN(fname)
}
#endif

	if( EPS_CP_PRINTCMD != printJob.attr.colorPlane ){
		if(EPS_LANG_ESCPR == printJob.printer->language ){
			/*** ESC/P-R ***/
			if(EPS_PM_JOB != obsGetPageMode()){
				retStatus = SendStartJob(FALSE);
			}
		} else{
			/*** ESC/Page ***/
#ifdef GCOMSW_CMD_ESCPAGE
			retStatus = pageStartJob();
#else
			retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
		}

		if(EPS_ERR_NONE != retStatus){
			goto epsStartJob_END;
		}
	} else{
		printJob.pageStatus = EPS_STATUS_INITIALIZED;
	}

	printJob.jobStatus = EPS_STATUS_ESTABLISHED;

epsStartJob_END:
/*** Return to Caller                                                                   */
	if (EPS_ERR_NOT_OPEN_IO == retStatus || EPS_ERR_COMM_ERROR == retStatus) {
		printJob.bComm = FALSE;
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	}

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsStartPage()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:					Description:                                    */
/* pageAttr     const EPS_JOB_ATTRIB*   I: This Page Attribute                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                            - Success                               */
/*      EPS_JOB_CANCELED                        - Cancelled operation by user           */
/*                                                (Do not return when Uni-Directional)  */
/*      << Error >>                                                                     */
/*      EPS_ERR_JOB_NOT_INITIALIZED             - JOB is NOT initialized                */
/*      EPS_ERR_PAGE_NOT_CLOSED                 - PAGE is NOT closed                    */
/*      EPS_ERR_PRINTER_ERR_OCCUR				- Printer Error happened                */
/*      EPS_ERR_OPR_FAIL                        - Internal Error                        */
/*      EPS_ERR_MEMORY_ALLOCATION               - Failed to allocate memory             */
/*                                                                                      */
/* Description:                                                                         */
/*      Starts the current page.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsStartPage (

        const EPS_PAGE_ATTRIB* pageAttr

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus;                  /* Return status of internal calls          */
EPS_UINT32      retBufSize = 0;
EPS_STATUS_INFO stInfo;

EPS_LOG_FUNCIN;

	(void)pageAttr;		/* unused now */

/*** EPS_CP_PRINTCMD not necessary call epsStartPage()                                  */
    if (EPS_CP_PRINTCMD == printJob.attr.colorPlane) {
        EPS_RETURN( EPS_ERR_NONE );
    }

/*** Has a Job been initialized                                                         */
    if (printJob.jobStatus != EPS_STATUS_ESTABLISHED) {
        EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
    }

/*** Has the Page Status been un-initialized; indicating the last page was closed       */
    if (printJob.pageStatus != EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_PAGE_NOT_CLOSED );
    }

/*** Check the printer status and the stacker error.                                    */
	if ( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
		retStatus = MonitorStatus(&stInfo);

		if( EPS_ERR_NONE == retStatus ){
			if( EPS_PREPARE_TRAYCLOSED == stInfo.nPrepare ){
				retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
			} else if( EPS_PM_JOB == obsGetPageMode() &&
					   EPS_ST_IDLE != stInfo.nState ){
				/* still printing */
				EPS_DBGPRINT(("*** StarPage Wait ***\n"))
				retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
			}
		}
		if( EPS_ERR_NONE != retStatus ){
			if( EPS_ERR_COMM_ERROR != retStatus && 
				EPS_PROTOCOL_USB != EPS_PRT_PROTOCOL(printJob.printer->protocol) )
			{	/* Ignore error, Make buffer full state. */
				EPS_DBGPRINT(("*** Ignore error on StartPage ***\n"))
				retStatus = EPS_ERR_NONE;
			} else{
				goto epsStartPage_END;
			}
		}
	}

	if( EPS_PM_JOB == obsGetPageMode() &&
		EPS_PROTOCOL_LPR == EPS_PRT_PROTOCOL(printJob.printer->protocol) ){
		retStatus = PRT_INVOKE_JOBFNC(jobFnc.StartPage, ());
		if (retStatus != EPS_ERR_NONE) {
			goto epsStartPage_END;
		}
	}
	if( EPS_RESET_SENT == printJob.resetSent ){
		/* If CancelPage is done, restart Job. */
        /*** protocol depend RestartJob                                                 */
		retStatus = PRT_INVOKE_JOBFNC(jobFnc.RestartJob, ());
		if (retStatus != EPS_ERR_NONE) {
			goto epsStartPage_END;
		}
	}

	if(0 == printJob.contData.saveStep){
		if(EPS_LANG_ESCPR == printJob.printer->language ){
			/*** ESC/P-R ***/
			if( EPS_RESET_SENT == printJob.resetSent ){
				/* If CancelPage is done, restart Job complete. */
				printJob.resetSent = EPS_RESET_NOT_SENT;
				/*** Send StartJob & StartPage Commands                                     */
				retStatus = SendStartJob(TRUE);
			} else if( EPS_PM_JOB == obsGetPageMode() ){
				/*** Send StartJob & StartPage Commands                                     */
				retStatus = SendStartJob(TRUE);
			} else{
				/*** Send StartPage Commands                                                */
				EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, sizeof(StartPage) )
				if(NULL == sendDataBuf){
					sendDataBufSize = 0;
					retStatus = EPS_ERR_MEMORY_ALLOCATION;
					goto epsStartPage_END;
				}

				memcpy(sendDataBuf, StartPage, sizeof(StartPage));
				retStatus = SendCommand(sendDataBuf, sizeof(StartPage), &retBufSize, TRUE);
			}
		} else{
			/*** ESC/Page ***/
#ifdef GCOMSW_CMD_ESCPAGE
			retStatus = pageStartPage();
#else
			retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
		}
	} else{
		/* send command retry */
		retStatus = SendLeftovers();
		if(EPS_ERR_INVALID_CALL == retStatus){
			retStatus = EPS_ERR_NONE;
		}
	}
	if(EPS_ERR_NONE == retStatus){
		printJob.contData.saveStep = 0;
	} else{
		printJob.contData.saveStep = 1;
	}

epsStartPage_END:
	if(EPS_ERR_NONE == retStatus){
		/*** Change Page Status                                                         */
		printJob.pageStatus = EPS_STATUS_INITIALIZED;
		printJob.contData.savePoint = EPS_SAVEP_NONE;

		/*** Set Vertical Offset                                                        */
        printJob.verticalOffset = 0;
		printJob.contData.jpgSize = 0;
		printJob.contData.epRetry = 0;
		printJob.contData.skipLine = FALSE;
		printJob.jpegSize = 0;
		printJob.bJpgLimit = FALSE;
		if( printJob.attr.duplex != EPS_DUPLEX_NONE ){
			printJob.needBand = TRUE;
		} else{
			printJob.needBand = FALSE;
		}
		
	} else if(EPS_ERR_COMM_ERROR == retStatus){
		printJob.bComm = FALSE;
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	} else{
		printJob.contData.savePoint = EPS_SAVEP_START_PAGE;
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     epsPrintBand()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* data          EPS_UINT8*         I: Pointer to image [RGB] data                      */
/* widthPixels   EPS_UINT32         I: The width of the raster band (in pixels)         */
/* heightPixels  EPS_UINT32*        I/O: In : Height of image (image lines)  (in pixels)*/
/*                                     : Out: Sent Height                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_OUT_OF_BOUNDS               - Print band is in out of printable area        */
/*      << Error >>                                                                     */
/*      EPS_ERR_PAGE_NOT_INITIALIZED    - Page is NOT initialized                       */
/*      EPS_ERR_INV_ARG_DATA            - Invalid argument "data"                       */
/*      EPS_ERR_INV_ARG_WIDTH_PIXELS    - Invalid argument "widthPixels"                */
/*      EPS_ERR_INV_ARG_HEIGHT_PIXELS   - Invalid argument "heightPixels"               */
/*      EPS_ERR_INV_ARG_DATASIZE		- data size limit								*/
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Prints a band of raster data or Jpeg file data.                                 */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsPrintBand (

        const EPS_UINT8*  data,             /* Pointer to image [RGB] data              */
        EPS_UINT32  widthPixels,            /* Width of image [ in pixels ]             */
        EPS_UINT32  *heightPixels           /* Height of image (image lines)            */

) {
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE    retStatus;
	EPS_PRN_DATA	prnData;

	prnData.version = 1;
	prnData.band.data = data;
	prnData.band.widthPixels = widthPixels;
	prnData.band.heightPixels = *heightPixels;

	retStatus = epsSendData(&prnData);

	*heightPixels = prnData.band.heightPixels;

	return retStatus;
}


EPS_ERR_CODE    epsSendData (

        EPS_PRN_DATA*  prnData				/* Pointer to image [RGB, JPEG] data        */

) {
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;

	EPS_LOG_FUNCIN

/*** Has a Page (which also means Job) been initialized                                 */
    if (printJob.pageStatus == EPS_STATUS_NOT_INITIALIZED) {
        retStatus = EPS_ERR_PAGE_NOT_INITIALIZED;
    }

/*	printJob.pageStatus = EPS_STATUS_PROCCESSING; */

	if( FALSE == printJob.bComm ){
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	}

/*** Validate input parameters                                                          */
	if( NULL == prnData ){
        EPS_RETURN( EPS_ERR_INV_ARG_DATA );
	}

	switch(printJob.attr.colorPlane){
	case EPS_CP_JPEG:
		if(EPS_ERR_NONE == retStatus){
			retStatus = PrintChunk(prnData->chunk.data, &prnData->chunk.dataSize);
		} else{
			prnData->chunk.dataSize = 0;
		}
		break;

	case EPS_CP_FULLCOLOR:
	case EPS_CP_256COLOR:
		if(EPS_ERR_NONE == retStatus){
			retStatus = PrintBand(prnData->band.data, prnData->band.widthPixels, &prnData->band.heightPixels);
		} else{
			prnData->band.heightPixels = 0;
		}
		break;

	case EPS_CP_PRINTCMD:
	default:
		if(EPS_ERR_NONE == retStatus){
			retStatus = SendCommand(prnData->chunk.data, prnData->chunk.dataSize, 
								&prnData->chunk.dataSize, FALSE);
		} else{
			prnData->chunk.dataSize = 0;
		}
		break;
	}

	if(EPS_ERR_COMM_ERROR == retStatus){
		printJob.bComm = FALSE;
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	}

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsEndPage()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nextPage     EPS_BOOL            Next page flag                                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_PAGE_NOT_INITIALIZED    - PAGE is NOT initialized                       */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Ends the current page.                                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsEndPage (

        EPS_BOOL    nextPage

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus= EPS_ERR_NONE;            /* Return status of internal calls  */
EPS_UINT32		retBufSize = 0;
EPS_UINT8*		pCmd = NULL;
EPS_STATUS_INFO stInfo;

EPS_LOG_FUNCIN;

/*** EPS_CP_PRINTCMD not necessary call epsEndPage()                                    */
	if (EPS_CP_PRINTCMD == printJob.attr.colorPlane) {
        EPS_RETURN( EPS_ERR_NONE );
    }

/*** Has a Page (which also means Job) been initialized                                 */
    if (printJob.pageStatus == EPS_STATUS_NOT_INITIALIZED) {
        EPS_RETURN( EPS_ERR_PAGE_NOT_INITIALIZED );
    }

	if( FALSE == printJob.bComm ){
		/*** Change the page status                                                     */
		printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
		/* Do not send "EndPage" commands */
		EPS_RETURN( EPS_ERR_NONE );
	}

/*======================================================================================*/
/*** Check the printer status                                                           */
/*======================================================================================*/
    if ( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
        /* Check the printer status */
		retStatus = MonitorStatus(NULL);
		if( EPS_JOB_CANCELED == retStatus ){
			goto epsEndPage_END;
		} else if(EPS_ERR_COMM_ERROR == retStatus ){
			goto epsEndPage_END;
		}
	}

	/* send command retry */
	retStatus = SendLeftovers();
	if(EPS_ERR_INVALID_CALL == retStatus){
		retStatus = EPS_ERR_NONE;
	}

/*** Send EndPage                                                                       */
	/* step 1 : send dummy band for duplex */
	if( EPS_ERR_NONE == retStatus && 0 == printJob.contData.saveStep ){
		printJob.contData.saveStep++;
		if(  TRUE == printJob.needBand ){
			retStatus = SendBlankBand();
			printJob.needBand = FALSE;
		}
	} 

	/* step 2 : send end page command */
	if( EPS_ERR_NONE == retStatus && 1 == printJob.contData.saveStep ){
		printJob.contData.saveStep++;

		if(EPS_LANG_ESCPR == printJob.printer->language ){
			/*** ESC/P-R ***/
			if( EPS_PM_JOB == obsGetPageMode() ){
				/* EndPage & EndJob */
				retStatus = SendEndJob(TRUE);
			} else{
				/* EndPage */
				EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, sizeof(EndPage) )
				if(NULL == sendDataBuf){
					sendDataBufSize = 0;
					retStatus = EPS_ERR_MEMORY_ALLOCATION;
					goto epsEndPage_END;
				}

				pCmd = sendDataBuf;
				memcpy(pCmd, EndPage, sizeof(EndPage));
				if( FALSE == nextPage ){
					if (EPS_DUPLEX_NONE != printJob.attr.duplex && 
						0 == (printJob.pageCount & 0x01)) {
						nextPage = TRUE;
					}
				}
				if(FALSE == nextPage
					|| EPS_IS_CDDVD( printJob.attr.mediaTypeIdx )){
					pCmd[10] = EPS_END_PAGE;
				} else{ 
					pCmd[10] = EPS_NEXT_PAGE;
				}

				retStatus = SendCommand(sendDataBuf, sizeof(EndPage), &retBufSize, TRUE);
			}
		} else{
			/*** ESC/Page ***/
#ifdef GCOMSW_CMD_ESCPAGE
			retStatus = pageEndPage();
#else
			retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
		}
	}


epsEndPage_END:
/*** Return to Caller                                                                   */
	if(EPS_ERR_NONE == retStatus){
		/*** Change the page status */
		printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
		printJob.pageCount++;
		printJob.contData.savePoint = EPS_SAVEP_NONE;
		printJob.contData.saveStep = 0;

		if( EPS_PM_JOB == obsGetPageMode() &&
			EPS_PROTOCOL_LPR == EPS_PRT_PROTOCOL(printJob.printer->protocol) &&
			printJob.contData.epRetry < 10){
			/* LPR JOB mode */
			retStatus = PRT_INVOKE_JOBFNC(jobFnc.EndPage, ());
			if(EPS_ERR_NONE == retStatus){
				if(EPS_IS_BI_PROTOCOL(printJob.printer->protocol)){
					MonitorStatus(&stInfo);
					if( EPS_ST_IDLE == stInfo.nState ){
						/* retry this function still idle */
						serDelayThread(2*_SECOND_, &epsCmnFnc);
						printJob.contData.epRetry++;
						printJob.contData.savePoint = EPS_SAVEP_END_PAGE;
						printJob.contData.saveStep = 2;
						printJob.pageStatus = EPS_STATUS_INITIALIZED;
						retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
						EPS_DBGPRINT(("*** EndPage Wait ***\n"))
					}
				}
			} else if(EPS_ERR_COMM_ERROR == retStatus){
				printJob.bComm = FALSE;
				retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
			}
		}
	} else if(EPS_ERR_COMM_ERROR == retStatus){
		printJob.bComm = FALSE;
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	} else{
		printJob.contData.savePoint = EPS_SAVEP_END_PAGE;
		printJob.contData.nextPage = nextPage;
	}

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsEndJob()                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A          void                N/A                                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Ends the current print job.                                                     */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsEndJob (

        void

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus;                      /* Return status of internal calls      */
EPS_INT32		nRetry = 0;
EPS_PRINT_JOB   tempPrintJob;
EPS_UINT32		retBufSize = 0;
EPS_BOOL		sendEndpage = FALSE;

EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;

/*** Has a Job been initialized; indicate start job was called                          */
    if (printJob.jobStatus == EPS_STATUS_NOT_INITIALIZED) {
        retStatus = EPS_ERR_JOB_NOT_INITIALIZED;
        goto JOBEND_END;
    }
	
	if (EPS_CP_PRINTCMD != printJob.attr.colorPlane) {
	/*** If not invoked epsEndPage, invoke it                                           */
		if (printJob.pageStatus != EPS_STATUS_NOT_INITIALIZED) {
			epsEndPage(FALSE);
		}

	/*** Send backside page for ESCPR duplex                                            */
		if(EPS_RESET_NOT_SENT == printJob.resetSent &&
		   EPS_DUPLEX_NONE != printJob.attr.duplex && 
		   EPS_LANG_ESCPR == printJob.printer->language && 
		   (printJob.pageCount & 0x01) )
		{
			memcpy(sendDataBuf, StartPage, sizeof(StartPage));
			retStatus = SendCommand(sendDataBuf, sizeof(StartPage), &retBufSize, TRUE);
			for(nRetry = 0; nRetry < 5 && printJob.bComm; nRetry++){
				if(EPS_ERR_NONE == retStatus || EPS_JOB_CANCELED == retStatus){
					break;
				}
				retStatus = SendLeftovers();
				if(EPS_ERR_INVALID_CALL == retStatus){
					retStatus = EPS_ERR_NONE;
				}
			}

			retStatus = SendBlankBand();
			for(nRetry = 0; nRetry < 5 && printJob.bComm; nRetry++){
				if(EPS_ERR_NONE == retStatus || EPS_JOB_CANCELED == retStatus){
					break;
				}
				retStatus = SendLeftovers();
				if(EPS_ERR_INVALID_CALL == retStatus){
					retStatus = EPS_ERR_NONE;
				}
			}

			sendEndpage = TRUE;
		}

	/*** Send EndJob                                                                    */
		if (printJob.jobStatus == EPS_STATUS_ESTABLISHED &&
			EPS_PM_JOB != obsGetPageMode() )
		{
			if(EPS_LANG_ESCPR == printJob.printer->language ){
				/*** ESC/P-R ***/
				retStatus = SendEndJob(sendEndpage);
			} else{
				/*** ESC/Page ***/
#ifdef GCOMSW_CMD_ESCPAGE
				retStatus = pageEndJob();
#else
				retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
			}
		}

		for(nRetry = 0; nRetry < 5 && printJob.bComm; nRetry++){
			if(EPS_ERR_NONE == retStatus){
				break;
			}
		
			EPS_DBGPRINT(("retry ENDJOB\n"))
			retStatus = SendLeftovers();
			if(EPS_ERR_INVALID_CALL == retStatus){
				retStatus = EPS_ERR_NONE;
			}
		}
	}

/*** protocol depend EndJob                                                             */
	retStatus = jobFnc.EndJob();

JOBEND_END:
/*** Clear Memory                                                                       */
#ifdef GCOMSW_CMD_ESCPAGE
	pageRelaseBuffer();
#endif
	EPS_SAFE_RELEASE(sendDataBuf );
    EPS_SAFE_RELEASE(tmpLineBuf  );
	sendDataBufSize = 0;
	tmpLineBufSize = 0;

/*** Reset continue buffer                                                              */
	printJob.contData.sendData = NULL;
	printJob.contData.sendDataSize = 0;

/*** Clear inside additional data buffer                                                */
	/* DEL EPS_SAFE_RELEASE( printJob.qrcode.bits );*/

/*** Clear and Copy Print_Job struct                                                    */
    memcpy(&tempPrintJob, &printJob, sizeof(EPS_PRINT_JOB));
    memset(&printJob, 0, sizeof(EPS_PRINT_JOB));

    printJob.pageStatus       = tempPrintJob.pageStatus;
	printJob.commMode         = tempPrintJob.commMode;
	printJob.printer          = tempPrintJob.printer;
	printJob.platform         = tempPrintJob.platform;

#ifdef GCOMSW_EPSON_SLEEP
    printJob.sleepSteps       = tempPrintJob.sleepSteps;
#endif
    
/*** Change the job status                                                              */
    printJob.jobStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
	printJob.bComm = TRUE;
	printJob.resetReq = FALSE;

#if 0
	if(EPS_ERR_NOT_CLOSE_IO == retStatus || EPS_ERR_COMM_ERROR == retStatus){
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	}
#else
    if (EPS_ERR_NONE != retStatus) {
		EPS_DBGPRINT(("epsEndJob failed (%d)\r\n", retStatus));
		retStatus = EPS_ERR_NONE;
	}
#endif

#ifdef LCOMSW_CMDDMP
	EPS_DF_CLOSE
#endif

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsCancelJob()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* reserve      EPS_BOOL            I:Call epsEndJob()                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_NEED_BIDIRECT           - Need Bi-Directional Communication             */
/*      EPS_ERR_CAN_NOT_RESET           - Failed to reset printer                       */
/*                                                                                      */
/* Description:                                                                         */
/*      Reset printer.                                                                  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsCancelJob (

        EPS_INT32	reserve

){

/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls          */
EPS_ERR_CODE    retTmp = EPS_ERR_NONE;

EPS_LOG_FUNCIN;
#if !LCOMSW_CANCEL_JOB
	(void)reserve;
#endif

/*** Validate communication mode                                                        */
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_NONE/*EPS_ERR_PRINTER_NOT_SET*/ );
	}

	if ( !EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

    if (printJob.jobStatus == EPS_STATUS_ESTABLISHED) {
		/*** Send the reset command to printer                                          */
		retStatus = PRT_INVOKE_JOBFNC(jobFnc.ResetPrinter, ());
		printJob.resetSent = EPS_RESET_SENT;
		printJob.resetReq = FALSE;
		if(EPS_ERR_NONE != retStatus){
			EPS_DBGPRINT(("ResetPrinter() failed (%d)\r\n", retStatus));
			retStatus = EPS_ERR_CAN_NOT_RESET;
			/* return retStatus;  Force continue */
		}

		/*** Change the page status                                                     */
		printJob.pageStatus = EPS_STATUS_NOT_INITIALIZED;
	}

#if LCOMSW_CANCEL_JOB
	if(0 == reserve){
#endif
		/*** Invoke  epsEndJob()                                                        */
		retTmp = epsEndJob();
		if (retTmp != EPS_ERR_NONE) {
			EPS_DBGPRINT(("epsEndJob() failed (%d)\r\n", retTmp));
			/* return retStatus;  Force continue */
		}
#if LCOMSW_CANCEL_JOB
	}
#endif

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsContinueJob()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A          void                N/A                                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_NEED_BIDIRECT           - Need Bi-Directional Communication             */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized                        */
/*      EPS_ERR_PAGE_NOT_INITIALIZED    - PAGR is NOT initialized                       */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Reset printer.                                                                  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsContinueJob (
								   
		void

){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE    retStatus = EPS_ERR_NONE;		/* Return status of internal calls  */
	EPS_INT32       nCnt = 0;

	EPS_LOG_FUNCIN;

/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
        EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}
/*** Has a Job been initialized                                                         */
	if (printJob.jobStatus == EPS_STATUS_NOT_INITIALIZED){
        EPS_RETURN( EPS_ERR_JOB_NOT_INITIALIZED );
	}

/*** Validate communication mode                                                        */
	if( !EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

/*** Recovery command                                                                   */
	switch( printJob.contData.lastError ){
		case EPS_PRNERR_PAPEROUT:
		case EPS_PRNERR_NOTRAY:
		/*case EPS_PRNERR_CDDVDCONFIG:*/
			if( EPS_ERR_PRINTER_ERR_OCCUR == MonitorStatus(NULL)){
				if( EPS_ERR_NONE != (retStatus = prtRecoverPE()) ){
					EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
				}
				/*serDelayThread(2*_SECOND_, &epsCmnFnc);*/

				/* Status change is delayed the condition of LPR ASSIST */
				if( EPS_PROTOCOL_LPR == EPS_PRT_PROTOCOL(printJob.printer->protocol)
					&& EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
					for(nCnt = 0; nCnt < 3; nCnt++){
						serDelayThread(2*_SECOND_, &epsCmnFnc);
						if( EPS_ERR_NONE == MonitorStatus(NULL) ){
							break;
						}
						EPS_DBGPRINT(("*** Recover Wait ***\n"))
					}
				}
			}
			break;

		case EPS_PRNERR_INTERFACE:
			/*serDelayThread(5*_SECOND_, &epsCmnFnc);*/
			break;
		default:
			break;
	}

/*** Check printer starus                                                               */
	retStatus = MonitorStatus(NULL);
	if(EPS_ERR_NONE != retStatus){
		EPS_RETURN( retStatus );
	}

/*** Send leftovers data                                                                */
	switch(printJob.contData.savePoint){
		case EPS_SAVEP_START_PAGE:
			retStatus = epsStartPage(NULL);
			break;
		case EPS_SAVEP_END_PAGE:
			retStatus = epsEndPage(printJob.contData.nextPage);
			break;

		default:
			retStatus = SendLeftovers();
			if(EPS_ERR_INVALID_CALL == retStatus){
				retStatus = EPS_ERR_NONE;
			} else if(EPS_ERR_NONE == retStatus){
				printJob.contData.skipLine = TRUE;
			} else {
				EPS_RETURN( retStatus );
			}
			break;
	}

/*** Return to Caller                                                                   */
	if(EPS_ERR_COMM_ERROR == retStatus){
		printJob.bComm = FALSE;
		retStatus = EPS_ERR_PRINTER_ERR_OCCUR;
	}

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsSetAdditionalData()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* data         EPS_ADD_DATA*       I: pointer to Additional data                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized                        */
/*      EPS_ERR_PAGE_NOT_CLOSED         - PAGR is initialized                           */
/*      EPS_ERR_INV_ARG_VALIDMEMBER     - Invalid argument "validMember"                */
/*      EPS_ERR_INV_ARG_ADDDATA         - Invalid argument "data"                       */
/*      EPS_ERR_INV_ARG_QRSOURCE        - Invalid argument "data.qrcode.source"         */
/*      EPS_ERR_INV_ARG_QRXPOS	        - Invalid argument "data.qrcode.xPos"           */
/*      EPS_ERR_INV_ARG_QRYPOS	        - Invalid argument "data.qrcode.position.y"     */
/*      EPS_ERR_QRSOURCE_TOO_LAGE       - source is too lage to convert QR code         */
/*                                                                                      */
/* Description:                                                                         */
/*      Set additional data print image.                                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsSetAdditionalData (
								   
		EPS_UINT32	dataType,
		const void*	data

){
/*** Declare Variable Local to Routine                                                  */
	EPS_LOG_FUNCIN;
	 /* This Func is ineffective in the current version. */
	EPS_RETURN( EPS_ERR_INVALID_CALL );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsRemAdditionalData()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* member       EPS_UINT32          I: pointer to Additional data                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_ARG_VALIDMEMBER     - Invalid argument "validMember"                */
/*                                                                                      */
/* Description:                                                                         */
/*      Release additional data setting by epsRemAdditionalData().                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsRemAdditionalData (
								   
		EPS_UINT32 dataType

){
	EPS_LOG_FUNCIN;
	 /* This Func is ineffective in the current version. */
	EPS_RETURN( EPS_ERR_INVALID_CALL );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsGetStatus()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* status       EPS_STATUS*         Pointer to the printer status.                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_NEED_BIDIRECT           - Need Bi-Directional Communication             */
/*      EPS_ERR_PRINTER_NOT_SET         - Target printer is not specified               */
/*      EPS_ERR_INV_ARG_STATUS          - Invalid argument                              */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Gets the printer status.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsGetStatus (

		EPS_STATUS			*status

){

/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE    ret = EPS_ERR_NONE;         /* Return status of internal calls      */
	EPS_STATUS_INFO	stInfo;
    EPS_BOOL		bIoStatus = FALSE;
	EPS_BOOL		bCanceling = FALSE;

	EPS_LOG_FUNCIN;

/*** Validate communication mode                                                        */
	if( !EPS_IS_BI_PROTOCOL(printJob.commMode) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
		EPS_RETURN(  EPS_ERR_PRINTER_NOT_SET );
	}

/*** Validate input parameters                                                          */
	if (status == NULL){
        EPS_RETURN( EPS_ERR_INV_ARG_STATUS );
	}
	
	memset(status, 0, sizeof(EPS_STATUS));

/*** Jpeg size limitation                                                               */
	if( TRUE == printJob.bJpgLimit ){
		status->printerStatus = EPS_PRNST_ERROR;
		status->errorCode = EPS_PRNERR_JPG_LIMIT;
		status->jobContinue  = FALSE;
		EPS_RETURN( EPS_ERR_NONE );
	}

/*** Get printer status                                                                 */
	memset(&stInfo, 0, sizeof(stInfo));
	if( TRUE == printJob.bComm ){
		ret = PRT_INVOKE_JOBFNC(jobFnc.GetStatus, 
								(&stInfo, &bIoStatus, &bCanceling));

		if(EPS_ERR_COMM_ERROR == ret && 
			EPS_STATUS_NOT_INITIALIZED != printJob.jobStatus){
			printJob.bComm = FALSE;
		}
	} else{
		ret = EPS_ERR_COMM_ERROR;
	}

//	EPS_DBGPRINT((" State\t: %d\n Error\t: %d\n IoStatus\t: %d\n Canceling\t: %d\n Prepare\t: %d\n", 
//		stInfo.nState, stInfo.nError, bIoStatus, bCanceling, stInfo.nPrepare))

	if(EPS_ERR_COMM_ERROR == ret	||
		EPS_ERR_NOT_OPEN_IO == ret	||
		EPS_ERR_NOT_CLOSE_IO == ret	) {
		status->printerStatus = EPS_PRNST_ERROR/*EPS_PRNST_OFFLINE*/;
		status->errorCode = EPS_PRNERR_COMM;
		status->jobContinue  = FALSE;
		EPS_RETURN( EPS_ERR_NONE );
	} else if (EPS_ERR_NONE != ret){
		EPS_RETURN( ret );
	}

   if(stInfo.nCancel == EPS_CAREQ_CANCEL) {
        EPS_DBGPRINT(("EPS SER : CANCEL REQUEST by PRINTER\r\n"));
		printJob.resetReq = TRUE;
    }
#if 0
   if(TRUE == printJob.resetReq){
		status->printerStatus = EPS_PRNST_ERROR/*EPS_PRNST_OFFLINE*/;
		status->errorCode = EPS_PRNERR_COMM;
		status->jobContinue  = FALSE;
   }
#endif

/*** Copy status data to input buffer                                                   */
	status->errorCode= stInfo.nError;
	printJob.contData.lastError = stInfo.nError;
	switch(stInfo.nState) {
    	case EPS_ST_ERROR:
			status->printerStatus = EPS_PRNST_ERROR;

			switch(stInfo.nError){
				case EPS_PRNERR_INKOUT:
					/* Convert Ink error */
					serGetInkError(&stInfo, &status->errorCode);
					break;

				case EPS_PRNERR_INTERFACE:
				case EPS_PRNERR_BUSY:
					if(EPS_STATUS_ESTABLISHED == printJob.jobStatus){
					/* between multi page(job of EPS_PM_JOB), it becomes busy. */
						status->printerStatus = EPS_PRNST_PRINTING;
						status->errorCode = EPS_PRNERR_NOERROR;
					} else{
						status->printerStatus = EPS_PRNST_BUSY;
					}

					break;

				case EPS_PRNERR_PAPERJAM:
					printJob.transmittable = FALSE;
					break;

				default:
					break;
			}
			break;

		case EPS_ST_WAITING:
			status->printerStatus = EPS_PRNST_PRINTING;
			break;

		case EPS_ST_IDLE:
			/* If printer doesn't move, it becomes idle. */
			if(EPS_STATUS_ESTABLISHED == printJob.jobStatus){
				status->printerStatus = EPS_PRNST_PRINTING;
			} else{
				status->printerStatus = EPS_PRNST_IDLE;
			}
			break;

		case EPS_ST_FACTORY_SHIPMENT:
			status->printerStatus = EPS_PRNST_ERROR;
			status->errorCode = EPS_PRNERR_FACTORY;	
			break;

		case EPS_ST_CLEANING:
		case EPS_ST_BUSY:
			if(EPS_STATUS_ESTABLISHED == printJob.jobStatus){
				/* The cleaning under the print is made "printing". */
				status->printerStatus = EPS_PRNST_PRINTING;
				status->errorCode = EPS_PRNERR_NOERROR;
			} else{
				status->errorCode = EPS_PRNERR_BUSY;
				status->printerStatus = EPS_PRNST_BUSY;
			}
			break;

		default: /* The others. */
			/* EPS_ST_SELF_PRINTING    */
			/* EPS_ST_SHUTDOWN         */
	        status->errorCode = EPS_PRNERR_BUSY;
			status->printerStatus = EPS_PRNST_BUSY;
			break;
	}

	if(EPS_PRNERR_NOERROR == status->errorCode){
		/* If other errors have not occurred */
		if(EPS_PREPARE_TRAYCLOSED == stInfo.nPrepare
			&& EPS_STATUS_ESTABLISHED == printJob.jobStatus
			&& EPS_STATUS_NOT_INITIALIZED == printJob.pageStatus){
			/* between epsStartJob to epsStartPage complete */
			status->errorCode = EPS_PRNERR_TRAYCLOSE;
			status->printerStatus = EPS_PRNST_ERROR;

		} else if(EPS_PRNWARN_DISABLE_CLEAN & stInfo.nWarn 
			&& EPS_MNT_CLEANING == printJob.attr.cmdType
			&& EPS_STATUS_INITIALIZED == printJob.jobStatus){
			/* GetStatus failed in epsStartJob()  */
			status->errorCode = EPS_PRNERR_DISABEL_CLEANING;
			status->printerStatus = EPS_PRNST_ERROR;

		} else if( bCanceling ){
			/* Printing has been canceled by the cancel request which had been triggered 
			 * by either the epsonPrintCancelJob/Page() API 
			 * or a user operation on the printer panel. */
			status->printerStatus = EPS_PRNST_CANCELLING;
			if(EPS_PRNERR_NOERROR == status->errorCode){
				status->errorCode = EPS_PRNERR_BUSY;
			}
		} else if( !bIoStatus ) {
			/* ESC/P-R Library had sent all that is needed to print 
			 * to the printer but the printer has not been completed printing yet. */
			status->printerStatus = EPS_PRNST_BUSY;
			if(EPS_PRNERR_NOERROR == status->errorCode){
				status->errorCode = EPS_PRNERR_BUSY;
			}
		} 
		/* ignore color-ink-end warning 
		else if( EPS_PRNWARN_COLOR_INKOUT & stInfo.nWarn
			&& EPS_STATUS_ESTABLISHED != printJob.jobStatus ){
			 color ink out is error, when besides job. 
			status->errorCode = EPS_PRNERR_INKOUT;
			status->printerStatus = EPS_PRNST_ERROR;
		}*/
	}

/*** Set continue flag                                                              */
	status->jobContinue = TRUE;
	if( EPS_STATUS_ESTABLISHED != printJob.jobStatus ){
        status->jobContinue = FALSE;
	} else{
		switch(status->errorCode){
		/* The error that it can not continue the printing job */
		case EPS_PRNERR_GENERAL:
		case EPS_PRNERR_FATAL:
		case EPS_PRNERR_PAPERJAM:
		/*case EPS_PRNERR_SIZE_TYPE_PATH:*/
		case EPS_PRNERR_SERVICEREQ:
		case EPS_PRNERR_CARDLOADING:
		case EPS_PRNERR_BATTERYVOLTAGE:
		case EPS_PRNERR_BATTERYTEMPERATURE:
		case EPS_PRNERR_BATTERYEMPTY:
        case EPS_PRNERR_COMM:
			status->jobContinue = FALSE;
			break;

		case EPS_PRNERR_BUSY:
			if(EPS_PRNST_CANCELLING == status->printerStatus){
			/* When last time page printing.
			   It's state EPS_PRNST_CANCELLING & EPS_PRNERR_BUSY.*/
				status->jobContinue = TRUE;
			} else{
				status->jobContinue = FALSE;
			}
			break;
		}
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsGetInkInfo()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* status       EPS_STATUS*         Pointer to the printer status.                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_NEED_BIDIRECT           - Need Bi-Directional Communication             */
/*      EPS_ERR_PRINTER_NOT_SET         - Target printer is not specified               */
/*      EPS_ERR_INV_ARG_INKINFO         - Invalid argument                              */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Gets the Ink Infomation.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsGetInkInfo (

		EPS_INK_INFO			*info

){
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE    ret = EPS_ERR_NONE;         /* Return status of internal calls      */

	EPS_LOG_FUNCIN;

/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SET );
	}

/*** Validate input parameters                                                          */
	if (info == NULL){
        EPS_RETURN( EPS_ERR_INV_ARG_INKINFO );
	}

	memset(info, 0, sizeof(EPS_INK_INFO));

/*** protocol GetStatus                                                                 */
	ret = prtGetInkInfo(printJob.printer, info);

/*** Return to Caller                                                                   */

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsGetSupportedMedia()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* supportedMedia   EPS_SUPPORTED_MEDIA*    Pointer to Supported Media Structure        */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_NEED_BIDIRECT           - Need Bi-Directional Communication             */
/*      EPS_ERR_PRINTER_NOT_SET         - Target printer is not specified               */
/*      EPS_ERR_INV_ARG_SUPPORTED_MEDIA - Invalid argument                              */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_NOT_OPEN_IO             - Cannot Open I/O Portal                        */
/*      EPS_ERR_NOT_CLOSE_IO            - Cannot Close I/O Portal                       */
/*      EPS_ERR_PROTOCOL_NOT_SUPPORTED  - Unsupported function Error                    */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Get supported media information from printer and save those data in             */
/*      "g_supportedMedia" structure.                                                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE     epsGetSupportedMedia (

        EPS_SUPPORTED_MEDIA*    supportedMedia

){

/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus, retGetPM;        /* Return status of internal calls      */
    EPS_UINT8       pmString[EPS_PM_MAXSIZE];   /* Retrieved PM data from printer       */
    EPS_INT32       pmSize = EPS_PM_MAXSIZE;

	EPS_PRINTER_INN* innerPrinter = NULL;

	EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    retStatus = retGetPM = EPS_ERR_NONE;

/*** Validate communication mode                                                        */
	if( !EPS_IS_BI_PROTOCOL(printJob.commMode) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SET );
	}
	innerPrinter = printJob.printer;

	if( !EPS_IS_BI_PROTOCOL(innerPrinter->protocol) ){
        EPS_RETURN( EPS_ERR_NEED_BIDIRECT );
	}

/*** Validate input parameters                                                          */
	if (supportedMedia == NULL){
		EPS_RETURN( EPS_ERR_INV_ARG_SUPPORTED_MEDIA );
    }

#if !_VALIDATE_SUPPORTED_MEDIA_DATA_
/*** If already exist, return current value                                             */
	if(innerPrinter->supportedMedia.numSizes > 0 
		&& NULL != innerPrinter->supportedMedia.sizeList){
		EPS_RETURN( DuplSupportedMedia(innerPrinter, supportedMedia) );
	}
#endif

	/* Clear the prev value                                                             */
	prtClearSupportedMedia(innerPrinter);

	/* Clear the Printer Model Information (Media data or "PM" data)                    */
	memset(pmString,0,EPS_PM_MAXSIZE);

	/*** Get PM from Printer                                                            */
	retStatus = prtGetPMString(innerPrinter, 1, pmString, &pmSize);

	/*** ESC/Page divergence ***/
	if(EPS_LANG_ESCPR != innerPrinter->language ){
#ifdef GCOMSW_CMD_ESCPAGE
		if( EPS_ERR_NONE == retStatus ) {
			/*** Create Media Infomation                                                */
			retStatus = pageCreateMediaInfo(innerPrinter, pmString, pmSize);
		}
#else
		retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif

	} else {
		if ( EPS_ERR_NONE == retStatus ){
			/*** Create Media Infomation                                                */
			retStatus = CreateMediaInfo(innerPrinter, pmString, pmSize);
		}
	}

/*** Copy the supproted media information from "printer" structure to input buffer      */
	if ( EPS_ERR_NONE == retStatus ){
		/* Copy to input buffer                                                         */
		retStatus = DuplSupportedMedia(innerPrinter, supportedMedia);
	}
/*** Return to Caller                                                                   */
    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsGetPrintableArea()                                               */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* jobAttr          const EPS_JOB_ATTRIB*   I: Print Job Attribute                      */
/* printableWidth   EPS_INT32*              O: Printable area width.                    */
/* printableHeight  EPS_INT32*              O: Printable area height.                   */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                            - Success                               */
/*      << Error >>                                                                     */
/*      EPS_ERR_LIB_NOT_INITIALIZED             - ESC/P-R Lib is NOT initialized        */
/*      EPS_ERR_INV_ARG_JBO_ATTRIB              - Invalid argument "jobAttr"            */
/*      EPS_ERR_INV_ARG_PRINTABLE_WIDTH         - Invalid argument "printableWidth"     */
/*      EPS_ERR_INV_ARG_PRINTABLE_HEIGHT        - Invalid argument "printableHeight"    */
/*      EPS_ERR_INV_MEDIA_SIZE                  - Invalid Media Size                    */
/*      EPS_ERR_INV_BORDER_MODE                 - Invalid Border Mode                   */
/*      EPS_ERR_INV_INPUT_RESOLUTION            - Invalid Input Resolution              */
/*      EPS_ERR_INV_TOP_MARGIN                  - Invalid Top Magirn                    */
/*      EPS_ERR_INV_LEFT_MARGIN                 - Invalid Left Margin                   */
/*      EPS_ERR_INV_BOTTOM_MARGIN               - Invalid Bottom Margin                 */
/*      EPS_ERR_INV_RIGHT_MARGIN                - Invalid Right Margin                  */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH     - Invalid Magin Setting (Width)         */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT    - Invalid Magin Setting (Height)        */
/*      EPS_ERR_OPR_FAIL                        - Internal Error                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Gets the printable area of the image.                                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    epsGetPrintableArea (

        EPS_JOB_ATTRIB*     jobAttr,
        EPS_UINT32*         printableWidth,
        EPS_UINT32*         printableHeight

){

/*** Declare Variable Local to Routine                                                  */
EPS_INT32       idx;                                /* General loop/index varaible      */
EPS_ERR_CODE    retStatus;                          /* Return status of internal calls  */
EPS_INT32       factor;                             /* Scaling factor for dpi           */
EPS_INT32       tempPrintableWidth;
EPS_INT32       tempPrintableHeight;
const EPS_MEDIA_INFO* pMI = NULL;
EPS_INT16		minCustomBorder;

EPS_LOG_FUNCIN;

/*** Has a Lib been initialized                                                         */
    if (libStatus != EPS_STATUS_INITIALIZED) {
        EPS_RETURN( EPS_ERR_LIB_NOT_INITIALIZED );
	}
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SET );
	}

/*** Validate input parameters                                                          */
    if (jobAttr == NULL)
        EPS_RETURN( EPS_ERR_INV_ARG_JOB_ATTRIB );
    
    if (printableWidth == NULL)
        EPS_RETURN( EPS_ERR_INV_ARG_PRINTABLE_WIDTH );
        
    if (printableHeight == NULL)
        EPS_RETURN( EPS_ERR_INV_ARG_PRINTABLE_HEIGHT );

	/*** ESC/Page divergence ***/
	if( EPS_LANG_ESCPR != printJob.printer->language ){
#ifdef GCOMSW_CMD_ESCPAGE
		retStatus = pageGetPrintableArea(jobAttr, printableWidth, printableHeight);
#else
		retStatus = EPS_ERR_LANGUAGE_NOT_SUPPORTED;
#endif
		EPS_RETURN( retStatus );
	}

/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;

/*** Validate/Confirm Page Attribute Data                                               */
    /*** Media Size                                                                     */
    if (! ( ( (jobAttr->mediaSizeIdx       >= EPS_MSID_A4              ) &&
              (jobAttr->mediaSizeIdx       <= EPS_MSID_HIVISION        )    ) ||
            ( (jobAttr->mediaSizeIdx       >= EPS_MSID_A3NOBI          ) &&
              (jobAttr->mediaSizeIdx       <= EPS_MSID_12X12           )    ) ||
            ( (jobAttr->mediaSizeIdx       == EPS_MSID_USER            )    )    ) )
        EPS_RETURN( EPS_ERR_INV_MEDIA_SIZE );

#if 0   /* These parameter are not used in this function */
	/*** Media Type                                                                     */
    if (! ( ( (jobAttr->mediaTypeIdx       >= EPS_MTID_PLAIN           ) &&
/*              (jobAttr->mediaTypeIdx       <= EPS_MTID_GLOSSYHAGAKI    )    ) ||*/
              (jobAttr->mediaTypeIdx       <= EPS_MTID_BUSINESSCOAT    )    ) ||
            ( (jobAttr->mediaTypeIdx       >= EPS_MTID_CDDVD           ) &&
              (jobAttr->mediaTypeIdx       <= EPS_MTID_CDDVDGLOSSY     )    ) ||
            ( (jobAttr->mediaTypeIdx       == EPS_MTID_CLEANING        )    )    ) )
        EPS_RETURN( EPS_ERR_INV_MEDIA_TYPE );
#endif

    /*** Border Mode                                                                    */
    if (! (   (jobAttr->printLayout        == EPS_MLID_BORDERLESS      ) ||
              (jobAttr->printLayout        == EPS_MLID_BORDERS         ) ||
              (jobAttr->printLayout        == EPS_MLID_CDLABEL         ) ||
              (jobAttr->printLayout        == EPS_MLID_DIVIDE16        ) ||
              (jobAttr->printLayout        == EPS_MLID_CUSTOM          )    ) )
        EPS_RETURN( EPS_ERR_INV_BORDER_MODE );

#if 0   /* These parameter are not used in this function */
    /*** Print Quality                                                                  */
    if (! (   (jobAttr->printQuality       == EPS_MQID_DRAFT           ) ||
              (jobAttr->printQuality       == EPS_MQID_NORMAL          ) ||
              (jobAttr->printQuality       == EPS_MQID_HIGH            )    ) )
        EPS_RETURN( EPS_ERR_INV_PRINT_QUALITY );

    /*** Color Mode                                                                     */
    if (! (   (jobAttr->colorMode          == EPS_CM_COLOR             ) ||
              (jobAttr->colorMode          == EPS_CM_MONOCHROME        )    ) )
        EPS_RETURN( EPS_ERR_INV_COLOR_MODE );
#endif

    /*** Input Image Resolution                                                         */
    /*** Select table and factor                                                        */
	if(jobAttr->inputResolution == EPS_IR_360X360){
		pMI = epsMediaSize;
	    factor = 1;
		minCustomBorder = EPS_BORDERS_MARGIN_360;
	} else if(jobAttr->inputResolution == EPS_IR_720X720){
		pMI = epsMediaSize;
		factor = 2;
		minCustomBorder = EPS_BORDERS_MARGIN_360;
	} else if(jobAttr->inputResolution == EPS_IR_300X300){
		pMI = epsMediaSize300;
	    factor = 1;
		minCustomBorder = EPS_BORDERS_MARGIN_300;
	} else if(jobAttr->inputResolution == EPS_IR_600X600){
		pMI = epsMediaSize300;
	    factor = 2;
		minCustomBorder = EPS_BORDERS_MARGIN_300;
	} else{
        EPS_RETURN( EPS_ERR_INV_INPUT_RESOLUTION )
	}

#if 0   /* These parameter are not used in this function */
    /*** Printing Direction                                                             */
    if (! (   (jobAttr->printDirection     == EPS_PD_BIDIREC           ) ||
              (jobAttr->printDirection     == EPS_PD_UNIDIREC          )    ) )
        EPS_RETURN( EPS_ERR_INV_PRINT_DIRECTION );

    /*** Color Plane                                                                    */
    if (! (   (jobAttr->colorPlane         == EPS_CP_FULLCOLOR         ) ||
              (jobAttr->colorPlane         == EPS_CP_256COLOR          )    ) )
        EPS_RETURN( EPS_ERR_INV_COLOR_PLANE );

    /*** Pallette Data                                                                  */
    if (jobAttr->colorPlane == EPS_CP_256COLOR) {
        if (! ((jobAttr->paletteSize       >= 3                        ) &&
               (jobAttr->paletteSize       <= 768/*765*/               )    ) )
            EPS_RETURN( EPS_ERR_INV_PALETTE_SIZE );
        if (    jobAttr->paletteData       == NULL                     )
            EPS_RETURN( EPS_ERR_INV_PALETTE_DATA );
    }

    /*** Brightness                                                                     */
    if (! (   (jobAttr->brightness         >= -50                      ) &&
              (jobAttr->brightness         <=  50                      )    ) )
        EPS_RETURN( EPS_ERR_INV_BRIGHTNESS );

    /*** Contrast                                                                       */
    if (! (   (jobAttr->contrast           >= -50                      ) &&
              (jobAttr->contrast           <=  50                      )    ) )
        EPS_RETURN( EPS_ERR_INV_CONTRAST );

    /*** Saturation                                                                     */
    if (! (   (jobAttr->saturation         >= -50                      ) &&
              (jobAttr->saturation         <=  50                      )    ) )
        EPS_RETURN( EPS_ERR_INV_SATURATION );
#endif

    /*** Margin                                                                         */
    if (jobAttr->printLayout == EPS_MLID_CUSTOM) {
        if (jobAttr->topMargin    < minCustomBorder*factor) EPS_RETURN( EPS_ERR_INV_TOP_MARGIN );
        if (jobAttr->leftMargin   < minCustomBorder*factor) EPS_RETURN( EPS_ERR_INV_LEFT_MARGIN );
        if (jobAttr->bottomMargin < minCustomBorder*factor) EPS_RETURN( EPS_ERR_INV_BOTTOM_MARGIN );
        if (jobAttr->rightMargin  < minCustomBorder*factor) EPS_RETURN( EPS_ERR_INV_RIGHT_MARGIN );
    }

#if 0   /* Don't need this logic */
    /*** If full-color mode, nullify the 256-color parameters                           */
    printJob.bpp = 1;
    if (jobAttr->colorPlane == EPS_CP_FULLCOLOR) {
        jobAttr->paletteSize = 0;
        jobAttr->paletteData = NULL;
        printJob.bpp          = 3;
    }
#endif

/*** Get Printable Area                                                                 */
    /*** Find the Media by ID                                                           */
    for (idx = 0; pMI[idx].id != -1; idx++) {
        if (pMI[idx].id == jobAttr->mediaSizeIdx)
            break;
    }
    if (pMI[idx].id == -1) {
        EPS_RETURN( EPS_ERR_INV_MEDIA_SIZE );
    }

    /*** Initialize Printable based on printLayout                                      */
    switch( jobAttr->printLayout ){
	case EPS_MLID_BORDERLESS:
        tempPrintableWidth  = pMI[idx].print_area_x_borderless * factor;
        tempPrintableHeight = pMI[idx].print_area_y_borderless * factor;
		break;

	case EPS_MLID_BORDERS:
	case EPS_MLID_DIVIDE16:	/* layout processing is not done. */
        tempPrintableWidth  = pMI[idx].print_area_x_border * factor;
        tempPrintableHeight = pMI[idx].print_area_y_border * factor;
		break;

	case EPS_MLID_CDLABEL:
		if( !( (jobAttr->cdDimOut >= EPS_CDDIM_IN_MIN   )
			&& (jobAttr->cdDimOut <= EPS_CDDIM_OUT_MAX ) ) ){
			EPS_RETURN( EPS_ERR_INV_CD_OUTDIM );
		}

		tempPrintableWidth  =
			tempPrintableHeight = elGetDots(jobAttr->inputResolution, jobAttr->cdDimOut);
		break;

	default: /* printLayout  == EPS_MLID_CUSTOM */
        tempPrintableWidth  = pMI[idx].paper_x *factor -
                              jobAttr->leftMargin      -
                              jobAttr->rightMargin;
        tempPrintableHeight = pMI[idx].paper_y *factor -
                              jobAttr->topMargin       -
                              jobAttr->bottomMargin;
    }

/*** Validate/Confirm Magin Setting                                                     */
    if (jobAttr->printLayout == EPS_MLID_CUSTOM) {
        if (tempPrintableWidth  <= 0) EPS_RETURN( EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH );
        if (tempPrintableHeight <= 0) EPS_RETURN( EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT );
    }

/*** Set Printable Area to input parameter                                              */
    if ((tempPrintableWidth > 0) && (tempPrintableHeight > 0)) {
        *printableWidth  = (EPS_UINT32)tempPrintableWidth;
        *printableHeight = (EPS_UINT32)tempPrintableHeight;
    } else{
        EPS_RETURN( EPS_ERR_OPR_FAIL );
    }
    
/*** Return to Caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   epsMakeMainteCmd()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* cmd          EPS_INT32           I:maintenance command type.                         */
/* buffer       EPS_UINT8*          I:pointer to command buffer.                        */
/* buffersize   EPS_UINT32*         I: buffer size.                                     */
/*                                  O: need size.                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      << Error >>                                                                     */
/*      EPS_ERR_LIB_NOT_INITIALIZED		- ESC/P-R Lib is NOT initialized				*/
/*      EPS_ERR_INV_ARG_CMDTYPE			- Invalid argument "cmd"						*/
/*      EPS_ERR_OPR_FAIL				- Internal Error								*/
/*      EPS_ERR_MEMORY_ALLOCATION		- Failed to allocate memory						*/
/*      EPS_ERR_PRINTER_NOT_SET         - Target printer is not specified               */
/*      EPS_ERR_LANGUAGE_NOT_SUPPORTED  - Unsupported function Error (language)         */
/*                                                                                      */
/* Description:                                                                         */
/*      Make maintenance command.                                                       */
/*      Call this function as specifying NULL to buffer, so that the memory size        */
/*      necessary for executing the maintenance command is returned to buffersize.      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE epsMakeMainteCmd     (

		EPS_INT32		cmd,
		EPS_UINT8*		buffer,
		EPS_UINT32*		buffersize

){
#ifdef GCOMSW_EF_MAINTE
/*** Declare Variable Local to Routine                                                  */
	EPS_ERR_CODE retStatus = EPS_ERR_NONE;      /* Return status of internal calls      */
	EPS_UINT8 *pCmdBuf = NULL;
	EPS_UINT8 *pCmdPos = NULL;
	EPS_UINT32 cmdSize = 0;
	EPS_UINT32 cmdBufSize = 0;

	EPS_LOG_FUNCIN;

/*** Has a target printer specified                                                     */
	if(NULL == printJob.printer){
		EPS_RETURN( EPS_ERR_PRINTER_NOT_SET );
	}

	if(EPS_LANG_ESCPR != printJob.printer->language ){
		EPS_RETURN( EPS_ERR_LANGUAGE_NOT_SUPPORTED );
	}


	if(buffer){
		cmdBufSize = 256;
		pCmdBuf = (EPS_UINT8*)EPS_ALLOC(cmdBufSize);
		pCmdPos = pCmdBuf;
	}

#define MakeMainteCmd_ADDCMD(CMD) {														\
		if(buffer){																		\
			retStatus = AddCmdBuff(&pCmdBuf, &pCmdPos, &cmdBufSize, CMD, sizeof(CMD));	\
			if(EPS_ERR_NONE != retStatus){												\
				goto epsMakeMainteCmd_END;												\
			}																			\
		}																				\
		cmdSize += sizeof(CMD);															\
	}

	switch(cmd){
	case EPS_MNT_NOZZLE:				/* nozzle check */
		MakeMainteCmd_ADDCMD(ExitPacketMode)
		MakeMainteCmd_ADDCMD(InitPrinter)
		MakeMainteCmd_ADDCMD(InitPrinter)

		MakeMainteCmd_ADDCMD(EnterRemoteMode)
		if(epsCmnFnc.getLocalTime){
			MakeMainteCmd_ADDCMD(RemoteTI)
			if( buffer ){
				MakeRemoteTICmd(pCmdPos - sizeof(RemoteTI));
			}
		}
		MakeMainteCmd_ADDCMD(RemoteJS)
		MakeMainteCmd_ADDCMD(RemoteNC)
		if(pCmdPos && obsIsA3Model(EPS_MDC_NOZZLE)){
			*(pCmdPos - sizeof(RemoteNC) + 5) = 0x10;
		}
		MakeMainteCmd_ADDCMD(ExitRemoteMode)

		MakeMainteCmd_ADDCMD(DataCR)
		MakeMainteCmd_ADDCMD(DataLF)
		MakeMainteCmd_ADDCMD(DataCR)
		MakeMainteCmd_ADDCMD(DataLF)
		
		MakeMainteCmd_ADDCMD(EnterRemoteMode)
		MakeMainteCmd_ADDCMD(RemoteVI)
		MakeMainteCmd_ADDCMD(RemoteLD)
		MakeMainteCmd_ADDCMD(ExitRemoteMode)

		MakeMainteCmd_ADDCMD(DataFF)
		MakeMainteCmd_ADDCMD(InitPrinter)
		MakeMainteCmd_ADDCMD(InitPrinter)
		
		MakeMainteCmd_ADDCMD(EnterRemoteMode)
		MakeMainteCmd_ADDCMD(RemoteJE)
		MakeMainteCmd_ADDCMD(ExitRemoteMode)
		break;

	case EPS_MNT_CLEANING:					/* head cleaning */
		MakeMainteCmd_ADDCMD(ExitPacketMode)
		MakeMainteCmd_ADDCMD(InitPrinter)
		MakeMainteCmd_ADDCMD(InitPrinter)

		MakeMainteCmd_ADDCMD(EnterRemoteMode)
		if(epsCmnFnc.getLocalTime){
			MakeMainteCmd_ADDCMD(RemoteTI)
			if( buffer ){
				MakeRemoteTICmd(pCmdPos - sizeof(RemoteTI));
			}
		}
		MakeMainteCmd_ADDCMD(RemoteCH)
		MakeMainteCmd_ADDCMD(ExitRemoteMode)

		MakeMainteCmd_ADDCMD(InitPrinter)
		MakeMainteCmd_ADDCMD(InitPrinter)
		
		MakeMainteCmd_ADDCMD(EnterRemoteMode)
		MakeMainteCmd_ADDCMD(RemoteJE)
		MakeMainteCmd_ADDCMD(ExitRemoteMode)
		break;

	default:
		retStatus = EPS_ERR_INV_CMDTYPE;
		break;
	}

epsMakeMainteCmd_END:
	if(EPS_ERR_NONE == retStatus){
		if( buffer ){
			if(*buffersize > 0){
				memcpy(buffer, pCmdBuf, Min(*buffersize, cmdSize));
			}
		} 
		*buffersize = cmdSize;
	}
	EPS_SAFE_RELEASE(pCmdBuf);

	EPS_RETURN( retStatus );
#else
	return EPS_ERR_INVALID_CALL;
#endif
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------             Internal Processing             ---------------------*/
/*--------------------                     for                     ---------------------*/
/*--------------------                     API                     ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   MonitorStatus()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pStInfo      EPS_STATUS_INFO     I: pointer to status info strucutre                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE            - Printer settled into one of the requested state(s)    */
/*      EPS_JOB_CANCELED        - Cancelled operation                                   */
/*      EPS_ERR_OPR_FAIL        - Internal Error                                        */
/*      EPS_ERR_PRINTER_ERR_OCCUR  - Printer Error happened                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Monitor the status of the printer.                                              */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE   MonitorStatus (

        EPS_STATUS_INFO *pStInfo

){
/*** Declare Variable Local to Routine                                                  */
    EPS_INT32       retStatus;
    EPS_STATUS_INFO StatInfo;

	EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;
    memset(&StatInfo, -1, sizeof(EPS_STATUS_INFO));

	retStatus = jobFnc.MonitorStatus(&StatInfo);
    if ( retStatus != EPS_ERR_NONE) {
		if(EPS_ERR_COMM_ERROR == retStatus && 
			EPS_STATUS_NOT_INITIALIZED != printJob.jobStatus){
			printJob.bComm = FALSE;
		}

		EPS_DBGPRINT(("EPS SER: STAT MON -> Status Retr Failed. [%d]\r\n", retStatus));
        EPS_RETURN( retStatus );
    }
	if( pStInfo ){
		memcpy(pStInfo, &StatInfo, sizeof(EPS_STATUS_INFO));
	}

	if(EPS_ST_ERROR == StatInfo.nState && EPS_PRNERR_PAPERJAM == StatInfo.nError){
		printJob.transmittable = FALSE;
	}

    if(StatInfo.nCancel == EPS_CAREQ_CANCEL) {
        EPS_DBGPRINT(("EPS SER : CANCEL REQUEST by PRINTER\r\n"));
		printJob.resetReq = TRUE;
        EPS_RETURN( EPS_JOB_CANCELED );
    }

/*	EPS_DBGPRINT(("M State\t: %d\n Error\t: %d\n Prepare\t: %d\n", 
		StatInfo.nState, StatInfo.nError, StatInfo.nPrepare))
*/
	/* The error occurs in printer. */
	printJob.contData.lastError = StatInfo.nError;

    /*EPS_DBGPRINT(("EPS SER: STAT MON -> Printer State [0x%x]\r\n",StatInfo.nState));*/
    if( StatInfo.nState & ( EPS_ST_IDLE          |
							EPS_ST_WAITING       |
							EPS_ST_SELF_PRINTING |
							EPS_ST_CLEANING      ) ){
        /*EPS_DBGPRINT(("EPS SER: STAT MON -> SETTLING\r\n"));*/
        EPS_RETURN( EPS_ERR_NONE );
    }else{
        EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     PrintBand()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* data          EPS_UINT8*         I: Pointer to image [RGB] data                      */
/* widthPixels   EPS_UINT32         I: The width of the raster band (in pixels)         */
/* heightPixels  EPS_UINT32*        I/O: In : Height of image (image lines)  (in pixels)*/
/*                                     : Out: Sent Height                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_OUT_OF_BOUNDS               - Print band is in out of printable area        */
/*      << Error >>                                                                     */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*      EPS_ERR_INV_ARG_WIDTH_PIXELS    - Invalid argument "widthPixels"                */
/*      EPS_ERR_INV_ARG_HEIGHT_PIXELS   - Invalid argument "heightPixels"               */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Prints a band of raster data.                                                   */
/*      It must be noted that this function automatically positions the raster bands;   */
/*      all subsequent raster bands after the first one are positioned directly         */
/*      underneath the preceding band.                                                  */
/*      Horizontal orientation of the raster bands never changes.                       */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    PrintBand (

        const EPS_UINT8*  data,             /* Pointer to image [RGB] data              */
        EPS_UINT32  widthPixels,            /* Width of image [ in pixels ]             */
        EPS_UINT32  *heightPixels           /* Height of image (image lines)            */

) {

/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus;
EPS_UINT32      idx = 0;                    /* Generl loop/index variable               */
EPS_IMAGE       line;                       /* Single line of image data                */
const EPS_UINT8*      tmpImageData;         /* Temporary image pointer                  */
EPS_BOOL        skipLine;                   /* Flag to skip or print current image line */
EPS_UINT32		linePixels;
EPS_UINT32		sendHeight;

	EPS_LOG_FUNCIN;

	if( NULL == data ){
        EPS_RETURN( EPS_ERR_INV_ARG_DATA );
	}
	if(NULL == heightPixels){
        EPS_RETURN( EPS_ERR_INV_ARG_HEIGHT_PIXELS );
	}
	if(*heightPixels == 0){
        EPS_RETURN( EPS_ERR_INV_ARG_HEIGHT_PIXELS );
	}
	sendHeight = *heightPixels;
	*heightPixels = 0;

	if( widthPixels == 0 ){
        EPS_RETURN( EPS_ERR_INV_ARG_WIDTH_PIXELS );
	}
 
/*** Initialize Local Variables                                                         */
    retStatus = EPS_ERR_NONE;

	if( printJob.printableAreaHeight <= (EPS_UINT32)printJob.verticalOffset ){
        /*** Return to Caller                                                           */
		EPS_RETURN( EPS_OUT_OF_BOUNDS );
	}
    
/*** Clip number is image lines if too many                                             */
	if (sendHeight > (printJob.printableAreaHeight - printJob.verticalOffset)){
        sendHeight =  printJob.printableAreaHeight - printJob.verticalOffset;
	}

/*** Send leftovers of last time And Skip first line of input data                      */
	idx = 0;	/* Initialize line counter */

	/*** Send leftovers data                                                            */
	retStatus = SendLeftovers();
	if(EPS_ERR_INVALID_CALL == retStatus){	/* It was unnecessary */
		retStatus = EPS_ERR_NONE;
		if(printJob.contData.skipLine == TRUE){
			idx++;							/* skip first line of input data */
			printJob.contData.skipLine = FALSE;
		}
	} else if(EPS_ERR_NONE == retStatus){	/* Sent leftovers */
		idx++;								/* skip first line of input data */
	} else{
		EPS_RETURN( retStatus );
	}

/*** The following section of code forces the core module to skip raster lines that are */
/*** completely white in order to minimize data size. This function is compatible with  */
/*** 8-bit palettized and 24-bit RGB bitmaps.                                           */
/*** ---------------------------------------------------------------------------------- */

    /*** Initialize variable                                                            */
    line.bytesPerLine = printJob.bpp * widthPixels;
	linePixels = Min(line.bytesPerLine, printJob.bpp * printJob.printableAreaWidth);

    for (; idx < sendHeight; idx++) {
        /* Calculate position of image line                                             */
        //if(idx == 1){
        	//idx = sendHeight;
        	//break;
        //}
        line.data = data + idx * line.bytesPerLine;

        /*** Make new LineRect                                                          */
        line.rect.top      = (EPS_INT32)(printJob.verticalOffset + idx);
        line.rect.left     = 0;
        line.rect.bottom   = line.rect.top + 1;
        line.rect.right    = (EPS_INT32)(widthPixels);
		
		/*** Test if current image data is completely white                             */
		if(EPS_LANG_ESCPR == printJob.printer->language ){
			skipLine = TRUE;
		} else{
			skipLine = FALSE; /* esc/page need anything band */
		}

		/* skip blank band for ESC/P-R */
		/* check blank page for duplex */
		if( EPS_LANG_ESCPR == printJob.printer->language ||
			(EPS_DUPLEX_NONE != printJob.attr.duplex && printJob.needBand) )
		{
			for (tmpImageData = line.data; 
				 tmpImageData < (line.data + linePixels); 
				 tmpImageData++ ) {
				if(*tmpImageData != printJob.whiteColorValue) {
					skipLine = FALSE;
					printJob.needBand = FALSE;
					break;
				}
			}

			if (skipLine == TRUE)continue;
		}
		/*** Image Data not completely white - print image data                         */
		//idx = sendHeight;
		//break;
		retStatus = PrintLine(&line);

        if (retStatus != EPS_ERR_NONE) {
            break;
        }
    }

/*** band positioning adds                                                              */
	printJob.verticalOffset += idx;
    
/*** Return to Caller                                                                   */
	*heightPixels = idx;

    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     PrintChunk()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* data         EPS_UINT8*          I: Pointer to image [RGB] data                      */
/* dataSize		EPS_UINT32*         I/O: In : size of image                             */
/*                                     : Out: Sent size                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_OUT_OF_BOUNDS               - Print band is in out of printable area        */
/*      << Error >>                                                                     */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*      EPS_ERR_INV_ARG_HEIGHT_PIXELS   - Invalid argument "heightPixels"               */
/*      EPS_ERR_INV_ARG_DATASIZE		- data size limit								*/
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Prints Jpeg file data.                                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    PrintChunk (

        const EPS_UINT8*  data,             /* Pointer to image [JPEG] data             */
        EPS_UINT32*       dataSize          /* size of image (chunked data)             */

) {
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;
EPS_UINT32      sendSize = 0;
EPS_UINT32      retBufSize = 0;
EPS_INT32       cmdBuffSize = ESCPR_HEADER_LENGTH + ESCPR_SEND_JPGDATA_LENGTH;
EPS_UINT8*      bufPtr = NULL;              /* Temporary buffer pointer                 */
EPS_INT32       paramSize = 0;              /* Size of command parameters only          */

EPS_INT32       i = 0;
#define	DUMMY_DATA_COUNT	(1000)

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
	if(NULL == dataSize){
        EPS_RETURN( EPS_ERR_INV_ARG_HEIGHT_PIXELS );
	}
#if LCOMSW_DUMMY_SEND
	if( *dataSize == 0 && 
		(EPS_PROTOCOL_USB & printJob.printer->protocol) ){
		/* USB not peform */
        EPS_RETURN( EPS_ERR_NONE );
	}
#else
	if( NULL == data ){
        EPS_RETURN( EPS_ERR_INV_ARG_DATA );
	}
	if(*dataSize == 0){
        EPS_RETURN( EPS_ERR_INV_ARG_HEIGHT_PIXELS );
	}
#endif

	sendSize = *dataSize;
	*dataSize = 0;

#if !_VALIDATE_SUPPORTED_MEDIA_DATA_
	if( EPS_JPEG_CHUNK_SIZE_MAX < sendSize){
		*dataSize = 0;
        EPS_RETURN( EPS_ERR_INV_ARG_DATASIZE );
	}

	if( (EPS_UINT32)printJob.printer->JpgMax < printJob.jpegSize + sendSize){
		printJob.bJpgLimit = TRUE;
        EPS_RETURN( EPS_ERR_INV_ARG_DATASIZE );
	}
#endif

	/************************************************************************************/
	/*** Send last time leftovers data                                                  */

	/*** Header                                                                         */
	retStatus = SendLeftovers();
	if(EPS_ERR_INVALID_CALL == retStatus){	/* It was unnecessary */
		retStatus = EPS_ERR_NONE;
	} else if(EPS_ERR_NONE != retStatus){
		*dataSize = 0;
		EPS_RETURN( retStatus );
	}

	/*** Data                                                                           */
	if(0 < printJob.contData.jpgSize){
		retBufSize = 0;
		retStatus = SendCommand(data, 
								Min(printJob.contData.jpgSize, sendSize),
								&retBufSize, FALSE); /* not save Leftovers */
		printJob.contData.jpgSize -= retBufSize;
		*dataSize = retBufSize;
		printJob.jpegSize += retBufSize;	/* add total size */
		if( EPS_ERR_NONE != retStatus ){
			EPS_RETURN( retStatus );
		}

		sendSize -= retBufSize;
		if(0 >= sendSize ){
			EPS_RETURN( EPS_ERR_NONE );
		}

		data += retBufSize;
	}

	/************************************************************************************/
	/*** Send this time data                                                            */

	/*** Header                                                                         */
	printJob.contData.jpgSize = sendSize;

#if !LCOMSW_DUMMY_SEND
	EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, ESCPR_JPGHEAD_LENGTH )
#else
	if(0 < sendSize){
		EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, ESCPR_JPGHEAD_LENGTH )
	} else{
		EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, ESCPR_JPGHEAD_LENGTH * DUMMY_DATA_COUNT )
	}
#endif
	if(NULL == sendDataBuf){
		sendDataBufSize = 0;
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	bufPtr = sendDataBuf;
	memcpy(bufPtr, SendDataCmd, sizeof(SendDataCmd));
	bufPtr += sizeof(SendDataCmd);

	paramSize = ESCPR_SEND_JPGDATA_LENGTH + sendSize;
	memSetEndian(EPS_ENDIAN_LITTLE, EPS_4_BYTES, (EPS_UINT32)paramSize, bufPtr);
	bufPtr += 4;
	memcpy(bufPtr, SendJpegDataName, sizeof(SendJpegDataName));
	bufPtr += sizeof(SendJpegDataName);
 
	memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, sendSize, bufPtr);

	if(0 < sendSize){
		retBufSize = 0;
		retStatus = SendCommand(sendDataBuf, cmdBuffSize, &retBufSize, TRUE);/* save Leftovers */
		if( EPS_ERR_NONE != retStatus ){
			EPS_RETURN( retStatus );
		}

		/*** Data                                                                           */
		retBufSize = 0;
		retStatus = SendCommand(data, sendSize, &retBufSize, FALSE);  /* NOT save Leftovers */

		*dataSize += retBufSize;
		printJob.contData.jpgSize -= retBufSize;

		printJob.jpegSize += retBufSize;	/* add total size */
	} else {
		/*** dummy data */
		bufPtr = sendDataBuf + cmdBuffSize;
		for(i=1; i < DUMMY_DATA_COUNT; i++){		/* 12x1000 byte */
			memcpy(bufPtr, sendDataBuf, cmdBuffSize);
			bufPtr += cmdBuffSize;
		}
		
		retStatus = SendCommand(sendDataBuf, ESCPR_JPGHEAD_LENGTH * DUMMY_DATA_COUNT, &retBufSize, TRUE);/* save Leftovers */
	}

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendLeftovers()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nStartStep   EPS_INT32           I: Step of Start Page comannds                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_INVALID_CALL            - This call was unnecessary                     */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      send leftovers data.                                                            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE    SendLeftovers (

			void 

){
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls          */
EPS_UINT32		retBufSize = 0;

	EPS_LOG_FUNCIN;

/*** send leftovers                                                                      */
#ifdef GCOMSW_CMD_ESCPAGE
	if(EPS_LANG_ESCPR == printJob.printer->language ){
#endif
		/*** ESC/P-R ***/
		if( NULL != printJob.contData.sendData && 0 < printJob.contData.sendDataSize){
			retStatus = SendCommand(printJob.contData.sendData, 
									printJob.contData.sendDataSize, &retBufSize, TRUE);
		} else{
			retStatus = EPS_ERR_INVALID_CALL;
		}
#ifdef GCOMSW_CMD_ESCPAGE
	} else{
		/*** ESC/Page ***/
		retStatus = pageSendLeftovers();
	}
#endif

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendBlankBand()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nStartStep   EPS_INT32           I: Step of Start Page comannds                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      send leftovers data.                                                            */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE    SendBlankBand (

			void 

){
	EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls      */
	EPS_UINT8		pix[3*16]; /* 16=ESC/Page24 minimum unit */
	EPS_IMAGE       line;

	EPS_LOG_FUNCIN;

#ifdef GCOMSW_CMD_ESCPAGE_S
	if(EPS_LANG_ESCPAGE_S == printJob.printer->language){
		EPS_RETURN( EPS_ERR_NONE );
	}
#endif

	if(EPS_CP_FULLCOLOR == printJob.attr.colorPlane){
		memset(pix, 0xFF, sizeof(pix));	/* white */
		if(EPS_LANG_ESCPAGE == printJob.printer->language ||
		   EPS_LANG_ESCPAGE_COLOR == printJob.printer->language)
		{
			memset(pix, 0x00, 3);		/* black dot */
		}
	} else{
		memset(pix, printJob.whiteColorValue, sizeof(pix));
		if(EPS_LANG_ESCPAGE == printJob.printer->language ||
		   EPS_LANG_ESCPAGE_COLOR == printJob.printer->language)
		{
			if(printJob.whiteColorValue != 0){
				memset(pix, 0, sizeof(pix));
			} else{
				memset(pix, 1, sizeof(pix));
			}
		}
	}
	line.data          = pix;
	line.bytesPerLine  = printJob.bpp;
	line.rect.top      = 0;
	line.rect.left     = 0;
	line.rect.bottom   = 1;
	if(EPS_LANG_ESCPR == printJob.printer->language || 
		EPS_CP_256COLOR == printJob.attr.colorPlane ){
		line.rect.right= 1;
	} else{
		line.rect.right= 16;
	}
	
	retStatus = PrintLine(&line);

	EPS_RETURN( retStatus );
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SetupJobAttrib()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* jobAttr      EPS_JOB_ATTRIB*     I: Data structure containing job attribut settings  */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                            - Success                               */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_COLOR_PLANE                 - Invalid Color Plane                   */
/*      EPS_ERR_INV_PALETTE_SIZE                - Invalid Palette Size                  */
/*      EPS_ERR_INV_PALETTE_DATA                - Invalid Palette Data                  */
/*      EPS_ERR_INV_MEDIA_SIZE                  - Invalid Media Size                    */
/*      EPS_ERR_INV_MEDIA_TYPE                  - Invalid Media Type                    */
/*      EPS_ERR_INV_BORDER_MODE                 - Invalid Border Mode                   */
/*      EPS_ERR_INV_PRINT_QUALITY               - Invalid Print Quality                 */
/*      EPS_ERR_INV_PAPER_SOURCE                - Invalid Paper source                  */
/*      EPS_ERR_INV_COLOR_MODE                  - Invalid Color Mode                    */
/*      EPS_ERR_INV_INPUT_RESOLUTION            - Invalid Input Resolution              */
/*      EPS_ERR_INV_PRINT_DIRECTION             - Invalid Print Direction               */
/*      EPS_ERR_INV_BRIGHTNESS                  - Invalid Brightness                    */
/*      EPS_ERR_INV_CONTRAST                    - Invalid Contrast                      */
/*      EPS_ERR_INV_SATURATION                  - Invalid Saturation                    */
/*      EPS_ERR_INV_APF_FLT                     - Invalid APF Filter                    */
/*      EPS_ERR_INV_APF_ACT                     - Invalid APF Scene                     */
/*      EPS_ERR_INV_APF_SHP                     - Invalid APF Sharpness                 */
/*      EPS_ERR_INV_APF_RDE                     - Invalid APF Redeye                    */
/*      EPS_ERR_INV_TOP_MARGIN                  - Invalid Top Magirn                    */
/*      EPS_ERR_INV_LEFT_MARGIN                 - Invalid Left Margin                   */
/*      EPS_ERR_INV_BOTTOM_MARGIN               - Invalid Bottom Margin                 */
/*      EPS_ERR_INV_RIGHT_MARGIN                - Invalid Right Margin                  */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH     - Invalid Magin Setting (Width)         */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT    - Invalid Magin Setting (Height)        */
/*                                                                                      */
/* Description:                                                                         */
/*      Confirm ESC/P-R Job Attribute.                                                  */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    SetupJobAttrib (

        const EPS_JOB_ATTRIB*     jobAttr    /* Print Attributes for this Page         */

){
	EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls      */

EPS_LOG_FUNCIN;



/*** Validate input parameters                                                          */
#ifndef LCOMSW_JOBPARAM_CEHCK_OFF
/*======================================================================================*/
/*** Validate/Confirm Page Attribute Data                                               */
/*======================================================================================*/
	/*** Color Plane                                                                    */
	
	if (! (   (jobAttr->colorPlane         == EPS_CP_FULLCOLOR         ) ||
			  (jobAttr->colorPlane         == EPS_CP_256COLOR          ) ||
			  (jobAttr->colorPlane         == EPS_CP_JPEG              ) ||
			  (jobAttr->colorPlane         == EPS_CP_PRINTCMD          ) ) ){
		EPS_RETURN( EPS_ERR_INV_COLOR_PLANE )
	}

	if( EPS_CP_PRINTCMD == jobAttr->colorPlane ){		/* Print command */
		/*** Structure version                                                          */
		if(EPS_JOB_ATTRIB_VER_2 > jobAttr->version){
			EPS_RETURN( EPS_ERR_INVALID_VERSION )
		}

		/*** Print command type                                                         */
		if (! (   (jobAttr->cmdType       == EPS_MNT_CUSTOM           ) ||
				  (jobAttr->cmdType       == EPS_MNT_CLEANING         ) ||
				  (jobAttr->cmdType       == EPS_MNT_NOZZLE           )    ) ){
			EPS_RETURN( EPS_ERR_INV_CMDTYPE )
		}

	} else{												/* Image(RGB, Jpeg) */
		/*** Media Size                                                                     */
		if (! ( ( (jobAttr->mediaSizeIdx       >= EPS_MSID_A4              ) &&
				  (jobAttr->mediaSizeIdx       <= EPS_MSID_HIVISION        )    ) ||
				( (jobAttr->mediaSizeIdx       >= EPS_MSID_A3NOBI          ) &&
				  (jobAttr->mediaSizeIdx       <= EPS_MSID_12X12           )    ) ||
	 			( (jobAttr->mediaSizeIdx       == EPS_MSID_USER            )    )    ) ){
			EPS_RETURN( EPS_ERR_INV_MEDIA_SIZE )
		}
		/*** Media Type                                                                     */
		if (! ( ( (jobAttr->mediaTypeIdx       >= EPS_MTID_PLAIN           ) &&
	/*              (jobAttr->mediaTypeIdx       <= EPS_MTID_GLOSSYHAGAKI    )    ) ||*/
				  (jobAttr->mediaTypeIdx       <= EPS_MTID_BUSINESSCOAT    )    ) ||
				( (jobAttr->mediaTypeIdx       >= EPS_MTID_CDDVD           ) &&
				  (jobAttr->mediaTypeIdx       <= EPS_MTID_CDDVDGLOSSY     )    ) ||
				( (jobAttr->mediaTypeIdx       == EPS_MTID_CLEANING        )    )    ) ){
			EPS_RETURN( EPS_ERR_INV_MEDIA_TYPE )
		}
		/*** Print Quality                                                                  */
		if (! (   (jobAttr->printQuality       == EPS_MQID_DRAFT           ) ||
				  (jobAttr->printQuality       == EPS_MQID_NORMAL          ) ||
				  (jobAttr->printQuality       == EPS_MQID_HIGH            )    ) ){
			EPS_RETURN( EPS_ERR_INV_PRINT_QUALITY )
		}
		/*** duplex                                                                         */
		if( !( (jobAttr->duplex             == EPS_DUPLEX_NONE             ) ||
			   (jobAttr->duplex             == EPS_DUPLEX_LONG             ) ||
			   (jobAttr->duplex		        == EPS_DUPLEX_SHORT            ) ) ){
			EPS_RETURN( EPS_ERR_INV_DUPLEX )
		}
		/*** Brightness                                                                     */
		if (! (   (jobAttr->brightness         >= -50                      ) &&
				  (jobAttr->brightness         <=  50                      )    ) ){
			EPS_RETURN( EPS_ERR_INV_BRIGHTNESS )
		}
		/*** Contrast                                                                       */
		if (! (   (jobAttr->contrast           >= -50                      ) &&
				  (jobAttr->contrast           <=  50                      )    ) ){
			EPS_RETURN( EPS_ERR_INV_CONTRAST )
		}
		/*** Saturation                                                                     */
		if (! (   (jobAttr->saturation         >= -50                      ) &&
				  (jobAttr->saturation         <=  50                      )    ) ){
			EPS_RETURN( EPS_ERR_INV_SATURATION )
		}
		/*** Paper Source                                                                   */
		if (! (   (jobAttr->paperSource        == EPS_MPID_AUTO			   ) ||
				  (jobAttr->paperSource        == EPS_MPID_REAR            ) ||
				  (jobAttr->paperSource        == EPS_MPID_FRONT1          ) ||
				  (jobAttr->paperSource        == EPS_MPID_FRONT2          ) ||
				  (jobAttr->paperSource        == EPS_MPID_FRONT3          ) ||
				  (jobAttr->paperSource        == EPS_MPID_FRONT4          ) ||
				  (jobAttr->paperSource        == EPS_MPID_CDTRAY          ) ||
				  
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_MP_TRAY		) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_CASSETTE1		) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_CASSETTE2		) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_CASSETTE3		) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_CASSETTE4		) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_PARAM_MANUALFEED	) ||
				  (jobAttr->paperSource        == EPS_MPID_PAGE_S_AUTO_TRAY		)
				     ) ){
			EPS_RETURN( EPS_ERR_INV_PAPER_SOURCE )
		}

		if(jobAttr->printLayout == EPS_MLID_CDLABEL){
			/*** CD Inside Diameter                                                     */
			if( !( (jobAttr->cdDimIn >= EPS_CDDIM_IN_MIN   )
				&& (jobAttr->cdDimIn <= EPS_CDDIM_IN_MAX ) ) ){
				EPS_RETURN( EPS_ERR_INV_CD_INDIM )
			}
			/*** CD Outside Diameter                                                    */
			if( !( (jobAttr->cdDimOut >= EPS_CDDIM_OUT_MIN   )
				&& (jobAttr->cdDimOut <= EPS_CDDIM_OUT_MAX ) ) ){
				EPS_RETURN( EPS_ERR_INV_CD_OUTDIM )
			}
		}

		if( EPS_CP_JPEG == jobAttr->colorPlane ){
			if( jobAttr->duplex != EPS_DUPLEX_NONE ){
				EPS_RETURN( EPS_ERR_INV_DUPLEX )
			}
		}
	}
#endif



/*======================================================================================*/
/*** Copy Input Page Attribute Data to Internal Variable                                */
/*======================================================================================*/
	switch( jobAttr->colorPlane ){
	case EPS_CP_FULLCOLOR:			/* RGB */
	case EPS_CP_256COLOR:
	    memcpy((void*)(&printJob.attr), (void*)jobAttr, sizeof(EPS_JOB_ATTRIB));
		retStatus = SetupRGBAttrib();
		if(EPS_ERR_NONE == retStatus){
			_SP_ChangeSpec_DraftOnly(printJob.printer, &printJob.attr);   
			/* Ignore the return value of this func */
		}
		break;

	case EPS_CP_JPEG:				/* Jpeg */
	    memcpy((void*)(&printJob.attr), (void*)jobAttr, sizeof(EPS_JOB_ATTRIB));
		retStatus = SetupJPGAttrib();
		if(EPS_ERR_NONE == retStatus){
			_SP_ChangeSpec_DraftOnly(printJob.printer, &printJob.attr);   
			/* Ignore the return value of this func */
		}
		break;

	case EPS_CP_PRINTCMD:			/* Print command */
	default:
	    memset((void*)(&printJob.attr), 0, sizeof(EPS_JOB_ATTRIB));
		printJob.attr.colorPlane = jobAttr->colorPlane;
		printJob.attr.cmdType = jobAttr->cmdType;
		break;

	}

	EPS_RETURN( retStatus )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SetupRGBAttrib()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* jobAttr      EPS_JOB_ATTRIB*     I: Data structure containing page attribut settings */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                            - Success                               */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_COLOR_MODE                  - Invalid Color Mode                    */
/*      EPS_ERR_INV_INPUT_RESOLUTION            - Invalid Input Resolution              */
/*      EPS_ERR_INV_PRINT_DIRECTION             - Invalid Print Direction               */
/*      EPS_ERR_INV_PALETTE_SIZE                - Invalid Palette Size                  */
/*      EPS_ERR_INV_PALETTE_DATA                - Invalid Palette Data                  */
/*      EPS_ERR_INV_TOP_MARGIN                  - Invalid Top Magirn                    */
/*      EPS_ERR_INV_LEFT_MARGIN                 - Invalid Left Margin                   */
/*      EPS_ERR_INV_BOTTOM_MARGIN               - Invalid Bottom Margin                 */
/*      EPS_ERR_INV_RIGHT_MARGIN                - Invalid Right Margin                  */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH     - Invalid Magin Setting (Width)         */
/*      EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT    - Invalid Magin Setting (Height)        */
/*                                                                                      */
/* Description:                                                                         */
/*      Confirm RGB Attribute.                                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    SetupRGBAttrib (

		void

){
/*** Declare Variable Local to Routine                                                  */
EPS_INT32       factor;                     /* Scaling factor for dpi                   */
EPS_INT32       idx;                        /* Paper size index                         */
EPS_INT32       tempPrintableWidth;         /* Temporary Variable                       */
EPS_INT32       tempPrintableHeight;        /* Temporary Variable                       */
const EPS_MEDIA_INFO* pMI = NULL;
EPS_INT16		borderPixels;

	EPS_LOG_FUNCIN;

/*** Initialize Global/Local Variables                                                  */
    idx                 = -1;
    tempPrintableWidth  = 0;
    tempPrintableHeight = 0;

#ifndef LCOMSW_JOBPARAM_CEHCK_OFF
/*** Validate input parameters                                                          */
	/*** Border Mode                                                                    */
	if(! ((printJob.attr.printLayout == EPS_MLID_BORDERLESS      ) ||
		  (printJob.attr.printLayout == EPS_MLID_BORDERS         ) ||
		  (printJob.attr.printLayout == EPS_MLID_CDLABEL         ) ||
		  (printJob.attr.printLayout == EPS_MLID_DIVIDE16        ) ||
		  (printJob.attr.printLayout == EPS_MLID_CUSTOM          ) ) ){
		EPS_RETURN( EPS_ERR_INV_BORDER_MODE )
	}
	/*** Color Mode                                                                 */
    if( !( (printJob.attr.colorMode          == EPS_CM_COLOR             ) ||
           (printJob.attr.colorMode          == EPS_CM_MONOCHROME        ) /*||
           (printJob.attr.colorMode          == EPS_CM_SEPIA             )*/ ) ){
		EPS_RETURN( EPS_ERR_INV_COLOR_MODE )
	}

	/*** Input Image Resolution                                                     */
    /*** Select table and factor                                                        */
	if(printJob.attr.inputResolution == EPS_IR_360X360){
		pMI = epsMediaSize;
	    factor = 1;
		borderPixels = EPS_BORDERS_MARGIN_360;
	} else if(printJob.attr.inputResolution == EPS_IR_720X720){
		pMI = epsMediaSize;
		factor = 2;
		borderPixels = EPS_BORDERS_MARGIN_360;
	} else if(printJob.attr.inputResolution == EPS_IR_300X300){
		pMI = epsMediaSize300;
		//pMI = epsMediaSize;
		factor = 1;
		borderPixels = EPS_BORDERS_MARGIN_300;
		//borderPixels = EPS_BORDERS_MARGIN_360;
	} else if(printJob.attr.inputResolution == EPS_IR_600X600){
		pMI = epsMediaSize300;
	    factor = 2;
		borderPixels = EPS_BORDERS_MARGIN_300;
	} else{
        EPS_RETURN( EPS_ERR_INV_INPUT_RESOLUTION )
	}

	/*** Printing Direction                                                         */
    if (! (   (printJob.attr.printDirection     == EPS_PD_BIDIREC           ) ||
	  	      (printJob.attr.printDirection     == EPS_PD_UNIDIREC          )    ) ){
		EPS_RETURN( EPS_ERR_INV_PRINT_DIRECTION )
	}

	/*** Pallette Data                                                              */
	if (printJob.attr.colorPlane == EPS_CP_256COLOR) {
		if (! ((printJob.attr.paletteSize       >= 3                        ) &&
			   (printJob.attr.paletteSize       <= 768/*765*/               )    ) ){
			EPS_RETURN( EPS_ERR_INV_PALETTE_SIZE )
		}

		if (    printJob.attr.paletteData       == NULL                     ){
			EPS_RETURN( EPS_ERR_INV_PALETTE_DATA )
		}
	}

    /*** Margin                                                                     */
	if (printJob.attr.printLayout == EPS_MLID_CUSTOM) {
		if (printJob.attr.topMargin    < borderPixels*factor) EPS_RETURN( EPS_ERR_INV_TOP_MARGIN );
		if (printJob.attr.leftMargin   < borderPixels*factor) EPS_RETURN( EPS_ERR_INV_LEFT_MARGIN );
		if (printJob.attr.bottomMargin < borderPixels*factor) EPS_RETURN( EPS_ERR_INV_BOTTOM_MARGIN );
		if (printJob.attr.rightMargin  < borderPixels*factor) EPS_RETURN( EPS_ERR_INV_RIGHT_MARGIN );
	}
#endif

/*======================================================================================*/
/*** Set the following parameter                                                        */
/***    - printJob.topMargin                                                            */
/***    - printJob.leftMargin                                                           */
/***    - printJob.printableAreaWidth                                                   */
/***    - printJob.printableAreaHeight                                                  */
/***    - printJob.borderlessModeInternal                                               */
/***    - printJob.verticalOffset                                                       */
/*======================================================================================*/
    /*** Find the Media by ID                                                           */
    for (idx = 0; pMI[idx].id != -1; idx++) {
        if (pMI[idx].id == printJob.attr.mediaSizeIdx)
            break;
    }
    if (pMI[idx].id == -1) {
        EPS_RETURN( EPS_ERR_INV_MEDIA_SIZE );
    }

    /*** Media Attributes                                                               */
    printJob.paperWidth  = pMI[idx].paper_x * factor;
    printJob.paperHeight = pMI[idx].paper_y * factor;

    /*** Initialize Printable based on border mode                                      */
    switch( printJob.attr.printLayout ){
	case EPS_MLID_BORDERLESS:
        printJob.topMargin      = pMI[idx].top_margin_borderless   * factor;
        printJob.leftMargin     = pMI[idx].left_margin_borderless  * factor;
        tempPrintableWidth      = pMI[idx].print_area_x_borderless * factor;
        tempPrintableHeight     = pMI[idx].print_area_y_borderless * factor;
        printJob.borderlessMode = EPS_BORDERLESS_NORMAL;
		break;

	case EPS_MLID_BORDERS:
	case EPS_MLID_DIVIDE16:  
		  printJob.topMargin      = borderPixels * factor;
        printJob.leftMargin     = borderPixels * factor;

        tempPrintableWidth      = pMI[idx].print_area_x_border * factor;
        tempPrintableHeight     = pMI[idx].print_area_y_border * factor;
        printJob.borderlessMode = EPS_BORDER_3MM_MARGINE;
		break;

	case EPS_MLID_CDLABEL:
		tempPrintableWidth  =
		tempPrintableHeight = elGetDots(printJob.attr.inputResolution, printJob.attr.cdDimOut);
		printJob.topMargin  = CDDVD_OFFSET_Y(printJob.attr.inputResolution, printJob.attr.cdDimOut);
		printJob.leftMargin = CDDVD_OFFSET_X(printJob.attr.inputResolution, printJob.attr.cdDimOut);
        printJob.borderlessMode = EPS_BORDER_3MM_MARGINE; /* It's no meaning & no effect */

		EPS_DBGPRINT(("dim: %d / top: %d / left: %d\n", printJob.attr.cdDimOut,
						printJob.topMargin, printJob.leftMargin));
		break;

	default: /* printJob.attr.printLayout  == EPS_MLID_CUSTOM */
        printJob.topMargin      = printJob.attr.topMargin;
        printJob.leftMargin     = printJob.attr.leftMargin;
        tempPrintableWidth      = pMI[idx].paper_x * factor		-
                                  printJob.attr.leftMargin      -
                                  printJob.attr.rightMargin;
        tempPrintableHeight     = pMI[idx].paper_y * factor		-
                                  printJob.attr.topMargin       -
                                  printJob.attr.bottomMargin;
		printJob.borderlessMode = EPS_BORDER_CUSTOM;
    }

    /*** Validate/Confirm Magin Setting                                                 */
    if (printJob.attr.printLayout == EPS_MLID_CUSTOM) {
        if (tempPrintableWidth  <= 0) EPS_RETURN( EPS_ERR_MARGIN_OVER_PRINTABLE_WIDTH );
        if (tempPrintableHeight <= 0) EPS_RETURN( EPS_ERR_MARGIN_OVER_PRINTABLE_HEIGHT );
    }

    /*** Set Printable Area */
   EPS_INT32 lOffset = printJob.attr.inputResolution/30;
    //printJob.printableAreaHeight  = (EPS_UINT32)lWidth + lOffset*2;
    //printJob.printableAreaWidth = (EPS_UINT32)lHeight + lOffset*2;
    
    printJob.printableAreaWidth  = (EPS_UINT32)tempPrintableWidth;
    printJob.printableAreaHeight = (EPS_UINT32)tempPrintableHeight;
    //printJob.printableAreaWidth  = (EPS_UINT32)tempPrintableHeight;
    //printJob.printableAreaHeight = (EPS_UINT32)tempPrintableWidth;
    /*
    areaWidth = tempPrintableHeight;
	 areaHeight = tempPrintableWidth;
    EPS_INT32 lOffset = printJob.attr.inputResolution/30;
    printJob.printableAreaHeight  = (EPS_UINT32)lHeight + lOffset*2;
    printJob.printableAreaWidth = (EPS_UINT32)lWidth + lOffset*2;
    */
    //printJob.printableAreaHeight  = (EPS_UINT32)lHeight + lOffset*2;
    //printJob.printableAreaWidth = (EPS_UINT32)lWidth + lOffset*2;


    /*** Scan through palette for the index value of white and set the global white
                                    value to this index for skipping white raster lines */
    if( printJob.attr.colorPlane == EPS_CP_256COLOR ){ 
	    printJob.bpp = 1;
	} else /* if(EPS_CP_FULLCOLOR) */{
        printJob.bpp = 3;
		printJob.attr.paletteSize = 0;
		printJob.attr.paletteData = NULL;
	}
	printJob.whiteColorValue = memSearchWhiteColorVal(printJob.attr.colorPlane, 
													printJob.attr.paletteData,
													printJob.attr.paletteSize);

    /*** Set "Base Point" Data                                                          */
    /*** BORDER                                                                         */
    printJob.border.top        =
    printJob.border.left       =
    printJob.border.bottom     =
	printJob.border.right      = borderPixels * factor;

    AdjustBasePoint();

	EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SetupJPGAttrib()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                            - Success                               */
/*      << Error >>                                                                     */
/*      EPS_ERR_INV_APF_FLT                     - Invalid APF Filter                    */
/*      EPS_ERR_INV_APF_ACT                     - Invalid APF Scene                     */
/*      EPS_ERR_INV_APF_SHP                     - Invalid APF Sharpness                 */
/*      EPS_ERR_INV_APF_RDE                     - Invalid APF Redeye                    */
/*                                                                                      */
/* Description:                                                                         */
/*      Confirm APF Attribute.                                                          */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    SetupJPGAttrib (

        void

){
	EPS_LOG_FUNCIN;

#ifndef LCOMSW_JOBPARAM_CEHCK_OFF
/*** Validate input parameters                                                          */
	/*** Border Mode                                                                    */
	if(! ((printJob.attr.printLayout == EPS_MLID_BORDERLESS      ) ||
		  (printJob.attr.printLayout == EPS_MLID_BORDERS         ) ||
		  (printJob.attr.printLayout == EPS_MLID_CDLABEL         ) ||
		  (printJob.attr.printLayout == EPS_MLID_DIVIDE16        ) /*||
		  (printJob.attr.printLayout == EPS_MLID_CUSTOM          ) */) ){
		EPS_RETURN( EPS_ERR_INV_BORDER_MODE )
	}
	/*** Color Mode (APF Filter)                                                        */
    if( !( (printJob.attr.colorMode      == EPS_CM_COLOR             ) ||
           (printJob.attr.colorMode      == EPS_CM_MONOCHROME        ) ||
           (printJob.attr.colorMode      == EPS_CM_SEPIA             ) ) ){
		EPS_RETURN( EPS_ERR_INV_COLOR_MODE )
	}

	/*** APF Scene                                                                      */
	if( !( (printJob.attr.apfAutoCorrect == EPS_APF_ACT_NOTHING  ) ||
		   (printJob.attr.apfAutoCorrect == EPS_APF_ACT_STANDARD ) ||
		   (printJob.attr.apfAutoCorrect == EPS_APF_ACT_PIM      ) ||
		   (printJob.attr.apfAutoCorrect == EPS_APF_ACT_PORTRATE ) ||
		   (printJob.attr.apfAutoCorrect == EPS_APF_ACT_VIEW     ) ||
		   (printJob.attr.apfAutoCorrect == EPS_APF_ACT_NIGHTVIEW) ) ){
		EPS_RETURN( EPS_ERR_INV_APF_ACT );
	}
	/*** APF Sharpness                                                                  */
	if (! ((printJob.attr.sharpness           >= -50     ) &&
  		   (printJob.attr.sharpness           <=  50     ) ) ){
		EPS_RETURN( EPS_ERR_INV_APF_SHP );
	}
	/*** APF Redeye                                                                     */
	if( !( (printJob.attr.redeye         == EPS_APF_RDE_NOTHING  ) ||
		   (printJob.attr.redeye         == EPS_APF_RDE_CORRECT  ) ) ){
		EPS_RETURN( EPS_ERR_INV_APF_RDE );
	}
#endif

    printJob.bpp = 0;
	printJob.attr.paletteSize = 0;
	printJob.attr.paletteData = NULL;

	EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   AddCmdBuff()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:			Type:               Description:                                    */
/* pBuff            EPS_UINT8**         IO: pointer to command buffer                   */
/* pPos             EPS_UINT8**         IO: pointer to append position                  */
/* bufSize          EPS_UINT32*         IO: pointer to size of command buffer(pBuff)    */
/* cmd              EPS_UINT8*          I : pointer to command                          */
/* cmdSize          EPS_UINT32          I : size of command(cmd)                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Append command to buffer. If the buffer is short, expand it.                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE AddCmdBuff(
						  
		EPS_UINT8 **pBuff, 
		EPS_UINT8 **pPos, 
		EPS_UINT32 *bufSize, 
		const EPS_UINT8 *cmd,
		EPS_UINT32 cmdSize
		
){
	EPS_UINT32	cmdPosDist = (EPS_UINT32)(*pPos - *pBuff);	/* command offset distance  */

	EPS_LOG_FUNCIN;

	EPS_MEM_GROW(EPS_UINT8*, *pBuff, bufSize, cmdPosDist + cmdSize )

	if(*pBuff != NULL){
		*pPos = *pBuff + cmdPosDist;
		memcpy(*pPos, cmd, cmdSize);
		*pPos += cmdSize;
		EPS_RETURN( EPS_ERR_NONE );
	} else{
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendStartJob()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:			Type:               Description:                                    */
/* bAddStartPage    EPS_BOOL            I: Append Start Page comannd                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send Start Job (&Page) commands.                                                */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    SendStartJob (

        EPS_BOOL bAddStartPage 

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls          */
EPS_UINT8*      pCmdPos = NULL;             /* Temporary buffer pointer                 */
EPS_UINT32      retBufSize = 0;             /* Size of buffer written                   */

	EPS_LOG_FUNCIN;

	pCmdPos = sendDataBuf;

/*======================================================================================*/
/*** Initialize Printer and Set Printer in ESC/PR mode                                  */
/*======================================================================================*/
#define SendStartJob_ADDCMD_LEN(CMD, EXPLEN) {												\
		retStatus = AddCmdBuff(&sendDataBuf, &pCmdPos, &sendDataBufSize, CMD, sizeof(CMD)+EXPLEN);	\
		if(EPS_ERR_NONE != retStatus){														\
			goto SendStartJob_END;															\
		}																					\
	}
#define SendStartJob_ADDCMD(CMD) SendStartJob_ADDCMD_LEN(CMD, 0)

	/*** Exit Packet Mode                                                               */
	SendStartJob_ADDCMD(ExitPacketMode)

	/*** Initialize Printer                                                             */
	SendStartJob_ADDCMD(InitPrinter)

	/*** Enter Remote Mode                                                              */
	SendStartJob_ADDCMD(EnterRemoteMode)

	/*** Remote Command - TI                                                            */
	if(epsCmnFnc.getLocalTime){
		SendStartJob_ADDCMD(RemoteTI)
		MakeRemoteTICmd(pCmdPos - sizeof(RemoteTI));
	}

	/*** Remote Command - JS                                                            */
	SendStartJob_ADDCMD(RemoteJS)

	/*** Remote Command - JH                                                            */
	SendStartJob_ADDCMD(RemoteJH)

	/*** Remote Command - HD                                                            */
	SendStartJob_ADDCMD(RemoteHD)
	*(pCmdPos - sizeof(RemoteHD) + 6) = printJob.platform;

	/*** Remote Command - PP                                                            */
	SendStartJob_ADDCMD(RemotePP)
	pCmdPos -= sizeof(RemotePP);
	
	switch(printJob.attr.paperSource){
	case EPS_MPID_REAR:
		pCmdPos[5] = 0x01; pCmdPos[6] = 0x00; 
		break;
	case EPS_MPID_FRONT1:
		pCmdPos[5] = 0x01; pCmdPos[6] = 0x01; 
		break;
	case EPS_MPID_FRONT2:
		pCmdPos[5] = 0x01; pCmdPos[6] = 0x02;
		break;
	case EPS_MPID_CDTRAY:
		pCmdPos[5] = 0x02; pCmdPos[6] = 0x01; 
		break;

	case EPS_MPID_AUTO:
	default:
		if( EPS_CP_JPEG == printJob.attr.colorPlane &&
			EPS_IS_CDDVD(printJob.attr.mediaTypeIdx) ){
			/* Jpeg CD print need PP  */
			pCmdPos[5] = 0x02; pCmdPos[6] = 0x01; 
		} else{
			EPS_DBGPRINT(("Paper Sourcr AutoSelect\n"));
			pCmdPos[5] = 0x01; pCmdPos[6] = 0xFF; /* auto select */
		}
		break;
	}
	
	pCmdPos += sizeof(RemotePP);

	/*** Remote Command - DP(duplex)                                                    */
	if(EPS_DUPLEX_NONE != printJob.attr.duplex){
		SendStartJob_ADDCMD(RemoteDP)
	}

	/*** Exit Remote Mode                                                               */
	SendStartJob_ADDCMD(ExitRemoteMode)

	/*** Enter ESC/P-R mode                                                             */
	if( EPS_CP_JPEG != printJob.attr.colorPlane ){	/* RGB  */
		SendStartJob_ADDCMD(ESCPRMode)
	} else{                                         /* JPEG */
		SendStartJob_ADDCMD(ESCPRModeJpg)
	}

	/*** ESC/PR "Print Quality" Command (Internal called Page Attributes)               */
	SendStartJob_ADDCMD_LEN(PrintQualityCmd, printJob.attr.paletteSize)
	MakeQualityCmd(pCmdPos - (sizeof(PrintQualityCmd) + printJob.attr.paletteSize) );

	/*** ESC/PR "apf setting" Command                                                   */
	if(EPS_CP_JPEG == printJob.attr.colorPlane){
		SendStartJob_ADDCMD(APFSettingCmd)
		MakeAPFCmd(pCmdPos-sizeof(APFSettingCmd));
	}

	/*** ESC/PR "Job" Command to Printer                                                */
	if( EPS_CP_JPEG != printJob.attr.colorPlane ){	/* RGB  */
		SendStartJob_ADDCMD(JobCmd)
		MakeJobCmd(pCmdPos-sizeof(JobCmd));
	} else{                                         /* JPEG */
		SendStartJob_ADDCMD(JobCmdJpg)
		MakeJobCmd(pCmdPos-sizeof(JobCmdJpg));
	}

	if(bAddStartPage){
		SendStartJob_ADDCMD(StartPage)
	}

	retStatus = SendCommand(sendDataBuf, (EPS_UINT32)(pCmdPos - sendDataBuf), &retBufSize, TRUE);

SendStartJob_END:
	if(EPS_ERR_NONE == retStatus){
		printJob.sendJS = TRUE;
	}

    /*** Return to Caller                                                               */
    EPS_RETURN( retStatus );
}


/*======================================================================================*/
/*** Set up Remote "TI" Command                                                         */
/*======================================================================================*/
static void    MakeRemoteTICmd (

        EPS_UINT8*		pBuf

){
/*** Declare Variable Local to Routine                                                  */
EPS_LOCAL_TIME  locTime;
EPS_UINT8       array2[2] = {0, 0};         /* Temporary Buffer for 2 byte Big Endian   */

	EPS_LOG_FUNCIN;

	/* Get platform local time */
	epsCmnFnc.getLocalTime(&locTime);

	/*** Skip Header                                                                    */
	pBuf += REMOTE_HEADER_LENGTH;
    
	/*** Set Attributes/Values                                                          */
	memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, locTime.year, array2);
	memcpy(pBuf, array2, sizeof(array2));
	pBuf += sizeof(array2);
	*pBuf++ = locTime.mon;
	*pBuf++ = locTime.day;
	*pBuf++ = locTime.hour;
	*pBuf++ = locTime.min;
	*pBuf   = locTime.sec;

	EPS_RETURN_VOID;
}


/*======================================================================================*/
/*** Set up ESC/PR "Print Quality" Command                                              */
/*======================================================================================*/
static void    MakeQualityCmd (

        EPS_UINT8*		pBuf

){
/*** Declare Variable Local to Routine                                                  */
EPS_UINT8*      pCmdPosTmp = NULL;
EPS_UINT8       array2[2] = {0, 0};         /* Temporary Buffer for 2 byte Big Endian   */
EPS_UINT8       array4[4] = {0, 0, 0, 0};   /* Temporary Buffer for 4 byte Big Endian   */

	EPS_LOG_FUNCIN;

	/*** Parameter Length                                                               */
	if(printJob.attr.paletteSize > 0){
		pCmdPosTmp = pBuf + ESCPR_CLASS_LENGTH;
		memSetEndian(EPS_ENDIAN_LITTLE, EPS_4_BYTES, 
					ESCPR_PRINT_QUALITY_LENGTH + printJob.attr.paletteSize, array4);
		memcpy(pCmdPosTmp, array4, sizeof(array4));
	}

	pBuf += ESCPR_HEADER_LENGTH;

	/*** Set Attributes/Values                                                          */
	*pBuf++ = (EPS_UINT8)printJob.attr.mediaTypeIdx;
	switch( printJob.attr.printQuality ){
		case EPS_MQID_DRAFT:
			*pBuf++ = 0;
			break;
		case EPS_MQID_HIGH:
			*pBuf++ = 2;
			break;
		case EPS_MQID_NORMAL:
		default:
			*pBuf++ = 1;
			break;
	}
	*pBuf++ = printJob.attr.colorMode;
	*pBuf++ = (EPS_UINT8)printJob.attr.brightness;
	*pBuf++ = (EPS_UINT8)printJob.attr.contrast;
	*pBuf++ = (EPS_UINT8)printJob.attr.saturation;
	*pBuf++ = printJob.attr.colorPlane;
    
	memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, printJob.attr.paletteSize, array2);
	memcpy(pBuf, array2, sizeof(array2));

	if (printJob.attr.paletteSize > 0){
		pBuf += sizeof(array2);
		memcpy(pBuf, printJob.attr.paletteData, printJob.attr.paletteSize);
	}

	EPS_RETURN_VOID;
}


/*======================================================================================*/
/*** Set up ESC/PR "APF setting" Command                                                */
/*======================================================================================*/
static void    MakeAPFCmd (

        EPS_UINT8*		pBuf

){
	EPS_LOG_FUNCIN;

	/*** Skip Header                                                                    */
    pBuf += ESCPR_HEADER_LENGTH;

	/*** Set Attributes/Values                                                          */
	*pBuf++ = printJob.attr.colorMode;
	*pBuf++ = printJob.attr.apfAutoCorrect;
	*pBuf++ = printJob.attr.sharpness;
	*pBuf++ = printJob.attr.redeye;

	EPS_RETURN_VOID;
}


/*======================================================================================*/
/*** Set up ESC/PR "Job" Command                                                        */
/*======================================================================================*/
static void    MakeJobCmd (

        EPS_UINT8*		pBuf

){
/*** Declare Variable Local to Routine                                                  */
EPS_UINT8       array2[2] = {0, 0};         /* Temporary Buffer for 2 byte Big Endian   */
EPS_UINT8       array4[4] = {0, 0, 0, 0};   /* Temporary Buffer for 4 byte Big Endian   */

	EPS_LOG_FUNCIN;

	/*** Skip Header                                                                    */
    pBuf += ESCPR_HEADER_LENGTH;

	if( EPS_CP_JPEG != printJob.attr.colorPlane ){	/* RGB  */
		/*EPS_DBGPRINT(("(%d, %d) / (%d, %d) / (%d, %d)\n",
			 printJob.paperWidth, printJob.paperHeight,
			 printJob.topMargin, printJob.leftMargin,
			 printJob.printableAreaWidth, printJob.printableAreaHeight))*/
		/*** Set Attributes/Values                                                      */
		memSetEndian(EPS_ENDIAN_BIG, EPS_4_BYTES, (EPS_UINT32)printJob.paperWidth, array4);
		memcpy(pBuf, array4, sizeof(array4));
		pBuf += sizeof(array4);
		memSetEndian(EPS_ENDIAN_BIG, EPS_4_BYTES, (EPS_UINT32)printJob.paperHeight, array4);
		memcpy(pBuf, array4, sizeof(array4));
		pBuf += sizeof(array4);
		memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, (EPS_UINT32)printJob.topMargin, array2);
		memcpy(pBuf, array2, sizeof(array2));
		pBuf += sizeof(array2);
		memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, (EPS_UINT32)printJob.leftMargin, array2);
		memcpy(pBuf, array2, sizeof(array2));
		pBuf += sizeof(array2);
		memSetEndian(EPS_ENDIAN_BIG, EPS_4_BYTES, printJob.printableAreaWidth, array4);
		memcpy(pBuf, array4, sizeof(array4));
		pBuf += sizeof(array4);
		memSetEndian(EPS_ENDIAN_BIG, EPS_4_BYTES, printJob.printableAreaHeight, array4);
		memcpy(pBuf, array4, sizeof(array4));
		pBuf += sizeof(array4);
		switch( printJob.attr.inputResolution ){
		case EPS_IR_720X720:
			*pBuf++ = 0x01;
			break;
		case EPS_IR_300X300:
			*pBuf++ = 0x02;
			break;
		case EPS_IR_600X600:
			*pBuf++ = 0x03;
			break;
		case EPS_IR_360X360:
		default:
			*pBuf++ = 0x00;
			break;
		}
		*pBuf   = printJob.attr.printDirection;

	} else{												/* JPEG  */
		/*** Set Attributes/Values                                                      */
		*pBuf++ = (EPS_UINT8)printJob.attr.mediaSizeIdx;
		if( EPS_IS_CDDVD( printJob.attr.mediaTypeIdx ) ){
			*pBuf++ = 0x0A;
		} else{
			switch( printJob.attr.printLayout ){
			case EPS_MLID_BORDERLESS:
				*pBuf++ = 0x01;
				break;
			case EPS_MLID_CDLABEL:
				*pBuf++ = 0x0A;
				break;
			case EPS_MLID_DIVIDE16:
				*pBuf++ = 0x90;
				break;
			default: /* FIXED, CUSTOM */
				*pBuf++ = 0x00;
			}
		}
		*pBuf++ = printJob.attr.cdDimIn;
		*pBuf++ = printJob.attr.cdDimOut;
		*pBuf   = printJob.attr.printDirection;
	}

	EPS_RETURN_VOID;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendEndJob()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* bAddEndPage  EPS_BOOL            I: Append End Page comannd                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      << Normal >>                                                                    */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      << Error >>                                                                     */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send End Page commands.                                                         */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    SendEndJob (

        EPS_BOOL bAddEndPage

){
/*** Declare Variable Local to Routine                                                  */
EPS_ERR_CODE    retStatus = EPS_ERR_NONE;   /* Return status of internal calls          */
EPS_UINT8*      pCmdPos = NULL;
EPS_UINT32      retBufSize;                         /* Size of buffer written           */

	EPS_LOG_FUNCIN

	EPS_MEM_GROW(EPS_UINT8*, sendDataBuf, &sendDataBufSize, 
				(sizeof(EndPage)+ sizeof(EndJob) + sizeof(InitPrinter)
				+ sizeof(EnterRemoteMode) + sizeof(RemoteLD) + sizeof(RemoteJE)+ sizeof(ExitRemoteMode)))
	if(NULL == sendDataBuf){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION )
	}

	pCmdPos = sendDataBuf;

	if(bAddEndPage){
		memcpy(pCmdPos, EndPage, sizeof(EndPage));
		pCmdPos[10] = EPS_END_PAGE;
		pCmdPos += sizeof(EndPage);
	}

	/*** End Job                                                                        */
	memcpy(pCmdPos, EndJob, sizeof(EndJob));
	pCmdPos += sizeof(EndJob);

	/*** Initialize                                                                     */
	memcpy(pCmdPos, InitPrinter, sizeof(InitPrinter));
	pCmdPos += sizeof(InitPrinter);

	memcpy(pCmdPos, EnterRemoteMode, sizeof(EnterRemoteMode));
	pCmdPos += sizeof(EnterRemoteMode);

	if(EPS_DUPLEX_NONE != printJob.attr.duplex){
		memcpy(pCmdPos, RemoteLD, sizeof(RemoteLD));
		pCmdPos += sizeof(RemoteLD);
	}

	/*** Remote Command - JE                                                            */
	memcpy(pCmdPos, RemoteJE, sizeof(RemoteJE));
	pCmdPos += sizeof(RemoteJE);

  	/*** Exit Remote Mode                                                               */
	memcpy(pCmdPos, ExitRemoteMode, sizeof(ExitRemoteMode));
	pCmdPos += sizeof(ExitRemoteMode);

	retStatus = SendCommand(sendDataBuf, (EPS_UINT32)(pCmdPos - sendDataBuf), &retBufSize, TRUE);

	if(EPS_JOB_CANCELED == retStatus){
		retStatus = EPS_ERR_NONE;
	}

	/*printJob.sendJS = FALSE;*/

    /*** Return to Caller                                                               */
    EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendCommand()                                                       */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Buffer       EPS_INT8*           I: Print Data Buffer                                */
/* BuffLen      EPS_UINT32          I: Print Data Buffer Size                           */
/* pSize        EPS_UINT32          O: Actual Length Transferred                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Sent the data successfully                    */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_ERR_OCCUR       - Printer Error happened                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Send print data to the active printer                                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE  SendCommand (

        const EPS_UINT8*    Buffer,
        EPS_UINT32          BuffLen,
        EPS_UINT32*         pSize,
        EPS_BOOL            bSave

){

#ifdef EPS_FILTER
	//long int i;
	EPS_UINT32 i;
	FILE* outfp = stdout;
	for (i = 0; i < BuffLen; i++){
		putc(*(Buffer + i), outfp);
	}
	*pSize = BuffLen;
	return EPS_ERR_NONE;
#endif	
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE  Ret = EPS_ERR_NONE;
    EPS_INT32  retStatus;
    EPS_UINT32 sendSize;
    EPS_UINT32 sentSize;
	EPS_INT32  nRetry = 0;
	EPS_UINT32 tmStart, tmNow, tmSpan;
    EPS_INT32  nDlySpan = 10;				/* first 10 ms */
    EPS_INT32  nDlyTotal = EPS_TIMEOUT_SEC;	/* total 1000 ms */

	EPS_LOG_FUNCIN;

/*** Validate input parameters                                                          */
    if ((Buffer  == NULL) || (BuffLen == 0))
        EPS_RETURN( EPS_ERR_OPR_FAIL );

/*** Initialize Global/Local Variables                                                  */
    retStatus = EPS_ERR_NONE;
    sentSize  = 0;
    sendSize  = BuffLen;
    *pSize    = 0;

	tmStart = tmNow = tmSpan = 0;

	if( FALSE == printJob.bComm){
		EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
	}

	if( EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){
/***------------------------------------------------------------------------------------*/
/*** Bi-Directional Mode                                                                */
/***------------------------------------------------------------------------------------*/
	/*** If printer reset command was already sent, don't send command any more.        */
	/*** When paper jam error happened, don't send command any more.                    */
	/*** "transmittable == FLSE" means "paper jam error happened".                      */
		if( printJob.resetSent == EPS_RESET_SENT || TRUE == printJob.resetReq ){
			EPS_RETURN( EPS_JOB_CANCELED );
		} else if(FALSE == printJob.transmittable){
			EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
		}
        
	/*** Send command                                                                   */
		if(epsCmnFnc.getTime){
			tmStart = epsCmnFnc.getTime();
			tmNow = tmSpan = 0;
		}

		while(nDlyTotal > 0){
			if(gStatusCount == EPS_ROOP_NUM){
				gStatusCount = 0;
				if ((Ret = MonitorStatus(NULL) ) != EPS_ERR_NONE){
					EPS_DBGPRINT(("MonitorStatus=%d\n", Ret));

					if(Ret == EPS_JOB_CANCELED){
						EPS_RETURN( Ret );
					} else if(Ret == EPS_ERR_COMM_ERROR){
						EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
					} else {
						/*anoter error*/
						if ( EPS_PROTOCOL_USB & printJob.printer->protocol ){
							break;
						} else{
							/* Net continue to buffer full. */
						}
					} 
				}
			}
			gStatusCount++;

			Ret = jobFnc.WriteData(Buffer, sendSize, &sentSize);
			*pSize += sentSize;
#ifdef LCOMSW_CMDDMP
	EPS_DF_WRITE(Buffer, sentSize)
#endif

			if (Ret != EPS_ERR_NONE){
				if (Ret == EPS_JOB_CANCELED) {
					/* UPnP Job turned idle */
					printJob.resetReq = TRUE;
					EPS_RETURN( Ret );
				} else if (Ret != EPS_COM_TINEOUT){
					EPS_DBGPRINT(("PRINT--> Print Failed [%d]\r\n",Ret));
					EPS_RETURN( EPS_ERR_COMM_ERROR );
				} else if(sendSize > sentSize){
					Buffer  += sentSize;
					sendSize -= sentSize;
				}
			} else if (sendSize > sentSize){     /* CBT returned OK, but size is less */
				Ret = EPS_COM_TINEOUT;
				Buffer  += sentSize;
				sendSize -= sentSize;
			} else {
				sendSize -= sentSize;
				break;
			}
	
			if( !(EPS_PROTOCOL_USB & printJob.printer->protocol)
				|| NULL == epsCmnFnc.getTime ){
				/* The count is made an upper bound. */
				if(nRetry++ > EPS_TIMEOUT_NUM){
					break;
				}
			} else{
				/* The elapsed time is made an upper bound. */
				tmNow = epsCmnFnc.getTime();
				tmSpan = (EPS_UINT32)(tmNow - tmStart);
				if( tmSpan >= EPS_TIMEOUT_SEC ){
					break;
				}

				/* Wait */
				nDlyTotal -= nDlySpan;
				if(nDlySpan < 200){
					nDlySpan += nDlySpan/2;
					if(nDlySpan > 200){
						nDlySpan = 200;		/* max 200ms */
					}
				}
				serDelayThread(nDlySpan, &epsCmnFnc);
			}
		}

		if(0 < sendSize){
			if(TRUE == bSave){
				/* save the leftovers */
				/*EPS_DBGPRINT(("%d / Save %d byte\r\n", Ret, sendSize));*/
				printJob.contData.sendDataSize = sendSize;
				printJob.contData.sendData = Buffer;
			}

			Ret = MonitorStatus(NULL);
			if (Ret == EPS_JOB_CANCELED) {
				EPS_RETURN( Ret );
			} else{
				EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
			}
		} else{
			printJob.contData.sendDataSize = 0;
		}

		if(Ret != EPS_ERR_NONE){
			EPS_RETURN( EPS_ERR_PRINTER_ERR_OCCUR );
		}

	} else{
/***------------------------------------------------------------------------------------*/
/*** Uni-Directional Communication Mode                                                 */
/***------------------------------------------------------------------------------------*/
		retStatus = jobFnc.WriteData(Buffer, BuffLen, &sentSize);

        if ((retStatus == 0) && (sendSize == sentSize)) {
            *pSize = sentSize;
        } else {
            EPS_RETURN( EPS_ERR_COMM_ERROR );
        }
	}

/*** Return to Caller                                                                   */
    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     AdjustBasePoint()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A          void                N/A                                                 */
/*                                                                                      */
/* Return value:    N/A                                                                 */
/*                                                                                      */
/* Description:     Change the base point setting and the printable area value for      */
/*                  the custum border mode.                                             */
/*                                                                                      */
/*******************************************|********************************************/
static void     AdjustBasePoint (

        void

){
    /* temporary variables for max PAW/PAL */
    EPS_UINT32  maxPAWidthBorder;
    EPS_UINT32  maxPAHeightBorder;
    EPS_UINT32  RightMargin;
    EPS_UINT32  BottomMargin;
    
	EPS_LOG_FUNCIN;
    
    printJob.offset_x = 0;
    printJob.offset_y = 0;
    

/*** Adjust the base point for custum border printing mode                              */
/*** (In case that left margin = 42 and top margin = 42 are NOT inputed)                */
    if (printJob.borderlessMode == EPS_BORDER_CUSTOM) {
        if ( (printJob.attr.leftMargin > printJob.border.left) ||
             (printJob.attr.topMargin  > printJob.border.top )    ) {

            printJob.offset_x = (EPS_INT16)(printJob.attr.leftMargin - printJob.border.left);
            printJob.offset_y = (EPS_INT16)(printJob.attr.topMargin  - printJob.border.top);

            RightMargin  = printJob.paperWidth
                                - printJob.attr.leftMargin - printJob.printableAreaWidth;
            BottomMargin = printJob.paperHeight
                                - printJob.attr.topMargin  - printJob.printableAreaHeight;

            printJob.printableAreaWidth  = printJob.paperWidth
                                                - printJob.border.left - RightMargin;
            printJob.printableAreaHeight = printJob.paperHeight
                                                - printJob.border.top  - BottomMargin;

            printJob.leftMargin = (EPS_INT16)(printJob.border.left);
            printJob.topMargin  = (EPS_INT16)(printJob.border.top);

            /* max PAW/PAL */
            maxPAHeightBorder = (EPS_UINT32)(printJob.paperHeight 
                                    - printJob.border.top  - printJob.border.bottom);
            maxPAWidthBorder  = (EPS_UINT32)(printJob.paperWidth 
                                    - printJob.border.left - printJob.border.right);

            if (printJob.printableAreaHeight > maxPAHeightBorder) {
                printJob.printableAreaHeight = maxPAHeightBorder;
            }

            if (printJob.printableAreaWidth > maxPAWidthBorder) {
                printJob.printableAreaWidth = maxPAWidthBorder;
            }
        }
    }

	EPS_RETURN_VOID;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   PrintLine()                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* line         EPS_IMAGE*          I: Image Data Structure                             */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Filter Print Raster Data.                                                       */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE     PrintLine (

        EPS_IMAGE*  line

){
    EPS_RECT        AdjBandRec;     /* Rectangle after BasePointAdjustment              */
    EPS_BANDBMP     InBmp;          /* Input band data                                  */
	EPS_LOG_FUNCIN;
    /* Initialize input image structure */
    InBmp.bits       = line->data;
    InBmp.widthBytes = line->bytesPerLine;

    /* change rectangle due to base point adjustment */
    AdjBandRec.top    = line->rect.top    + printJob.offset_y;
    AdjBandRec.left   = line->rect.left   + printJob.offset_x;
    AdjBandRec.bottom = line->rect.bottom + printJob.offset_y;
    AdjBandRec.right  = line->rect.right  + printJob.offset_x;
    
    /* band is not visible */
    if ((EPS_UINT32)AdjBandRec.bottom > printJob.printableAreaHeight){
        EPS_RETURN( EPS_ERR_NONE );
    }

    if ((EPS_UINT32)AdjBandRec.right > printJob.printableAreaWidth){
        AdjBandRec.right = (EPS_INT32)printJob.printableAreaWidth;
    }

#ifdef GCOMSW_CMD_ESCPAGE
	if(EPS_LANG_ESCPR == printJob.printer->language ){
#endif
		/*** ESC/P-R ***/
		EPS_RETURN( SendLine(&InBmp, &AdjBandRec) );
#ifdef GCOMSW_CMD_ESCPAGE
	} else{
		/*** ESC/Page ***/
		EPS_RETURN( pageColorRow(&InBmp, &AdjBandRec) );
	}
#endif

}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   SendLine()                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pInBmp       const EPS_BANDBMP   I: [RGB] Image Data                                 */
/* pBandRec     EPS_RECT            I: Band rectangle information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Sent the data successfully                    */
/*      EPS_JOB_CANCELED                - Cancelled operation by user                   */
/*                                        (Do not return when Uni-Directional)          */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Send a line data to printer.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE     SendLine (

        const EPS_BANDBMP*  pInBmp,
        EPS_RECT*           pBandRec
        
){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    retStatus;                  /* Return status of internal calls          */
    EPS_UINT32      retBufSize;                 /* Size of buffer written                   */
    EPS_UINT32      cmdSize;                    /* All Size of Print Job Command            */
    EPS_UINT32      cpyCount;                   /* Counter for Set Command                  */
    EPS_UINT16      compDataSize;               /* Raster Data Size                         */
    EPS_UINT16      linePixelSize;              /* Raster Pixel Size                        */
    EPS_UINT8       compFlag;                   /* Compression flg (1:done compression  0:undone compression) */
    EPS_UINT32      paramSize;                  /* Parameter Length                         */
    EPS_UINT8       array2[2] = {0, 0};         /* Temporary Buffer for 2 byte Big Endian   */
    EPS_UINT8       array4[4] = {0, 0, 0, 0};   /* Temporary Buffer for 4 byte Big Endian   */
    EPS_UINT8*      compData;                   /* Compression Data Pointer                 */
    EPS_UINT8*      sdBuf;                      /* Send Data Buffer Pointer                 */
	const EPS_UINT8* srcAddr;
    
#if LCOMSW_PACKET_4KB
    EPS_INT32       rest_size;
    EPS_INT32       idx;
#endif

	EPS_LOG_FUNCIN;

/*** Initialize Local Variables                                                         */
    compFlag = EPS_RLE_COMPRESS_DONE;

/*** Initialize global valiable                                                         */
    memset(sendDataBuf, 0xFF, (EPS_UINT32)sendDataBufSize);
    memset(tmpLineBuf,  0xFF, (EPS_UINT32)tmpLineBufSize );

/*** Initialize valiable                                                                */
    sdBuf    = sendDataBuf;
    compData = tmpLineBuf;


/*    EPS_DBGPRINT(("MakeOneRasterData: T,B,L,R [%d,%d,%d,%d]\r\n", 
            pBandRec->top, pBandRec->bottom, pBandRec->left, pBandRec->right));
*/
    if( (EPS_UINT32)(pBandRec->right - pBandRec->left) <= printJob.printableAreaWidth){
        linePixelSize = (EPS_UINT16)(pBandRec->right - pBandRec->left);
    } else{
        linePixelSize = (EPS_UINT16) printJob.printableAreaWidth;
    }

#if ESCPR_DEBUG_IMAGE_LOG
    EPS_DBGPRINT(("ESCPRCMD : ImageData\r\n")
	EPS_DUMP(pInBmp->bits, (pBandRec->right - pBandRec->left) * 3)
#endif
    
	/*** Layout Filter */
	if(EPS_MLID_CDLABEL == printJob.attr.printLayout){
#ifdef GCOMSW_EL_CDLABEL
		elCDClipping(pInBmp->bits, sdBuf, printJob.bpp, pBandRec);
		srcAddr = sdBuf;
#else
		srcAddr = pInBmp->bits;
#endif

	} else{
		srcAddr = pInBmp->bits;
	}

	/*** RunLength Encode */
	
	compDataSize = RunLengthEncode(srcAddr,
                            compData,
                            linePixelSize,
                            printJob.bpp,
                            &compFlag);

	//compFlag = EPS_RLE_COMPRESS_NOT_DONE;
	if(compDataSize < 0){
		//return compDataSize;
	}
	
	/* Set Parameter Length */
    paramSize = (EPS_UINT32)(ESCPR_SEND_DATA_LENGTH + compDataSize);
    cmdSize   = ESCPR_HEADER_LENGTH + paramSize;
    
/*** Set Parameter                                                                      */
    cpyCount = 0;
    
    /* Header */
    memcpy(sdBuf, SendDataCmd, sizeof(SendDataCmd));
    cpyCount += sizeof(SendDataCmd);

    /* Parameter Length */
    memSetEndian(EPS_ENDIAN_LITTLE, EPS_4_BYTES, (EPS_UINT32)paramSize, array4);
    memcpy(sdBuf + cpyCount, array4, sizeof(array4));
    cpyCount += sizeof(array4);
    
    /* Command Name */
    memcpy(sdBuf + cpyCount, SendDataName, sizeof(SendDataName));
    cpyCount += sizeof(SendDataName);

    /* lXoffset */
    memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, (EPS_UINT32)pBandRec->left, array2);
    memcpy((sdBuf + cpyCount), array2, sizeof(array2));
    cpyCount += sizeof(array2);

    /* lYoffset */
	memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, (EPS_UINT32)pBandRec->top, array2);
    memcpy((sdBuf + cpyCount), array2, sizeof(array2));
    cpyCount += sizeof(array2);
    
    /* Compression Mode */
    if(compFlag == EPS_RLE_COMPRESS_DONE){
        *(sdBuf + cpyCount) = EPS_COMP_RLE;
    } else{
        *(sdBuf + cpyCount) = EPS_COMP_NON;
    }
    cpyCount += sizeof(EPS_UINT8);
    
    /* Raster Data Size */
    memSetEndian(EPS_ENDIAN_BIG, EPS_2_BYTES, (EPS_UINT32)compDataSize, array2);
    memcpy((sdBuf + cpyCount), array2, sizeof(array2));
    cpyCount += sizeof(array2);

    /* RGB Raster Data */
    memcpy((sdBuf + cpyCount), compData, compDataSize);
    cpyCount += compDataSize;
    
    if (cmdSize != cpyCount) {
        EPS_RETURN( EPS_ERR_OPR_FAIL );
    }
    
    
    /* Send Print Quality Command to Printer */
/*** -----------------------------------------------------*/
#if LCOMSW_PACKET_4KB   /* Set 1 packett size in 4096byte */
/*** -----------------------------------------------------*/
    if(cmdSize > ESCPR_PACKET_SIZE_4KB) {
        /* Send 4090 bytes Data                                                         */
        for(idx = 0; idx < (EPS_INT32)(cmdSize / ESCPR_PACKET_SIZE_4KB); idx++){

#if ESCPR_DEBUG_IMAGE_LOG
			EPS_DUMP(sdBuf, ESCPR_PACKET_SIZE_4KB)
#endif /*  ESCPR_DEBUG_IMAGE_LOG */

            retBufSize = 0;

            retStatus = SendCommand(sdBuf, ESCPR_PACKET_SIZE_4KB, &retBufSize, TRUE);
            if (!((retStatus == EPS_ERR_NONE) && (ESCPR_PACKET_SIZE_4KB == retBufSize))) {
                EPS_RETURN( retStatus );
            }
            sdBuf += ESCPR_PACKET_SIZE_4KB;
        }
        
        /* Send Rest Data                                                               */
        rest_size  = cmdSize - ((EPS_INT32)(cmdSize / ESCPR_PACKET_SIZE_4KB)) * ESCPR_PACKET_SIZE_4KB;
        retBufSize = 0;

        retStatus = SendCommand(sdBuf, rest_size, &retBufSize, TRUE);
        if (!((retStatus == EPS_ERR_NONE) && (rest_size == retBufSize))) {
            EPS_RETURN( retStatus );
        }

#if ESCPR_DEBUG_IMAGE_LOG
		EPS_DUMP(sdBuf, rest_size)
#endif /*  ESCPR_DEBUG_IMAGE_LOG */

    } else /* if(cmdSize <= ESCPR_PACKET_SIZE_4KB) */{

#if ESCPR_DEBUG_IMAGE_LOG
		EPS_DUMP(sdBuf, cmdSize)
#endif /*  ESCPR_DEBUG_IMAGE_LOG */

        retBufSize = 0;

        retStatus = SendCommand(sdBuf, cmdSize, &retBufSize, TRUE);
        if (!((retStatus == EPS_ERR_NONE) && (cmdSize == retBufSize))) {
            EPS_RETURN( retStatus );
        }

    }

/*** -----------------------------------------------------*/
#else /*  LCOMSW_PACKET_4KB */
/*** -----------------------------------------------------*/

#if ESCPR_DEBUG_IMAGE_LOG
	EPS_DUMP(sdBuf, cpyCount)
#endif /*  ESCPR_DEBUG_IMAGE_LOG */

    retBufSize = 0;
    retStatus = SendCommand(sdBuf, cmdSize, &retBufSize, TRUE);
    if (!((retStatus == EPS_ERR_NONE) && (cmdSize == retBufSize))) {
        EPS_RETURN( retStatus );
    }

/*** -----------------------------------------------------*/
#endif /* LCOMSW_PACKET_4KB */
/*** -----------------------------------------------------*/

    EPS_RETURN( EPS_ERR_NONE );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   RunLengthEncode()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrcAddr     const EPS_UINT8*    I: An address of original raster                    */
/* pDstAddr     EPS_UINT8*          O: An address of compressed raster                  */
/* pixel        EPS_INT16           I: Original Raster Size                             */
/* bpp          EPS_UINT8           I: Bytes Per Pixel                                  */
/* pCompress    EPS_UINT8*          O: Compression Flag                                 */
/*                                                                                      */
/* Return value:                                                                        */
/*      Compressed Data Size (byte)                                                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Runlength Compression for RGB Data.                                             */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_UINT16    RunLengthEncode (

        const EPS_UINT8*    pSrcAddr,
        EPS_UINT8*          pDstAddr,
        EPS_UINT16          pixel,
        EPS_UINT8           bpp,
        EPS_UINT8*          pCompress

){
	const EPS_UINT8* pSrcPos = pSrcAddr;           /* pointer to srcBuffer               */
	EPS_UINT8*   pDstPos     = pDstAddr;           /* pointer to destBuffer              */
    EPS_UINT16   srcCnt      = 0;                  /* counter                            */
    EPS_UINT16   repCnt      = 0;                  /* replay conter                      */
    EPS_UINT16   retCnt      = 0;                  /* conpressed data size               */
    EPS_UINT32   copySize    = 0;
    EPS_UINT16   widthPixels = pixel * bpp;
	EPS_BOOL     bCompress   = TRUE;

	EPS_LOG_FUNCIN;
int i_index = 0;
    while (srcCnt < pixel) {
        /* In case of replay data                                                       */
        if ((srcCnt + 1 < pixel) && (!memcmp(pSrcPos, pSrcPos + bpp, bpp))) {
            repCnt = 2;
            while ((srcCnt + repCnt < pixel)                                              &&
                   (repCnt < 0x81)                                                        &&
                   (!memcmp(pSrcPos + (repCnt - 1) * bpp, pSrcPos + repCnt * bpp, bpp))    ) {
                repCnt++;
            }
		
            /* Renewal compressed data size counter */
            retCnt += 1 + bpp;

			/* If compressed data size is bigger than original data size,               */
			/* stop compression process.                                                */
			if( retCnt > widthPixels ){
				bCompress  = FALSE;
				retCnt -= 1 + bpp;		/* rewind counter */
				break;
			}

			/* Set replay count and data                                                */
            /* Set data counter                     */
             if(pDstPos == NULL || (pDstPos+1) == NULL || (pDstPos + bpp) == NULL || pSrcPos == NULL ||( pSrcPos + bpp * repCnt) == NULL){
            	return i_index * (-1);
            }
            *pDstPos++ = (EPS_UINT8)(0xFF - repCnt + 2 );
            
            /* Set data                             */
            memcpy(pDstPos, pSrcPos, bpp);

            /* Renewal data size counter            */
            srcCnt += repCnt;
            i_index += bpp * repCnt;

            /* Renewal original data address        */
            if((pSrcPos + bpp * repCnt) == NULL){
            	return i_index * (-1);
            }
            pSrcPos += bpp * repCnt;

            /* Renewal compressed data address      */
            pDstPos += bpp;


        }

        /* In case of non replay data                                                   */
        else {
            copySize = 0;
            repCnt   = 1;

            /* compare next data with next and next data                                */
            while ((srcCnt + repCnt + 1< pixel)                                          &&
                   (repCnt < 0x80)                                                       &&
                   (memcmp(pSrcPos + repCnt * bpp, pSrcPos + (repCnt + 1) * bpp, bpp))    ){
                repCnt++;
            }
            
            /* Renewal compressed data size counter */
            retCnt += 1 + repCnt * bpp;

			/* If compressed data size is bigger than original data size,               */
			/* stop compression process.                                                */
			if( retCnt > widthPixels ){
				bCompress  = FALSE;
				retCnt -= 1 + repCnt * bpp;	/* rewind counter */
				break;
			}

			/* Set data counter                     */
			if(pDstPos == NULL || (pDstPos +1) == NULL){
				return i_index *(-1);
			}
            *pDstPos++ = (EPS_UINT8)(repCnt - 1);
            /* Renewal data size counter            */

            /* Size of non replay data (byte)       */
            srcCnt  += repCnt;
            copySize = (EPS_UINT32)(repCnt * bpp);
		if(pDstPos == NULL || (pDstPos +1) == NULL || (pDstPos + copySize) == NULL || (pSrcPos + copySize) == NULL || pSrcPos == NULL){
            			return i_index * (-1);
            		}
            		
            /* Set data                             */
            memcpy(pDstPos, pSrcPos, copySize);

            /* Renewal original data address        */
            pSrcPos += copySize;

            /* Renewal compressed data address      */
            pDstPos += copySize;
            i_index += copySize;
        }
    }

	if(TRUE == bCompress){
	    *pCompress = EPS_RLE_COMPRESS_DONE;
	} else{
		retCnt = widthPixels;
		memcpy(pDstAddr, pSrcAddr, widthPixels);
	    *pCompress = EPS_RLE_COMPRESS_NOT_DONE;
	}
    
    EPS_RETURN( retCnt );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   CreateMediaInfo()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:               Description:									*/
/* innerPrinter     EPS_PRINTER_INN*    I: printer that it has original structure		*/
/* pmString			EPS_INT8*			I: PM reply string								*/
/* pmSize			EPS_INT32			I: size of PM reply string						*/
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Marge paper source to EPS_SUPPORTED_MEDIA.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    CreateMediaInfo(
	
		EPS_PRINTER_INN*	innerPrinter,
		EPS_UINT8*			pmString,
		EPS_INT32			pmSize
	
){
	EPS_ERR_CODE	retStatus = EPS_ERR_NONE;
    EPS_UINT8*      cmdField;                   /* Pointer of pm command field          */
    EPS_INT32       pmIdx;                      /* pm string index                      */
    EPS_UINT32      idx;                        /* Index                                */
    EPS_INT32       sIdx = 0;                   /* Media size index                     */
    EPS_INT32       tIdx = 0;                   /* Media type index                     */
    EPS_INT32       num_mType = 0;              /* Media type number                    */
    EPS_INT32       num_mSize = 0;              /* Media size number                    */
	EPS_BOOL		extPaper = FALSE;			/* extend paper source is exist         */

	EPS_LOG_FUNCIN;

	/*** Is this really "PM" data                                                       */
	cmdField = (EPS_UINT8*)strstr((const char*)pmString,"PM");
	if (cmdField == NULL ) {
		EPS_DBGPRINT(("Get Model Info faild : ModelInfo = [%s]\r\n",pmString));
		EPS_RETURN( EPS_ERR_OPR_FAIL );
	}
EPS_DUMP(pmString, 256);

#if _VALIDATE_SUPPORTED_MEDIA_DATA_
	if(innerPrinter->supportedMedia.numSizes != -1){
		/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                      */
		retStatus = _SP_ChangeSpec_UpdatePMReply(innerPrinter, pmString, pmSize);
		if (retStatus != EPS_ERR_NONE) {
			EPS_RETURN( retStatus );
		}
	}
#else
/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                              */
    retStatus = _SP_ChangeSpec_UpdatePMReply(innerPrinter, pmString, pmSize);
    if (retStatus != EPS_ERR_NONE) {
        EPS_RETURN( EPS_ERR_OPR_FAIL );  /* Invalid format */
    }
#endif

/*** Create the structure of the support media                                          */
	innerPrinter->supportedMedia.resolution = EPS_IR_360X360;	/* default support */

    /*** Count "Paper Size" field  & check format */
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */
    while( pmIdx < EPS_PM_MAXSIZE ) {
        switch(pmString[pmIdx]) {
		case 'R':
			if( 720 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				innerPrinter->supportedMedia.resolution |= EPS_IR_720X720;
			} else if( 600 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				innerPrinter->supportedMedia.resolution |= EPS_IR_300X300 | EPS_IR_600X600;
			} else if( 300 == ((pmString[pmIdx+1] << 8) + pmString[pmIdx+2]) ){
				innerPrinter->supportedMedia.resolution |= EPS_IR_300X300;
			}
			pmIdx += 6;
			break;

		case 'M':
			innerPrinter->supportedMedia.JpegSizeLimit = 
				(pmString[pmIdx+1] << 24) + (pmString[pmIdx+2] << 16) + (pmString[pmIdx+3] << 8) + pmString[pmIdx+4]; 
			innerPrinter->JpgMax = innerPrinter->supportedMedia.JpegSizeLimit;
			pmIdx += 6;
			break;

		case 'S':
			/* move T field */
			if(pmIdx < EPS_PM_MAXSIZE-2){
				pmIdx += 2;
			} else{
				EPS_RETURN( EPS_ERR_OPR_FAIL );
			}

			num_mSize++;

			for(; pmIdx < EPS_PM_MAXSIZE-4; pmIdx += 4) {	/* 4 = T x x / */
				if(pmString[pmIdx] == '/'){
					pmIdx += 1;
					break;
				} else if(pmString[pmIdx] != 'T') {
					EPS_RETURN( EPS_ERR_OPR_FAIL );
				}
			}
			if(pmIdx >= EPS_PM_MAXSIZE-4){
				EPS_RETURN( EPS_ERR_OPR_FAIL );
			}
			break;

		default:
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == 0x0D && pmString[pmIdx+1] == 0x0A) {
			break;
		}
	}                      
	if(pmIdx >= EPS_PM_MAXSIZE){
		EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
	}
    
    /* Allocate memory for the media size list. */
    innerPrinter->supportedMedia.sizeList =
            (EPS_MEDIA_SIZE*)EPS_ALLOC( sizeof(EPS_MEDIA_SIZE) * num_mSize );   
    if( innerPrinter->supportedMedia.sizeList == NULL ){
        EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
    }
	memset(innerPrinter->supportedMedia.sizeList, 0, sizeof(EPS_MEDIA_SIZE) * num_mSize);
    innerPrinter->supportedMedia.numSizes = num_mSize;
    
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */
    for(sIdx = 0; sIdx < num_mSize; sIdx++) {
		if(pmString[pmIdx] == 'M' || pmString[pmIdx] == 'R') {
			pmIdx += 6;
			sIdx--;
			continue;
		}

        innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID = pmString[pmIdx+1];
/*		EPS_DBGPRINT(("Size=%d\r\n", pmString[pmIdx+1]));*/
		pmIdx += 2;

        /* For the given paper type, iterate through the paper type to get the number
         * of media types contained in it */
        num_mType = 0;
		for(idx = pmIdx; idx < EPS_PM_MAXSIZE-4; idx += 4) {
			if(pmString[idx] == '/'){
				idx += 1;
				break;
			}
			num_mType++;
		}

        /* Allocate memory for the media type array. */
        innerPrinter->supportedMedia.sizeList[sIdx].typeList = 
            (EPS_MEDIA_TYPE*)EPS_ALLOC( sizeof(EPS_MEDIA_TYPE) * num_mType );
        
        if (innerPrinter->supportedMedia.sizeList[sIdx].typeList == NULL) {
            EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
        }
        
		memset(innerPrinter->supportedMedia.sizeList[sIdx].typeList, 0, sizeof(EPS_MEDIA_TYPE) * num_mType);
        innerPrinter->supportedMedia.sizeList[sIdx].numTypes = num_mType;

		for(tIdx = 0; tIdx < num_mType; tIdx++) {
            innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].mediaTypeID       = pmString[pmIdx+1];
/*            EPS_DBGPRINT(("\tType=%d (%02X)\r\n", pmString[pmIdx+1], pmString[pmIdx+2]));*/
                               
            /* Bitwise OR with 10000000 - Check for borderless */
            if( pmString[pmIdx+2] & 0x80 ){
                innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERLESS;
            }
            /* Bitwise OR with 01000000 - Check for border "disable" mode */
            if( !(pmString[pmIdx+2] & 0x40) ){
				innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].layout |= EPS_MLID_BORDERS;
            }

            /* set quality */
            innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].quality |= (pmString[pmIdx+2] & EPS_MQID_ALL);

			/* set duplex */
            if( pmString[pmIdx+2] & 0x10 &&
				obsEnableDuplex(innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID) ){
				innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_ENABLE;/*EPS_DUPLEX_SHORT*/;
			} else{
				innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].duplex = EPS_DUPLEX_DISABLE;
			}

			/* Bitwise OR with 00001000 - Check for extend paper source */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
            if( pmString[pmIdx+2] & 0x08 ){
                extPaper = TRUE;
			} else {
				/* DEFAULT. All printer support rear paper source */
				innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].paperSource = EPS_MPID_REAR;
			}

			/* param2 value check */
			if( !(pmString[pmIdx+2] & (0x01 | 0x02 | 0x04)) ){
				printf("\n\n!!!!!!!!!  Quality is not described. !!!!!!!!!\n"
						"SizeID=0x%02X / TypeID=0x%02X / param2=0x%02X\n", 
						innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID,
						pmString[pmIdx+1], pmString[pmIdx+2]);
			} 
			if( !(pmString[pmIdx+2] & 0x80) && (pmString[pmIdx+2] & 0x40) ){
				printf("\n\n!!!!!!!!!  Layout is not described. !!!!!!!!!\n"
						"SizeID=0x%02X / TypeID=0x%02X / param2=0x%02X\n", 
						innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID,
						pmString[pmIdx+1], pmString[pmIdx+2]);
			}

#else
            if( pmString[pmIdx+2] & 0x08 ){
                extPaper = TRUE;
			}
			/* DEFAULT. All printer support rear paper source */
			innerPrinter->supportedMedia.sizeList[sIdx].typeList[tIdx].paperSource = EPS_MPID_REAR;
#endif
			pmIdx += 4;	/* move next field */
		}
		pmIdx += 1;	/* skip terminater */
	}

/*** Add extend infomation                                                              */
	if( EPS_ERR_NONE == retStatus && TRUE == extPaper ){
		retStatus = GetPaperSource(innerPrinter);
		if( EPS_ERR_NONE != retStatus){
			prtClearSupportedMedia(innerPrinter);
		}
	}

	serAppendMedia(&innerPrinter->supportedMedia);

	EPS_RETURN( retStatus );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetPaperSource()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* innerPrinter     EPS_PRINTER_INN*       I: printer that it has original structure    */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Marge paper source to EPS_SUPPORTED_MEDIA.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    GetPaperSource(
	
		EPS_PRINTER_INN*		innerPrinter
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_UINT8       pmString[EPS_PM_MAXSIZE];   /* Retrieved PM data from printer       */
    EPS_INT32       pmSize = EPS_PM_MAXSIZE;
    EPS_UINT32      pmIdx;                      /* pm string index                      */
    EPS_INT32       sIdx = 0;                   /* Media size index                     */
    EPS_INT32       tIdx = 0;                   /* Media type index                     */
	EPS_MEDIA_SIZE  *pMSize = NULL;

	EPS_LOG_FUNCIN;

	/* Clear the Printer Model Information (Media data or "PM" data)                    */
    memset(pmString, 0, EPS_PM_MAXSIZE);

/*** Get PM2 from Printer                                                               */
	ret = prtGetPMString(innerPrinter, 2, pmString, &pmSize);
	if(ret == EPS_ERR_PROTOCOL_NOT_SUPPORTED){
		EPS_RETURN( EPS_ERR_NONE );
	} else if(ret != EPS_ERR_NONE){
		EPS_RETURN( ret );
	}

	/*** Is this really "PM" data                                                       */
	if( strstr((const char*)pmString, "PM") == NULL ) {
		EPS_DBGPRINT(("Get Model Info faild : ModelInfo = [%s]\r\n", pmString));
		EPS_RETURN( EPS_ERR_OPR_FAIL );
	}

	/* Delete the command header of pm string                                           */
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */

/*** Check to make sure the PM reply has a valid beginning                              */
    if(pmString[pmIdx] != 'S' && pmString[pmIdx+2] != 'T') {
        EPS_RETURN( EPS_ERR_OPR_FAIL );
    }
    
/*** Create the structure of the support media                                          */
    /*** Count "Paper Size" field  & check format */
    for(; pmIdx < EPS_PM_MAXSIZE-7; ) {		/* 7 = S x T x x // */
        if(pmString[pmIdx] != 'S') {
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
		}
		
		/* search size ID */
		pmIdx++;
		pMSize = NULL;
	    for(sIdx = 0; sIdx < innerPrinter->supportedMedia.numSizes; sIdx++){
			if(pmString[pmIdx] == innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID){
				pMSize = &innerPrinter->supportedMedia.sizeList[sIdx];
/*				EPS_DBGPRINT(("Size = %d\n", innerPrinter->supportedMedia.sizeList[sIdx].mediaSizeID))*/
				break;
			}
		}

		pmIdx++;	/* move next field */

		while( pmIdx < EPS_PM_MAXSIZE-4 ){	/* 4 = T x x / */
			if(pmString[pmIdx] == 'T'){
				if(NULL != pMSize){
					/* search type ID */
					pmIdx++;
					for(tIdx = 0; tIdx < pMSize->numTypes; tIdx++){
						if(pmString[pmIdx] == pMSize->typeList[tIdx].mediaTypeID){
							pMSize->typeList[tIdx].paperSource = pmString[pmIdx+1];
#if !_VALIDATE_SUPPORTED_MEDIA_DATA_
							pMSize->typeList[tIdx].paperSource &= EPS_MPID_ALL_ESCPR;
#endif
							pmIdx += 3;
							break;
						}
					}
					if(tIdx >= pMSize->numTypes){
						/* Skip unknown T */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
						printf("\n\n!!!!!!!!! pm2 contains TypeID(0x%02X) that doesn't exist in pm1.  !!!!!!!!!\n", pmString[pmIdx]);
#endif
						pmIdx += 3;
					}
				} else{
					/* Skip unknown S */
#if _VALIDATE_SUPPORTED_MEDIA_DATA_
						printf("\n\n!!!!!!!!! pm2 contains SizeID(0x%02X) that doesn't exist in pm1.  !!!!!!!!!\n", pmString[pmIdx]);
#endif
					pmIdx += 4;
				}
			} else if(pmString[pmIdx] == '/') {
				pmIdx++;
				break;
			} else{
				EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
			}
		}
		if(pmIdx >= EPS_PM_MAXSIZE-4){
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
		}
		
		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == 0x0D && pmString[pmIdx+1] == 0x0A) {
			break;
		}
	}                      
	if(pmIdx >= EPS_PM_MAXSIZE){
		EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   GetJpgMax()                                                         */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* innerPrinter     EPS_PRINTER_INN*       IO: printer that it has original structure   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_OPR_FAIL                - Internal Error                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Marge paper source to EPS_SUPPORTED_MEDIA.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    GetJpgMax(
	
		EPS_PRINTER_INN*		printer
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_UINT8       pmString[EPS_PM_MAXSIZE];   /* Retrieved PM data from printer       */
    EPS_INT32       pmSize = EPS_PM_MAXSIZE;
    EPS_UINT32      pmIdx;                      /* pm string index                      */

	EPS_LOG_FUNCIN;

	if( !EPS_IS_BI_PROTOCOL(printJob.printer->protocol) ){	/* Uni direction */
		/* set unlimited(2GB) */
		printer->JpgMax = EPS_JPEG_SIZE_UNLIMIT;
		EPS_RETURN( EPS_ERR_NONE )
	}

	/* Clear the Printer Model Information (Media data or "PM" data)                    */
    memset(pmString, 0, EPS_PM_MAXSIZE);

/*** Get PM1 from Printer                                                               */
	ret = prtGetPMString(printer, 1, pmString, &pmSize);
	if(ret == EPS_ERR_PROTOCOL_NOT_SUPPORTED){
		/* set unlimited(2GB) */
		printer->JpgMax = EPS_JPEG_SIZE_UNLIMIT;
		EPS_RETURN( EPS_ERR_NONE )
	} else if(ret != EPS_ERR_NONE){
		EPS_RETURN( ret );
	}

/*** "Filter" Raw "PM" data (Remake the correct pm stirng)                              */
    ret = _SP_ChangeSpec_UpdatePMReply(printer, pmString, pmSize);
    if (ret != EPS_ERR_NONE) {
        EPS_RETURN( EPS_ERR_OPR_FAIL );  /* Invalid format */
    }

	/*** Is this really "PM" data                                                       */
	if( strstr((const char*)pmString, "PM") == NULL ) {
		EPS_DBGPRINT(("Get Model Info faild : ModelInfo = [%s]\r\n", pmString));
		EPS_RETURN( EPS_ERR_OPR_FAIL );
	}

	/* Delete the command header of pm string                                           */
	pmIdx = EPS_PM_HEADER_LEN;				/* skip the command header of pm string     */
    while( pmIdx < EPS_PM_MAXSIZE ) {
        switch(pmString[pmIdx]) {
		case 'M':
			printer->JpgMax = 
				(pmString[pmIdx+1] << 24) + (pmString[pmIdx+2] << 16) + (pmString[pmIdx+3] << 8) + pmString[pmIdx+4]; 

			pmIdx += 6;
			break;

		case 'R':
			pmIdx += 6;
			break;

		case 'S':
			/* move T field */
			if(pmIdx < EPS_PM_MAXSIZE-2){
				pmIdx += 2;
			} else{
				EPS_RETURN( EPS_ERR_OPR_FAIL );
			}

			for(; pmIdx < EPS_PM_MAXSIZE-4; pmIdx += 4) {	/* 4 = T x x / */
				if(pmString[pmIdx] == '/'){
					pmIdx += 1;
					break;
				} else if(pmString[pmIdx] != 'T') {
					EPS_RETURN( EPS_ERR_OPR_FAIL );
				}
			}
			if(pmIdx >= EPS_PM_MAXSIZE-4){
				EPS_RETURN( EPS_ERR_OPR_FAIL );
			}
			break;

		default:
			EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
		}

		/* If we run into an occurrence of carriage return followed by line feed,
		 * we have found the terminating characters of the string. */
		if(pmString[pmIdx] == 0x0D && pmString[pmIdx+1] == 0x0A) {
			break;
		}
	}                      
	if(pmIdx >= EPS_PM_MAXSIZE || 0 == printer->JpgMax){
		EPS_RETURN( EPS_ERR_OPR_FAIL ); /* bad format */
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   DuplSupportedMedia()                                                */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                  Description:                                 */
/* innerPrinter     EPS_PRINTER_INN*       I: printer that it has original structure    */
/* pMedia           EPS_SUPPORTED_MEDIA*   O: pointer to a distination                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Alloc memory failed                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Duplicate EPS_SUPPORTED_MEDIA to user buffer.                                   */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    DuplSupportedMedia(
	
		EPS_PRINTER_INN*		innerPrinter,
		EPS_SUPPORTED_MEDIA*    pMedia
	
){
	EPS_ERR_CODE	ret = EPS_ERR_NONE;
    EPS_INT32       idx;

	EPS_LOG_FUNCIN;

	ClearSupportedMedia();

	g_supportedMedia.JpegSizeLimit = innerPrinter->supportedMedia.JpegSizeLimit;
	g_supportedMedia.resolution = innerPrinter->supportedMedia.resolution;
    g_supportedMedia.numSizes = innerPrinter->supportedMedia.numSizes;
    g_supportedMedia.sizeList = (EPS_MEDIA_SIZE*)EPS_ALLOC( sizeof(EPS_MEDIA_SIZE) * innerPrinter->supportedMedia.numSizes );
	if( g_supportedMedia.sizeList ){
		for(idx = 0; idx < innerPrinter->supportedMedia.numSizes; idx++) {
			g_supportedMedia.sizeList[idx].mediaSizeID = innerPrinter->supportedMedia.sizeList[idx].mediaSizeID;
			g_supportedMedia.sizeList[idx].numTypes    = innerPrinter->supportedMedia.sizeList[idx].numTypes;
			g_supportedMedia.sizeList[idx].typeList = 
                    (EPS_MEDIA_TYPE*)EPS_ALLOC( sizeof(EPS_MEDIA_TYPE) * innerPrinter->supportedMedia.sizeList[idx].numTypes );
			if( g_supportedMedia.sizeList[idx].typeList ){
				memcpy(g_supportedMedia.sizeList[idx].typeList, 
						innerPrinter->supportedMedia.sizeList[idx].typeList,
						sizeof(EPS_MEDIA_TYPE) * innerPrinter->supportedMedia.sizeList[idx].numTypes);
			} else{
				ret = EPS_ERR_MEMORY_ALLOCATION;
				break;
			}
		}
	} else{
		ret = EPS_ERR_MEMORY_ALLOCATION;
	}

	if(EPS_ERR_NONE == ret){
		/* Copy to out param */
        pMedia->JpegSizeLimit = g_supportedMedia.JpegSizeLimit;
 		pMedia->resolution = g_supportedMedia.resolution;
		pMedia->numSizes = g_supportedMedia.numSizes;
        pMedia->sizeList = g_supportedMedia.sizeList;

	} else{
		/* If error occur, unwind. */
		for(idx = 0; idx < g_supportedMedia.numSizes; idx++) {
			EPS_SAFE_RELEASE(g_supportedMedia.sizeList[idx].typeList);
		}
		EPS_SAFE_RELEASE(g_supportedMedia.sizeList);
		g_supportedMedia.numSizes = 0;
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     ClearSupportedMedia()                                             */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:            Type:                   Description:                                */
/* (none)                                                                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      void                                                                            */
/*                                                                                      */
/* Description:                                                                         */
/*      Crean up inside list of supported media structure.                              */
/*                                                                                      */
/*******************************************|********************************************/
void ClearSupportedMedia(
							
	void

){
    EPS_INT32       idx;

	EPS_LOG_FUNCIN;

	/* Clear "supportedMedia"                                              */
	if( NULL != g_supportedMedia.sizeList){
		for(idx = 0; idx < g_supportedMedia.numSizes; idx++) {
			EPS_SAFE_RELEASE(g_supportedMedia.sizeList[idx].typeList);
		}
		EPS_SAFE_RELEASE(g_supportedMedia.sizeList);
		g_supportedMedia.numSizes = 0;
	}

	EPS_RETURN_VOID;
}

/*_______________________________   epson-escpr-api.c   ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
