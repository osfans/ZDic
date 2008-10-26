#include <PalmOS.h>
#include <HsCreators.h>
#include <SmartTextEngine.h>
#include "ZDicTools.h"
#include "SysZLib.h"

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
