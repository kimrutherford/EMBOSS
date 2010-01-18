/* @source bundlewin application
**
** Microsoft Windows Developer bundling
**
** @author Alan Bleasby
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
**
**
** This is a UNIX application, only tested under Linux. Its purpose
** is to take a CVS checkout of EMBOSS from the developers' site
** and construct a zipped bundle suitable for compiling under
** Microsoft Visual C++ Express 2005.
**
** Compile using: cc -o bundlewin -O2 bundlewin.c
**
** Usage: If you have checked-out EMBOSS from CVS in /fu/bar then:
**
**        ./bundlewin /fu/bar/emboss
**
** This will create a file called   '/fu/bar/emboss/emboss.zip'
** The zip file can then be transferred to Windows. When extracted
** it will create a folder called 'win32'.
******************************************************************************/

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define COREDEF "ajaxdll.def"
#define PCREDEF "epcredll.def"
#define EXPATDEF "eexpatdll.def"
#define ZLIBDEF "ezlibdll.def"
#define GRAPHICSDEF "ajaxgdll.def"
#define ENSEMBLDEF "ensembldll.def"
#define AJAXDBDEF "ajaxdbdll.def"
#define ACDDEF "acddll.def"
#define NUCLEUSDEF "nucleusdll.def"

#define TMPFILE  "/tmp/hfuncts_1"
#define TMPFILE2 "/tmp/hfuncts_2"
#define BUILDDIR "winbuild"

#define PROJECT "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\")"




typedef struct node
{
    char prog[MAXNAMLEN];
    struct node *next;
} listnode;


/*
** Exclude specific programs
*/

static char *exclude_names[]=
{
    "dbxreport.c",
    "dbxstat.c",
    NULL
};

    

/*
** Specify any programs with special memory reqirements.
*/

typedef struct SStackHeap
{
    char *progname;
    char *stacksize;
    char *heapsize;
} OStackHeap;

static OStackHeap memory[] = 
{
    {"cirdna", "5000000", "5000000"},
    {"lindna", "7000000", "7000000"},
    {NULL, NULL, NULL}
};


    
    
static int dir_exists(char *path);
static int file_exists(char *path);

static void copy_ajax(char *path);
static void copy_nucleus(char *path);
static void copy_plplot_inc(char *basedir);
static void copy_DLLs(char *basedir);
static int  copy_apps(char *basedir, listnode *head);
static void copy_data(char *basedir);
static void copy_doc(char *basedir);
static void copy_test(char *basedir);
static void copy_jemboss(char *basedir);
static void copy_mysql(char *basedir);
static void copy_redist(char *basedir);
    
static void create_directories(char *basedir);
static void make_ajax_header_exports(char *basedir,char *subdir);
static int header_exports(char *dir, FILE *fp);
static void extract_funcnames(char *filename, FILE *fout);
static void write_coreexports(char *fn, char *basedir);
static void write_genajaxexport(char *fn,char *basedir,char *exportdef,
                                char *subdir,char *dllname);

static void write_nucleusexports(char *fn,char *basedir);
static void read_make_check(char *basedir, listnode **head);
static void add_node(listnode **head, char *progname);
static void make_uids(char ***uids, int napps);
static void read_prognames(char *basedir, int napps, char ***names);
static void write_solution(char *basedir, char **prognames, char **uids,
			   int napps);
static void write_projects(char *basedir, char **prognames, char **uids,
			   int napps);
static void sub_text(char *line, char *tline, char *given, char *rep);
static void zip_up(char *basedir);

static void extract_expat_funcnames(char *filename, FILE *fout);
static void make_expat_header_exports(char *basedir);
static void make_zlib_header_exports(char *basedir);
static void extract_zlib_funcnames(char *filename, FILE *fout);




int main(int argc, char **argv)
{
    char basedir[MAXNAMLEN];
    char ajaxdir[MAXNAMLEN];
    char nucleusdir[MAXNAMLEN];
    listnode *head = NULL;
    listnode *ptr;
    listnode *tptr;
    char **uids = NULL;
    char **prognames = NULL;
    char *vcfiles = NULL;

    FILE *fp = NULL;
    
    int len;
    int napps;
    
    if(argc>1)
	strcpy(basedir,argv[1]);
    else
    {
	fprintf(stdout,"Path to emboss CVS top level: ");
	fgets(basedir,MAXNAMLEN,stdin);
	len = strlen(basedir);
	if(basedir[len-1] == '\n')
	    basedir[len-1] = '\0';
    }




    /* Create Windows build directories as a 'win32' subdir of basedir */
    create_directories(basedir);

    /* Copy project files for ajax, nucleus and DLLs */    

    copy_ajax(basedir);
    copy_nucleus(basedir);
    copy_plplot_inc(basedir);
    copy_DLLs(basedir);

    /* Copy data files */

    copy_data(basedir);
    copy_doc(basedir);
    copy_test(basedir);
    copy_jemboss(basedir);
    copy_mysql(basedir);
    copy_redist(basedir);



    
    /* Construct exports file (ajaxdll.def) */


    
    make_ajax_header_exports(basedir,"core");
    write_coreexports(TMPFILE,basedir);

    make_expat_header_exports(basedir);
    write_genajaxexport(TMPFILE,basedir,EXPATDEF,"expat","eexpat");

    make_zlib_header_exports(basedir);
    write_genajaxexport(TMPFILE,basedir,ZLIBDEF,"zlib","ezlib");


    make_ajax_header_exports(basedir,"pcre");    
    write_genajaxexport(TMPFILE,basedir,PCREDEF,"pcre","epcre");


    make_ajax_header_exports(basedir,"graphics");
    write_genajaxexport(TMPFILE,basedir,GRAPHICSDEF,"graphics","ajaxg");


    make_ajax_header_exports(basedir,"ensembl");
    write_genajaxexport(TMPFILE,basedir,ENSEMBLDEF,"ensembl","ensembl");


    make_ajax_header_exports(basedir,"ajaxdb");    
    write_genajaxexport(TMPFILE,basedir,AJAXDBDEF,"ajaxdb","ajaxdb");


    make_ajax_header_exports(basedir,"acd");        
    write_genajaxexport(TMPFILE,basedir,ACDDEF,"acd","acd");
    

    /* Construct exports file (nucleusdll.def) */

    sprintf(nucleusdir,"%s/emboss/nucleus",basedir);
    
    if(!(fp=fopen(TMPFILE,"w")))
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }
    

    if(header_exports(nucleusdir,fp) < 0)
    {
	fprintf(stderr,"Cannot open directory %s/nucleus",basedir);
	exit(-1);
    }
    fclose(fp);
    
    write_nucleusexports(TMPFILE,basedir);
    

    read_make_check(basedir,&head);
    napps = copy_apps(basedir,head);

    ptr = head;
    while(ptr->next)
    {
	tptr = ptr;
	ptr = ptr->next;
	free(tptr);
    }
    

    make_uids(&uids,napps);
    read_prognames(basedir,napps,&prognames);

    write_solution(basedir,prognames,uids,napps);
    write_projects(basedir,prognames,uids,napps);

    /* Create the zip archive */
    zip_up(basedir);

    return 0;
}




static void copy_nucleus(char *basedir)
{
    char command[MAXNAMLEN];
    
    sprintf(command,"cp -f %s/emboss/nucleus/*.h %s/win32/nucleus",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/nucleus/*.c %s/win32/nucleus",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_ajax(char *basedir)
{
    char command[MAXNAMLEN];

    /* First copy ajax .h & .c files */

    sprintf(command,"cp -f %s/emboss/ajax/pcre/*.h %s/win32/ajax/pcre",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/pcre/*.c %s/win32/ajax/pcre",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/expat/*.h %s/win32/ajax/expat",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/expat/*.c %s/win32/ajax/expat",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/zlib/*.h %s/win32/ajax/zlib",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/zlib/*.c %s/win32/ajax/zlib",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/core/*.h %s/win32/ajax/core",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/core/*.c %s/win32/ajax/core",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/ajax/graphics/*.h %s/win32/ajax/graphics",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/graphics/*.c %s/win32/ajax/graphics",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/ajax/ensembl/*.h %s/win32/ajax/ensembl",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/ensembl/*.c %s/win32/ajax/ensembl",
            basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/ajax/ajaxdb/*.h %s/win32/ajax/ajaxdb",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/ajaxdb/*.c %s/win32/ajax/ajaxdb",
            basedir, basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/ajax/acd/*.h %s/win32/ajax/acd",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/ajax/acd/*.c %s/win32/ajax/acd",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    
    
    /* Copy extra files required by win32 */
    sprintf(command,"cp -fR %s/emboss/win32/ajax/* %s/win32/ajax/core",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    return;
}




static void copy_plplot_inc(char *basedir)
{
    char command[MAXNAMLEN];


    sprintf(command,"cp -f %s/emboss//plplot/*.h "
	    "%s/win32/plplot-inc/include/plplot",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/plplotwin/*.h "
	    "%s/win32/plplot-inc/include/plplot",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/plplotwin/gd/include/*.h "
	    "%s/win32/plplot-inc/include/plplot",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }



    sprintf(command,"cp -f %s/emboss/win32/plplot-inc/lib/Debug/plplot.lib "
	    "%s/win32/plplot-inc/lib/Debug",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/win32/plplot-inc/lib/Release/plplot.lib "
	    "%s/win32/plplot-inc/lib/Release",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/plplotwin/gd/lib/bgd.lib "
	    "%s/win32/plplot-inc/lib/Debug",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/plplotwin/gd/lib/bgd.lib "
	    "%s/win32/plplot-inc/lib/Release",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp -f %s/emboss/plplotwin/gd/lib/bgd.dll "
	    "%s/win32/DLLs/Debug",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -f %s/emboss/plplotwin/gd/lib/bgd.dll "
	    "%s/win32/DLLs/Release",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_DLLs(char *basedir)
{
    char command[MAXNAMLEN];

    /* First copy top level files */

    sprintf(command,"cp -f %s/emboss/win32/DLLs/DLLs.* %s/win32/DLLs",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    /* Copy ajax project files */

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/acd.vcproj "
            "%s/win32/DLLs/ajax/acd",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/ajaxdb.vcproj "
            "%s/win32/DLLs/ajax/ajaxdb",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/ajaxg.vcproj "
            "%s/win32/DLLs/ajax/graphics",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/core.vcproj "
            "%s/win32/DLLs/ajax/core",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/ensembl.vcproj "
            "%s/win32/DLLs/ajax/ensembl",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/expat.vcproj "
            "%s/win32/DLLs/ajax/expat",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/zlib.vcproj "
            "%s/win32/DLLs/ajax/zlib",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/ajax/pcre.vcproj "
            "%s/win32/DLLs/ajax/pcre",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    /* Copy nucleus project files */

    sprintf(command,"cp -fR %s/emboss/win32/DLLs/nucleus/* "
	    "%s/win32/DLLs/nucleus",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_data(char *basedir)
{
    char command[MAXNAMLEN];



    sprintf(command,"cp -dfpR %s/emboss/emboss/data %s/win32",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fp %s/emboss/plplot/lib/pl*.fnt %s/win32/data",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fp %s/emboss/win32/misc/*.txt %s/win32/data",
	    basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp -fp %s/emboss/win32/misc/emboss.default "
	    "%s/win32/apps/release",basedir,basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}



static void copy_doc(char *basedir)
{
    char command[MAXNAMLEN];

    sprintf(command,"cp -dfpR %s/emboss/doc %s/win32",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_jemboss(char *basedir)
{
    char command[MAXNAMLEN];

    sprintf(command,"cp -dfpR %s/emboss/jemboss %s/win32",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_test(char *basedir)
{
    char command[MAXNAMLEN];

    sprintf(command,"cp -dfpR %s/emboss/test %s/win32",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_mysql(char *basedir)
{
    char command[MAXNAMLEN];

    sprintf(command,"cp %s/emboss/win32/mysql/Debug/*.dll "
            "%s/win32/mysql/Debug",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/mysql/Debug/*.lib "
            "%s/win32/mysql/Debug",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/mysql/Debug/*.pdb "
            "%s/win32/mysql/Debug",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/mysql/Release/*.dll "
            "%s/win32/mysql/Release",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp %s/emboss/win32/mysql/Release/*.lib "
            "%s/win32/mysql/Release",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp %s/emboss/win32/mysql/Release/*.pdb "
            "%s/win32/mysql/Release",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }


    sprintf(command,"cp %s/emboss/win32/mysql/include/*.h "
            "%s/win32/mysql/include",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/mysql/include/*.def "
            "%s/win32/mysql/include",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/mysql/include/mysql/*.h "
            "%s/win32/mysql/include/mysql",basedir,
	    basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void copy_redist(char *basedir)
{
    char command[MAXNAMLEN];
    char prompt[MAXNAMLEN];
    
    static char *def="/dos8/vc2008/vc/redist/x86/Microsoft.VC90.CRT";
    static char *def2="/root/vc80";
    int len;
    
    char vc80[MAXNAMLEN];
    char vc90[MAXNAMLEN];
    char *ev = NULL;


    if((ev = getenv("vc90files")))
        strcpy(vc90,ev);
    else
        strcpy(vc90,def);
    

    fprintf(stdout,"Redist 90 file directory [%s]: ",vc90);
    fgets(prompt,MAXNAMLEN,stdin);
    if(*prompt == '\n')
        sprintf(prompt,"%s",vc90);

    len = strlen(prompt);
    if(prompt[len-1] == '\n')
        prompt[len-1] = '\0';

    
    sprintf(command,"cp %s/* %s/win32/redist",prompt,basedir);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    if((ev = getenv("vc80files")))
        strcpy(vc80,ev);
    else
        strcpy(vc80,def2);

    
    fprintf(stdout,"Redist 80 file directory [%s]: ",vc80);
    fgets(prompt,MAXNAMLEN,stdin);
    if(*prompt == '\n')
        sprintf(prompt,"%s",vc80);

    len = strlen(prompt);
    if(prompt[len-1] == '\n')
        prompt[len-1] = '\0';

    
    sprintf(command,"cp %s/* %s/win32/redist",prompt,basedir);

    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void zip_up(char *basedir)
{
    char command[MAXNAMLEN];

    fprintf(stdout,"Creating %s/emboss.zip\n",basedir);

    sprintf(command,"find %s/win32 -name CVS -exec rm -rf {} \\; "
	    ">/dev/null 2>&1",basedir,basedir);
    system(command);

    sprintf(command,"cd %s; zip -r emboss win32 >/dev/null 2>&1",basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"rm -rf %s/win32",basedir);
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    return;
}




static void create_directories(char *basedir)
{
    char newdir[MAXNAMLEN];
    static char *dirs[] = 
    {
	"win32",
	"win32/ajax",
        "win32/ajax/pcre",
        "win32/ajax/expat",
        "win32/ajax/zlib",
        "win32/ajax/core",
        "win32/ajax/graphics",
        "win32/ajax/ensembl",
        "win32/ajax/ajaxdb",
        "win32/ajax/acd",
	"win32/nucleus",
	"win32/emboss",
	"win32/apps",
	"win32/acd",
	"win32/DLLs",
        "win32/redist",
	"win32/plplot-inc",
	"win32/plplot-inc/include",
	"win32/plplot-inc/include/plplot",
	"win32/plplot-inc/lib",
	"win32/plplot-inc/lib/Debug",
	"win32/plplot-inc/lib/Release",
	"win32/DLLs/ajax",
	"win32/DLLs/ajax/pcre",
	"win32/DLLs/ajax/expat",
	"win32/DLLs/ajax/zlib",
	"win32/DLLs/ajax/core",
	"win32/DLLs/ajax/graphics",
	"win32/DLLs/ajax/ensembl",
	"win32/DLLs/ajax/ajaxdb",
	"win32/DLLs/ajax/acd",
	"win32/DLLs/ajax/core/Debug",
	"win32/DLLs/ajax/core/Release",
	"win32/DLLs/ajax/pcre/Debug",
	"win32/DLLs/ajax/pcre/Release",
	"win32/DLLs/ajax/expat/Debug",
	"win32/DLLs/ajax/expat/Release",
	"win32/DLLs/ajax/zlib/Debug",
	"win32/DLLs/ajax/zlib/Release",
	"win32/DLLs/ajax/graphics/Debug",
	"win32/DLLs/ajax/graphics/Release",
	"win32/DLLs/ajax/ensembl/Debug",
	"win32/DLLs/ajax/ensembl/Release",
	"win32/DLLs/ajax/ajaxdb/Debug",
	"win32/DLLs/ajax/ajaxdb/Release",
	"win32/DLLs/ajax/acd/Debug",
	"win32/DLLs/ajax/acd/Release",
	"win32/DLLs/nucleus",
	"win32/DLLs/nucleus/Debug",
	"win32/DLLs/nucleus/Release",
	"win32/DLLs/Debug",
	"win32/DLLs/Release",
	"win32/DLLs/_UpgradeReport_Files",
	"win32/apps/debug",
	"win32/apps/release",
	"win32/apps/emboss",
	"win32/apps/_UpgradeReport_Files",
	"win32/mysql",
	"win32/mysql/include",
 	"win32/mysql/include/mysql",
	"win32/mysql/Debug",
	"win32/mysql/Release",
	NULL
    };
    int i;


    i = 0;
    while(dirs[i])
    {
	sprintf(newdir,"%s/%s",basedir,dirs[i]);

	if(!dir_exists(newdir))
	    if(mkdir(newdir,0755) == -1)
	    {
		fprintf(stderr,"Cannot create directory %s\n",newdir);
		exit(-1);
	    }
	++i;
    }

    return;
}





static int header_exports(char *dir, FILE *fp)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char filename[MAXNAMLEN];
    

    if(!(indir = opendir(dir)))
	return -1;


    while((dp=readdir(indir)))
    {
#ifndef CYGWIN
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif
	len = strlen(dp->d_name);
	if(len > 2 && !strcmp(dp->d_name + len - 2, ".h"))
	{
	    sprintf(filename,"%s/%s",dir,dp->d_name);
#ifdef DEBUG
	    fprintf(stdout,"Processing %s\n",filename);
#endif
	    extract_funcnames(filename,fp);
	}
	
    }

    closedir(indir);

    return 0;
}




static void extract_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    char command[MAXNAMLEN];
    
    int found = 0;
    char c;
    char *p;
    char *q;
    int len;
    

    if(!(inf=fopen(filename,"r")))
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }

    /* Look for start of prototypes */
    found = 0;
    while(fgets(line,MAXNAMLEN,inf))
	if(strstr(line,"Prototype definitions"))
	{
	    found = 1;
	    break;
	}

    if(!found)
    {
	fclose(inf);
	return;
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
	if(strstr(line,"End of prototype"))
	    break;
	
	c = *line;
	if(c == ' ' || c=='\n' || c=='\t' || c=='*' || c=='#')
	    continue;
	if(!strncmp(line,"/*",2))
	    continue;

	/*
	** Expects start of a function definition here so looks for
	** an opening parenthesis. Exits if not found
        */
	p = line;
	while(*p && *p!='(')
	    ++p;
	if(!*p)
	{
	    fprintf(stderr,"Cannot find '(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t')
	    --p;
	++p;
	while(*p=='*')
	    ++p;
	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"%s\n",tline);
    }

    fclose(inf);

    return;
}




static void write_coreexports(char *fn,char *basedir)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/ajax/core/%s",basedir,COREDEF);
    if(!(outf=fopen(filename,"w")))
    {
	fprintf(stderr,"Cannot open AJAX definitions file %s\n",filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""ajax.dll""\n\n");
    fprintf(outf,"EXPORTS\n");

    fprintf(outf,"\tAssert_Failed\n");
    fprintf(outf,";dirent_w32\n");
    fprintf(outf,"\tclosedir\n\topendir\n\treaddir\n;others\n");

    if(!(inf=fopen(TMPFILE2,"r")))
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static void write_genajaxexport(char *fn,char *basedir,char *exportdef,
                                char *subdir,char *dllname)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/ajax/%s/%s",basedir,subdir,exportdef);
    if(!(outf=fopen(filename,"w")))
    {
	fprintf(stderr,"Cannot open %s definitions file %s\n",subdir,filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""%s.dll""\n\n",dllname);
    fprintf(outf,"EXPORTS\n");

    if(!(inf=fopen(TMPFILE2,"r")))
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static void write_nucleusexports(char *fn,char *basedir)
{
    FILE *outf;
    FILE *inf;
    
    char command[MAXNAMLEN];
    char line[MAXNAMLEN];
    char filename[MAXNAMLEN];
    
    sprintf(command,"sort %s > %s",fn, TMPFILE2);
    system(command);
    unlink(fn);

    sprintf(filename,"%s/win32/DLLs/nucleus/%s",basedir,NUCLEUSDEF);
    if(!(outf=fopen(filename,"w")))
    {
	fprintf(stderr,"Cannot open NUCLEUS definitions file %s\n",filename);
	exit(-1);
    }

    fprintf(outf,"LIBRARY \t""nucleus.dll""\n\n");
    fprintf(outf,"EXPORTS\n");

    /*
    ** EmbPropTable no longer exists but leave this as an example
    ** of how to add rogue data definitions
    **    fprintf(outf,"\tEmbPropTable DATA\n;others\n");
    */

    if(!(inf=fopen(TMPFILE2,"r")))
    {
	fprintf(stderr,"Cannot open sorted function file %s\n",TMPFILE2);
	exit(-1);
    }
    
    while(fgets(line,MAXNAMLEN,inf))
	fprintf(outf,"\t%s",line);
	
    fclose(inf);
    fclose(outf);

    unlink(TMPFILE2);

    return;
}




static int dir_exists(char *path)
{
    struct stat buf;

    if(stat(path,&buf) == -1)
	return 0;

    if((int)buf.st_mode & S_IFDIR)
	return 1;
    else
    {
	fprintf(stderr,"%s exists but isn't a directory\n",path);
	exit(-1);
    }
    
    return 0;
}




static int file_exists(char *path)
{
    struct stat buf;

    if(stat(path,&buf) == -1)
	return 0;

    if((int)buf.st_mode & S_IFREG)
	return 1;
    else
    {
	fprintf(stderr,"%s exists but isn't a regular file\n",path);
	exit(-1);
    }

    return 0;
}




static int copy_apps(char *basedir, listnode *head)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char dir[MAXNAMLEN];
    int ignore;
    listnode *listptr;
    char command[MAXNAMLEN];
    char acdname[MAXNAMLEN];
    char filename[MAXNAMLEN];
    struct stat buf;
    int count;
    int i;
    int excluded = 0;
    
    fprintf(stdout,"Copying applications\n");
    
    sprintf(dir,"%s/emboss/emboss",basedir);
    

    if(!(indir = opendir(dir)))
    {
	fprintf(stderr,"Can't locate directory %s\n",dir);
	exit(-1);
    }
    
    count = 0;

    while((dp=readdir(indir)))
    {
#ifndef CYGWIN
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif
	len = strlen(dp->d_name);
	if(len > 2)
	{
	    sprintf(filename,"%s/%s",dir,dp->d_name);
	    stat(filename,&buf);
	    if((int)buf.st_mode & S_IFDIR)
		continue;

	    len = strlen(dp->d_name);
	    if(strcmp(dp->d_name + len -2,".c"))
		continue;

            i = 0;
            excluded = 0;
            while(exclude_names[i])
            {
                if(!strcmp(dp->d_name,exclude_names[i]))
                {
                    excluded = 1;
                    break;
                }

                ++i;
            }

            if(excluded)
                continue;

	    ignore = 0;
	    listptr = head;
	    while(listptr->next)
	    {
		if(!strcmp(listptr->prog,dp->d_name))
		{
		    ignore = 1;
		    break;
		}
		listptr = listptr->next;
	    }
	    

	    if(ignore)
		continue;

	    ++count;	    
	    
	    sprintf(command,"cp %s/%s %s/win32/emboss",dir,dp->d_name,
		    basedir);
	    if(system(command))
	    {
		fprintf(stderr,"Can't execute %s\n",command);
		exit(-1);
	    }
	    
	    strcpy(acdname,dp->d_name);
	    len = strlen(acdname);
	    acdname[len-2] = '\0';
	    strcat(acdname,".acd");

	    sprintf(filename,"%s/acd/%s",dir,acdname);
	    if(stat(filename,&buf) == -1)
		continue;

	    sprintf(command,"cp %s/acd/%s %s/win32/acd",dir,acdname,
		    basedir);

	    if(system(command))
	    {
		fprintf(stderr,"Can't execute %s\n",command);
		exit(-1);
	    }
	}
    }
    

    closedir(indir);



    sprintf(command,"cp %s/acd/codes.english %s/win32/acd",dir,basedir);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/acd/*.standard %s/win32/acd",dir,basedir);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }

    sprintf(command,"cp %s/emboss/win32/exes/*.exe %s/win32/apps/release",
	    basedir,basedir);
    
    if(system(command))
    {
	fprintf(stderr,"Can't execute %s\n",command);
	exit(-1);
    }
    


    return count;
}




static void read_make_check(char *basedir, listnode **head)
{
    char filename[MAXNAMLEN];
    FILE *fp;
    char line[MAXNAMLEN];
    char tmpstr[MAXNAMLEN];
    char progname[MAXNAMLEN];
    
    int found;
    int len;
    char *p;
    
    sprintf(filename,"%s/emboss/emboss/Makefile.am",basedir);
    
    if(!(fp=fopen(filename,"r")))
    {
	fprintf(stderr,"Cannot open %s\n",filename);
	exit(-1);
    }

    found = 0;
    
    while(fgets(line,MAXNAMLEN,fp))
	if(strstr(line,"check_PROGRAMS ="))
	{
	    found = 1;
	    break;
	}

    if(!found)
    {
	fprintf(stderr,"Cannot find ""check_PROGRAMS ="" line\n");
	exit(-1);
    }
    
    len = strlen(line);
    if(line[len-1] = '\n')
	line[len-1] = '\0';

    while(*line)
    {
	p = line;
	if(strstr(line,"check_PROGRAMS"))
	{
	    while(*p != '=')
		++p;
	    ++p;
	    while(*p == ' ' || *p == '\t')
		++p;
	}
	else
	{
	    while(*p && (*p == ' ' || *p == '\t'))
		++p;
	}
	strcpy(tmpstr,p);	

	p = strtok(tmpstr," \\\t\n");
	if(p)
	{
	    sprintf(progname,"%s.c",p);
	    add_node(head,progname);
	}
	
	while((p=strtok(NULL," \\\t\n")))
	{
	    sprintf(progname,"%s.c",p);
	    add_node(head,progname);
	}

	if(!fgets(line,MAXNAMLEN,fp))
	{
	    fprintf(stderr,"Unexpected EOF in file %s\n",filename);
	    exit(-1);
	}

	p = line;
	len = strlen(line);
	if(line[len-1]=='\n')
	    line[len-1] = '\0';
	while(*p && (*p == ' ' || *p == '\t'))
	    ++p;
	if(!*p)
	    *line = '\0';
    }

    fclose(fp);

    return;
}




static void add_node(listnode **head, char *progname)
{
    listnode *tmp;

    if(!(tmp = malloc(sizeof(*tmp))))
    {
	fprintf(stderr,"Malloc failure creating list node\n");
	exit(-1);
    }

    strcpy(tmp->prog,progname);
    tmp->next = *head;
    *head = tmp;

    return;
}




static void make_uids(char ***uids, int napps)
{
    static char *uidext="-fc97-11d7-88ec-005056c00008";
    unsigned int uid;
    int i;
    char **uida;
    
    uid = 0xf34f5640;

    *uids = (char **) malloc(sizeof(char **) * napps);
    if(!*uids)
    {
	fprintf(stderr,"Error mallocing uid array\n");
	exit(-1);
    }

    uida = *uids;
    
    for(i=0;i<napps;++i)
    {
	uida[i] = (char *) malloc(MAXNAMLEN);
	if(!uida[i])
	{
	    fprintf(stderr,"Error mallocing uid array element\n");
	    exit(-1);
	}

	sprintf(uida[i],"%x%s",uid,uidext);
	++uid;
    }

    return;
}




static void read_prognames(char *basedir, int napps, char ***names)
{
    struct dirent *dp;
    DIR *indir;
    int len;
    char filename[MAXNAMLEN];
    char dir[MAXNAMLEN];
    char **pnames = NULL;
    int i;
    int j;
    int excluded = 0;
    
    fprintf(stdout,"Reading program names\n");

    *names = (char **) malloc(sizeof(char **) * napps);
    if(!*names)
    {
	fprintf(stderr,"Error mallocing uid array\n");
	exit(-1);
    }

    pnames = *names;
    
    for(i=0;i<napps;++i)
    {
	pnames[i] = (char *) malloc(MAXNAMLEN);
	if(!pnames[i])
	{
	    fprintf(stderr,"Error mallocing progname array element\n");
	    exit(-1);
	}
    }


    sprintf(dir,"%s/win32/emboss",basedir);

    if(!(indir = opendir(dir)))
    {
	fprintf(stderr,"Error opening directory %s\n",dir);
	exit(-1);
    }

    i = 0;
    
    while((dp=readdir(indir)))
    {
#ifndef CYGWIN
	if(!dp->d_ino || !strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#else
	if(!strcmp(dp->d_name,".")  || !strcmp(dp->d_name,".."))
	    continue;
#endif

        j = 0;
        excluded = 0;
        while(exclude_names[j])
        {
            if(!strcmp(dp->d_name,exclude_names[j]))
            {
                printf("Found %s\n",dp->d_name);
                
                excluded = 1;
                break;
            }

            ++j;
        }
        

        len = strlen(dp->d_name);
	if(len > 2 && !strcmp(dp->d_name + len - 2, ".c") && !excluded)
	{
	    sprintf(filename,"%s",dp->d_name);
	    filename[len-2] = '\0';
	    strcpy(pnames[i],filename);
	    ++i;
	}
	
    }

    closedir(indir);


    return;
}




static void write_solution(char *basedir, char **prognames, char **uids,
			   int napps)
{
    char filename[MAXNAMLEN];
    FILE *fp;
    int i;
    int j;
    char *p;
    

    for(i=0;i<napps;++i)
    {
	p = uids[i];
	while(*p)
	{
	    *p = (char) toupper((int)*p);
	    ++p;
	}
    }
    

    sprintf(filename,"%s/win32/apps/apps.sln",basedir);
    
    if(!(fp=fopen(filename,"w")))
    {
	fprintf(stderr,"Cannot open %s\n",filename);
	exit(-1);
    }


    fprintf(fp,"Microsoft Visual Studio Solution File, Format Version 10.00\n");
    fprintf(fp,"# Visual C++ Express 2008\n");


    for(i=0;i<napps;++i)
    {
	fprintf(fp,"%s = \"%s\", \"emboss\\%s\\%s.vcproj\", \"{%s}\"\n",
		PROJECT,prognames[i],prognames[i],prognames[i],uids[i]);
	fprintf(fp,"EndProject\n");
    }

    fprintf(fp,"Global\n");
    fprintf(fp,"\tGlobalSection(SolutionConfigurationPlatforms"
	    " = preSolution\n");
    fprintf(fp,"\t\tDebug|Win32 = Debug|Win32\n");
    fprintf(fp,"\t\tRelease|Win32 = Release|Win32\n");
    fprintf(fp,"\tEndGlobalSection\n");
    fprintf(fp,"\tGlobalSection(ProjectConfigurationPlatforms)"
	    " = postSolution\n");

    for(i=0;i<napps;++i)
    {
	fprintf(fp,"\t\t{%s}.Debug|Win32.ActiveCfg = Debug|Win32\n",
		uids[i]);
	fprintf(fp,"\t\t{%s}.Debug|Win32.Build.0 = Debug|Win32\n",
		uids[i]);
	fprintf(fp,"\t\t{%s}.Release|Win32.ActiveCfg = Release|Win32\n",
		uids[i]);	
	fprintf(fp,"\t\t{%s}.Release|Win32.Build.0 = Release|Win32\n",
		uids[i]);
    }

    fprintf(fp,"\tEndGlobalSection\n");
    fprintf(fp,"\tGlobalSection(SolutionProperties) = preSolution\n");
    fprintf(fp,"\t\tHideSolutionNode = FALSE\n");
    fprintf(fp,"\tEndGlobalSection\n");


    fprintf(fp,"\tGlobalSection(SolutionConfigurationPlatforms ) "
            "= preSolution\n");
    fprintf(fp,"\t\tDebug|Win32 = Debug|Win32\n");
    fprintf(fp,"\t\tRelease|Win32 = Release|Win32\n");
    fprintf(fp,"\tEndGlobalSection\n");
    

    fprintf(fp,"EndGlobal\n");
    
    fclose(fp);

    chmod(filename,0755);

    return;
}




static void write_projects(char *basedir, char **prognames, char **uids,
			   int napps)
{
    char filename[MAXNAMLEN];
    int i;
    int j;
    FILE *inf;
    FILE *outf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    char *p;
    int found;
    
    for(i=0;i<napps;++i)
    {
	sprintf(filename,"%s/win32/apps/emboss/%s",basedir,prognames[i]);
	if(mkdir(filename,0755) == -1)
	{
	    fprintf(stderr,"Cannot create project directory %s\n",filename);
	    exit(-1);
	}
    }

    for(i=0;i<napps;++i)
    {
	sprintf(filename,"%s/win32/apps/emboss/%s/%s.vcproj",basedir,
		prognames[i],prognames[i]);
	if(!(outf=fopen(filename,"w")))
	{
	    fprintf(stderr,"Cannot open %s\n",filename);
	    exit(-1);
	}

	sprintf(filename,"%s/emboss/win32/apps/template.vcproj",basedir);
	if(!(inf=fopen(filename,"r")))
	{
	    fprintf(stderr,"Cannot open %s\n",filename);
	    exit(-1);
	}

	while(fgets(line,MAXNAMLEN,inf))
	{
	    if((p=strstr(line,"<PROGNAME>")))
	    {
		sub_text(line,tline,"<PROGNAME>",prognames[i]);
		fprintf(outf,"%s",tline);
		continue;
	    }

	    if((p=strstr(line,"<UID>")))
	    {
		sub_text(line,tline,"<UID>",uids[i]);
		fprintf(outf,"%s",tline);
		continue;
	    }


	    if((p=strstr(line,"<STACKSIZE>")))
	    {
		j = 0;
		found = 0;
		while(memory[j].progname)
		{
		    if(!strcmp(memory[j].progname,prognames[i]))
		    {
			found = 1;
			break;
		    }

		    ++j;
		}
		

		if(found)
		    fprintf(outf,"\t\t\t\tStackReserveSize=\"%s\"\n",
			    memory[j].stacksize);

		continue;
	    }

	    if((p=strstr(line,"<HEAPSIZE>")))
	    {
		j = 0;
		found = 0;
		while(memory[j].progname)
		{
		    if(!strcmp(memory[j].progname,prognames[i]))
		    {
			found = 1;
			break;
		    }

		    ++j;
		}
		

		if(found)
		    fprintf(outf,"\t\t\t\tHeapReserveSize=\"%s\"\n",
			    memory[j].heapsize);

		continue;
	    }

	    fprintf(outf,"%s",line);
	}
	
	fclose(inf);
	fclose(outf);

	sprintf(filename,"%s/win32/apps/emboss/%s/%s.vcproj",basedir,
		prognames[i],prognames[i]);
	chmod(filename,0755);
    }
    

    return;
}




static void sub_text(char *line, char *tline, char *given, char *rep)
{
    char *p;

    p = strstr(line,given);
    strncpy(tline,line,p-line);
    tline[p-line] = '\0';
    strcat(tline,rep);

    p += strlen(given);
    strcat(tline,p);

    return;
}



static void make_ajax_header_exports(char *basedir,char *subdir)
{
    char defsdir[MAXNAMLEN];
    FILE *fp = NULL;
    

    sprintf(defsdir,"%s/emboss/ajax/%s",basedir,subdir);
    
    if(!(fp=fopen(TMPFILE,"w")))
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }
    

    if(header_exports(defsdir,fp) < 0)
    {
	fprintf(stderr,"Cannot open directory %s/emboss/ajax/%s",basedir,
                subdir);
	exit(-1);
    }

    fclose(fp);

    return;
}




static void make_expat_header_exports(char *basedir)
{
    char hfname[MAXNAMLEN];
    FILE *fp = NULL;
    

    sprintf(hfname,"%s/emboss/ajax/expat/expat.h",basedir);


    if(!(fp=fopen(TMPFILE,"w")))
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }

    extract_expat_funcnames(hfname,fp);

    fclose(fp);
    
    return;
}




static void extract_expat_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    char command[MAXNAMLEN];
    
    int found = 0;
    char *p;
    char *q;
    int len;
    

    if(!(inf=fopen(filename,"r")))
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
	if(!strstr(line,"XMLPARSEAPI"))
	    continue;

        if(!fgets(line,MAXNAMLEN,inf))
        {
            fprintf(stderr,"Error in expat.h: expected line after "
                    "XMLPARSEAPI\n");
            exit(1);
        }

	/*
	** Expects start of a function definition here so looks for
	** an opening parenthesis. Exits if not found
        */
	p = line;
	while(*p && *p!='(')
	    ++p;
	if(!*p)
	{
	    fprintf(stderr,"Cannot find '(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t' && p != line)
	    --p;

        if(p != line)
            ++p;

	while(*p=='*')
	    ++p;

	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"e%s\n",tline);
    }

    fclose(inf);

    return;
}




static void make_zlib_header_exports(char *basedir)
{
    char hfname[MAXNAMLEN];
    FILE *fp = NULL;
    

    sprintf(hfname,"%s/emboss/ajax/zlib/zlib.h",basedir);


    if(!(fp=fopen(TMPFILE,"w")))
    {
	fprintf(stderr,"Cannot open temporary file %s\n",TMPFILE);
	exit(-1);
    }

    extract_zlib_funcnames(hfname,fp);

    fclose(fp);
    
    return;
}




static void extract_zlib_funcnames(char *filename, FILE *fout)
{
    FILE *inf;
    char line[MAXNAMLEN];
    char tline[MAXNAMLEN];
    char command[MAXNAMLEN];
    
    int found = 0;
    char *p;
    char *q;
    int len;
    

    if(!(inf=fopen(filename,"r")))
    {
	fprintf(stderr,"Cannot open header file %s\n",filename);
	exit(-1);
    }


    while(fgets(line,MAXNAMLEN,inf))
    {
        if(!strncmp(line,"/*",2))
        {
            while(strncmp(line,"*/",2))
                if(!fgets(line,MAXNAMLEN,inf))
                {
                    fprintf(stderr,"extract_zlib_funcnames: Can't find /*\n");
                    exit(1);
                }
            
            continue;
        }

        if(!strstr(line,"ZEXTERN"))
	    continue;

	/*
	** Expects start of a function definition here so looks for
	** "OF(". Exits if not found
        */
        p = strstr(line,"OF(");
	if(!p)
	{
	    fprintf(stderr,"Cannot find 'OF(' in line:\n%s",line);
	    exit(-1);
	}

	/* Find last character of function name */
	--p;
	while(*p == ' ' || *p == '\t')
	    --p;
	
	q = p;

	/* Find start character of function name */
	while(*p != ' ' && *p != '\t' && p != line)
	    --p;

        ++p;

	while(*p=='*')
	    ++p;

	len = q - p + 1;

	strncpy(tline,p,len);
	tline[len] = '\0';

	fprintf(fout,"e%s\n",tline);
    }

    fclose(inf);

    return;
}
