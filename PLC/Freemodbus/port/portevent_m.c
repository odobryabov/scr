/*
 * FreeModbus Libary: STM32 Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portevent_m.c v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#include "port.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
/* ----------------------- Variables ----------------------------------------*/
static eMBEventType eQueuedEvent;
static BOOL     xEventInQueue;
uint32_t xMasterOsEvent;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBMasterPortEventInit( void )
{
	xEventInQueue = FALSE;
	return TRUE;
}

BOOL
xMBMasterPortEventPost( eMBMasterEventType eEvent )
{
	xEventInQueue = TRUE;
	eQueuedEvent = (eMBMasterEventType)eEvent;
	return TRUE;
}

BOOL
xMBMasterPortEventGet( eMBMasterEventType * eEvent )
{
	
	   BOOL xEventHappened = FALSE;

    if ( xEventInQueue )
    {
        *eEvent = (eMBMasterEventType)eQueuedEvent;
        xEventInQueue = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}

void vMBMasterOsResInit( void )
{
//	rt_sem_init(&xMasterRunRes, "master res", 0x01 , RT_IPC_FLAG_PRIO);
	ENTER_CRITICAL_SECTION( );
	xMasterOsEvent = 1;
	EXIT_CRITICAL_SECTION( );
}
/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
BOOL xMBMasterRunResTake( LONG lTimeOut )
{
	/*If waiting time is -1 .It will wait forever */
	ENTER_CRITICAL_SECTION( );
	
	if ( xMasterOsEvent > 0 ) 
		{
			xMasterOsEvent--;
			EXIT_CRITICAL_SECTION( );
		}
	else 
		{
			EXIT_CRITICAL_SECTION( );

			if ( lTimeOut == -1 ) 
				{
					while ( !xMasterOsEvent ) 
					;
					return  TRUE;
				}
			else 
				{
					while ( 1 ) 
						{
							if ( lTimeOut == 0 )
							break;

							if ( xMasterOsEvent )
							return  TRUE;

							lTimeOut--;
						}
				}
			return FALSE;
			}
	return  TRUE;
}

void vMBMasterRunResRelease( void )
{
	/* release resource */
//	rt_sem_release(&xMasterRunRes);
	ENTER_CRITICAL_SECTION( );
	xMasterOsEvent += 1;
	EXIT_CRITICAL_SECTION( );
}

/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(UCHAR ucDestAddress, const UCHAR* pucPDUData,
		USHORT ucPDULength) {
	/**
	 * @note This code is use OS's event mechanism for modbus master protocol stack.
	 * If you don't use OS, you can change it.
	 */
	/* You can add your code under here. */
	xMasterOsEvent = EV_MASTER_ERROR_RESPOND_TIMEOUT;
}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(UCHAR ucDestAddress, const UCHAR* pucPDUData,
		USHORT ucPDULength) {
	/**
	 * @note This code is use OS's event mechanism for modbus master protocol stack.
	 * If you don't use OS, you can change it.
	 */
	/* You can add your code under here. */
	xMasterOsEvent = EV_MASTER_ERROR_RECEIVE_DATA;
}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(UCHAR ucDestAddress, const UCHAR* pucPDUData,
		USHORT ucPDULength) {
	/**
	 * @note This code is use OS's event mechanism for modbus master protocol stack.
	 * If you don't use OS, you can change it.
	 */
	/* You can add your code under here. */
	xMasterOsEvent = EV_MASTER_ERROR_EXECUTE_FUNCTION;
}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestScuuess( void ) {
	/**
	 * @note This code is use OS's event mechanism for modbus master protocol stack.
	 * If you don't use OS, you can change it.
	 */
	/* You can add your code under here. */
	xMasterOsEvent = EV_MASTER_PROCESS_SUCESS;
}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void ) {
	
	eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;
	
    uint32_t recvedEvent;

    uint32_t i;
	
    for ( i = 0; i < 2400000; i++ );
	
    recvedEvent = xMasterOsEvent;
	
	switch (recvedEvent)
	{
	case EV_MASTER_PROCESS_SUCESS:
		break;
	case EV_MASTER_ERROR_RESPOND_TIMEOUT:
	{
		eErrStatus = MB_MRE_TIMEDOUT;
		break;
	}
	case EV_MASTER_ERROR_RECEIVE_DATA:
	{
		eErrStatus = MB_MRE_REV_DATA;
		break;
	}
	case EV_MASTER_ERROR_EXECUTE_FUNCTION:
	{
		eErrStatus = MB_MRE_EXE_FUN;
		break;
	}
	}
    return eErrStatus;
}

#endif
