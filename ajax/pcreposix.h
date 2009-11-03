/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* Copyright (c) 1997-2003 University of Cambridge */

#ifndef _PCREPOSIX_H
#define _PCREPOSIX_H

/* This is the header for the POSIX wrapper interface to the PCRE Perl-
Compatible Regular Expression library. It defines the things POSIX says should
be there. I hope. */

/* Have to include stdlib.h in order to ensure that size_t is defined. */

#include <stdlib.h>

/* Allow for C++ users */

#ifdef __cplusplus
extern "C" {
#endif

/* Options defined by POSIX. */

#define REG_ICASE     0x01
#define REG_NEWLINE   0x02
#define REG_NOTBOL    0x04
#define REG_NOTEOL    0x08

/* These are not used by PCRE, but by defining them we make it easier
to slot PCRE into existing programs that make POSIX calls. */

#define REG_EXTENDED  0
#define REG_NOSUB     0

/* Error values. Not all these are relevant or used by the wrapper. */

enum {
  REG_ASSERT = 1,  /* internal error ? */
  REG_BADBR,       /* invalid repeat counts in {} */
  REG_BADPAT,      /* pattern error */
  REG_BADRPT,      /* ? * + invalid */
  REG_EBRACE,      /* unbalanced {} */
  REG_EBRACK,      /* unbalanced [] */
  REG_ECOLLATE,    /* collation error - not relevant */
  REG_ECTYPE,      /* bad class */
  REG_EESCAPE,     /* bad escape sequence */
  REG_EMPTY,       /* empty expression */
  REG_EPAREN,      /* unbalanced () */
  REG_ERANGE,      /* bad range inside [] */
  REG_ESIZE,       /* expression too big */
  REG_ESPACE,      /* failed to get memory */
  REG_ESUBREG,     /* bad back reference */
  REG_INVARG,      /* bad argument */
  REG_NOMATCH      /* match failed */
};


#define NSUBEXP  10

/* @data regex_t **************************************************************
**
** The structure representing a compiled regular expression.
**
** @attr re_pcre [void*] Undocumented
** @attr re_nsub [size_t] Undocumented
** @attr re_erroffset [size_t*] Undocumented
** @attr orig [const char*] Undocumented
** @attr startp [char*[NSUBEXP]] Undocumented
** @attr endp [char*[NSUBEXP]] Undocumented
** @@
******************************************************************************/

typedef struct {
  void *re_pcre;
  size_t re_nsub;
  size_t *re_erroffset;
  const char* orig;
  char *startp[NSUBEXP];
  char *endp[NSUBEXP];
} regex_t;

typedef int regoff_t;

/* @data regmatch_t ***********************************************************
**
** The structure in which a captured offset is returned.
**
** @attr rm_so [regoff_t] Regmatch start offset
** @attr rm_eo [regoff_t] Regmatch end offset
** @@
******************************************************************************/

typedef struct {
  regoff_t rm_so;
  regoff_t rm_eo;
} regmatch_t;

/* The functions */

/*
** Prototype definitions
*/

extern int pcre_regcomp(regex_t *, const char *, int);
extern int pcre_regexec(const regex_t *, const char *, size_t, regmatch_t *, int);
extern size_t pcre_regerror(int, const regex_t *, char *, size_t);
extern void pcre_regfree(regex_t *);

/*
** End of prototype definitions
*/

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* End of pcreposix.h */
