#include <PalmOS.h>
#include <HsCreators.h>
#include <SmartTextEngine.h>
#include "ZDicTools.h"
#include "SysZLib.h"
#include "ZDicLib.h"
#include "ZDic_Rsc.h"

#pragma mark -


Err ZDicToolsOpenZLib(UInt16 *refNumP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind("Z.lib", refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad('libr', 'ZLib', refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = STEOpen(*refNumP);
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

Err ZDicToolsCloseZLib(UInt16 refNum)
{
	Err error;
	UInt16 numsApp = 0;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = ZLibClose(refNum, &numsApp);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	
	return error;
}


Err ZDicToolsOpenSMTLib(UInt16 *refNumP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(steLibDBName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(sysFileTLibrary, hsFileCSmartTextEngine, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = ZLibOpen(*refNumP);
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

Err ZDicToolsCloseSMTLib(UInt16 refNum)
{
	Err error;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = STEClose(refNum);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	
	return error;
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current form.
 *
 * PARAMETERS:  -> formId id of the form to display
 *
 * RETURNED:     address of object as a void pointer
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

void * GetObjectPtr(UInt16 objectID)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}

/***********************************************************************
 *
 * FUNCTION:    ZDicToolsWinGetBounds
 *
 * DESCRIPTION: This routine returns the bounds of the winH argument
 *              in display relative coordinates.
 *
 * PARAMETERS:  winH -> window for which to get bounds
 *					 rP	<-	set to the bounds of the window
 * 
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
void ZDicToolsWinGetBounds(WinHandle winH, RectangleType* rP)
{
	WinHandle		oldDrawWinH;

    oldDrawWinH = WinGetDrawWindow ();
    
	// Set the draw and acitve windows to fixe bugs of chinese os.
	WinSetDrawWindow ( winH );
	WinGetDrawWindowBounds ( rP );
	
	if ( oldDrawWinH != 0 )
	    WinSetDrawWindow ( oldDrawWinH );
	    
	return;
}

/*
 * FUNCTION: ZDicLib_OpenLibrary
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
 
Err ZDicLib_OpenLibrary(UInt16 *refNumP, UInt32 * clientContextP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(ZDicLibName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(ZDicLibTypeID, ZDicLibCreatorID, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = ZDicLibOpen(*refNumP, clientContextP);
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
 * FUNCTION: ZDicLib_CloseLibrary
 *
 * DESCRIPTION:	
 *
 * User-level call to closes the shared library.  This handles removal
 * of the library from system if there are no users remaining.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number obtained from ZDicLib_OpenLibrary().
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

Err ZDicLib_CloseLibrary(UInt16 refNum, UInt32 clientContext)
{
	Err error;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = ZDicLibClose(refNum, clientContext);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	else if (error == ZDicLibErrStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}
