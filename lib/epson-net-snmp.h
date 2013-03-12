/*_______________________________   epson-net-snmp.h   _________________________________*/

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
/*                           Epson SNMP Protocol Definitions                            */
/*                                                                                      */
/*******************************************|********************************************/

#ifndef __EPSON_NET_SNMP_H__
#define __EPSON_NET_SNMP_H__
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------  Includes   -------------------------------------*/
/*******************************************|********************************************/
#include "epson-typedefs.h"


/*-----------------------------------  Definitions  ------------------------------------*/
/*******************************************|********************************************/

/****************************************************************************************/
/* public SNMP functions                                                                */
typedef EPS_ERR_CODE	(*SNMP_GetStatus    )(EPS_SOCKET, const EPS_INT8*, EPS_STATUS_INFO*);
typedef EPS_ERR_CODE	(*SNMP_GetInkInfo   )(const EPS_INT8*, EPS_STATUS_INFO*         );
typedef EPS_ERR_CODE	(*SNMP_GetPMString  )(const EPS_PRINTER_INN*, EPS_INT32, 
										      EPS_UINT8*, EPS_INT32*                    );
typedef EPS_ERR_CODE	(*SNMP_MechCommand  )(const EPS_PRINTER_INN*, EPS_INT32         );

typedef struct tagEPS_SNMP_FUNCS {
	SNMP_GetStatus		GetStatus;
	SNMP_GetInkInfo		GetInkInfo;
	SNMP_GetPMString    GetPMString;
	SNMP_MechCommand	MechCommand;
}EPS_SNMP_FUNCS;


/*--------------------------  Public Function Declarations   ---------------------------*/
/*******************************************|********************************************/
extern void			snmpSetupSTFunctions(EPS_SNMP_FUNCS*, const EPS_PRINTER_INN*        );

extern EPS_ERR_CODE snmpOpenSocket  (EPS_SOCKET*                                        );
extern void         snmpCloseSocket (EPS_SOCKET*                                        );

extern EPS_ERR_CODE snmpFindStart   (EPS_SOCKET*, const EPS_INT8*, EPS_BOOL             );
extern EPS_ERR_CODE snmpFind        (EPS_SOCKET, EPS_UINT16, EPS_INT32, EPS_PRINTER_INN**);
extern EPS_ERR_CODE snmpFindEnd	    (EPS_SOCKET                                         );
extern EPS_ERR_CODE snmpProbeByID	(EPS_INT8*, EPS_UINT16, EPS_INT32, EPS_UINT32, EPS_PRINTER_INN**);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* def __EPSON_NET_SNMP_H__ */

/*_______________________________   epson-net-snmp.h   _________________________________*/

/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*       1         2         3         4         5         6         7         8        */
/*******************************************|********************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***** End of File *** End of File *** End of File *** End of File *** End of File ******/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
