/* @source winchkexes application
**
** Compare exes in apps directory with those listed in the installer ISS file
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
** This is a UNIX or Borland (bcc32) application, only rigorously tested under
** Linux (purely as it's more convenient for the core developers).
** Its purpose
** is to take a CVS checkout of EMBOSS from the developers' site
** and construct a directory structure suitable for compiling under
** Microsoft Visual C++ Express.
**
** Compile using: cc -o winchkexes -O2 winchkexes.c   (UNIX)
**
** Usage: winchkexes embossappdir iss-file
**
** This program scans the emboss applications directory (UNIX) for
** files with the 'other execute' mode bit set. It then compares
** the filenames of those assumed executables with the application
** names given in the ISS file for Microsoft setup.exe creation.
** N.B.
** The Source: lines for applications in the ISS file must be
** enclosed by the following comment lines.
** ; Applications
** ; End of Applications
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>




static int find_exes(const char *dirname, char ***aptr);
static int find_issexes(const char *issname, char ***aptr);




static int find_issexes(const char *issname, char ***aptr)
{
    char **aarray = NULL;
    FILE *inf = NULL;
    char line[MAXPATHLEN];
    char *ret;
    int ns = 0;
    int i = 0;
    char *pend   = NULL;
    char *pstart = NULL;
    int len = 0;
    
    inf = fopen(issname,"rb");

    if(!inf)
    {
        fprintf(stderr,"Error: Cannot open ISS file [%s]\n",issname);
        exit(-1);
    }
    
    while((ret = fgets(line,MAXPATHLEN,inf)))
    {
        if(!strncmp(line,"; Applications",14))
            break;
    }
    
    if(!ret)
    {
        fprintf(stderr,"Missing \"; Applications\" marker\n");
        exit(-1);
    }
    
    while((ret = fgets(line,MAXPATHLEN,inf)))
    {
        if(!strncmp(line,"; End of Applications",21))
            break;

        if(!strncmp(line,"Source:",7))
            ++ns;
    }
    
    if(!ret)
    {
        fprintf(stderr,"Missing \"; End of Applications\" marker\n");
        exit(-1);
    }
    
    if(ns)
    {
        *aptr = malloc(sizeof(char *) * (ns + 1));
        aarray = *aptr;

        for(i = 0; i < ns; ++i)
            aarray[i] = malloc(sizeof(char) * MAXPATHLEN);

        aarray[ns] = NULL;
    }
    
    fclose(inf);
    inf = fopen(issname,"rb");

    while((ret = fgets(line,MAXPATHLEN,inf)))
    {
        if(!strncmp(line,"; Applications",14))
            break;
    }

    i = 0;
    
    while((ret = fgets(line,MAXPATHLEN,inf)))
    {
        if(!strncmp(line,"; End of Applications",21))
            break;

        if(!strncmp(line,"Source:",7))
        {
            pend = strstr(line,".exe");
            pstart = pend - 1;

            while(isalpha(*pstart) || isdigit(*pstart))
                --pstart;

            ++pstart;

            len = pend - pstart;
            strncpy(aarray[i],pstart,len);
            aarray[i++][len] = '\0';
        }
    }

    fclose(inf);

    return ns;
}




static int find_exes(const char *dirname, char ***aptr)
{
    char **earray = NULL;
    DIR *dirp = NULL;
    struct dirent *dresult = NULL;
    char fname[MAXPATHLEN];
    char buf[sizeof(struct dirent) + MAXPATHLEN];
    struct stat sbuf;
    int count = 0;
    int i;
    
    dirp = opendir(dirname);

    if(!dirp)
    {
        fprintf(stderr,"Error: Cannot open directory [%s]\n",dirname);
        exit(-1);
    }

    count = 0;
    
    while(!readdir_r(dirp,(struct dirent *)buf, &dresult))
    {
        if(!dresult)
            break;
        
        if(dresult->d_type == DT_REG)
        {
            sprintf(fname,"%s/%s",dirname,dresult->d_name);

            if(!stat(fname,&sbuf))
            {
                if(sbuf.st_mode & S_IXOTH)
                    ++count;
            }
        }
    }

    rewinddir(dirp);


    *aptr  = malloc((sizeof(char *) * (count + 1)));
    earray = *aptr;
    
    for(i = 0; i < count; ++i)
        earray[i] = malloc(sizeof(char) * MAXPATHLEN);

    earray[count] = NULL;
    
    i = 0;
    
    while(!readdir_r(dirp,(struct dirent *)buf, &dresult))
    {
        if(!dresult)
            break;
        
        if(dresult->d_type == DT_REG)
        {
            sprintf(fname,"%s/%s",dirname,dresult->d_name);

            if(!stat(fname,&sbuf))
            {
                if(sbuf.st_mode & S_IXOTH)
                    strcpy(earray[i++],dresult->d_name);
            }
        }
    }

    
    closedir(dirp);
    
    return count;
}




int main(int argc, char **argv)
{
    char **exe_files = NULL;
    char **iss_exes  = NULL;
    
    int nexes = 0;
    int niss = 0;
    
    int i = 0;
    int j = 0;
    
    if(argc < 3)
    {
        fprintf(stderr,"Usage: winchkexes embossappdir iss-file\n");
        exit(-1);
    }


    nexes = find_exes(argv[1],&exe_files);
    niss  = find_issexes(argv[2],&iss_exes);
    

    for(i = 0; i < nexes; ++i)
    {
        for(j = 0; j < niss; ++j)
        {
            if(!strcmp(exe_files[i],iss_exes[j]))
                break;
        }
    
        if(j == niss)
            fprintf(stdout,"%s\n",exe_files[i]);
    }


    if(nexes)
    {
        i = 0;

        while(exe_files[i])
            free(exe_files[i++]);

        free(exe_files);
    }

    if(niss)
    {
        i = 0;

        while(iss_exes[i])
            free(iss_exes[i++]);

        free(iss_exes);
    }

    
    return 0;
}
