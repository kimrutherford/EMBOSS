
#ifndef ajarch_h
#define ajarch_h


#include <sys/types.h>
#include <stdio.h>


#if defined(__cplusplus)
#define AJ_BEGIN_DECLS extern "C" {
#define AJ_END_DECLS }
#else /* !__cplusplus */
#define AJ_BEGIN_DECLS
#define AJ_END_DECLS
#endif /* !__cplusplus */

AJ_BEGIN_DECLS


#if defined(AJ_Linux64) || defined(AJ_Solaris64) || defined(AJ_IRIX64)  \
    || defined(AJ_OSF164) || defined(AJ_FreeBSD64)
#define HAVE64
#endif

#if !defined(AJ_LinuxLF) && !defined(AJ_SolarisLF) && !defined(AJ_IRIXLF) \
    && !defined(AJ_AIXLF) && !defined(AJ_HPUXLF) && !defined(AJ_MACOSXLF) \
    && !defined(AJ_FreeBSDLF) && !defined(__CYGWIN__) && !defined(WIN32)
typedef int ajint;
typedef long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long ajulong;
#endif


#ifdef AJ_LinuxLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_LinuxLF */

#ifdef AJ_FreeBSDLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_FreeBSDLF */

#ifdef AJ_SolarisLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_SolarisLF */

#ifdef AJ_HPUXLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#if !defined(HPUX64PTRS)
#define fopen(a,b) (FILE*)fopen64(a,b)
#endif /* !HPUX64PTRS */
#endif /* AJ_HPUXLF */

#ifdef AJ_IRIXLF
#define HAVE64
typedef int ajint;
typedef off64_t ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long ajulong;
#define ftell(a) ftell64(a)
#define fseek(a,b,c) fseek64(a,b,c)
#endif /* AJ_IRIXLF */

#ifdef AJ_AIXLF
#define HAVE64
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)

typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#endif /* AJ_AIXLF */

#ifdef AJ_MACOSXLF
#define HAVE64
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)

typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#endif /* AJ_MACOSXLF */

#ifdef __CYGWIN__
#define __int64 long long
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define fseek(a,b,c) fseeko(a,b,c)
#endif

#ifdef WIN32
typedef int ajint;
typedef long ajlong;       /* May need to make this long long. */
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define fseek(a,b,c) _fseeki64(a,b,c)
#endif /* WIN32 */


#ifdef __GNUC__
#define __deprecated __attribute__((deprecated))
#define __noreturn __attribute__((noreturn))
#define __warn_unused_result __attribute__((warn_unused_result))
#else /* !__GNUC__ */
#define __deprecated
#define __noreturn
#define __warn_unused_result
#endif /* !__GNUC__ */

AJ_END_DECLS

#endif

