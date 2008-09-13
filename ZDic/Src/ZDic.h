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
#include "ZDicFont.h"

/*********************************************************************
 * Internal Structures
 *********************************************************************/
#define MAX_LINE_LEN		3
#define MAX_WORD_LEN		32
#define MAX_WORD_ITEM		10
#define MAX_HIS_FAR			13
#define MAX_DICT_NUM		18
#define MAX_DICTNAME_LEN	dmDBNameLength				//  dmDBNameLength for pdb name, 256 for vfs filename and terminal by chrNull.
#define MAX_DESCFIELD_SIZE	(1024 * 8)

#define ZDIC_DICT_IDX_EXT	".idx"
#define ZDIC_DICT_PATH		"/PALM/PROGRAMS/MSFILES/"	// patch of dictionary in card.
#define ZDIC_VOICE_PATH		"/PALM/PROGRAMS/MSFILES/ZDICVOICE/"	// patch of voice in card.
#define ZDIC_DICT_PATH_LEN	23
#define ZDIC_DICT_MENUID	3000

#define ZDIC_READ_BUFFER_SIZE	1024						// read buffer for ZDicGetDictBlockIdxByWord
#define ZDIC_MAX_RECORD_SIZE	4096						// 4K max record size.
#define ZDIC_INDEX_BUFFER_ITEM	256

#define TIMES_PRE_SECOND        10
#define COORD_SCROLLBAR_WIDTH         7
#define WORDLIST_HEIGHT_HALF    58
#define WORDLIST_HEIGHT_FULL    112
#define COORD_TOOLBAR_HEIGHT         15
#define COORD_SPACE             1
#define COORD_START_Y               15
#define COORD_START_X               0
#define COORD_SCROLL_BTON_HIGHT     8
#define COORD_GOLDEN_SCALE      (16)/(26)                     // 1.618

#define LOOK_UP_FULL_PARITY	0xffff

// phonetic type.
typedef enum {
	eGmxPhonetic = 0, eEfanPhonetic, eLazywormPhonetic, eMutantPhonetic, eNumberPhonetic, eNonePhonetic
} phoneticEnum;

typedef struct
{
	Int16			itemNumber;										// item number.
	Int16			curMainDictIndex;								// dictionary index of normal launch.
	Int16			curDADictIndex;									// dictionary index of DA launch.
	Char			dictName[MAX_DICT_NUM][MAX_DICTNAME_LEN + 1];	// database name of dictionary.
	Char			displayName[MAX_DICT_NUM][dmDBNameLength + 1];	// database name of dictionary.
	UInt16			volRefNum[MAX_DICT_NUM];						// volume index of dictionary. vfsInvalidVolRef if in ram.
	phoneticEnum	phonetic[MAX_DICT_NUM];							// phonetic type of dictionary.
} ZDicDBDictInfoType;

typedef struct ZDicPreferenceType
{
	FontID	font;											// current font.
	ZDicDBDictInfoType	dictInfo;							// all dictionary information.
	Char	history[MAX_HIS_FAR][MAX_WORD_LEN + 1];			// history list
	
	Boolean getClipBoardAtStart;							// true if get word for clip board at normal launch.
	Boolean enableIncSearch;								// true if enable incremental search else false.
	Boolean enableSingleTap;								// true if enable jump search for single tap.
	Boolean enableWordList;									// true if display word list.
	Boolean enableHighlightWord;							// true if always highlight word field.
	Boolean enableTryLowerSearch;							// true if try lower case on search failed.
	Boolean useSystemFont;						            // true if disable phonetic font support(use system standar font).
	Boolean enableJumpSearch;								// true if enable jump search else false.
	Boolean enableAutoSpeech;                               // true if enable automatic speech.
	PointType	daFormLocation;								// location of DA form.
	UInt16	incSearchDelay;									// incremental search delay.
	
	UInt16	exportCagetoryIndex;							// category index when export to memo
	Boolean	exportPrivate;									// private when export to memo
	UInt32	exportAppCreatorID;							    // creator id of export to the application.
	
	Boolean reserve6;


} ZDicPreferenceType;

/*********************************************************************
 * Global variables
 *********************************************************************/

extern ZDicPreferenceType g_prefs;

/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'ZDic'
#define appKDicCreator			'Kdic'
#define appSugarMemoCreator     'SGMM'
#define appSugarMemoDBType      'Voc0'
#define appName					"ZDic"
#define appDBType				'Dict'
#define appIdxType				'ZIdx'
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x03 // 01->02->03

#define AppGlobalFtr			0
#define AppIsRuningFtr			1
#define AppVoiceBufFtr          2

#define sysZDicCmdTibrProLaunch	sysAppLaunchCmdCustomBase
#define sysZDicCmdDALaunch		60000

// form update code.
#define updateDictionaryChanged	1
#define updateFontChanged		2

#define CACHE_SIZE		4096

typedef struct {
	Int32		decodeSize;					// valid date size in decodeBuf
	UInt8		decodeHead;					// fill it to zero, the flag of buffer head.
	UInt8		decodeBuf[CACHE_SIZE + 1];
	UInt16		blkIndex;					// block index of decodeBuf
	UInt16      head;                       // head offset of current word
	UInt16      tail;                       // tail offset of current word
} ZDictDecodeBufType;

typedef struct
{
	UInt16		itemUsed;
	Char		*itemPtr[MAX_WORD_ITEM];
	Char		itemBuf[MAX_WORD_ITEM][MAX_WORD_LEN + 1];
	UInt16		itemBlkIndex[MAX_WORD_ITEM];
	UInt16		itemHead[MAX_WORD_ITEM];
	UInt16		itemTail[MAX_WORD_ITEM];
} ZDicWordListType;


typedef struct AppGlobalObj{
    // initial count
    struct AppGlobalObj *prvGlobalP;           // launch count, enable free global when only it be zero.
    
	// db
	DmOpenRef	dbP;							// pointer of dictionary in ram
	FileRef		fileRef;						// pointer of dictionary in card.
	FileRef		idxRef;							// pointer of index file.
	
	// db info
	Boolean		bDBInfoInitial;					// true if blkNumber and firstRecordOffset is valid. else false.
	UInt16		blkNumber;						// block number of dictionary
	UInt32		firstRecordOffset;				// only use for fileRef, the first record offset of dict file in card.
	UInt32		strListOffset;					// String list offset in dictionary file.
	UInt16		compressFlag;					// = 0 the data not to decode, else need decode.
	
	// font
    UInt16          fontLibrefNum;              // ZDic font share library.
    Boolean         fontLibLoad;                // True is be load else false.
	ZDicFontType    font;                       // font data.

    // DIA(dynamic input area) support
    Boolean     bDiaEnable;                     // true if DIA is enable.
    UInt32      diaVersion;                     // DIA version
    UInt16      diaPinInputTriggerState;        // PINGetInputTriggerState
    UInt16      diaPinInputAreaState;           // PINGetInputAreaState
    
	// current state.
	Boolean		subLaunch;					    // true if sub launch else false.
	Boolean		wordListIsOn;					// current word list states, true if display else hide.	

	// use for incremental search
	Boolean		needSearch;						// true if a search be delay.
	Boolean		putinHistory;					// parameters
	Boolean		updateWordList;
	Boolean		highlightWordField;
	Boolean		updateDescField;
	
	// decode buffer.
	ZDictDecodeBufType	descript;				// Main buffer, use for seek.
	ZDictDecodeBufType	wordlist;				// Main buffer, use for word list.

	UInt16		historySeekIdx;					// current history seek index, zero if at top.
	
	// wordlist
	ZDicWordListType	wordlistBuf;			// word list item buffer.

	Char		dictTriggerName[MAX_DICTNAME_LEN + 1]; // use for display tigger text

	Char		*listItem[MAX_DICT_NUM];
	
	// init string
	FormPtr		prvActiveForm;				// user for DA to get active field of prv active form.
	Char		initKeyWord[MAX_WORD_LEN + 1];	// use for get init string at start zdic.

	ZDicPreferenceType prefs;					// use for save and load preference setting.
	
	Char		pathName[ZDIC_DICT_PATH_LEN + MAX_DICTNAME_LEN + 1];	// use for open file in card.
	UInt32		indexBuf[ZDIC_INDEX_BUFFER_ITEM];						// use for PrvZDicVFSInitIndexFile
	union {
		UInt8				readBuf[ZDIC_READ_BUFFER_SIZE];				// read buffer for ZDicGetDictBlockIdxByWord
		ZDicDBDictInfoType	dictInfoList;
		UInt8				recordBuf[ZDIC_MAX_RECORD_SIZE];			// use for PrvZDicVFSGetRowRecord
	}data;
	
} AppGlobalType;

typedef AppGlobalType *AppGlobalPtr;

extern Err AppInitialGlobal();
extern AppGlobalType * AppGetGlobal();
extern Err AppFreeGlobal();
extern Err MainFormAdjustObject ( const RectanglePtr toBoundsP );
extern void DAFormAdjustFormBounds ( AppGlobalType	*global, FormPtr frmP, RectangleType curBounds, RectangleType displayBounds );
#endif /* ZDIC_H_ */
