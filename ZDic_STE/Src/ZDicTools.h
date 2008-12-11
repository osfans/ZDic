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

extern Err ZDicToolsOpenZLib(UInt16 *refNumP, Boolean *found);
extern Err ZDicToolsCloseZLib(UInt16 refNum, Boolean found);
extern Err ZDicToolsOpenSMTLib(UInt16 *refNumP, Boolean *found);
extern Err ZDicToolsCloseSMTLib(UInt16 refNum, Boolean found);
extern void * GetObjectPtr(UInt16 objectID);
extern void ZDicToolsWinGetBounds(WinHandle winH, RectangleType* rP);
extern Err ZDicLib_OpenLibrary(UInt16 *refNumP, UInt32 * clientContextP, Boolean *found);
extern Err ZDicLib_CloseLibrary(UInt16 refNum, UInt32 clientContext, Boolean found);

#endif // _ZDIC_TOOLS_H