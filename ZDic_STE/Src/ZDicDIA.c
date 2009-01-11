#include <PalmOS.h>
#include <CtlGlue.h>    // for CtlGlueSetFont
#include "ZDic.h"
#include "ZDicTools.h"

#include "ZDicConfig.h"
#include "ZDicDIA.h"

#include "ZDic_Rsc.h"

#if defined( ZDIC_DIA_SUPPORT )

Err ZDicDIALibInitial ( AppGlobalType *	global )
{
    Err err;
    
    global->bDiaEnable = false;
    
    err = FtrGet(pinCreator, pinFtrAPIVersion, &global->diaVersion);
    if (!err && global->diaVersion)
    {
        //PINS exists
        global->bDiaEnable = true;

         // save input area and trigger state
        global->diaPinInputAreaState = PINGetInputAreaState ();
        global->diaPinInputTriggerState = PINGetInputTriggerState ();
        
   }
    
    return errNone;
}

Err ZDicDIAFormLoadInitial ( AppGlobalType* global, FormPtr frmP )
{
    WinHandle   formWinH;
    Err         err;
    
    if ( global->bDiaEnable )
    {
        // Set the same policy for each form in application.
        err = FrmSetDIAPolicyAttr ( frmP, frmDIAPolicyCustom );
    
        // Enable the input trigger in normal launch else disable.
        //err = PINSetInputTriggerState( global->subLaunch ? pinInputTriggerDisabled : pinInputTriggerEnabled);
        err = PINSetInputAreaState ( global->diaPinInputAreaState );
        formWinH = FrmGetWindowHandle (frmP);
        WinSetConstraintsSize(formWinH, 160, 225, 225, 160, 160, 160);
    }

    return errNone;
}

Err ZDicDIAFormClose ( AppGlobalType* global )
{
    Err         err;
    
    if ( global->bDiaEnable )
    {
        // Enable the input trigger in normal launch else disable.
        err = PINSetInputTriggerState( global->diaPinInputTriggerState );
    }

    return errNone;
}

void ZDicDIACmdNotify ( MemPtr cmdPBP )
{
    if ( ( ( SysNotifyParamType* ) cmdPBP )->notifyType == sysNotifyDisplayResizedEvent )
    {
        EventType resizedEvent;

        MemSet(&resizedEvent, sizeof(EventType), 0);

        // add winDisplayChangedEvent to the event queue
        resizedEvent.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue(&resizedEvent, 0, true);
    }
    
    return;
}

Boolean ZDicDIAWinEnter (  AppGlobalType* global, EventType * eventP )
{
    EventType   resizedEvent;
    FormPtr     frmP;
    Err         err;

    if ( global->bDiaEnable )
    {
        frmP = FrmGetActiveForm ();
        
        if ( eventP->data.winEnter.enterWindow == FrmGetWindowHandle ( frmP ) )
        {
            MemSet(&resizedEvent, sizeof(EventType), 0);

            // add winDisplayChangedEvent to the event queue
            resizedEvent.eType = winDisplayChangedEvent;
            EvtAddUniqueEventToQueue(&resizedEvent, 0, true);
            
            // Enable the input trigger in normal launch else disable.
            //err = PINSetInputTriggerState( global->subLaunch ? pinInputTriggerDisabled : pinInputTriggerEnabled);
               
            // restore input area
            err = PINSetInputAreaState ( global->diaPinInputAreaState );

        }
    }    
    return true;
}

static void ZDicDIAResizeForm ( FormType *frmP, RectangleType* fromBoundsP, 
    RectangleType* toBoundsP, AppGlobalType* global )
{
    Int16           heightDelta, widthDelta;
    UInt16          numObjects, i;
    Coord           x, y;
    RectangleType   objBounds;
    FieldType*      fldP;

    heightDelta = widthDelta = 0;
    numObjects = 0;
    x = y = 0;

    // Determine the amount of the change
    heightDelta = ( toBoundsP->extent.y - toBoundsP->topLeft.y ) -
        ( fromBoundsP->extent.y - fromBoundsP->topLeft.y );
    widthDelta = ( toBoundsP->extent.x - toBoundsP->topLeft.x ) -
        ( fromBoundsP->extent.x - fromBoundsP->topLeft.x );

    // Iterate through objects and re-position them.
    // This form consists of a big text field and
    // command buttons. We move the command buttons to the
    // bottom and resize the text field to display more data.
    numObjects = FrmGetNumberOfObjects ( frmP );
    for ( i = 0; i < numObjects; i++ )
    {
        if ( frmGraffitiStateObj == FrmGetObjectType ( frmP, i ) )
        {
            // Change x position
            FrmGetObjectPosition ( frmP, i, &x, &y );
            FrmSetObjectPosition ( frmP, i, x + widthDelta, y );
            continue;
        }
        
        switch ( FrmGetObjectId ( frmP, i ) ) 
        {
        case MainNewButton:
        case MainGotoButton:
        case MainHistoryPopupList:
        case MainHistoryTrigger:
        case MainBackHistoryRepeatButton:
        case MainCapChange:
        case MainExportToMemo:
        case MainSettingButton:
        case MainBtnHeadButton:
            // Change x, y position.
            FrmGetObjectPosition ( frmP, i, &x, &y );
            FrmSetObjectPosition ( frmP, i, x + widthDelta, y +
                heightDelta );
            break;
            
        case MainExitButton:
        //case MainJumpPushButton:
        //case MainSelectPushButton:
        case MainPlayVoice:
            // Change x position
            FrmGetObjectPosition ( frmP, i, &x, &y );
            FrmSetObjectPosition ( frmP, i, x + widthDelta, y );
            break;
            
        case MainWordButton:
        case MainToolbarBtBg:
            // Change y position.
            FrmGetObjectBounds ( frmP, i, &objBounds );
            objBounds.topLeft.y += heightDelta;
            FrmSetObjectBounds ( frmP, i, &objBounds );
            break;

        case MainWordField:
            // Change y, width.
            FrmGetObjectBounds ( frmP, i, &objBounds );
            objBounds.topLeft.y += heightDelta;
            objBounds.extent.x += widthDelta;
            FrmSetObjectBounds ( frmP, i, &objBounds );
            fldP = (FieldType*) FrmGetObjectPtr ( frmP, i );
            FldRecalculateField ( fldP, false );
            break;
        }
    }
    
    // Adjust the word list and desc field.
    MainFormAdjustObject ( toBoundsP );
    
    return; 
}

Boolean ZDicDIADisplayChange ( AppGlobalType* global )
{
    Boolean handled = false;
    
    if ( global->bDiaEnable )
    {
        FormPtr         frmP;
        RectangleType   curBounds, displayBounds;
   
        // get the current bounds for the form
        frmP = FrmGetActiveForm ();
        ZDicToolsWinGetBounds ( FrmGetWindowHandle ( frmP ), &curBounds );
            
        // get the new display window bounds
        ZDicToolsWinGetBounds ( WinGetDisplayWindow (), &displayBounds );

        if ( curBounds.topLeft.x == displayBounds.topLeft.x
            && curBounds.topLeft.y == displayBounds.topLeft.y
            && curBounds.extent.x == displayBounds.extent.x
            && curBounds.extent.y == displayBounds.extent.y )
            
            return false;

        switch ( FrmGetFormId ( frmP ) )
        {
	        case MainForm:            
	            ZDicDIAResizeForm ( frmP, &curBounds, &displayBounds, global );
	            WinSetBounds ( FrmGetWindowHandle ( frmP ), &displayBounds );
	            break;
	            
	        case DAForm:
	        case DAFormLarge:
	        	// adjust the bround and set new form bround
	        	DAFormAdjustFormBounds ( global, frmP, curBounds, displayBounds );
	            break;       
        }
        // save input area state
        global->diaPinInputAreaState = PINGetInputAreaState ();

        handled = true;
    }

    return handled;
}


#endif


