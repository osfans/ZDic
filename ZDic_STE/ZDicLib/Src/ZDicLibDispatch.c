/*
 * ZDicLibDispatch.c
 *
 * dispatch table for ZDicLib shared library
 *
 * This wizard-generated code is based on code adapted from the
 * SampleLib project distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
/* Our library public definitions (library API) */
#define BUILDING_ZDICLIB
#include "ZDicLib.h"

/* Local prototypes */
Err ZDicLibInstall(UInt16 refNum, SysLibTblEntryType *entryP);
static MemPtr ZDicLibDispatchTable(void);


Err ZDicLibInstall(UInt16 refNum, SysLibTblEntryType *entryP)
{
	#pragma unused(refNum)

	/* Install pointer to our dispatch table */
	entryP->dispatchTblP = (MemPtr *) ZDicLibDispatchTable();
	
	/* Initialize globals pointer to zero (we will set up our
	 * library globals in the library "open" call). */
	entryP->globalsP = 0;

	return 0;
}

/* Palm OS uses short jumps */
#define prvJmpSize		4

/* Now, define a macro for offset list entries */
#define libDispatchEntry(index)		(kOffset+((index)*prvJmpSize))

/* Finally, define the size of the dispatch table's offset list --
 * it is equal to the size of each entry (which is presently 2
 * bytes) times the number of entries in the offset list (including
 * the @Name entry). */
#define numberOfAPIs        (12)
#define entrySize           (2)
#define	kOffset             (entrySize * (5 + numberOfAPIs))

static MemPtr asm ZDicLibDispatchTable(void)
{
	LEA		@Table, A0			/* table ptr */
	RTS							/* exit with it */

@Table:
	/* Offset to library name */
	DC.W		@Name
	
	/*
	 * Library function dispatch entries
	 *
	 * ***IMPORTANT***
	 * The index parameters passed to the macro libDispatchEntry
	 * must be numbered consecutively, beginning with zero.
	 *
	 * The hard-wired values need to be used for offsets because
	 * CodeWarrior's inline assembler doesn't support label subtraction.
	 */
	
    /* Standard API entry points */
	DC.W		libDispatchEntry(0)			    /* ZDicLibOpen */
	DC.W		libDispatchEntry(1)		    	/* ZDicLibClose */
	DC.W		libDispatchEntry(2)	    		/* ZDicLibSleep */
	DC.W		libDispatchEntry(3) 			/* ZDicLibWake */

    /* Custom API entry points */
	DC.W		libDispatchEntry(4)		/* ZDicLibLookup */
	DC.W		libDispatchEntry(5)		/* ZDicLibLookupForward */
	DC.W		libDispatchEntry(6)		/* ZDicLibLookupBackward */
	DC.W		libDispatchEntry(7)		/* ZDicLibLookupCurrent */
	DC.W		libDispatchEntry(8)		/* ZDicLibLookupWordListInit */
	DC.W		libDispatchEntry(9)		/* ZDicLibLookupWordListSelect */
	DC.W		libDispatchEntry(10)	/* ZDicLibDBInitDictList */
	DC.W		libDispatchEntry(11)	/* ZDicLibDBInitIndexRecord */
	DC.W		libDispatchEntry(12)	/* ZDicLibOpenCurrentDict */
	DC.W		libDispatchEntry(13)	/* ZDicLibCloseCurrentDict */
	DC.W		libDispatchEntry(14)	/* ZDicLibDBInitPopupList */
	DC.W		libDispatchEntry(15)	/* ZDicLibDBInitShortcutList */

    /* Standard API entry points */
	JMP         ZDicLibOpen
	JMP         ZDicLibClose
	JMP         ZDicLibSleep
	JMP         ZDicLibWake

    /* Custom API entry points */
    JMP         ZDicLibLookup
    JMP         ZDicLibLookupForward
    JMP         ZDicLibLookupBackward
    JMP         ZDicLibLookupCurrent
    JMP         ZDicLibLookupWordListInit
    JMP         ZDicLibLookupWordListSelect
    JMP         ZDicLibDBInitDictList
    JMP         ZDicLibDBInitIndexRecord
    JMP         ZDicLibOpenCurrentDict
    JMP         ZDicLibCloseCurrentDict
    JMP         ZDicLibDBInitPopupList
    JMP         ZDicLibDBInitShortcutList    
	
@Name:
	DC.B		"ZDicLib"
}
