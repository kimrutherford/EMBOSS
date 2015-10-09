/* @source embossupdate application
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




/* @prog embossupdate *********************************************************
**
** Checks for more recent updates to EMBOSS
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outfile = NULL;
    AjPStr tmpstr = NULL;
    char tmpver[512] = {'\0'};

    AjPStr host  = NULL;
    AjPStr path  = NULL;

    ajint port = 80;

    AjPStr updateurl = NULL;
    AjPStr packagename = NULL;
    AjPStr updatedbname = NULL;

    AjPFilebuff updatefile = NULL;
    AjPStr rdline = NULL;
    AjPStr updatemsg = NULL;
    AjPStr initversion = NULL;
    AjPStr fixversion = NULL;
    AjPStr embossversion = NULL;
    AjPStr embosssystem = NULL;

    ajuint lastfix = 0;
    ajuint v1 = 0;
    ajuint v2 = 0;
    ajuint v3 = 0;
    ajuint v4 = 0;
    AjBool updates = ajFalse;
    AjBool doheader = ajFalse;

    embInit("embossupdate", argc, argv);

    outfile = ajAcdGetOutfile("outfile");

    ajStrAssignS(&packagename, ajNamValuePackage());
    if (!ajStrGetLen(packagename))
        ajStrAssignC(&packagename, "EMBOSS");

    ajStrAssignS(&embossversion, ajNamValueVersion());
    if (!ajStrGetLen(embossversion))
        ajStrAssignC(&embossversion, "0.0.0.0");

    ajFmtScanS(embossversion, "%u.%u.%u.%u", &v1, &v2, &v3, &v4);

    ajStrAssignS(&embosssystem, ajNamValueSystem());
    if (!ajStrGetLen(embosssystem))
        ajStrAssignC(&embosssystem, "(unknown)");

    ajDebug("package '%S' version '%S' system '%S'\n",
            packagename, embossversion, embosssystem);

    updatedbname = ajStrNewC("EMBOSSupdates");

    if(ajStrMatchC(embosssystem, "windows"))
        ajFmtPrintS(&updateurl,
                    "http://emboss.open-bio.org/updates/m%S.fixes",
                    packagename);
    else
        ajFmtPrintS(&updateurl,
                    "http://emboss.open-bio.org/updates/%S.fixes",
                    packagename);

    ajHttpUrlDeconstruct(updateurl, &port, &host, &path);

    ajDebug("updateurl '%S' host '%S' port %u path '%S'\n",
           updateurl, host, port, path);

    updatefile = ajHttpRead(NULL, updatedbname, NULL, host, port, path);

    if(!updatefile)
    {
        ajFmtPrintF(outfile,
                    "%S release %S: update information not available",
                    packagename, embossversion);
    }

    ajFilebuffHtmlNoheader(updatefile);
    doheader = ajFalse;

    while(ajBuffreadLine(updatefile, &rdline))
    {
        ajDebug("doheader %B updates %B version '%S' msglen %u\n%S",
               doheader, updates, fixversion, ajStrGetLen(updatemsg), rdline);

        if(doheader)
        {
            if(ajStrPrefixC(rdline, "Fix "))
                doheader = ajFalse;
            else
                ajStrAppendS(&updatemsg, rdline);
        }

        if(ajStrPrefixC(rdline, "Release "))
        {
            ajFmtScanS(rdline, "Release %S", &initversion);
            ajFmtPrintS(&fixversion, "%S.0", initversion);
            doheader = ajTrue;
        }

        if(ajStrPrefixC(rdline, "Fix "))
        {
            ajFmtScanS(rdline, "Fix %u", &lastfix);
            ajFmtPrintS(&fixversion, "%S.%u", initversion, lastfix);
            doheader = ajFalse;
        }

        if(ajStrCmpS(embossversion, fixversion) < 0)
        {
            if(!updates)
                ajFmtPrintF(outfile,
                            "\nRelease %S: "
                            "A more recent version is available\n\n",
                            embossversion);
            updates = ajTrue;
            ajFmtPrintF(outfile, "%S", rdline);
        }
    }

    if(updates)
    {
        ajFmtPrintF(outfile, "\n%S", updatemsg);
    }

    ajStrAssignC(&tmpstr, ensSoftwareGetVersion());
    if (!ajStrGetLen(tmpstr))
        ajStrAssignC(&tmpstr, "(unknown)");

    c_plgver(tmpver);
    ajStrAssignC(&tmpstr, tmpver);
    if (!ajStrGetLen(tmpstr))
        ajStrAssignC(&tmpstr, "(unknown)");

#ifdef ELIBSOURCES
    ajStrAssignC(&tmpstr, pcre_version());
#else
    ajStrAssignC(&tmpstr, "");
#endif

    if (!ajStrGetLen(tmpstr))
        ajStrAssignC(&tmpstr, "(unknown)");

    ajStrAssignC(&tmpstr, zlibVersion());
    if (!ajStrGetLen(tmpstr))
        ajStrAssignC(&tmpstr, "(unknown)");

    ajStrAssignC(&tmpstr, XML_ExpatVersion());
    if (!ajStrGetLen(tmpstr))
        ajStrAssignC(&tmpstr, "(unknown)");

    if(!updates)
        ajFmtPrintF(outfile, "%S %S is the latest version (%S)\n",
                    packagename, embossversion, fixversion);

    ajStrDel(&updatemsg);
    ajStrDel(&updateurl);
    ajStrDel(&packagename);
    ajStrDel(&updatedbname);
    ajStrDel(&rdline);
    ajStrDel(&initversion);
    ajStrDel(&fixversion);
    ajStrDel(&embossversion);
    ajStrDel(&embosssystem);
    ajStrDel(&tmpstr);
    ajFileClose(&outfile);
    ajFilebuffDel(&updatefile);
    ajStrDel(&host);
    ajStrDel(&path);

    embExit();

    return 0;
}
