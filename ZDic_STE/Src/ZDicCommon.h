#ifndef ZDIC_COMMON_H_
#define ZDIC_COMMON_H_

/* Palm OS common definitions */
#include <VFSMgr.h>

#define MAX_LINE_LEN		3
#define MAX_WORD_LEN		32
#define MAX_WORD_ITEM		10
#define MAX_WORD_ITEM_TINY	14
#define MAX_HIS_FAR			13
#define MAX_DICT_NUM		24
#define MAX_DICTNAME_LEN	dmDBNameLength				//  dmDBNameLength for pdb name, 256 for vfs filename and terminal by chrNull.
#define MAX_DESCFIELD_SIZE	(1024 * 8)
#define MAX_SHORTCUTNOTE_LEN	8
#define MAX_IPA_LEN	100

#define ZDIC_DICT_IDX_EXT	".idx"
#define ZDIC_DICT_PATH		"/PALM/PROGRAMS/MSFILES/"	// path of dictionary in card.
#define ZDIC_VOICE_PATH		"/PALM/PROGRAMS/MSFILES/ZDICVOICE/"	// path of voice in card.
#define ZDIC_DICT_PATH_LEN	23
#define ZDIC_DICT_MENUID	3000

#define ZDIC_READ_BUFFER_SIZE	1024						// read buffer for ZDicGetDictBlockIdxByWord
#define ZDIC_MAX_RECORD_SIZE	0x4000						// 4K max record size.
#define ZDIC_INDEX_BUFFER_ITEM	256

#define TIMES_PRE_SECOND        10
#define COORD_SCROLLBAR_WIDTH   7
#define WORDLIST_HEIGHT_HALF    58
#define WORDLIST_HEIGHT_FULL    112
#define COORD_TOOLBAR_HEIGHT         16
#define COORD_SPACE             1
#define COORD_START_Y               15
#define COORD_START_X               1
#define COORD_SCROLL_BTON_HIGHT     8
#define COORD_GOLDEN_SCALE      (16)/(26)                     // 1.618

#define LOOK_UP_FULL_PARITY	0xffff

// phonetic type.
typedef enum {
	eGmxPhonetic = 0, eEfanPhonetic, eLazywormPhonetic, eMutantPhonetic, eNumberPhonetic, eNonePhonetic
} phoneticEnum;

typedef struct
{
	Int8			totalNumber;										// item number.
	Int8			curMainDictIndex;								// dictionary index of normal launch.
	Int8			curDADictIndex;									// dictionary index of DA launch.
	Char			dictName[MAX_DICT_NUM][MAX_DICTNAME_LEN + 1];	// database name of dictionary.
	Char			displayName[MAX_DICT_NUM][dmDBNameLength + 1];	// database name of dictionary.
	UInt16			volRefNum[MAX_DICT_NUM];						// volume index of dictionary. vfsInvalidVolRef if in ram.
	phoneticEnum	phonetic[MAX_DICT_NUM];							// phonetic type of dictionary.
	Boolean			showInShortcut[MAX_DICT_NUM];					// if the dict is show in shortcut list
	WChar			keyShortcutChr[MAX_DICT_NUM];					// one key change to the dict
	WChar			keyShortcutKeycode[MAX_DICT_NUM];				// one key change to the dict
	Boolean			showInPopup[MAX_DICT_NUM];						// if the dict is show in popup list
	Char			menuShortcut[MAX_DICT_NUM];
	Char			noteShortcut[MAX_DICT_NUM][MAX_SHORTCUTNOTE_LEN + 1];
} ZDicDBDictInfoType;

typedef struct
{
	Int8			totalNumber;
	Int8			curIndex;	
	Int8			dictIndex[MAX_DICT_NUM];
} ZDicDBDictShortcutInfoType;

/*typedef struct
{
	Int8			totalNumber;
	Int8			curIndex;	
	Int8			dictIndex[MAX_DICT_NUM];
} ZDicDBDictPopupInfoType;*/

typedef struct ZDicPreferenceType
{
	FontID	font;											// current font.
	FontID	fontDA;											// current DA font.
	ZDicDBDictInfoType	dictInfo;							// all dictionary information.
	ZDicDBDictShortcutInfoType shortcutInfo;				// all dictionary's shortcut information
	ZDicDBDictShortcutInfoType popupInfo;						// all dictionary's popup information
	Char	history[MAX_HIS_FAR][MAX_WORD_LEN + 1];			// history list
	
	Boolean	isTreo;
	Boolean getClipBoardAtStart;							// true if get word for clip board at normal launch.
	Boolean enableIncSearch;								// true if enable incremental search else false.
	Boolean enableSingleTap;								// true if enable jump search for single tap.
	Boolean enableWordList;									// true if display word list.
	Boolean enableHighlightWord;							// true if always highlight word field.
	Boolean enableTryLowerSearch;							// true if try lower case on search failed.
	Boolean useSystemFont;						            // true if disable phonetic font support(use system standar font).
	Boolean enableJumpSearch;								// true if enable jump search else false.
	Boolean enableAutoSpeech;                               // true if enable automatic speech.
	Boolean useSkin;
	UInt16	mainFormID;
	
	UInt8	dictMenu;										// Dict Menu use which list
	UInt8	menuType;										// Dict Menu use which type
	UInt8	daSize;											// DA window Size
	UInt8	headColor;										// ZDic head font color
	IndexedColorType	bodyColor;							// ZDic body font color
	IndexedColorType	backColor;							// ZDic back font color
	IndexedColorType	linkColor;							// ZDic link color
	PointType	daFormLocation;								// location of DA form.
	UInt16	incSearchDelay;									// incremental search delay.
	
	UInt16	exportCagetoryIndex;							// category index when export to memo
	Boolean	exportPrivate;									// private when export to memo
	UInt32	exportAppCreatorID;							    // creator id of export to the application.
	
	Boolean OptUD;
	Boolean UD;
	Boolean OptLR;
	Boolean LR;
	Boolean SwitchUDLR;
	
	UInt8	SelectKeyUsed;
	UInt8	SelectKeyFunc;
	UInt8	OptSelectKeyFunc;
	
	WChar	keyPlaySoundChr;
	WChar	keyPlaySoundKeycode;
	WChar	keyWordListChr;
	WChar	keyWordListKeycode;
	WChar	keyHistoryChr;
	WChar	keyHistoryKeycode;
	WChar	keyEnlargeDAChr;
	WChar	keyEnlargeDAKeycode;
	WChar	keyOneKeyChgDicChr;
	WChar	keyOneKeyChgDicKeycode;
	WChar	keyExportChr;
	WChar	keyExportKeycode;
	WChar	keyClearFieldChr;
	WChar	keyClearFieldKeycode;
	WChar	keyShortcutChr;
	WChar	keyShortcutKeycode;
	WChar	keyPopupChr;
	WChar	keyPopupKeycode;
	WChar	keyGobackChr;
	WChar	keyGobackKeycode;
	WChar	keySearchAllChr;
	WChar	keySearchAllKeycode;
	
	Boolean reserve6;

} ZDicPreferenceType;


/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'ZDic'
#define appKDicCreator			'Kdic'
#define appSugarMemoCreator     'SGMM'
#define appSugarMemoDBType      'Voc0'
#define appSuperMemoCreator     'SMem'
#define appSuperMemoDBType      'SM01'
#define appName					"ZDic"
#define appDBType				'Dict'
#define appIdxType				'ZIdx'
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x02 // 01->02->03

#define AppGlobalFtr			0
#define AppIsRuningFtr			1
#define AppVoiceBufFtr          2

#define sysZDicCmdTibrProLaunch	sysAppLaunchCmdCustomBase
#define sysZDicCmdDALaunch		60000

// form update code.
#define updateDictionaryChanged	1
#define updateFontChanged		2

#define CACHE_SIZE	0x4000

#define PDB_FIRST_RECORD_OFFSET	0x4e
#define PDB_RECORD_ENTER_SIZE_INBYTE	8
#define PDB_INDEX_HEAD_SIZE_INBYTE	0x10
#define INDEX_RECORD_MARKER	"ZDicDBIdxRecMarkMagic"

typedef struct
{
	Char	idxRecMark[dmDBNameLength];
	UInt16	blkNumber;
	UInt8	reserve[30];
} ZDicIndexRecHeadType;

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


/*typedef struct
{
	UInt16		itemUsed;
	Char		*itemPtr[MAX_WORD_ITEM_TINY];
	Char		itemBuf[MAX_WORD_ITEM_TINY][MAX_WORD_LEN + 1];
	UInt16		itemBlkIndex[MAX_WORD_ITEM_TINY];
	UInt16		itemHead[MAX_WORD_ITEM_TINY];
	UInt16		itemTail[MAX_WORD_ITEM_TINY];
} ZDicWordListTinyType;*/

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
    /*UInt16          fontLibrefNum;              // ZDic font share library.
    Boolean         fontLibLoad;                // True is be load else false.
	ZDicFontType    font;*/                       // font data.
	UInt16      zdicLibRefNum;                  // ZDic Library Reference Number
	UInt32      zdicLibClientContext;           // ZDic Library Client Context
	
	UInt16      zlibRefNum;                     // ZLib Library Reference Number
	
	UInt16      smtLibRefNum;                   // Smart Text Engine Library Reference Number
	UInt16      smtEngineRefNum;                // Smart Text Engine Reference Number

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
	ZDicWordListType		wordlistBuf;			// word list item buffer.
	//ZDicWordListTinyType	wordlisttinyBuf;

	Char		dictTriggerName[MAX_DICTNAME_LEN + 1]; // use for display tigger text

	Char		*listItem[MAX_DICT_NUM];
	Char		*shortcutlistItem[MAX_DICT_NUM];
	Char		shortcutlistShowTemp[MAX_DICT_NUM][MAX_DICTNAME_LEN + MAX_SHORTCUTNOTE_LEN + 3];
	Char		*shortcutlistShow[MAX_DICT_NUM];
	Char		*popuplistItem[MAX_DICT_NUM];
	
	// init string
	FormPtr		prvActiveForm;				// user for DA to get active field of prv active form.
	Char		initKeyWord[MAX_WORD_LEN + 1];	// use for get init string at start zdic.

	ZDicPreferenceType prefs;					// use for save and load preference setting.
	
	Char		pathName[ZDIC_DICT_PATH_LEN + MAX_DICTNAME_LEN + 1];	// use for open file in card.
	UInt32		indexBuf[ZDIC_INDEX_BUFFER_ITEM];						// use for PrvZDicVFSInitIndexFile
	union {
		UInt8				readBuf[ZDIC_READ_BUFFER_SIZE];				// read buffer for ZDicGetDictBlockIdxByWord
		ZDicDBDictInfoType	dictInfoList;
		ZDicDBDictShortcutInfoType shortcutInfoList;
		ZDicDBDictShortcutInfoType popupInfoList;
		UInt8				recordBuf[ZDIC_MAX_RECORD_SIZE];			// use for PrvZDicVFSGetRowRecord
	}data;
	
	Char		optflag;						//option key flag, 0: not pressed, 1: pressed once 2: always on
	Char 		phonetic[MAX_IPA_LEN + MAX_WORD_LEN + 1];					//ipa phonetic	
	UInt8		brightness;
	Boolean		zlibFound;		//zdiclib or zlib or ste opened
	Boolean		zdicLibFound;
	Boolean		STEFound;
} AppGlobalType;

#endif /* ZDIC_COMMON_H_ */