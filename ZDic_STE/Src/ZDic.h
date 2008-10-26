/*
 * ZDic.h
 *
 * header file for ZDic
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#ifndef ZDIC_H_
#define ZDIC_H_

#include <VFSMgr.h>
#include "ZDicCommon.h"

/*********************************************************************
 * Internal Structures
 *********************************************************************/

/*********************************************************************
 * Global variables
 *********************************************************************/
// Because we need support DA in one prc file so we cannot use any 
// global varibles!!!

/*********************************************************************
 * Internal Constants
 *********************************************************************/

typedef AppGlobalType *AppGlobalPtr;

extern Err AppInitialGlobal();
extern AppGlobalType * AppGetGlobal();
extern Err AppFreeGlobal();
extern Err MainFormAdjustObject ( const RectanglePtr toBoundsP );
extern void DAFormAdjustFormBounds ( AppGlobalType	*global, FormPtr frmP, RectangleType curBounds, RectangleType displayBounds );

#endif /* ZDIC_H_ */
