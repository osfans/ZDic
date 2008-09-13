#include <PalmOS.h>
#include "ZDicTools.h"


#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    ZDicToolsLibInitial
 *
 * DESCRIPTION: This routine first find the share library, if the library
 *				dosen't load then load it. and return reference number of
 *              the share library.
 *
 * PARAMETERS:  -> libType Type of library database.
 *				-> libCreator Creator of library database
 *				-> nameP Pointer to the name of a loaded library.
 *				<- refNumP Pointer to a variable for returning the
 *				   library reference number (on failure,
 *				   this variable is undefined).
 *				<- bLoadP True if we load the library else false.
 *
 * RETURNED:    errNone if no error
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
 
Err ZDicToolsLibInitial(UInt32 libType, UInt32 libCreator, const Char *nameP,
	UInt16 *refNumP, Boolean *bLoadP)
{
	Err	err;
	
	*refNumP = 0;
	*bLoadP = false;
	
	err = SysLibFind(nameP, refNumP);
	if(err)
		{  
		err = SysLibLoad(libType, libCreator, refNumP);
		if (!err)
			*bLoadP = true;
		}
	
	return err;
}

/***********************************************************************
 *
 * FUNCTION:    ZDicToolsLibRelease
 *
 * DESCRIPTION: This routine Unload a library previously loaded .
 *				must called after GSLLibInitial.
 *
 * PARAMETERS:  -> refNum The library reference number
 *				-> bLoadP true if we need release the library.
 *
 * RETURNED:    errNone if no error
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
Err ZDicToolsLibRelease(UInt16 refNum, Boolean bLoad)
{
	Err	err;
	
	if(bLoad)
		err = SysLibRemove(refNum);
	
	return err;
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
