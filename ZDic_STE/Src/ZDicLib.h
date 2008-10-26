/*
 * ZDicLib.h
 *
 * public header for shared library
 *
 * This wizard-generated code is based on code adapted from the
 * SampleLib project distributed as part of the Palm OS SDK 4.0,
 *
 * Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */

#ifndef ZDICLIB_H_
#define ZDICLIB_H_

/* Palm OS common definitions */
#include <SystemMgr.h>
#include <VFSMgr.h>
#include "ZDicCommon.h"

/* If we're actually compiling the library code, then we need to
 * eliminate the trap glue that would otherwise be generated from
 * this header file in order to prevent compiler errors in CW Pro 2. */
#ifdef BUILDING_ZDICLIB
	#define ZDICLIB_LIB_TRAP(trapNum)
#else
	#define ZDICLIB_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/*********************************************************************
 * Type and creator of Sample Library database
 *********************************************************************/

#define		ZDicLibCreatorID	'ZDlb'
#define		ZDicLibTypeID		sysFileTLibrary

/*********************************************************************
 * Internal library name which can be passed to SysLibFind()
 *********************************************************************/

#define		ZDicLibName		"ZDicLib"

/*********************************************************************
 * ZDicLib result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *********************************************************************/

/* invalid parameter */
#define ZDicLibErrParam		(appErrorClass | 1)		

/* library is not open */
#define ZDicLibErrNotOpen		(appErrorClass | 2)		

/* returned from ZDicLibClose() if the library is still open */
#define ZDicLibErrStillOpen	(appErrorClass | 3)		

/* not found the dict */
#define ZDicLibNotFoundDict 	(appErrorClass | 4)		

/*********************************************************************
 * Internal Structures
 *********************************************************************/


/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Standard library open, close, sleep and wake functions */

extern Err ZDicLibOpen(UInt16 refNum, UInt32 * clientContextP)
	ZDICLIB_LIB_TRAP(sysLibTrapOpen);
				
extern Err ZDicLibClose(UInt16 refNum, UInt32 clientContext)
	ZDICLIB_LIB_TRAP(sysLibTrapClose);

extern Err ZDicLibSleep(UInt16 refNum)
	ZDICLIB_LIB_TRAP(sysLibTrapSleep);

extern Err ZDicLibWake(UInt16 refNum)
	ZDICLIB_LIB_TRAP(sysLibTrapWake);

/* Custom library API functions */

extern Err ZDicLibLookup(UInt16 refNum, const UInt8 *wordStr, UInt16 *matchs, 
                      UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 5);
    	
extern Err ZDicLibLookupForward(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 6);
    
extern Err ZDicLibLookupBackward(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 7);
    
extern Err ZDicLibLookupCurrent(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 8);
    
extern Err ZDicLibLookupWordListInit(UInt16 refNum, WinDirectionType direction, Boolean init)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 9);
    
extern Err ZDicLibLookupWordListSelect(UInt16 refNum, UInt16 select, UInt8 **explainPtr, UInt32 *explainLen)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 10);

extern Err ZDicLibDBInitDictList(UInt16 refNum, UInt32 type, UInt32 creator, Int16* dictCount)
	ZDICLIB_LIB_TRAP(sysLibTrapBase + 11);
	    
extern Err ZDicLibDBInitIndexRecord(UInt16 refNum)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 12);
    
extern Err ZDicLibOpenCurrentDict(UInt16 refNum)
	ZDICLIB_LIB_TRAP(sysLibTrapBase + 13);
	    
extern Err ZDicLibCloseCurrentDict(UInt16 refNum)
    ZDICLIB_LIB_TRAP(sysLibTrapBase + 14);
    
extern Err ZDicLibDBInitPopupList(UInt16 refNum)
	ZDICLIB_LIB_TRAP(sysLibTrapBase + 15);

extern Err ZDicLibDBInitShortcutList(UInt16 refNum)
	ZDICLIB_LIB_TRAP(sysLibTrapBase + 16);
	
#ifdef __cplusplus
}
#endif


#endif /* ZDICLIB_H_ */
