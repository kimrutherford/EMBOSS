/* @source acdgalaxy application
**
** Convert ACD file into GALAXY tool
**
** @author Peter Rice
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




/* @prog acdgalaxy ************************************************************
**
** ACD converter to GALAXY tool definition
**
** Unlike other ACD report programs (acdpretty and acdtable) acdgalaxy will run
** through command line checking and prompting the user, while converting the
** internal data into a GALAXY tool definition
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajuint iargs = 1;

    if(argc < 2)
	ajFatal("Error - must specify an application to compile\n");

    ajAcdSetControl("acdhelp");
    ajAcdSetControl("acdgalaxy");
    ajAcdSetControl("acdnocommandline");
    if(ajCharMatchC(argv[1], "protein"))
    {
        iargs++;
        ajAcdSetControl("acdgalaxyprotein");
    }
    else if(ajCharMatchC(argv[1], "nucleotide"))
    {
        iargs++;
        ajAcdSetControl("acdgalaxynucleotide");
    }
    
    embInit(argv[iargs], argc-iargs, &argv[iargs]);

    ajAcdExit(ajTrue);		/* turn off the 'never used' ACD warnings */

    embExit();

    return 0;
}

/*
** Notes on Galaxy definitions
**
** galaxy_dist/tools/emboss_5/emboss_xxxx.xml
** DOS file!!!
**
** <tool> tag
** name with numeric suffix (how many have mnore than one?) e.g. antigenic1
** version
**
** <description> (first para of documentation)
**
** <command> name, qualifiers with variable names for input1.. out_file1 ...
**       paramname out_format (rformat etc.)
**
** <inputs>
** sequence input:
**   <param format="data" for sequence etc. <label>On query</label>
**
** general qualifiers:
**   <param name="qualname" size="n" (width?) type="text" (even for numbers)
**          value="defvalue">
**   <label>information/help string</label>
**
** lists e.g. rformat
** <param name="out_format1" type="select">
**    <label>standard help text
**    <option value="formatname"FormatName</option>
**
** output:
**   <output>
**     <data format="appname" name="out_file1">
**        why use the appname for the format? why not embossreport?
**
** tests:
** ======
**
** <param> name="input1" value="testfile1"/>
**  ... set values for all inputs and for outfile
** <output name="out_file1" file="emboss_appname_out.somename"/>
**
** Python part
** ===========
**
** <code file="emboss_format_corrector.py" />
** picks up any output from the string out_format1 etc as a known format:
** ncbi = fasta
** excel = tabular
** text = txt
**
** Then checks html
** then checks png for graphics
**
** <help> link to docs for this version on emboss.sf.net
**
** ... and that's all
** ... at least for antigenic ... needle is followed by text and comments
** which look very much like the help text, with default values,
** and then an example input and output
**
** need to check how this actually looks in GALAXY
**
** Consider new output format(s) for GALAXY
**
** Consider limiting the formats to those GALAXY supports
** Maybe add a full output format option for display only?
**
** Consider how to define the tests
** test data lives in galaxy_dist/test-data
** How are the tests run?
*/

