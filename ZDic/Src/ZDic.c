/*
* ZDic.c
*
* main file for ZDic
*
* This wizard-generated code is based on code adapted from the
* stationery files distributed as part of the Palm OS SDK 4.0.
*
* Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
* All rights reserved.
*/

#include <PalmOS.h>
#include <PalmOSGlue.h>
#include <CtlGlue.h>
#include <LstGlue.h>        // for LstGlueSetFont
#include <Window.h>         // for WinScreenGetAttribute
#include <SonyChars.h>
#include <PalmOneNavigator.h>	// for 5-way Navigator

#include "ZDic.h"
#include "ZDicConfig.h"
#include "ZDicDB.h"
#include "ZDicDIA.h"
#include "ZDicTools.h"
#include "ZDicExport.h"
#include "ZDicRegister.h"
#include "ZDicVoice.h"

#include "ZDic_Rsc.h"
#include "Decode.h"

/*********************************************************************
 * Entry Points
 *********************************************************************/

/*********************************************************************
 * Global variables
 *********************************************************************/

static Err ToolsPlayVoice ( void );
static Err PrefFormPopupPrefsDialog( void );
static Int16 ZDicDBInitDictList( UInt32 type, UInt32 creator );
static Boolean PrefFormHandleEvent( FormType *frmP, EventType *eventP, Boolean *exit );
static Boolean DAFormHandleEvent( EventType * eventP );
static Err MainFormSearch( Boolean putinHistory, Boolean updateWordList, Boolean highlightWordField, Boolean updateDescField, Boolean bEnableBreak, Boolean bEnableAutoSpeech );
static Err MainFormChangeWordFieldCase( void );
static Err MainFormUpdateWordList( void );
static void MainFormWordListUseAble( Boolean turnOver, Boolean redraw );
static Boolean MainFormHandleEvent( EventType * eventP );


/*********************************************************************
 * Internal Constants
 *********************************************************************/

/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

//#pragma mark -
/*********************************************************************
 * Internal Functions
 *********************************************************************/

/*
 *
 * FUNCTION:    HideObject
 *
 * DESCRIPTION: This routine set an object not-usable and erases it
 *              if the form it is in is visible.
 *
 * PARAMETERS:  frm      - pointer to a form
 *              objectID - id of the object to set not usable
 *
 * RETURNED:    nothing
 *
 *
 */
static void HideObject ( const FormPtr frm, UInt16 objectID )
{

    FrmHideObject ( frm, FrmGetObjectIndex ( frm, objectID ) );
}

/*
 *
 * FUNCTION:    ShowObject
 *
 * DESCRIPTION: This routine set an object usable and draws the object if
 *              the form it is in is visible.
 *
 * PARAMETERS:  frm      - pointer to a form
 *              objectID - id of the object to set usable
 *
 * RETURNED:    nothing
 *
 *
 */
static void ShowObject ( const FormPtr frm, UInt16 objectID )
{
    FrmShowObject ( frm, FrmGetObjectIndex ( frm, objectID ) );
}

/*
 *
 * FUNCTION:    IsInside
 *
 * DESCRIPTION: Check the point whether in the rectangle range.
 *
 * RETURNED:    true if inside else false
 *
 *
 */

static Boolean IsInside( RectanglePtr r, UInt16 x, UInt16 y )
{
    return ( 0 <= x && x <= r->extent.x && 0 <= y && y <= r->extent.y );
}

/*
 *
 * FUNCTION:    IsOutside
 *
 * DESCRIPTION: Check the point whether out side the rectangle range.
 *
 * RETURNED:    true if outside else true
 *
 *
 */

static Boolean IsOutside( RectanglePtr r, UInt16 x, UInt16 y )
{
    return ( !IsInside( r, x, y ) );
}

#pragma mark -

/***********************************************************************
 *
 * FUNCTION:	ToolsQuitApp
 *
 * DESCRIPTION: Send appStopEvent to event queue to stop application.
 *
 * PARAMETERS:
 *				nothing.
 *
 * RETURN:
 *				nothing
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static void ToolsQuitApp( void )
{
    EventType	newEvent;

    MemSet( &newEvent, sizeof( newEvent ), 0 );
    newEvent.eType = appStopEvent;
    EvtAddEventToQueue( &newEvent );
    return ;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsFormatExplain
 *
 * DESCRIPTION: Format explain for easy display.
 *
 * PARAMETERS:
 *				->	p Pointer of explain string.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err ToolsFormatExplain( UInt8 *p )
{
    UInt16	i, j;

    if ( p == NULL )
        return errNone;

    // get key word.
    i = 0;
    while ( p[ i ] != chrHorizontalTabulation && p[ i ] != chrNull )
        i++;
    if ( p[ i ] == chrHorizontalTabulation )
        p[ i++ ] = chrLineFeed;

    // get explain, translate "\n" to charLineFeed;
    j = i;
    while ( p[ i ] != chrNull )
    {
        if ( p[ i ] == '\\' && p[ i + 1 ] == chrSmall_N )
        {
            if ( j != 0 )
                p[ j++ ] = chrLineFeed;
            i++;
        }
        else
        {
            if ( i != j )
                p[ j ] = p[ i ];
            j++;
        }

        i++;
    }
    p[ j ] = chrNull;

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsGMXTranslate
 *
 * DESCRIPTION: This routine translate lazy or efan phonetic to GMX phonetic.
 *
 * PARAMETERS:
 *				->	p Pointer of explain string.
 *				->	index Source phonetic index.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err ToolsGMXTranslate( Char *p, phoneticEnum index )
{
    MemHandle transH;
    Char *trans;
    UInt16	i, j, offset, strID;
    WChar ch;

    if ( index == eGmxPhonetic || index == eNonePhonetic || p == NULL )
        return errNone;

    // get GMX to efan phonetic table and translate it.
    switch ( index )
    {
    case eEfanPhonetic:
        strID = efan2GMXString;
        break;
    case eLazywormPhonetic:
        strID = lazy2GMXString;
        break;
    case eMutantPhonetic:
        strID = mutant2GMXString;
        break;
    default:
        return errNone;
    }

    transH = DmGetResource( strRsc, strID );
    if ( transH == NULL )
    {
        return DmGetLastErr();
    }

    trans = ( Char * ) MemHandleLock( transH );

    // seek  phonetic string.
    i = 0;

NextPhoneticStr:
    while ( true )
    {
        offset = TxtGetNextChar( p, i, &ch );
        if ( ch == chrNull
                || ch == chrSolidus
                || ch == chrLeftSquareBracket
                || ch == chrLessThanSign )
            break;

        i += offset;
    }

    if ( ch == chrSolidus || ch == chrLeftSquareBracket || ch == chrLessThanSign )
    {
        i += offset;
        offset = TxtGetNextChar( p, i, &ch );

        while ( ch != chrSolidus && ch != chrRightSquareBracket
                && ch != chrGreaterThanSign && ch != chrNull )
        {
            j = 0;
            while ( ( WChar ) * ( trans + j ) != ch && ( WChar ) * ( trans + j ) != chrNull )
            {
                j++;
                j++;
            }

            if ( ( WChar ) * ( trans + j ) == ch )
                TxtSetNextChar( p, i, ( WChar ) * ( trans + j + 1 ) );

            i += offset;
            offset = TxtGetNextChar( p, i, &ch );
        }

        if ( ch != chrNull )
        {
            i += offset;
            goto NextPhoneticStr;
        }
    }

    MemHandleUnlock( transH );
    DmReleaseResource( transH );

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsSetFieldHandle
 *
 * DESCRIPTION:	Set new handle for a filed by id and free old handle.
 *
 * PARAMETERS:	->	objID Field resource id.
 *				->	newTxtH New handle that should be set the field.
 *				->	redraw True if need to redraw the field.
 *
 * RETURNED:	errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static Err ToolsSetFieldHandle( UInt16 objID, MemHandle newTxtH, Boolean redraw )
{
    FieldType * field;
    MemHandle oldTxtH;

    field = ( FieldType * ) GetObjectPtr( objID );

    // change the text and update the display
    oldTxtH = FldGetTextHandle( field );
    FldSetTextHandle( field, newTxtH );

    // Reset insert point.
    FldSetInsertionPoint ( field, 0 );

    if ( redraw )
        FldDrawField( field );

    // free the old text handle
    if ( oldTxtH != NULL )
        MemHandleFree( oldTxtH );

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsSetFieldPtr
 *
 * DESCRIPTION:	Set new text for a filed by id and free old handle.
 *
 * PARAMETERS:	->	objID Field resource id.
 *				->	text Pointer of the sting.
 *				->	textLen Length of the string.
 *				->	redraw True if need to redraw the field.
 *
 * RETURNED:	errNone if success else fail.
 *
 * Note:		we must use ToolsSetFieldPtr to replace FldInsert,
 *				because ToolsSetFieldPtr sets the field's dirty attribute and
 *				posts a fldChangedEvent to the event queue. But we
 *				have a search when receive a fldChangedEvent.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static Err ToolsSetFieldPtr( UInt16 objID, const Char *text, Int16 textLen, Boolean redraw )
{
    MemHandle	bufH;
    Char	*str;

    bufH = MemHandleNew( textLen + 1 );
    if ( bufH == NULL )
        return memErrNotEnoughSpace;

    // Build new text handle.
    str = MemHandleLock( bufH );
    MemMove( str, text, textLen );
    str[ textLen ] = chrNull;
    MemHandleUnlock( bufH );

    // Set new handle.
    ToolsSetFieldHandle( objID, bufH, redraw );

    return errNone;
}

/*
 * FUNCTION: ToolsHighlightField
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static void ToolsHighlightField( UInt16 fieldID )
{
    Char * wordStr;
    FieldType *field;

    // select all the text in word field, then use can clear it easy.
    field = GetObjectPtr( fieldID );
    wordStr = FldGetTextPtr ( field );

    if ( wordStr != NULL && *wordStr != chrNull )
    {
        // Reset insert point. and select it all.
        FldSetInsertionPoint ( field, 0 );
        FldSetSelection ( field, 0, StrLen( wordStr ) );
    }
}


/***********************************************************************
 *
 * FUNCTION:	ToolsUpdateScrollBar
 *
 * DESCRIPTION:	Update scroll bar by special field.
 *
 * PARAMETERS:	->	objID Field resource id.
 *				->	scrollBarID Scrollbar resource id.
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static void ToolsUpdateScrollBar ( UInt16 fieldID, UInt16 scrollBarID )
{
    UInt16 scrollPos;
    UInt16 textHeight;
    UInt16 fieldHeight;
    Int16 maxValue;
    FieldPtr fld;
    ScrollBarPtr bar;

    fld = GetObjectPtr ( fieldID );
    bar = GetObjectPtr ( scrollBarID );

    FldGetScrollValues ( fld, &scrollPos, &textHeight, &fieldHeight );

    if ( textHeight > fieldHeight )
    {
        // On occasion, such as after deleting a multi-line selection of text,
        // the display might be the last few lines of a field followed by some
        // blank lines.  To keep the current position in place and allow the user
        // to "gracefully" scroll out of the blank area, the number of blank lines
        // visible needs to be added to max value.  Otherwise the scroll position
        // may be greater than maxValue, get pinned to maxvalue in SclSetScrollBar
        // resulting in the scroll bar and the display being out of sync.
        maxValue = ( textHeight - fieldHeight ) + FldGetNumberOfBlankLines ( fld );
    }
    else if ( scrollPos )
        maxValue = scrollPos;
    else
        maxValue = 0;

    SclSetScrollBar ( bar, scrollPos, 0, maxValue, fieldHeight - 1 );
}

/***********************************************************************
 *
 * FUNCTION:	ToolsScroll
 *
 * DESCRIPTION:	Scroll the field and update the scrollbar.
 *
 * PARAMETERS:	->	linesToScroll Number of scroll line.
 *				->	updateScrollbar True if update the scrollbar else not.
 *				->	objID Field resource id.
 *				->	scrollBarID Scrollbar resource id.
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static void ToolsScroll ( Int16 linesToScroll, Boolean updateScrollbar, UInt16 fieldID, UInt16 scrollBarID )
{
    UInt16	blankLines;
    FieldPtr	fld;

    fld = GetObjectPtr ( fieldID );
    blankLines = FldGetNumberOfBlankLines ( fld );

    if ( linesToScroll < 0 )
        FldScrollField ( fld, -linesToScroll, winUp );
    else if ( linesToScroll > 0 )
        FldScrollField ( fld, linesToScroll, winDown );

    // If there were blank lines visible at the end of the field
    // then we need to update the scroll bar.
    if ( blankLines || updateScrollbar )
    {
        ToolsUpdateScrollBar( fieldID, scrollBarID );
    }
}

/***********************************************************************
 *
 * FUNCTION:	ToolsScrollWord
 *
 * DESCRIPTION:	scroll to next/prev word.
 *
 * PARAMETERS:	->	direction Direction of page scroll.
 *				->	objID Field resource id.
 *				->	scrollBarID Scrollbar resource id.
 *				->  inputFieldID Input field resource id.
 *				->	highlightWordField True if highlight inputFieldID.
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static void ToolsScrollWord ( WinDirectionType direction, UInt16 fieldID,
                              UInt16 scrollBarID, UInt16 inputFieldID, UInt16 playerBtonID, Boolean highlightWordField )
{
    UInt8	* explainPtr;
    UInt32	explainLen;
    ZDicDBDictInfoType	*dictInfoP;
    AppGlobalType	*global;
    Err	err;

    global = AppGetGlobal();
    dictInfoP = &global->prefs.dictInfo;

    if ( direction == winUp )
    {
        err = ZDicLookupBackward( &explainPtr, &explainLen, &global->descript );

    }
    else
    {
        err = ZDicLookupForward( &explainPtr, &explainLen, &global->descript );
    }

    if ( err == errNone )
    {
        FormType * frmP;
        MemHandle	bufH;
        Char	*str;
        Int16	len;

        bufH = MemHandleNew( explainLen + 1 );
        if ( bufH == NULL )
            return ;

        // format string.
        str = MemHandleLock( bufH );
        MemMove( str, explainPtr, explainLen );
        str[ explainLen ] = chrNull;
        ToolsFormatExplain( ( UInt8* ) str );
        if ( dictInfoP->phonetic[ dictInfoP->curMainDictIndex ] != 0 )
            ToolsGMXTranslate( str, dictInfoP->phonetic[ dictInfoP->curMainDictIndex ] );
        MemHandleUnlock( bufH );

        // update display.
        ToolsSetFieldHandle( fieldID, bufH, true );
        ToolsUpdateScrollBar ( fieldID, scrollBarID );

        // update input field
        // get key word.
        len = 0;
        while ( explainPtr[ len ] != chrHorizontalTabulation && explainPtr[ len ] != chrNull && len < MAX_WORD_LEN )
        {
            global->data.readBuf[ len ] = explainPtr[ len ];
            len++;
        }
        global->data.readBuf[ len ] = chrNull;
        ToolsSetFieldPtr( inputFieldID, ( char * ) global->data.readBuf, len, true );

        if ( highlightWordField )
            ToolsHighlightField( inputFieldID );

        // Display or hide the player button
        frmP = FrmGetActiveForm ();
        if ( ZDicVoiceIsExist ( ( Char * ) explainPtr ) )
        {
            ShowObject ( frmP, playerBtonID );
            if ( global->prefs.enableAutoSpeech )
                ToolsPlayVoice ();
        }
        else
            HideObject ( frmP, playerBtonID );



    }
}

/***********************************************************************
 *
 * FUNCTION:	ToolsPageScroll
 *
 * DESCRIPTION:	Page scroll the field and update the scrollbar.
 *
 * PARAMETERS:	->	direction Direction of page scroll.
 *				->	objID Field resource id.
 *				->	scrollBarID Scrollbar resource id.
 *				->  inputFieldID Input field resource id.
 *				->	highlightWordField True if highlight inputFieldID.
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static void ToolsPageScroll ( WinDirectionType direction, UInt16 fieldID,
                              UInt16 scrollBarID, UInt16 inputFieldID, UInt16 playerBtonID, Boolean highlightWordField )
{
    UInt16	linesToScroll;
    FieldPtr	fld;

    fld = GetObjectPtr ( fieldID );

    if ( FldScrollable ( fld, direction ) )
    {
        linesToScroll = FldGetVisibleLines ( fld ) - 1;

        if ( direction == winUp )
            linesToScroll = -linesToScroll;

        ToolsScroll( linesToScroll, true, fieldID, scrollBarID );

        return ;
    }

    ToolsScrollWord ( direction, fieldID, scrollBarID, inputFieldID, playerBtonID, highlightWordField );

    return ;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsChangeCase
 *
 * DESCRIPTION: This routine change the string case.
 *
 * PARAMETERS:
 *				<->	newStr Pass the pointer of string and ret result.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err ToolsChangeCase( Char* newStr )
{
    Int16 strLen, i;
    UInt16 offset;
    WChar ch;

    // if it is not ascii sting then do not change case.
    i = 0;
    while ( true )
    {
        offset = TxtGetNextChar( newStr, i, &ch );
        if ( ch == chrNull )
            break;

        if ( offset != 1 )
            return ~errNone;

        i += offset;
    }

    strLen = StrLen( newStr );

    if ( TxtCharIsLower ( newStr[ 0 ] ) )
    {
        // "word" -> "WORD"
        for ( i = 0; i < strLen; i++ )
        {
            if ( TxtCharIsLower( newStr[ i ] ) )
                newStr[ i ] -= 'a' - 'A';
        }
    }
    else if ( TxtCharIsUpper ( newStr[ 0 ] ) )
    {
        if ( strLen > 1 && TxtCharIsLower ( newStr[ 1 ] ) )
        {
            // "Word" -> "word"
            StrToLower ( newStr, newStr );
        }
        else
        {
            if ( strLen > 1 )
            {
                // "WORD" -> "Word"
                for ( i = 1; i < strLen; i++ )
                {
                    if ( TxtCharIsUpper( newStr[ i ] ) )
                        newStr[ i ] += 'a' - 'A';
                }
            }
            else
            {
                // "W" -> "w"
                if ( TxtCharIsUpper( newStr[ 0 ] ) )
                    newStr[ 0 ] += 'a' - 'A';
            }
        }
    }
    else
        return ~errNone;

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsWordNotYuanYin
 *
 * DESCRIPTION: This routine change the string by word assay.
 *
 * PARAMETERS:
 *				<->	newStr Pass the pointer of string and ret result.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/
__inline static Int16 ToolsWordSameAs( const Char *s1, const Char *s2 )
{
    Int16	idx;

    // compare key string.
    idx = 0;
    while ( *( s1 + idx ) == *( s2 + idx )
            && *( s1 + idx ) != chrNull )
    {
        idx++;
    }

    return *( s1 + idx ) - *( s2 + idx );
}

__inline static Boolean ToolsWordNotYuanYin ( Char c )
{
    return ( c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' );
}

static Err ToolsWordAssay ( Char* newStr )
{
    Int16 tail;
    Boolean err = ~errNone;

    tail = StrLen ( newStr );
    if ( tail <= 4 )
        return false;

    // n.
    if ( newStr[ tail - 1 ] == 's' )
    {
        // fu yin + "oes" -> fu yin + "o" : "tomatoes" -> "tomato"
        if ( ToolsWordSameAs ( &newStr[ tail - 3 ], "oes" ) == 0
                && ToolsWordNotYuanYin ( newStr[ tail - 4 ] ) )
        {
            newStr[ tail - 2 ] = chrNull;
        }

        // fu yin + "ies" -> fu yin + "y": "babies" -> "baby"
        else if ( ToolsWordSameAs ( &newStr[ tail - 3 ], "ies" ) == 0
                  && ToolsWordNotYuanYin ( newStr[ tail - 4 ] ) )
        {
            newStr[ tail - 3 ] = 'y';
            newStr[ tail - 2 ] = chrNull;
        }

        else
        {
            newStr[ tail - 1 ] = chrNull;
        }

        err = errNone;
    }

    // adj/adv : "busy" "busier" "busiest"
    else if ( ToolsWordSameAs ( &newStr[ tail - 3 ], "ier" ) == 0
              && ToolsWordNotYuanYin ( newStr[ tail - 4 ] ) )
    {
        newStr[ tail - 3 ] = 'y';
        newStr[ tail - 2 ] = chrNull;
        err = errNone;
    }
    else if ( ToolsWordSameAs ( &newStr[ tail - 4 ], "iest" ) == 0
              && ToolsWordNotYuanYin ( newStr[ tail - 5 ] ) )
    {
        newStr[ tail - 4 ] = 'y';
        newStr[ tail - 3 ] = chrNull;
        err = errNone;
    }

    // v. + ing
    else if ( ToolsWordSameAs ( &newStr[ tail - 3 ], "ing" ) == 0 )
    {
        newStr[ tail - 3 ] = chrNull;
        err = errNone;
    }

    // v. + ed "selected" -> "select"
    else if ( ToolsWordSameAs ( &newStr[ tail - 2 ], "ed" ) == 0 )
    {
        newStr[ tail - 2 ] = chrNull;
        err = errNone;
    }

    return err;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsSearch
 *
 * DESCRIPTION: This routine search the word that in the special field.
 *
 * PARAMETERS:
 *				->	fieldID Filed resource ID.
 *				<-	matchsP Ret the matchs character number,
 *					LOOK_UP_FULL_PARITY means match nicety.
 *				<-	explainPP Ret the pointer of explain.
 *				<-	explainLenP Ret the explain length.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err ToolsSearch( UInt16 fieldID, UInt16 *matchsP, UInt8 **explainPP, UInt32 *explainLenP )
{
    Char	* wordStr;
    UInt8	*explainPtr;
    UInt32	explainLen;
    UInt16	matchs1, matchs2, matchs3, matchsTemp, len;
    AppGlobalType	*global;
    ZDicDBDictInfoType	*dictInfo;
    UInt8	newStr[ 4 ][ MAX_WORD_LEN + 1 ];
    Boolean	dirty1, dirty2;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;
    *matchsP = 0;
    *explainPP = NULL;
    *explainLenP = 0;

    wordStr = FldGetTextPtr ( GetObjectPtr( fieldID ) );
    if ( wordStr == NULL )
        return ~errNone;

    // switch case search string.
    StrNCopy( ( Char * ) & newStr[ 0 ][ 0 ], ( Char * ) wordStr, MAX_WORD_LEN );
    newStr[ 0 ][ MAX_WORD_LEN ] = chrNull;

    // Clear the tail space.
    len = StrLen ( ( Char * ) & newStr[ 0 ][ 0 ] );
    while ( len > 0 && newStr[ 0 ][ len - 1 ] == chrSpace )
        len--;
    newStr[ 0 ][ len ] = chrNull;

    matchs1 = matchs2 = matchs3 = 0;
    dirty1 = dirty2 = false;

    err = ZDicLookup( &newStr[ 0 ][ 0 ], &matchs1, &explainPtr, &explainLen, &global->descript );
    *matchsP = matchs1;
    len = StrLen( ( Char * ) & newStr[ 0 ][ 0 ] );
    if ( matchs1 == LOOK_UP_FULL_PARITY )
        goto FIND_OUT;

    if ( ToolsWordAssay ( ( Char* ) & newStr[ 0 ][ 0 ] ) == errNone )
    {
        ZDicLookup( &newStr[ 0 ][ 0 ], &matchsTemp, &explainPtr, &explainLen, &global->descript );
        *matchsP = matchsTemp;
        len = StrLen( ( Char * ) & newStr[ 0 ][ 0 ] );
        if ( matchsTemp == LOOK_UP_FULL_PARITY )
            goto FIND_OUT;

        // restore the old word.
        StrNCopy( ( Char * ) & newStr[ 0 ][ 0 ], ( Char * ) wordStr, MAX_WORD_LEN );
        newStr[ 0 ][ MAX_WORD_LEN ] = chrNull;

        // Clear the tail space.
        len = StrLen ( ( Char * ) & newStr[ 0 ][ 0 ] );
        while ( len > 0 && newStr[ 0 ][ len - 1 ] == chrSpace )
            len--;
        newStr[ 0 ][ len ] = chrNull;

        dirty1 = true;
    }

    if ( ( global->prefs.enableTryLowerSearch && matchs1 < len ) || err )
    {
        dirty1 = true;
        StrCopy( ( Char * ) & newStr[ 1 ][ 0 ], ( Char * ) & newStr[ 0 ][ 0 ] );
        err = ToolsChangeCase( ( Char * ) & newStr[ 1 ][ 0 ] );
        if ( err == errNone )
        {
            err = ZDicLookup( ( UInt8* ) & newStr[ 1 ][ 0 ], &matchs2, &explainPtr, &explainLen, &global->descript );
            *matchsP = matchs2;
            if ( matchs2 == LOOK_UP_FULL_PARITY )
                goto FIND_OUT;
            len = StrLen( ( Char * ) & newStr[ 1 ][ 0 ] );
            if ( matchs2 < len || err )
            {
                dirty2 = true;
                StrCopy( ( Char * ) & newStr[ 2 ][ 0 ], ( Char * ) & newStr[ 1 ][ 0 ] );
                err = ToolsChangeCase( ( Char * ) & newStr[ 2 ][ 0 ] );
                if ( err == errNone )
                {
                    err = ZDicLookup( &newStr[ 2 ][ 0 ], &matchs3, &explainPtr, &explainLen, &global->descript );
                    *matchsP = matchs3;
                    if ( matchs3 == LOOK_UP_FULL_PARITY )
                        goto FIND_OUT;
                }
            }
        }
    }

    err = errNone;
    if ( matchs1 >= matchs2 && matchs1 >= matchs3 )
    {
        if ( dirty1 )
            err = ZDicLookup( &newStr[ 0 ][ 0 ], &matchs1, &explainPtr, &explainLen, &global->descript );
        *matchsP = matchs1;
    }
    else if ( matchs2 >= matchs1 && matchs2 >= matchs3 )
    {
        if ( dirty2 )
            err = ZDicLookup( ( UInt8* ) & newStr[ 1 ][ 0 ], &matchs2, &explainPtr, &explainLen, &global->descript );
        *matchsP = matchs2;
    }
    else
    {
        // it always not dirty, so we do not search again.
        *matchsP = matchs3;
    }

FIND_OUT:
    *explainPP = explainPtr;
    *explainLenP = explainLen;

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsMemAppend
 *
 * DESCRIPTION: This routine append a stirng to the end of memory block.
 *
 * PARAMETERS:  -> srcHPtr Pointer of the memory handle.
 *				-> str String pointer.
 *				-> strLen String length.
 *				-> phoneticIdx Index of phonetic.
 *
 * RETURNED:    errNone if no error
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
static Err ToolsAppendExplain( MemHandle *srcHPtr, UInt8 *str, Int16 strLen, phoneticEnum phoneticIdx )
{
    MemHandle	desH;
    Char	*srcP, *desP;
    UInt32	srcSize, desSize;

    ErrNonFatalDisplayIf ( str == NULL, "Bad parameter" );

    if ( strLen == 0 )
        return errNone;

    if ( *srcHPtr == NULL )
    {
        desH = MemHandleNew( strLen + 1 );
        if ( desH == NULL )
        {
            FrmAlert ( NoEnoughMemoryAlert );
            return memErrNotEnoughSpace;
        }

        desP = MemHandleLock( desH );
        MemMove( desP, str, strLen );
        desP[ strLen ] = chrNull;

        // format string.
        ToolsFormatExplain( ( UInt8* ) desP );
        ToolsGMXTranslate( desP, phoneticIdx );

        MemHandleUnlock( desH );
    }
    else
    {
        srcP = MemHandleLock( *srcHPtr );
        srcSize = StrLen( srcP );
        desSize = srcSize + strLen + 1;

        desH = MemHandleNew( desSize );
        if ( desH == NULL )
        {
            MemHandleUnlock( *srcHPtr );
            FrmAlert ( NoEnoughMemoryAlert );
            return memErrNotEnoughSpace;
        }
        desP = MemHandleLock( desH );

        MemMove( desP, srcP, srcSize );
        MemMove( desP + srcSize, str, strLen );
        desP[ desSize - 1 ] = chrNull;

        // format string.
        ToolsFormatExplain( ( UInt8* ) ( desP + srcSize ) );
        ToolsGMXTranslate( ( desP + srcSize ), phoneticIdx );

        MemHandleUnlock( desH );
        MemHandleUnlock( *srcHPtr );

        MemHandleFree( *srcHPtr );
    }

    *srcHPtr = desH;

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsAllDictionaryCommand
 *
 * DESCRIPTION: Get explain of all dictionary.
 *
 * PARAMETERS:
 *				->	wordFieldID Key word field resource id.
 *				->	descFieldID Description field resource id.
 *				->	scrollBarID Scroll bar resource id.
 *
 * RETURN:
 *				errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err ToolsAllDictionaryCommand( UInt16 wordFieldID,
                                      UInt16 descFieldID, UInt16 scrollBarID )
{
    AppGlobalType	* global;
    ZDicDBDictInfoType	*dictInfo;
    Int16	oldMainDictIndex;
    UInt8	newStr[ MAX_WORD_LEN + 1 ];
    UInt8	dbName[ dmDBNameLength + 4 ];	// +4 for 4 linefreed character.
    Char	*wordStr;
    MemHandle	bufH;
    UInt16	matchs, len;
    UInt8	*explainPtr;
    UInt32	explainLen;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;
    bufH = NULL;

    // Get key string for search.
    wordStr = FldGetTextPtr ( GetObjectPtr( wordFieldID ) );
    if ( wordStr == NULL )
        return errNone;

    StrNCopy( ( Char * ) newStr, wordStr, MAX_WORD_LEN );
    newStr[ MAX_WORD_LEN ] = chrNull;
    len = StrLen ( ( Char * ) newStr );
    if ( len == 0 )
        return errNone;

    // Save current dictioary index and close it.
    oldMainDictIndex = dictInfo->curMainDictIndex;
    ZDicCloseCurrentDict();
    dictInfo->curMainDictIndex = 0;

    // Go through all dictionary and get it explain.
    while ( dictInfo->curMainDictIndex < dictInfo->itemNumber )
    {
        err = ZDicOpenCurrentDict();
        if ( err != errNone )
        {
            ToolsQuitApp();
        }

        // Get dictionary name.
        if ( dictInfo->curMainDictIndex == 0 )
        {
            StrCopy( ( Char* ) dbName, &dictInfo->displayName[ dictInfo->curMainDictIndex ][ 0 ] );
        }
        else
        {
            dbName[ 0 ] = '\n';
            dbName[ 1 ] = '\n';
            StrCopy( ( Char* ) & dbName[ 2 ], &dictInfo->displayName[ dictInfo->curMainDictIndex ][ 0 ] );
        }

        // Append linefreed character at the end of dictionary name.
        StrCat( ( Char* ) dbName, "\n\n" );

        err = ToolsAppendExplain ( &bufH, dbName, StrLen( ( Char* ) dbName ), eNonePhonetic );
        if ( err != errNone )
            break;

        err = ToolsSearch( wordFieldID, &matchs, &explainPtr, &explainLen );
        if ( err == errNone && matchs >= len && explainPtr != NULL && explainLen != 0 )
        {
            err = ToolsAppendExplain ( &bufH, explainPtr, explainLen, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );
            if ( err != errNone )
                break;
        }

        ZDicCloseCurrentDict();
        dictInfo->curMainDictIndex++;
    }


    // Clear the menu status from the display
    MenuEraseStatus( 0 );

    // update display.
    ToolsSetFieldHandle( descFieldID, bufH, true );
    ToolsUpdateScrollBar ( descFieldID, scrollBarID );

    // Restore current dictionary.
    dictInfo->curMainDictIndex = oldMainDictIndex;
    err = ZDicOpenCurrentDict();
    if ( err != errNone )
    {
        ToolsQuitApp();
    }

    // Restore decode buffer.
    err = ToolsSearch( wordFieldID, &matchs, &explainPtr, &explainLen );

    return true;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsChangeDictionaryCommand
 *
 * DESCRIPTION: Change dictionary if use select a dict form menu.
 *
 * PARAMETERS:
 *				->	formId Form ID that need to update.
 *				->	command Menu item id.
 *
 * RETURN:
 *				true if handled else false.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Boolean ToolsChangeDictionaryCommand( UInt16 formId, UInt16 command )
{
    AppGlobalType	* global;
    ZDicDBDictInfoType	*dictInfo;
    Boolean	handled = false;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    if ( command >= ZDIC_DICT_MENUID && command < ZDIC_DICT_MENUID + dictInfo->itemNumber )
    {
        ZDicCloseCurrentDict();
        dictInfo->curMainDictIndex = command - ZDIC_DICT_MENUID;
        FrmUpdateForm( formId, updateDictionaryChanged );

        handled = true;
    }

    return handled;
}
/***********************************************************************
 *
 * FUNCTION:	ToolsCreatDictionarysMenu
 *
 * DESCRIPTION: Initial dictionary popmenu.
 *
 * PARAMETERS:
 *				nothing.
 *
 * RETURN:
 *				nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static void ToolsCreatDictionarysMenu( void )
{

    AppGlobalType	* global;
    ZDicDBDictInfoType	*dictInfo;
    Int16	i;
    UInt16	baseID;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    baseID = DictionarysDictAll;
    for ( i = 0; i < dictInfo->itemNumber /* && i < 9*/; i++ )
    {
        err = MenuAddItem ( baseID, ZDIC_DICT_MENUID + i, '1' + i, &dictInfo->displayName[ i ][ 0 ] );
        baseID = ZDIC_DICT_MENUID + i;
    }

    return ;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetFieldHighlightText
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:	buf	<-> a point of buffer.
 *		size	 -> Buffer size.
 *		nearWord -> true if no highlight text then ret nearest text.
 *
 * RETURNED:	Err - zero if no error, else the error
 *
 *
 ***********************************************************************/
static Err ToolsGetFieldHighlightText( FieldPtr fieldP, Char *buf, UInt32 size, Boolean nearWord )
{
    UInt16 startPosition;
    UInt16 endPosition;
    Char * fieldText;
    Err err = ~errNone;

    if ( fieldP == NULL )
        return err;

    fieldText = FldGetTextPtr ( fieldP );
    if ( fieldText == NULL )
        return err;

    FldGetSelection ( fieldP, &startPosition, &endPosition );
    if ( startPosition == endPosition )
    {
        Boolean foundWord;
        UInt32 start, end;

        if ( !nearWord )
            return err;

        // Get the word that near by insertion position.
        // Is there a word to select? We'll go for the word following the passed
        // offset (better would be to use offset + leading edge flag), and if that
        // doesn't work, check the word _before_ the offset.
        foundWord = TxtWordBounds( fieldText, StrLen( fieldText ), startPosition, &start, &end );
        if ( ( !foundWord ) && ( startPosition > 0 ) )
        {
            UInt16 prevPos = startPosition - TxtPreviousCharSize( fieldText, startPosition );
            foundWord = TxtWordBounds( fieldText, StrLen( fieldText ), prevPos, &start, &end );
        }

        if ( !foundWord )
            return err;

        startPosition = start;
        endPosition = end;
    }

    if ( size != 0 && buf != NULL )
    {
        if ( size >= endPosition - startPosition + 1 )
            size = endPosition - startPosition;
        MemMove( buf, fieldText + startPosition, size );
        MemSet( buf + size, 1, 0 );
    }

    err = errNone;
    return err;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetFormHighlightText
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:	
 *
 * RETURNED:	Err - zero if no error, else the error
 *
 *
 ***********************************************************************/
static Err ToolsGetFormHighlightText( FormPtr frmPtr, Char *buf, UInt32 size )
{
    FieldType	* fieldP;
    TableType	*tableP;
    UInt16	numObjects, focusIdx, i;

    if ( frmPtr == NULL )
        return ~errNone;

    numObjects = FrmGetNumberOfObjects( frmPtr );
    focusIdx = FrmGetFocus ( frmPtr );

    // check focus field at first, if it is nothing then go through all fields.
    i = focusIdx;
    while ( i < numObjects )
    {
        switch ( FrmGetObjectType( frmPtr, i ) )
        {
        case frmFieldObj:
            fieldP = FrmGetObjectPtr( frmPtr, i );
            break;
        case frmTableObj:
            tableP = FrmGetObjectPtr ( frmPtr, i );
            fieldP = TblGetCurrentField( tableP );
            break;
        default:
            fieldP = NULL;
            break;
        }

        if ( fieldP != NULL )
        {
            if ( ToolsGetFieldHighlightText( fieldP, buf, size, true ) == errNone )
                return errNone;
        }

        if ( i == focusIdx )
        {
            // there is nothing in active field, so we go through all fields.
            focusIdx = 0xffff;
            i = 0;
        }
        else
            i++;
    }

    return ~errNone;
}

/***********************************************************************
 *
 * FUNCTION: 	 ToolsFontTruncateName
 *
 * DESCRIPTION: This routine trunctated a category name such that it
 *              is short enough to display.
 *
 * PARAMETERS:	name    - pointer to the name of the new category
 *				maxWidth	- maximum pixel width that name can occupy.
 *				font - font will to be displayed.
 *
 * RETURNED:	 nothing
 *
 *
 ***********************************************************************/
static void ToolsFontTruncateName ( Char * name, FontID font, UInt16 maxWidth )
{
    UInt32 bytesThatFit;
    UInt32 charEnd;
    Int16 truncWidth;
    UInt32 length;
    FontID curFont;
    AppGlobalType* global;

    global = AppGetGlobal();

    curFont = ZDicFontSet ( global->fontLibrefNum, &global->font, font );

    length = StrLen( name );
    bytesThatFit = FntWidthToOffset( name, length, maxWidth, NULL, &truncWidth );

    // If we need to truncate, then figure out how far back to trim to
    // get enough horizongal width for the ellipsis character.

    if ( bytesThatFit < length )
    {
        Int16 ellipsisWidth = FntCharWidth( chrEllipsis );

        // Not enough space for a single ellipsis, so return an empty string.

        if ( ellipsisWidth > maxWidth )
        {
            name[ 0 ] = '\0';
            return ;
        }
        else if ( ellipsisWidth > maxWidth - truncWidth )
        {
            bytesThatFit = FntWidthToOffset( name, length, maxWidth - ellipsisWidth, NULL, &truncWidth );
        }

        // Should never happen, but make sure we don't create a string that's
        // longer than the max category length (leave room for ellipsis & null)
        bytesThatFit = bytesThatFit < dmDBNameLength - 2
                       ? bytesThatFit : dmDBNameLength - 2;

        // Also make sure we don't truncate in the middle of a character.
        TxtCharBounds( name, bytesThatFit, &bytesThatFit, &charEnd );
        bytesThatFit += TxtSetNextChar( name, bytesThatFit, chrEllipsis );
        name[ bytesThatFit ] = '\0';
    }
    else if ( length > dmCategoryLength - 1 )
    {
        TxtCharBounds( name, dmCategoryLength - 1, &bytesThatFit, &charEnd );
        name[ bytesThatFit ] = '\0';
    }

    ZDicFontSet ( global->fontLibrefNum, &global->font, curFont );
}


/*
 * FUNCTION: ToolsPutWordFieldToHistory
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static Err ToolsPutWordFieldToHistory( UInt16 fieldID )
{
    Char * wordStr;
    AppGlobalType *	global;
    Boolean exist = false;
    Int16	i;

    global = AppGetGlobal();

    wordStr = FldGetTextPtr ( GetObjectPtr( fieldID ) );
    if ( wordStr == NULL || wordStr[ 0 ] == chrNull )
        return errNone;


    for ( i = 0; i < MAX_HIS_FAR && global->prefs.history[ i ][ 0 ] != chrNull; i++ )
    {
        if ( StrCompare ( &global->prefs.history[ i ][ 0 ], wordStr ) == 0 )
        {
            // if the word already in history then move it to top;
            if ( i != 0 )
            {
                MemMove( &global->prefs.history[ 1 ][ 0 ], &global->prefs.history[ 0 ][ 0 ], ( MAX_WORD_LEN + 1 ) * i );
                StrCopy( &global->prefs.history[ 0 ][ 0 ], wordStr );
            }
            exist = true;
        }
    }

    if ( !exist )
    {
        // push current key string to history.
        MemMove( &global->prefs.history[ 1 ][ 0 ],
                 &global->prefs.history[ 0 ][ 0 ], ( MAX_HIS_FAR - 1 ) * ( MAX_WORD_LEN + 1 ) );
        StrNCopy( &global->prefs.history[ 0 ][ 0 ], wordStr, MAX_WORD_LEN );
    }

    // reset the history seek.
    global->historySeekIdx = 0;

    return errNone;
}

/*
 * FUNCTION: ToolsGetStartWord
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 
 *
 * RETURNED:    nothing
 *
 */

static void ToolsGetStartWord( AppGlobalType *global )
{
    UInt16	textLen;
    MemHandle	textH;
    Char	*textPtr;

    global->initKeyWord[ 0 ] = chrNull;

    // Get text from clipboard if it is enable.
    if ( global->prefs.getClipBoardAtStart )
    {
        textH = ClipboardGetItem ( clipboardText, &textLen );
        if ( ( textH != NULL ) && ( textLen != 0 ) )
        {
            textPtr = MemHandleLock( textH );
            if ( textLen > MAX_WORD_LEN )
                textLen = MAX_WORD_LEN;
            MemMove( &global->initKeyWord[ 0 ], textPtr, textLen );
            global->initKeyWord[ textLen ] = chrNull;
            MemHandleUnlock( textH );
        }
    }

    // if the clip board is empty then get highlight string.
    if ( global->initKeyWord[ 0 ] == chrNull )
    {
        ToolsGetFormHighlightText( global->prvActiveForm, &global->initKeyWord[ 0 ], MAX_WORD_LEN );
    }

    if ( global->initKeyWord[ 0 ] == chrNull && global->prefs.history[ 0 ][ 0 ] != chrNull )
    {
        MemMove( &global->initKeyWord[ 0 ], &global->prefs.history[ 0 ][ 0 ], MAX_WORD_LEN );
        global->initKeyWord[ MAX_WORD_LEN ] = chrNull;
    }

    // clear head space character and tail space.
    if ( global->initKeyWord[ 0 ] != chrNull )
    {
        Int16	i;

        i = 0;
        while ( global->initKeyWord[ i ] == chrSpace )
            i++;

        if ( i != 0 )
            StrCopy ( &global->initKeyWord[ 0 ], &global->initKeyWord[ i ] );

        i = StrLen ( &global->initKeyWord[ 0 ] );
        while ( i > 0 && global->initKeyWord[ i - 1 ] == chrSpace )
            i--;
        global->initKeyWord[ i ] = chrNull;
    }

    return ;
}

/*
 * FUNCTION: ToolsClearInput
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 */

static Boolean ToolsClearInput( UInt16 fieldID )
{
    FormType	* frmP;
    UInt16	idx;

    ToolsSetFieldHandle( fieldID, NULL, true );
    frmP = FrmGetActiveForm();
    idx = FrmGetObjectIndex( frmP, fieldID );
    FrmSetFocus ( frmP, idx );

    return true;
}

/***********************************************************************
 *
 * FUNCTION:     ToolsOpenNextDict
 *
 * DESCRIPTION:	Open next dictionary.
 *
 * PARAMETERS:	formID.
 *
 * RETURNED:	errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
static void ToolsOpenNextDict( UInt16 formID )
{
    ZDicDBDictInfoType	* dictInfo;
    AppGlobalType	*global;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    ZDicCloseCurrentDict();

    dictInfo->curMainDictIndex ++;
    if ( dictInfo->curMainDictIndex >= dictInfo->itemNumber )
        dictInfo->curMainDictIndex = 0;

    FrmUpdateForm( formID, updateDictionaryChanged );

    return ;
}

/*
 * FUNCTION: ToolsPlayVoice
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static Err ToolsPlayVoice ( void )
{
    UInt8 * explainPtr;
    UInt32 explainLen;
    AppGlobalType *global;
    Err err = errNone;

    global = AppGetGlobal();

    err = ZDicLookupCurrent( &explainPtr, &explainLen, &global->descript );
    if ( err == errNone )
    {
        ZDicVoicePlayWord ( ( Char* ) explainPtr );
    }

    return err;
}

/*
 * FUNCTION: ToolsSendMenuCmd
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static void ToolsSendMenuCmd ( UInt16 chr )
{
    EventType event;
    Err error;

    event.eType = keyDownEvent;
    event.penDown = 0;
    event.tapCount = 1;
    event.data.keyDown.chr = vchrCommand;
    event.data.keyDown.keyCode = vchrCommand;
    event.data.keyDown.modifiers = commandKeyMask;
    if ( MenuHandleEvent( 0, &event, &error ) )
    {
        EvtGetEvent( &event, evtNoWait );
        EvtGetEvent( &event, evtNoWait );
        EvtGetEvent( &event, evtNoWait );

        event.eType = menuCmdBarOpenEvent;
        FrmDispatchEvent( &event );
        event.eType = keyDownEvent;
        event.penDown = 0;
        event.tapCount = 1;
        event.data.keyDown.chr = chr;
        event.data.keyDown.keyCode = 0;
        event.data.keyDown.modifiers = 0;
        EvtAddEventToQueue ( &event );
    }

    return ;
}

#pragma mark -

/***********************************************************************
 *
 * FUNCTION:     AppReigsterWinDisplayChangedNotification
 *
 * DESCRIPTION:  Register for NotifyMgr notifications for WinDisplayChanged.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void AppReigsterWinDisplayChangedNotification(void)
{
	UInt16 cardNo;
	LocalID dbID;
	Err err;

	err = SysCurAppDatabase(&cardNo, &dbID);
	ErrNonFatalDisplayIf(err != errNone, "can't get app db info");
	if(err == errNone)
	{
		err = SysNotifyRegister(cardNo, dbID, sysNotifyDisplayResizedEvent,
								NULL, sysNotifyNormalPriority, NULL);

#if EMULATION_LEVEL == EMULATION_NONE
		ErrNonFatalDisplayIf((err != errNone) && (err != sysNotifyErrDuplicateEntry), "can't register");
#endif

	}

	return;
}
/*
 * FUNCTION: AppGetGlobal
 *
 * DESCRIPTION: 
 *
 * This routine return global pointer.
 *
 * PARAMETERS:
 *
 * nothing.
 *
 * RETURNED:
 *     return global pointer if success else return NULL. 
 */

AppGlobalType * AppGetGlobal()
{
    UInt32	global;
    Err	err;

    err = FtrGet( appFileCreator, AppGlobalFtr, &global );
    return err == errNone ? ( AppGlobalType * ) global : NULL;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop( void )
{
    UInt16 error;
    EventType event;
    Int32 ticksPreHalfSecond;
    FormType *form, *originalForm;

    ticksPreHalfSecond = SysTicksPerSecond() / TIMES_PRE_SECOND;

    // Remember the original form
    originalForm = FrmGetActiveForm();
    form = FrmInitForm( MainForm );
    FrmSetActiveForm( form );

    event.eType = frmOpenEvent;
    MainFormHandleEvent( &event );

    do
    {
        /* change timeout if you need periodic nilEvents */
        EvtGetEvent( &event, ticksPreHalfSecond /*evtWaitForever*/ );
    
        if ( event.eType != nilEvent )
            error = errNone;

        if ( SysHandleEvent( &event ) )
            continue;

        if ( MenuHandleEvent( NULL, &event, &error ) )
            continue;

        if ( MainFormHandleEvent( &event ) )
            continue;

        FrmHandleEvent( FrmGetActiveForm(), &event );

    }
    while ( event.eType != appStopEvent );

    event.eType = frmCloseEvent;
    MainFormHandleEvent( &event );

    FrmEraseForm( form );
    FrmDeleteForm( form );
    FrmSetActiveForm ( originalForm );
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for da sub launch.
 */

static void AppDAEventLoop( Boolean enableSmallDA, Boolean *bGotoMainForm )
{
    EventType event;
    Boolean done = false;
    UInt16 error;
    Int32 ticksPreHalfSecond;

    ticksPreHalfSecond = SysTicksPerSecond() / TIMES_PRE_SECOND;
    *bGotoMainForm = false;

    // Initial form
    event.eType = frmOpenEvent;
    if ( enableSmallDA )
        DAFormHandleEvent( &event );
    else
        MainFormHandleEvent( &event );

    do
    {
        EvtGetEvent( &event, ticksPreHalfSecond );
        
        if ( event.eType == keyDownEvent
                && EvtKeydownIsVirtual( &event )
                && event.data.keyDown.chr == vchrJogBack )
        {
            break;
        }

        if ( SysHandleEvent( &event ) )
            continue;

        if ( MenuHandleEvent( NULL, &event, &error ) )
            continue;

        if ( enableSmallDA )
        {
            // Handle More button. we exit da form and goto main form.
            if ( event.eType == ctlSelectEvent && event.data.ctlSelect.controlID == DAMoreButton )
            {
                FieldType	* field;
                Char *str;

                // pass content of word field to Main form.
                field = GetObjectPtr( DAWordField );
                str = FldGetTextPtr( field );
                if ( str != NULL && *str != chrNull )
                {
                    AppGlobalType * global;

                    // Get highlight text.
                    global = AppGetGlobal();
                    StrNCopy( &global->initKeyWord[ 0 ], str, MAX_WORD_LEN );
                    global->initKeyWord[ MAX_WORD_LEN ] = chrNull;
                }

                *bGotoMainForm = true;
                break;
            }

            // pen outside da form then exit.
            else if ( event.eType == penDownEvent )
            {
                RectangleType bounds;

                FrmGetFormBounds( FrmGetActiveForm(), &bounds );
                RctInsetRectangle( &bounds, 2 );
                if ( IsOutside( &bounds, event.screenX, event.screenY ) )
                    break;
            }

        }
        else
        {
            // Handle exit button.
            if ( event.eType == ctlSelectEvent && event.data.ctlSelect.controlID == MainExitButton )
                break;
        }


        if ( enableSmallDA )
        {
            if ( DAFormHandleEvent( &event ) )
                continue;
        }
        else
        {
            if ( MainFormHandleEvent( &event ) )
                continue;
        }

        FrmHandleEvent( FrmGetActiveForm(), &event );

    }
    while ( event.eType != appStopEvent );

    // pass appstop event to background form.
    if ( event.eType == appStopEvent )
    {
        ToolsQuitApp();
    }

    event.eType = frmCloseEvent;
    if ( enableSmallDA )
        DAFormHandleEvent( &event );
    else
        MainFormHandleEvent( &event );

}

/*
 * FUNCTION: AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * RETURNED:
 *     errNone - if nothing went wrong
 */

static Err AppStart( Boolean subLaunch )
{
    AppGlobalType *global, *oldGlobal;
    UInt16 prefsSize;
    Int16 prefsVersion, itemNum;
    Err	err = ~errNone;

    // global already exist, so only double it and return.
    oldGlobal = AppGetGlobal();

    // We force to only two lever deep and only have one DA.
    if ( oldGlobal != NULL
        && ( oldGlobal->prvGlobalP != NULL || oldGlobal->subLaunch ) )
        return memErrNotEnoughSpace;

    global = ( AppGlobalType * ) MemPtrNew( sizeof( AppGlobalType ) );
    if ( global == NULL )
        return memErrNotEnoughSpace;

    MemSet( global, sizeof( AppGlobalType ), 0 );

    err = FtrSet( appFileCreator, AppGlobalFtr, ( UInt32 ) global );
    if ( err != errNone )
        goto exit;

    // Initial global.
    global->prvGlobalP = oldGlobal;
    global->subLaunch = subLaunch;

    /* Read the saved preferences / saved-state information. */
    prefsSize = sizeof( global->prefs );
    prefsVersion = PrefGetAppPreferences(
                       appFileCreator, appPrefID, &global->prefs, &prefsSize, true );
    if ( prefsVersion > appPrefVersionNum || prefsSize != sizeof( global->prefs ) )
    {
        prefsVersion = noPreferenceFound;
    }

    if ( prefsVersion != appPrefVersionNum )
    {
        /* no prefs; initialize pref struct with default values */
        MemSet( &global->prefs, sizeof( global->prefs ), 0 );
        global->prefs.font = eZDicFontSmall;
        global->prefs.getClipBoardAtStart = false;
        global->prefs.enableIncSearch = true;
        global->prefs.enableSingleTap = true;
        global->prefs.enableWordList = false;
        global->prefs.enableHighlightWord = true;
        global->prefs.enableTryLowerSearch = true;
        global->prefs.useSystemFont = false;
        global->prefs.enableJumpSearch = true;
        global->prefs.enableAutoSpeech = false;
        global->prefs.daFormLocation.x = ResLoadConstant( DAFormOriginX );
        global->prefs.daFormLocation.y = ResLoadConstant( DAFormOriginY );
        global->prefs.incSearchDelay = ResLoadConstant( DefaultIncSearchDelay );
        global->prefs.exportCagetoryIndex = dmUnfiledCategory;
        global->prefs.exportPrivate = false;
        global->prefs.exportAppCreatorID = sysFileCMemo;

        // Do not initial dictionary list at start every time. it maybe slowly.
        // We initial dictionary when
        // 1. fist time to run.
        // 2. open preferences dialog in menu.
        // 3. open dictionary fail.

        // Initial dictionary list.
        itemNum = ZDicDBInitDictList( appDBType, appKDicCreator );
        if ( itemNum == 0 )
        {
            FrmAlert ( DictNotFoundAlert );
            err = ~errNone;
            goto exit;
        }

        // Initial index record of dictionary database that in dictionary list.
        ZDicDBInitIndexRecord();
    }

    // Open current dictionary.
    err = ZDicOpenCurrentDict ();
    if ( err != errNone )
        goto exit;

    // Initial font share library.
    err = ZDicToolsLibInitial ( ZDicFontTypeID, ZDicFontCreatorID, ZDicFontName,
                                &global->fontLibrefNum, &global->fontLibLoad );
    if ( err != errNone )
    {
        FrmAlert ( NoFontAlert );
        goto exit;
    }
    ZDicFontInit ( global->fontLibrefNum, &global->font, global->prefs.useSystemFont );

    // Initial dia.
    ZDicDIALibInitial ( global );

    // Get initial word;
    global->prvActiveForm = FrmGetActiveForm();
    ToolsGetStartWord(global);

    return err;
    
exit:
    ZDicCloseCurrentDict();

    // Release the phonic font resource when app quits.
    if ( global->fontLibrefNum )
    {
        ZDicFontRelease ( global->fontLibrefNum, &global->font );
        ZDicToolsLibRelease( global->fontLibrefNum, global->fontLibLoad );
    }

    MemPtrFree( global );
    FtrSet( appFileCreator, AppGlobalFtr, ( UInt32 ) NULL );
    return err;
}

/*
 * FUNCTION: AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 */

static void AppStop(void)
{
    AppGlobalType * global;

    global = AppGetGlobal();
    if ( global != NULL )
    {
        ZDicCloseCurrentDict();

        // Release the phonic font resource when app quits.
        ZDicFontRelease ( global->fontLibrefNum, &global->font );

        ZDicToolsLibRelease( global->fontLibrefNum, global->fontLibLoad );

        /*
         * Write the saved preferences / saved-state information.  This
         * data will be saved during a HotSync backup. 
         */
        PrefSetAppPreferences(
            appFileCreator, appPrefID, appPrefVersionNum,
            &global->prefs, sizeof( global->prefs ), true );

        // Set previous  global to current global.
        FtrSet( appFileCreator, AppGlobalFtr, ( UInt32 ) global->prvGlobalP );
        MemPtrFree( global );
    }
    
    // Release all memory that used by ZDicSpeech.
    ZDicVoiceRelease();
    
    return;
}

#pragma mark -

/*
 * FUNCTION: RomVersionCompatible
 *
 * DESCRIPTION: 
 *
 * This routine checks that a ROM version is meet your minimum 
 * requirement.
 *
 * PARAMETERS:
 *
 * requiredVersion
 *     minimum rom version required
 *     (see sysFtrNumROMVersion in SystemMgr.h for format)
 *
 * launchFlags
 *     flags that indicate if the application UI is initialized
 *     These flags are one of the parameters to your app's PilotMain
 *
 * RETURNED:
 *     error code or zero if ROM version is compatible
 */

static Err RomVersionCompatible( UInt32 requiredVersion, UInt16 launchFlags )
{
    UInt32 romVersion;

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
    if ( romVersion < requiredVersion )
    {
        if ( ( launchFlags &
                ( sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp ) ) ==
                ( sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp ) )
        {
            FrmAlert ( RomIncompatibleAlert );

            /* Palm OS versions before 2.0 will continuously relaunch this
             * app unless we switch to another safe one. */
            if ( romVersion < kPalmOS20Version )
            {
                AppLaunchWithCommand(
                    sysFileCDefaultApp,
                    sysAppLaunchCmdNormalLaunch, NULL );
            }
        }

        return sysErrRomIncompatible;
    }

    return errNone;
}

/*
 * FUNCTION: PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 * 
 * PARAMETERS:
 *
 * cmd
 *     word value specifying the launch code. 
 *
 * cmdPB
 *     pointer to a structure that is associated with the launch code
 *
 * launchFlags
 *     word value providing extra information about the launch.
 *
 * RETURNED:
 *     Result of launch, errNone if all went OK
 */

UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags )
{
    Err error;

    error = RomVersionCompatible ( ourMinVersion, launchFlags );
    if ( error )
        return ( error );

    switch ( cmd )
    {
    case sysAppLaunchCmdNormalLaunch:

        error = AppStart( false );
        if ( error )
            return error;

        /*
         * start application by opening the main form
         * and then entering the main event loop 
         */
        AppEventLoop();

        AppStop();

        break;

    case sysZDicCmdTibrProLaunch:
    case sysZDicCmdDALaunch:
        {
            FormPtr form, originalForm;
            AppGlobalType *	global;
            Boolean bGotoMainForm;

            error = AppStart( true );
            if ( error )
                return error;

            global = AppGetGlobal();

            // Remember the original form
            originalForm = FrmGetActiveForm();
            form = FrmInitForm( DAForm );
            FrmSetActiveForm( form );

            AppDAEventLoop( true, &bGotoMainForm );

            FrmEraseForm( form );
            FrmDeleteForm( form );
            FrmSetActiveForm ( originalForm );

            if ( bGotoMainForm && global->prvGlobalP == NULL )
            {
                // Remember the original form
                originalForm = FrmGetActiveForm();
                form = FrmInitForm( MainForm );
                FrmSetActiveForm( form );

                AppDAEventLoop( false, &bGotoMainForm );

                FrmEraseForm( form );
                FrmDeleteForm( form );
                FrmSetActiveForm ( originalForm );
            }

            AppStop();
            break;
        }

    case sysAppLaunchCmdSystemReset:
        //AppReigsterWinDisplayChangedNotification();
        break;
        
    case sysAppLaunchCmdNotify:
        //ZDicDIACmdNotify ( cmdPBP );
        break;
    
    }

    return errNone;
}

#pragma mark -

/*
 * FUNCTION: PrefFormInit
 *
 * DESCRIPTION: This routine initializes the PrefsForm form.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void PrefFormInit( FormType *frmP )
{
    AppGlobalType	* global;
    ZDicDBDictInfoType	*dictInfo;
    ListType	*listP;
    Int16	itemNum;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    // Initial dictionary list.
    itemNum = ZDicDBInitDictList( appDBType, appKDicCreator );
    if ( itemNum == 0 )
    {
        FrmAlert ( DictNotFoundAlert );
        ToolsQuitApp();
        return ;
    }

    // Initial index record of dictionary database that in dictionary list.
    ZDicDBInitIndexRecord();

    listP = ( ListType* ) GetObjectPtr( PrefsDictList );
    if ( listP )
    {
        // initial the list.
        LstSetListChoices ( listP, global->listItem, dictInfo->itemNumber );
        LstSetSelection ( listP, dictInfo->curMainDictIndex );
    }

    // Set the phonetic list
    listP = ( ListType* ) GetObjectPtr( PrefsPhoneticList );
    LstSetSelection ( listP, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );

    // Set font group selection.
    FrmSetControlGroupSelection ( frmP, 1,
                                  global->prefs.font == eZDicFontSmall ?
                                  PrefsFontStandardPushButton : PrefsFontLargePushButton );

    return ;
}

/***********************************************************************
 *
 * FUNCTION:	PrefMoveDictionary
 *
 * DESCRIPTION:	Handler of move dictionary if user tap up or down button.
 *
 * PARAMETERS:	->	direction Direction of page scroll.
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/

static void PrefMoveDictionary( WinDirectionType direction )
{
    AppGlobalType	* global;
    ZDicDBDictInfoType	*dictInfo, *dictBuf;
    ListType	*listP;
    Int16	idx;
    Int16	offset;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;
    dictBuf = &global->data.dictInfoList;

    if ( dictInfo->itemNumber <= 1 )
        return ;

    offset = 0;
    switch ( direction )
    {
    case winLeft:
    case winUp:
        if ( dictInfo->curMainDictIndex > 0 )
            offset = -1;
        break;
    case winRight:
    case winDown:
        if ( dictInfo->curMainDictIndex < dictInfo->itemNumber - 1 )
            offset = 1;
        break;
    default:
        ErrNonFatalDisplay( "Bad parameter" );
    }
    if ( offset == 0 )
        return ;

    idx = dictInfo->curMainDictIndex;

    // Switch dictionary information.
    StrCopy( &dictBuf->dictName[ 0 ][ 0 ], &dictInfo->dictName[ idx ][ 0 ] );
    StrCopy( &dictBuf->displayName[ 0 ][ 0 ], &dictInfo->displayName[ idx ][ 0 ] );
    dictBuf->volRefNum[ 0 ] = dictInfo->volRefNum[ idx ];
    dictBuf->phonetic[ 0 ] = dictInfo->phonetic[ idx ];

    StrCopy( &dictInfo->dictName[ idx ][ 0 ], &dictInfo->dictName[ idx + offset ][ 0 ] );
    StrCopy( &dictInfo->displayName[ idx ][ 0 ], &dictInfo->displayName[ idx + offset ][ 0 ] );
    dictInfo->volRefNum[ idx ] = dictInfo->volRefNum[ idx + offset ];
    dictInfo->phonetic[ idx ] = dictInfo->phonetic[ idx + offset ];

    StrCopy( &dictInfo->dictName[ idx + offset ][ 0 ], &dictBuf->dictName[ 0 ][ 0 ] );
    StrCopy( &dictInfo->displayName[ idx + offset ][ 0 ], &dictBuf->displayName[ 0 ][ 0 ] );
    dictInfo->volRefNum[ idx + offset ] = dictBuf->volRefNum[ 0 ];
    dictInfo->phonetic[ idx + offset ] = dictBuf->phonetic[ 0 ];

    dictInfo->curMainDictIndex += offset;

    // Update dictionary list.
    listP = ( ListType* ) GetObjectPtr( PrefsDictList );
    if ( listP )
    {
        LstSetSelection ( listP, dictInfo->curMainDictIndex );
        LstDrawList ( listP );
    }

    return ;
}

/*
 * FUNCTION: PrefFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "PrefsForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean PrefFormHandleEvent( FormType *frmP, EventType *eventP, Boolean *exit )
{
    Boolean	handled = false;
    ListType	*listP;
    AppGlobalType	*global;
    ZDicDBDictInfoType	*dictInfo;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;
    *exit = false;

    switch ( eventP->eType )
    {
    case frmUpdateEvent:
        /*
         * To do any custom drawing here, first call
         * FrmDrawForm(), then do your drawing, and
         * then set handled to true. 
         */
        FrmDrawForm( frmP );
        break;

    case ctlSelectEvent:
        {
            if ( eventP->data.ctlSelect.controlID == PrefsDoneButton )
            {
                *exit = true;
                handled = true;
            }
            else if ( eventP->data.ctlSelect.controlID == PrefsUpButton )
            {
                PrefMoveDictionary ( winUp );
                handled = true;
            }
            else if ( eventP->data.ctlSelect.controlID == PrefsDownButton )
            {
                PrefMoveDictionary ( winDown );
                handled = true;
            }
            break;
        }

    case lstSelectEvent:
        if ( eventP->data.lstSelect.listID == PrefsDictList )
        {
            // set current dictionary.
            dictInfo->curMainDictIndex = eventP->data.lstSelect.selection;

            // Set the phonetic list
            listP = ( ListType* ) GetObjectPtr( PrefsPhoneticList );
            LstSetSelection ( listP, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );
            handled = true;
        }
        else if ( eventP->data.lstSelect.listID == PrefsPhoneticList )
        {
            listP = ( ListType* ) GetObjectPtr( PrefsDictList );
            dictInfo->phonetic[ LstGetSelection( listP ) ] = eventP->data.lstSelect.selection;
            handled = true;
        }

        break;


    }

    return handled;
}

/*
 * FUNCTION: PrefFormPopupPrefsDialog
 *
 * DESCRIPTION: This routine popup "PrefsForm" dialog.
 *
 * PARAMETERS: nothing.
 *
 * RETURNED: nothing.
 */

static Err PrefFormPopupPrefsDialog( void )
{
    EventType	event;
    FormPtr	frmP, originalForm;
    Boolean	exit, handled;
    UInt16	originalFormID;
    AppGlobalType * global;

    global = AppGetGlobal();

    ZDicCloseCurrentDict();

    // Display the preference from the display

    // Remember the original form
    originalForm = FrmGetActiveForm();
    originalFormID = FrmGetFormId ( originalForm );

    frmP = FrmInitForm ( PrefsForm );
    FrmSetActiveForm( frmP );
    PrefFormInit( frmP );
    FrmDrawForm ( frmP );

    do
    {
        EvtGetEvent( &event, evtWaitForever );

        if ( SysHandleEvent ( &event ) )
            continue;

        if ( event.eType == appStopEvent )
            EvtAddEventToQueue( &event );

        handled = PrefFormHandleEvent( frmP, &event, &exit );

        // Check if the form can handle the event
        if ( !handled )
            FrmHandleEvent ( frmP, &event );

    }
    while ( event.eType != appStopEvent && !exit );


    // update user change.
    {
        UInt16 index, objID;
        FontID newFont;
        AppGlobalType	*global;

        global = AppGetGlobal();

        // Get font setting.
        index = FrmGetControlGroupSelection ( frmP, 1 );
        objID = FrmGetObjectId ( frmP, index );
        newFont = objID == PrefsFontStandardPushButton ? eZDicFontSmall : eZDicFontLarge;
        if ( newFont != global->prefs.font )
        {
            global->prefs.font = newFont;
            FrmUpdateForm( originalFormID, updateDictionaryChanged | updateFontChanged );
        }
        else
            FrmUpdateForm( originalFormID, updateDictionaryChanged );
    }

    FrmEraseForm ( frmP );
    FrmDeleteForm ( frmP );
    FrmSetActiveForm ( originalForm );

    return errNone;
}

#pragma mark -


///////////////////////////////////////////////////////////////////////////

/*
 * FUNCTION: DetailsFormInit
 *
 * DESCRIPTION: This routine initializes the DetailsForm form.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void DetailsFormInit( FormType *frmP )
{
    AppGlobalType * global;

    global = AppGetGlobal();

    // Set the get clipboard at normal launch.
    CtlSetValue ( GetObjectPtr ( DetailsGetClipBoardAtStart ), global->prefs.getClipBoardAtStart );

    // Set the incremental search.
    CtlSetValue ( GetObjectPtr ( DetailsEnableIncSearch ), global->prefs.enableIncSearch );

    // Set the enable jump search for single tap.
    CtlSetValue ( GetObjectPtr ( DetailsEnableSingleTap ), global->prefs.enableSingleTap );

    // Set the enable highlight word field.
    CtlSetValue ( GetObjectPtr ( DetailsEnableHighlightWord ), global->prefs.enableHighlightWord );

    // Set the enable try lower case on search failed..
    CtlSetValue ( GetObjectPtr ( DetailsEnableTryLowerSearch ), global->prefs.enableTryLowerSearch );

    // Set the disable phonetic font support inside.
    CtlSetValue ( GetObjectPtr ( DetailsDisablePhoneticFont ), global->prefs.useSystemFont );

    // Set the slider values of incremental search delay.
    CtlSetSliderValues ( GetObjectPtr ( DetailSearchDelay ), NULL, NULL, NULL, &global->prefs.incSearchDelay );

    // Set the enable automatic speech.
    CtlSetValue ( GetObjectPtr ( DetailsEnableAutoSpeech ), global->prefs.enableAutoSpeech );

    return ;
}

/*
 * FUNCTION: DetailsfFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "DetailsForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean DetailsFormHandleEvent( FormType * frmP, EventType * eventP, Boolean *exit )
{
    Boolean	handled = false;
    AppGlobalType	*global;

    global = AppGetGlobal();
    *exit = false;

    switch ( eventP->eType )
    {
    case frmOpenEvent:
        {
            frmP = FrmGetActiveForm();
            DetailsFormInit( frmP );
            FrmDrawForm( frmP );
            handled = true;
            break;
        }

    case frmUpdateEvent:
        /*
         * To do any custom drawing here, first call
         * FrmDrawForm(), then do your drawing, and
         * then set handled to true. 
         */
        frmP = FrmGetActiveForm();
        FrmDrawForm( frmP );
        break;

    case ctlSelectEvent:
        {
            if ( eventP->data.ctlSelect.controlID == DetailsDoneButton )
            {
                // Get the get clipboard at normal launch setting.
                global->prefs.getClipBoardAtStart = ( CtlGetValue ( GetObjectPtr ( DetailsGetClipBoardAtStart ) ) != 0 );

                // Get the incremental search setting.
                global->prefs.enableIncSearch = ( CtlGetValue ( GetObjectPtr ( DetailsEnableIncSearch ) ) != 0 );

                // Get the incremental search setting.
                global->prefs.enableSingleTap = ( CtlGetValue ( GetObjectPtr ( DetailsEnableSingleTap ) ) != 0 );

                // Get the enable highlight word field.
                global->prefs.enableHighlightWord = ( CtlGetValue ( GetObjectPtr ( DetailsEnableHighlightWord ) ) != 0 );

                // Get the enable highlight word field.
                global->prefs.enableTryLowerSearch = ( CtlGetValue ( GetObjectPtr ( DetailsEnableTryLowerSearch ) ) != 0 );

                // Get the disable phonetic font support inside.
                global->prefs.useSystemFont = ( CtlGetValue ( GetObjectPtr ( DetailsDisablePhoneticFont ) ) != 0 );

                // Get the enable automatic speech.
                global->prefs.enableAutoSpeech = ( CtlGetValue ( GetObjectPtr ( DetailsEnableAutoSpeech ) ) != 0 );

                *exit = true;
                handled = true;
            }
            else if ( eventP->data.ctlSelect.controlID == DetailSearchDelay )
            {
                global->prefs.incSearchDelay = eventP->data.ctlSelect.value;
                handled = true;
            }

            break;
        }
    }

    return handled;
}

/*
 * FUNCTION: DetailsFormPopupDetailsDialog
 *
 * DESCRIPTION: This routine popup "DetailsForm" dialog.
 *
 * PARAMETERS: nothing.
 *
 * RETURNED: nothing.
 */

static Err DetailsFormPopupDetailsDialog( void )
{
    EventType	event;
    FormPtr	frmP, originalForm;
    Boolean	exit, handled;

    // Remember the original form
    originalForm = FrmGetActiveForm();

    frmP = FrmInitForm ( DetailsForm );
    FrmSetActiveForm( frmP );
    DetailsFormInit( frmP );
    FrmDrawForm ( frmP );

    do
    {
        EvtGetEvent( &event, evtWaitForever );

        if ( SysHandleEvent ( &event ) )
            continue;

        if ( event.eType == appStopEvent )
            EvtAddEventToQueue( &event );

        handled = DetailsFormHandleEvent( frmP, &event, &exit );

        // Check if the form can handle the event
        if ( !handled )
            FrmHandleEvent ( frmP, &event );

    }
    while ( event.eType != appStopEvent && !exit );


    FrmEraseForm ( frmP );
    FrmDeleteForm ( frmP );
    FrmSetActiveForm ( originalForm );

    return errNone;
}

#pragma mark -


/*
 * FUNCTION: DAFormSearch
 *
 * DESCRIPTION: This routine search the word that in word field.
 *
 * PARAMETERS: putinHistory -> if be ture then put the word to history list.
 *
 * RETURNED: errNone if success else fail.
 *
 */

static Err DAFormSearch( Boolean putinHistory, Boolean highlight, Boolean bEnableBreak, Boolean bEnableAutoSpeech )
{
    UInt16	matchs;
    UInt8	*explainPtr;
    UInt32	explainLen;
    AppGlobalType	*global;
    ZDicDBDictInfoType	*dictInfo;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    if ( bEnableBreak )
    {
        Int32 ticksPreHalfSecond;
        Int16 delayTimes;

        ticksPreHalfSecond = SysTicksPerSecond() / TIMES_PRE_SECOND;
        delayTimes = global->prefs.incSearchDelay;
        while ( !EvtSysEventAvail ( true ) && delayTimes > 0 )
        {
            Int16 x, y;
            Boolean penDown;

            EvtGetPen ( &x, &y, &penDown );
            if ( penDown )
                break;

            SysTaskDelay ( ticksPreHalfSecond );
            delayTimes--;
        }

        if ( delayTimes > 0 )
        {
            global->needSearch = true;
            global->putinHistory = putinHistory;
            global->highlightWordField = highlight;
            return errNone;
        }
        else
            global->needSearch = false;
    }

    err = ToolsSearch( DAWordField, &matchs, &explainPtr, &explainLen );
    if ( err == errNone && explainPtr != NULL && explainLen != 0 )
    {
        FormType * frmP;
        MemHandle	bufH;
        Char *str;

        // Clear the menu status from the display
        MenuEraseStatus( 0 );

        if ( putinHistory )
        {
            ToolsPutWordFieldToHistory( DAWordField );
        }

        bufH = MemHandleNew( explainLen + 1 );
        if ( bufH == NULL )
            return memErrNotEnoughSpace;

        // format string.
        str = MemHandleLock( bufH );
        MemMove( str, explainPtr, explainLen );
        str[ explainLen ] = chrNull;
        ToolsFormatExplain( ( UInt8* ) str );
        ToolsGMXTranslate( str, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );
        MemHandleUnlock( bufH );

        // update display.
        ToolsSetFieldHandle( DADescriptionField, bufH, true );
        ToolsUpdateScrollBar ( DADescriptionField, DADescriptionScrollBar );

        // select all the text in word field, then use can clear it easy.
        if ( highlight )
            ToolsHighlightField( DAWordField );

        // Display or hide the player button
        frmP = FrmGetActiveForm ();
        if ( ZDicVoiceIsExist ( ( Char * ) explainPtr ) )
        {
            ShowObject ( frmP, DAPlayVoice );
            if ( bEnableAutoSpeech )
                ToolsPlayVoice ();
        }
        else
            HideObject ( frmP, DAPlayVoice );

    }

    return errNone;
}

/*
 * FUNCTION: DAFormHandleNilEvent
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED: true if the field handled the event
 *
 */

static Boolean DAFormHandleNilEvent( void )
{
    AppGlobalType	* global;

    global = AppGetGlobal();

    return false;
}

/*
 * FUNCTION: DAFormAdjustFormBounds
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:  -> curBounds Input current bounds.
 *
 * RETURNED:
 */
#define DAFormBorder	3

void DAFormAdjustFormBounds ( AppGlobalType	*global, FormPtr frmP,
                              RectangleType curBounds, RectangleType displayBounds )
{
    // adjust the bround and set new form bround
    if ( curBounds.topLeft.x < DAFormBorder )
        curBounds.topLeft.x = DAFormBorder;
    if ( curBounds.topLeft.y < DAFormBorder )
        curBounds.topLeft.y = DAFormBorder;
    displayBounds.extent.x -= DAFormBorder;
    displayBounds.extent.y -= DAFormBorder;
    if ( curBounds.topLeft.x + curBounds.extent.x > displayBounds.extent.x )
        curBounds.topLeft.x = displayBounds.extent.x - curBounds.extent.x;
    if ( curBounds.topLeft.y + curBounds.extent.y > displayBounds.extent.y )
        curBounds.topLeft.y = displayBounds.extent.y - curBounds.extent.y;
    global->prefs.daFormLocation = curBounds.topLeft;

    // Use new bround for da form.
    curBounds.topLeft = global->prefs.daFormLocation;
    WinSetBounds( FrmGetWindowHandle ( frmP ), &curBounds );

    return ;
}

/*
 * FUNCTION: DAFormMoveForm
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *
 * RETURNED:
 */

static Boolean DAFormMoveForm( EventType * eventP )
{
    FormType	* frmP;
    Coord	x, y, dx, dy, oldx, oldy;
    Boolean	penDown = true;
    RectangleType	r, displayBounds;
    WinHandle	winH;
    AppGlobalType	*global;
    WinDrawOperation	oldMode;
    WinHandle	oldDrawWinH, oldActiveWinH;

    global = AppGetGlobal();

    frmP = FrmGetActiveForm();
    FrmGetObjectBounds ( frmP, FrmGetObjectIndex ( frmP, DAMoveRepeatButton ), &r );
    if ( !RctPtInRectangle ( eventP->screenX, eventP->screenY, &r ) )
        return false;

    // Set the draw and acitve windows to fixe bugs of chinese os.
    WinSetDrawWindow ( FrmGetWindowHandle ( frmP ) );
    WinSetActiveWindow ( FrmGetWindowHandle ( frmP ) );

    // Hide DA form.
    oldDrawWinH = WinGetDrawWindow ();
    oldActiveWinH = WinGetActiveWindow ();
    FrmEraseForm( frmP );

    // get the new display window bounds
    winH = WinGetDisplayWindow ();
    WinSetDrawWindow ( winH );
    ZDicToolsWinGetBounds ( winH, &displayBounds );

    FrmGetFormBounds( frmP, &r );
    RctInsetRectangle( &r, 2 );

    oldMode = WinSetDrawMode ( winInvert );

    PenGetPoint ( &x, &y, &penDown );
    WinWindowToDisplayPt ( &x, &y );
    dx = x - r.topLeft.x;
    dy = y - r.topLeft.y;
    oldx = x;
    oldy = y;
    WinPaintRectangleFrame ( roundFrame, &r );

    // Trace the pen until it move enough to constitute a move operation or until
    // the pen it released.
    while ( true )
    {
        PenGetPoint ( &x, &y, &penDown );
        WinWindowToDisplayPt ( &x, &y );

        if ( ! penDown )
            break;

        if ( x != oldx || y != oldy )
        {
            WinPaintRectangleFrame ( roundFrame, &r );
            r.topLeft.x = x - dx;
            r.topLeft.y = y - dy;
            oldx = x;
            oldy = y;
            WinPaintRectangleFrame ( roundFrame, &r );
        }
    }

    WinPaintRectangleFrame ( roundFrame, &r );

    // adjust the bround and set new form bround
    DAFormAdjustFormBounds ( global, frmP, r, displayBounds );

    WinSetDrawMode ( oldMode );

    // Show DA form.
    WinSetDrawWindow ( oldDrawWinH );
    WinSetActiveWindow ( oldActiveWinH );
    FrmDrawForm( frmP );

    return true;
}

/*
 * FUNCTION: MainFormIncrementalSearch
 *
 * DESCRIPTION: Adds a character to word field and looks up the word.
 *
 * PARAMETERS: event - EventType* containing character to add to word field.
 *
 * RETURNED: true if the field handled the event
 *
 */

static Boolean DAFormIncrementalSearch( EventType * event )
{

    FormType	* frmP;
    UInt16	fldIndex;
    FieldPtr	fldP;
    AppGlobalType	*global;

    global = AppGetGlobal();

    frmP = FrmGetActiveForm();
    fldIndex = FrmGetObjectIndex( frmP, DAWordField );
    FrmSetFocus( frmP, fldIndex );
    fldP = FrmGetObjectPtr ( frmP, fldIndex );

    if ( FldHandleEvent ( fldP, event ) || event->eType == fldChangedEvent )
    {
        if ( global->prefs.enableIncSearch )
        {
            DAFormSearch( false, false, true, global->prefs.enableAutoSpeech );
        }

        return true;
    }

    return false;
}

/*
 * FUNCTION: DAFormChangeWordFieldCase
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */
static Err DAFormChangeWordFieldCase( void )
{
    FieldType	* fieldP;
    Char	*wordStr, newStr[ MAX_WORD_LEN + 1 ];
    Err	err;
    AppGlobalType *	global;

    global = AppGetGlobal();

    fieldP = GetObjectPtr( DAWordField );
    wordStr = FldGetTextPtr ( fieldP );
    if ( wordStr == NULL || wordStr[ 0 ] == chrNull )
        return errNone;

    MemSet( newStr, sizeof( newStr ), 0 );
    StrNCopy( newStr, wordStr, MAX_WORD_LEN );

    err = ToolsChangeCase ( newStr );
    if ( err == errNone )
    {
        ToolsSetFieldPtr ( DAWordField, newStr, StrLen( newStr ), true );
        DAFormSearch( false, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
    }

    return errNone;
}

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean DAFormDoCommand( UInt16 command )
{
    Boolean handled = false;
    AppGlobalType *	global;

    global = AppGetGlobal();

    if ( command == DictionarysDictAll
            || command >= ZDIC_DICT_MENUID )
    {
        handled = ToolsChangeDictionaryCommand( DAForm, command );
        return handled;
    }

    switch ( command )
    {
    case OptionsAboutZDic:
        {
            FormType * frmP;

            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the About Box.
            frmP = FrmInitForm ( AboutForm );
            FrmDoDialog ( frmP );
            FrmDeleteForm ( frmP );

            handled = true;
            break;
        }
    case OptionsDictOptions:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the preference from the display
            PrefFormPopupPrefsDialog();
            handled = true;

            // Open new dictionary database.
            if ( ZDicOpenCurrentDict() != errNone )
            {
                ToolsQuitApp();
            }
            else
            {
                DAFormSearch( false, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
            }

            break;
        }
    case OptionsPreferences:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the details from the display
            DetailsFormPopupDetailsDialog();
            handled = true;
            break;
        }
    case OptionsExportOptions:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display exorpt options from.
            ExportPopupDialog();
            handled = true;
            break;
        }
    case OptionsGotoWord:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            DAFormSearch( true, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
            handled = true;
            break;
        }
    case OptionsClearWord:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            handled = ToolsClearInput ( DAWordField );
            break;
        }
    case OptionsClearHistory:
        {
            global->prefs.history[ 0 ][ 0 ] = chrNull;
            global->historySeekIdx = 0;
            handled = true;
            break;
        }
    case OptionsChangeCase:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            DAFormChangeWordFieldCase();
            handled = true;
            break;
        }
    case OptionsExportMemo:
        {
            ExportToMemo( DADescriptionField );
            handled = true;
            break;
        }
    case OptionsExportSugarMemo:
        {
            ExportToSugarMemo( DADescriptionField );
            handled = true;
            break;
        }
    }

    return handled;
}

/***********************************************************************
 *
 * FUNCTION:	DAFormUpdateDisplay
 *
 * DESCRIPTION: This routine redraw the DAForm form.
 *
 * PARAMETERS:
 *				->	updateCode Update code.
 *
 * RETURN:
 *				nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static void DAFormUpdateDisplay( UInt16 updateCode )
{
    AppGlobalType	* global;
    Err	err = errNone;

    global = AppGetGlobal();

    if ( updateCode & updateDictionaryChanged )
    {
        // Open new dictionary database.
        err = ZDicOpenCurrentDict();
        if ( err != errNone )
        {
            ToolsQuitApp();
            return ;
        }

        DAFormSearch( false, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
    }

    if ( updateCode & updateFontChanged )
    {
        FieldType * field;

        field = ( FieldType* ) GetObjectPtr( DADescriptionField );
        FldSetFont( field, ( global->prefs.font == eZDicFontSmall ? global->font.smallFontID : global->font.largeFontID ) );

        ToolsUpdateScrollBar ( DADescriptionField, DADescriptionScrollBar );
    }

    if ( updateCode & frmRedrawUpdateCode )
    {
        FrmDrawForm( FrmGetActiveForm() );
    }

    return ;
}

/*
 * FUNCTION: DAFormInit
 *
 * DESCRIPTION: This routine initializes the DAForm form.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void DAFormInit( FormType *frmP )
{
    AppGlobalType	* global;
    FieldType	*field;

    global = AppGetGlobal();

    // initial description and word field font.
    field = GetObjectPtr( DADescriptionField );
    FldSetFont( field, ( global->prefs.font == eZDicFontSmall ? global->font.smallFontID : global->font.largeFontID ) );
    field = GetObjectPtr( DAWordField );
    FldSetFont( field, global->font.smallFontID );

    // Initial word for word field.
    if ( global->initKeyWord[ 0 ] != chrNull )
    {
        ToolsSetFieldPtr( DAWordField, &global->initKeyWord[ 0 ], StrLen( &global->initKeyWord[ 0 ] ), false );
    }
    else
    {
        ToolsSetFieldPtr( DAWordField, &global->initKeyWord[ 0 ], 2, false );
    }

    FrmSetFocus( frmP, FrmGetObjectIndex( frmP, DAWordField ) );

    // Set get word source.
    FrmSetControlGroupSelection ( frmP, 1,
                                  global->prefs.getClipBoardAtStart ?
                                  DAClipboardPushButton : DASelectPushButton );

    // set da form location
    {
        WinHandle	winH;
        RectangleType	r;

        // Set the draw and acitve windows to fixe bugs of chinese os.
        WinSetDrawWindow ( FrmGetWindowHandle ( frmP ) );
        WinSetActiveWindow ( FrmGetWindowHandle ( frmP ) );

        WinGetDrawWindowBounds ( &r );
        r.topLeft = global->prefs.daFormLocation;
        winH = FrmGetWindowHandle ( frmP );
        WinSetBounds( winH, &r );
    }

    return ;
}

/*
 * FUNCTION: DAFormHandleEvent
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean DAFormHandleEvent( EventType * eventP )
{
    Boolean	handled = false;
    FormType	*frmP;
    AppGlobalType	*global;

    global = AppGetGlobal();

    switch ( eventP->eType )
    {
    case menuEvent:
        return DAFormDoCommand( eventP->data.menu.itemID );

    case menuOpenEvent:
        {
            ToolsCreatDictionarysMenu ();
            // don't set handled = true
            break;
        }

    case frmOpenEvent:

        // Do not draw anything before FrmDrawForm because
        // FrmDrawForm will save current screen.
        frmP = FrmGetActiveForm();
        
        ZDicDIAFormLoadInitial ( global, frmP );
        DAFormInit( frmP );
        ZDicDIADisplayChange ( global );
        FrmDrawForm ( frmP );
        DAFormSearch( true, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
        handled = true;
        break;

    case frmUpdateEvent:
        /*
         * To do any custom drawing here, first call
         * FrmDrawForm(), then do your drawing, and
         * then set handled to true. 
         */
        DAFormUpdateDisplay ( eventP->data.frmUpdate.updateCode );
        handled = true;
        break;

    case frmCloseEvent:
        ToolsSetFieldHandle( DADescriptionField, NULL, false );
        ToolsSetFieldHandle( DAWordField, NULL, false );
        ZDicDIAFormClose ( global );
        break;

    case winDisplayChangedEvent:
        handled = ZDicDIADisplayChange ( global );
        if ( handled )
        {
            frmP = FrmGetActiveForm ();
            FrmDrawForm ( frmP );
        }
        break;

    case winEnterEvent:
        ZDicDIAWinEnter ( global, eventP );
        handled = true;
        break;

    case penDownEvent:
        handled = DAFormMoveForm( eventP );
        break;

    case keyDownEvent:
        {
            if ( eventP->data.keyDown.chr == chrLineFeed )
            {
                DAFormSearch( true, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
                handled = true;
            }

            else if ( EvtKeydownIsVirtual( eventP ) )
            {

                if ( NavDirectionPressed( eventP, Up )
                        || ( eventP->data.keyDown.chr ) == vchrRockerUp
                        || ( eventP->data.keyDown.chr ) == vchrPageUp
                        || ( eventP->data.keyDown.chr ) == vchrThumbWheelUp
                        //	|| (eventP->data.keyDown.chr) == vchrJogUp
                   )
                {
                    ToolsPageScroll ( winUp, DADescriptionField, DADescriptionScrollBar, DAWordField, DAPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Down )
                          || ( eventP->data.keyDown.chr ) == vchrRockerDown
                          || ( eventP->data.keyDown.chr ) == vchrPageDown
                          || ( eventP->data.keyDown.chr ) == vchrThumbWheelDown
                          //	|| (eventP->data.keyDown.chr) == vchrJogDown
                        )
                {
                    ToolsPageScroll ( winDown, DADescriptionField, DADescriptionScrollBar, DAWordField, DAPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Left ) )
                {
                    ToolsScrollWord ( winUp, DADescriptionField, DADescriptionScrollBar, DAWordField, DAPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Right ) )
                {
                    ToolsScrollWord ( winDown, DADescriptionField, DADescriptionScrollBar, DAWordField, DAPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavSelectPressed( eventP )
                          || ( eventP->data.keyDown.chr ) == vchrRockerCenter
                          || ( eventP->data.keyDown.chr ) == vchrThumbWheelPush
                          || ( eventP->data.keyDown.chr ) == vchrJogRelease
                        )
                {
                    ToolsOpenNextDict( DAForm );
                    handled = true;
                }
            }

            else
            {
                handled = DAFormIncrementalSearch( eventP );
            }

            break;
        }

    case fldChangedEvent:
        {
            if ( eventP->data.fldChanged.fieldID == DAWordField )
                handled = DAFormIncrementalSearch( eventP );
            else
                ToolsUpdateScrollBar ( DADescriptionField, DADescriptionScrollBar );

            break;
        }

    case sclRepeatEvent:
        {
            ToolsScroll ( eventP->data.sclRepeat.newValue -
                          eventP->data.sclRepeat.value, false, DADescriptionField, DADescriptionScrollBar );
            break;
        }

    case ctlSelectEvent:
        {
            if ( eventP->data.ctlSelect.controlID == DAClipboardPushButton
                    || eventP->data.ctlSelect.controlID == DASelectPushButton )
            {
                global->prefs.getClipBoardAtStart =
                    eventP->data.ctlSelect.controlID == DAClipboardPushButton ? true : false;
                ToolsGetStartWord (global);

                // Initial word for word field.
                if ( global->initKeyWord[ 0 ] != chrNull )
                {
                    ToolsSetFieldPtr( DAWordField, &global->initKeyWord[ 0 ], StrLen( &global->initKeyWord[ 0 ] ), true );
                    DAFormSearch( true, global->prefs.enableHighlightWord, false, global->prefs.enableAutoSpeech );
                }

                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == DANextDict )
            {
                ToolsOpenNextDict( DAForm );
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == DAPlayVoice )
            {
                ToolsPlayVoice ();
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == DAExportToMemo )
            {
                ToolsSendMenuCmd( global->prefs.exportAppCreatorID == sysFileCMemo ? chrCapital_M : chrCapital_Z );
                handled = true;
            }


            break;
        }

    case nilEvent:
        {
            handled = DAFormHandleNilEvent ();
            break;
        }

        // If there have not any event then we continue the broken search.
        if ( global->needSearch && !EvtSysEventAvail ( true ) )
        {
            DAFormSearch( global->putinHistory, global->highlightWordField, false, global->prefs.enableAutoSpeech );
        }
    }

    return handled;
}

///////////////////////////////////////////////////////////////////////////

#pragma mark -

/*
 * FUNCTION: MainFormAdjustObject
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

Err MainFormAdjustObject ( const RectanglePtr toBoundsP )
{
    FormPtr frmP;
    ControlPtr ctlP;
    FieldPtr fldP;
    RectangleType	descFieldBound, descScrollBarBound, wordListBound, upBtonBound, downBtonBound;
    UInt16	descFieldIndex, descScrollBarIndex, wordListIndex, upBtonIndex, downBtonIndex;
    AppGlobalType	*global;

    global = AppGetGlobal();

    frmP = FrmGetActiveForm();
    descFieldIndex = FrmGetObjectIndex ( frmP, MainDescriptionField );
    FrmGetObjectBounds( frmP, descFieldIndex, &descFieldBound );
    descScrollBarIndex = FrmGetObjectIndex ( frmP, MainDescScrollBar );
    FrmGetObjectBounds( frmP, descScrollBarIndex, &descScrollBarBound );
    wordListIndex = FrmGetObjectIndex ( frmP, MainWordList );
    FrmGetObjectBounds( frmP, wordListIndex, &wordListBound );
    upBtonIndex = FrmGetObjectIndex ( frmP, MainWordListUpRepeatButton );
    FrmGetObjectBounds( frmP, upBtonIndex, &upBtonBound );
    downBtonIndex = FrmGetObjectIndex ( frmP, MainWordListDownRepeatButton );
    FrmGetObjectBounds( frmP, downBtonIndex, &downBtonBound );

    // Only display desc field.
    if ( ! global->wordListIsOn )
    {
        HideObject( frmP, MainWordListUpRepeatButton );
        HideObject( frmP, MainWordListDownRepeatButton );
        HideObject( frmP, MainWordList );
        HideObject( frmP, MainDescriptionField );
        HideObject( frmP, MainDescScrollBar );

        descFieldBound.topLeft.x = COORD_START_X;
        descFieldBound.topLeft.y = COORD_START_Y;
        descFieldBound.extent.x = ( toBoundsP->extent.x - COORD_SCROLLBAR_WIDTH );
        descFieldBound.extent.y = ( toBoundsP->extent.y - COORD_START_Y - COORD_TOOLBAR_HEIGHT );
        FrmSetObjectBounds( frmP, descFieldIndex, &descFieldBound );

        descScrollBarBound.topLeft.x = descFieldBound.topLeft.x + descFieldBound.extent.x;
        descScrollBarBound.topLeft.y = descFieldBound.topLeft.y;
        descScrollBarBound.extent.x = COORD_SCROLLBAR_WIDTH;
        descScrollBarBound.extent.y = descFieldBound.extent.y;
        FrmSetObjectBounds ( frmP, descScrollBarIndex, &descScrollBarBound );

        ShowObject( frmP, MainDescriptionField );
        ShowObject( frmP, MainDescScrollBar );
    }

    // word list is under of desc field.
    else if ( toBoundsP->extent.x < toBoundsP->extent.y )
    {
        HideObject( frmP, MainWordListUpRepeatButton );
        HideObject( frmP, MainWordListDownRepeatButton );
        HideObject( frmP, MainWordList );
        HideObject( frmP, MainDescriptionField );
        HideObject( frmP, MainDescScrollBar );

        wordListBound.topLeft.x = COORD_START_X + COORD_SPACE;
        wordListBound.topLeft.y = toBoundsP->extent.y - COORD_TOOLBAR_HEIGHT - wordListBound.extent.y;
        wordListBound.extent.x = toBoundsP->extent.x - COORD_SPACE * 4 - COORD_SCROLLBAR_WIDTH;
        FrmSetObjectBounds ( frmP, wordListIndex, &wordListBound );

        upBtonBound.topLeft.x = wordListBound.topLeft.x + wordListBound.extent.x + COORD_SPACE;
        upBtonBound.topLeft.y = wordListBound.topLeft.y - COORD_SPACE;
        upBtonBound.extent.x = COORD_SCROLLBAR_WIDTH + 2 * COORD_SPACE;
        upBtonBound.extent.y = wordListBound.extent.y / 2;
        FrmSetObjectBounds ( frmP, upBtonIndex, &upBtonBound );

        downBtonBound.extent.x = upBtonBound.extent.x;
        downBtonBound.extent.y = upBtonBound.extent.y;
        downBtonBound.topLeft.x = upBtonBound.topLeft.x;
        downBtonBound.topLeft.y = wordListBound.topLeft.y + wordListBound.extent.y + COORD_SPACE - downBtonBound.extent.y;
        FrmSetObjectBounds ( frmP, downBtonIndex, &downBtonBound );

        descFieldBound.topLeft.x = COORD_START_X;
        descFieldBound.topLeft.y = COORD_START_Y;
        descFieldBound.extent.x = ( toBoundsP->extent.x - COORD_SCROLLBAR_WIDTH );
        descFieldBound.extent.y = wordListBound.topLeft.y - descFieldBound.topLeft.y - COORD_SPACE;
        FrmSetObjectBounds ( frmP, descFieldIndex, &descFieldBound );

        descScrollBarBound.topLeft.x = descFieldBound.topLeft.x + descFieldBound.extent.x;
        descScrollBarBound.topLeft.y = descFieldBound.topLeft.y;
        descScrollBarBound.extent.x = COORD_SCROLLBAR_WIDTH;
        descScrollBarBound.extent.y = descFieldBound.extent.y;
        FrmSetObjectBounds ( frmP, descScrollBarIndex, &descScrollBarBound );

        // Set the label of the up and down button.
        ctlP = GetObjectPtr ( MainWordListUpRepeatButton );
        CtlGlueSetFont ( ctlP, symbolFont );
        FrmCopyLabel ( frmP, MainWordListUpRepeatButton, "\10" );

        ctlP = GetObjectPtr ( MainWordListDownRepeatButton );
        CtlGlueSetFont ( ctlP, symbolFont );
        FrmCopyLabel ( frmP, MainWordListDownRepeatButton, "\7" );

        ShowObject( frmP, MainWordListUpRepeatButton );
        ShowObject( frmP, MainWordListDownRepeatButton );
        ShowObject( frmP, MainWordList );
        ShowObject( frmP, MainDescriptionField );
        ShowObject( frmP, MainDescScrollBar );
    }
    // word list at left of desc field.
    else
    {
        HideObject( frmP, MainWordListUpRepeatButton );
        HideObject( frmP, MainWordListDownRepeatButton );
        HideObject( frmP, MainWordList );
        HideObject( frmP, MainDescriptionField );
        HideObject( frmP, MainDescScrollBar );

        upBtonBound.topLeft.x = COORD_START_X;
        upBtonBound.topLeft.y = COORD_START_Y + COORD_SPACE;
        upBtonBound.extent.x = ( toBoundsP->extent.x - COORD_SCROLLBAR_WIDTH ) * 3 / 8;
        upBtonBound.extent.y = COORD_SCROLLBAR_WIDTH + COORD_SPACE;
        FrmSetObjectBounds ( frmP, upBtonIndex, &upBtonBound );

        wordListBound.topLeft.x = upBtonBound.topLeft.x + COORD_SPACE;
        wordListBound.topLeft.y = upBtonBound.topLeft.y + upBtonBound.extent.y + COORD_SPACE;
        wordListBound.extent.x = upBtonBound.extent.x - COORD_SPACE * 2;
        FrmSetObjectBounds( frmP, wordListIndex, &wordListBound );

        downBtonBound.topLeft.x = upBtonBound.topLeft.x;
        downBtonBound.topLeft.y = wordListBound.topLeft.y + wordListBound.extent.y + COORD_SPACE;
        downBtonBound.extent.x = upBtonBound.extent.x;
        downBtonBound.extent.y = upBtonBound.extent.y;
        FrmSetObjectBounds ( frmP, downBtonIndex, &downBtonBound );

        descFieldBound.topLeft.x = wordListBound.topLeft.x + wordListBound.extent.x + 2 * COORD_SPACE;
        descFieldBound.topLeft.y = COORD_START_Y;
        descFieldBound.extent.x = ( toBoundsP->extent.x - COORD_SCROLLBAR_WIDTH ) - descFieldBound.topLeft.x;
        descFieldBound.extent.y = ( toBoundsP->extent.y - COORD_START_Y - COORD_TOOLBAR_HEIGHT );
        FrmSetObjectBounds( frmP, descFieldIndex, &descFieldBound );

        descScrollBarBound.topLeft.x = descFieldBound.topLeft.x + descFieldBound.extent.x;
        descScrollBarBound.topLeft.y = descFieldBound.topLeft.y;
        descScrollBarBound.extent.x = COORD_SCROLLBAR_WIDTH;
        descScrollBarBound.extent.y = descFieldBound.extent.y;
        FrmSetObjectBounds ( frmP, descScrollBarIndex, &descScrollBarBound );

        // Set the label of the up and down button.
        ctlP = GetObjectPtr ( MainWordListUpRepeatButton );
        CtlGlueSetFont ( ctlP, symbol7Font );
        FrmCopyLabel ( frmP, MainWordListUpRepeatButton, "\1" );

        ctlP = GetObjectPtr ( MainWordListDownRepeatButton );
        CtlGlueSetFont ( ctlP, symbol7Font );
        FrmCopyLabel ( frmP, MainWordListDownRepeatButton, "\2" );

        ShowObject( frmP, MainWordListUpRepeatButton );
        ShowObject( frmP, MainWordListDownRepeatButton );
        ShowObject( frmP, MainWordList );
        ShowObject( frmP, MainDescriptionField );
        ShowObject( frmP, MainDescScrollBar );
    }

    fldP = ( FieldType* ) FrmGetObjectPtr ( frmP, descFieldIndex );
    FldRecalculateField ( fldP, false );

    return errNone;
}

/*
 * FUNCTION: MainFormPutWordFieldToHistory
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static Err MainFormHistorySeekBack( void )
{
    UInt16	oldIdx, i;
    AppGlobalType	*global;

    global = AppGetGlobal();

    // check history list to adjust current history index.
    i = 0;
    while ( i <= global->historySeekIdx
            && global->prefs.history[ i ][ 0 ] != chrNull )
    {
        i++;
    }

    if ( i < global->historySeekIdx
            || global->prefs.history[ global->historySeekIdx ][ 0 ] == chrNull )
    {
        global->historySeekIdx = 0;
    }

    // seek next.
    oldIdx = global->historySeekIdx;

    // loop back buffer.
    global->historySeekIdx++;
    if ( global->historySeekIdx >= MAX_HIS_FAR )
        global->historySeekIdx = 0;

    if ( global->prefs.history[ global->historySeekIdx ][ 0 ] == chrNull )
        global->historySeekIdx = 0;

    if ( global->prefs.history[ global->historySeekIdx ][ 0 ] == chrNull
            || oldIdx == global->historySeekIdx )
    {
        // if history is empty or only one item in history then we do nothing.
        SndPlaySystemSound( sndWarning );
        return ~errNone;
    }

    // put history word to search, but not put it to history list.
    ToolsSetFieldPtr ( MainWordField, &global->prefs.history[ global->historySeekIdx ][ 0 ],
                       StrLen( &global->prefs.history[ global->historySeekIdx ][ 0 ] ), true );
    MainFormSearch( false, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );

    return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	MainFormSearch
 *
 * DESCRIPTION: This routine search the word that in word field.
 *
 * PARAMETERS:
 *				->	putinHistory If be true then put the word to history list.
 *				->	updateWordList If be true then update word list.
 *				->	highlightWordField If be true then highlight the word field.
 *				->	updateDescField If be true then update descript field.
 *				->	bAppend If be true then append else replace.
 *
 * RETURN:
 *				errNone if success.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err MainFormSearch( Boolean putinHistory, Boolean updateWordList,
                           Boolean highlightWordField, Boolean updateDescField,
                           Boolean bEnableBreak, Boolean bEnableAutoSpeech )
{
    UInt16	matchs;
    UInt8	*explainPtr;
    UInt32	explainLen;
    AppGlobalType	*global;
    ZDicDBDictInfoType	*dictInfo;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    if ( bEnableBreak )
    {
        Int32 ticksPreHalfSecond;
        Int16 delayTimes;

        ticksPreHalfSecond = SysTicksPerSecond() / TIMES_PRE_SECOND;
        delayTimes = global->prefs.incSearchDelay;
        while ( !EvtSysEventAvail ( true )
                && delayTimes > 0 )
        {
            Int16 x, y;
            Boolean penDown;

            EvtGetPen ( &x, &y, &penDown );
            if ( penDown )
                break;

            SysTaskDelay ( ticksPreHalfSecond );
            delayTimes--;
        }

        if ( delayTimes > 0 )
        {
            global->needSearch = true;
            global->putinHistory = putinHistory;
            global->updateWordList = updateWordList;
            global->highlightWordField = highlightWordField;
            global->updateDescField = updateDescField;
            return errNone;
        }
        else
            global->needSearch = false;
    }

    err = ToolsSearch( MainWordField, &matchs, &explainPtr, &explainLen );
    if ( err == errNone && explainPtr != NULL && explainLen != 0 )
    {
        FormType * frmP;
        MemHandle	bufH;
        Char *str;

        // Clear the menu status from the display
        MenuEraseStatus( 0 );

        if ( putinHistory )
        {
            ToolsPutWordFieldToHistory( MainWordField );
        }

        if ( updateDescField )
        {
            bufH = MemHandleNew( explainLen + 1 );
            if ( bufH == NULL )
            {
                FrmAlert ( NoEnoughMemoryAlert );
                return memErrNotEnoughSpace;
            }

            // format string.
            str = MemHandleLock( bufH );
            MemMove( str, explainPtr, explainLen );
            str[ explainLen ] = chrNull;
            ToolsFormatExplain( ( UInt8* ) str );
            ToolsGMXTranslate( str, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );
            MemHandleUnlock( bufH );

            // update display.
            ToolsSetFieldHandle( MainDescriptionField, bufH, true );
            ToolsUpdateScrollBar ( MainDescriptionField, MainDescScrollBar );
        }

        if ( updateWordList )
            MainFormUpdateWordList();

        // select all the text in word field, then use can clear it easy.
        if ( highlightWordField )
            ToolsHighlightField( MainWordField );

        // Display or hide the player button
        frmP = FrmGetActiveForm ();
        if ( ZDicVoiceIsExist ( ( Char * ) explainPtr ) )
        {
            ShowObject ( frmP, MainPlayVoice );
            if ( bEnableAutoSpeech )
                ToolsPlayVoice ();
        }
        else
            HideObject ( frmP, MainPlayVoice );
    }

    return errNone;
}

/*
 * FUNCTION: MainFormChangeWordFieldCase
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */
static Err MainFormChangeWordFieldCase( void )
{
    FieldType	* fieldP;
    Char	*wordStr, newStr[ MAX_WORD_LEN + 1 ];
    Err	err;
    AppGlobalType *global;

    global = AppGetGlobal();

    fieldP = GetObjectPtr( MainWordField );
    wordStr = FldGetTextPtr ( fieldP );
    if ( wordStr == NULL || wordStr[ 0 ] == chrNull )
        return errNone;

    MemSet( newStr, sizeof( newStr ), 0 );
    StrNCopy( newStr, wordStr, MAX_WORD_LEN );

    err = ToolsChangeCase ( newStr );
    if ( err == errNone )
    {
        ToolsSetFieldPtr ( MainWordField, newStr, StrLen( newStr ), true );
        MainFormSearch( false, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );
    }

    return errNone;
}

/*
 * FUNCTION: MainFormWordListPageScroll
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:   direction     winUp or winDown
 *
 * RETURNED:    nothing
 *
 */

static void MainFormWordListPageScroll ( WinDirectionType direction, Int16 selectItemNum )
{
    ListType	* listP;
    AppGlobalType	*global;
    Err	err;

    global = AppGetGlobal();

    if ( direction == winUp )
    {
        err = ZDicLookupWordListInit( winUp, false );

    }
    else
    {
        err = ZDicLookupWordListInit( winDown, false );
    }

    if ( err )
        return ;

    listP = ( ListType* ) GetObjectPtr( MainWordList );
    LstSetListChoices ( listP, &global->wordlistBuf.itemPtr[ 0 ], global->wordlistBuf.itemUsed );
    LstSetSelection ( listP, selectItemNum );
    LstDrawList ( listP );

}

/*
 * FUNCTION: MainFormSelectWordList
 *
 * DESCRIPTION: when user select a word from word list, we
 *				should display it in descript field.
 *
 * PARAMETERS: event.
 *
 * RETURNED: true if the field handled the event
 *
 */

static Boolean MainFormSelectWordList( EventType * event )
{
    UInt8	* explainPtr;
    UInt32	explainLen;
    AppGlobalType	*global;
    ZDicDBDictInfoType	*dictInfo;
    Err	err;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    if ( event->data.lstSelect.selection >= 0
            && event->data.lstSelect.selection < MAX_WORD_ITEM )
    {
        err = ZDicLookupWordListSelect( event->data.lstSelect.selection, &explainPtr, &explainLen );
        if ( err == errNone )
        {
            FormType * frmP;
            MemHandle	bufH;
            Char *str;
            Int16	len;

            bufH = MemHandleNew( explainLen + 1 );
            if ( bufH == NULL )
                return memErrNotEnoughSpace;

            // format string.
            str = MemHandleLock( bufH );
            MemMove( str, explainPtr, explainLen );
            str[ explainLen ] = chrNull;
            ToolsFormatExplain( ( UInt8* ) str );
            ToolsGMXTranslate( str, dictInfo->phonetic[ dictInfo->curMainDictIndex ] );
            MemHandleUnlock( bufH );

            // update display.
            ToolsSetFieldHandle( MainDescriptionField, bufH, true );
            ToolsUpdateScrollBar ( MainDescriptionField, MainDescScrollBar );

            // update input field
            // get key word.
            len = 0;
            while ( explainPtr[ len ] != chrHorizontalTabulation && explainPtr[ len ] != chrNull && len < MAX_WORD_LEN )
            {
                global->data.readBuf[ len ] = explainPtr[ len ];
                len++;
            }
            global->data.readBuf[ len ] = chrNull;
            ToolsSetFieldPtr( MainWordField, ( char * ) global->data.readBuf, len, true );

            if ( global->prefs.enableHighlightWord )
                ToolsHighlightField( MainWordField );

            // Display or hide the player button
            frmP = FrmGetActiveForm ();
            if ( ZDicVoiceIsExist ( ( Char * ) explainPtr ) )
            {
                ShowObject ( frmP, MainPlayVoice );
                if ( global->prefs.enableAutoSpeech )
                    ToolsPlayVoice ();
            }
            else
                HideObject ( frmP, MainPlayVoice );
        }
    }

    return true;
}

/*
 * FUNCTION: MainFormUpdateWordList
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static Err MainFormUpdateWordList( void )
{
    AppGlobalType	* global;
    ListType	*listP;
    Err	err;

    global = AppGetGlobal();

    if ( global->wordListIsOn )
    {
        err = ZDicLookupWordListInit( winDown, true );
        if ( err == errNone )
        {
            // if description is changed so word list must changed. else do nothing.
            listP = ( ListType* ) GetObjectPtr( MainWordList );
            LstSetListChoices ( listP, &global->wordlistBuf.itemPtr[ 0 ], global->wordlistBuf.itemUsed );
            LstSetSelection ( listP, 0 );
            LstDrawList ( listP );
        }

    }
    return errNone;
}

/*
 * FUNCTION: MainFormPopupHistoryList
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */

static Err MainFormPopupHistoryList( void )
{
    AppGlobalType	* global;
    UInt16	i, count;
    MemHandle	itemsHandle;
    Char	**itemsPtr;
    ListType	*listP;
    Int16	newSelection;

    global = AppGetGlobal();

    // get history list item number;
    count = 0;
    while ( global->prefs.history[ count ][ 0 ] != chrNull )
        count++;

    if ( count == 0 )
        return errNone;
    itemsHandle = MemHandleNew( sizeof( Char* ) * count );
    if ( itemsHandle == NULL )
        return memErrNotEnoughSpace;
    itemsPtr = MemHandleLock ( itemsHandle );
    for ( i = 0; i < count; i++ )
    {
        *( itemsPtr + i ) = &global->prefs.history[ i ][ 0 ];
    }

    listP = GetObjectPtr( MainHistoryPopupList );
    LstSetListChoices ( listP, itemsPtr, count );
    LstSetHeight ( listP, count );
    LstSetSelection ( listP, noListSelection );
    LstGlueSetFont ( listP, global->font.smallFontID );
    newSelection = LstPopupList ( listP );

    if ( newSelection != noListSelection )
    {
        ToolsSetFieldPtr ( MainWordField, &global->prefs.history[ newSelection ][ 0 ],
                           StrLen( &global->prefs.history[ newSelection ][ 0 ] ), true );
        MainFormSearch( false, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );
    }
    MemHandleUnlock( itemsHandle );
    MemHandleFree( itemsHandle );

    return errNone;
}

/*
 * FUNCTION: MainFormChangeDictionary
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 */ 
/*
static Err MainFormChangeDictionary(void)
{
	ZDicDBDictInfoType	*dictInfo;
	AppGlobalType		*global;
	ListType			*listP;
	Int16				newSelection, i;
 
	global = AppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
	
	listP = (ListType*)GetObjectPtr(MainDictionaryPopList);
	if (listP)
	{
		// set the list item pointer.
		i = 0;
		while (i < dictInfo->itemNumber)
		{
			global->listItem[i] = &dictInfo->displayName[i][0];
			i++;
		}
 
		// popup dictionary list.
		LstSetListChoices (listP, global->listItem, dictInfo->itemNumber);
		LstSetSelection (listP, dictInfo->curMainDictIndex);
		LstSetHeight (listP, dictInfo->itemNumber);
		newSelection = LstPopupList (listP);
		
		if (newSelection != noListSelection)
		{
			ZDicCloseCurrentDict();
			dictInfo->curMainDictIndex = newSelection;
			FrmUpdateForm(MainForm, updateDictionaryChanged);
		}
	}
	
	return errNone;
}
*/ 
/*
 * FUNCTION: MainFormIncrementalSearch
 *
 * DESCRIPTION: Adds a character to word field and looks up the word.
 *
 * PARAMETERS: event - EventType* containing character to add to word field.
 *
 * RETURNED: true if the field handled the event
 *
 */

static Boolean MainFormIncrementalSearch( EventType * event )
{

    FormType	* frmP;
    UInt16	fldIndex;
    FieldPtr	fldP;
    AppGlobalType	*global;

    global = AppGetGlobal();

    frmP = FrmGetActiveForm();
    fldIndex = FrmGetObjectIndex( frmP, MainWordField );
    FrmSetFocus( frmP, fldIndex );
    fldP = FrmGetObjectPtr ( frmP, fldIndex );

    if ( FldHandleEvent ( fldP, event ) || event->eType == fldChangedEvent )
    {
        if ( global->prefs.enableIncSearch )
        {
            MainFormSearch( false, true, false, true, true, global->prefs.enableAutoSpeech );
        }

        return true;
    }

    return false;
}

/***********************************************************************
 *
 * FUNCTION:	MainFormJumpSearch
 *
 * DESCRIPTION: Jump search a word if user select a word.
 *
 * PARAMETERS:
 *				->	event The Event that we received.
 *
 * RETURN:
 *				true if handled else false.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Boolean MainFormJumpSearch( EventType * event )
{
    FormType	* frmP;
    Char	*buf;
    FieldType	*field;
    AppGlobalType	*global;
    Boolean	handled = false;

    if ( event->data.fldEnter.fieldID != MainDescriptionField )
        return false;

    field = ( FieldType * ) GetObjectPtr( MainDescriptionField );
    if ( FldHandleEvent ( field, event ) )
    {
        global = AppGetGlobal();
        buf = ( Char * ) global->data.readBuf;

        if ( global->prefs.enableJumpSearch )
        {
            // put old word into history and search new word and put it into history.
            ToolsPutWordFieldToHistory( MainWordField );
            if ( ToolsGetFieldHighlightText( field, buf, MAX_WORD_LEN, global->prefs.enableSingleTap ) == errNone )
            {
                ToolsSetFieldPtr( MainWordField, buf, StrLen( buf ), true );
                MainFormSearch( true, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );

                // Set force to input field.
                frmP = FrmGetActiveForm ();
                FrmSetFocus( frmP, FrmGetObjectIndex( frmP, MainWordField ) );
            }
        }
        handled = true;
    }

    return handled;
}

/***********************************************************************
 *
 * FUNCTION:	MainFormUpdateDisplay
 *
 * DESCRIPTION: This routine redraw the MainForm form.
 *
 * PARAMETERS:
 *				->	updateCode Update code.
 *
 * RETURN:
 *				nothing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static void MainFormUpdateDisplay( UInt16 updateCode )
{
    FormType * frmP;
    ZDicDBDictInfoType	*dictInfo;
    AppGlobalType	*global;
    Err	err = errNone;

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    if ( updateCode & updateDictionaryChanged )
    {
        // Open new dictionary database.
        err = ZDicOpenCurrentDict();
        if ( err != errNone )
        {
            ToolsQuitApp();
            return ;
        }

        // Update dictionary name and research current word.
        StrCopy( global->dictTriggerName, &dictInfo->displayName[ dictInfo->curMainDictIndex ][ 0 ] );
        ToolsFontTruncateName( global->dictTriggerName, stdFont, ResLoadConstant( DictionaryNameMaxLenInPixels ) );
        frmP = FrmGetActiveForm ();
        FrmCopyTitle ( frmP, global->dictTriggerName );
        MainFormSearch( false, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );
    }

    if ( updateCode & updateFontChanged )
    {
        FieldType * field;

        field = ( FieldType* ) GetObjectPtr( MainDescriptionField );
        FldSetFont( field, ( global->prefs.font == eZDicFontSmall ? global->font.smallFontID : global->font.largeFontID ) );

        ToolsUpdateScrollBar ( MainDescriptionField, MainDescScrollBar );
    }

    if ( updateCode & frmRedrawUpdateCode )
    {
        FrmDrawForm( FrmGetActiveForm() );
    }

    return ;
}

/*
 * FUNCTION: MainFormWordListUseAble
 *
 * DESCRIPTION: This routine display wordlist or hide it.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void MainFormWordListUseAble( Boolean turnOver, Boolean redraw )
{
    FormType	* frmP;
    RectangleType formBounds;
    AppGlobalType	*global;

    global = AppGetGlobal();

    frmP = FrmGetActiveForm();

    // get the current bounds for the form
    ZDicToolsWinGetBounds ( FrmGetWindowHandle ( frmP ), &formBounds );

    if ( turnOver )
    {
        global->wordListIsOn = !global->wordListIsOn;
    }

    MainFormAdjustObject ( &formBounds );

    if ( redraw )
    {
        MemHandle	textH;
        FieldType	*fldP;

        if ( global->wordListIsOn )
            ShowObject ( frmP, MainWordList );

        fldP = GetObjectPtr ( MainDescriptionField );
        textH = FldGetTextHandle( fldP );
        FldSetTextHandle ( fldP, NULL );
        FldSetTextHandle ( fldP, textH );
        FldDrawField( fldP );
        MainFormUpdateWordList();
        ToolsUpdateScrollBar ( MainDescriptionField, MainDescScrollBar );
    }

    return ;
}

/*
 * FUNCTION: MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void MainFormInit( FormType *frmP )
{
    ZDicDBDictInfoType	* dictInfo;
    AppGlobalType	*global;
    FieldType	*field;
    ListType	*lst;

    // Set the draw and acitve windows to fixe bugs of chinese os.
    WinSetDrawWindow ( FrmGetWindowHandle ( frmP ) );
    WinSetActiveWindow ( FrmGetWindowHandle ( frmP ) );

    global = AppGetGlobal();
    dictInfo = &global->prefs.dictInfo;

    global->historySeekIdx = 0;

    // initial word list draw function.
    lst = ( ListType * ) GetObjectPtr( MainWordList );
    LstGlueSetFont ( lst, global->font.smallFontID );

    // initial dictionary name
    StrCopy( global->dictTriggerName, &dictInfo->displayName[ dictInfo->curMainDictIndex ][ 0 ] );
    ToolsFontTruncateName( global->dictTriggerName, stdFont, ResLoadConstant( DictionaryNameMaxLenInPixels ) );
    FrmCopyTitle ( frmP, global->dictTriggerName );

    // initial description and word field font.
    field = GetObjectPtr( MainDescriptionField );
    FldSetFont( field, ( global->prefs.font == eZDicFontSmall ? global->font.smallFontID : global->font.largeFontID ) );
    field = GetObjectPtr( MainWordField );
    FldSetFont( field, global->font.smallFontID );

    // Initial word for word field.
    if ( global->initKeyWord[ 0 ] != chrNull )
    {
        ToolsSetFieldPtr( MainWordField, &global->initKeyWord[ 0 ], StrLen( &global->initKeyWord[ 0 ] ), false );
    }
    else
    {
        ToolsSetFieldPtr( MainWordField, &global->initKeyWord[ 0 ], 2, false );
    }
    FrmSetFocus( frmP, FrmGetObjectIndex( frmP, MainWordField ) );

    // Initial word list
    global->wordListIsOn = global->prefs.enableWordList;
    MainFormWordListUseAble( false, false );

    // Set enable jump search group selection.
    FrmSetControlGroupSelection ( frmP, 1,
                                  global->prefs.enableJumpSearch ?
                                  MainJumpPushButton : MainSelectPushButton );

    return ;
}

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean MainFormDoCommand( UInt16 command )
{
    Boolean handled = false;
    AppGlobalType *	global;

    global = AppGetGlobal();

    if ( command >= ZDIC_DICT_MENUID )
    {
        handled = ToolsChangeDictionaryCommand( MainForm, command );
        return handled;
    }

    switch ( command )
    {
    case OptionsAboutZDic:
        {
            FormType * frmP;

            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the About Box.
            frmP = FrmInitForm ( AboutForm );
            FrmDoDialog ( frmP );
            FrmDeleteForm ( frmP );

            handled = true;
            break;
        }
    case OptionsDictOptions:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the preference from the display
            PrefFormPopupPrefsDialog();
            handled = true;
            break;
        }
    case OptionsPreferences:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display the details from the display
            DetailsFormPopupDetailsDialog();
            handled = true;
            break;
        }
    case OptionsExportOptions:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            // Display exorpt options from.
            ExportPopupDialog();
            handled = true;
            break;
        }
    case OptionsGotoWord:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            MainFormSearch( true, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );
            handled = true;
            break;
        }
    case OptionsClearWord:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            handled = ToolsClearInput ( MainWordField );
            break;
        }
    case OptionsClearHistory:
        {
            global->prefs.history[ 0 ][ 0 ] = chrNull;
            global->historySeekIdx = 0;
            handled = true;
            break;
        }
    case OptionsChangeCase:
        {
            // Clear the menu status from the display
            MenuEraseStatus( 0 );

            MainFormChangeWordFieldCase();
            handled = true;
            break;
        }
    case OptionsList:
        {
            AppGlobalType	*global;

            // Clear the menu status from the display
            MenuEraseStatus( 0 );


            global = AppGetGlobal();

            if ( !global->prefs.enableWordList && global->wordListIsOn )
                global->prefs.enableWordList = false;
            else
                global->prefs.enableWordList = !global->prefs.enableWordList;

            global->wordListIsOn = global->prefs.enableWordList;
            MainFormWordListUseAble( false, true );
            handled = true;
            break;
        }
    case OptionsExportMemo:
        {
            ExportToMemo ( MainDescriptionField );
            handled = true;
            break;
        }
    case OptionsExportSugarMemo:
        {
            ExportToSugarMemo( MainDescriptionField );
            handled = true;
            break;
        }
    case DictionarysDictAll:
        {
            ToolsAllDictionaryCommand( MainWordField, MainDescriptionField, MainDescScrollBar );
            break;
        }
    }

    return handled;
}

/*
 * FUNCTION: MainFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "MainForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean MainFormHandleEvent( EventType * eventP )
{
    Boolean	handled = false;
    FormType	*frmP;
    AppGlobalType	*global;

    global = AppGetGlobal();

    switch ( eventP->eType )
    {
    case menuEvent:
        return MainFormDoCommand( eventP->data.menu.itemID );

    case menuOpenEvent:
        {
            ToolsCreatDictionarysMenu ();
            // don't set handled = true
            break;
        }

    case frmOpenEvent:

        // Do not draw anything before FrmDrawForm because
        // FrmDrawForm will save current screen.
        frmP = FrmGetActiveForm();
        ZDicDIAFormLoadInitial ( global, frmP );
        MainFormInit( frmP );
        ZDicDIADisplayChange ( global );
        FrmDrawForm ( frmP );

        MainFormSearch( true, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );

        handled = true;
        break;

    case frmUpdateEvent:
        /*
         * To do any custom drawing here, first call
         * FrmDrawForm(), then do your drawing, and
         * then set handled to true. 
         */
        MainFormUpdateDisplay( eventP->data.frmUpdate.updateCode );
        handled = true;
        break;

    case frmCloseEvent:
        ToolsSetFieldHandle( MainDescriptionField, NULL, false );
        ToolsSetFieldHandle( MainWordField, NULL, false );
        ZDicDIAFormClose ( global );
        break;

    case winDisplayChangedEvent:
        handled = ZDicDIADisplayChange ( global );
        if ( handled )
        {
            frmP = FrmGetActiveForm ();
            FrmDrawForm ( frmP );
        }
        break;

    case winEnterEvent:
        ZDicDIAWinEnter ( global, eventP );
        handled = true;
        break;

    case ctlSelectEvent:
        {
            if ( eventP->data.ctlSelect.controlID == MainNewButton )
            {
                handled = ToolsClearInput ( MainWordField );
            }

            else if ( eventP->data.ctlSelect.controlID == MainGotoButton )
            {
                ToolsAllDictionaryCommand( MainWordField, MainDescriptionField, MainDescScrollBar );
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainExitButton )
            {
                // get history list for seek back.
                ToolsQuitApp();
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainHistoryTrigger )
            {
                MainFormPopupHistoryList();
                handled = true;
            }

            //			else if (eventP->data.ctlSelect.controlID == MainDictionaryTrigger)
            //			{
            //				MainFormChangeDictionary();
            //				handled = true;
            //			}

            else if ( eventP->data.ctlSelect.controlID == MainWordListDownRepeatButton )
            {
                MainFormWordListPageScroll( winDown, 0 );
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainWordListUpRepeatButton )
            {
                MainFormWordListPageScroll( winUp, 0 );
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainWordButton )
            {
                AppGlobalType	* global;

                global = AppGetGlobal();

                if ( !global->prefs.enableWordList && global->wordListIsOn )
                    global->prefs.enableWordList = false;
                else
                    global->prefs.enableWordList = !global->prefs.enableWordList;

                global->wordListIsOn = global->prefs.enableWordList;
                MainFormWordListUseAble( false, true );
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainSelectPushButton
                      || eventP->data.ctlSelect.controlID == MainJumpPushButton )
            {
                global->prefs.enableJumpSearch =
                    eventP->data.ctlSelect.controlID == MainSelectPushButton ? false : true;
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainPlayVoice )
            {
                ToolsPlayVoice ();
                handled = true;
            }

            else if ( eventP->data.ctlSelect.controlID == MainExportToMemo )
            {
                ToolsSendMenuCmd( global->prefs.exportAppCreatorID == sysFileCMemo ? chrCapital_M : chrCapital_Z );
                handled = true;
            }

            break;
        }

    case ctlRepeatEvent:
        {
            switch ( eventP->data.ctlRepeat.controlID )
            {
            case MainWordListUpRepeatButton:
                MainFormWordListPageScroll( winUp, 0 );
                break;

            case MainWordListDownRepeatButton:
                MainFormWordListPageScroll( winDown, 0 );
                break;

            case MainBackHistoryRepeatButton:
                MainFormHistorySeekBack ();
                break;
            }
        }

    case lstSelectEvent:
        if ( eventP->data.ctlSelect.controlID == MainWordList )
        {
            handled = MainFormSelectWordList( eventP );
        }

        break;

    case keyDownEvent:
        {
            if ( eventP->data.keyDown.chr == chrLineFeed )
            {
                MainFormSearch( true, true, global->prefs.enableHighlightWord, true, false, global->prefs.enableAutoSpeech );
                handled = true;
            }

            else if ( EvtKeydownIsVirtual( eventP ) )
            {

                if ( NavDirectionPressed( eventP, Up )
                        || ( eventP->data.keyDown.chr ) == vchrRockerUp
                        || ( eventP->data.keyDown.chr ) == vchrPageUp
                        || ( eventP->data.keyDown.chr ) == vchrThumbWheelUp
                        //	|| (eventP->data.keyDown.chr) == vchrJogUp
                   )
                {
                    ToolsPageScroll ( winUp, MainDescriptionField, MainDescScrollBar, MainWordField, MainPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Down )
                          || ( eventP->data.keyDown.chr ) == vchrRockerDown
                          || ( eventP->data.keyDown.chr ) == vchrPageDown
                          || ( eventP->data.keyDown.chr ) == vchrThumbWheelDown
                          //	|| (eventP->data.keyDown.chr) == vchrJogDown
                        )
                {
                    ToolsPageScroll ( winDown, MainDescriptionField, MainDescScrollBar, MainWordField, MainPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Left ) )
                {
                    ToolsScrollWord ( winUp, MainDescriptionField, MainDescScrollBar, MainWordField, MainPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavDirectionPressed( eventP, Right ) )
                {
                    ToolsScrollWord ( winDown, MainDescriptionField, MainDescScrollBar, MainWordField, MainPlayVoice, global->prefs.enableHighlightWord );
                    handled = true;
                }
                else if ( NavSelectPressed( eventP )
                          || ( eventP->data.keyDown.chr ) == vchrRockerCenter
                          || ( eventP->data.keyDown.chr ) == vchrThumbWheelPush
                          || ( eventP->data.keyDown.chr ) == vchrJogRelease
                        )
                {
                    ToolsOpenNextDict( MainForm );
                    handled = true;
                }
            }

            else
            {
                handled = MainFormIncrementalSearch( eventP );
            }

            break;
        }

    case fldChangedEvent:
        {
            if ( eventP->data.fldChanged.fieldID == MainWordField )
                handled = MainFormIncrementalSearch( eventP );
            else
                ToolsUpdateScrollBar ( MainDescriptionField, MainDescScrollBar );
            break;
        }

    case fldEnterEvent:
        {
            handled = MainFormJumpSearch( eventP );
            break;
        }

    case sclRepeatEvent:
        {
            ToolsScroll ( eventP->data.sclRepeat.newValue -
                          eventP->data.sclRepeat.value, false, MainDescriptionField, MainDescScrollBar );
            break;
        }

        // If there have not any event then we continue the broken search.
        if ( global->needSearch && !EvtSysEventAvail ( true ) )
        {
            MainFormSearch( global->putinHistory, global->updateWordList,
                            global->highlightWordField, global->updateDescField, false, global->prefs.enableAutoSpeech );
        }

    }

    return handled;
}

///////////////////////////////////////////////////////////////////////////
