/* @source embossversion application
**
** Writes the current EMBOSS version number
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include "expat.h"
#include "zlib.h"




/* @prog embossversion ********************************************************
**
** Writes the current EMBOSS version number
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outfile = NULL;
    AjBool full = AJFALSE;
    AjPStr tmpstr = NULL;
    char tmpver[512] = {'\0'};

    embInit("embossversion", argc, argv);

    full = ajAcdGetBoolean("full");
    outfile = ajAcdGetOutfile("outfile");

    tmpstr = ajStrNewRes(128);

    if (!full) {
        ajStrAssignS(&tmpstr, ajNamValueVersion());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "%S\n", tmpstr);
    }
    else {
        ajStrAssignS(&tmpstr, ajNamValuePackage());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "PackageName: %S\n", tmpstr);

        ajStrAssignS(&tmpstr, ajNamValueVersion());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "LibraryVersion: %S\n", tmpstr);

        ajStrAssignC(&tmpstr, ensSoftwareGetVersion());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "EnsemblVersion: %S\n", tmpstr);

        c_plgver(tmpver);
        ajStrAssignC(&tmpstr, tmpver);
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "PlplotVersion: %S\n", tmpstr);

#ifdef ELIBSOURCES
        ajStrAssignC(&tmpstr, pcre_version());
#else
        ajStrAssignC(&tmpstr, "");
#endif
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "PcreVersion: %S\n", tmpstr);

        ajStrAssignC(&tmpstr, zlibVersion());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "ZlibVersion: %S\n", tmpstr);

        ajStrAssignC(&tmpstr, XML_ExpatVersion());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "ExpatVersion: %S\n", tmpstr);

        ajStrAssignS(&tmpstr, ajNamValueSystem());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "System: %S\n", tmpstr);

        ajStrAssignS(&tmpstr, ajNamValueInstalldir());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "InstallDirectory: %S\n", tmpstr);

        ajStrAssignS(&tmpstr, ajNamValueRootdir());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "RootDirectory: %S\n", tmpstr);
	    
        ajStrAssignS(&tmpstr, ajNamValueBasedir());
	if (!ajStrGetLen(tmpstr))
	    ajStrAssignC(&tmpstr, "(unknown)");
	ajFmtPrintF(outfile, "BaseDirectory: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("standard", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Standard: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("userdir", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_UserDir: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("acdroot", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_AcdRoot: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("data", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Data: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("filter", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Filter: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("format", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Format: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("graphics", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Graphics: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("httpversion", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_HttpVersion: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("language", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Language: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("options", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Options: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outdirectory", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutDirectory: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outfeatformat", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutFeatFormat: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("outformat", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_OutFormat: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("proxy", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Proxy: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("stdout", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Stdout: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("debug", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Debug: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("acdwarnrange", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_WarnRange: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("sql", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Sql: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("mysql", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Mysql: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("postgresql", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Postgresql: %S\n", tmpstr);
	    
	if (!ajNamGetValueC("axis2c", &tmpstr))
	    ajStrAssignC(&tmpstr, "(default)");
	ajFmtPrintF(outfile, "Emboss_Axis2c: %S\n", tmpstr);

	ajFmtPrintF(outfile, "\n");
        ajFmtPrintF(outfile, "Compile Flag Definitions:\n");
	ajFmtPrintF(outfile, "\n");
#ifdef HAVE64
	ajFmtPrintF(outfile, "HAVE64\n");
#endif
#ifdef AJ_Linux64
	ajFmtPrintF(outfile, "AJ_Linux64\n");
#endif
#ifdef AJ_Solaris64
	ajFmtPrintF(outfile, "AJ_Solaris64\n");
#endif
#ifdef AJ_IRIX64
	ajFmtPrintF(outfile, "AJ_IRIX64\n");
#endif
#ifdef AJ_OSF164
	ajFmtPrintF(outfile, "AJ_OSF164\n");
#endif
#ifdef AJ_FreeBSD64
	ajFmtPrintF(outfile, "AJ_FreeBSD64\n");
#endif
#ifdef AJ_LinuxLF
	ajFmtPrintF(outfile, "AJ_LinuxLF\n");
#endif
#ifdef AJ_SolarisLF
	ajFmtPrintF(outfile, "AJ_SolarisLF\n");
#endif
#ifdef AJ_IRIXLF
	ajFmtPrintF(outfile, "AJ_IRIXLF\n");
#endif
#ifdef AJ_AIXLF
	ajFmtPrintF(outfile, "AJ_AIXLF\n");
#endif
#ifdef AJ_HPUXLF
	ajFmtPrintF(outfile, "AJ_HPUXLF\n");
#endif
#ifdef AJ_MACOSXLF
	ajFmtPrintF(outfile, "AJ_MACOSXLF\n");
#endif
#ifdef AJ_FreeBSDLF
	ajFmtPrintF(outfile, "AJ_FreeBSDLF\n");
#endif
#ifdef __CYGWIN__
	ajFmtPrintF(outfile, "__CYGWIN__\n");
#endif
#ifdef WIN32
	ajFmtPrintF(outfile, "WIN32\n");
#endif
#ifdef HAVE_CONFIG_H
	ajFmtPrintF(outfile, "HAVE_CONFIG_H\n");
#endif
#ifdef __GNUC__
	ajFmtPrintF(outfile, "__GNUC__\n");
#endif
#ifdef S_IFREG
	ajFmtPrintF(outfile, "S_IFREG\n");
#endif
#ifdef S_IFDIR
	ajFmtPrintF(outfile, "S_IFDIR\n");
#endif
#ifdef __cplusplus
	ajFmtPrintF(outfile, "__cplusplus\n");
#endif

        /* config.h definitions */

	ajFmtPrintF(outfile, "\n");
        ajFmtPrintF(outfile, "Config Header Definitions:\n");
	ajFmtPrintF(outfile, "\n");
#ifdef AXIS2C_LOC
	ajFmtPrintF(outfile, "AXIS2C_LOC '%s'\n", AXIS2C_LOC);
#endif
#ifdef GETPGRP_VOID
	ajFmtPrintF(outfile, "GETPGRP_VOID %d\n", GETPGRP_VOID);
#endif
#ifdef HAVE_AXIS2C
	ajFmtPrintF(outfile, "HAVE_AXIS2C %d\n", HAVE_AXIS2C);
#endif
#ifdef HAVE_DIRENT_H
	ajFmtPrintF(outfile, "HAVE_DIRENT_H %d\n", HAVE_DIRENT_H);
#endif
#ifdef HAVE_DLFCN_H
	ajFmtPrintF(outfile, "HAVE_DLFCN_H %d\n", HAVE_DLFCN_H);
#endif
#ifdef HAVE_DOPRNT
	ajFmtPrintF(outfile, "HAVE_DOPRNT %d\n", HAVE_DOPRNT);
#endif
#ifdef HAVE_FORK
	ajFmtPrintF(outfile, "HAVE_FORK %d\n", HAVE_FORK);
#endif
#ifdef HAVE_INTTYPES_H
	ajFmtPrintF(outfile, "HAVE_INTTYPES_H %d\n", HAVE_INTTYPES_H);
#endif
#ifdef HAVE_JAVA
	ajFmtPrintF(outfile, "HAVE_JAVA\n");
#endif
#ifdef HAVE_LIBM
	ajFmtPrintF(outfile, "HAVE_LIBM %d\n", HAVE_LIBM);
#endif
#ifdef HAVE_MCHECK
	ajFmtPrintF(outfile, "HAVE_MCHECK %d\n", HAVE_MCHECK);
#endif
#ifdef HAVE_MEMMOVE
	ajFmtPrintF(outfile, "HAVE_MEMMOVE %d\n", HAVE_MEMMOVE);
#endif
#ifdef HAVE_MEMORY_H
	ajFmtPrintF(outfile, "HAVE_MEMORY_H %d\n", HAVE_MEMORY_H);
#endif
#ifdef HAVE_MYSQL
	ajFmtPrintF(outfile, "HAVE_MYSQL %d\n", HAVE_MYSQL);
#endif
#ifdef HAVE_NDIR_H
	ajFmtPrintF(outfile, "HAVE_NDIR_H %d\n", HAVE_NDIR_H);
#endif
#ifdef HAVE_POSTGRESQL
	ajFmtPrintF(outfile, "HAVE_POSTGRESQL %d\n", HAVE_POSTGRESQL);
#endif
#ifdef HAVE_STDINT_H
	ajFmtPrintF(outfile, "HAVE_STDINT_H %d\n", HAVE_STDINT_H);
#endif
#ifdef HAVE_STDLIB_H
	ajFmtPrintF(outfile, "HAVE_STDLIB_H %d\n", HAVE_STDLIB_H);
#endif
#ifdef HAVE_STRFTIME
	ajFmtPrintF(outfile, "HAVE_STRFTIME %d\n", HAVE_STRFTIME);
#endif
#ifdef HAVE_STRINGS_H
	ajFmtPrintF(outfile, "HAVE_STRINGS_H %d\n", HAVE_STRINGS_H);
#endif
#ifdef HAVE_STRING_H
	ajFmtPrintF(outfile, "HAVE_STRING_H %d\n", HAVE_STRING_H);
#endif
#ifdef HAVE_SYS_DIR_H
	ajFmtPrintF(outfile, "HAVE_SYS_DIR_H %d\n", HAVE_SYS_DIR_H);
#endif
#ifdef HAVE_SYS_NDIR_H
	ajFmtPrintF(outfile, "HAVE_SYS_NDIR_H %d\n", HAVE_SYS_NDIR_H);
#endif
#ifdef HAVE_SYS_STAT_H
	ajFmtPrintF(outfile, "HAVE_SYS_STAT_H %d\n", HAVE_SYS_STAT_H);
#endif
#ifdef HAVE_SYS_TYPES_H
	ajFmtPrintF(outfile, "HAVE_SYS_TYPES_H %d\n", HAVE_SYS_TYPES_H);
#endif
#ifdef HAVE_TARGETCONFIG_H
	ajFmtPrintF(outfile, "HAVE_TARGETCONFIG_H %d\n", HAVE_TARGETCONFIG_H);
#endif
#ifdef HAVE_UNISTD_
	ajFmtPrintF(outfile, "HAVE_UNISTD_ %d\n", HAVE_UNISTD_);
#endif
#ifdef HAVE_VFORK
	ajFmtPrintF(outfile, "HAVE_VFORK %d\n", HAVE_VFORK);
#endif
#ifdef HAVE_VFORK_H
	ajFmtPrintF(outfile, "HAVE_VFORK_H %d\n", HAVE_VFORK_H);
#endif
#ifdef HAVE_VPRINTF
	ajFmtPrintF(outfile, "HAVE_VPRINTF %d\n", HAVE_VPRINTF);
#endif
#ifdef HAVE_WORKING_FORK
	ajFmtPrintF(outfile, "HAVE_WORKING_FORK %d\n", HAVE_WORKING_FORK);
#endif
#ifdef HAVE_WORKING_VFORK
	ajFmtPrintF(outfile, "HAVE_WORKING_VFORK %d\n",HAVE_WORKING_VFORK );
#endif
#ifdef HPUX64PTRS
	ajFmtPrintF(outfile, "HPUX64PTRS %d\n", HPUX64PTRS);
#endif
#ifdef LT_OBJDIR
	ajFmtPrintF(outfile, "LT_OBJDIR '%s'\n", LT_OBJDIR);
#endif
#ifdef PACKAGE
	ajFmtPrintF(outfile, "PACKAGE '%s'\n", PACKAGE);
#endif
#ifdef PACKAGE_BUGREPORT
	ajFmtPrintF(outfile, "PACKAGE_BUGREPORT '%s'\n", PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_NAME
	ajFmtPrintF(outfile, "PACKAGE_NAME '%s'\n", PACKAGE_NAME);
#endif
#ifdef PACKAGE_STRING
	ajFmtPrintF(outfile, "PACKAGE_STRING '%s'\n", PACKAGE_STRING);
#endif
#ifdef PACKAGE_TARNAME
	ajFmtPrintF(outfile, "PACKAGE_TARNAME '%s'\n", PACKAGE_TARNAME);
#endif
#ifdef PACKAGE_URL
	ajFmtPrintF(outfile, "PACKAGE_URL '%s'\n", PACKAGE_URL);
#endif
#ifdef PACKAGE_VERSION
	ajFmtPrintF(outfile, "PACKAGE_VERSION '%s'\n", PACKAGE_VERSION);
#endif
#ifdef PLD_pdf
	ajFmtPrintF(outfile, "PLD_pdf %d\n", PLD_pdf);
#endif
#ifdef PLD_png
	ajFmtPrintF(outfile, "PLD_png %d\n", PLD_png);
#endif
#ifdef PLD_xwin
	ajFmtPrintF(outfile, "PLD_xwin %d\n", PLD_xwin);
#endif
#ifdef STDC_HEADERS
	ajFmtPrintF(outfile, "STDC_HEADERS %d\n", STDC_HEADERS);
#endif
#ifdef TM_IN_SYS_TIME
	ajFmtPrintF(outfile, "TM_IN_SYS_TIME %d\n", TM_IN_SYS_TIME);
#endif
#ifdef VERSION
	ajFmtPrintF(outfile, "VERSION '%s'\n", VERSION);
#endif
#ifdef WORDS_BIGENDIAN
	ajFmtPrintF(outfile, "WORDS_BIGENDIAN %d\n", WORDS_BIGENDIAN);
#endif
#ifdef X_DISPLAY_MISSING
	ajFmtPrintF(outfile, "X_DISPLAY_MISSING %d\n", X_DISPLAY_MISSING);
#endif
#ifdef _FORTIFY_SOURCE
	ajFmtPrintF(outfile, "_FORTIFY_SOURCE %d\n", _FORTIFY_SOURCE);
#endif
	ajFmtPrintF(outfile, "\n");
	ajFmtPrintF(outfile, "Sizes (bytes):\n");
	ajFmtPrintF(outfile, "\n");
	ajFmtPrintF(outfile, "int          %2u\n", sizeof(int));
	ajFmtPrintF(outfile, "ajint        %2u\n", sizeof(ajint));
	ajFmtPrintF(outfile, "short        %2u\n", sizeof(short));
	ajFmtPrintF(outfile, "ajshort      %2u\n", sizeof(ajshort));
	ajFmtPrintF(outfile, "long         %2u\n", sizeof(long));
	ajFmtPrintF(outfile, "ajlong       %2u\n", sizeof(ajlong));
	ajFmtPrintF(outfile, "void pointer %2u\n", sizeof(void*));
	ajFmtPrintF(outfile, "size_t       %2u\n", sizeof(size_t));
    }

    ajStrDel(&tmpstr);
    ajFileClose(&outfile);

    embExit();

    return 0;
}
