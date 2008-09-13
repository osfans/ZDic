/******************************************************************************
 *
 * Copyright (c) 1998-2000 GSL, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ZDicDB.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef _ZDIC_DB_H
#define _ZDIC_DB_H

#include <PalmOS.h>
#include <VFSMgr.h>

extern Err ZDicLookup(const UInt8 *wordStr, UInt16 *matchs, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript);
extern Err ZDicLookupForward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript);
extern Err ZDicLookupBackward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript);
extern Err ZDicLookupCurrent(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript);
extern Err ZDicLookupWordListInit(WinDirectionType direction, Boolean init);
extern Err ZDicLookupWordListSelect(UInt16 select, UInt8 **explainPtr, UInt32 *explainLen);

extern Int16 ZDicDBInitDictList(UInt32 type, UInt32 creator);
extern Err ZDicDBInitIndexRecord(void);
extern Err ZDicOpenCurrentDict();
extern Err ZDicCloseCurrentDict();
extern Err ZDicGetDictRecord(UInt16 recIndex, UInt8 **recPtr, UInt16 *recSize);
extern Err ZDicReleaseDictRecord(UInt8 *recPtr);

#endif // _ZDIC_DB_H