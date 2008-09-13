/******************************************************************************
 *
 * Copyright (c) 1998-2000 GSL, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ZDicTools.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef _ZDIC_TOOLS_H
#define _ZDIC_TOOLS_H

#include <PalmOS.h>

extern Err ZDicToolsLibInitial(UInt32 libType, UInt32 libCreator, const Char *nameP,
	UInt16 *refNumP, Boolean *bLoadP);
	
extern Err ZDicToolsLibRelease(UInt16 refNum, Boolean bLoad);
extern void * GetObjectPtr(UInt16 objectID);
extern void ZDicToolsWinGetBounds(WinHandle winH, RectangleType* rP);
#endif // _ZDIC_TOOLS_H