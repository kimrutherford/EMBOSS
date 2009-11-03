/*
** Utility to create Makefile.am files given the directory containing
** the files plus the extra path information for installation.
**
** Can be used for the EMBOSS  acd, doc/programs/html and
** doc/programs/text directories.
**
** Usage:  makeam directory installdatapath
**
** e.g.
**
** makeam /fubar/emboss/emboss/emboss/acd acd > Makefile.am
** makeam /fubar/emboss/embosss/doc/programs/html doc/programs/html > 
**         Makefile.am
** makeam /fubar/emboss/embosss/doc/programs/text doc/programs/text > 
**         Makefile.am
**
** The program ignores:
**   a) Directories
**   b) Any file with the prefix "Makefile"
**   c) Any file with the suffix '~'
** All other files are incorporated
**
** Author: (C) Alan Bleasby.
** Date: 20th February 2005
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
*/

#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MAXLINES 20
#define LINELEN  76

#define PKGTYPE 0
#define TABTYPE 1

static void get_dir_count(char *path, int *count, int *filelen);
static void get_names(char ***names, char *path, int count, int filelen);
static void write_packagedata(char **names, int *npackages);


int main(int argc, char **argv)
{
    int nfiles   = 0;
    int filelen  = 0;
    char **names = NULL;
    int i;
    int npackages = 0;
    

    if(argc < 3)
    {
	fprintf(stderr,"Usage: makeam directory installdatapath\n");
	exit(-1);
    }
    
    get_dir_count(argv[1],&nfiles,&filelen);
    get_names(&names,argv[1],nfiles,filelen);
    write_packagedata(names,&npackages);

    fprintf(stdout,"\n");
    for(i=0;i<npackages;++i)
    {
	if(!i)
	    fprintf(stdout,"pkgdatadir=$(prefix)/share/$(PACKAGE)/%s\n",
		    argv[2]);
	else
	    fprintf(stdout,"pkgdata%ddir=$(prefix)/share/$(PACKAGE)/%s\n",
		    i+1,argv[2]);
    }
    
    i = 0;
    while(names[i])
	free((void *)names[i++]);
    free((void *)names);
    

    return 0;
}






static void get_dir_count(char *path, int *count, int *filelen)
{
    struct dirent *dp;
    DIR *dir;
    char dirpath[MAXPATHLEN];
    char fullname[MAXPATHLEN];
    int len;
    struct stat buf;

    strcpy(dirpath,path);
    len = strlen(dirpath);
    if(dirpath[len-1] != '/')
	strcat(dirpath,"/");
    

    if(!(dir = opendir(dirpath)))
    {
	fprintf(stderr,"Error: Cannot open directory [%s]\n",path);
	exit(-1);
    }
    
    while((dp=readdir(dir)))
    {
	if(!dp->d_ino || *dp->d_name == '.' ||
	   !strncmp(dp->d_name,"Makefile",8))
	    continue;

	sprintf(fullname,"%s%s",dirpath,dp->d_name);
	if(!stat(fullname,&buf))
	    if(buf.st_mode & S_IFDIR)
		continue;

	len = strlen(dp->d_name);
	if(dp->d_name[len-1] == '~')
	    continue;
	
	++(*count);
	*filelen = (len > *filelen) ? len : *filelen;
    }

    closedir(dir);

    return;
}




static void get_names(char ***names, char *path, int count, int filelen)
{
    struct dirent *dp;
    DIR *dir;
    char dirpath[MAXPATHLEN];
    char fullname[MAXPATHLEN];
    int len;
    struct stat buf;
    int i;
    
    if(!((*names) = (char **) malloc((count+1) * sizeof(char *))))
    {
	fprintf(stderr,"Error: Memory allocation for array\n");
	exit(-1);
    }

    (*names)[count] = NULL;

    for(i=0;i<count;++i)
	if(!((*names)[i] = (char *) malloc(filelen+1)))
	{
	    fprintf(stderr,"Error: Memory allocation for char *\n");
	    exit(-1);
	}


    

    strcpy(dirpath,path);
    len = strlen(dirpath);
    if(dirpath[len-1] != '/')
	strcat(dirpath,"/");
    

    if(!(dir = opendir(dirpath)))
    {
	fprintf(stderr,"Error: Cannot open directory [%s]\n",path);
	exit(-1);
    }

    i = 0;
    while((dp=readdir(dir)))
    {
	if(!dp->d_ino || *dp->d_name == '.' ||
	   !strncmp(dp->d_name,"Makefile",8))
	    continue;
	
	sprintf(fullname,"%s%s",dirpath,dp->d_name);
	if(!stat(fullname,&buf))
	    if(buf.st_mode & S_IFDIR)
		continue;

	len = strlen(dp->d_name);
	if(dp->d_name[len-1] == '~')
	    continue;


	strcpy((*names)[i++],dp->d_name);
    }

    closedir(dir);

    return;
}




static void write_packagedata(char **names, int *npackages)
{
    int start = 1;
    int pos = 0;
    char line[MAXPATHLEN];
    int nlines = 0;
    int linetype = 0;
    int maxlinelength = 0;
    int len = 0;
    int do_write = 0;
    
    while(names[pos])
    {
	if(start)
	{
	    fprintf(stdout,"\n");
	    ++(*npackages);
	    if(*npackages == 1)
		strcpy(line,"pkgdata_DATA =");
	    else
		sprintf(line,"pkgdata%d_DATA =",*npackages);
	    linetype = PKGTYPE;
	    start = 0;
	    do_write = 1;
	}
	

	if(linetype == PKGTYPE)
	    maxlinelength = LINELEN;
	else
	    maxlinelength = LINELEN - 8;
	

	len = strlen(line);
	if(len + strlen(names[pos]) < maxlinelength)
	{
	    strcat(line," ");
	    strcat(line,names[pos++]);
	}
	else
	{
	    if(!names[pos+1])
	    {
		strcat(line," \\");		
		fprintf(stdout,"%s\n",line);
		fprintf(stdout,"\t%s\n",names[pos++]);
		do_write = 0;
		continue;
	    }


	    if(nlines == MAXLINES)
	    {
		fprintf(stdout,"%s\n",line);
		start = 1;
		nlines = 0;
		continue;
	    }
	    
	    strcat(line," \\");
	    linetype = TABTYPE;
	    fprintf(stdout,"%s\n",line);
	    sprintf(line,"\t%s",names[pos++]);
	    ++nlines;
	}
    }

    if(do_write)
	fprintf(stdout,"%s\n",line);
    

    return;
}
