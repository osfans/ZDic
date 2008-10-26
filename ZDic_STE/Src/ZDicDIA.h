/******************************************************************************
 *
 * Copyright (c) 1998-2000 GSL, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ZDicDIA.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef _ZDIC_DIA_H
#define _ZDIC_DIA_H

#include <PalmOS.h>

#ifdef ZDIC_DIA_SUPPORT

extern Err ZDicDIALibInitial ( AppGlobalType*	global );
extern Err ZDicDIAFormLoadInitial ( AppGlobalType*	global, FormPtr frmP );
extern Err ZDicDIAFormClose ( AppGlobalType* global );
extern void ZDicDIACmdNotify ( MemPtr cmdPBP );
extern Boolean ZDicDIAWinEnter ( AppGlobalType* global,  EventType * eventP );
extern Boolean ZDicDIADisplayChange ( AppGlobalType* global );

#else

#define ZDicDIALibInitial           (errNone)
#define ZDicDIAFormLoadInitial      (errNone)
#define ZDicDIAFormClose            (errNone)
#define ZDicDIACmdNotify
#define ZDicDIAWinEnter             (false)
#define ZDicDIADisplayChange        (false)

#endif

#endif // _ZDIC_DIA_H