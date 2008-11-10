/*
 * ZDicLibImpl.c
 *
 * implementation for shared library
 *
 * This wizard-generated code is based on code adapted from the
 * SampleLib project distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */

/* Our library public definitions (library API) */
#define BUILDING_ZDICLIB
#include "ZDicLib_Rsc.h"
#include "ZDicLib.h"
#include "ZDicLibPrivate.h"
#include "Decode.h"

/*********************************************************************
 *
 * LIBRARY GLOBALS:
 *
 * IMPORTANT:
 * ==========
 * Libraries are *not* allowed to have global or static variables.
 * Instead, they allocate a memory chunk to hold their persistent data, 
 * and save a handle to it in the library's system library table entry.
 * Example functions below demostrate how the library "globals" chunk 
 * is set up, saved, and accessed.
 *
 * We use a movable memory chunk for our library globals to minimize
 * dynamic heap fragmentation.  Our library globals are locked only
 * when needed and unlocked as soon as possible.  This behavior is
 * critical for healthy system performance.
 *
 *********************************************************************/
#define OFFSET_RECNUM	2
#define OFFSET_COMPRESS	3
#define OFFSET_HEAD		8


/*********************************************************************
 * Internally used routines
 *********************************************************************/

static ZDicLibGlobalsType * PrvMakeGlobals(UInt16 refNum);
static void PrvFreeGlobals(UInt16 refNum);
static ZDicLibGlobalsType * PrvLockGlobals(UInt16 refNum);
static Boolean PrvIsLibOpen(UInt16 refNum);
static Err PrvCreateClientContext(ZDicLibGlobalsType * gP, UInt32 * clientContextP);
static Err PrvDestroyClientContext(ZDicLibGlobalsType * gP, UInt32 clientContext);
static ZDicLibClientContextType * PrvLockContext(UInt32 context);

static Err PrvZDicLibLookupForward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript );
static Err PrvZDicLibLookupBackward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript );
static Err PrvZDicLibLookupCurrent(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript );
static Err PrvZDicLibDBInitIndexRecord(void);
static Err PrvZDicLibDBInitDictList(UInt32 type, UInt32 creator, Int16* dictCount);

static Int16 PrvZDicStrCompare(const UInt8 *s1, const UInt8 *s2);
static Boolean PrvZDicBlockLookUp(const UInt8 *wordStr, UInt8 *block, UInt16* head, UInt16* tail, UInt16 *same);                      	
static Err PrvZDicDBGetAllDict(UInt32 type, UInt32 creator, ZDicDBDictInfoType *dbListP);
static void PrvZDicDBWriteCache(void *recordP, UInt32 *offset, const void *srcP, UInt32 bytes, UInt8 *cache, UInt32 *cacheIdx);
static Err PrvZDicDBInitIndexRecord(DmOpenRef dbP, const Char *dbName);
static UInt8 PrvZDicVFSGetChar(FileRef fileRef, UInt8 *buf, UInt32 *curIdx);
static void PrvZDicVFSWriteCache(FileRef fileRef, const UInt32 *srcP, UInt32 *cache, UInt32 *cacheIdx);
static Err PrvZDicVFSGetStrList(FileRef fileRef, UInt32 *firstRecordOffset, UInt32 *strListOffset, UInt16 *itemNum, UInt16 *compressFlag);
static void PrvZDicVFSGetIdxName(Char *pathName);
static Err PrvZDicVFSInitIndexFile(UInt16 volRefNum, const Char *fileName);
static Err PrvZDicVFSGetRecordSize(FileRef fileRef, UInt16 recIndex, UInt16 *recSize);
static Err PrvZDicVFSGetRowRecord(FileRef fileRef, UInt16 recIndex,	UInt8 *recBuf, UInt16 *recSize);
static void PrvZDicDBGetAllPopup(ZDicDBDictShortcutInfoType *dbListP);
static UInt16 PrvZDicDBLookup(const UInt8 *wordStr, const UInt8 *strList, const UInt32 *idxList, UInt32 itemNum);
static UInt16 PrvZDicVFSLookup(const UInt8 *wordStr, 
 	FileRef fileRef, UInt32 strListOffset,
	FileRef idxRef, UInt32 idxListOffset, UInt32 itemNum, UInt8 *strBuf, UInt32 strBufLen);
static AppGlobalType * PrvAppGetGlobal(void);
static Err PrvZDicDictGetIdxByWord(const UInt8 *wordStr, UInt16 *index, UInt16 *size);
static Err PrvZDicReleaseDictRecord(UInt8 *recPtr);
static Err PrvZDicGetDictRecord(UInt16 recIndex, UInt8 **recPtr, UInt16 *recSize);
static void PrvZDicDBGetAllShortcut(ZDicDBDictShortcutInfoType *dbListP);
static void PrvDebugPrint(UInt32 z, Int16 x, Int16 y, UInt16 time);

/*********************************************************************
 * Internally used macros
 *********************************************************************/

/* Unlock globals */
#define PrvUnlockGlobals(gP)        MemPtrUnlock(gP)

/* Unlock the client context */
#define PrvUnlockContext(contextP)	MemPtrUnlock(contextP)
 
/*********************************************************************
 * Library API Routines
 *********************************************************************/
 
/*
 * FUNCTION: ZDicLibOpen
 *
 * DESCRIPTION:
 *
 * Opens the library, creates and initializes the globals.
 * This function must be called before any other library functions.
 *
 * If ZDicLibOpen fails, do not call any other library API functions.
 * If ZDicLibOpen succeeds, call ZDicLibClose when you are done using
 * the library to enable it to release critical system resources.
 *
 * LIBRARY DEVELOPER NOTES:
 *
 * The library's "open" and "close" functions should *not* take an excessive
 * amount of time to complete.  If the processing time for either of these
 * is lengthy, consider creating additional library API function(s) to handle
 * the time-consuming chores.
 *
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * clientContextP
 *		pointer to variable for returning client context.  The client context is
 *		used to maintain client-specific data for multiple client support.  The 
 *		value returned here will be used as a parameter for other library 
 *		functions which require a client context.  
 *
 * CALLED BY: anyone who wants to use this library
 *
 * RETURNS:
 *		errNone
 *		memErrNotEnoughSpace
 *
 * SIDE EFFECTS:
 *		*clientContextP will be set to client context on success, or zero on error.
 */

Err ZDicLibOpen(UInt16 refNum, UInt32 *clientContextP)
{
	ZDicLibGlobalsType *gP;
	Err err = errNone;
	Int16 originalOpenCount = 0;

	/* Error-check our parameters */
	ErrFatalDisplayIf(
		clientContextP == NULL, 
		"null context variable pointer");

	/* Initialize return variable */
	*clientContextP = 0;

	/* Get library globals */
	gP = PrvLockGlobals(refNum);

	/* Check if already open */
	if (!gP)
	{
		/* Allocate and initialize our library globals. */
		gP = PrvMakeGlobals(refNum);
		if ( !gP )
			err = memErrNotEnoughSpace;
	}

	/* If we have globals, create a client context, increment open
	 * count, and unlock our globals */
	if ( gP )
	{
		originalOpenCount = gP->openCount;

		err = PrvCreateClientContext(gP, clientContextP);
		if ( !err )
			gP->openCount++;

		PrvUnlockGlobals(gP);

		/* If there was an error creating a client context and there  */
		/* are no other clients, free our globals */
		if ( err && (originalOpenCount == 0) )
			PrvFreeGlobals(refNum);
	}

	return( err );
}

/*
 * FUNCTION: ZDicLibClose
 *
 * DESCRIPTION:	
 *
 * Closes the library, frees client context and globals.
 *
 * ***IMPORTANT***
 * May be called only if ZDicLibOpen succeeded.
 *
 * If other applications still have the library open, decrements
 * the reference count and returns ZDicLibErrStillOpen.
 *
 * LIBRARY DEVELOPER NOTES:
 *
 * The library's "open" and "close" functions should *not* take an excessive
 * amount of time to complete.  If the processing time for either of these
 * is lengthy, consider creating additional library API function(s) to handle
 * the time-consuming chores.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * clientContext
 *		client context
 *
 * CALLED BY: Whoever wants to close the library
 *
 * RETURNS:
 *		errNone
 *		ZDicLibErrStillOpen -- library is still open by others (no error)
 */

Err ZDicLibClose(UInt16 refNum, UInt32 clientContext)
{
	ZDicLibGlobalsType * gP;
	Int16 openCount;
	Int16 contextCount;
	Err err = errNone;

	gP = PrvLockGlobals(refNum);

	/* If not open, return */
	if (!gP)
	{
		/* MUST return zero here to get around a bug in system v1.x that
		 * would cause SysLibRemove to fail. */
		return errNone;
	}

	/* Destroy the client context (we ignore the return code in this implementation) */
	PrvDestroyClientContext(gP, clientContext);

	/* Decrement our library open count */
	gP->openCount--;

	/* Error check for open count underflow */
	ErrFatalDisplayIf(gP->openCount < 0, "ZDicLib open count underflow");

	/* Save the new open count and the context count */
	openCount = gP->openCount;
	contextCount = gP->contextCount;

	PrvUnlockGlobals(gP);

	/* If open count reached zero, free our library globals */
	if ( openCount <= 0 )
	{
		/* Error check to make sure that all client contexts were destroyed */
		ErrFatalDisplayIf(contextCount != 0, "not all client contexts were destroyed");

		/* Free our library globals */
		PrvFreeGlobals(refNum);
	}
	else
	{
		/* return this error code to inform the caller
		 * that others are still using this library */
		err = ZDicLibErrStillOpen;
	}

	return err;
}

/*
 * FUNCTION: ZDicLibSleep
 *
 * DESCRIPTION:
 *
 * Handles system sleep notification.
 *
 * ***IMPORTANT***
 * This notification function is called from a system interrupt.
 * It is only allowed to use system services which are interrupt-
 * safe.  Presently, this is limited to EvtEnqueueKey, SysDisableInts,
 * SysRestoreStatus.  Because it is called from an interrupt,
 * it must *not* take a long time to complete to preserve system
 * integrity.  The intention is to allow system-level libraries
 * to disable hardware components to conserve power while the system
 * is asleep.
 * 
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: System
 *
 * RETURNS:	errNone
 */

Err ZDicLibSleep(UInt16 refNum)
{
	#pragma unused(refNum)
	return errNone;
}

/*
 * FUNCTION: ZDicLibWake
 *
 * DESCRIPTION:
 *
 * Handles system wake notification.
 *
 * ***IMPORTANT***
 * This notification function is called from a system interrupt.
 * It is only allowed to use system services which are interrupt-
 * safe.  Presently, this is limited to EvtEnqueueKey, SysDisableInts,
 * SysRestoreStatus.  Because it is called from an interrupt,
 * it must *not* take a long time to complete to preserve system
 * integrity.  The intention is to allow system-level libraries
 * to enable hardware components which were disabled when the system
 * went to sleep.
 * 
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: System
 *
 * RETURNS:	errNone
 */

Err ZDicLibWake(UInt16 refNum)
{
	#pragma unused(refNum)
	return errNone;
}


#pragma mark -
/////////////////////////////////////////////////////////////////////////////

/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookup
 *
 * DESCRIPTION:	search a word detail by word string.
 *
 * PARAMETERS:	
 *
 * RETURNED:     errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/
Err ZDicLibLookup(UInt16 refNum, const UInt8 *wordStr, UInt16 *matchs, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalType *	global;
	MemHandle		bufH;
	UInt16			blkIndex, match1, match2;
	UInt8			*str;
	UInt16			recSize;
	Boolean			first, tryAgain;
	Err				err;
	
	ErrNonFatalDisplayIf(wordStr == NULL, "Can not find empty string");
	ErrNonFatalDisplayIf((wordStr[0] == 0xff) && (wordStr[1] == 0xff), "Can not find empty string");
	
	*explainPtr = NULL;
	*explainLen = 0;
	*matchs = 0;

	global = PrvAppGetGlobal();
	descript = &global->descript;

	err = PrvZDicDictGetIdxByWord ((UInt8*)wordStr, &blkIndex, &recSize);
	if (err != errNone || recSize == 0)
		return ~errNone;

	tryAgain = true;
	bufH = NULL;
	
FOUND_BLOCK:

	// if the new blkIndex same as old blkIndex then we use old data in buffer directly.
	if (descript->blkIndex != blkIndex)
	{
		// get dict record and decode it to text.
		err = PrvZDicGetDictRecord (blkIndex, &str, &recSize);
		if (err) return err;

		Decoder(global->zlibRefNum, global->compressFlag, (unsigned char *)str, recSize, (unsigned char *)&descript->decodeBuf[0], &descript->decodeSize);
		descript->decodeBuf[descript->decodeSize] = chrNull;
		PrvZDicReleaseDictRecord(str);
		
		descript->blkIndex = blkIndex;
	}

	// find out nearest string from text buffer.	
	first = PrvZDicBlockLookUp(wordStr, descript->decodeBuf, &descript->head, &descript->tail, &match1);
	*explainPtr = descript->decodeBuf + descript->head;
	*explainLen = descript->tail - descript->head;
	*matchs = match1;

	if (first && blkIndex > 1 && tryAgain)
	{
		// if the nearest string is the first string in current block,
		// then we seek back.
		blkIndex--;

		err = PrvZDicGetDictRecord (blkIndex, &str, &recSize);
		if (err) return err;

		Decoder(global->zlibRefNum, global->compressFlag,(unsigned char *)str, recSize, (unsigned char *)&descript->decodeBuf[0], &descript->decodeSize);
		
		descript->decodeBuf[descript->decodeSize] = chrNull;
		PrvZDicReleaseDictRecord(str);

		descript->blkIndex = blkIndex;
		PrvZDicBlockLookUp(wordStr, descript->decodeBuf, &descript->head, &descript->tail, &match2);
		if (match2 >= match1)
		{
			*explainPtr = descript->decodeBuf + descript->head;
			*explainLen = descript->tail - descript->head;
			*matchs = match2;
		}
		else
		{
			blkIndex++;
			tryAgain = false;
			goto FOUND_BLOCK;
		}		
	}

	if (*explainLen < MAX_LINE_LEN)
	{
		// if current is bad word then get next.
		// and do not change matchs.
		err = PrvZDicLibLookupForward(explainPtr, explainLen, descript );
	}
	
	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookupForward
 *
 * DESCRIPTION:	search a next word after call ZDicLookup
 *
 * PARAMETERS:	
 *
 * RETURNED:     errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/

Err ZDicLibLookupForward(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	return PrvZDicLibLookupForward(explainPtr, explainLen, descript);
}


/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookupBackward
 *
 * DESCRIPTION:	search a previouse word after call ZDicLookup
 *
 * PARAMETERS:	
 *
 * RETURNED:     errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/

Err ZDicLibLookupBackward(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	return PrvZDicLibLookupBackward(explainPtr, explainLen, descript);
}


/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookupCurrent
 *
 * DESCRIPTION:	get current word after call ZDicLookup
 *
 * PARAMETERS:	
 *
 * RETURNED:     errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/

Err ZDicLibLookupCurrent(UInt16 refNum, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	return PrvZDicLibLookupCurrent(explainPtr, explainLen, descript);
}

/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookupGetWordList
 *
 * DESCRIPTION:	Get word list.
 *
 * PARAMETERS:
 *
 * RETURNED:    errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/

Err ZDicLibLookupWordListInit(UInt16 refNum, WinDirectionType direction, Boolean init)
{
	AppGlobalType	*global;
	UInt8			*explainPtr;
	UInt32			explainLen;
	UInt8			*p,maxworditem;
	Int16			item, i;
	Err				err = errNone;

	global = PrvAppGetGlobal();
	
	/*if(global->prefs.font == global->font.smallTinyFontID ||global->prefs.font == global->font.largeTinyFontID)
    	maxworditem = MAX_WORD_ITEM_TINY;
    if(global->prefs.font == global->font.smallFontID ||global->prefs.font == global->font.largeFontID)
    */	maxworditem = MAX_WORD_ITEM;

	if (init)
	    global->wordlist = global->descript;
	
	switch (direction)
	{
		case winDown:
		{
			// first we seek backword, but we not use it.
			//err = ZDicLookupBackward(&explainPtr, &explainLen, eWordlist);
			break;
		}
		
		case winUp:
		{
			// first we seek backword two page, but we not use it.
			for (item = 0; item < 2 * maxworditem - 2; item++)
			{
				err = PrvZDicLibLookupBackward(&explainPtr, &explainLen, &global->wordlist);
			}
			break;
		}
	}

	/*if(global->prefs.font == global->font.smallTinyFontID ||global->prefs.font == global->font.largeTinyFontID)
    {
		// Get word list.
		global->wordlisttinyBuf.itemUsed = 0;
		
		for (item = 0; item < maxworditem; item++)
		{
			if (item == 0)
			{
				err = ZDicLookupCurrent(&explainPtr, &explainLen, &global->wordlist);
			}
			else
			{
				err = ZDicLookupForward(&explainPtr, &explainLen, &global->wordlist);
			}
			if (err)
			{
				return item > 0 ? errNone : ~errNone;
			}

			p = explainPtr;

			// get key word.
			i = 0;
			while (p[i] != chrHorizontalTabulation && p[i] != chrNull && i < MAX_WORD_LEN)
			{
				global->wordlisttinyBuf.itemBuf[item][i] = p[i];
				i++;
			}
			global->wordlisttinyBuf.itemBuf[item][i] = chrNull;
			global->wordlisttinyBuf.itemUsed++;
			global->wordlisttinyBuf.itemPtr[item] = &global->wordlisttinyBuf.itemBuf[item][0];
			global->wordlisttinyBuf.itemBlkIndex[item] = global->wordlist.blkIndex;
			global->wordlisttinyBuf.itemHead[item] = global->wordlist.head;
			global->wordlisttinyBuf.itemTail[item] = global->wordlist.tail;
		}
    }
    else if(global->prefs.font == global->font.smallFontID ||global->prefs.font == global->font.largeFontID)
    {*/
    			// Get word list.
		global->wordlistBuf.itemUsed = 0;
		
		for (item = 0; item < maxworditem; item++)
		{
			if (item == 0)
			{
				err = PrvZDicLibLookupCurrent(&explainPtr, &explainLen, &global->wordlist);
			}
			else
			{
				err = PrvZDicLibLookupForward(&explainPtr, &explainLen, &global->wordlist);
			}
			if (err)
			{
				return item > 0 ? errNone : ~errNone;
			}

			p = explainPtr;

			// get key word.
			i = 0;
			while (p[i] != chrHorizontalTabulation && p[i] != chrNull && i < MAX_WORD_LEN)
			{
				global->wordlistBuf.itemBuf[item][i] = p[i];
				i++;
			}
			global->wordlistBuf.itemBuf[item][i] = chrNull;
			global->wordlistBuf.itemUsed++;
			global->wordlistBuf.itemPtr[item] = &global->wordlistBuf.itemBuf[item][0];
			global->wordlistBuf.itemBlkIndex[item] = global->wordlist.blkIndex;
			global->wordlistBuf.itemHead[item] = global->wordlist.head;
			global->wordlistBuf.itemTail[item] = global->wordlist.tail;
		}

    //}
    /*
	// Get word list.
	global->wordlistBuf.itemUsed = 0;
	
	for (item = 0; item < maxworditem; item++)
	{
		if (item == 0)
		{
			err = ZDicLookupCurrent(&explainPtr, &explainLen, &global->wordlist);
		}
		else
		{
			err = ZDicLookupForward(&explainPtr, &explainLen, &global->wordlist);
		}
		if (err)
		{
			return item > 0 ? errNone : ~errNone;
		}

		p = explainPtr;

		// get key word.
		i = 0;
		while (p[i] != chrHorizontalTabulation && p[i] != chrNull && i < MAX_WORD_LEN)
		{
			global->wordlistBuf.itemBuf[item][i] = p[i];
			i++;
		}
		global->wordlistBuf.itemBuf[item][i] = chrNull;
		global->wordlistBuf.itemUsed++;
		global->wordlistBuf.itemPtr[item] = &global->wordlistBuf.itemBuf[item][0];
		global->wordlistBuf.itemBlkIndex[item] = global->wordlist.blkIndex;
		global->wordlistBuf.itemHead[item] = global->wordlist.head;
		global->wordlistBuf.itemTail[item] = global->wordlist.tail;
	}
	*/
	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:     ZDicLibLookupWordListSelect
 *
 * DESCRIPTION:	
 *
 * PARAMETERS:
 *
 * RETURNED:    errNone if found else not fount
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh			19/07/04	Initial Revision
 *
 ***********************************************************************/

Err ZDicLibLookupWordListSelect(UInt16 refNum, UInt16 select, UInt8 **explainPtr, UInt32 *explainLen)
{
	UInt8			*str;
	UInt16			recSize;
	AppGlobalType	*global;
	Err				err;
	
	global = PrvAppGetGlobal();

	/*if(global->prefs.font == global->font.smallTinyFontID ||global->prefs.font == global->font.largeTinyFontID)
    {
		if (select >= global->wordlisttinyBuf.itemUsed)
			return ~errNone;

		if (global->wordlisttinyBuf.itemBlkIndex[select] != global->descript.blkIndex)
		{

			// get dict record and decode it to text.
			err = ZDicGetDictRecord (global->wordlisttinyBuf.itemBlkIndex[select], &str, &recSize);
			if (err) return err;
		

			Decoder(global->compressFlag,(unsigned char *)str, recSize, (unsigned char *)&global->descript.decodeBuf[0], &global->descript.decodeSize);
			
			global->descript.decodeBuf[global->descript.decodeSize] = chrNull;
			ZDicReleaseDictRecord(str);
			
			global->descript.blkIndex = global->wordlisttinyBuf.itemBlkIndex[select];		
		}
		
		global->descript.head = global->wordlisttinyBuf.itemHead[select];
		global->descript.tail = global->wordlisttinyBuf.itemTail[select];
    }
    else if(global->prefs.font == global->font.smallFontID ||global->prefs.font == global->font.largeFontID)
    {*/
    	if (select >= global->wordlistBuf.itemUsed)
			return ~errNone;

		if (global->wordlistBuf.itemBlkIndex[select] != global->descript.blkIndex)
		{

			// get dict record and decode it to text.
			err = PrvZDicGetDictRecord (global->wordlistBuf.itemBlkIndex[select], &str, &recSize);
			if (err) return err;
		

			Decoder(global->zlibRefNum, global->compressFlag,(unsigned char *)str, recSize, (unsigned char *)&global->descript.decodeBuf[0], &global->descript.decodeSize);
			
			global->descript.decodeBuf[global->descript.decodeSize] = chrNull;
			PrvZDicReleaseDictRecord(str);
			
			global->descript.blkIndex = global->wordlistBuf.itemBlkIndex[select];		
		}
		
		global->descript.head = global->wordlistBuf.itemHead[select];
		global->descript.tail = global->wordlistBuf.itemTail[select];

    //}
/*
	if (select >= global->wordlistBuf.itemUsed)
		return ~errNone;

	if (global->wordlistBuf.itemBlkIndex[select] != global->descript.blkIndex)
	{

		// get dict record and decode it to text.
		err = ZDicGetDictRecord (global->wordlistBuf.itemBlkIndex[select], &str, &recSize);
		if (err) return err;
	
		if (global->compressFlag == 0)
		{
			MemMove (&global->descript.decodeBuf[0], str, recSize);
			global->descript.decodeSize = recSize;
		}
		else
		{
			LZSS_Decoder((unsigned char *)str, recSize, (unsigned char *)&global->descript.decodeBuf[0], &global->descript.decodeSize);
		}
		global->descript.decodeBuf[global->descript.decodeSize] = chrNull;
		ZDicReleaseDictRecord(str);
		
		global->descript.blkIndex = global->wordlistBuf.itemBlkIndex[select];		
	}
	
	global->descript.head = global->wordlistBuf.itemHead[select];
	global->descript.tail = global->wordlistBuf.itemTail[select];
*/
	*explainPtr = &global->descript.decodeBuf[global->descript.head];
	*explainLen = global->descript.tail - global->descript.head;

	return errNone;
}

#pragma mark -






/***********************************************************************
 *
 * FUNCTION: ZDicLibDBInitIndexRecord
 *
 * DESCRIPTION: Initial all index of dictionarys that in ram and card.
 *
 * PARAMETERS:
 *				nothing.
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

Err ZDicLibDBInitIndexRecord(UInt16 refNum)
{
	return PrvZDicLibDBInitIndexRecord();
}

Err ZDicLibDBInitPopupList(UInt16 refNum)
{
	AppGlobalType				*global;
	ZDicDBDictInfoType 			*dictInfo;
	ZDicDBDictShortcutInfoType		*dblist, *popupInfo;
	UInt16						i,j;
	Int16						counter;
	
	global = PrvAppGetGlobal();
	dblist = &global->data.popupInfoList;
	dictInfo = &global->prefs.dictInfo;
	popupInfo = &global->prefs.popupInfo;
	
	popupInfo->totalNumber = 0;
	
	PrvZDicDBGetAllPopup(dblist);
	
	if (dblist != NULL && dblist->totalNumber > 0)
	{
		//从旧list里提取出新list也有的，并往前排列
		for (i = 0, j = 0; i < MAX_DICT_NUM && popupInfo->dictIndex[i] != -1; i++)//i遍历旧list
		{
			for (counter = 0; counter < dblist->totalNumber; counter++)//counter遍历临时list
			{
				if (popupInfo->dictIndex[i] == dblist->dictIndex[counter])//发现相同则break
					break;
			}
			
			if (counter < dblist->totalNumber)//如果之前一步后的counter在新list的中间某处
			{
				dblist->dictIndex[counter] = -1;//临时list，非新增项标注为-1
				if (i != j)//j比i小时，说明旧list里有新list里所没有的，那把后面的复制到前面去。
				{
					popupInfo->dictIndex[j] = popupInfo->dictIndex[i];
				}
				
				j++;//新list标号前进一个
			}
		}
		
		//旧list里末尾添加新list里新增的
		if (j < MAX_DICT_NUM)
		{
			for (counter = 0; counter < dblist->totalNumber && j < MAX_DICT_NUM; counter++)
			{
				if (dblist->dictIndex[counter] != -1)
				{
					popupInfo->dictIndex[j] = dblist->dictIndex[counter];
					j++;
				}
			}	
		}
		
		//如果没达到上限，则给最后一项后面赋-1
		if (j < MAX_DICT_NUM)
			popupInfo->dictIndex[j] = -1;
		
		
		//
		i = 0;
		while (popupInfo->dictIndex[i] != -1 &&  i < MAX_DICT_NUM)
		{
			global->popuplistItem[i] = &dictInfo->displayName[popupInfo->dictIndex[i]][0];
			i++;
		}
		
		popupInfo->totalNumber = i;
	}

	if (popupInfo->curIndex >= popupInfo->totalNumber)
		popupInfo->curIndex = 0;
		
	return errNone;		
}


Err ZDicLibDBInitShortcutList(UInt16 refNum)
{
	AppGlobalType				*global;
	ZDicDBDictInfoType 			*dictInfo;
	ZDicDBDictShortcutInfoType	*dblist, *shortcutInfo;
	UInt16						i,j;
	Int16						counter;
	
	global = PrvAppGetGlobal();
	dblist = &global->data.shortcutInfoList;
	dictInfo = &global->prefs.dictInfo;
	shortcutInfo = &global->prefs.shortcutInfo;
	
	shortcutInfo->totalNumber = 0;
	
	PrvZDicDBGetAllShortcut(dblist);
	
	if (dblist != NULL && dblist->totalNumber > 0)
	{
		//从旧list里提取出新list也有的，并往前排列
		for (i = 0, j = 0; i < MAX_DICT_NUM && shortcutInfo->dictIndex[i] != -1; i++)//i遍历旧list
		{
			for (counter = 0; counter < dblist->totalNumber; counter++)//counter遍历临时list
			{
				if (shortcutInfo->dictIndex[i] == dblist->dictIndex[counter])//发现相同则break
					break;
			}
			
			if (counter < dblist->totalNumber)//如果之前一步后的counter在新list的中间某处
			{
				dblist->dictIndex[counter] = -1;//临时list，非新增项标注为-1
				if (i != j)//j比i小时，说明旧list里有新list里所没有的，那把后面的复制到前面去。
				{
					shortcutInfo->dictIndex[j] = shortcutInfo->dictIndex[i];
				}
				
				j++;//新list标号前进一个
			}
		}
		
		//旧list里末尾添加新list里新增的
		if (j < MAX_DICT_NUM)
		{
			for (counter = 0; counter < dblist->totalNumber && j < MAX_DICT_NUM; counter++)
			{
				if (dblist->dictIndex[counter] != -1)
				{
					shortcutInfo->dictIndex[j] = dblist->dictIndex[counter];
					j++;
				}
			}	
		}
		
		//如果没达到上限，则给最后一项后面赋-1
		if (j < MAX_DICT_NUM)
			shortcutInfo->dictIndex[j] = -1;
		
		
		//
		i = 0;
		while (shortcutInfo->dictIndex[i] != -1 &&  i < MAX_DICT_NUM)
		{
			global->shortcutlistItem[i] = &dictInfo->displayName[shortcutInfo->dictIndex[i]][0];
			i++;
		}
		
		shortcutInfo->totalNumber = i;
	}

	if (shortcutInfo->curIndex >= shortcutInfo->totalNumber)
		shortcutInfo->curIndex = 0;
		
	return errNone;		
}

/***********************************************************************
 *
 * FUNCTION: ZDicLibDBInitDictList
 *
 * DESCRIPTION: Updata all dictionary database.
 *
 * PARAMETERS:	-> type Type of Dictionary database.
 *				-> creator Creator of Dictionary database.
 *
 * RETURN:		Dictionarys number.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *				
 ***********************************************************************/
 
Err ZDicLibDBInitDictList(UInt16 refNum, UInt32 type, UInt32 creator, Int16* dictCount)
{
	return PrvZDicLibDBInitDictList(type, creator, dictCount);
}




/***********************************************************************
 *
 * FUNCTION:     ZDicLibOpenCurrentDict
 *
 * DESCRIPTION:	Open current dictionary and index file.
 *
 * PARAMETERS:	nothing.
 *
 * RETURNED:	errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *
 ***********************************************************************/
Err ZDicLibOpenCurrentDict(UInt16 refNum)
{
	ZDicDBDictInfoType	*dictInfo;
	LocalID				dbID;
	Char				*pathName;
	Err					err;
	Boolean				bFirst = true;
    AppGlobalType       *global;
    
	global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;

TRY_OPEN_DICT:
	err = errNone;
	global->bDBInfoInitial = false;	// database information need initial.
	global->descript.blkIndex = 0;	// decode buffer need reload.
	
	if (dictInfo->volRefNum[dictInfo->curMainDictIndex] == vfsInvalidVolRef)
	{
		// current dict in ram, so open it by database.
		dbID = DmFindDatabase (0, &dictInfo->dictName[dictInfo->curMainDictIndex][0]);
		if (dbID)
		{
			global->dbP = DmOpenDatabase (0, dbID, dmModeReadOnly);
			if (global->dbP == NULL)
				err = DmGetLastErr();
		}
		else
			err = DmGetLastErr();
	}
	else
	{
		pathName = global->pathName;
		global->fileRef = global->idxRef = NULL;
	
		// current dict in card, so open it by file.
		StrCopy(pathName, ZDIC_DICT_PATH);
		StrCat(pathName, &dictInfo->dictName[dictInfo->curMainDictIndex][0]);
		err = VFSFileOpen (dictInfo->volRefNum[dictInfo->curMainDictIndex],
			pathName, vfsModeRead, &global->fileRef);
		if (err == errNone)
		{		
			// Open index file
			PrvZDicVFSGetIdxName (pathName);
			err = VFSFileOpen (dictInfo->volRefNum[dictInfo->curMainDictIndex],
				pathName, vfsModeRead, &global->idxRef);
			if (err != errNone)
			{
				VFSFileClose (global->fileRef);
				global->fileRef = NULL;
			}
		}
	}
	
	if (err != errNone && bFirst)
	{
		Int16	itemNum;
		
		bFirst = false;

		// If can not found the dict then 
		// Initial dictionary list and try again.
		err = PrvZDicLibDBInitDictList(appDBType, appKDicCreator, &itemNum);
		if (itemNum == 0)
		{
			//FrmAlert (DictNotFoundAlert);
			return ZDicLibNotFoundDict;	
		}
		
		// Initial index record of dictionary database that in dictionary list.
		PrvZDicLibDBInitIndexRecord();

		goto TRY_OPEN_DICT;
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:     ZDicLibCloseCurrentDict
 *
 * DESCRIPTION:	
 *
 * PARAMETERS:	
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/
Err ZDicLibCloseCurrentDict(UInt16 refNum)
{
	ZDicDBDictInfoType	*dictInfo;
    AppGlobalType *global;
    
    global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
 
	if (dictInfo->volRefNum[dictInfo->curMainDictIndex] == vfsInvalidVolRef)
	{
		if (global->dbP != NULL)
		{
			DmCloseDatabase(global->dbP);
			global->dbP = NULL;
		}
	}
	else
	{
		if (global->fileRef != NULL)
		{
			VFSFileClose(global->fileRef);
			global->fileRef = NULL;
		}

		if (global->idxRef != NULL)
		{
			VFSFileClose(global->idxRef);
			global->idxRef = NULL;
		}
	}
	
	return errNone;
}


#pragma mark -
/////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Private Functions
 *********************************************************************/

/*
 * FUNCTION: PrvMakeGlobals
 *
 * DESCRIPTION: Create our library globals.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: internal
 *
 * RETURNS:
 *
 *		pointer to our *locked* library globals
 *		NULL if our globals	could not be created.
 */

static ZDicLibGlobalsType * PrvMakeGlobals(UInt16 refNum)
{
	ZDicLibGlobalsType * gP = NULL;
	MemHandle gH;
	SysLibTblEntryType * libEntryP;

	/* Get library globals */
	libEntryP = SysLibTblEntry(refNum);
	ErrFatalDisplayIf(libEntryP == NULL, "invalid ZDicLib refNum");

	/* Error check to make sure the globals don't already exist */
	ErrFatalDisplayIf(libEntryP->globalsP, "ZDicLib globals already exist");

	/* Allocate and initialize our library globals. */
	gH = MemHandleNew(sizeof(ZDicLibGlobalsType));
	if ( !gH )
		return( NULL );

	/* Save the handle of our library globals in the system library table  */
	/* entry so we can later retrieve it using SysLibTblEntry(). */
	libEntryP->globalsP = (void*)gH;

	/* Lock our globals (should not fail) */
	gP = PrvLockGlobals(refNum);
	ErrFatalDisplayIf(gP == NULL, "failed to lock ZDicLib globals");

	/* Set the owner of our globals memory chunk to "system" (zero), so it won't get
	 * freed automatically by Memory Manager when the first application to call
	 * ZDicLibOpen exits.  This is important if the library is going to stay open
	 * between apps. */
	MemPtrSetOwner(gP, 0);

	/* Initialize our library globals */
	MemSet(gP, sizeof(ZDicLibGlobalsType), 0);

	/* for convenience and debugging, save ref in globals structure */
	gP->thisLibRefNum = refNum;

	/* initial open count */
	gP->openCount = 0;

	/* return a pointer to our *locked* globals */
	return( gP );
}

/*
 * FUNCTION: PrvFreeGlobals
 *
 * DESCRIPTION: Free our library globals.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: internal
 *
 * RETURNS: nothing
 */

static void PrvFreeGlobals(UInt16 refNum)
{
	MemHandle gH;
	SysLibTblEntryType * libEntryP;

	/* Get our library globals handle */
	libEntryP = SysLibTblEntry(refNum);
	ErrFatalDisplayIf(libEntryP == NULL, "invalid ZDicLib refNum");

	gH = (MemHandle)(libEntryP->globalsP);

	/* Free our library globals */
	if ( gH )
	{
		/* clear our globals reference */
		libEntryP->globalsP = NULL;

		/* free our globals */
		MemHandleFree(gH);
	}
}

/*
 * FUNCTION: PrvLockGlobals
 *
 * DESCRIPTION:	Lock our library globals
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: internal
 *
 * RETURNS:
 *		pointer to our library globals
 *		NULL if our globals	have not been created yet.
 */

static ZDicLibGlobalsType * PrvLockGlobals(UInt16 refNum)
{
	ZDicLibGlobalsType * gP = NULL;
	MemHandle gH;
	SysLibTblEntryType * libEntryP;

	libEntryP = SysLibTblEntry(refNum);
	if ( libEntryP )
		gH = (MemHandle)(libEntryP->globalsP);
	if ( gH )
		gP = (ZDicLibGlobalsType *)MemHandleLock(gH);

	return( gP );
}

/*
 * FUNCTION: PrvIsLibOpen
 *
 * DESCRIPTION:	Check if the library has been opened.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number returned by SysLibLoad() or SysLibFind().
 *
 * CALLED BY: internal
 *
 * RETURNS: non-zero if the library has been opened
 */

static Boolean PrvIsLibOpen(UInt16 refNum)
{
	ZDicLibGlobalsType * gP;
	Boolean	isOpen = false;

	gP = PrvLockGlobals(refNum);

	if ( gP )
	{
		isOpen = true;
		PrvUnlockGlobals(gP);
	}

	return( isOpen );
}

/*
 * FUNCTION: PrvCreateClientContext
 *
 * DESCRIPTION:	
 *
 * Create a client context for storing client-specific data.
 * The client context allows the library to support multiple clients.
 *
 * PARAMETERS:
 *
 * gP
 *		pointer to our locked globals
 *
 * clientContextP
 *		pointer to variable for returning client context
 *
 * CALLED BY: Internal
 *
 * RETURNS:
 *		errNone
 *		memErrNotEnoughSpace
 *		ZDicLibErrNotOpen -- the library is not open
 *
 * SIDE EFFECTS:
 *
 *		*clientContextP will be set to client context on success, or zero on error.
 */

static Err PrvCreateClientContext(ZDicLibGlobalsType * gP, UInt32 * clientContextP)
{
	Err err = errNone;
	MemHandle contextH;
	ZDicLibClientContextType * contextP;

	/* Error-check our parameters */
	ErrFatalDisplayIf(gP == NULL, "null globals pointer");
	ErrFatalDisplayIf(clientContextP == NULL, "null context variable pointer");

	/* Initialize return variable */
	*clientContextP = 0;

	/* Allocate a new client context structure */
	contextH = MemHandleNew(sizeof(ZDicLibClientContextType));
	if ( !contextH )
	{
		err = memErrNotEnoughSpace;
	}
	else
	{
		/* save context chunk handle in return variable */
		*clientContextP = (UInt32)contextH;

		/* Initialize the context chunk */
		contextP = (ZDicLibClientContextType *)MemHandleLock(contextH);

		/* save address of open routine as signature to validate context */
		contextP->libSignature = (void *)&ZDicLibOpen;

		/* TODO: Insert code to initialize context members */

		PrvUnlockContext(contextP);

		/* increment context count (for debugging) */
		gP->contextCount++;
		ErrFatalDisplayIf(gP->contextCount == 0, "context count overflow");
	}

	return( err );
}

/*
 * FUNCTION: PrvDestroyClientContext
 *
 * DESCRIPTION:	Destroy a client context which was created by PrvCreateClientContext.
 *
 * PARAMETERS:
 *
 * gP
 *		pointer to our locked globals
 *
 * clientContextP
 *		client context
 *
 * CALLED BY: Internal
 *
 * RETURNS:
 *		errNone
 *		ZDicLibErrNotOpen -- the library is not open
 */

static Err PrvDestroyClientContext(ZDicLibGlobalsType * gP, UInt32 clientContext)
{
	ZDicLibClientContextType *	contextP;

	/* Error-check our parameters */
	ErrFatalDisplayIf(gP == NULL, "null globals pointer");

	/* Validate the client context by locking it */
	contextP = PrvLockContext(clientContext);

	if ( contextP )
	{
		/* freeing a locked chunk is permitted by the system */
		MemPtrFree(contextP);

		/* decrement context count (for debugging) */
		gP->contextCount--;
		ErrFatalDisplayIf(gP->contextCount < 0, "context count underflow");
	}

	return errNone;
}

/*
 * FUNCTION: PrvLockContext
 *
 * DESCRIPTION:	Validate and lock a client context.
 *
 * PARAMETERS: 
 *
 * context
 *		a client context to lock
 *
 * CALLED BY: internal
 *
 * RETURNS: pointer to the locked client context.
 */
 
static ZDicLibClientContextType * PrvLockContext(UInt32 context)
{
	ZDicLibClientContextType *	contextP = NULL;

	/* Error-check our parameters */
	ErrFatalDisplayIf(context == 0, "null client context");

	/* Lock the client context */
	contextP = (ZDicLibClientContextType *)MemHandleLock((MemHandle)context);
	ErrFatalDisplayIf(contextP == NULL, "failed to lock client context");

	/* Validate the client context */
	ErrFatalDisplayIf(contextP->libSignature != (void *)&ZDicLibOpen,
		"invalid client context");

	return( contextP );
}

#pragma mark -
/////////////////////////////////////////////////////////////////////////////

static Err PrvZDicLibDBInitIndexRecord(void)
{
	ZDicDBDictInfoType	*dictInfo;
	AppGlobalType		*global;
	LocalID				dbID;
	Int16				i;
	Err					err = errNone;

	//PrvDebugPrint(999999, 30, 30, 5);
		
	global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
	
	i = 0;
	while (dictInfo->dictName[i][0] != chrNull &&  i < MAX_DICT_NUM)
	{
		if (dictInfo->volRefNum[i] == vfsInvalidVolRef)
		{
			// current dict in ram, so open it by database.
			dbID = DmFindDatabase (0, &dictInfo->dictName[i][0]);
			if (dbID)
			{
				global->dbP = DmOpenDatabase (0, dbID, dmModeReadWrite);
				if (global->dbP != NULL)
				{
					PrvZDicDBInitIndexRecord (global->dbP, &dictInfo->dictName[i][0]);
					DmCloseDatabase (global->dbP);
				}
			}
			
		}
		else
		{
			PrvZDicVFSInitIndexFile(dictInfo->volRefNum[i], &dictInfo->dictName[i][0]);
		}
	
		i++;
	}
	
	return err;
}

static Err PrvZDicLibLookupForward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{

	AppGlobalType		*global;
	UInt32				strLen;
	UInt16				oldHead;
	UInt16				oldTail;
	UInt8               *buf;
	Err					err;
	
	global = PrvAppGetGlobal();
	
	*explainPtr = NULL;
	*explainLen = 0;
	
	if (descript->blkIndex == 0)
		return ~errNone;
		
	oldHead = descript->head;
	oldTail = descript->tail;
	buf = descript->decodeBuf;
		
	// skip last LineFeed char.
READ_NEXT_WORD:

	descript->head = descript->tail;

	if ( buf[descript->head] == chrNull || buf[descript->head + 1] == chrNull)
	{
        UInt8   *str;
        UInt16  recSize;
		
		//
		// already read over current block then read next block
		//
READ_NEXT_BLOCK:
		if (descript->blkIndex >= global->blkNumber)
		{
			// there not next word, so restore old states.
			descript->head = oldHead;
			descript->tail = oldTail;
	
			return ~errNone;
		}

		err = PrvZDicGetDictRecord (descript->blkIndex + 1, &str, &recSize);
		if (err) return err;

		descript->blkIndex++;

	
		Decoder(global->zlibRefNum, global->compressFlag,(unsigned char *)str, recSize, (unsigned char *)&descript->decodeBuf[0], &descript->decodeSize);
		
		descript->decodeBuf[descript->decodeSize] = chrNull;
		PrvZDicReleaseDictRecord(str);
		
		descript->head = descript->tail = 0;
	}
	else
	{
	    descript->head++;
	}

	// skip head space.
	while ( buf[descript->head] == chrSpace ) descript->head++;
		
	// read a line.
	descript->tail = descript->head;
	while ( buf[descript->tail]  != chrLineFeed && buf[descript->tail] != chrNull ) descript->tail++;
	
	// no used data in the end of buffer, so we read next block.
	if ( buf[descript->tail] == chrNull )
		goto READ_NEXT_BLOCK;

	strLen = descript->tail - descript->head;
	if (strLen < MAX_LINE_LEN)
		goto READ_NEXT_WORD;
		
	// build result
	*explainPtr = &buf[descript->head];
	*explainLen = strLen;
	
	return errNone;
}

static Err PrvZDicLibLookupBackward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalType		*global;
	UInt32				strLen;
	UInt16				oldHead;
	UInt16				oldTail;
	UInt8               *buf;
	Err					err;

	global = PrvAppGetGlobal();

	*explainPtr = NULL;
	*explainLen = 0;

	if ( descript->blkIndex == 0 )
		return ~errNone;

	oldHead = descript->head;
	oldTail = descript->tail;
	buf = descript->decodeBuf;

	// skip \n.
READ_NEXT_WORD:

    // Seek to end of next word.
	while ( descript->head > 0 && buf[descript->head - 1] == chrLineFeed ) descript->head--;
	descript->tail = descript->head;

	if ( descript->head == 0 )
	{
        UInt8   *str;
        UInt16  recSize;
    	
		//
		// already read over current block then read last block
		//
		if (descript->blkIndex <= 1)
		{
			// there not previous word, so restore old states.
			descript->head = oldHead;
			descript->tail = oldTail;

			return ~errNone;
		}

		// get dict record and decode it to text.
		err = PrvZDicGetDictRecord (descript->blkIndex - 1, &str, &recSize);
		if (err) return err;

		descript->blkIndex--;


		Decoder(global->zlibRefNum, global->compressFlag,(unsigned char *)str, recSize, (unsigned char *)buf, &descript->decodeSize);

		buf[descript->decodeSize] = chrNull;
		PrvZDicReleaseDictRecord(str);
		
		// skip no used data in the end of buffer.
		descript->head = descript->decodeSize - 1;		
		while ( descript->head > 0 && buf[descript->head] != chrLineFeed ) descript->head--;
		
		goto READ_NEXT_WORD;
	}
	else
	{
	    descript->head--;
	}

	// read backward a line.
	while ( descript->head > 0 && buf[descript->head - 1] != chrLineFeed ) descript->head--;

	strLen = descript->tail - descript->head;
	if (strLen < MAX_LINE_LEN)
		goto READ_NEXT_WORD;
		
	// build result
	*explainPtr = &buf[descript->head];
	*explainLen = strLen;

	return errNone;
}


static Err PrvZDicLibLookupCurrent(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalType		*global;
	UInt32				strLen;
	Err					err = errNone;

	global = PrvAppGetGlobal();

	*explainPtr = NULL;
	*explainLen = 0;

	if (descript->blkIndex == 0)
		return ~errNone;

	strLen = descript->tail - descript->head;
	if (strLen < MAX_LINE_LEN)
	{
		// if current is bad word then get next.
		err = PrvZDicLibLookupForward(explainPtr, explainLen, descript);
	}
	else
	{	
		// build result
		*explainPtr = &descript->decodeBuf[descript->head];
		*explainLen = strLen;
	}

	return err;

}

static Err PrvZDicLibDBInitDictList(UInt32 type, UInt32 creator, Int16* dictCount)
{
	Int16				counter;
	ZDicDBDictInfoType 	*dblist, *dictInfo;
	Err					err;
	AppGlobalType		*global;
		
	global = PrvAppGetGlobal();
	dblist = &global->data.dictInfoList;
	dictInfo = &global->prefs.dictInfo;
	dictInfo->totalNumber = 0;
	
	// Get all dictionary in ram and card.
	err = PrvZDicDBGetAllDict (type, creator, dblist);
	if (err == errNone && dblist != NULL && dblist->totalNumber > 0)
	{
		Int16 i, j;
		
		// check valid of the old list at first.
		for (i = 0, j = 0; i < MAX_DICT_NUM && dictInfo->dictName[i][0] != chrNull; i++)
		{
			for (counter = 0; counter < dblist->totalNumber; counter++)
			{
				// the volume reference number may be changed after reset or reinsert card
				// so we only compare file name.
				if (StrCompare (&dictInfo->dictName[i][0], &dblist->dictName[counter][0]) == 0)
					break;
			}
			
			// if the dictionary is no exist then remove it from list.
			if (counter < dblist->totalNumber)
			{
				// the dictionary already in list so we do nothing for it.
				dblist->dictName[counter][0] = chrNull;

				if (i != j)
				{
					StrCopy(&dictInfo->dictName[j][0], &dictInfo->dictName[i][0]);
					StrCopy(&dictInfo->displayName[j][0], &dictInfo->displayName[i][0]);
					dictInfo->phonetic[j] = dictInfo->phonetic[i];
				}

				// the volume reference number must update.
				dictInfo->volRefNum[j] = dblist->volRefNum[counter];

				j++;
			}
		}
		
		// then we should append new dictionary behind.
		if (j < MAX_DICT_NUM)
		{			
			for (counter = 0; counter < dblist->totalNumber && j < MAX_DICT_NUM; counter++)
			{
				if (dblist->dictName[counter][0] != chrNull)
				{
					StrCopy(&dictInfo->dictName[j][0], &dblist->dictName[counter][0]);
					StrCopy(&dictInfo->displayName[j][0], &dblist->displayName[counter][0]);
					dictInfo->volRefNum[j] = dblist->volRefNum[counter];
					dictInfo->phonetic[j] = eGmxPhonetic;
					j++;
				}
			}	
		}
		
		// the list is not full so set the end of list.
		if (j < MAX_DICT_NUM)
			dictInfo->dictName[j][0] = chrNull;
		
		
		// set the list item pointer.
		i = 0;
		while (dictInfo->dictName[i][0] != chrNull &&  i < MAX_DICT_NUM)
		{
			global->listItem[i] = &dictInfo->displayName[i][0];
			i++;
		}
		
		dictInfo->totalNumber = i;
	}

	if (dictInfo->curMainDictIndex >= dictInfo->totalNumber)
		dictInfo->curMainDictIndex = 0;

	if (dictInfo->curDADictIndex >= dictInfo->totalNumber)
		dictInfo->curDADictIndex = 0;


	*dictCount = dictInfo->totalNumber;
	return err;
}

#pragma mark -
/////////////////////////////////////////////////////////////////////////////


/***********************************************************************
 *
 * FUNCTION: PrvZDicStrCompare
 *
 * DESCRIPTION: Compare two strings
 *
 * PARAMETERS:
 *				-> s1 Pointer to a string. 
 *				-> s2 Pointer to a string. 
 *
 * RETURN:
 *				Returns 0 if the strings match. 
 *				Returns a positive number if s1 larger than s2 
 *				Returns a negative number if s1 smaller than s2.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Int16 PrvZDicStrCompare(const UInt8 *s1, const UInt8 *s2)
{
	Int16	idx;
	
	// compare key string.
	idx = 0;
	while(*(s1 + idx) == *(s2 + idx)
		&& *(s1 + idx) != chrNull)
	{
		idx++;
	}
		
	return  *(s1 + idx) - *(s2 + idx);
}

static Boolean PrvZDicBlockLookUp(const UInt8 *wordStr, UInt8 *block,
	UInt16* head, UInt16* tail, UInt16 *same)
{
	UInt8	*str;
	UInt16	begin, end, lastBegin, lastEnd, idx;
	UInt16	match, lastMatch;
	Boolean	noFound = true;
	
	// find out nearest string from text buffer.
	str = block;
	idx = begin = end = 0;
	match = 0;
	noFound = true;
	while(*(str + end) != chrNull && noFound)
	{
		// save last compare result.
		lastBegin = begin;
		lastEnd = end;
		lastMatch = match;

		begin = idx;		
		
		// compare key string.
		match = 0;
		while(*(str + idx)== *(wordStr + match)
			&& *(wordStr + match) != chrNull
			&& *(str + idx) != chrHorizontalTabulation
			&& *(str + idx) != chrLineFeed
			&& *(str + idx) != chrNull)
		{
			match++;
			idx++;
		}
		
		if (*(str + idx) >= (*(wordStr + match)))
		{
			// complete same.
			if (*(wordStr + match) == chrNull && *(str + idx) == chrHorizontalTabulation)
				match = LOOK_UP_FULL_PARITY;
				
			noFound = false;
		}
			
		// skip remain chars of explain.
		while(*(str + idx) != chrLineFeed && *(str + idx) != chrNull) idx++;
		end = idx;
		idx++;

	}

	// we look backward for get nearest string.
	if (lastMatch >= match && lastBegin != lastEnd)
	{
		begin = lastBegin;
		end = lastEnd;
		match = lastMatch;
	}

	// return nearest string.
	*head = begin;
	*tail = end;
	*same = match;
	
	// ret true if the nearest string is first string.
	return (lastBegin == lastEnd);
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicDBGetAllDict
 *
 * DESCRIPTION: Get all dictionary database in ram and card.
 *
 * PARAMETERS:	-> type Type of Dictionary database.
 *				-> creator Creator of Dictionary database.
 *				<- dbListP Dictionary database list.
 *
 * RETURN:		errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	14/Aug/04	Initial Revision 
 *				
 ***********************************************************************/
 
static Err PrvZDicDBGetAllDict(UInt32 type, UInt32 creator, ZDicDBDictInfoType *dbListP)
{
	// use for get dict information in ram.
	SysDBListItemType	*dbListIDsP;
	MemHandle			dbListIDsH;
	Boolean				status;
	UInt16				dbCount;
 
 	// use for get dict information in all card.
 	UInt32				vfsMgrVersion;
	UInt16				volRefNum[MAX_DICT_NUM + 1];
	UInt32				volIterator;
	UInt32				fileType, fileCreator;
	Char				*pathName;
	
	Int16				index;
	AppGlobalType		*global;
	Err					err = errNone;

	global = PrvAppGetGlobal();
	pathName = global->pathName;
	dbListP->totalNumber = 0;
	
	//////////////////////////////////////////////////////////////
	//
	// Get all dictionary in ram.
	//
	//////////////////////////////////////////////////////////////

	status = SysCreateDataBaseList(type, creator,
		&dbCount, &dbListIDsH, false);

	if (status == true && dbCount > 0)
	{
		dbListIDsP = MemHandleLock (dbListIDsH);		
		for (index = 0; index < dbCount && dbListP->totalNumber < MAX_DICT_NUM; index++)
		{
			StrCopy (&dbListP->dictName[dbListP->totalNumber][0], &dbListIDsP[index].name[0]);
			StrCopy (&dbListP->displayName[dbListP->totalNumber][0], &dbListIDsP[index].name[0]);
			dbListP->volRefNum[dbListP->totalNumber] = vfsInvalidVolRef;
			dbListP->totalNumber++;
		}
		MemHandleUnlock (dbListIDsH);
		MemHandleFree (dbListIDsH);
	}

	//////////////////////////////////////////////////////////////
	//
	// Get all dictionary information in card.
	//
	//////////////////////////////////////////////////////////////

	// Check VFS Manager.
	err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion);
	if(err)
		return errNone;	// VFS Manager not installed

	// Get all volume information
	index = 0;
	volIterator = vfsIteratorStart;
	while (volIterator != vfsIteratorStop && index < MAX_DICT_NUM)
	{
		err = VFSVolumeEnumerate(&volRefNum[index], &volIterator);
		if (err)
			break;
 
		index++;
	}
	volRefNum[index] = vfsInvalidVolRef;

	// Get all dictionary in all volume
	index = 0;
	while (volRefNum[index] != vfsInvalidVolRef)
	{
		FileInfoType info;
		FileRef dirRef;
		UInt32 dirIterator;
		FileRef fileRef;
		Char nameBuf[256];
 
		// open the directory first, to get the directory reference
		// volRefNum must have already been defined
		err = VFSFileOpen(volRefNum[index], ZDIC_DICT_PATH, vfsModeRead, &dirRef);
		if(err)
		{
			index++;
			continue;
		}

		dirIterator = vfsIteratorStart;
		while (dirIterator != vfsIteratorStop
			&& dbListP->totalNumber < MAX_DICT_NUM)
		{
			info.nameP = nameBuf;	// point to local buffer
			info.nameBufLen = 256;

			// Get the next file
			err = VFSDirEntryEnumerate (dirRef, &dirIterator, &info);
			if (err)
				break;
			
			// If the file name is too long then we skip it.
			if (StrLen(info.nameP) >= MAX_DICTNAME_LEN)
				continue;
			
			StrCopy (&dbListP->dictName[dbListP->totalNumber][0], info.nameP);

			// Check the file whether is special file by type and creator.
			StrCopy(pathName, ZDIC_DICT_PATH);
			StrCat(pathName, info.nameP);
			err = VFSFileOpen (volRefNum[index], pathName, vfsModeRead, &fileRef);
			if (err)
				continue;
				
			err = VFSFileDBInfo (fileRef, &dbListP->displayName[dbListP->totalNumber][0]/*name*/, NULL/*attributes*/,
				NULL/*version*/, NULL/*crDate*/, NULL/*modDateP*/,
				NULL/*bckUpDate*/, NULL/*modNum*/, NULL/*appInfoH*/,
				NULL/*sortInfoH*/, &fileType/*type*/, &fileCreator/*creator*/,
				NULL/*numRecords*/);
			if (err == errNone && creator == fileCreator && type == fileType)
			{
				dbListP->volRefNum[dbListP->totalNumber] = volRefNum[index];	
				dbListP->totalNumber++;
			}
			VFSFileClose(fileRef);
		}
   		
   		VFSFileClose(dirRef);
		index++;
	}
	
	return errNone;
}

/*
 * FUNCTION: PrvZDicDBWriteCache
 *
 * DESCRIPTION: Write data to cache, if the cache is full then dump cache to
 * 				storage memory. set srcP to NULL for dump the remain data to
 *				storage memory.
 *
 * PARAMETERS:
 *				->	recordP Pointer of storage memory.
 *				<->	offset Offset within record to start writing.
 *				->	srcP Pointer to data to copy into record.
 *				->	bytes Number of bytes to write.
 *				->	cache Pointer of cache.
 *				<->	cacheIdx Offset within cache to start writing.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 */

static void PrvZDicDBWriteCache(void *recordP, UInt32 *offset, const void *srcP, UInt32 bytes, UInt8 *cache, UInt32 *cacheIdx)
{
	UInt32 i;
	
	if(srcP == NULL && *cacheIdx != 0)
	{
		DmWrite(recordP, *offset, cache, *cacheIdx);	
		*offset = *offset + *cacheIdx;
		*cacheIdx = 0;
		
		return;
	}
	
	i = 0;
	do 
	{
		if(*cacheIdx == ZDIC_READ_BUFFER_SIZE)
		{
			DmWrite(recordP, *offset, cache, *cacheIdx);	
			*offset = *offset + *cacheIdx;
			*cacheIdx = 0;
		}
	
		*(cache + *cacheIdx ) = *((UInt8*)srcP + i);
		(*cacheIdx)++;
		i++;
	}while (i < bytes);

	return;
}

/*
 * FUNCTION: PrvZDicDBWriteCache
 *
 * DESCRIPTION: Initial the index record if the record is not exist.
 *
 * PARAMETERS:
 *				-> dbP Dictionary database pointer.
 *				-> dbName Name of the database.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 */



static Err PrvZDicDBInitIndexRecord(DmOpenRef dbP, const Char *dbName)
{
	UInt16		recNum, recIdx, idx;
	MemHandle	recH;
	UInt16		*recP;
	MemHandle	idxH;
	UInt8		*idxP, *cacheP;
	UInt32		size;
	Int16		result;
	UInt8 		*str;
	UInt16		strNumber, step;
	UInt32		strOffset, writeOffset;
	Char		markStr[] = INDEX_RECORD_MARKER;	// index record marker
	ZDicIndexRecHeadType	idxHead;
	FormPtr		frmP, curFormP;
	ControlType	*controlP;
	Char		*messageP;
	UInt32		cacheIdx;
	AppGlobalType	*global;
	DmOpenRef   zdicLibRef;
	
	global = PrvAppGetGlobal();
	cacheP = global->data.readBuf;
	messageP = global->pathName;
	
	/*
	 * Get the last record and check it whether is index record.
	 */
	
	recNum = DmNumRecords (dbP);
	recH = DmQueryRecord (dbP, recNum - 1);
	if (recH == NULL)
		return DmGetLastErr();
	
	recP = MemHandleLock(recH);
	result = MemCmp (recP, markStr, StrLen(markStr));
	MemHandleUnlock(recH);
	
	// the index record already build, so we do nothing.
	if (result == 0)
		return errNone;

	/*
	 * now we should build the index record at the end of dictionary db.
	 */
	
	// get first record info.
	recH = DmQueryRecord (dbP, 0);
	if (recH == NULL)
		return DmGetLastErr();
	recP = MemHandleLock (recH);
	strNumber = recP[OFFSET_RECNUM];
	strOffset = sizeof(UInt16) * (OFFSET_HEAD + strNumber);
	str = (UInt8*)recP;

	// first we get the size of index record.
	size = sizeof(ZDicIndexRecHeadType) + strNumber * sizeof(UInt32);
	recIdx = dmMaxRecordIndex;
	idxH = DmNewRecord (dbP, &recIdx, size);
	if (idxH == NULL)
	{
		MemHandleUnlock(recH);
		return DmGetLastErr();
	}
	idxP = MemHandleLock(idxH);	
	
	// build index record head.
	MemSet(&idxHead, sizeof(ZDicIndexRecHeadType), 0);
	StrCopy(idxHead.idxRecMark, markStr);
	idxHead.blkNumber = strNumber;
	
	writeOffset = 0;
	cacheIdx = 0;
	PrvZDicDBWriteCache (idxP, &writeOffset, &idxHead, sizeof(idxHead), cacheP, &cacheIdx);

	// Display process form.
	// We need open the ZDic Library database at first
	zdicLibRef = DmOpenDatabaseByTypeCreator(ZDicLibTypeID, ZDicLibCreatorID, dmModeReadOnly);	
	curFormP = FrmGetActiveForm ();
	frmP = FrmInitForm (ProcessForm);
	FrmSetActiveForm (frmP);
	controlP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, ProcessFormProcessButton));
			
	// build index list.
	step = 101;
	for (idx = 1; idx <= strNumber; idx++)
	{
		if (step != (idx * 100) / (strNumber))
		{
			step = (idx * 100) / (strNumber);
			StrPrintF (messageP, "%s : %d %%", dbName, step);
			if (idx == 1)
			{
				CtlSetLabel (controlP, messageP); 
				FrmDrawForm (frmP);
			}
			else
			{
				CtlDrawControl (controlP);
			}
		}
		
		// skip head string.
		while(*(str + strOffset) != chrNull) strOffset++;
		strOffset++;
	
		PrvZDicDBWriteCache (idxP, &writeOffset, &strOffset, sizeof(strOffset), cacheP, &cacheIdx);
		
		// skip tail string.
		while(*(str + strOffset) != chrNull) strOffset++;
		strOffset++;
	}

	PrvZDicDBWriteCache (idxP, &writeOffset, NULL, 0, cacheP, &cacheIdx);
	
	// Erease sort dialog
	FrmEraseForm (frmP);
	FrmDeleteForm (frmP);
	FrmSetActiveForm (curFormP);
	DmCloseDatabase(zdicLibRef);
			
	MemHandleUnlock (idxH);
	MemHandleUnlock (recH);
	DmReleaseRecord (dbP, recIdx, true);

	
	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	PrvZDicVFSGetChar
 *
 * DESCRIPTION: Read a byte from file, use cache to speed read.
 *
 * PARAMETERS:	-> fileRef File reference returned from VFSFileOpen.
 *				-> buf Cache buffer.
 *				<-> curIdx Index of cache buffer.
 *
 * RETURNED:	ret the byte that read from file.
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/

static UInt8 PrvZDicVFSGetChar(FileRef fileRef, UInt8 *buf, UInt32 *curIdx)
{
	UInt8 c;
	
	if(*curIdx == ZDIC_READ_BUFFER_SIZE)
	{
		*curIdx = 0;	
		VFSFileRead (fileRef, ZDIC_READ_BUFFER_SIZE, buf, NULL);
	}
	
	c = *(buf + *curIdx);
	*curIdx += 1;
	
	return c; 	
}

/***********************************************************************
 *
 * FUNCTION:	PrvZDicVFSWriteCache
 *
 * DESCRIPTION: Write data to file, use cache to speed write.
 *
 * PARAMETERS:	->	fileRef File reference returned from VFSFileOpen.
 *				->	srcP Pointer to data to copy into record.
 *				->	bytes Number of bytes to write.
 *				->	cache Pointer of cache.
 *				<->	cacheIdx Offset within cache to start writing.
 *
 * RETURNED:	ret the byte that read from file.
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/

static void PrvZDicVFSWriteCache(FileRef fileRef, const UInt32 *srcP, UInt32 *cache, UInt32 *cacheIdx)
{	
	if (srcP == NULL && *cacheIdx != 0)
	{
		VFSFileWrite (fileRef, *cacheIdx * sizeof(UInt32), cache, NULL);
		*cacheIdx = 0;
		
		return;
	}
	
	if(*cacheIdx == ZDIC_INDEX_BUFFER_ITEM)
	{
		VFSFileWrite (fileRef, ZDIC_INDEX_BUFFER_ITEM * sizeof(UInt32), cache, NULL);
		*cacheIdx = 0;
	}
		
	*(cache + *cacheIdx ) = *srcP;
	(*cacheIdx)++;
	
	return; 	
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicVFSGetStrList
 *
 * DESCRIPTION: Get the offset of string list and index list in dictionary file.
 *
 * PARAMETERS:
 *				-> fileRef Dictionary file pointer.
 *				<- firstRecordOffset Offset of first record.
 *				<- strListOffset Offset of string list.
 *				<- idxListOffset Offset of index list.
 *				<- itemNum Item number in list.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err PrvZDicVFSGetStrList(FileRef fileRef, UInt32 *firstRecordOffset, UInt32 *strListOffset, UInt16 *itemNum, UInt16 *compressFlag)
{
	UInt32			firstRecord;
	UInt16			blkNumber;
	UInt16			compress;
	Err				err;
	
	ErrNonFatalDisplayIf(fileRef == NULL, "Bad parameter");
		
	// Get first record(string list) offset.
	err = VFSFileSeek (fileRef, vfsOriginBeginning, PDB_FIRST_RECORD_OFFSET);
	if (err) return err;
	err = VFSFileRead (fileRef, sizeof(firstRecord), &firstRecord, NULL);
	if (err) return err;
	err = VFSFileSeek (fileRef, vfsOriginBeginning, firstRecord);
	if (err) return err;
		
	// Get item number in list.
	err = VFSFileSeek (fileRef, vfsOriginCurrent, sizeof(UInt16) * OFFSET_RECNUM);
	if (err) return err;
	err = VFSFileRead (fileRef, sizeof(blkNumber), &blkNumber, NULL);
	if (err) return err;
	
	// Get compress flag.
	//err = VFSFileSeek (fileRef, vfsOriginBeginning, sizeof(UInt16) * OFFSET_COMPRESS);
	//if (err) return err;
	err = VFSFileRead (fileRef, sizeof(compress), &compress, NULL);
	if (err) return err;

	
	if (firstRecordOffset != NULL) *firstRecordOffset = firstRecord; 
	if (strListOffset != NULL) *strListOffset = firstRecord + sizeof(UInt16) * (OFFSET_HEAD + blkNumber);
	if (itemNum != NULL) *itemNum = blkNumber;
	if (compressFlag != NULL) *compressFlag = compress;

	return err;
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicVFSGetIdxName
 *
 * DESCRIPTION: Get path name of index file by path name of dictionary
 *				file.
 *
 * PARAMETERS:
 *				-> fileName Pass file path name
 *				<- pathName ret index file path name.
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

static void PrvZDicVFSGetIdxName(Char *pathName)
{
	Int16 len;
	
	len = StrLen(pathName);
	
	if (len >= 4 && (StrCaselessCompare  (pathName + len - 4, ".pdb") == 0))
	{
		StrCopy (pathName + len - 4, ".idx");
	}
	else
		StrCat(pathName, ZDIC_DICT_IDX_EXT);
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicVFSInitIndexFile
 *
 * DESCRIPTION: Creat index file if it is not exist.
 *
 * PARAMETERS:
 *				-> volume Volume reference of dictionary file.
 *				-> fileName File name of dictionary file.
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

static Err PrvZDicVFSInitIndexFile(UInt16 volRefNum, const Char *fileName)
{
	Char		markStr[] = INDEX_RECORD_MARKER;	// index record marker
	ZDicIndexRecHeadType	idxHead;
	Err			err;
	Char			*pathName;
	FileRef			fileRef, idxRef;
	AppGlobalType	*global;
	DmOpenRef       zdicLibRef;
	
	////////////////////////////////////////////////////////////////////
	//
	// First we should check the valid of index file.
	//
	////////////////////////////////////////////////////////////////////
	//PrvDebugPrint(10000, 30, 30, 2);
	
	global = PrvAppGetGlobal();
	pathName = global->pathName;
	fileRef = idxRef = NULL;
	
	do{
		// Get patch name of index file.
		StrCopy (pathName, ZDIC_DICT_PATH);
		StrCat (pathName, fileName);
		PrvZDicVFSGetIdxName (pathName);

		// Check vaild of the index file.	
		err = VFSFileOpen (volRefNum, pathName, vfsModeRead, &idxRef);
		if (err) break;

		err = VFSFileRead (idxRef, sizeof(ZDicIndexRecHeadType), &idxHead, NULL);
		if (err) break;
		
		if (StrCompare (idxHead.idxRecMark, markStr) != 0)
			break;
		
		// OK! the index file has already exist and valid.
		VFSFileClose (idxRef);
		return errNone;

	}while (0);
	
	////////////////////////////////////////////////////////////////////
	//
	// Second we should open dictionary file and create index file.
	//
	////////////////////////////////////////////////////////////////////
    //PrvDebugPrint(10001, 30, 30, 2);
    	
	if (idxRef != NULL)
		VFSFileClose (idxRef);
	fileRef = idxRef = NULL;
	
	
	do{
		UInt32		strListOffset, idx;
		UInt16		strNumber, step;
		UInt8		*readCacheP;
		UInt32		readCacheIdx = ZDIC_READ_BUFFER_SIZE;
		UInt32		*writeCacheP;
		UInt32		writeCacheIdx = 0;
		FormPtr		frmP, curFormP;
		ControlType	*controlP;
		Char		*messageP;
		
		readCacheP = global->data.readBuf;
		writeCacheP = global->indexBuf;
		messageP = global->pathName;
		
		// Create index file.
		err = VFSFileCreate(volRefNum, pathName);
		if (err) break;
		
		err = VFSFileOpen (volRefNum, pathName, vfsModeWrite, &idxRef);
		if (err) break;
			
		// Open dictionary file.
		StrCopy (pathName, ZDIC_DICT_PATH);
		StrCat (pathName, fileName);
		err = VFSFileOpen (volRefNum, pathName, vfsModeRead, &fileRef);
		if (err) break;

		// Get string list form dictionary.
		err = PrvZDicVFSGetStrList (fileRef, NULL, &strListOffset, &strNumber, NULL);
		if (err) break;
		
		// Locate begin of index 
		err = VFSFileSeek (fileRef, vfsOriginBeginning, strListOffset);
		if (err) break;

		// Build index head.
		MemSet(&idxHead, sizeof(ZDicIndexRecHeadType), 0);
		StrCopy(idxHead.idxRecMark, markStr);
		idxHead.blkNumber = strNumber;
		VFSFileSeek (idxRef, vfsOriginBeginning, 0);
		VFSFileWrite (idxRef, sizeof(idxHead), &idxHead, NULL);

		// Display process form.
		// We need open the ZDic Library database at first
		zdicLibRef = DmOpenDatabaseByTypeCreator(ZDicLibTypeID, ZDicLibCreatorID, dmModeReadOnly);	
		curFormP = FrmGetActiveForm ();
		frmP = FrmInitForm (ProcessForm);
		FrmSetActiveForm (frmP);
		controlP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, ProcessFormProcessButton));

		// Now build index list.
		step = 101;
		for (idx = 1; idx <= strNumber; idx++)
		{
			if (step != (idx * 100) / (strNumber))
			{
				step = (idx * 100) / (strNumber);
				StrPrintF (messageP, "%s : %d %%", fileName, step);
				if (idx == 1)
				{
					CtlSetLabel (controlP, messageP); 
					FrmDrawForm (frmP);
				}
				else
				{
					CtlDrawControl (controlP);
				}
			}
			
			// skip head string.
			while(PrvZDicVFSGetChar (fileRef, readCacheP, &readCacheIdx) != chrNull) strListOffset++;
			strListOffset++;
			
			PrvZDicVFSWriteCache (idxRef, &strListOffset, writeCacheP, &writeCacheIdx);

			// skip tail string.
			while(PrvZDicVFSGetChar (fileRef, readCacheP, &readCacheIdx) != chrNull) strListOffset++;
			strListOffset++;
		}

		// Dump remainder data to index file.
		PrvZDicVFSWriteCache (idxRef, NULL, writeCacheP, &writeCacheIdx);
		
		// Erease process form.
		FrmEraseForm (frmP);
		FrmDeleteForm (frmP);
		FrmSetActiveForm (curFormP);
		DmCloseDatabase(zdicLibRef);
			
	}while (0);

	if (idxRef != NULL)
		VFSFileClose (idxRef);

	if (fileRef != NULL)
		VFSFileClose (fileRef);
	
	return err;
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicVFSGetRecordSize
 *
 * DESCRIPTION: Get row record size.
 *
 * PARAMETERS:	-> ref Dictionary pointer.
 *				-> recIndex Record index.(>0 and <= max record index)
 *				<- recSize Ret reocrd size.
 *
 * RETURN:		errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static Err PrvZDicVFSGetRecordSize(FileRef fileRef, UInt16 recIndex, UInt16 *recSize)
{
	UInt32	firstRecord;
	UInt16	blkNumber;
	Err		err = errNone;

	// Get first record(string list) offset.
	err = VFSFileSeek (fileRef, vfsOriginBeginning, PDB_FIRST_RECORD_OFFSET);
	if (err) return err;
	err = VFSFileRead (fileRef, sizeof(firstRecord), &firstRecord, NULL);
	if (err) return err;
	err = VFSFileSeek (fileRef, vfsOriginBeginning, firstRecord);
	if (err) return err;
		
	// Get row record number.
	err = VFSFileSeek (fileRef, vfsOriginCurrent, sizeof(UInt16) * OFFSET_RECNUM);
	if (err) return err;
	err = VFSFileRead (fileRef, sizeof(blkNumber), &blkNumber, NULL);
	if (err) return err;
	
	if (recIndex == 0 || recIndex > blkNumber)
	{
		ErrNonFatalDisplay ("bad record index");
		return 0;
	}
	
	// Get row record size.
	err = VFSFileSeek (fileRef, vfsOriginBeginning, firstRecord + PDB_INDEX_HEAD_SIZE_INBYTE + (recIndex - 1) * sizeof(UInt16));
	if (err) return err;

	err = VFSFileRead (fileRef, sizeof(UInt16), recSize, NULL);
	if (err) return err;

	return err;
}

/***********************************************************************
 *
 * FUNCTION:     PrvZDicVFSGetRowRecord
 *
 * DESCRIPTION:	Get the record by record index.
 *
 * PARAMETERS:	-> fileRef Dictionary pointer.
 *				-> recIndex Record index.
 *				<-> recBuf Pointer of record buffer.
 *				<->recSize Set the record buffer size and ret the record size.
 *
 * RETURNED:	errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/
static Err PrvZDicVFSGetRowRecord(FileRef fileRef, UInt16 recIndex,
	UInt8 *recBuf, UInt16 *recSize)
{
	UInt32			offset, bufSize;
	Err				err;

	bufSize = *recSize;
	
	err = PrvZDicVFSGetRecordSize (fileRef, recIndex, recSize);
	if (err) return err;
	
	// Get the record entry offset.
	offset = PDB_FIRST_RECORD_OFFSET + (UInt32)recIndex * PDB_RECORD_ENTER_SIZE_INBYTE * sizeof(UInt8);
	
	// Move to the record entry in index list.
	err = VFSFileSeek (fileRef, vfsOriginBeginning, offset);
	if (err) return err;
	
	// Read the record offset.
	err = VFSFileRead (fileRef, sizeof(offset), &offset, NULL);
	if (err) return err;

	// Move to the record.
	err = VFSFileSeek (fileRef, vfsOriginBeginning, offset);
	if (err) return err;

	// Read the reocrd.
	if (bufSize < *recSize) *recSize = bufSize;
	err = VFSFileRead (fileRef, *recSize, recBuf, NULL);
	if (err) return err;
	
	return err;
}

static void PrvZDicDBGetAllPopup(ZDicDBDictShortcutInfoType *dbListP)
{
	AppGlobalType				*global;
	ZDicDBDictInfoType			*dictInfo;
	UInt16						i,j;
	
	global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
	
	dbListP->totalNumber = 0;
	
	for(i=0,j=0;i<MAX_DICT_NUM && dictInfo->dictName[i][0] != chrNull;i++)
	{
		if(dictInfo->showInPopup[ i ])
		{
			dbListP->dictIndex[j] = i;
			j++;
		}
	}
	
	dbListP->totalNumber = j;
}

static void PrvZDicDBGetAllShortcut(ZDicDBDictShortcutInfoType *dbListP)
{
	AppGlobalType				*global;
	ZDicDBDictInfoType			*dictInfo;
	UInt16						i,j;
	
	global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
	
	dbListP->totalNumber = 0;
	
	for(i=0,j=0;i<MAX_DICT_NUM && dictInfo->dictName[i][0] != chrNull;i++)
	{
		if(dictInfo->showInShortcut[ i ])
		{
			dbListP->dictIndex[j] = i;
			j++;
		}
	}
	
	dbListP->totalNumber = j;
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicDBLookup
 *
 * DESCRIPTION: Search the item index that include the wordStr. half search
 *
 * PARAMETERS:
 *				->	wordStr Pointer of key string.
 *				->	strList Pointer of string list.
 *				->	idxList	Pointer of index list.
 *				->	itemNum	Item number in list.
 *
 * RETURN:
 *		Return the item index that include the word string.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static UInt16 PrvZDicDBLookup(const UInt8 *wordStr, const UInt8 *strList,
	const UInt32 *idxList, UInt32 itemNum)
{
	UInt16		kmin, probe, i, idx;
	Int16		result;				// result of comparing two string
	const UInt8	*str;

	result = 0;
	kmin = probe = 0;

	while (itemNum > 0)
	{
		i = itemNum / 2;
		probe = kmin + i;
		str = strList + *(idxList + probe);
		
		// compare key string.
		idx = 0;
		while(*(str + idx) == *(wordStr + idx)
			&& *(str + idx) != chrNull)
		{
			idx++;
		}
		
		result = *(wordStr + idx) - *(str + idx);
		if (result == 0)
			goto findWordString;

		if (result < 0)
			itemNum = i;
		else
		{
			kmin = probe + 1;
			itemNum = itemNum - i - 1;
		}
	}

	if (result >= 0)
		probe++;

findWordString:
	return probe;
}

/***********************************************************************
 *
 * FUNCTION: PrvZDicVFSLookup
 *
 * DESCRIPTION: Search the item index that include the wordStr. half search
 *
 * PARAMETERS:
 *				->	wordStr Pointer of key string.
 *				->	fileRef Pointer of dictionary file.
 *				->	strListOffset Offset of string list in dictionary.
 *				->	idxRef	Pointer of the index file.
 *				->	idxListOffset Offset of index list in index file.
 *				->	itemNum	Item number in list.
 *				->	strBuf Poiner of buffer.
 *				->	strBufLen Size of buffer.
 *
 * RETURN:
 *		Return the item index that include the word string.
 *
 * REVISION HISTORY:
 *		Name			Date		Description
 *		----			----		-----------
 *		ZhongYuanHuan	12/Aug/04	Initial Revision
 *
 ***********************************************************************/

static UInt16 PrvZDicVFSLookup(const UInt8 *wordStr,
	FileRef fileRef, UInt32 strListOffset,
	FileRef idxRef, UInt32 idxListOffset, UInt32 itemNum, UInt8 *strBuf, UInt32 strBufLen)
{
	UInt16		kmin, probe, i;
	Int16		result;				// result of comparing two string
	UInt32		strOffset;
	Err			err = errNone;

	result = 0;
	kmin = probe = 0;

	while (itemNum > 0)
	{
		i = itemNum / 2;
		probe = kmin + i;
		// Get the string offset from index list and read the string.
		err = VFSFileSeek (idxRef, vfsOriginBeginning, idxListOffset + sizeof(UInt32) * probe);
		if (err) break;
		
		err = VFSFileRead (idxRef, sizeof(strOffset), &strOffset, NULL);
		if (err) break;
		
		err = VFSFileSeek (fileRef, vfsOriginBeginning, strOffset);
		if (err) break;

		VFSFileRead (fileRef, strBufLen, strBuf, NULL);
		strBuf[strBufLen - 1] = chrNull;
	
		// Compare between wordStr and current string.
		result = PrvZDicStrCompare (wordStr, strBuf);
		if (result == 0)
			goto findWordString;

		if (result < 0)
			itemNum = i;
		else
		{
			kmin = probe + 1;
			itemNum = itemNum - i - 1;
		}
	}

	if (result >= 0)
		probe++;

findWordString:
	return probe;
}

/***********************************************************************
 *
 * FUNCTION:     PrvZDicGetDictRecord
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	-> recIndex Record index.
 *				<- recPtr Ret pointer of record.
 *				<- recSize Ret size of record.
 *
 * RETURNED:	errNone if success else fail.
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/
static Err PrvZDicGetDictRecord(UInt16 recIndex, UInt8 **recPtr, UInt16 *recSize)
{
	AppGlobalType		*global;
	ZDicDBDictInfoType	*dictInfo;
	Err					err = errNone;

	*recPtr = NULL;
	*recSize = sizeof(global->data.recordBuf);
	
	global = PrvAppGetGlobal();
 	dictInfo = &global->prefs.dictInfo;
 
	if (dictInfo->volRefNum[dictInfo->curMainDictIndex] == vfsInvalidVolRef)
	{
		MemHandle recH;
		
		recH = DmQueryRecord (global->dbP, recIndex);
		if (recH == NULL) return DmGetLastErr ();
		
		*recSize = MemHandleSize (recH);
		*recPtr = MemHandleLock (recH);
	}
	else
	{
		err = PrvZDicVFSGetRowRecord(global->fileRef, recIndex,
			global->data.recordBuf, recSize);
		if (err == errNone)
			*recPtr = global->data.recordBuf;
	}
	
	return err;
}

/***********************************************************************
 *
 * FUNCTION:     PrvZDicReleaseDictRecord
 *
 * DESCRIPTION:	
 *
 * PARAMETERS:	
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		zyh		27/02/03	Initial Revision
 *
 ***********************************************************************/
static Err PrvZDicReleaseDictRecord(UInt8 *recPtr)
{
	AppGlobalType		*global;
	ZDicDBDictInfoType	*dictInfo;

	global = PrvAppGetGlobal();
 	dictInfo = &global->prefs.dictInfo;
 
	if (dictInfo->volRefNum[dictInfo->curMainDictIndex] == vfsInvalidVolRef)
	{
		MemPtrUnlock (recPtr);
	}

	return errNone;
}




/***********************************************************************
 *
 * FUNCTION: PrvZDicDictGetIdxByWord
 *
 * DESCRIPTION: Get block index in current dictionary that include special
 *				string.
 *
 * PARAMETERS:
 *				->	wordStr Pointer of key string.
 *				<-	index Ret the block index.
 *				<->	size Ret the block size.
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

static Err PrvZDicDictGetIdxByWord(const UInt8 *wordStr, UInt16 *index, UInt16 *size)
{
	ZDicDBDictInfoType	*dictInfo;
	AppGlobalType 		*global;
	MemHandle			recH, idxH;
	UInt16				*recP;
	UInt32				*idxP;
	UInt16				blkIndex;
	UInt16				recSize;
	Err					err = errNone;

	ErrNonFatalDisplayIf(wordStr == NULL, "Can not find empty string");
		
	global = PrvAppGetGlobal();
	dictInfo = &global->prefs.dictInfo;
	
	*index = 0;
	*size = 0;

	// all dictionary in ram are small, so we can read index record directly.
	if (dictInfo->volRefNum[dictInfo->curMainDictIndex] == vfsInvalidVolRef)
	{
		// Get string list(first record) and index list(last record).
		recH = DmQueryRecord (global->dbP, 0);
		if (recH == NULL)
			return DmGetLastErr();
		
		recP = MemHandleLock(recH);
		global->blkNumber = recP[OFFSET_RECNUM];
		global->compressFlag = recP[OFFSET_COMPRESS];

		idxH = DmQueryRecord (global->dbP, global->blkNumber + 1);
		if (idxH == NULL)
			return DmGetLastErr();
		
		idxP = MemHandleLock(idxH);
		idxP = (UInt32*)(((UInt8*)idxP) + sizeof(ZDicIndexRecHeadType));

		blkIndex = PrvZDicDBLookup (wordStr, (UInt8*)recP, idxP, global->blkNumber);
		blkIndex++;
		
		if (blkIndex > global->blkNumber) blkIndex = global->blkNumber;
		recSize = recP[OFFSET_HEAD + blkIndex - 1];
		MemHandleUnlock(recH);
		MemHandleUnlock(idxH);
	}
	else
	{
		UInt8	*buf;
	
		buf = global->data.readBuf;
		
		// Get current dictionary file information if need.
		if (!global->bDBInfoInitial)
		{
			// Get string list form dictionary.
			err = PrvZDicVFSGetStrList (global->fileRef, &global->firstRecordOffset, &global->strListOffset, &global->blkNumber, &global->compressFlag);
			if (err) return err;
			
			global->bDBInfoInitial = true;
		}
		
		blkIndex = PrvZDicVFSLookup(wordStr, global->fileRef, global->strListOffset,
			 global->idxRef, sizeof(ZDicIndexRecHeadType), global->blkNumber,
			 buf, MAX_WORD_LEN);
		blkIndex++;

		// Get the block(record) size.
		if (blkIndex > global->blkNumber) blkIndex = global->blkNumber;
		err = VFSFileSeek (global->fileRef, vfsOriginBeginning, global->firstRecordOffset + sizeof(UInt16) * (OFFSET_HEAD + blkIndex - 1));
		if (err) return err;
		err = VFSFileRead (global->fileRef, sizeof(recSize), &recSize, NULL);
		if (err) return err;

	}

	*index = blkIndex;
	*size = recSize;
	
	//PrvDebugPrint(blkIndex, 30, 30, 2);
	//PrvDebugPrint(recSize, 30, 60, 2);
	return err;
}


/*
 * FUNCTION: PrvAppGetGlobal
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

static AppGlobalType * PrvAppGetGlobal(void)
{
    UInt32	global;
    Err	err;

    err = FtrGet( appFileCreator, AppGlobalFtr, &global );
    return err == errNone ? ( AppGlobalType * ) global : NULL;
}

static void PrvDebugPrint(UInt32 z, Int16 x, Int16 y, UInt16 time)
{
	Char s[20];
	
	StrIToA(s, z);
	WinDrawChars(s, StrLen(s), x, y);
	if (time != 0)
		SysTaskDelay(SysTicksPerSecond() * time);
}