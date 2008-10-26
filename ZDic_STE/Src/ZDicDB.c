#include "PalmOS.h"
#include "ZDic.h"
#include "ZDicLib.h"
#include "ZDicDB.h"

#pragma mark -
/////////////////////////////////////////////////////////////////////////////

Err ZDicLookup(const UInt8 *wordStr, UInt16 *matchs, UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalPtr global = AppGetGlobal();	
	return ZDicLibLookup(global->zdicLibRefNum, wordStr, matchs, explainPtr, explainLen, descript);
}

Err ZDicLookupForward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalPtr global = AppGetGlobal();	
	return ZDicLibLookupForward(global->zdicLibRefNum, explainPtr, explainLen, descript);
}

Err ZDicLookupBackward(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalPtr global = AppGetGlobal();	
	return ZDicLibLookupBackward(global->zdicLibRefNum, explainPtr, explainLen, descript);
}

Err ZDicLookupCurrent(UInt8 **explainPtr, UInt32 *explainLen, ZDictDecodeBufType* descript )
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibLookupCurrent(global->zdicLibRefNum, explainPtr, explainLen, descript);
}

Err ZDicLookupWordListInit(WinDirectionType direction, Boolean init)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibLookupWordListInit(global->zdicLibRefNum, direction, init);
}

Err ZDicLookupWordListSelect(UInt16 select, UInt8 **explainPtr, UInt32 *explainLen)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibLookupWordListSelect(global->zdicLibRefNum, select, explainPtr, explainLen);
}

Err ZDicDBInitIndexRecord(void)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibDBInitIndexRecord(global->zdicLibRefNum);
}


Err ZDicDBInitShortcutList(void)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibDBInitShortcutList(global->zdicLibRefNum);
}

Err ZDicDBInitPopupList(void)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibDBInitPopupList(global->zdicLibRefNum);
}

Err ZDicDBInitDictList(UInt32 type, UInt32 creator, Int16* dictCount)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibDBInitDictList(global->zdicLibRefNum, type, creator, dictCount);
}

Err ZDicOpenCurrentDict(void)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibOpenCurrentDict(global->zdicLibRefNum);
}

Err ZDicCloseCurrentDict(void)
{
	AppGlobalPtr global = AppGetGlobal();
	return ZDicLibCloseCurrentDict(global->zdicLibRefNum);
}

