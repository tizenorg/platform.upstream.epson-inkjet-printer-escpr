/*________________________________   epson-net-snmp.c   ________________________________*/

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
/*                                SNMP protocol Module                                  */
/*                                                                                      */
/*                                Public Function Calls                                 */
/*                              --------------------------                              */
/*      EPS_ERR_CODE snmpFindStart          (sock                               );      */
/*		EPS_ERR_CODE snmpFind               (sock, printer                      );      */
/*      EPS_ERR_CODE snmpFindEnd	        (sock                               );      */
/*      EPS_ERR_CODE snmpProbeByID          (printerUUID, timeout, printer      );      */
/*      EPS_ERR_CODE snmpGetStatus          (sock, address, pstInfo             );      */
/*      EPS_ERR_CODE snmpGetPMString        (printer, pString, bufSize          );      */
/*      EPS_ERR_CODE snmpMechCommand        (printer, Command                   );      */
/*      EPS_ERR_CODE snmpOpenSocket			(sock                               );      */
/*		EPS_ERR_CODE snmpCloseSocket		(sock                               );      */
/*		EPS_ERR_CODE mibGetPhysAddress		(address, val, vallen				);		*/
/*		EPS_ERR_CODE mibConfirmPrintPort	(address, printPort					);		*/
/*                                                                                      */
/*******************************************|********************************************/

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-escpr-def.h"
#include "epson-escpr-err.h"
#include "epson-escpr-mem.h"
#include "epson-escpr-services.h"
#include "epson-protocol.h"
#include "epson-net-snmp.h"
#ifdef GCOMSW_CMD_ESCPAGE
	#include "epson-escpr-media.h"

	#ifdef GCOMSW_CMD_ESCPAGE_S
	#include "epson-escpage-s.h"
	#endif
#endif

/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/
#ifdef EPS_LOG_MODULE_LPR
#define EPS_LOG_MODULE	EPS_LOG_MODULE_LPR
#else
#define EPS_LOG_MODULE	0
#endif

#define SNMP_PORT_NUM			(161)				/* Protocol port number				*/
#define SNMP_MAX_BUF			(512)				/* Communication buffer size		*/
#define SNMP_MAX_OID			(128)				/* max oid size		                */

/* SNMP variables                                                                       */
#define ASN_VT_INTEGER			(0x02)				/* integer                          */
#define ASN_VT_OCTET_STRING		(0x04)				/* octet stream                     */
#define ASN_VT_NULL				(0x05)				/* null                             */
#define ASN_VT_OBJECT_ID		(0x06)				/* OID                              */
#define ASN_VT_SEQUENCE			(0x30)				/* sequence                         */

/* SNMP message                                                                         */
#define ASN_PDU_GET				(EPS_UINT8)(0xA0)   /* GetRequest                       */
#define ASN_PDU_GET_NEXT		(EPS_UINT8)(0xA1)   /* GetNextRequest                   */
#define ASN_PDU_RESP			(EPS_UINT8)(0xA2)   /* Response                         */
#define ASN_PDU_SET				(EPS_UINT8)(0xA3)   /* SetRequest                       */
#define ASN_PDU_TRAP			(EPS_UINT8)(0xA4)   /* Trap                             */
#define ASN_PDU_TRAP_RSP		(EPS_UINT8)(0xC2)   /* TrapResponse                     */


#define SNMP_VERSION			(0)
#define SNMP_COMMUNITY_STR		"public"
/* Response corde */
#define SNMP_ERR_NONE			(0)					/* noError(0)						*/
#define SNMP_ERR_TOO_BIG		(1)					/* tooBig(1)						*/
#define SNMP_ERR_SUCH_NAME		(2)					/* noSuchName(2)					*/
#define SNMP_ERR_BAD_VALUE		(3)					/* badValue(3)						*/
#define SNMP_ERR_READ_ONLY		(4)					/* readOnly(4)						*/
#define SNMP_ERR_GENERAL		(5)					/* genErr(5)						*/

#define SNMP_OBJID_LEN			(129)

/* MIB status */
	/* hrDeviceStatus */
#define MIB_DEVST_RUNNING		(2)
#define MIB_DEVST_WARNING		(3)
#define MIB_DEVST_DOWN			(5)
	/* hrPrinterStatus */
#define MIB_PRNST_OTHER			(1)
#define MIB_PRNST_IDLE			(3)
#define MIB_PRNST_PRINTING		(4)
#define MIB_PRNST_WARMUP		(5)
	/* hrPrinterDetectedErrorState */
#define MIB_PRERR_NOPAPER		(1 << 6)
#define MIB_PRERR_NOINK			(1 << 4)
#define MIB_PRERR_COVEROPEN		(1 << 3)
#define MIB_PRERR_PAPERJAM		(1 << 2)

/*---------------------------  Data Structure Declarations   ---------------------------*/
/*******************************************|********************************************/
/* SNMP variant type */
typedef struct tag_ASN_VARIANT {       
	EPS_INT8	type;
	EPS_UINT32	length;
	union{
		EPS_INT32	v_long;            /* Integer                                       */
		EPS_INT8*	v_str;             /* OCTET_STRING, ASN_VT_OBJECT_ID                */
	} val;				
}ASN_VARIANT;

/* If type is OBJECT_ID, ParseField() alloc heap.   									*/
#define EPS_REREASE_VARIANT( v )				\
	{											\
		if( ASN_VT_OBJECT_ID == (v).type ){		\
			EPS_SAFE_RELEASE( (v).val.v_str );	\
		}										\
		(v).type = ASN_VT_NULL;					\
	}


/*----------------------------  ESC/P-R Lib Global Variables  --------------------------*/
/*******************************************|********************************************/

	/*** Extern Function                                                                */
extern EPS_NET_FUNC		epsNetFnc;
extern EPS_CMN_FUNC		epsCmnFnc;

	/*** Find                                                                           */
extern EPS_BOOL			g_FindBreak;			/* Find printer end flag                */

	/* CPU Endian-ness                                                                  */
extern EPS_INT16		cpuEndian;

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
     interfaces(2) ifTable(2) ifEntry(1) 6 } */
static EPS_INT8 s_oidPhysAddress[]  = "1.3.6.1.2.1.2.2.1.6";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
	 tcp(6) tcpConnTable(13) tcpConnEntry(1) 3 } */
static EPS_INT8 s_oidTcpConnLocalPort[]= "1.3.6.1.2.1.6.13.1.3";


#ifdef GCOMSW_CMD_ESCPAGE
/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
      host(25) hrDevice(3) hrDeviceTable(2) hrDeviceEntry(1) hrDeviceStatus(5) } */
static EPS_INT8 s_oidDevStatus[]	= "1.3.6.1.2.1.25.3.2.1.5";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
     host(25) hrDevice(3) hrPrinterTable(5) hrPrinterEntry(1) hrPrinterStatus(1) } */
static EPS_INT8 s_oidPrinterStatus[]= "1.3.6.1.2.1.25.3.5.1.1";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
     host(25) hrDevice(3) hrPrinterTable(5) hrPrinterEntry(1) hrPrinterDetectedErrorState(2) } */
static EPS_INT8 s_oidPrinterError[] = "1.3.6.1.2.1.25.3.5.1.2";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) printmib(43) prtMarkerColorant(12)
      prtMarkerColorantTable(1) prtMarkerColorantEntry(1) 4 } */
static EPS_INT8 s_oidMarkerColorant[] = "1.3.6.1.2.1.43.12.1.1.4";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
     printmib(43) prtMarkerSupplies(11) prtMarkerSuppliesTable(1) prtMarkerSuppliesEntry(1)
	 prtMarkerSuppliesLevel(9) } */
static EPS_INT8 s_oidMarkerLevel[]	= "1.3.6.1.2.1.43.11.1.1.9";

/* { iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) 
     printmib(43) prtInput(8) prtInputTable(2) prtInputEntry(1) prtInputName(13) } */
static EPS_INT8 s_oidInputName[]	= "1.3.6.1.2.1.43.8.2.1.13";
#endif

static EPS_INT8 s_oidPrvPrinter[]		= "1.3.6.1.4.1.1248.1.2.2.1.1.1.1";
static EPS_INT8 s_oidPrvBonjourName[]	= "1.3.6.1.4.1.1248.1.1.3.1.14.4.1.2";
static EPS_INT8 s_oidPrvStatus[]		= "1.3.6.1.4.1.1248.1.2.2.1.1.1.4";
static EPS_INT8 s_oidPrvCtrl[]			= "1.3.6.1.4.1.1248.1.2.2.44.1.1.2";

static EPS_INT8 g_TrnBuff[SNMP_MAX_BUF];


/*--------------------------  Public Functions Declaration   ---------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE snmpGetStatus	(EPS_SOCKET, const EPS_INT8*, EPS_STATUS_INFO*      );
static EPS_ERR_CODE snmpGetInkInfo	(const EPS_INT8*, EPS_STATUS_INFO*                  );
static EPS_ERR_CODE snmpGetPMString (const EPS_PRINTER_INN*, EPS_INT32, 
									 EPS_UINT8*, EPS_INT32*                             );
static EPS_ERR_CODE snmpMechCommand (const EPS_PRINTER_INN*, EPS_INT32                  );
#ifdef GCOMSW_CMD_ESCPAGE
static EPS_ERR_CODE snmpGetStatus_Page(EPS_SOCKET, const EPS_INT8*, EPS_STATUS_INFO*    );
static EPS_ERR_CODE snmpGetInkInfo_Page(const EPS_INT8*, EPS_STATUS_INFO*               );
static EPS_ERR_CODE snmpGetPMString_Page(const EPS_PRINTER_INN*, EPS_INT32, 
									 EPS_UINT8*, EPS_INT32*                             );
static EPS_ERR_CODE snmpMechCommand_Page(const EPS_PRINTER_INN*, EPS_INT32              );
#endif

/*--------------------------  Local Functions Declaration   ----------------------------*/
/*******************************************|********************************************/
static EPS_ERR_CODE SnmpTransact    (const EPS_INT8*, EPS_INT32, const EPS_INT8*, 
									 EPS_UINT8, ASN_VARIANT* pdu                        );
static EPS_ERR_CODE SnmpTransactS   (EPS_SOCKET, const EPS_INT8*, EPS_INT32, 
									 const EPS_INT8*, EPS_UINT8, ASN_VARIANT*           );

static EPS_ERR_CODE SnmpFindRecv    (EPS_SOCKET, EPS_UINT16, EPS_INT32, 
									 const EPS_INT8*, const EPS_INT8*, EPS_PRINTER_INN**);

static EPS_ERR_CODE mibGetPhysAddress(const EPS_INT8*, EPS_INT8*, EPS_UINT32            );
static EPS_ERR_CODE mibConfirmPrintPort(const EPS_INT8*, EPS_UINT16						);
#ifdef GCOMSW_CMD_ESCPAGE_S
static EPS_ERR_CODE mibGetMaxMediaXFeedDir(const EPS_INT8* address, EPS_UINT32* val		);
#endif
static EPS_ERR_CODE GetPDU          (EPS_INT8*, EPS_INT32, EPS_UINT8, const EPS_INT8*, 
									 ASN_VARIANT*, EPS_INT8*, EPS_INT32                 );

#ifdef GCOMSW_CMD_ESCPAGE
static EPS_INT32 GetColorID         (EPS_INT8*                                          );
#endif
static EPS_ERR_CODE SnmpWalkMib     (EPS_SOCKET, const EPS_INT8*, const EPS_INT8*, ASN_VARIANT*);
static EPS_UINT8    GetRequestId    (void                                               );
static EPS_ERR_CODE ParseLength     (EPS_INT8**, EPS_INT32*, EPS_UINT32*                );
static EPS_ERR_CODE ParseField      (EPS_INT8**, EPS_INT32*, ASN_VARIANT*               );

static EPS_ERR_CODE CreateCommand   (EPS_INT8*, EPS_UINT8, EPS_UINT8, const EPS_INT8*, EPS_INT32*);
static EPS_INT8*    MakeLength      (EPS_INT32, EPS_INT8*                               );
static EPS_INT8*    MakeIntField    (EPS_INT32, EPS_INT8*                               );
static EPS_INT8*    MakeStrField    (const EPS_INT8*, EPS_INT8*                         );
static EPS_ERR_CODE MakeOidField    (const EPS_INT8*, EPS_INT8**                        );
static EPS_ERR_CODE MakeSequens     (EPS_INT8*, EPS_UINT32*, EPS_BOOL                   );

static EPS_UINT16   IntToBer        (EPS_INT32, EPS_INT8*                               );
static EPS_INT32    BerToInt        (EPS_INT8*, EPS_INT32                               );
static EPS_ERR_CODE StrToOid        (const EPS_INT8*, EPS_INT8*, EPS_UINT16*            );
static EPS_ERR_CODE OidToStr        (EPS_INT8*, EPS_UINT32, EPS_INT8**                  );



/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------              Public Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   snmpSetupSTFunctions()                                              */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* N/A                                                                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE				- Success (Opened Communication)                    */
/*                                                                                      */
/* Description:                                                                         */
/*     Initialize protocol layer.                                                       */
/*                                                                                      */
/*******************************************|********************************************/
void	snmpSetupSTFunctions (
								
			EPS_SNMP_FUNCS*			fnc,
			const EPS_PRINTER_INN*	printer
		 
){
	EPS_LOG_FUNCIN

#ifdef GCOMSW_CMD_ESCPAGE
	if( EPS_LANG_ESCPR != printer->language ){
		fnc->GetStatus		= snmpGetStatus_Page;
		fnc->GetInkInfo		= snmpGetInkInfo_Page;
		fnc->GetPMString    = snmpGetPMString_Page;
		fnc->MechCommand    = snmpMechCommand_Page;
	} 
	else
#endif
	{
		fnc->GetStatus		= snmpGetStatus;
		fnc->GetInkInfo		= snmpGetInkInfo;
		fnc->GetPMString    = snmpGetPMString;
		fnc->MechCommand    = snmpMechCommand;
	}

	EPS_RETURN_VOID
}

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpOpenSocket()                                                  */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock	        EPS_SOCKET*         O: SNMP socket                                      */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_CODE	: result code                                                   */
/*                                                                                      */
/* Description:                                                                         */
/*      Create SNMP socket for any command.                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpOpenSocket(
							
		EPS_SOCKET* sock
		
){
	*sock = epsNetFnc.socket( EPS_PF_INET, EPS_SOCK_DGRAM, EPS_PROTOCOL_UDP );
	if( EPS_INVALID_SOCKET == *sock ){
		return EPS_ERR_COMM_ERROR;
	}

	return EPS_ERR_NONE;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpCloseSocket()                                                 */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock	        EPS_SOCKET*         I/O: SNMP socket                                    */
/*                                                                                      */
/* Return value:                                                                        */
/*      (none)                                                                          */
/*                                                                                      */
/* Description:                                                                         */
/*      Destory SNMP socket.                                                            */
/*                                                                                      */
/*******************************************|********************************************/
void snmpCloseSocket(
					 
		EPS_SOCKET* sock
		
){
	if(EPS_INVALID_SOCKET != *sock){
		epsNetFnc.close( *sock );
		*sock = EPS_INVALID_SOCKET;
	}
}



/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpFindStart()		                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          O: Discover send/recv Socket                        */
/* address      EPS_INT8*           I: Destination IP Address string (NULL terminate)   */
/* multi        EPS_BOOL            I: If TRUE, send multicast                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR								    	                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Open socekt & Send discover messasge.                                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpFindStart (

		EPS_SOCKET*		sock,
		const EPS_INT8*	address,
		EPS_BOOL        multi

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT8	nRqID = 0;
	EPS_INT32	nSize = 0;
	ASN_VARIANT pdu;

	EPS_INT32 i = 0;					/* Temporary counter for number of discoveries	*/

	EPS_LOG_FUNCIN

	/* Create UDP broadcast socket														*/
	if(EPS_INVALID_SOCKET == *sock){	/* First Time */
		*sock = epsNetFnc.socket( EPS_PF_INET, EPS_SOCK_DGRAM, EPS_PROTOCOL_UDP );
		if( EPS_INVALID_SOCKET == *sock ){
			EPS_RETURN( EPS_ERR_COMM_ERROR )
		}

		if(multi){
			if( EPS_SOCKET_SUCCESS != epsNetFnc.setBroadcast(*sock) ){
				/* If error occurr, close socket											*/
				epsNetFnc.close( *sock );
				*sock = EPS_INVALID_SOCKET;
				EPS_RETURN( EPS_ERR_COMM_ERROR )
			}
		}
	}

	/* Create  command															        */
	memset(&pdu, 0, sizeof(pdu));
	nRqID = GetRequestId();
	if( EPS_ERR_NONE != (ret = CreateCommand(g_TrnBuff, ASN_PDU_GET_NEXT, nRqID,
										s_oidPrvPrinter, &nSize)) ){
		EPS_RETURN( ret )
	}

	/* Send																				*/
	for (i = 0; i < EPSNET_NUM_DISCOVERIES; i++) {
		if ( 0 >= epsNetFnc.sendTo(*sock, (char*)g_TrnBuff, nSize,  
									   address, SNMP_PORT_NUM, 0)){
				/* If error occurr, close socket											*/
			epsNetFnc.close( *sock );
			*sock = EPS_INVALID_SOCKET;
			EPS_RETURN( EPS_ERR_COMM_ERROR )
		}
	}
	
	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpFind()	                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: Discover Socket			                        */
/* printer		EPS_PRINTER_INN**   O: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_COMM_ERROR   	        - socket error		                            */
/*      EPS_ERR_MEMORY_ALLOCATION							                            */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_PRINTER_NOT_USEFUL      - received but not usefl                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Receive discover messasge. Get printer name.                                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpFind(

		EPS_SOCKET			sock,
		EPS_UINT16			printPort,
		EPS_INT32			printProtocol,
		EPS_PRINTER_INN**   printer
		
){
	EPS_LOG_FUNCIN

	EPS_RETURN( SnmpFindRecv(sock, printPort, printProtocol, NULL, NULL, printer) )
}



/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpFindEnd() 				                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: Discover Socket			                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success                                           */
/*      EPS_ERR_COMM_ERROR   	    - Close socket failed  		                        */
/*                                                                                      */
/* Description:                                                                         */
/*      close discover process.						                                    */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpFindEnd(
						 
         EPS_SOCKET sock
		 
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8  *pRecvBuf = NULL;							/* Receive Buffer				*/

EPS_LOG_FUNCIN
	if(EPS_INVALID_SOCKET == sock){
		EPS_RETURN( EPS_ERR_NONE )
	}

	pRecvBuf = (EPS_INT8*)EPS_ALLOC( SNMP_MAX_BUF );
	if( NULL == pRecvBuf ){
		ret = EPS_ERR_MEMORY_ALLOCATION;
		goto snmpFindEnd_END;
	}

	/* Read All resopnse */
	while(0 < epsNetFnc.receive(sock, pRecvBuf, SNMP_MAX_BUF, EPSNET_FIND_RECV_TIMEOUT) );

snmpFindEnd_END:
	EPS_SAFE_RELEASE( pRecvBuf );

	epsNetFnc.shutdown(sock, EPS_SHUTDOWN_SEND);
	epsNetFnc.shutdown(sock, EPS_SHUTDOWN_RECV);
	epsNetFnc.shutdown(sock, EPS_SHUTDOWN_BOTH);

	/* Close UDP socket																	*/
	if (EPS_SOCKET_SUCCESS != epsNetFnc.close(sock) ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   snmpProbeByID()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printerUUID  EPS_INT8*    		I: printr ID String                                 */
/* timeout      EPS_UINT32          I: find timeout                                     */
/* printer	    EPS_PRINTER*		O: Pointer for Alloc Printer infomation structure   */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                - Success                                           */
/*      EPS_ERR_COMM_ERROR	        - socket failed      		                        */
/*      EPS_ERR_MEMORY_ALLOCATION   - Failed to allocate memory                         */
/*      EPS_ERR_PRINTER_NOT_FOUND   - printer not found                                 */
/*                                                                                      */
/* Description:                                                                         */
/*     Probe printer by ID String.				                                        */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE   snmpProbeByID (

		EPS_INT8*			printerUUID,
		EPS_UINT16			printPort,
		EPS_INT32			printProtocol,
		EPS_UINT32			timeout, 
		EPS_PRINTER_INN**	printer

){
/*** Declare Variable Local to Routine                                                  */
    EPS_ERR_CODE    ret = EPS_ERR_NONE;				/* Return status of internal calls  */
	EPS_SOCKET	    sock = EPS_INVALID_SOCKET;
	EPS_UINT32	    tmStart, tmNow, tmSpan, tmReq;
	EPS_INT8		compName[EPS_NAME_BUFFSIZE];
	EPS_INT8		compSysName[EPS_NAME_BUFFSIZE];
    EPS_INT8*		pPos = NULL;
    EPS_INT32		nSegCnt = 0;

EPS_LOG_FUNCIN

/*** Initialize Local & global Variables                                                */
	if(epsCmnFnc.getTime){
		tmStart = tmReq = epsCmnFnc.getTime();
	} else{
		timeout = tmStart = tmNow = tmReq = 0;
	}

/*** Parse definition String                                                            */
	pPos = strtok(printerUUID, EPS_NETID_SEP);
    for(nSegCnt = 0; pPos != NULL && nSegCnt < EPS_NETID_SEGNUM; nSegCnt++){
		switch(nSegCnt){
		case 0:			/* Get model name */
			strcpy(compName, pPos);
			break;
		case 1:			/* Get SysName */
			strcpy(compSysName, pPos);
			break;
		}

		pPos = strtok(NULL, EPS_NETID_SEP);
    }
	if(nSegCnt < EPS_NETID_SEGNUM){
		EPS_RETURN( EPS_ERR_INV_ARG_PRINTER_ID )
	}

/*** probe message broadcast                                                            */
	ret = snmpFindStart( &sock, EPSNET_UDP_BROADCAST_ADDR, TRUE );
	if(EPS_ERR_NONE != ret){
		goto snmpProbeByID_END;
	}

/*** Check response                                                                     */
	ret = EPS_ERR_PRINTER_NOT_FOUND;
	while( EPS_ERR_PRINTER_NOT_FOUND == ret ||
		   EPS_ERR_PRINTER_NOT_USEFUL== ret )
	{
		ret = SnmpFindRecv(sock, printPort, printProtocol, compSysName, compName, printer);

		/* epsCancelFindPriter() */
		if( epsCmnFnc.lockSync && epsCmnFnc.unlockSync ){
			if( 0 == epsCmnFnc.lockSync() ){
				if( g_FindBreak ){
					epsCmnFnc.unlockSync();
					break;
				}
				epsCmnFnc.unlockSync();
			}
		}

		/* Timeout */
		if(timeout > 0){
			tmNow = epsCmnFnc.getTime();
			tmSpan = (EPS_UINT32)(tmNow - tmStart);
			/*EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )*/
			if( tmSpan >= timeout ){
				break;
			}
		}

		/*EPS_DBGPRINT( ("TM %u - %u <> %u\n", tmNow, tmStart, tmSpan) )*/
		if( EPS_COM_NOT_RECEIVE == ret ){
			/* beef up */
			if( EPSNET_FIND_REREQUEST_TIME <= (EPS_UINT32)(tmNow - tmReq) ){
				/*EPS_DBGPRINT( ("beef up\n") )*/
				ret = snmpFindStart( &sock, EPSNET_UDP_BROADCAST_ADDR, TRUE );
				if(EPS_ERR_NONE != ret){
					goto snmpProbeByID_END;
				}
			}

			ret = EPS_ERR_PRINTER_NOT_FOUND;
		} else{
			tmReq = tmNow;
		}
	}

	if( EPS_ERR_PRINTER_NOT_USEFUL == ret || 
		EPS_COM_NOT_RECEIVE == ret ){
		ret = EPS_ERR_PRINTER_NOT_FOUND;
	}

/*** Return to Caller																	*/
snmpProbeByID_END:
	snmpFindEnd(sock);
 
	if( EPS_ERR_NONE != ret ){
		EPS_SAFE_RELEASE( *printer );
	}

	EPS_RETURN( ret );
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   snmpGetStatus()			                                            */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock	        EPS_SOCKET*         I: SNMP socket                                      */
/* address      EPS_INT8*           I: target printer IP Address                        */
/* pstInfo      EPS_STATUS_INFO*    O: Printer Status information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the printer status and analyze the status string.                           */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpGetStatus(
								 
		EPS_SOCKET sock, 
		const EPS_INT8* address, 
		EPS_STATUS_INFO* pstInfo
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;
	EPS_INT32 retry = 0;

EPS_LOG_FUNCIN

	memset(&pdu, 0, sizeof(pdu));

	for(retry = 0; retry < EPSNET_STAT_RETRY; retry++){
		ret = SnmpTransactS(sock, address, EPSNET_STAT_RECV_TIMEOUT, 
							s_oidPrvStatus, ASN_PDU_GET_NEXT, &pdu);
		if(EPS_ERR_NONE == ret ){
			break;
		}
		EPS_REREASE_VARIANT( pdu );
		EPS_DBGPRINT(("GetStatus retry %d\n", retry))
	}

	if( EPS_ERR_NONE == ret ){
		if(ASN_VT_OCTET_STRING == pdu.type){
			ret = serAnalyzeStatus(pdu.val.v_str, pstInfo);
		} else{
			ret = EPS_ERR_COMM_ERROR;
		}
	}

	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}

#ifdef GCOMSW_CMD_ESCPAGE
EPS_ERR_CODE snmpGetStatus_Page(
								 
		EPS_SOCKET sock, 
		const EPS_INT8* address, 
		EPS_STATUS_INFO* pstInfo
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;
	EPS_INT32   retry = 0;
	EPS_INT32   devstat, prnstat;

	EPS_LOG_FUNCIN

	pstInfo->nState = EPS_ST_IDLE;
	pstInfo->nError = EPS_PRNERR_NOERROR;
	pstInfo->nWarn  = EPS_PRNWARN_NONE;
	pstInfo->nCancel= EPS_CAREQ_NOCANCEL;
	pstInfo->nInkNo = 0;
	memset(pstInfo->nColorType, 0, sizeof(pstInfo->nColorType) );
	memset(pstInfo->nColor, 0, sizeof(pstInfo->nColor) );

	memset(&pdu, 0, sizeof(pdu));

	/* get Device Status */
	for(retry = 0; retry < EPSNET_STAT_RETRY; retry++){
		ret = SnmpTransactS(sock, address, EPSNET_STAT_RECV_TIMEOUT, 
							s_oidDevStatus, ASN_PDU_GET_NEXT, &pdu);
		if(EPS_ERR_NONE == ret ){
			break;
		}
		EPS_REREASE_VARIANT( pdu );
		EPS_DBGPRINT(("GetStatus retry %d\n", retry))
	}
	if( EPS_ERR_NONE == ret && ASN_VT_INTEGER == pdu.type){
		devstat = pdu.val.v_long;
	} else{
		ret = EPS_ERR_COMM_ERROR;
		goto snmpGetStatus_END;
	}
	EPS_REREASE_VARIANT( pdu );

	/* get Printer Status */
	for(retry = 0; retry < EPSNET_STAT_RETRY; retry++){
		ret = SnmpTransactS(sock, address, EPSNET_STAT_RECV_TIMEOUT,
							s_oidPrinterStatus, ASN_PDU_GET_NEXT, &pdu);
		if(EPS_ERR_NONE == ret ){
			break;
		}
		EPS_REREASE_VARIANT( pdu );
		EPS_DBGPRINT(("GetStatus retry %d\n", retry))
	}
	if( EPS_ERR_NONE == ret && ASN_VT_INTEGER == pdu.type){
		prnstat = pdu.val.v_long;
	} else{
		ret = EPS_ERR_COMM_ERROR;
		goto snmpGetStatus_END;
	}
	EPS_REREASE_VARIANT( pdu );

	/*EPS_DBGPRINT(("%d / %d\n", devstat, prnstat))*/
	switch(devstat){
	case MIB_DEVST_RUNNING:
	case MIB_DEVST_WARNING:
		switch(prnstat){
		case MIB_PRNST_IDLE:
		case MIB_PRNST_OTHER:	/* Standby */
			pstInfo->nState = EPS_ST_IDLE;
			pstInfo->nError = EPS_PRNERR_NOERROR;
			break;
		case MIB_PRNST_PRINTING:
			pstInfo->nState = EPS_ST_WAITING;
			pstInfo->nError = EPS_PRNERR_NOERROR;
			break;
		default:
		/*case MIB_PRNST_WARMUP:*/
			pstInfo->nState = EPS_ST_BUSY;
			pstInfo->nError = EPS_PRNERR_BUSY;
			break;
		}
		break;

	case MIB_DEVST_DOWN:
		switch(prnstat){
		case MIB_PRNST_WARMUP:
			pstInfo->nState = EPS_ST_BUSY;
			pstInfo->nError = EPS_PRNERR_BUSY;
			break;
		default:
		/*case MIB_PRNST_OTHER:*/
			pstInfo->nState = EPS_ST_ERROR;
			pstInfo->nError = EPS_PRNERR_GENERAL;
			break;
		}
		break;

	default:
		pstInfo->nState = EPS_ST_ERROR;
		pstInfo->nError = EPS_PRNERR_GENERAL;
		break;
	}

	if( EPS_ST_ERROR == pstInfo->nState ){
		/* get error reason */
		for(retry = 0; retry < EPSNET_STAT_RETRY; retry++){
			ret = SnmpTransactS(sock, address, EPSNET_STAT_RECV_TIMEOUT, 
								s_oidPrinterError, ASN_PDU_GET_NEXT, &pdu);
			if(EPS_ERR_NONE == ret ){
				break;
			}
			EPS_REREASE_VARIANT( pdu );
			EPS_DBGPRINT(("GetStatus retry %d\n", retry))
		}

		if( EPS_ERR_NONE == ret && ASN_VT_OCTET_STRING == pdu.type){
			EPS_DBGPRINT(("0x%02X\n", pdu.val.v_str[0]))
			if( pdu.val.v_str[0] & MIB_PRERR_NOPAPER ){
				pstInfo->nError = EPS_PRNERR_PAPEROUT;
			} else if( pdu.val.v_str[0] & MIB_PRERR_NOINK ){
				pstInfo->nError = EPS_PRNERR_INKOUT;
			} else if( pdu.val.v_str[0] & MIB_PRERR_COVEROPEN ){
				pstInfo->nError = EPS_PRNERR_COVEROPEN;
			} else if( pdu.val.v_str[0] & MIB_PRERR_PAPERJAM ){
				pstInfo->nError = EPS_PRNERR_PAPERJAM;
			} else{
				pstInfo->nError = EPS_PRNERR_GENERAL;
			}
		} else{
			ret = EPS_ERR_COMM_ERROR;
			goto snmpGetStatus_END;
		}
	}

snmpGetStatus_END:
	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}
#endif /* GCOMSW_CMD_ESCPAGE */

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   snmpGetInkInfo()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* address      EPS_INT8*           I: target printer IP Address                        */
/* pstInfo      EPS_STATUS_INFO*    O: Printer Status information                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Success                                       */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get the marker info.                                                            */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpGetInkInfo(
								 
		const EPS_INT8* address, 
		EPS_STATUS_INFO* pstInfo
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;

	EPS_LOG_FUNCIN

	memset(&pdu, 0, sizeof(pdu));

	ret = SnmpTransact(address, EPSNET_RECV_TIMEOUT, s_oidPrvStatus, ASN_PDU_GET_NEXT, &pdu);

	if( EPS_ERR_NONE == ret ){
		if(ASN_VT_OCTET_STRING == pdu.type){
			ret = serAnalyzeStatus(pdu.val.v_str, pstInfo);
		} else{
			ret = EPS_ERR_COMM_ERROR;
		}
	}

	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}

#ifdef GCOMSW_CMD_ESCPAGE
EPS_ERR_CODE snmpGetInkInfo_Page(
								 
		const EPS_INT8* address, 
		EPS_STATUS_INFO* pstInfo
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_SOCKET	soc;
	ASN_VARIANT pdu;
	EPS_INT32   i;

	EPS_LOG_FUNCIN

	pstInfo->nInkNo = 0;
	memset(&pdu, 0, sizeof(pdu));

	ret = snmpOpenSocket(&soc);
	if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		goto snmpGetInkInfo_END;
	}

	/* walk MarkerName record */
	i = 0;
	ret = SnmpWalkMib(soc, address, s_oidMarkerColorant, &pdu);
	while(EPS_ERR_NONE == ret){
		if(ASN_VT_OCTET_STRING == pdu.type){
			pdu.val.v_str[pdu.length] = '\0';
			pstInfo->nColorType[i++] = GetColorID(pdu.val.v_str);
		}

		/* next */
		EPS_REREASE_VARIANT( pdu );
		ret = SnmpWalkMib(soc, address, NULL, &pdu);
	}
	if(EPS_COM_NEXT_RECORD == ret){
		ret = EPS_ERR_NONE;
	} else if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		goto snmpGetInkInfo_END;
	}

	pstInfo->nInkNo = i;

	/* walk MarkerLevel record */
	i = 0;
	EPS_REREASE_VARIANT( pdu );
	ret = SnmpWalkMib(soc, address, s_oidMarkerLevel, &pdu);
	while(EPS_ERR_NONE == ret){
		if(ASN_VT_INTEGER == pdu.type){
			pstInfo->nColor[i++] = serInkLevelNromalize(pdu.val.v_long);
		}

		/* next */
		EPS_REREASE_VARIANT( pdu );
		ret = SnmpWalkMib(soc, address, NULL, &pdu);
	}
	if(EPS_COM_NEXT_RECORD == ret){
		ret = EPS_ERR_NONE;
	} else if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		goto snmpGetInkInfo_END;
	}

snmpGetInkInfo_END:
	snmpCloseSocket(&soc);
	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}
#endif /* GCOMSW_CMD_ESCPAGE */


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     snmpGetPMString()	        										*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printer      EPS_PRINTER_INN*    I: Pointer to a PrinterInfo                         */
/* pString		EPS_UINT8*			O: Pointer to PM String                             */
/* bufSize		EPS_INT32			I: pString buffer size                              */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success										*/
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends request to printer for supported media.  Parses response and stores		*/
/*		PM String : pString                                                             */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE snmpGetPMString(
								  
		const EPS_PRINTER_INN*	printer, 
 		EPS_INT32               type,
        EPS_UINT8*              pString,
		EPS_INT32*              bufSize

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;
	EPS_INT8	cOid[SNMP_MAX_OID];

	EPS_LOG_FUNCIN

	memset(&pdu, 0, sizeof(pdu));

	/* Create GetPM command                                                             */
	if(1 == type){
		sprintf(cOid, "%s.1.%d.%d.%d.%d.%d", s_oidPrvCtrl, 'p', 'm', 0x01, 0x00, 0x01);
	} else if(2 == type){
		sprintf(cOid, "%s.1.%d.%d.%d.%d.%d", s_oidPrvCtrl, 'p', 'm', 0x01, 0x00, 0x02);
	} else{
		EPS_RETURN( EPS_ERR_OPR_FAIL )
	}

	ret = SnmpTransact(printer->location, EPSNET_RECV_TIMEOUT,  cOid, ASN_PDU_GET, &pdu);

	if( EPS_ERR_NONE == ret ){
		if(ASN_VT_OCTET_STRING == pdu.type){
			*bufSize = Min((EPS_UINT32)*bufSize, pdu.length-1);
			memcpy(pString, pdu.val.v_str+1, *bufSize);
		} else{
			ret = EPS_ERR_COMM_ERROR;
		}
	}
	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}

#ifdef GCOMSW_CMD_ESCPAGE
EPS_ERR_CODE snmpGetPMString_Page(
								  
		const EPS_PRINTER_INN*	printer, 
 		EPS_INT32               type,
        EPS_UINT8*              pString,
		EPS_INT32*              bufSize

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;
	EPS_SOCKET	soc;
	EPS_UINT32 paperSource = EPS_MPID_AUTO;

	EPS_LOG_FUNCIN

	/* walk MarkerLevel record for enum paper tray */
	*bufSize = 0;
	ret = snmpOpenSocket(&soc);
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}

	ret = SnmpWalkMib(soc, printer->location, s_oidInputName, &pdu);
	while(EPS_ERR_NONE == ret){
		if(ASN_VT_OCTET_STRING == pdu.type){
			if( strncmp(pdu.val.v_str, "MP Tray", pdu.length) == 0 ){
				paperSource |= EPS_MPID_MPTRAY;
			} else if( strncmp(pdu.val.v_str, "LC1", pdu.length) == 0 ){
				paperSource |= EPS_MPID_FRONT1;
			} else if( strncmp(pdu.val.v_str, "LC2", pdu.length) == 0 ){
				paperSource |= EPS_MPID_FRONT2;
			} else if( strncmp(pdu.val.v_str, "LC3", pdu.length) == 0 ){
				paperSource |= EPS_MPID_FRONT3;
			} else if( strncmp(pdu.val.v_str, "LC4", pdu.length) == 0 ){
				paperSource |= EPS_MPID_FRONT4;
			}
		}

		/* next */
		EPS_REREASE_VARIANT( pdu );
		ret = SnmpWalkMib(soc, printer->location, NULL, &pdu);
	}

	if(EPS_COM_NEXT_RECORD == ret){
		ret = EPS_ERR_NONE;
	} else if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		*bufSize = 0;
	}
	snmpCloseSocket(&soc);

	if(EPS_ERR_NONE == ret){
		*bufSize = sizeof(EPS_UINT32);
		memcpy(pString, &paperSource, sizeof(EPS_UINT32));
	}
		
	EPS_RETURN( ret )
}
#endif	/*GCOMSW_CMD_ESCPAGE*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:   snmpMechCommand()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* Command      EPS_INT32           I: Command Code                                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE                    - Mech command executed successfully            */
/*      EPS_ERR_JOB_NOT_INITIALIZED     - JOB is NOT initialized						*/
/*      EPS_ERR_COMM_ERROR              - Mech command execution error                  */
/*                                                                                      */
/* Description:                                                                         */
/*      Sends mechanincal commands to the printer.                                      */
/*                                                                                      */
/*******************************************|********************************************/
EPS_ERR_CODE    snmpMechCommand (

        const EPS_PRINTER_INN*	printer, 
        EPS_INT32   Command
        
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT pdu;
	EPS_INT8	cOid[SNMP_MAX_OID];

EPS_LOG_FUNCIN
	memset(&pdu, 0, sizeof(pdu));

	/* Create control command */
	switch( Command ){
	case EPS_CBTCOM_PE:
		sprintf(cOid, "%s.1.%d.%d.%d.%d.%d", s_oidPrvCtrl, 'p', 'e', 0x01, 0x00, 0x01);
		break;
	case EPS_CBTCOM_RJ:
		sprintf(cOid, "%s.1.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", 
			s_oidPrvCtrl, 'r', 'j', 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         /* E   , S   , C   , P   , R   , L   , i   , b    */
                            0x45, 0x53, 0x43, 0x50, 0x52, 0x4c, 0x69, 0x62);
		break;
	default:
		EPS_RETURN( EPS_ERR_OPR_FAIL )
	}

	ret = SnmpTransact(printer->location, EPSNET_RECV_TIMEOUT, cOid, ASN_PDU_GET, &pdu);

	if( EPS_ERR_NONE == ret ){
		EPS_DBGPRINT(("%s\n", pdu.val.v_str+1))
		if( (ASN_VT_OCTET_STRING == pdu.type) &&
			(strstr(pdu.val.v_str+1,"OK") != NULL) ){
			ret = EPS_ERR_NONE;
		} else{
			ret = EPS_ERR_COMM_ERROR;
		}
	}

	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}

#ifdef GCOMSW_CMD_ESCPAGE
EPS_ERR_CODE    snmpMechCommand_Page (

        const EPS_PRINTER_INN*	printer, 
        EPS_INT32   Command
        
){
	EPS_LOG_FUNCIN
	(void)printer;
	(void)Command;
    EPS_RETURN( EPS_ERR_PROTOCOL_NOT_SUPPORTED )
}
#endif	/*GCOMSW_CMD_ESCPAGE*/


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*--------------------               Local Functions               ---------------------*/
/*%%%%%%%%%%%%%%%%%%%%                                             %%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SnmpTransact()    												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* address      EPS_INT8*           I: Destination IP Address string (NULL terminate)   */
/* recvtimeout  EPS_UINT32          I: receive response timeout                         */
/* oid          EPS_INT8*           I: Object ID                                        */
/* request		EPS_INT8**		    I: Request type                                     */
/* pdu          ASN_VARIANT*	    O: recieved value                                   */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success			                            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_COM_TINEOUT                 - Receive timeout                               */
/*                                                                                      */
/* Description:                                                                         */
/*      SNMP send request and receive responce transaction.								*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE SnmpTransact(
								 
		const EPS_INT8* address, 
		EPS_INT32  recvtimeout,
		const EPS_INT8* oid, 
		EPS_UINT8       request,
		ASN_VARIANT*    pdu
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_SOCKET   soc;

EPS_LOG_FUNCIN

	ret = snmpOpenSocket(&soc);
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( ret )
	}

	ret = SnmpTransactS(soc, address, recvtimeout, oid, request, pdu);

	snmpCloseSocket(&soc);

	EPS_RETURN( ret )
}


static EPS_ERR_CODE SnmpTransactS(
								 
		EPS_SOCKET sock, 
		const EPS_INT8* address, 
		EPS_INT32  recvtimeout,
		const EPS_INT8* oid, 
		EPS_UINT8       request,
		ASN_VARIANT* pdu
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT8	nRqID = 0;
	EPS_INT32	nSize = 0;
	EPS_INT32	nRecvSize = 0;

EPS_LOG_FUNCIN

	memset(pdu, 0, sizeof(ASN_VARIANT));

	/* Create GetStatus command															*/
	nRqID = GetRequestId();
	if( EPS_ERR_NONE != (ret = CreateCommand(g_TrnBuff, request, nRqID,
										oid, &nSize)) ){
		EPS_RETURN( ret )
	}

	/* Send																				*/
	if( epsNetFnc.sendTo(sock, (char*)g_TrnBuff, nSize, 
						address, SNMP_PORT_NUM, EPSNET_SEND_TIMEOUT) <= 0 ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}

	ret = EPS_COM_READ_MORE;
	while(ret == EPS_COM_READ_MORE){
		/* Wireless network always send pair message and receive. 
		   This behave make probrem that receive befor message. If it occur, try receive once.*/
		nRecvSize = epsNetFnc.receive( sock, g_TrnBuff, SNMP_MAX_BUF, recvtimeout ); 
		if( 0 >= nRecvSize ){							/* Error Occur or Not Recieve   */
			EPS_RETURN( EPS_ERR_COMM_ERROR )
		}
		ret = GetPDU(g_TrnBuff, nRecvSize, nRqID, oid, pdu, NULL, 0);
	}

	if( EPS_ERR_NONE != ret ){
		EPS_REREASE_VARIANT( *pdu );
		if(EPS_COM_NEXT_RECORD == ret){
			ret = EPS_ERR_COMM_ERROR;
		}
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SnmpWalkMib()  												    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: Socket                                           */
/* address      EPS_INT8*           I: Destination IP Address string (NULL terminate)   */
/* oid          EPS_INT8*           I: Object ID. set first time only.                  */
/* pdu          ASN_VARIANT*	    O: recieved value                                   */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success			                            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_COM_TINEOUT                 - Receive timeout                               */
/*                                                                                      */
/* Description:                                                                         */
/*      SNMP walk mib record transaction. 								                */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE SnmpWalkMib(
								 
		EPS_SOCKET sock, 
		const EPS_INT8* address, 
		const EPS_INT8* oid, 
		ASN_VARIANT*	pdu
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT8	nRqID = 0;
	EPS_INT32	nSize = 0;
	EPS_INT32	nRecvSize = 0;
	static EPS_INT8    orgObjID[SNMP_OBJID_LEN];
	static EPS_INT8    nextObjID[SNMP_OBJID_LEN];

EPS_LOG_FUNCIN

	memset(pdu, 0, sizeof(ASN_VARIANT));

	/* Create command */
	nRqID = GetRequestId();
	if( oid ){
		strcpy(orgObjID, oid);
		strcpy(nextObjID, oid);
	}
	ret = CreateCommand(g_TrnBuff, ASN_PDU_GET_NEXT, nRqID, nextObjID, &nSize);
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

	/* Send */
	if( epsNetFnc.sendTo(sock, (char*)g_TrnBuff, nSize, 
						address, SNMP_PORT_NUM, EPSNET_SEND_TIMEOUT) <= 0 ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}

	ret = EPS_COM_READ_MORE;
	while(ret == EPS_COM_READ_MORE){
		/* Wireless network always send pair message and receive. 
		   This behave make probrem that receive befor message. If it occur, try receive once.*/
		nRecvSize = epsNetFnc.receive( sock, g_TrnBuff, SNMP_MAX_BUF, EPSNET_RECV_TIMEOUT ); 
		if( 0 >= nRecvSize ){							/* Error Occur or Not Recieve   */
			EPS_RETURN( EPS_ERR_COMM_ERROR )
		}
		ret = GetPDU(g_TrnBuff, nRecvSize, nRqID, orgObjID, pdu, nextObjID, SNMP_OBJID_LEN);
	}

	if( EPS_ERR_NONE != ret ){
		EPS_REREASE_VARIANT( *pdu );
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     mibGetPhysAddress() 												*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* address      EPS_INT8*           I: Destination IP Address string (NULL terminate)   */
/* val          EPS_INT8*           O: pointer to return value                          */
/* vallen		EPS_UINT32		    I: lenght of val buffer                             */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success			                            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_COM_TINEOUT                 - Receive timeout                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Get MacAddress.																	*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE mibGetPhysAddress(
								 
		const EPS_INT8* address, 
		EPS_INT8*		val, 
		EPS_UINT32		vallen 
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT  pdu;
	EPS_UINT8    *p;
	EPS_UINT32	 i;

	EPS_LOG_FUNCIN

	memset(&pdu, 0, sizeof(pdu));
	ret = SnmpTransact(address, EPSNET_RECV_TIMEOUT, s_oidPhysAddress, ASN_PDU_GET_NEXT, &pdu);

	if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		goto mibGetPhysAddress_END;
	}
	if(ASN_VT_OCTET_STRING != pdu.type){
		ret = EPS_ERR_COMM_ERROR;
		goto mibGetPhysAddress_END;
	}

	/* convert to string */
	p = (EPS_UINT8*)val;
	vallen -= 2;
	for(i = 0; i < pdu.length && i < vallen; i++ ){
		*p = ((EPS_UINT8)pdu.val.v_str[i] >> 4);
		if(*p < 10){
			*p += 0x30;
		} else{
			*p += 0x37;
		}
		p++;

		*p = ((EPS_UINT8)pdu.val.v_str[i] & 0x0F);
		if(*p < 10){
			*p += 0x30;
		} else{
			*p += 0x37;
		}
		p++;
	}
	*p = '\0';

mibGetPhysAddress_END:
	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     mibConfirmPrintPort() 											*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* printPort	EPS_UINT16		    I: print port                                       */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success			                            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_COM_TINEOUT                 - Receive timeout                               */
/*      EPS_COM_NEXT_RECORD             - got next recode                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Confirm whether can use Print-port.												*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE mibConfirmPrintPort(
						
		const EPS_INT8* address, 
		EPS_UINT16		printPort 
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_SOCKET	 sckTmp;
	ASN_VARIANT  pdu;

	EPS_LOG_FUNCIN
		
	ret = snmpOpenSocket(&sckTmp);
	if( EPS_ERR_NONE != ret ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}

	/* walk Tcp port record */
	ret = SnmpWalkMib(sckTmp, address, s_oidTcpConnLocalPort, &pdu);
	while(EPS_ERR_NONE == ret){
		if(ASN_VT_INTEGER == pdu.type){
			EPS_DBGPRINT(("pdu.val.v_long = %d, %d\n", printPort, pdu.val.v_long));
			if( printPort == pdu.val.v_long) {
				break;
			}
		}
		/* next */
		EPS_REREASE_VARIANT( pdu );
		ret = SnmpWalkMib(sckTmp, address, NULL, &pdu);
	}

	EPS_REREASE_VARIANT( pdu );
	snmpCloseSocket(&sckTmp);

	EPS_RETURN( ret )
}


#ifdef GCOMSW_CMD_ESCPAGE_S
/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     mibGetMaxMediaXFeedDir() 											*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* address      EPS_INT8*           I: Destination IP Address string (NULL terminate)   */
/* val          EPS_INT8*           O: pointer to return value                          */
/* vallen		EPS_UINT32		    I: lenght of val buffer                             */
/*                                                                                      */
/* Return value:																		*/
/*      EPS_ERR_NONE					- Success			                            */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_COM_TINEOUT                 - Receive timeout                               */
/*                                                                                      */
/* Description:                                                                         */
/*      Get MacAddress.																	*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE mibGetMaxMediaXFeedDir(
								 
		const EPS_INT8* address, 
		EPS_UINT32*		val 
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	ASN_VARIANT  pdu;
	/* iso(1) org(3) dod(6) internet(1) mgmt(2) mib-2(1) printmib(43) prtMediaPath(13)
      prtMediaPathTable(4) prtMediaPathEntry(1) prtMediaPathMaxMediaXFeedDir(6) */
	const EPS_INT8 oidMaxMediaXFeedDir[]	= "1.3.6.1.2.1.43.13.4.1.6";

	EPS_LOG_FUNCIN

	*val = 0;

	memset(&pdu, 0, sizeof(pdu));
	ret = SnmpTransact(address, EPSNET_RECV_TIMEOUT, oidMaxMediaXFeedDir, ASN_PDU_GET_NEXT, &pdu);

	if( EPS_ERR_NONE != ret ){
		ret = EPS_ERR_COMM_ERROR;
		goto mibGetMaxMediaXFeedDir_END;
	}
	if(ASN_VT_INTEGER != pdu.type){
		ret = EPS_ERR_COMM_ERROR;
		goto mibGetMaxMediaXFeedDir_END;
	}

	*val = pdu.val.v_long;

mibGetMaxMediaXFeedDir_END:
	EPS_REREASE_VARIANT( pdu );

	EPS_RETURN( ret )
}
#endif


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     SnmpFindRecv()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* sock		    EPS_SOCKET          I: socket used in discovery phase	                */
/* printPort	EPS_UINT16          I: print data sending port                          */
/* printProtocol EPS_INT32          I: print protocol                                   */
/* compSysName		EPS_INT8*       I: unique ID of target printer                      */
/* compName		EPS_INT8*           I: model name of target printer                     */
/* printer		EPS_PRINTER_INN**   O: pointer for found printer structure              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*      EPS_ERR_PRINTER_NOT_FOUND       - printer not found                             */
/*      EPS_ERR_PRINTER_NOT_USEFUL      - received but not usefl                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Read discoverySocket and store information of valid devices that responded to	*/
/*		discovery message.																*/
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE SnmpFindRecv(

		EPS_SOCKET			sock,
		EPS_UINT16			printPort,
		EPS_INT32			printProtocol,
		const EPS_INT8*	    compSysName,
		const EPS_INT8*     compName,
		EPS_PRINTER_INN**   printer

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8    *pRecvBuf = NULL;						/* Receive Buffer				*/
	EPS_INT32   nRecvSize = 0;							/* Received Length				*/
	ASN_VARIANT pdu;
	EPS_INT8    sFromAdder[EPS_ADDR_BUFFSIZE];			/* Remote Address				*/
	EPS_UINT16  nFromPort = 0;							/* Remote Port					*/
	EPS_INT32	cmdLevel = 0;
	EPS_INT8    idString[EPS_PRNID_BUFFSIZE];
	EPS_INT8    devidString[SNMP_MAX_BUF];

	EPS_LOG_FUNCIN

	memset(&pdu, 0, sizeof(pdu));

	pRecvBuf = (EPS_INT8*)EPS_ALLOC( SNMP_MAX_BUF );
	if( NULL == pRecvBuf ){
		EPS_RETURN( EPS_ERR_MEMORY_ALLOCATION );
	}
	memset(sFromAdder, 0, sizeof(sFromAdder));

	/* -------------------------------------------------------------------------------- */
	/* Recieve discover response														*/
	nRecvSize = epsNetFnc.receiveFrom( sock, pRecvBuf, SNMP_MAX_BUF, 
								sFromAdder, &nFromPort,
								EPSNET_FIND_RECV_TIMEOUT); 
	if( 0 >= nRecvSize ){			/* Error Occur or No response*/
		ret = EPS_COM_NOT_RECEIVE;
		goto SnmpFindRecv_END;
	}
	/* Existing check */
	EPS_DBGPRINT(("%s\n", sFromAdder))
	if( prtIsRegistered(sFromAdder, NULL, printProtocol) ){
		ret = EPS_ERR_PRINTER_NOT_FOUND;
		goto SnmpFindRecv_END;
	}

	/* parse value */
	ret = GetPDU(pRecvBuf, nRecvSize, 0, s_oidPrvPrinter, &pdu, NULL, 0);
	if( EPS_COM_ERR_REPLY   == ret ||
		EPS_COM_READ_MORE   == ret ||
		EPS_COM_NEXT_RECORD == ret ||
		EPS_ERR_COMM_ERROR  == ret ){
		ret = EPS_ERR_PRINTER_NOT_USEFUL;
	}
	if( EPS_ERR_NONE != ret ){
		goto SnmpFindRecv_END;
	}
	if (pdu.type != ASN_VT_OCTET_STRING) {
		ret = EPS_ERR_PRINTER_NOT_USEFUL;
		goto SnmpFindRecv_END;
	}
	memset(devidString, 0, sizeof(devidString));
	strncpy(devidString, pdu.val.v_str, Min(pdu.length, SNMP_MAX_BUF-1));
	devidString[Min(pdu.length, SNMP_MAX_BUF-1)] = 0;

	/* Create printer data															    */
	*printer = (EPS_PRINTER_INN*)EPS_ALLOC( sizeof(EPS_PRINTER_INN) );
	if(NULL == *printer){
		ret = EPS_ERR_MEMORY_ALLOCATION;
		goto SnmpFindRecv_END;
	}
	memset( *printer, 0, sizeof(EPS_PRINTER_INN) );

	if( !serParseDeviceID(devidString, (*printer)->manufacturerName, (*printer)->modelName, 
										&cmdLevel, &(*printer)->language) ){
		ret = EPS_ERR_PRINTER_NOT_USEFUL;				/* Not ESC/P-R,ESC/Page Printer */
		goto SnmpFindRecv_END;
	}
	if( EPS_PROTOCOL_LPR != printProtocol ){
		if( EPS_LANG_ESCPR != (*printer)->language ){
			ret = EPS_ERR_PRINTER_NOT_USEFUL;	/* Page Printer Not support via Raw */
			goto SnmpFindRecv_END;
		}
	}

	switch(cmdLevel){
	case 0: /* Support all. For Uin communication  */
	case 2:
		(*printer)->supportFunc |= EPS_SPF_JPGPRINT; /* Jpeg print */
	case 1:
		(*printer)->supportFunc |= EPS_SPF_RGBPRINT; /* RGB print */
	}

	(*printer)->protocol = printProtocol;
	(*printer)->printPort = printPort;
	strcpy( (*printer)->location, sFromAdder );

	/* confirm whether can use Print-port */
	if( 0 != printPort && EPS_LANG_ESCPR == (*printer)->language){
		EPS_DBGPRINT(("mibConfirmPrintPort\n"))
		ret = mibConfirmPrintPort(sFromAdder, printPort);
		if( EPS_ERR_NONE != ret ){
			ret = EPS_ERR_PRINTER_NOT_USEFUL;
			goto SnmpFindRecv_END;
		}
	}

	/* Get mac address */
	ret = mibGetPhysAddress(sFromAdder, (*printer)->macAddress, sizeof((*printer)->macAddress));
	if(EPS_ERR_NONE != ret){
		ret = EPS_ERR_PRINTER_NOT_USEFUL;
		goto SnmpFindRecv_END;
	}
	
	if( NULL != compSysName && NULL != compName){
		/* compaire name */
		if( strcmp(compSysName, (*printer)->macAddress) ||
			strcmp(compName, (*printer)->modelName) ){

			ret = EPS_ERR_PRINTER_NOT_USEFUL;
			goto SnmpFindRecv_END;
		}
	}

	/* Get friendly name */
	EPS_REREASE_VARIANT( pdu );
	ret = SnmpTransact(sFromAdder, EPSNET_RECV_TIMEOUT, s_oidPrvBonjourName, ASN_PDU_GET_NEXT, &pdu);
	if( EPS_ERR_NONE == ret && ASN_VT_OCTET_STRING == pdu.type){
		if(pdu.length < EPS_NAME_BUFFSIZE-1){
			strncpy( (*printer)->friendlyName, pdu.val.v_str, pdu.length);
		} else{
			strncpy( (*printer)->friendlyName, pdu.val.v_str, EPS_NAME_BUFFSIZE-2);
		}
	} else{
		ret = EPS_ERR_NONE; /* ignoure */
	}

	sprintf(idString, EPS_NET_IDPRM_STR, (*printer)->modelName, (*printer)->macAddress);
	prtSetIdStr(*printer, idString);

#ifdef GCOMSW_CMD_ESCPAGE_S
	if(EPS_LANG_ESCPAGE_S == (*printer)->language){
		EPS_UINT32 feed = 0;
		if( EPS_ERR_NONE == mibGetMaxMediaXFeedDir(sFromAdder, &feed) ){
			(*printer)->feedDir = pageS_ParseFeedDir(feed);
		}
	}
#endif

/*** Return to Caller                                                                   */
SnmpFindRecv_END:
	EPS_REREASE_VARIANT( pdu );
	EPS_SAFE_RELEASE( pRecvBuf );
	if(EPS_ERR_NONE != ret){
		EPS_SAFE_RELEASE( *printer );
	}

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     CreateCommand()                                                   */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pBuf	        EPS_INT8*           O: SNMP request buffer                              */
/* nPDUType     EPS_UINT8           I: PDU type                                         */
/* nRqID        EPS_UINT8           I: SNMP request ID                                  */
/* psIdentifire EPS_INT8*           I: MIB number string                                */
/* pDataSize    EPS_INT32*          O: SNMP request buffer size                         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Create SNMP request command.                                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE  CreateCommand(
								   
		EPS_INT8*   pBuff, 
		EPS_UINT8   nPDUType, 
		EPS_UINT8   nRqID, 
		const EPS_INT8*   psIdentifire, 
		EPS_INT32*  pDataSize
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8*	pPos = pBuff;
	EPS_INT8*	pPDUTop = NULL;
	EPS_INT8*	pObjTop = NULL;
	EPS_UINT32	nLen = 0;

	/* version */
	pPos = MakeIntField(SNMP_VERSION, pPos);
	/* Community */
	pPos = MakeStrField(SNMP_COMMUNITY_STR, pPos);
	/* PDU Type */
	*(pPos++) = nPDUType;
	pPDUTop = pPos;

	/* Request ID */
	pPos = MakeIntField(nRqID, pPos);
	/* Error Index */
	pPos = MakeIntField(0, pPos);
	/* Error Status */
	pPos = MakeIntField(0, pPos);
	pObjTop = pPos;

	/* OID */
	ret = MakeOidField(psIdentifire, &pPos);
	/* Value(NULL) */
	*(pPos++) = ASN_VT_NULL;
	*(pPos++) = (EPS_INT8)(0);

	/* Set Object Seupence */
	nLen = (EPS_UINT32)(pPos - pObjTop);
	ret = MakeSequens(pObjTop, &nLen, TRUE);
	ret = MakeSequens(pObjTop, &nLen, TRUE);

	/* Set Object Seupence */
	nLen = (EPS_UINT32)(pObjTop - pPDUTop) + nLen;
	ret = MakeSequens(pPDUTop, &nLen, FALSE);

	/* Set All Seupence */
	nLen = (EPS_UINT32)(pPDUTop - pBuff) + nLen;
	ret = MakeSequens(pBuff, &nLen, TRUE);

	*pDataSize = nLen;

	return 	ret;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     GetPDU()                                                          */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pBuf	        EPS_INT8*           I: SNMP reply buffer                                */
/* nBufSize     EPS_INT32           I: SNMP reply buffer size                           */
/* nRqID        EPS_UINT8           I: SNMP request ID                                  */
/* pObjID       EPS_INT8*           I: requested OID                                    */
/* pPDU         ASN_VARIANT*        O: PDU field structure                              */
/* pResObjID    EPS_INT8*           O: response OID                                     */
/* nResObjIDSize EPS_INT8*          I: size of pResObjID buffer                         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Get PDU from SNMP reply.                                                        */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE GetPDU(
						   
		EPS_INT8*   pBuf, 
		EPS_INT32   nBufSize,
		EPS_UINT8   nRqID, 
		const EPS_INT8*   pObjID, 
		ASN_VARIANT *pPDU,
		EPS_INT8*   pResObjID, 
		EPS_INT32   nResObjIDSize

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT32	nAllSize = 0;
	EPS_UINT32	nSeqSize = 0;
	ASN_VARIANT	vField;
	EPS_INT32   nDataSize = nBufSize;
	const EPS_INT8 *pOrg, *pRes;

#define SNMP_CHECK_INTVALUE( r, v, n )								\
	if(EPS_ERR_NONE != r ){											\
		EPS_RETURN( r )												\
	} else 	if( ASN_VT_INTEGER != v.type || n != v.val.v_long ){	\
		EPS_DBGPRINT(("type=%d / value=%d\n", v.type, v.val.v_long)) \
		EPS_REREASE_VARIANT( v );									\
		EPS_RETURN( EPS_ERR_COMM_ERROR )							\
	}

EPS_LOG_FUNCIN

	vField.length = 0;
	vField.type = ASN_VT_NULL;
	vField.val.v_str = NULL;

	/* skip top tag */
	pBuf++;	
	nDataSize--;
	if( EPS_ERR_NONE != (ret = ParseLength( &pBuf, &nDataSize, &nAllSize ) ) ){
		EPS_RETURN( ret )
	}

	/* check version */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	SNMP_CHECK_INTVALUE( ret, vField, SNMP_VERSION );

	/* check community */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	if( ASN_VT_OCTET_STRING != vField.type 
		|| NULL == memStrStrWithLen(vField.val.v_str, vField.length, SNMP_COMMUNITY_STR) ){
		EPS_REREASE_VARIANT( vField );
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}
	EPS_REREASE_VARIANT( vField );
	
	/* check PDU Type */
	if( (EPS_UINT8)(*pBuf) != ASN_PDU_RESP ){
		EPS_RETURN( EPS_ERR_COMM_ERROR )
	}
	pBuf += 1;
	nDataSize--;
	/* skip sequence size */
	if( EPS_ERR_NONE != (ret = ParseLength( &pBuf, &nDataSize, &nSeqSize ) ) ){
		EPS_RETURN( ret )
	}

	/* check Request ID */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	if(nRqID != vField.val.v_long)
	{				
		if(EPS_ERR_NONE != ret )
		{											
			EPS_RETURN( ret )
		} 
		else if( ASN_VT_INTEGER != vField.type || 
			(nRqID != 0 && nRqID > vField.val.v_long ) )
		{	
			/* The data in front of one was received.->retry */
			EPS_REREASE_VARIANT( vField );								
			EPS_RETURN( EPS_COM_READ_MORE )
		}
	}

	/* check Error */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	SNMP_CHECK_INTVALUE( ret, vField, SNMP_ERR_NONE );

	/* skip Error Index */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	if(EPS_ERR_NONE != ret ){
		EPS_RETURN( ret )	
	}
	/* skip Sequence Head */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	if(EPS_ERR_NONE != ret ){
		EPS_RETURN( ret )	
	}
	ret = ParseField( &pBuf, &nDataSize, &vField );
	if(EPS_ERR_NONE != ret ){
		EPS_RETURN( ret )	
	}

	/* Get MIB ID */
	ret = ParseField( &pBuf, &nDataSize, &vField );
	/*EPS_DBGPRINT(("\nORG : %s\nRES : %s\n", pObjID, vField.val.v_str))*/

	/* Check response MIB ID */
	pOrg = pObjID;
	pRes = vField.val.v_str;
    while(*pOrg != '\0' && *pRes != '\0'){
		if( *pOrg++ < *pRes++ ){
			/*if( strchr(pOrg, '.') != NULL){*/
				ret = EPS_COM_NEXT_RECORD;
			/*}  else{ next item } */
			break;
		}
    }
	if( pResObjID ){
		memset(pResObjID, 0, nResObjIDSize);
		memcpy(pResObjID, vField.val.v_str, Min(nResObjIDSize-1, (EPS_INT32)strlen(vField.val.v_str)));
	}
	
	EPS_REREASE_VARIANT( vField );
	if( EPS_ERR_NONE != ret){
		EPS_RETURN( ret )
	}

	/* get valiable */
	ret = ParseField( &pBuf, &nDataSize, &vField );

	*pPDU = vField;

	EPS_RETURN( ret )
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     ParseField()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc	        EPS_INT8**          I/O: PDU buffer                                     */
/* pDataSize    EPS_INT32*          I/O: PDU buffer size                                */
/* pVal         ASN_VARIANT*        O: PDU field structure                              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Parse PDU field.  And move next area.                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE ParseField(
							   
		EPS_INT8**      pSrc, 
		EPS_INT32*		pDataSize,
		ASN_VARIANT*    pVal
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_UINT32 nInt = 0;

	/* Field type */
	pVal->type = *((*pSrc)++);
	(*pDataSize)--;
	if(*pDataSize <= 0){		/* data size not enough */
		return EPS_ERR_COMM_ERROR;
	}

	/* Field length */
	if( EPS_ERR_NONE != (ret = ParseLength( pSrc, pDataSize, &nInt ) ) ){
		pVal->length = 0;
		return ret;
	}
	pVal->length = nInt;
	if((EPS_UINT32)*pDataSize < pVal->length){		/* data size not enough */
		return EPS_ERR_COMM_ERROR;
	}

	/* Field value */
	switch( pVal->type ){
	case ASN_VT_INTEGER:
		pVal->val.v_long = BerToInt(*pSrc, pVal->length);
		break;

	case ASN_VT_NULL:
		pVal->val.v_long = 0;
		break;

	case ASN_VT_OCTET_STRING:
		pVal->val.v_str = *pSrc;
		break;

	case ASN_VT_OBJECT_ID:
		{
			EPS_INT8* p = NULL;
			ret = OidToStr( *pSrc, pVal->length, &p);
			if( EPS_ERR_NONE != ret ){
				return ret;
			}
			pVal->val.v_str = p;
		}
		break;

	case ASN_VT_SEQUENCE:
		pVal->val.v_str = *pSrc;
		return ret;	/* Not move next */
		break;

	}

	*pSrc += pVal->length;	/* Move next field */
	*pDataSize -= pVal->length;

	return ret;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakeIntField()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc	        EPS_INT32           I: int value                                        */
/* pDst         EPS_INT8*           I: PDU buffer                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT8*	: pointer for next area                                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Make BER integer field.  And move next area.                                    */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT8* MakeIntField(
							  
		EPS_INT32 nSrc, 
		EPS_INT8* pDst
		
){
	EPS_INT16	nLen = 0;

	/* Field type */
	*(pDst++) = ASN_VT_INTEGER;

	/* Field value */
	nLen = IntToBer(nSrc, pDst);
	memmove(pDst+nLen, pDst , nLen);

	/* Field length */
	pDst = MakeLength( nLen, pDst );
	pDst += nLen;

	return pDst;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakeStrField()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrc	        EPS_INT8*           I: string                                           */
/* pDst         EPS_INT8*           I: PDU buffer                                       */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT8*	: pointer for next area                                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Make BER string field.  And move next area.                                     */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT8* MakeStrField(
							  
		const EPS_INT8* pSrc, 
		EPS_INT8*       pDst
		
){
	EPS_UINT32	nLen = (EPS_UINT32)strlen(pSrc);

	/* Field type */
	*(pDst++) = ASN_VT_OCTET_STRING;

	/* Field length */
	pDst = MakeLength( nLen, pDst );

	/* Field value */
	memcpy(pDst, pSrc, nLen);

	pDst += nLen;

	return pDst;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakeOidField()                                                    */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrc	        EPS_INT8*           I: ObjectID string                                  */
/* pDst         EPS_INT8**          I/O: PDU buffer(point to length field)              */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Make BER ObjectID field.  And move next area.                                   */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE MakeOidField(
								 
		const EPS_INT8* pSrc, 
		EPS_INT8**      pDst
		
){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8	cOid[SNMP_MAX_OID];
	EPS_UINT16	nLen = 0;

	/* Field type */
	*(*pDst)++ = ASN_VT_OBJECT_ID;

	/* Create OID */
	nLen = sizeof(cOid);
	if( EPS_ERR_NONE != (ret = StrToOid(pSrc, cOid, &nLen)) ){
		return ret;
	}

	/* Field length */
	*pDst = MakeLength( nLen, *pDst );

	/* Field value */
	memcpy(*pDst, cOid, nLen);

	*pDst += nLen;

	return ret;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakeSequens()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrc	        EPS_INT8*           I: PDU buffer(point to length field)                */
/* pSize        EPS_UINT32*         O: pointer for length                               */
/* bNeedType    EPS_BOOL            I: pointer for length                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Make BER Sequens. And move next area.                                           */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE MakeSequens(
								
		EPS_INT8*   pSrc, 
		EPS_UINT32* pSize, 
		EPS_BOOL    bNeedType

){
	EPS_ERR_CODE ret = EPS_ERR_NONE;
	EPS_INT8	nLengthLen = 0;

	/* Length field length calculation */
	if(*pSize <= 0x7F){
		nLengthLen = 1;
	} else{
		if( EPS_ENDIAN_LITTLE == cpuEndian ){
			nLengthLen = (sizeof(EPS_INT32) / sizeof(EPS_INT8)) - 1;
			for(; nLengthLen > -1 && 0x00 == *((EPS_UINT8*)pSize + nLengthLen); nLengthLen--);
		} else{ 
			EPS_INT8 nLim = sizeof(EPS_INT32) / sizeof(EPS_INT8);
			for(; nLengthLen < nLim && 0x00 == *((EPS_INT8*)pSize + nLengthLen); nLengthLen++);
		}
	}

	if( bNeedType ){
		memmove(pSrc + nLengthLen + 1, pSrc, *pSize);	/* 1 = Field type */

		/* Field type */
		*(pSrc++) = ASN_VT_SEQUENCE;

		/* Field length */
		pSrc = MakeLength( *pSize, pSrc );

		*pSize += nLengthLen + 1;
	} else{
		memmove(pSrc + nLengthLen, pSrc, *pSize);

		/* Field length */
		pSrc = MakeLength( *pSize, pSrc );

		*pSize += nLengthLen;
	}

	return ret;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     ParseLength()                                                     */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* pSrc	        EPS_INT8**          I/O: PDU buffer(point to length field)              */
/* pLength      EPS_UINT32*         O: pointer for length                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*      EPS_ERR_COMM_ERROR              - Communication Error                           */
/*                                                                                      */
/* Description:                                                                         */
/*      Parse Length field.  And move next area.                                        */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE ParseLength(
								
		EPS_INT8**  pSrc, 
		EPS_INT32*  pDataSize,
		EPS_UINT32* pLength
		
){
	*pLength = 0;

	if( (**pSrc) & 0x80 ){
		EPS_INT8 n = 0;
		EPS_INT8 nLenLength = (**pSrc) ^ 0x80;
		if(sizeof(EPS_UINT32) < (EPS_UINT8)nLenLength){
			/* Over flow */
			return EPS_ERR_MEMORY_ALLOCATION;
		}

		(*pSrc)++;	(*pDataSize)--;

		if( EPS_ENDIAN_LITTLE == cpuEndian ){
			for(n = nLenLength-1; n >= 0; n--){
				*((EPS_INT8*)&(*pLength) + n) = *(*pSrc)++;	
				(*pDataSize)--;
			}
		} else{
			for(n = sizeof(EPS_UINT32) - nLenLength; (EPS_UINT8)n < sizeof(EPS_UINT32); n++){
				*((EPS_INT8*)&(*pLength) + n) = *(*pSrc)++;
				(*pDataSize)--;
				if(*pDataSize <= 0){	/* data size not enough */
					return EPS_ERR_COMM_ERROR;
				}
			}
		}
	} else{
		*pLength = *(*pSrc)++;
		(*pDataSize)--;
	}

	if((EPS_UINT32)*pDataSize < *pLength){	/* data size not enough */
		return EPS_ERR_COMM_ERROR;
	}

	return EPS_ERR_NONE;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     MakeLength()                                                      */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nLength	    EPS_INT32           I: length value                                     */
/* pDst         EPS_INT8*           O: length field                                     */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT8*	: pointer for next area                                             */
/*                                                                                      */
/* Description:                                                                         */
/*      Make BER length field.  And move next area.                                     */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT8* MakeLength(
							
		EPS_INT32 nLength, 
		EPS_INT8* pDst

){
	if(nLength <= 0x7F){
		*(pDst++) = (EPS_INT8)nLength;
	} else{

		if( EPS_ENDIAN_LITTLE == cpuEndian ){
			EPS_INT8	n = (sizeof(EPS_INT32) / sizeof(EPS_INT8)) - 1;

			/* set length */
			for(; n > -1 && 0x00 == *((EPS_INT8*)&nLength + n); n--);
			*(pDst++) = (EPS_INT8)(0x80 & (n+1));

			for(; n > -1; n--){
				*(pDst++) = *((EPS_INT8*)&nLength + n);
			}
		} else{
			EPS_INT8 nLim = sizeof(EPS_INT32) / sizeof(EPS_INT8);
			EPS_INT8 n = 0;

			/* set length */
			for(; n < nLim && 0x00 == *((EPS_INT8*)&nLength + n); n++);
			*(pDst++) = (EPS_INT8)(0x80 & (nLim-n));

			for(; n < nLim; n++){
				*(pDst++) = *((EPS_INT8*)&nLength + n);
			}
		}	
	}

	return pDst;
}


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     OidToStr()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc	        EPS_INT8*           I: ObjectID binary                                  */
/* nSrcLen      EPS_UINT16          O: ObjectID binary length                           */
/* pDst         EPS_INT8**          O: ObjectID string                                  */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Convert binary MIB ObjectID to string.                                          */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE OidToStr(
							 
		EPS_INT8*   pSrc, 
		EPS_UINT32  nSrcLen, 
		EPS_INT8**  pDst
){

#define SNMP_OID_SIZE_INI	(32)
#define SNMP_OID_SIZE_GRW	(16)	/* 32bit max = 4294967295 is 10 character */
	EPS_UINT16	nCnt = 0;
	EPS_UINT16	nDstSize = 0;
	EPS_UINT16	nDstLen = 0;
	EPS_INT8*	pTmp = NULL;
	EPS_UINT32	nElm = 0;
	EPS_INT8*	pEnd = pSrc + nSrcLen;

	nDstSize = SNMP_OID_SIZE_INI;
	*pDst = (EPS_INT8*)EPS_ALLOC( nDstSize );

	sprintf(*pDst, "%d.%d.", *pSrc / 40, *pSrc % 40);
	pSrc++;

	while( pSrc < pEnd ){
		nDstLen = (EPS_UINT16)strlen(*pDst);
		if( nDstSize - nDstLen < SNMP_OID_SIZE_GRW ){
			nDstSize += SNMP_OID_SIZE_GRW;
			pTmp = (EPS_INT8*)EPS_ALLOC( nDstSize );
			if(pTmp){
				memcpy(pTmp, *pDst, nDstLen+1);
				EPS_SAFE_RELEASE( *pDst );
				*pDst = pTmp;
			} else{
				EPS_SAFE_RELEASE( *pDst );
				return EPS_ERR_MEMORY_ALLOCATION;
			}
		}

		if( 0x80 & *pSrc ){
			nElm = 0;
			while( 0x80 & pSrc[nCnt] ){
				nElm += (EPS_INT8)( *(pSrc++) ^ 0x80 );
				nElm <<= 7;
			}
			nElm += (EPS_INT8)(*(pSrc++));

			sprintf(&(*pDst)[nDstLen], "%d.", nElm);
		} else{
			sprintf(&(*pDst)[nDstLen], "%d.", *(pSrc++)); 
		}
	}

	nDstLen = (EPS_UINT16)strlen(*pDst) - 1;
	(*pDst)[nDstLen] = '\0';

	return EPS_ERR_NONE;
}	


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     StrToOid()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc		    EPS_INT8*           I: ObjectID string                                  */
/* pDst		    EPS_INT8*           O: ObjectID binary                                  */
/* pDstLen      EPS_INT16*          I/O: ObjectID binary length                         */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_ERR_NONE					- Success	                                    */
/*      EPS_ERR_MEMORY_ALLOCATION       - Failed to allocate memory                     */
/*                                                                                      */
/* Description:                                                                         */
/*      Convert string to MIB ObjectID binary.                                          */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_ERR_CODE StrToOid(
							 
		const EPS_INT8* pSrc, 
		EPS_INT8*       pDst, 
		EPS_UINT16*     pDstLen
		
){
	EPS_UINT16	nCnt = 0;
	EPS_INT8*	pTmp = NULL;
	EPS_INT8*	p = NULL;
	EPS_UINT32	nElm = 0;
	EPS_UINT8	nBinNum = 0;
	EPS_UINT8	n = 0;

	pTmp = (EPS_INT8*)EPS_ALLOC( strlen(pSrc)+1 );
	if(!pTmp){
		return EPS_ERR_MEMORY_ALLOCATION;
	}
	strcpy(pTmp, pSrc);

	p = strtok(pTmp, ".");
	if( NULL != p ){
		pDst[nCnt] = (EPS_INT8)(atoi(p) * 40);
	} else{
		EPS_FREE(pTmp);
		return EPS_ERR_NONE;
	}
	p = strtok(NULL, ".");
	if( NULL != p ){
		pDst[nCnt] += (EPS_INT8)(atoi(p));
		nCnt++;
	} else{
		EPS_FREE(pTmp);
		return EPS_ERR_NONE;
	}

	p = strtok(NULL, ".");
	while( NULL != p){
		if(*pDstLen-1 < nCnt){
			return EPS_ERR_MEMORY_ALLOCATION;
		}

		nElm = atoi(p);
		if(0x7f >= nElm){
			pDst[nCnt] = (EPS_INT8)nElm;
		} else{
			/* Necessary byte number calculation */
			for(nBinNum = 0; !((nElm << nBinNum) & 0x80000000); nBinNum++);

			nBinNum = sizeof(EPS_UINT32)*8 - nBinNum;
			nBinNum = nBinNum / 7 + ((nBinNum%7)?1:0);
			nCnt += nBinNum-1;

			/* It stores it by dividing 7 bits. */
			pDst[nCnt] = (EPS_INT8)(nElm & 0x7F);
			for(n = 1; n < nBinNum; n++){
				pDst[nCnt-n] = (EPS_INT8)(((nElm >> 7*n) & 0x7F) + 0x80);
			}
		}

		nCnt++;
		p = strtok(NULL, ".");
	}

	EPS_SAFE_RELEASE(pTmp);
	*pDstLen = nCnt;

	return EPS_ERR_NONE;
}	


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     IntToBer()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc		    EPS_INT32           I: int value                                        */
/* pDst		    EPS_INT32           O: pointer for BER Integer                          */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT16	: BER Integer length                                                */
/*                                                                                      */
/* Description:                                                                         */
/*      Convert integer to BER integer.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_UINT16 IntToBer(

		EPS_INT32 nSrc, 
		EPS_INT8* pDst

){
	EPS_UINT16	nCnt = 0;

	if( EPS_ENDIAN_LITTLE == cpuEndian ){
		EPS_INT8	n = (sizeof(EPS_INT32) / sizeof(EPS_INT8)) - 1;

		for(; n > -1; n--){
			if( 0 == nCnt){
				/* An unnecessary bit is thrown away. */
				if( 0xff == (EPS_INT32)*((EPS_UINT8*)&nSrc + n) ){
					if( n > 0 &&
						*((EPS_UINT8*)&nSrc + n-1) & 0x80 ){
						continue;
					}
				} else if( 0x00 == *((EPS_UINT8*)&nSrc + n) ){
					if( n > 0 &&
						!(*((EPS_UINT8*)&nSrc + n-1) & 0x80) ){
						continue;
					}
				}
			}

			*(pDst++) = *((EPS_UINT8*)&nSrc + n);
			nCnt++;
		}
	} else{
		EPS_INT8	nMax = (sizeof(EPS_INT32) / sizeof(EPS_INT8));
		EPS_INT8	n = 0;

		for(; n < nMax; n++){
			if( 0 == nCnt){
				/* An unnecessary bit is thrown away. */
				if( 0xff == *((EPS_UINT8*)&nSrc + n) ){
					if( n < nMax-1 &&
						*((EPS_UINT8*)&nSrc + n+1) & 0x80 ){
						continue;
					}
				} else if( 0x00 == *((EPS_UINT8*)&nSrc + n) ){
					if( n < nMax-1 &&
						!(*((EPS_UINT8*)&nSrc + n+1) & 0x80) ){
						continue;
					}
				}
			}

			*(pDst++) = *((EPS_UINT8*)&nSrc + n);
			nCnt++;
		}
	}

	return nCnt;
}	


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     BerToInt()                                                        */
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* Name:        Type:               Description:                                        */
/* nSrc		    EPS_INT32           I: pointer for BER Integer                          */
/* pDst		    EPS_INT32           I: BER Integer length                               */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT32	: int value                                                         */
/*                                                                                      */
/* Description:                                                                         */
/*      Convert BER integer to integer.                                                 */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_INT32 BerToInt(

		EPS_INT8* pSrc, 
		EPS_INT32 nLen

){
	EPS_INT32	nDst = 0;

	if( EPS_ENDIAN_LITTLE == cpuEndian ){
		EPS_INT8	nMax = (sizeof(EPS_INT32) / sizeof(EPS_INT8)) - 1;
		EPS_INT32	n = 0;
		EPS_INT32	m = nLen - 1;

		for(; n < nMax && m > -1; n++, m--){
			*((EPS_INT8*)&nDst + n) = *(pSrc+m);
		}

		if( n < nMax ){
			if( *(pSrc) & 0x80 ){
				memset(((EPS_INT8*)&nDst + n), 0xff, nMax-(n-1));
			} else{
				memset(((EPS_INT8*)&nDst + n), 0x00, nMax-(n-1));
			}
		}
	} else{
		EPS_INT8	nDstSize = (sizeof(EPS_INT32) / sizeof(EPS_INT8));
		EPS_INT32	n = nDstSize-nLen;
		EPS_INT32	m = 0;

		if( *(pSrc) & 0x80 ){
			memset(&nDst, 0xff, nDstSize-nLen);
		} else{
			memset(&nDst, 0x00, nDstSize-nLen);
		}

		for(; n < nDstSize && m < nLen; n++, m++){
			*((EPS_INT8*)&nDst + n) = *(pSrc+m);
		}
	}

	return nDst;
}	


/*******************************************|********************************************/
/*                                                                                      */
/* Function name:     GetRequestId()		    										*/
/*                                                                                      */
/* Arguments                                                                            */
/* ---------                                                                            */
/* (none)										                                        */
/*                                                                                      */
/* Return value:                                                                        */
/*      EPS_INT16	: RequestID									                        */
/*                                                                                      */
/* Description:                                                                         */
/*      Generate SNMP request ID (value is between from 1 to 127).                      */
/*                                                                                      */
/*******************************************|********************************************/
static EPS_UINT8 GetRequestId(void)
{
	static EPS_UINT8 nId = 1;

	nId++;		
	if( (char)0x7f < nId ){
		nId = 1;
	}

	return nId;
}

#ifdef GCOMSW_CMD_ESCPAGE
static EPS_INT32 GetColorID(EPS_INT8* name)
{
	EPS_INT32 id = EPS_COLOR_UNKNOWN;
	EPS_INT8* p;

	if( memStrStr(name, "Black", FALSE) ){
		id = EPS_COLOR_BLACK;
	} else if( memStrStr(name, "Cyan", FALSE) ){
	    id = EPS_COLOR_CYAN;
	} else if( memStrStr(name, "Magenta", FALSE) ){
		id = EPS_COLOR_MAGENTA;
	} else if( memStrStr(name, "Yellow", FALSE) ){
		id = EPS_COLOR_YELLOW;
	} else if( memStrStr(name, "Red", FALSE) ){
		id = EPS_COLOR_RED;
	} else if( memStrStr(name, "Violet", FALSE) ){
		id = EPS_COLOR_VIOLET;
	} else if( memStrStr(name, "Clear", FALSE) ){
		id = EPS_COLOR_CLEAR;
	} else if( memStrStr(name, "Gray", FALSE) ){
		id = EPS_COLOR_GRAY;
	} else if( memStrStr(name, "Orange", FALSE) ){
		id = EPS_COLOR_ORANGE;
	} else if( memStrStr(name, "Green", FALSE) ){
		id = EPS_COLOR_GREEN;
	}

	if( (p = memStrStr(name, "Light", TRUE)) != NULL ){
		if(EPS_COLOR_CYAN == id){
			id = EPS_COLOR_LIGHTCYAN;
		} else if(EPS_COLOR_MAGENTA == id){
			id = EPS_COLOR_LIGHTMAGENTA;
		} else if(EPS_COLOR_YELLOW == id){
			id = EPS_COLOR_LIGHTYELLOW;
		} else if(EPS_COLOR_BLACK == id){
			if( memStrStr(p, "Light", FALSE) ){
				id = EPS_COLOR_LIGHTLIGHTBLACK;
			} else{
				id = EPS_COLOR_LIGHTBLACK;
			} 
		} 

	} else if( memStrStr(name, "Matte", FALSE) ){
		if(EPS_COLOR_BLACK == id){
			id = EPS_COLOR_MATTEBLACK;
		}
	} else if( memStrStr(name, "Dark", FALSE) ){
		if(EPS_COLOR_YELLOW == id){
			id = EPS_COLOR_DARKYELLOW;
		}
	} else if( memStrStr(name, "Photo", FALSE) ){
		if(EPS_COLOR_BLACK == id){
			id = EPS_COLOR_PHOTOBLACK;
		}
	}

	return id;
}
#endif /* GCOMSW_CMD_ESCPAGE */

/*________________________________   epson-net-snmp.c   ________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
