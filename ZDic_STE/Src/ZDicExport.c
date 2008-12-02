#include "ZDic.h"
#include "ZDicExport.h"
#include "ZDicTools.h"
#include "ZDic_Rsc.h"

#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    ExportSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Export Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    index of the selected category.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/
static UInt16 ExportSelectCategory ( DmOpenRef dbP, UInt16* category )
{
    Char * name;
    Boolean categoryEdited;

    name = ( Char * ) CtlGetLabel ( GetObjectPtr ( ExportOptionsCategoryPopupTriger ) );

    categoryEdited = CategorySelect ( dbP, FrmGetActiveForm (),
                                      ExportOptionsCategoryPopupTriger, ExportOptionsCategoryList,
                                      false, category, name, 1, categoryDefaultEditCategoryString );

    return ( categoryEdited );
}



/***********************************************************************
 *
 * FUNCTION:    ExportApply
 *
 * DESCRIPTION: This routine applies the changes made in the Export Dialog.
 *
 * PARAMETERS:  category - new catagory
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/
static void ExportApply ( FormType *frmP, UInt16 category )
{
    AppGlobalType	* global;
    UInt16 index, objID;

    global = AppGetGlobal();

    // Get the current setting of the secret checkbox.
    global->prefs.exportPrivate = ( CtlGetValue ( GetObjectPtr ( ExportOptionsPrivateCheckBox ) ) != 0 );
    global->prefs.exportCagetoryIndex = category;

    index = FrmGetControlGroupSelection ( frmP, 1 );
    objID = FrmGetObjectId ( frmP, index );
    global->prefs.exportAppCreatorID = objID == ExportOptionsMemoPad ? sysFileCMemo : (objID == ExportOptionsSugarMemo ?appSugarMemoCreator : appSuperMemoCreator);

    return ;
}


/***********************************************************************
 *
 * FUNCTION:    ExportInit
 *
 * DESCRIPTION: This routine initializes the Export Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/
static void ExportInit ( FormType * frmP, DmOpenRef dbP )
{
    Char * name;
    ControlPtr	ctl;
    AppGlobalType	*global;

    global = AppGetGlobal();

    // If the record is marked secret, turn on the secret checkbox.
    ctl = GetObjectPtr ( ExportOptionsPrivateCheckBox );
    if ( global->prefs.exportPrivate )
        CtlSetValue ( ctl, true );
    else
        CtlSetValue ( ctl, false );


    // Set the label of the category trigger.
    ctl = GetObjectPtr ( ExportOptionsCategoryPopupTriger );
    name = ( Char * ) CtlGetLabel ( ctl );
    CategoryGetName ( dbP, global->prefs.exportCagetoryIndex, name );
    CategorySetTriggerLabel ( ctl, name );

    // Set export application group selection.
    FrmSetControlGroupSelection ( frmP, 1,
                                  global->prefs.exportAppCreatorID == sysFileCMemo ?
                                  ExportOptionsMemoPad : (global->prefs.exportAppCreatorID == appSugarMemoCreator? ExportOptionsSugarMemo : ExportOptionsSuperMemo) );

    if ( global->prefs.exportAppCreatorID == sysFileCMemo )
    {
        FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateCheckBox) );
        FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateLabel) );
        FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryPopupTriger) );
        FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryLabel) );
    }
    
    return ;
}


/***********************************************************************
 *
 * FUNCTION:    ExportHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Export
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/

static Boolean ExportHandleEvent ( FormType * frmP, EventType * eventP,
                                   DmOpenRef dbP, Boolean *exit, UInt16 *category, Boolean *categoryEdited )
{
    Boolean handled = false;
    AppGlobalType	*global;

    global = AppGetGlobal();
    *exit = false;

    if ( eventP->eType == ctlSelectEvent )
    {
        switch ( eventP->data.ctlSelect.controlID )
        {
        case ExportOptionsOKBton:
            ExportApply ( frmP, *category );
            *exit = true;
            handled = true;
            break;

        case ExportOptionsCancelBton:
            *exit = true;
            handled = true;
            break;

        case ExportOptionsCategoryPopupTriger:
            *categoryEdited = ExportSelectCategory ( dbP, category ) || *categoryEdited;
            handled = true;
            break;

        case ExportOptionsSugarMemo:
        case ExportOptionsSuperMemo:
            FrmHideObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateCheckBox) );
            FrmHideObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateLabel) );
            FrmHideObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryPopupTriger) );
            FrmHideObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryLabel) );
            break;
        
        case ExportOptionsMemoPad:
            FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateCheckBox) );
            FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsPrivateLabel) );
            FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryPopupTriger) );
            FrmShowObject ( frmP, FrmGetObjectIndex (frmP, ExportOptionsCategoryLabel) );
            break;

        }
    }

    return ( handled );
}

/***********************************************************************
 *
 * FUNCTION:    ExportPopupDialog
 *
 * DESCRIPTION: This routine popup "Export Options" dialog
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/

Err ExportPopupDialog( void )
{
    EventType	event;
    FormPtr	frmP, originalForm;
    Boolean	exit, handled;
    DmOpenRef	dbP;
    UInt16	category;
    Boolean	categoryEdited;
    AppGlobalType	*global;

    global = AppGetGlobal();

    // Open memo pad database.
    dbP = DmOpenDatabaseByTypeCreator ( 'DATA', sysFileCMemo, dmModeReadWrite );
    if ( dbP == NULL )
        return DmGetLastErr();
    category = global->prefs.exportCagetoryIndex;
    categoryEdited = false;

    // Remember the original form
    originalForm = FrmGetActiveForm();

    frmP = FrmInitForm ( ExportOptionsForm );
    FrmSetActiveForm( frmP );
    ExportInit( frmP, dbP );
    FrmDrawForm ( frmP );

    do
    {
        EvtGetEvent( &event, evtWaitForever );

        if ( SysHandleEvent ( &event ) )
            continue;

        if ( event.eType == appStopEvent )
            EvtAddEventToQueue( &event );

        handled = ExportHandleEvent( frmP, &event, dbP, &exit, &category, &categoryEdited );

        // Check if the form can handle the event
        if ( !handled )
            FrmHandleEvent ( frmP, &event );

    }
    while ( event.eType != appStopEvent && !exit );


    FrmEraseForm ( frmP );
    FrmDeleteForm ( frmP );
    FrmSetActiveForm ( originalForm );

    if ( dbP != NULL )
        DmCloseDatabase( dbP );

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:    ExportToMemo
 *
 * DESCRIPTION: This routine export text to memo pad with field id.
 *
 * PARAMETERS:  -> fieldID Field resource id.
 *
 * RETURNED:    errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Set/04	Initial Revision
 *
 ***********************************************************************/

Err ExportToMemo(Char	* str)
{
    Int16	size;
    //FieldType	*fieldP;
    DmOpenRef	dbP;
    MemHandle	recordH;
    Char	*recordP, tail;
    UInt16	index, attr;//startPosition, endPosition, 
    Err	err;
    AppGlobalType	*global;

    global = AppGetGlobal();

    attr = dmUnfiledCategory;
    tail = chrNull;
    err = errNone;
    dbP = NULL;
    do
    {
        dbP = DmOpenDatabaseByTypeCreator ( 'DATA', sysFileCMemo, dmModeReadWrite );
        if ( dbP == NULL )
        {
            err = DmGetLastErr();
            break;
        }
        if ( str == NULL || *str == chrNull )
            break;
            
        // if use select some text then we only export select text.
        /*FldGetSelection ( fieldP, &startPosition, &endPosition );
        if ( startPosition == endPosition )
        {
            startPosition = 0;
            size = StrLen( str );
        }
        else
        {
            size = endPosition - startPosition;
        }*/
        size = StrLen (str);

        // Add a new record at the end of the database.
        index = DmNumRecords ( dbP );
        recordH = DmNewRecord ( dbP, &index, size + 1 );

        // If the allocate failed, display a warning.
        if ( ! recordH )
        {
            FrmAlert ( DeviceFullAlert );
            err = ~errNone;
            break;
        }

        // Pack the the data into the new record.
        recordP = MemHandleLock ( recordH );
        DmWrite( recordP, 0, str, size );
        DmWrite( recordP, size, &tail, sizeof( tail ) );
        MemPtrUnlock ( recordP );

        // Set the category of the new record to the current category.
        DmRecordInfo ( dbP, index, &attr, NULL, NULL );

        // Get the attrib of new record.
        attr &= ~dmRecAttrCategoryMask;
        attr |= global->prefs.exportCagetoryIndex;

        if ( global->prefs.exportPrivate )
            attr |= dmRecAttrSecret;
        else
            attr &= ~dmRecAttrSecret;

        DmSetRecordInfo ( dbP, index, &attr, NULL );
        DmReleaseRecord ( dbP, index, true );

    }
    while ( false );

    if ( dbP != NULL )
        DmCloseDatabase( dbP );


    return err;
}

#pragma mark ----

/***********************************************************************
 *
 * FUNCTION:    SugarMemoGetDatabase
 *
 * DESCRIPTION: Get the application's database.  Open the database if it
 *              exists, create it if neccessary.
 *
 * PARAMETERS:  *dbPP - pointer to a database ref (DmOpenRef) to be set
 *              mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:    errNone if success else fail
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	15/Jan/05	Initial Revision
*
 ***********************************************************************/
static Err GetDatabase ( DmOpenRef *dbPP, UInt16 mode,  UInt16 command)
{
    MemHandle dbNameH;
    Char *dbNameP;

    MemHandle dbImageH;
    MemPtr dbImageP;

    DmOpenRef dbP;
    LocalID dbID;
    
    DmResID dbNameID, dbImageID;
    Boolean newDB = false;

    Err err;

    dbNameH = 0;
    dbNameP = NULL;
    dbImageH = 0;
    dbImageP = NULL;
    dbP = NULL;
    *dbPP = NULL;
    err = errNone;

    if( command == OptionsExportSugarMemo )
    {
    	dbNameID = SugarMemoDBName;
    	dbImageID = SugarMemoDefaultDBImage;
    }
    else
    {
    	dbNameID = SuperMemoDBName;
    	dbImageID = SuperMemoDefaultDBImage;
    }
    // Get database dbID.
    dbNameH = DmGetResource( strRsc, dbNameID );
    dbNameP = MemHandleLock( dbNameH );

    dbID = DmFindDatabase ( 0, dbNameP );
    if ( dbID == 0 )
    {
        // Create default database by image.
        dbImageH = DmGetResource( 'data', dbImageID );
        dbImageP = MemHandleLock( dbImageH );
        err = DmCreateDatabaseFromImage ( dbImageP );
        if ( err != errNone )
            goto exit;

        dbID = DmFindDatabase ( 0, dbNameP );
        if ( dbID == 0 )
        {
            err = DmGetLastErr();
            goto exit;
        }
        newDB = true;
    }

    // Open database by dbID
    dbP = DmOpenDatabase ( 0, dbID, mode );
    if ( dbP == NULL )
    {
        err = DmGetLastErr();
        goto exit;
    }

    *dbPP = dbP;
    
    if ( newDB && command == OptionsExportSuperMemo )// Update supermemo create date
    {
		DateType today;
		MemHandle recordH; 
    	MemPtr recordP;        
		
        recordH = DmGetRecord (dbP, 0); 
		recordP = MemHandleLock ( recordH );
		DateSecondsToDate (TimGetSeconds(), &today);
	    DmWrite( recordP, 8, &DateToInt(today), 2 );		
		MemPtrUnlock ( recordP );
		DmReleaseRecord ( dbP, 0, true );
	}

exit:
    if ( dbNameH != 0 )
    {
        MemHandleUnlock( dbNameH );
        DmReleaseResource( dbNameH );
    }

    if ( dbImageH != 0 )
    {
        MemHandleUnlock( dbImageH );
        DmReleaseResource( dbImageH );
    }

    return err;
}

/***********************************************************************
 *
 * FUNCTION:    ExportToSu(ga|pe)rMemo
 *
 * DESCRIPTION: This routine export text to Su(ga|pe)rMemo with field id.
 *
 * PARAMETERS:  -> fieldID Field resource id.
 *
 * RETURNED:    errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	07/Sep/04	Initial Revision
 *		osfans			08/Nov/15	SuperMemo Revision
 *
 ***********************************************************************/
Err ExportToSMemo(Char *str, UInt16 command)
{
    MemHandle recordH; 
    MemPtr recordP;
    Int16	size, sizemore=0;
    DmOpenRef	dbP;
    UInt16 index, attr, recHeadSize;
    UInt16 chrNullOffset[ 3 ], i, j, offset;
    WChar ch;
    Char tail = chrNull;
    Err	err;
    AppGlobalType	*global;

    global = AppGetGlobal();

    attr = dmUnfiledCategory;
    tail = chrNull;
    err = errNone;
    dbP = NULL;
    do
    {
        // Get SugarMemo database, if it is not exist then create it.
        err = GetDatabase ( &dbP, dmModeReadWrite, command );
        if ( err != errNone || dbP == NULL )
            break;

        // Get text that will exported.
        /*fieldP = GetObjectPtr ( fieldID );
        if ( fieldP == NULL )
            break;

        str = FldGetTextPtr( fieldP );*/
        if ( str == NULL || *str == chrNull )
            break;
        size = StrLen( str );

        // Get all chrNull offset.
        // there have four fields in a SurgarMemo record.
        // but we only use three fields, the fourth field
        // always be empty.
        // j < 3 means we only used three filed.
        // field #1: keyword <-chrNullOffset[0]
        // field #2: phonetic
        // field #3: explain
        // field #4: note.
        i = j = 0;
        while ( i < size && j < 3 )
        {
            offset = TxtGetNextChar( str, i, &ch );
            if ( ch == chrNull || ch == chrLineFeed )
            {
            	if( j == 0 ){
            		chrNullOffset[ 0 ] = i;
            		str[i] = chrNull;     		
            		if (global->phonetic[0])
            		{	
            			chrNullOffset[ 1 ] = StrLen(global->phonetic);
            			i = chrNullOffset[ 1 ] ;      			
            		}
            		else
            		{
            			chrNullOffset[ 1 ] = i;
            			sizemore++;
            		}            		
            		j = 2;
            	}
            	else
            	{
            		if (StrNCompare( str + i, "\n\n\n", 3) == 0)// field #3\n\n\nfield #4
            		{	
            			str[i] = chrNull; 
            			chrNullOffset[ j++ ] = i;
            			sizemore -= 2;
            		}
            	}
            }
            i += offset;
        }

        if ( j < 3 )
        {
            chrNullOffset[ j++ ] = size;
            sizemore++;
        }
        sizemore++;

        // Get SugarMemo record Head.
        if( command == OptionsExportSugarMemo )
        {
			typedef struct
			{
			    UInt8 recHead[ 25 ];
			    UInt16 keywordLen;
			}
			SugarMemoRecHeadType;
			SugarMemoRecHeadType *recHeadImageP, recHead;
			MemHandle recHeadImageH;
			
    		recHeadSize = sizeof( SugarMemoRecHeadType );	        
	        recHeadImageH = DmGetResource( 'data', SugarMemoDefaultRecordHead );
	        recHeadImageP = ( SugarMemoRecHeadType * ) MemHandleLock( recHeadImageH );
	        recHead = *recHeadImageP;
	        DateSecondsToDate (TimGetSeconds(), (DateType *)(&recHead.recHead[22]));//today
	        recHead.keywordLen = chrNullOffset[ 1 ] + 1;
	        	    
	        MemHandleUnlock( recHeadImageH );
	        DmReleaseResource( recHeadImageH );

	        // Add a new record at the end of the database.
	        index = DmNumRecords ( dbP );
	        recordH = DmNewRecord ( dbP, &index, recHeadSize + size + sizemore );

	        // If the allocate failed, display a warning.
	        if ( ! recordH )
	        {
	            FrmAlert ( DeviceFullAlert );
	            err = ~errNone;
	            break;
	        }

	        // Pack the the data into the new record.
	        recordP = MemHandleLock ( recordH );        
	        
	        DmWrite( recordP, 0, &recHead, recHeadSize ); // record head,(remember data)
	        offset = recHeadSize;   
        }
        else
        {   		
    		UInt16 recHead[10]={0x2, 0xffff};
    		UInt16 indexSize;	        
    		
	        index = DmNumRecords ( dbP );	        
	               
	        recordH = DmQueryRecord (dbP, 15);
	        indexSize = MemHandleSize ( recordH );
	        DmReleaseRecord ( dbP, 15, true );
	        
	        //append new record index 
	        recordH = DmResizeRecord (dbP, 15, indexSize+2);
	        recordP = MemHandleLock (recordH);						
			DmWrite( recordP, indexSize, &index, 2);
			MemPtrUnlock ( recordP );
			DmReleaseRecord ( dbP, 15, true );
			
    		sizemore += 2;
    		DateSecondsToDate (TimGetSeconds(), (DateType *)(&recHead[3]));
    		recHeadSize = sizeof(recHead);       
			 
			// Add a new record at the end of the database.
	        recordH = DmNewRecord ( dbP, &index, recHeadSize + size + sizemore );

	        // If the allocate failed, display a warning.
	        if ( ! recordH )
	        {
	            FrmAlert ( DeviceFullAlert );
	            err = ~errNone;
	            break;
	        }

	        // Pack the the data into the new record.
	        recordP = MemHandleLock ( recordH );	        
	        DmWrite( recordP, 0, recHead, recHeadSize ); // record head,(remember data)
	        offset = recHeadSize;	        
	    }	            
        
    	DmWrite( recordP, offset, str, chrNullOffset[ 0 ] + 1);    // record field #1\0
    	offset += chrNullOffset[ 0 ] + 1;
    	
        if(global->phonetic[0])
        {
        	DmWrite( recordP, offset, &str[chrNullOffset[ 0 ] + 1], chrNullOffset[ 1 ] - chrNullOffset[ 0 ] - 1);    // record field #2
        	offset += chrNullOffset[ 1 ] - chrNullOffset[ 0 ] - 1;
        }
    	DmWrite( recordP, offset, &tail, sizeof( tail ) );  // end of field #2
    	offset += sizeof( tail );       
        
    	DmWrite( recordP, offset, &str[chrNullOffset[ 1 ] + 1], chrNullOffset[ 2 ] - chrNullOffset[ 1 ] );    // record field #3\0
    	offset += chrNullOffset[ 2 ] - chrNullOffset[ 1 ];
    	
        if(chrNullOffset[ 2 ] == size)// record field #4\0
        {
        	DmWrite( recordP, offset, &tail, sizeof( tail ) );
        	offset+=sizeof( tail);
        }
        else
        {
        	DmWrite( recordP, offset, &str[chrNullOffset[ 2 ] + 3], size - chrNullOffset[ 2 ] - 2 );
        	offset+=size - chrNullOffset[ 2 ] - 2 ;
        }
        	
        if( command == OptionsExportSuperMemo )
        {
        	DmWrite( recordP,  offset, &tail, sizeof( tail) ); //field #5
        	offset+=sizeof( tail);
	    	DmWrite( recordP,  offset, &tail, sizeof( tail) ); //field #6	
        }        
        else if ( global->phonetic[0] )// Now translate GMX phonetic to SugarMemo phonetic.
        {
            MemHandle transH;
            Char *trans;
            
            str = (Char *)recordP + recHeadSize + chrNullOffset[ 0 ] + 1;
            transH = DmGetResource( strRsc, sugarMemo2GMXString );
            trans = ( Char * ) MemHandleLock( transH );

            // seek  phonetic string.
            i = 0;
            while ( true )
            {
                offset = TxtGetNextChar( str, i, &ch );
                if ( ch == chrNull )
                    break;
                
                // Seek the char in translate map.
                j = 0;
                while ( ( WChar ) * ( trans + j ) != chrNull && ( WChar ) * ( trans + j + 1 ) != ch )
                {
                    j++;
                    j++;
                }

                // Ok, we find the char in translate map, so translate to SugarMemo from GMX.
                if ( ( WChar ) * ( trans + j ) != chrNull && ( WChar ) * ( trans + j + 1 ) == ch )
                {
                    DmWrite( recordP, recHeadSize + chrNullOffset[ 0 ] + i + 1, trans + j, sizeof(Char) );
                }
                // The phonetic of sugarmemo must lager than space character.
                else if ( ch < chrSpace )
                {
                    Char c = chrSpace;
                    DmWrite( recordP, recHeadSize + chrNullOffset[ 0 ] + i + 1, &c, sizeof(Char) );
                }
                
                i += offset;
            }

            MemHandleUnlock( transH );
            DmReleaseResource( transH );
        }
        
        MemPtrUnlock ( recordP );

        // Get the attrib of new record and set it to unfiled and no secret.
        //DmRecordInfo ( dbP, index, &attr, NULL, NULL );
        //attr &= ~dmRecAttrCategoryMask;
        //attr &= ~dmRecAttrSecret;

        //DmSetRecordInfo ( dbP, index, &attr, NULL );
        DmReleaseRecord ( dbP, index, true );
        
    }
    while ( false );

    if ( dbP != NULL )
        DmCloseDatabase( dbP );


    return err;
}
