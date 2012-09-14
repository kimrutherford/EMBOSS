/* @source mkembassy application
**
** Bundles EMBASSY files for distribution
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
** This is a UNIX application.
**
** Compile using: cc -o mkembassy -O2 mkembassy.c
**
** Usage: mkembassy embosstopdir outputdir
**
** This program expects a freshly checked-out EMBOSS CVS tree.
** It's parameters are:
** a) The EMBOSS topdir (the one containing ajax, nucleus, etc)
** b) An existing, preferably empty directory to contain the
**    resulting bundles.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

char *extra_dist[] =  {
    "depcomp",
    "ltmain.sh",
    "install-sh",
    "config.sub",
    "config.guess",
    NULL
};



static int find_embassy(const char *dirname, char ***aptr);


static int find_embassy(const char *dirname, char ***aptr)
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
        
        if(dresult->d_type == DT_DIR)
        {
            if(!strcmp(dresult->d_name,"CVS") || *dresult->d_name == '.')
                continue;
            
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
        
        if(dresult->d_type == DT_DIR)
        {
            if(!strcmp(dresult->d_name,"CVS") || *dresult->d_name == '.')
                continue;

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
    char **emb_dirs = NULL;
    char embassydir[MAXPATHLEN];
    char command[MAXPATHLEN];
    
    int nemb = 0;

    
    int i = 0;
    int j = 0;
    
    if(argc < 3)
    {
        fprintf(stderr,"Usage: mkembassy embosstopdir outputdir\n");
        exit(-1);
    }

    sprintf(embassydir,"%s/embassy",argv[1]);
    nemb = find_embassy(embassydir,&emb_dirs);
    
    sprintf(command,"cd %s; autoreconf -fi; ./configure",argv[1]);
    if(system(command))
    {
        fprintf(stderr,"Can't execute %s\n",command);
        exit(-1);
    }
    
    for(i = 0; i < nemb; ++i)
    {
        j = 0;
        while(extra_dist[j])
        {
            sprintf(command,"cd %s; cp -f %s %s/%s", argv[1],extra_dist[j],
                    embassydir, emb_dirs[i]);
            if(system(command))
            {
                fprintf(stderr,"Can't execute %s\n",command);
                exit(-1);
            }
            
            ++j;
        }

        sprintf(command,"find %s/%s -name CVS | xargs rm -rf",embassydir,
                emb_dirs[i]);
        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }

        sprintf(command,"find %s/%s -name .cvsignore | xargs rm -rf",
                embassydir, emb_dirs[i]);
        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }

        sprintf(command,"cd %s/%s; autoreconf -fi; ./configure; make  dist;"
                "cp -f *gz %s",embassydir,
                emb_dirs[i],argv[2]);
        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }
    }


    if(nemb)
    {
        i = 0;

        while(emb_dirs[i])
            free(emb_dirs[i++]);

        free(emb_dirs);
    }

    return 0;
}
