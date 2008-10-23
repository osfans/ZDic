#include <PalmOS.h>

void da_main()
{
	  LocalID  dbID;
	  UInt32  result;
	   
	  dbID = DmFindDatabase(0, "ZDic");
	  if (dbID)
	  {
	    SysAppLaunch(0, dbID, 0, 60000, NULL, &result);//60000,50011,50012,50013
	  }
}