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

#ifndef _ZDIC_EXPORT_H
#define _ZDIC_EXPORT_H

#include <PalmOS.h>

extern Err ExportPopupDialog(void);
extern Err ExportToMemo(UInt16 fieldID);
extern Err ExportToSugarMemo( UInt16 fieldID );
	
#endif // _ZDIC_EXPORT_H