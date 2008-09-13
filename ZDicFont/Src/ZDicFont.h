/*
 * ZDicFont.h
 *
 * public header for shared library
 *
 * This wizard-generated code is based on code adapted from the
 * SampleLib project distributed as part of the Palm OS SDK 4.0,
 *
 * Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */

#ifndef ZDICFONT_H_
#define ZDICFONT_H_

/* Palm OS common definitions */
#include <SystemMgr.h>

/* If we're actually compiling the library code, then we need to
 * eliminate the trap glue that would otherwise be generated from
 * this header file in order to prevent compiler errors in CW Pro 2. */
#ifdef BUILDING_ZDICFONT
	#define ZDICFONT_LIB_TRAP(trapNum)
#else
	#define ZDICFONT_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/*********************************************************************
 * Type and creator of Sample Library database
 *********************************************************************/

#define		ZDicFontCreatorID	'ZDic'
#define		ZDicFontTypeID		sysFileTLibrary

/*********************************************************************
 * Internal library name which can be passed to SysLibFind()
 *********************************************************************/

#define		ZDicFontName		"ZDicFont"

typedef enum
{
    eZDicFontSmall, eZDicFontLarge, eZDicFontCount
} ZDicFontEnum;

typedef struct
{
    DmOpenRef   fontLibP;                       // ZDict font library pointer.
    
	MemHandle	phonicSmallFontH;				// handle of small phonetic resource.
	MemHandle	phonicLargeFontH;				// handle of large phonetic resource.
	FontType	*phonicSmallFontP;				// pointer of small phonetic resource.
	FontType	*phonicLargeFontP;				// pointer of large phonetic resource.
	FontID		smallFontID;					// small font id.
	FontID		largeFontID;					// large font id.
	UInt32      reserve1;
	UInt32      reserve2;
	
} ZDicFontType;

#define kPHONIC_SMALL_FONT_ID	(fntAppFontCustomBase + 1 + 78)	// 78: When da launch, some application maybe use fntAppFontCustomBase + 1
#define kPHONIC_LARGE_FONT_ID	(fntAppFontCustomBase + 2 + 78)

/*********************************************************************
 * ZDicFont result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *********************************************************************/

/* invalid parameter */
#define ZDicFontErrParam		(appErrorClass | 1)		

/* library is not open */
#define ZDicFontErrNotOpen		(appErrorClass | 2)		

/* returned from ZDicFontClose() if the library is still open */
#define ZDicFontErrStillOpen	(appErrorClass | 3)		

/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Standard library open, close, sleep and wake functions */

extern Err ZDicFontOpen(UInt16 refNum, UInt32 * clientContextP)
	ZDICFONT_LIB_TRAP(sysLibTrapOpen);
				
extern Err ZDicFontClose(UInt16 refNum, UInt32 clientContext)
	ZDICFONT_LIB_TRAP(sysLibTrapClose);

extern Err ZDicFontSleep(UInt16 refNum)
	ZDICFONT_LIB_TRAP(sysLibTrapSleep);

extern Err ZDicFontWake(UInt16 refNum)
	ZDICFONT_LIB_TRAP(sysLibTrapWake);

/* Custom library API functions */
extern Err ZDicFontInit ( UInt16 refNum, ZDicFontType* fontP, Boolean bUseSysFont )
	ZDICFONT_LIB_TRAP(sysLibTrapCustom + 0);

extern Err ZDicFontRelease ( UInt16 refNum, ZDicFontType* fontP )
	ZDICFONT_LIB_TRAP(sysLibTrapCustom + 1);

extern FontID ZDicFontSet ( UInt16 refNum, ZDicFontType* fontP, FontID font )
	ZDICFONT_LIB_TRAP(sysLibTrapCustom + 2);

extern FontID ZDicFontGetFontID ( UInt16 refNum, ZDicFontType* fontP, ZDicFontEnum font )
	ZDICFONT_LIB_TRAP(sysLibTrapCustom + 3);

#ifdef __cplusplus
}
#endif

/*
 * FUNCTION: ZDicFont_OpenLibrary
 *
 * DESCRIPTION:
 *
 * User-level call to open the library.  This inline function
 * handles the messy task of finding or loading the library
 * and calling its open function, including handling cleanup
 * if the library could not be opened.
 * 
 * PARAMETERS:
 *
 * refNumP
 *		Pointer to UInt16 variable that will hold the new
 *      library reference number for use in later calls
 *
 * clientContextP
 *		pointer to variable for returning client context.  The client context is
 *		used to maintain client-specific data for multiple client support.  The 
 *		value returned here will be used as a parameter for other library 
 *		functions which require a client context.  
 *
 * CALLED BY: System
 *
 * RETURNS:
 *		errNone
 *		memErrNotEnoughSpace
 *      sysErrLibNotFound
 *      sysErrNoFreeRAM
 *      sysErrNoFreeLibSlots
 *
 * SIDE EFFECTS:
 *		*clientContextP will be set to client context on success, or zero on
 *      error.
 */
 
__inline Err ZDicFont_OpenLibrary(UInt16 *refNumP, UInt32 * clientContextP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(ZDicFontName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(ZDicFontTypeID, ZDicFontCreatorID, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = ZDicFontOpen(*refNumP, clientContextP);
		if (error != errNone)
		{
			if (loaded)
			{
				SysLibRemove(*refNumP);
			}

			*refNumP = sysInvalidRefNum;
		}
	}
	
	return error;
}

/*
 * FUNCTION: ZDicFont_CloseLibrary
 *
 * DESCRIPTION:	
 *
 * User-level call to closes the shared library.  This handles removal
 * of the library from system if there are no users remaining.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number obtained from ZDicFont_OpenLibrary().
 *
 * clientContext
 *		client context (as returned by the open call)
 *
 * CALLED BY: Whoever wants to close the library
 *
 * RETURNS:
 *		errNone
 *		sysErrParamErr
 */

__inline Err ZDicFont_CloseLibrary(UInt16 refNum, UInt32 clientContext)
{
	Err error;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = ZDicFontClose(refNum, clientContext);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	else if (error == ZDicFontErrStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}

#endif /* ZDICFONT_H_ */
