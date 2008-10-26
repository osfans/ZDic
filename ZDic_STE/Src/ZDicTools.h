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

extern Err ZDicToolsOpenZLib(UInt16 *refNumP);
extern Err ZDicToolsCloseZLib(UInt16 refNum);
extern Err ZDicToolsOpenSMTLib(UInt16 *refNumP);
extern Err ZDicToolsCloseSMTLib(UInt16 refNum);
extern void * GetObjectPtr(UInt16 objectID);
extern void ZDicToolsWinGetBounds(WinHandle winH, RectangleType* rP);
#endif // _ZDIC_TOOLS_H