#include <PalmOS.h>
#include <VFSMgr.h>

#include "ZDic.h"
#include "ZDicVoice.h"

#define appZDicSpeechFileCreator    'ZDsp'	// register your own at http://www.palmos.com/dev/creatorid/
#define sysZDicSpeechCmdSearch      sysAppLaunchCmdCustomBase
#define sysZDicSpeechCmdDestory     (sysAppLaunchCmdCustomBase + 1)
#define sysZDicSpeechCmdSpeech      (sysAppLaunchCmdCustomBase + 2)
#define sysZDicSpeechCmdRelease     (sysAppLaunchCmdCustomBase + 3)

typedef struct
{
    Char *str;
    UInt16 len;

    Err err;
}
AppSpeechCmdType;

#pragma mark -

/*
 *
 * FUNCTION:    PrvZDicVoiceCallWithCommand
 *
 * DESCRIPTION: This routine sub launch ZDicSpeech by cmd and cmdPBP
 *
 * PARAMETERS:
 *
 * RETURNED:    errNone if success else fail.
 *
 *
 */

static Err PrvZDicVoiceCallWithCommand( UInt16 cmd, AppSpeechCmdType *cmdPBP )
{
    UInt16	cardNo;
    LocalID	dbID;
    DmSearchStateType	searchState;
    UInt32 result;
    Err err = errNone;

    if (cmdPBP)
    	cmdPBP->err = ~errNone;
    DmGetNextDatabaseByTypeCreator( true, &searchState, sysFileTApplication,
                                    appZDicSpeechFileCreator, true, &cardNo, &dbID );
    if ( dbID )
    {
        err = SysAppLaunch( cardNo, dbID, 0, cmd, ( MemPtr ) cmdPBP, &result );
        ErrNonFatalDisplayIf( err, "Could not launch app" );
    }

    return err;
}

/*
 *
 * FUNCTION:    ZDicVoicePlayWord
 *
 * DESCRIPTION: This routine play a word.
 *
 * PARAMETERS:  wordP    - pointer to string of the word.
 *
 * RETURNED:    nothing
 *
 *
 */

Err ZDicVoicePlayWord ( char *explainPtr )
{
    AppSpeechCmdType search;

    // Check word or phrase.
    search.str = explainPtr;
    search.len = 0;
    while ( explainPtr[ search.len ] != chrHorizontalTabulation
            && explainPtr[ search.len ] != chrNull
            && search.len < MAX_WORD_LEN )
    {
        search.len++;
    }

    // clean tail space.
    while ( search.len > 0 && explainPtr[ search.len - 1 ] == chrSpace )
    {
        search.len--;
    }

    if ( search.len == 0 )
        return false;

    PrvZDicVoiceCallWithCommand( sysZDicSpeechCmdSpeech, &search );

    return search.err;
}

/*
 *
 * FUNCTION:    ZDicVoiceIsExist
 *
 * DESCRIPTION: This routine check the voice of word whether is exist.
 *
 * PARAMETERS:  wordP    - pointer to string of the word.
 *
 * RETURNED:    nothing
 *
 *
 */

Boolean ZDicVoiceIsExist ( char *explainPtr )
{
    AppSpeechCmdType search;

    // Check word or phrase.
    search.str = explainPtr;
    search.len = 0;
    while ( explainPtr[ search.len ] != chrHorizontalTabulation
            && explainPtr[ search.len ] != chrSpace // if the first word of the text can found, the we try speech whold text.
            && explainPtr[ search.len ] != chrNull
            && search.len < MAX_WORD_LEN )
    {
        search.len++;
    }

    // clean tail space.
    while ( search.len > 0 && explainPtr[ search.len - 1 ] == chrSpace )
    {
        search.len--;
    }

    if ( search.len == 0 )
        return false;

    PrvZDicVoiceCallWithCommand( sysZDicSpeechCmdSearch, &search );

    return ( search.err == errNone );
}

/*
 *
 * FUNCTION:    ZDicVoiceRelease
 *
 * DESCRIPTION: This routine release all memory that used by ZDicSpeech.
 *
 * PARAMETERS:  wordP    - pointer to string of the word.
 *
 * RETURNED:    nothing
 *
 *
 */

Err ZDicVoiceRelease ( void )
{
    PrvZDicVoiceCallWithCommand( sysZDicSpeechCmdRelease, NULL );

    return errNone;
}
