#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef ELIBSOURCES
#define ELIBSOURCES
#endif

#ifdef ELIBSOURCES

#ifdef WIN32
#include <io.h>
#define open _open
#define lseek _lseek
#define read _read
#define close _close
#define write _write
#endif

/* all linked symbols */
#  define _dist_code            _edist_code
#  define _length_code          _elength_code
#  define _tr_align             _etr_align
#  define _tr_flush_block       _etr_flush_block
#  define _tr_init              _etr_init
#  define _tr_stored_block      _etr_stored_block
#  define _tr_tally             _etr_tally
#  define adler32               eadler32

#  define adler32_combine64     eadler32_combine64
#  define compress              ecompress
#  define compress2             ecompress2
#  define compressBound         ecompressBound
#  define crc32                 ecrc32

#  define crc32_combine64       ecrc32_combine64
#  define deflate               edeflate
#  define deflateBound          edeflateBound
#  define deflateCopy           edeflateCopy
#  define deflateEnd            edeflateEnd
#  define deflateInit2_         edeflateInit2_
#  define deflateInit_          edeflateInit_
#  define deflateParams         edeflateParams
#  define deflatePrime          edeflatePrime
#  define deflateReset          edeflateReset
#  define deflateSetDictionary  edeflateSetDictionary
#  define deflateSetHeader      edeflateSetHeader
#  define deflateTune           edeflateTune
#  define deflate_copyright     edeflate_copyright
#  define get_crc_table         eget_crc_table
#  define gz_error              egz_error
#  define gz_intmax             egz_intmax
#  define gz_strwinerror        egz_strwinerror
#  define gzbuffer              egzbuffer
#  define gzclearerr            egzclearerr
#  define gzclose               egzclose
#  define gzclose_r             egzclose_r
#  define gzclose_w             egzclose_w
#  define gzdirect              egzdirect
#  define gzdopen               egzdopen
#  define gzeof                 egzeof
#  define gzerror               egzerror
#  define gzflush               egzflush
#  define gzgetc                egzgetc
#  define gzgets                egzgets

#  define gzoffset64            egzoffset64

#  define gzopen64              egzopen64
#  define gzprintf              egzprintf
#  define gzputc                egzputc
#  define gzputs                egzputs
#  define gzread                egzread
#  define gzrewind              egzrewind

#  define gzseek64              egzseek64
#  define gzsetparams           egzsetparams

#  define gztell64              egztell64
#  define gzungetc              egzungetc
#  define gzwrite               egzwrite
#  define inflate               einflate
#  define inflateBack           einflateBack
#  define inflateBackEnd        einflateBackEnd
#  define inflateBackInit_      einflateBackInit_
#  define inflateCopy           einflateCopy
#  define inflateEnd            einflateEnd
#  define inflateGetHeader      einflateGetHeader
#  define inflateInit2_         einflateInit2_
#  define inflateInit_          einflateInit_
#  define inflateMark           einflateMark
#  define inflatePrime          einflatePrime
#  define inflateReset          einflateReset
#  define inflateReset2         einflateReset2
#  define inflateSetDictionary  einflateSetDictionary
#  define inflateSync           einflateSync
#  define inflateSyncPoint      einflateSyncPoint
#  define inflateUndermine      einflateUndermine
#  define inflate_copyright     einflate_copyright
#  define inflate_fast          einflate_fast
#  define inflate_table         einflate_table
#  define uncompress            euncompress
#  define zError                ezError
#  define zcalloc               ezcalloc
#  define zcfree                ezcfree
#  define zlibCompileFlags      ezlibCompileFlags
#  define zlibVersion           ezlibVersion

#  define gzopen                egzopen
#  define gzseek                egzseek
#  define gztell                egztell
#  define gzoffset              egzoffset
#  define adler32_combine       eadler32_combine
#  define crc32_combine         ecrc32_combine

#endif
