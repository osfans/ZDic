/*
 * ZDicFontDispatch.c
 *
 * dispatch table for ZDicFont shared library
 *
 * This wizard-generated code is based on code adapted from the
 * SampleLib project distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
/* Our library public definitions (library API) */
#define BUILDING_ZDICFONT
#include "ZDicFont.h"

/* Local prototypes */
Err ZDicFontInstall(UInt16 refNum, SysLibTblEntryType *entryP);
static MemPtr ZDicFontDispatchTable(void);


Err ZDicFontInstall(UInt16 refNum, SysLibTblEntryType *entryP)
{
	#pragma unused(refNum)

	/* Install pointer to our dispatch table */
	entryP->dispatchTblP = (MemPtr *) ZDicFontDispatchTable();
	
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
#define numberOfAPIs        (4)
#define entrySize           (2)
#define	kOffset             (entrySize * (5 + numberOfAPIs))

static MemPtr asm ZDicFontDispatchTable(void)
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
	DC.W		libDispatchEntry(0)			    /* ZDicFontOpen */
	DC.W		libDispatchEntry(1)		    	/* ZDicFontClose */
	DC.W		libDispatchEntry(2)	    		/* ZDicFontSleep */
	DC.W		libDispatchEntry(3) 			/* ZDicFontWake */

    /* Custom API entry points */
	DC.W		libDispatchEntry(4) 			/* ZDicFontInit */
	DC.W		libDispatchEntry(5) 			/* ZDicFontRelease */
	DC.W		libDispatchEntry(6) 			/* ZDicFontSet */
	DC.W		libDispatchEntry(7) 			/* ZDicFontGetFontID */

    /* Standard API entry points */
	JMP         ZDicFontOpen
	JMP         ZDicFontClose
	JMP         ZDicFontSleep
	JMP         ZDicFontWake

    /* Custom API entry points */
	JMP			ZDicFontInit
	JMP			ZDicFontRelease
	JMP			ZDicFontSet
	JMP			ZDicFontGetFontID
		
@Name:
	DC.B		"ZDicFont"
}
