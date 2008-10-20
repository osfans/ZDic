#ifndef __ZLIB_H__
#define __ZLIB_H__

#define ZLIB_VERSION "1.1.3"

#define PALMOS
#include "zconf.h"

/* constants */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */

#define Z_DEFLATED   8
/* The deflate compression method (the only one supported in this version) */

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));

struct internal_state;

typedef struct z_stream_s {
    Bytef    *next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total nb of input bytes read so far */

    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total nb of bytes output so far */

    char     *msg;      /* last error message, NULL if no error */
    struct internal_state FAR *state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: ascii or binary */
    uLong   adler;      /* adler32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} z_stream;

typedef z_stream FAR *z_streamp;

    /* Function declarations */
Err ZLibOpen(UInt refNum) SYS_TRAP(sysLibTrapOpen);
Err ZLibClose(UInt refNum, UIntPtr numappsP) SYS_TRAP(sysLibTrapClose);
Err ZLibSleep(UInt refNum) SYS_TRAP(sysLibTrapSleep);
Err ZLibWake(UInt refNum) SYS_TRAP(sysLibTrapWake);

Err ZLibdeflateinit2(UInt refnum, z_streamp strm, int level, int method,
                      int windowBits, int memLevel,
                      int strategy, const char *version, int stream_size)
        SYS_TRAP(sysLibTrapCustom);

Err ZLibdeflate(UInt refnum, z_streamp strm, int flush)
        SYS_TRAP(sysLibTrapCustom+1);

Err ZLibdeflateend(UInt refnum, z_streamp strm)
        SYS_TRAP(sysLibTrapCustom+2);

Err ZLibinflateinit2(UInt refnum, z_streamp strm,
                      int windowBits, const char *version, int stream_size)
        SYS_TRAP(sysLibTrapCustom+3);

Err ZLibinflate(UInt refnum, z_streamp strm, int flush)
        SYS_TRAP(sysLibTrapCustom+4);

Err ZLibinflateend(UInt refnum, z_streamp strm)
        SYS_TRAP(sysLibTrapCustom+5);

uLong ZLibcrc32(UInt refnum, uLong crc, const Bytef * buf, uInt len)
        SYS_TRAP(sysLibTrapCustom+6);

uLong ZLibadler32(UInt refnum, uLong adler, const Bytef * buf, uInt len)
        SYS_TRAP(sysLibTrapCustom+7);

Err ZLibcompress2(UInt refnum, Bytef * dest, uLongf * destLen,
                   const Bytef * source, uLong sourceLen, int level)
        SYS_TRAP(sysLibTrapCustom+8);

Err ZLibuncompress(UInt refnum, Bytef * dest, uLongf * destLen,
                    const Bytef * source, uLong sourceLen)
        SYS_TRAP(sysLibTrapCustom+9);

#ifndef NOZLIBDEFS
UInt ZLibRef = 0;
#else
extern UInt ZLibRef;
#endif


#define ZLSetup  !ZLibRef && SysLibFind("Z.lib", &ZLibRef) && SysLibLoad('libr', 'ZLib', &ZLibRef) \
		   && SysLibFind("Z.lib", &ZLibRef) && ZLibOpen(ZLibRef) && (ErrFatalDisplay("No ZLib"),0)

#define ZLTeardown  if(ZLibRef){\
                      UInt zltmp;\
                      ZLibClose(ZLibRef,&zltmp);\
                      if(!zltmp)\
                        SysLibRemove(ZLibRef);\
                      ZLibRef = 0;}


#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        ZLibdeflateinit2((ZLibRef),(strm),(level),(method),(windowBits),(memLevel),\
                      (strategy), ZLIB_VERSION, sizeof(z_stream))
#define inflateInit2(strm, windowBits) \
        ZLibinflateinit2((ZLibRef), (strm), (windowBits), ZLIB_VERSION, sizeof(z_stream))

/* 13 bits requires 32k, Memlevel 6 requires 32k.  Each increment doubles (for each) */
#define deflateInit(a,b) ZLibdeflateinit2((ZLibRef), a, b, Z_DEFLATED, 13, 6, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream))
#define deflateInit_(a,b,c,d) ZLibdeflateinit2((ZLibRef), a, b, Z_DEFLATED, 13, 6, Z_DEFAULT_STRATEGY, c, d)

#define inflateInit(a) ZLibinflateinit2((ZLibRef), a, 15, ZLIB_VERSION, sizeof(z_stream))
#define inflateInit_(a,b,c) ZLibinflateinit2((ZLibRef), a, 15, b, c)

#define inflate(x,y) ZLibinflate(ZLibRef,x,y)
#define deflate(x,y) ZLibdeflate(ZLibRef,x,y)
#define inflateEnd(x) ZLibinflateend(ZLibRef,x)
#define deflateEnd(x) ZLibdeflateend(ZLibRef,x)
#define crc32(a,b,c) ZLibcrc32(ZLibRef,a,b,c)
#define adler32(a,b,c) ZLibadler32(ZLibRef,a,b,c)

#endif
