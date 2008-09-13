/******************************************************************************
 *
 * Copyright (c) 1998-2000 GSL, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ZDicVoice.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef _ZDIC_VOICE_H
#define _ZDIC_VOICE_H

#include <PalmOS.h>

extern Err ZDicVoicePlayWord ( char *explainPtr );
extern Boolean ZDicVoiceIsExist ( char *explainPtr );
extern Err ZDicVoiceRelease ( void );
#endif // _ZDIC_TOOLS_H