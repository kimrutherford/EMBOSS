#ifndef ELIBSOURCES
#define ELIBSOURCES
#endif

#ifdef ELIBSOURCES

#define zlibVersion ezlibVersion
#define deflate edeflate
#define deflateEnd edeflateEnd
#define inflate einflate
#define inflateEnd einflateEnd
#define deflateSetDictionary edeflateSetDictionary
#define deflateCopy edeflateCopy
#define deflateReset edeflateReset
#define deflateParams edeflateParams
#define deflateBound edeflateBound
#define deflatePrime edeflatePrime
#define inflateSetDictionary einflateSetDictionary
#define inflateSync einflateSync
#define inflateCopy einflateCopy
#define inflateReset einflateReset
#define inflateBack einflateBack
#define inflateBackEnd einflateBackEnd
#define zlibCompileFlags ezlibCompileFlags
#define compress ecompress
#define compress2 ecompress2
#define compressBound ecompressBound
#define uncompress euncompress
#define gzopen egzopen
#define gzdopen egzdopen
#define gzsetparams egzsetparams
#define gzread egzread
#define gzwrite egzwrite
#define gzprintf egzprintf
#define gzputs egzputs
#define gzgets egzgets
#define gzputc egzputc
#define gzgetc egzgetc
#define gzungetc egzungetc
#define gzflush egzflush
#define gzseek egzseek
#define gzrewind egzrewind
#define gztell egztell
#define gzeof egzeof
#define gzclose egzclose
#define gzclearerr egzclearerr
#define adler32 eadler32
#define crc32 ecrc32
#define crc32_combine ecrc32_combine
#define deflateSetHeader edeflateSetHeader
#define deflateTree edeflateTree
#define gzdirect egzdirect
#define gzerror egzerror
#define inflateGetHeader einflateGetHeader
#define inflatePrime einflatePrime
#define adler32_combine eadler32_combine
#define deflateTune edeflateTune

#define deflateInit_ edeflateInit_
#define deflateInit2_ edeflateInit2_
#define inflateInit_ einflateInit_
#define inflateInit2_ einflateInit2_
#define inflateBackInit_ einflateBackInit_
#define inflateSyncPoint einflateSyncPoint
#define get_crc_table eget_crc_table
#define zError ezError
#define _tr_init _etr_init
#define _tr_flush_block _etr_flush_block
#define inflate_fast einflate_fast
#define inflate_table einflate_table
#define z_error ez_error
#define zcalloc ezcalloc
#define zcfree ezcfree
#endif
