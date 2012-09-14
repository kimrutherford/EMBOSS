/* @source compembassy application
**
** Test whether EMBASSY bundles compile and install
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
** This is a UNIX application
**
** Compile using: cc -o compembassy -O2 compembassy.c
**
** Usage: compembassy bundledir prefixdir
**
** This program expects you to have compiled and installed EMBOSS
** into a given prefix. It accepts a directory containing just
** the bundled EMBASSY 'gz' files. It extracts each in turn
** and configures, makes and installs to the given prefix.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


static int find_gz(const char *dirname, char ***aptr);
static int find_dirname(const char *dirname, char ***aptr);




static int find_gz(const char *dirname, char ***aptr)
{
    char **earray = NULL;
    DIR *dirp = NULL;
    struct dirent *dresult = NULL;
    char fname[MAXPATHLEN];
    char buf[sizeof(struct dirent) + MAXPATHLEN];
    struct stat sbuf;
    int count = 0;
    int i;
    int len;
    
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
            len = strlen(dresult->d_name);

            if(strcmp(&dresult->d_name[len-3],".gz"))
                continue;
            
            ++count;
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
            len = strlen(dresult->d_name);
            if(strcmp(&dresult->d_name[len-3],".gz"))
                continue;

            strcpy(earray[i++],dresult->d_name);
        }
    }

    
    closedir(dirp);
    
    return count;
}




static int find_dirname(const char *dirname, char ***aptr)
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
            if(*dresult->d_name == '.')
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
            if(*dresult->d_name == '.')
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
    char **gzfiles = NULL;
    char bundledir[MAXPATHLEN];
    char tarfile[MAXPATHLEN];
    char command[MAXPATHLEN];
    char **dnames;
    
    int nemb = 0;
    int ndirs = 0;
    
    
    int i = 0;
    int j = 0;
    
    int len = 0;
    
    if(argc < 3)
    {
        fprintf(stderr,"Usage: compembassy bundledir prefixdir\n");
        exit(-1);
    }

    sprintf(bundledir,"%s",argv[1]);
    nemb = find_gz(bundledir,&gzfiles);
    
    for(i = 0; i < nemb; ++i)
    {
        strcpy(tarfile,gzfiles[i]);
        len = strlen(tarfile);
        tarfile[len-3] = '\0';
        
        sprintf(command,"cd %s; gunzip %s; tar xf %s",bundledir,gzfiles[i],
                tarfile);

        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }

        ndirs = find_dirname(bundledir,&dnames);
        if(ndirs != 1)
        {
            fprintf(stderr,"Fatal: Too many directories in %s\n",bundledir);
            exit(-1);
        }


        sprintf(command,"cd %s/%s; ./configure --prefix=%s; "
                "make; make install",bundledir,dnames[0],argv[2]);

        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }
        
        sprintf(command,"cd %s; rm -rf %s; gzip %s",
                bundledir,dnames[0],tarfile);

        if(system(command))
        {
            fprintf(stderr,"Can't execute %s\n",command);
            exit(-1);
        }
        

        if(ndirs)
        {
            j = 0;
            
            while(dnames[j])
                free(dnames[j++]);
            
            free(dnames);
        }
    }


    if(nemb)
    {
        i = 0;

        while(gzfiles[i])
            free(gzfiles[i++]);

        free(gzfiles);
    }

    return 0;
}
