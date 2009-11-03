/******************************************************************************
** @source AJAX Jave Native Interface (JNI) functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version 1.0
** @version 2.0 Added Jemboss suid authorisation functions
** @modified Jul 07 2001 ajb First version
** @modified Mar 02 2002 ajb First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#ifdef HAVE_JAVA

#include "ajax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <strings.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <ctype.h>


#ifdef HAVE_JAVA
#include <jni.h>

#ifdef __hpux
#include <signal.h>
#include <stropts.h>
#endif

#if defined (__SVR4) && defined (__sun)
#include <signal.h>
#endif

#ifdef linux
#include <signal.h>
#endif

#if defined(__CYGWIN__)
#include <sys/termios.h>
#endif


#ifdef HAVE_POLL		/* Only for OSs without FD macros */
#include <poll.h>
#endif


#define UIDLIMIT 100		/* Minimum acceptable uid and gid     */
#define GIDLIMIT 1		/* Alter these to suit security prefs */

#define AJ_OUTBUF 10000		/* pipe buffer size      */
#define R_BUFFER  2048		/* Reentrant buffer size */

#define TIMEOUT 30		/* Arbitrary timeout (secs) */

#if defined (__SVR4) && defined (__sun)
#include <sys/filio.h>
#endif

#ifndef WIN32
#include <pwd.h>
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#if !defined(__ppc__) && !defined(__APPLE__) && !defined(__FreeBSD__) && !defined(WIN32)
#include <crypt.h>
#endif



#ifdef N_SHADOW
#include <shadow.h>
#endif
#ifdef R_SHADOW
#include <shadow.h>
#endif
#ifdef HPUX_SHADOW
#include <shadow.h>
#endif

#ifdef PAM
#if defined(__ppc__) || defined(__APPLE__)
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif
#endif

#ifdef AIX_SHADOW
#include <userpw.h>
#endif

#ifdef HPUX_SHADOW
#include <prot.h>
#endif


/*
 *  The following prototypes are used by the root tomcat authorisation
 *  system.
 */

static void java_core_dump(void);
#ifndef NO_AUTH
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			 ajint *gid, AjPStr *home);
#endif


#ifdef PAM
static int PAM_conv(int num_msg, struct pam_message **msg,
		    struct pam_response **resp, void *appdata_ptr);
#endif



/*
 *  The following prototypes are used by the jembossctl authorisation
 *  system
 */
static char **make_array(const AjPStr str);
static void java_tidy_command(AjPStr *str1, AjPStr *str2, AjPStr *str3,
			      AjPStr *str4, AjPStr *str5, AjPStr *str6);
static void java_tidy_command2(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf);
static void java_tidy_command3(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf, int *commpipe,
			       int *outpipe, int *errpipe);



static ajint java_send_auth(int tchan, int rchan,
			    const char *cuser, const char *cpass,
			    AjPStr *errstd);
static ajint java_emboss_fork(int tchan,const char *cuser,const char *cpass,
			      const AjPStr clemboss, const AjPStr enviro,
			      const AjPStr dir,
			      char *buf, AjPStr *errstd);
static ajint java_batch_fork(int tchan,const char *cuser,const char *cpass,
			     const AjPStr clemboss,const  AjPStr enviro,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd);
static ajint java_make_dir(int tchan, const char *cuser, const char *cpass,
			   const AjPStr dir,
			   char *buf, AjPStr *errstd);
static ajint java_delete_file(int tchan, const char *cuser, const char *cpass,
			      const AjPStr ufile, char *buf, AjPStr *errstd);
static ajint java_rename_file(int tchan, const char *cuser,const  char *cpass,
			      const AjPStr ufile, const AjPStr u2file, char *buf,
			      AjPStr *errstd);
static ajint java_delete_dir(int tchan, const char *cuser, const char *cpass,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd);
static ajint java_list_files(int tchan, const char *cuser, const char *cpass,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd);
static ajint java_list_dirs(int tchan, const char *cuser, const char *cpass,
			    const AjPStr dir,
			    char *buf, AjPStr *errstd);
static ajint java_get_file(int tchan, int rchan,
			   const char *cuser, const char *cpass,
			   const AjPStr file, char *buf, unsigned char **fbuf,
			   int *size, AjPStr *errstd);
static ajint java_put_file(int tchan, int rchan,
			   const char *cuser, const char *cpass,
			   const AjPStr file, char *buf,
			   const unsigned char *fbuf,
			   int size, AjPStr *errstd);
static ajint java_seq_attrib(int tchan,const char *cuser,const char *cpass,
			     const AjPStr usa,
			     char *buf, AjPStr *errstd);
static ajint java_seqset_attrib(int tchan,
				const char *cuser,const char *cpass,
				const AjPStr usa,
				char *buf, AjPStr *errstd);

static void java_wait_for_term(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf);
static void java_wait_for_file(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf,
			       unsigned char *fbuf,int size);

static int java_jembossctl(ajint command,
			   const AjPStr username, const AjPStr password,
			   const AjPStr environment,
			   const AjPStr str1, const AjPStr str2,
			   AjPStr *outstd, AjPStr *errstd,
			   unsigned char **fbuf, int *fsize);


static AjBool ajJavaGetSeqFromUsa (const AjPStr thys, AjPSeq *seq);
static AjBool ajJavaGetSeqsetFromUsa (const AjPStr thys, AjPSeqset *seq);


static int java_pipe_write(int tchan, const char *buf, int n, int seconds,
			   AjPStr *errstd);
static int java_pipe_read(int rchan, char *buf, int n, int seconds,
			  AjPStr *errstd);
static int java_snd(int tchan,const char *buf,int len,AjPStr *errstd);
static int java_rcv(int rchan, char *buf, AjPStr *errstd);


static int java_block(int chan, unsigned long flag);




/* @func Ajax.seqType *********************************************************
**
** Return length, weight and type information for a sequence
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] usa [jstring] usa
**
** @return [jboolean] true if the sequence exists
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqType
           (JNIEnv *env, jobject obj, jstring usa)
{

    AjPStr name    = NULL;
    char *javaname = NULL;
    AjBool ok      = ajFalse;
    AjPSeq seq     = NULL;
    AjBool nuc     = ajFalse;
    ajint  len     = 0;
    float  weight  = 0.;
    jclass jvc;
    jfieldID field;

    jvc = (*env)->GetObjectClass(env,obj);

    name = ajStrNew();
    seq  = ajSeqNew();

    javaname = (char *) (*env)->GetStringUTFChars(env,usa,0);
    ajStrAssignC(&name,javaname);
    (*env)->ReleaseStringUTFChars(env,usa,javaname);

    ok = ajJavaGetSeqFromUsa(name,&seq);

    if(ok)
    {
	len = ajSeqGetLen(seq);
	nuc = ajSeqIsNuc(seq);
	weight = seq->Weight;

	field = (*env)->GetStaticFieldID(env,jvc,"length","I");
	(*env)->SetStaticIntField(env,jvc,field,len);

	field = (*env)->GetStaticFieldID(env,jvc,"protein","Z");
	(*env)->SetStaticBooleanField(env,jvc,field,(unsigned char)!nuc);

	field = (*env)->GetStaticFieldID(env,jvc,"weight","F");
	(*env)->SetStaticFloatField(env,jvc,field,weight);
    }

    ajStrDel(&name);
    ajSeqDel(&seq);

    return (unsigned char) ok;
}




/* @func Ajax.seqsetType ******************************************************
**
** Return length, weight and type information for a seqset
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] usa [jstring] usa
**
** @return [jboolean] true if the sequences exist
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqsetType
           (JNIEnv *env, jobject obj, jstring usa)
{
    AjPStr name    = NULL;
    char *javaname = NULL;
    AjBool ok      = ajFalse;
    AjPSeqset seq  = NULL;
    AjBool nuc     = ajFalse;
    ajint  len     = 0;
    float  weight  = 0.;
    jclass jvc;
    jfieldID field;

    name = ajStrNew();
    seq  = ajSeqsetNew();

    jvc = (*env)->GetObjectClass(env,obj);

    javaname = (char *) (*env)->GetStringUTFChars(env,usa,0);
    ajStrAssignC(&name,javaname);
    (*env)->ReleaseStringUTFChars(env,usa,javaname);

    ok = ajJavaGetSeqsetFromUsa(name,&seq);

    if(ok)
    {
	len = ajSeqsetGetLen(seq);
	nuc = ajSeqsetIsNuc(seq);
	weight = ajSeqsetGetTotweight(seq);

	field = (*env)->GetStaticFieldID(env,jvc,"length","I");
	(*env)->SetStaticIntField(env,jvc,field,len);

	field = (*env)->GetStaticFieldID(env,jvc,"protein","Z");
	(*env)->SetStaticBooleanField(env,jvc,field,(unsigned char)!nuc);

	field = (*env)->GetStaticFieldID(env,jvc,"weight","F");
	(*env)->SetStaticFloatField(env,jvc,field,weight);
    }

    ajStrDel(&name);
    ajSeqsetDel(&seq);

    return (unsigned char) ok;
}




/* @funcstatic ajJavaGetSeqFromUsa ********************************************
**
** Return a sequence given a USA
**
** @param [r] thys [const AjPStr] usa
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool ajJavaGetSeqFromUsa(const AjPStr thys, AjPSeq *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    ajNamInit("emboss");

    seqin = ajSeqinNew();
    seqin->multi = ajFalse;
    seqin->Text  = ajFalse;

    ajSeqinUsa (&seqin, thys);
    ok = ajSeqRead(*seq, seqin);
    ajSeqinDel (&seqin);

    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic ajJavaGetSeqsetFromUsa *****************************************
**
** Return a seqset given a usa
**
** @param [r] thys [const AjPStr] usa
** @param [w] seq [AjPSeqset*] seqset
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool ajJavaGetSeqsetFromUsa(const AjPStr thys, AjPSeqset *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    ajNamInit("emboss");

    seqin = ajSeqinNew();
    seqin->multi = ajTrue;
    seqin->Text  = ajFalse;

    ajSeqinUsa (&seqin, thys);
    ok = ajSeqsetRead(*seq, seqin);
    ajSeqinDel (&seqin);


    if(!ok)
	return ajFalse;

    return ajTrue;
}




#ifndef WIN32

/* @func Ajax.userInfo ********************************************************
**
** Return the uid, gid and home directory of a user
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
**
** @return [jboolean] true if the username/password are valid
** @@
******************************************************************************/


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_userInfo
(JNIEnv *env, jobject obj, jstring door, jstring key)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr home     = NULL;
    char *juser     = NULL;
    char *jpass     = NULL;
    AjBool ok       = ajFalse;

    jclass jvc;
    jfieldID field;
    jstring  jhpstr;

    ajint uid = 0;
    ajint gid = 0;
    const char  *hp;

    jvc = (*env)->GetObjectClass(env,obj);

    java_core_dump();

    username = ajStrNew();
    password = ajStrNew();
    home     = ajStrNew();

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	ajStrDel(&username);
	ajStrDel(&password);
	ajStrDel(&home);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);

    jpass = (char *) (*env)->GetStringUTFChars(env,key,0);

    if(jpass)
	ajStrAssignC(&password,jpass);
    else
    {
	ajStrDel(&username);
	ajStrDel(&password);
	ajStrDel(&home);

	return (unsigned char)ajFalse;
    }
    (*env)->ReleaseStringUTFChars(env,key,jpass);

    if(!ajStrGetLen(username) || !ajStrGetLen(password))
    {
	ajStrDel(&username);
	ajStrDel(&password);
	ajStrDel(&home);
	return (unsigned char)ajFalse;
    }

#ifndef NO_AUTH
    ok = java_pass(username,password,&uid,&gid,&home);
#endif

    field = (*env)->GetStaticFieldID(env,jvc,"uid","I");
    (*env)->SetStaticIntField(env,jvc,field,uid);

    field = (*env)->GetStaticFieldID(env,jvc,"gid","I");
    (*env)->SetStaticIntField(env,jvc,field,gid);


    hp = ajStrGetPtr(home);
    field = (*env)->GetFieldID(env,jvc,"home","Ljava/lang/String;");

    jhpstr = (*env)->NewStringUTF(env,hp);
    (*env)->SetObjectField(env,obj,field,jhpstr);


    bzero((void*)ajStrGetPtr(username),ajStrGetLen(username));
    bzero((void*)ajStrGetPtr(password),ajStrGetLen(password));
    bzero((void*)ajStrGetPtr(home),ajStrGetLen(home));

    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&home);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @funcstatic java_core_dump *************************************************
**
** Set process coredump size to be zero
**
** @return [void]
** @@
******************************************************************************/

static void java_core_dump()
{
    struct rlimit limit;

    limit.rlim_cur = 0;
    limit.rlim_max = 0;

    setrlimit(RLIMIT_CORE,&limit);

    return;
}




#ifdef N_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct passwd *pwd  = NULL;
    char *p = NULL;


    shadow = getspnam(ajStrGetPtr(username));

    if(!shadow)                 /* No such username */
        return ajFalse;


    pwd = getpwnam(ajStrGetPtr(username));

    if(!pwd)
        return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),shadow->sp_pwdp);

    if(!strcmp(p,shadow->sp_pwdp))
        return ajTrue;

    return ajFalse;
}
#endif




#ifdef R_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct spwd sresult;
    struct passwd *pwd  = NULL;
    struct passwd presult;
    char *p    = NULL;
    char *sbuf = NULL;
    char *buf  = NULL;
#ifdef _POSIX_C_SOURCE
    int ret=0;
#endif

    if(!(buf=(char*)malloc(R_BUFFER)) || !(sbuf=(char*)malloc(R_BUFFER)))
	return ajFalse;

    shadow = getspnam_r(ajStrGetPtr(username),&sresult,sbuf,R_BUFFER);

    if(!shadow)                 /* No such username */
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }


#ifdef _POSIX_C_SOURCE
    ret = getpwnam_r(ajStrGetPtr(username),&presult,buf,R_BUFFER,&pwd);

    if(ret!=0)
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
#else
    pwd = getpwnam_r(ajStrGetPtr(username),&presult,buf,R_BUFFER);

    if(!pwd)
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
#endif

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),shadow->sp_pwdp);

    if(!strcmp(p,shadow->sp_pwdp))
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajTrue;
    }

    AJFREE(buf);
    AJFREE(sbuf);

    return ajFalse;
}
#endif




#ifdef AIX_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct userpw *shadow = NULL;
    struct passwd *pwd  = NULL;
    char *p = NULL;

    shadow = getuserpw(ajStrGetPtr(username));
    if(!shadow)
	return ajFalse;

    pwd = getpwnam(ajStrGetPtr(username));
    if(!pwd)
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),shadow->upw_passwd);

    if(!strcmp(p,shadow->upw_passwd))
	return ajTrue;

    return ajFalse;
}
#endif




#ifdef HPUX_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct spwd sresult;
    struct passwd *pwd  = NULL;
    struct passwd presult;
    char *p    = NULL;
    char *epwd = NULL;
    char *buf  = NULL;
    int ret=0;
    int trusted;

    trusted = iscomsec();
    if(!(epwd=(char *)malloc(R_BUFFER)))
	return ajFalse;


    if(trusted)
    {
	shadow = getspnam(ajStrGetPtr(username));

	if(!shadow)
	{
	    AJFREE(epwd);

	    return ajFalse;
	}

	strcpy(epwd,shadow->sp_pwdp);
    }


    if(!(buf=(char *)malloc(R_BUFFER)))
	return ajFalse;

    ret = getpwnam_r(ajStrGetPtr(username),&presult,buf,R_BUFFER,&pwd);

    if(ret!=0)
    {
	AJFREE(buf);
	AJFREE(epwd);
	return ajFalse;
    }

    if(!trusted)
	strcpy(epwd,pwd->pw_passwd);

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;
    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),epwd);

    if(!strcmp(p,epwd))
    {
	AJFREE(buf);
	AJFREE(epwd);

	return ajTrue;
    }

    AJFREE(buf);
    AJFREE(epwd);


    return ajFalse;
}
#endif




#ifdef NO_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct passwd *pwd  = NULL;
    char *p = NULL;

    pwd = getpwnam(ajStrGetPtr(username));
    if(!pwd)		 /* No such username */
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),pwd->pw_passwd);

    if(!strcmp(p,pwd->pw_passwd))
	return ajTrue;

    return ajFalse;
}
#endif




#ifdef RNO_SHADOW
/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct passwd *pwd  = NULL;
    char *p = NULL;
    struct passwd result;
    char *buf = NULL;
#if defined(_OSF_SOURCE) || defined(__FreeBSD__)
    int ret=0;
#endif

    if(!(buf=(char *)malloc(R_BUFFER)))
	return ajFalse;

#if defined(_OSF_SOURCE) || defined(__FreeBSD__)
    ret = getpwnam_r(ajStrGetPtr(username),&result,buf,R_BUFFER,&pwd);
    if(ret!=0)		 /* No such username */
    {
	AJFREE(buf);

	return ajFalse;
    }
#else
    pwd = getpwnam_r(ajStrGetPtr(username),&result,buf,R_BUFFER);
    if(!pwd)		 /* No such username */
    {
	AJFREE(buf);

	return ajFalse;
    }
#endif

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

    p = crypt(ajStrGetPtr(password),pwd->pw_passwd);

    if(!strcmp(p,pwd->pw_passwd))
    {
	AJFREE(buf);

	return ajTrue;
    }

    AJFREE(buf);

    return ajFalse;
}
#endif




#ifdef PAM

struct ad_user
{
    char *username;
    char *password;
};


/* @header PAM_conv **********************************************************
**
******************************************************************************/
static int PAM_conv(int num_msg, struct pam_message **msg,
		    struct pam_response **resp, void *appdata_ptr)
{
    struct ad_user *user;
    struct pam_response *response;
    int i;

    user = (struct ad_user *)appdata_ptr;

    if (msg == NULL || resp == NULL || user == NULL)
    	return PAM_CONV_ERR;

    response= (struct pam_response *)
    	malloc(num_msg * sizeof(struct pam_response));

    for(i=0;i<num_msg;++i)
    {
	response[i].resp_retcode = 0;
	response[i].resp = NULL;

	switch(msg[i]->msg_style)
	{
	case PAM_PROMPT_ECHO_ON:
	    /* Store the login as the response */
	    response[i].resp = appdata_ptr ?
		(char *)strdup(user->username) : NULL;
	    break;

	case PAM_PROMPT_ECHO_OFF:
	    /* Store the password as the response */
	    response[i].resp = appdata_ptr ?
		(char *)strdup(user->password) : NULL;
	    break;

	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
	    break;

	default:
	    if(response)
		free(response);

	    return PAM_CONV_ERR;
	}
    }

    /* On success, return the response structure */
    *resp= response;
    return PAM_SUCCESS;
}




/* @header java_pass **********************************************************
**
******************************************************************************/
static AjBool java_pass(AjPStr username,AjPStr password,ajint *uid,
			ajint *gid,AjPStr *home)
{
    struct ad_user user_info;

    struct pam_cv
    {
	int (*cv)(int,struct pam_message **,struct pam_response **,void *);
	void *userinfo;
    };

    struct pam_cv conv;
    pam_handle_t *pamh = NULL;
    int retval;

    struct passwd *pwd = NULL;

    user_info.username = (char *) ajStrGetPtr(username);
    user_info.password = (char *) ajStrGetPtr(password);

    conv.cv = PAM_conv;
    conv.userinfo = (void *)&user_info;

    pwd = getpwnam(ajStrGetPtr(username));

    if(!pwd)		 /* No such username */
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssignC(home,pwd->pw_dir);

#ifndef DEBIAN
    retval = pam_start("login",ajStrGetPtr(username),
		       (struct pam_conv*)&conv,&pamh);
#else
    retval = pam_start("ssh",ajStrGetPtr(username),
		       (struct pam_conv*)&conv,&pamh);
#endif

    if (retval == PAM_SUCCESS)
	retval= pam_authenticate(pamh,PAM_SILENT);

    if(retval==PAM_SUCCESS)
	retval = pam_acct_mgmt(pamh,0);

    if(pam_end(pamh,retval)!=PAM_SUCCESS)
    {
	pamh = NULL;

	return ajFalse;
    }

    if(retval==PAM_SUCCESS)
	return ajTrue;

    return ajFalse;
}
#endif




/* @func Ajax.setuid **********************************************************
**
** Set the uid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] uid [jint] uid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setuid
(JNIEnv *env, jclass j, jint uid)
{
    return((jint)setuid((uid_t)uid));
}




/* @func Ajax.seteuid *********************************************************
**
** Set the effective uid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] uid [jint] uid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_seteuid
(JNIEnv *env, jclass j, jint uid)
{
#ifndef __hpux
    return((jint)seteuid((uid_t)uid));
#else
    return -1;
#endif
}




/* @func Ajax.setgid **********************************************************
**
** Set the gid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] gid [jint] gid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setgid
(JNIEnv *env, jclass j, jint gid)
{
    return((jint)setgid((gid_t)gid));
}




/* @func Ajax.setegid *********************************************************
**
** Set the effective gid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] gid [jint] gid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setegid
(JNIEnv *env, jclass j, jint gid)
{
#ifndef __hpux
    return((jint)setegid((gid_t)gid));
#else
    return -1;
#endif
}




/* @func Ajax.getuid **********************************************************
**
** Return the uid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] uid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getuid
(JNIEnv *env, jclass j)
{
    return((jint)getuid());
}




/* @func Ajax.getgid **********************************************************
**
** Return the gid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] gid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getgid
(JNIEnv *env, jclass j)
{
    return((jint)getgid());
}




/* @func Ajax.geteuid *********************************************************
**
** Return the effective uid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] uid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_geteuid
(JNIEnv *env, jclass j)
{
    return((jint)geteuid());
}




/* @func Ajax.getegid *********************************************************
**
** Return the effective gid of the current process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] gid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getegid
(JNIEnv *env, jclass j)
{
    return((jint)getegid());
}




/* @func Ajax.fork ************************************************************
**
** Fork off new process with given uid & gid, chdir and execute command
** return stdout and stderr of the child process
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] commandline [jstring] command to execute
** @param [r] environment [jstring] shell environment
** @param [r] directory [jstring] chdir here before execution
** @param [r] uid [jint] uid for setuid
** @param [r] gid [jint] gid for setgid
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_fork
(JNIEnv *env, jobject obj, jstring commandline,
 jstring environment, jstring directory, jint uid, jint gid)
{
    jclass jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;

    char  *sptr;

    AjPStr prog = NULL;
    AjPStr cl   = NULL;
    AjPStr envi = NULL;
    AjPStr dir  = NULL;


    char **argp = NULL;
    char **envp = NULL;
    int  pid;
    int  status = 0;
    int  i=0;

    int  outpipe[2];
    int  errpipe[2];

#ifdef HAVE_POLL
    struct pollfd ufds[2];
    unsigned int nfds;
#else
    fd_set rec;
    struct timeval t;
#endif

    int nread = 0;
    char *buf;
    AjPStr outstd = NULL;
    AjPStr errstd = NULL;
    int retval    = 0;
    const char *save;

    jvc = (*env)->GetObjectClass(env,obj);

    if(!uid || !gid)
	return (unsigned char)ajFalse;

    if(uid<UIDLIMIT)
	return (unsigned char)ajFalse;

    if(gid<GIDLIMIT)
	return (unsigned char)ajFalse;

    AJCNEW0(buf,AJ_OUTBUF+1);


    prog  = ajStrNew();
    cl    = ajStrNew();
    envi  = ajStrNew();
    dir   = ajStrNew();

    outstd = ajStrNew();
    errstd = ajStrNew();


    sptr = (char *) (*env)->GetStringUTFChars(env,commandline,0);
    ajStrAssignC(&cl,sptr);
    (*env)->ReleaseStringUTFChars(env,commandline,sptr);


    ajSysFuncStrtokR(ajStrGetPtr(cl)," \t\n",&save,&prog);


    sptr = (char *) (*env)->GetStringUTFChars(env,environment,0);
    ajStrAssignC(&envi,sptr);
    (*env)->ReleaseStringUTFChars(env,environment,sptr);

    sptr = (char *) (*env)->GetStringUTFChars(env,directory,0);
    ajStrAssignC(&dir,sptr);
    (*env)->ReleaseStringUTFChars(env,directory,sptr);


    argp = make_array(cl);
    envp = make_array(envi);

    if(!ajSysFileWhichEnv(&prog,envp))
    {
	java_tidy_command(&prog,&cl,&envi,&dir,&outstd,&errstd);
	i = 0;

	while(argp[i])
	    AJFREE(argp[i++]);

	AJFREE(argp);

	i = 0;

	while(envp[i])
	    AJFREE(envp[i++]);

	AJFREE(envp);

	return (unsigned char)ajFalse;
    }


    while(pipe(outpipe)==-1);
    while(pipe(errpipe)==-1);

#if defined (__SVR4) && defined (__sun)
    pid = fork1();
#else
    pid = fork();
#endif

    if(pid == -1)
    {
	java_tidy_command(&prog,&cl,&envi,&dir,&outstd,&errstd);
	i = 0;

	while(argp[i])
	    AJFREE(argp[i]);

	AJFREE(argp);

	i = 0;

	while(envp[i])
	    AJFREE(envp[i]);

	AJFREE(envp);

	return (unsigned char)ajFalse;
    }


    if(!pid)			/* Child */
    {
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);

	if(setgid(gid)==-1)
	{
	    fprintf(stderr,"setgid failure");
	    exit(-1);
	}

	if(setuid(uid)==-1)
	{
	    fprintf(stderr,"setuid failure");
	    exit(-1);
	}

	if(chdir(ajStrGetPtr(dir))==-1)
	{
	    fprintf(stderr,"chdir failure");
	    exit(-1);
	}

	if(execve(ajStrGetPtr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure");
	    exit(-1);
	}
    }

    *buf = '\0';

#ifdef HAVE_POLL
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	ufds[0].fd = outpipe[0];
	ufds[1].fd = errpipe[0];
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = POLLIN | POLLPRI;
	nfds = 2;

	if(!(retval=poll(ufds,nfds,1)) || retval==-1)
	    continue;

	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(&outstd,buf);
	}


	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(&errstd,buf);
	}
    }


    ufds[0].fd = outpipe[0];
    ufds[1].fd = errpipe[0];
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    nfds = 2;

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppendC(&outstd,buf);
	}

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppendC(&errstd,buf);
	}
#else
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);

	if(FD_ISSET(outpipe[0],&rec))
	{
	    nread = read(outpipe[0],(void *)buf,AJ_OUTBUF);
	    buf[nread]='\0';
	    ajStrAppendC(&outstd,buf);
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);

	if(FD_ISSET(errpipe[0],&rec))
	{
	    nread = read(errpipe[0],(void *)buf,AJ_OUTBUF);
	    buf[nread]='\0';
	    ajStrAppendC(&errstd,buf);
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);

    if(FD_ISSET(outpipe[0],&rec))
    {
	nread = read(outpipe[0],(void *)buf,AJ_OUTBUF);
	buf[nread]='\0';
	ajStrAppendC(&outstd,buf);
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);

    if(FD_ISSET(errpipe[0],&rec))
    {
	nread = read(errpipe[0],(void *)buf,AJ_OUTBUF);
	buf[nread]='\0';
	ajStrAppendC(&errstd,buf);
    }
#endif

    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    close(errpipe[0]);
    close(errpipe[1]);
    close(outpipe[0]);
    close(outpipe[1]);

    i = 0;

    while(argp[i])
	AJFREE(argp[i]);

    AJFREE(argp);

    i = 0;

    while(envp[i])
	AJFREE(envp[i]);

    AJFREE(envp);


    AJFREE(buf);

    ajStrDel(&prog);
    ajStrDel(&cl);
    ajStrDel(&envi);
    ajStrDel(&dir);
    ajStrDel(&outstd);
    ajStrDel(&errstd);

    return (unsigned char)ajTrue;
}




/* @funcstatic make_array *****************************************************
**
** Construct argv and env arrays for Ajax.fork
**
** @param [r] str [const AjPStr] space separated tokens
**
** @return [char**] env or argv array
******************************************************************************/

static char** make_array(const AjPStr str)
{
    int n;
    char **ptr   = NULL;
    AjPStr buf;
    const char *save = NULL;

    buf = ajStrNew();

    n = ajStrParseCountMultiC(str," \t\n");

    AJCNEW0(ptr,n+1);

    ptr[n] = NULL;

    n = 0;

    if(!ajSysFuncStrtokR(ajStrGetPtr(str)," \t\n",&save,&buf))
	return ptr;

    ptr[n++] = ajCharNewS(buf);

    while(ajSysFuncStrtokR(NULL," \t\n",&save,&buf))
	ptr[n++] = ajCharNewS(buf);

    ajStrDel(&buf);

    return ptr;
}




/* @funcstatic java_send_auth *************************************************
**
** Send verification/homedir request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] rchan [int] return channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_send_auth(int tchan, int rchan,
			    const char *cuser, const char *cpass,
			    AjPStr *errstd)
{
    AjPStr cmnd = NULL;
    char   c;
    int    n;

    cmnd = ajStrNew();
    ajFmtPrintS(&cmnd,"%d %s %s",COMM_AUTH,cuser,cpass);

    if(java_snd(tchan,ajStrGetPtr(cmnd),ajStrGetLen(cmnd)+1,errstd)<0)
    {
	ajStrAppendC(errstd,"Error sending (java_send_auth)\n");
	ajStrDel(&cmnd);

	return -1;
    }


    n = java_rcv(rchan,(char *)&c,errstd);

    if(n==-1)
    {
	ajStrAppendC(errstd,"Error receiving (java_send_auth)\n");
	ajStrDel(&cmnd);

	return -1;
    }



    ajStrDel(&cmnd);


    if(!c)
	return -1;

    return 0;
}




/* @funcstatic java_emboss_fork ***********************************************
**
** Send an emboss fork request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] clemboss [const AjPStr] emboss command line
** @param [r] envi [const AjPStr] emboss environment
** @param [r] dir [const AjPStr] directory to invoke emboss from
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_emboss_fork(int tchan, const char *cuser, const char *cpass,
			      const AjPStr clemboss, const AjPStr envi,
			      const AjPStr dir,
			      char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",EMBOSS_FORK,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(clemboss));
    p += (ajStrGetLen(clemboss)+1);
    sprintf(p,"%s",ajStrGetPtr(envi));
    p += (ajStrGetLen(envi)+1);
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_emboss_fork)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_batch_fork ************************************************
**
** Send a detached emboss fork request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] clemboss [const AjPStr] emboss command line
** @param [r] envi [const AjPStr] emboss environment
** @param [r] dir [const AjPStr] directory to invoke emboss from
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_batch_fork(int tchan, const char *cuser,const char *cpass,
			     const AjPStr clemboss, const AjPStr envi,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",BATCH_FORK,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(clemboss));
    p += (ajStrGetLen(clemboss)+1);
    sprintf(p,"%s",ajStrGetPtr(envi));
    p += (ajStrGetLen(envi)+1);
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_batch_fork)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_make_dir **************************************************
**
** Send a make directory request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] dir [const AjPStr] directory to create
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_make_dir(int tchan,const char *cuser,const char *cpass,
			   const AjPStr dir,
			   char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",MAKE_DIRECTORY,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_make_dir)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_delete_file ***********************************************
**
** Send a delete file request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] ufile [const AjPStr] file to delete
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_delete_file(int tchan,const char *cuser,const char *cpass,
			      const AjPStr ufile,
			      char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",DELETE_FILE,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(ufile));
    n = (p-buf) + ajStrGetLen(ufile) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_delete_file)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_rename_file ***********************************************
**
** Send a rename file request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] ufile [const AjPStr] file to rename
** @param [r] u2file [const AjPStr] new filename
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_rename_file(int tchan,const char *cuser,const char *cpass,
			      const AjPStr ufile,
			      const AjPStr u2file, char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",RENAME_FILE,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(ufile));
    p += (ajStrGetLen(ufile)+1);
    sprintf(p,"%s",ajStrGetPtr(u2file));
    n = (p-buf) + ajStrGetLen(u2file) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_rename_file)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_delete_dir ************************************************
**
** Send a delete directory request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] dir [const AjPStr] directory to delete
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_delete_dir(int tchan,const char *cuser,const char *cpass,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",DELETE_DIR,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_delete_dir)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_list_files ************************************************
**
** Send a list files request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] dir [const AjPStr] directory to scan
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_list_files(int tchan,const char *cuser,const char *cpass,
			     const AjPStr dir,
			     char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",LIST_FILES,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_list_files)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_list_dirs *************************************************
**
** Send a list directories request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] dir [const AjPStr] directory to scan
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_list_dirs(int tchan,const char *cuser,const char *cpass,
			    const AjPStr dir,
			    char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",LIST_DIRS,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(dir));
    n = (p-buf) + ajStrGetLen(dir) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_list_dirs)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_get_file **************************************************
**
** Send a get file request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] rchan [int] return channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] file [const AjPStr] file to get
** @param [w] buf [char*] socket buffer
** @param [w] fbuf [unsigned char**] file buffer
** @param [w] size [int*] file size
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_get_file(int tchan,int rchan,
			   const char *cuser,const char *cpass,
			   const AjPStr file, char *buf,unsigned char **fbuf,
			   int *size, AjPStr *errstd)
{
    char *p = NULL;
    int n;
    int rlen = 0;

    n = sprintf(buf,"%d %s %s",GET_FILE,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(file));
    n = (p-buf) + ajStrGetLen(file) +1;


    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_get_file)\n");

	return -1;
    }


    rlen = java_rcv(rchan,buf,errstd);

    if(rlen==-1)
    {
	*size = 0;
	ajStrAppendC(errstd,"Reading error (java_get_file)\n");

	return -1;
    }


    if(sscanf(buf,"%d",&n)!=1)
    {
	ajStrAppendC(errstd,"Bad size (java_get_file)\n");

	return -1;
    }



    if(n==-1)
    {
	ajStrAppendC(errstd,"Bad size (java_get_file)\n");

	return -1;
    }

    *size = n;

    if(n)
	if(!(*fbuf=(unsigned char *)malloc(n)))
	{
	    ajStrAppendC(errstd,"Malloc error (java_get_file)\n");

	    return -1;
	}

    return 0;
}




/* @funcstatic java_put_file **************************************************
**
** Send a put file request to jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] rchan [int] return channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] file [const AjPStr] file to get
** @param [w] buf [char*] socket buffer
** @param [r] fbuf [const unsigned char*] file buffer
** @param [r] size [int] file size
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_put_file(int tchan,int rchan,
			   const char *cuser,const char *cpass,
			   const AjPStr file,
			   char *buf,const unsigned char *fbuf,int size,
			   AjPStr *errstd)
{
    char *p  = NULL;
    int n;
    int pos  = 0;
    int rlen = 0;


    n = sprintf(buf,"%d %s %s",PUT_FILE,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(file));
    n = (p-buf) + ajStrGetLen(file) +1;

    /* send command */
    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_put_file)\n");

	return -1;
    }


    rlen = java_rcv(rchan,buf,errstd);

    if(rlen==-1)
    {
	ajStrAppendC(errstd,"Socket Recv error 2\n");

	return -1;
    }




    /* Send file size */
    n = sprintf(buf,"%d",size);

    if(java_snd(tchan,buf,n+1,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_put_file)\n");

	return -1;
    }


    rlen = java_rcv(rchan,buf,errstd);

    if(rlen==-1)
    {
	ajStrAppendC(errstd,"Socket Recv error 2\n");

	return -1;
    }



    while(pos+JBUFFLEN < size)
    {
 	memcpy((void *)buf,(const void *)&fbuf[pos],JBUFFLEN);

	if(java_snd(tchan,buf,JBUFFLEN,errstd) < 0)
	{
	    ajStrAppendC(errstd,"Sending error (java_put_file)\n");

	    return -1;
	}

	pos += JBUFFLEN;
    }

    if(size)
	if(size-pos)
	{
	    memcpy((void *)buf,(const void *)&fbuf[pos],size-pos);

	    if(java_snd(tchan,buf,size-pos,errstd) < 0)
	    {
		ajStrAppendC(errstd,"Sending error (java_put_file)\n");

		return -1;
	    }
	}

    return 0;
}




/* @funcstatic java_seq_attrib ************************************************
**
** Get sequence attributes via jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] usa [const AjPStr] usa
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_seq_attrib(int tchan,const char *cuser,const char *cpass,
			     const AjPStr usa,
			     char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",SEQ_ATTRIB,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(usa));
    n = (p-buf) + ajStrGetLen(usa) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_seq_attrib)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_seqset_attrib *********************************************
**
** Get seqset attributes from jembossctl
**
** @param [r] tchan [int] to channel file descriptor
** @param [r] cuser [const char*] username
** @param [r] cpass [const char*] password
** @param [r] usa [const AjPStr] usa
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_seqset_attrib(int tchan,const char *cuser,const char *cpass,
				const AjPStr usa,
				char *buf, AjPStr *errstd)
{
    char *p = NULL;
    int n;


    n = sprintf(buf,"%d %s %s",SEQSET_ATTRIB,cuser,cpass);
    p = buf+n+1;
    sprintf(p,"%s",ajStrGetPtr(usa));
    n = (p-buf) + ajStrGetLen(usa) +1;

    if(java_snd(tchan,buf,n,errstd) < 0)
    {
	ajStrAppendC(errstd,"Sending error (java_seqset_attrib)\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_wait_for_term *********************************************
**
** Wait for jembossctl to terminate
**
** @param [r] pid [int] jembossctl pid
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [w] outpipe [int*] stdout pipe
** @param [w] errpipe [int*] stderr pipe
** @param [w] buf [char*] pipe buffer
**
** @return [void]
******************************************************************************/

static void java_wait_for_term(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf)
{
#ifdef HAVE_POLL
    struct pollfd ufds[2];
    unsigned int  nfds;
#else
    fd_set rec;
    struct timeval t;
#endif

    int nread  = 0;
    int  status;
    int retval = 0;
    unsigned long block = 0;


    block = 1;

    if(java_block(outpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 1a. %d\n",errno);

	return;
    }

    if(java_block(errpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 2a. %d\n",errno);

	return;
    }

    *buf = '\0';


#ifdef HAVE_POLL
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	ufds[0].fd = outpipe[0];
	ufds[1].fd = errpipe[0];
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = POLLIN | POLLPRI;
	nfds = 2;

	if(!(retval=poll(ufds,nfds,1)) || retval==-1)
	    continue;

	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(outstd,buf);
	}


	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(errstd,buf);
	}
    }


    ufds[0].fd = outpipe[0];
    ufds[1].fd = errpipe[0];
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    nfds = 2;

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(outstd,buf);
	}

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    buf[nread]='\0';
	    ajStrAppendC(errstd,buf);
	}
#else
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);

	if(FD_ISSET(outpipe[0],&rec))
	{
	    *buf = '\0';

	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread > 0)
	    {
		buf[nread]='\0';
		ajStrAppendC(outstd,buf);
	    }
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(errpipe[0],&rec))
	{
	    *buf = '\0';

	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread>0)
	    {
		buf[nread]='\0';
		ajStrAppendC(errstd,buf);
	    }
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);

    if(FD_ISSET(outpipe[0],&rec))
    {
	*buf = '\0';

	while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);

	if(nread>0)
	{
	    buf[nread]='\0';
	    ajStrAppendC(outstd,buf);
	}
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);

    if(FD_ISSET(errpipe[0],&rec))
    {
	*buf = '\0';

	while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);

	if(nread >0)
	{
	    buf[nread]='\0';
	    ajStrAppendC(errstd,buf);
	}
    }
#endif

    block = 0;

    if(java_block(outpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 3a. %d\n",errno);

	return;
    }

    if(java_block(errpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 4a. %d\n",errno);

	return;
    }


    return;
}




/* @funcstatic java_wait_for_file *********************************************
**
** Wait for jembossctl to terminate. Receive a file from stdout pipe
**
** @param [r] pid [int] jembossctl pid
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [w] outpipe [int*] stdout pipe
** @param [w] errpipe [int*] stderr pipe
** @param [w] buf [char*] pipe buffer
** @param [w] fbuf [unsigned char*] buffer for file
** @param [r] size [int] size of file to receive
**
** @return [void]
******************************************************************************/

static void java_wait_for_file(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf,
			       unsigned char *fbuf,int size)
{
#ifdef HAVE_POLL
    struct pollfd ufds[2];
    unsigned int  nfds;
#else
    fd_set rec;
    struct timeval t;
#endif

    int nread  = 0;
    int  status;
    int retval = 0;
    unsigned char *ptr  = NULL;
    unsigned long block = 0;


    block = 1;

    if(java_block(outpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 5a. %d\n",errno);

	return;
    }

    if(java_block(errpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 6a. %d\n",errno);

	return;
    }

    *buf = '\0';

    if(size)
	ptr = fbuf;


#ifdef HAVE_POLL
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	ufds[0].fd = outpipe[0];
	ufds[1].fd = errpipe[0];
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = POLLIN | POLLPRI;
	nfds = 2;

	if(!(retval=poll(ufds,nfds,1)) || retval==-1)
	    continue;

	if(size && ((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI)))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread >0)
	    {
		memcpy((void *)ptr,(const void *)buf,nread);
		ptr += nread;
	    }
	}


	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread > -1)
	    {
		buf[nread]='\0';
		ajStrAppendC(errstd,buf);
	    }
	}
    }


    ufds[0].fd = outpipe[0];
    ufds[1].fd = errpipe[0];
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    nfds = 2;

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if(size && ((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI)))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread > 0)
	    {
		memcpy((void *)ptr,(const void *)buf,nread);
		ptr += nread;
	    }
	}

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread > -1)
	    {
		buf[nread]='\0';
		ajStrAppendC(errstd,buf);
	    }
	}
#else
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);

	if(size && FD_ISSET(outpipe[0],&rec))
	{
	    *buf = '\0';
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread>0)
	    {
		memcpy((void *)ptr,(const void *)buf,nread);
		ptr += nread;
	    }
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);

	if(FD_ISSET(errpipe[0],&rec))
	{
	    *buf = '\0';

	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);

	    if(nread > -1)
	    {
		buf[nread]='\0';
		ajStrAppendC(errstd,buf);
	    }
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);

    if(size && FD_ISSET(outpipe[0],&rec))
    {
	*buf = '\0';

	while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);

	if(nread>0)
	{
	    memcpy((void *)ptr,(const void *)buf,nread);
	    ptr += nread;
	}
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);

    if(FD_ISSET(errpipe[0],&rec))
    {
	*buf = '\0';
	while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);

	if(nread>-1)
	{
	    buf[nread]='\0';
	    ajStrAppendC(errstd,buf);
	}
    }
#endif

    if(size)
	if(ptr-fbuf != size)
	    ajStrAppendC(errstd,"\nIncomplete file read\n");

    block = 0;

    if(java_block(outpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 7a. %d\n",errno);

	return;
    }

    if(java_block(errpipe[0],block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 8a. %d\n",errno);

	return;
    }

    return;
}




/* @funcstatic java_jembossctl ************************************************
**
** fork jembossctl and broker requests
**
** @param [r] command [ajint] command type
** @param [r] username [const AjPStr] username
** @param [r] password [const AjPStr] password
** @param [r] environment [const AjPStr] environment
** @param [r] str1 [const AjPStr] command string. depends on command
** @param [r] str2 [const AjPStr] command string. depends on command
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [w] fbuf [unsigned char**] file buffer
** @param [w] fsize [int*] file size
**
** @return [int] 0=success -1=failure
******************************************************************************/

static int java_jembossctl(ajint command,
			   const AjPStr username, const AjPStr password,
			   const AjPStr environment,
			   const AjPStr str1, const AjPStr str2,
			   AjPStr *outstd, AjPStr *errstd,
			   unsigned char **fbuf, int *fsize)
{
    AjPStr dir    = NULL;
    AjPStr unused = NULL;

    AjPStr prog     = NULL;    /* Fork strings */
    AjPStr cl       = NULL;
    AjPStr clemboss = NULL;
    AjPStr envi     = NULL;

    int    i=0;


    const char *cuser;
    const char *cpass;

    char *buff  = NULL;
    int size = 0;

    int rlen = 0;

    char **argp = NULL;
    char **envp = NULL;
    int  pid;

    int  outpipe[2];
    int  errpipe[2];
    int  commpipe[2];
    int  tchan;
    int  rchan;

    int retval = 0;
    char c;
    int  n;

    cuser = ajStrGetPtr(username);
    cpass = ajStrGetPtr(password);

    unused   = ajStrNew();
    clemboss = ajStrNew();
    dir      = ajStrNew();
    cl       = ajStrNew();
    envi     = ajStrNew();
    prog     = ajStrNew();


    if(!(buff=(char*)malloc(JBUFFLEN)))
    {
	ajStrAppendC(errstd,"Malloc error\n");
	java_tidy_command(&unused,&cl,&clemboss,&dir,&envi,&prog);

	return -1;
    }



    /* Set environment for execve */
    ajStrAssignS(&envi,environment);
    envp = make_array(envi);

    /* Set program name */
    ajStrAssignC(&prog,"jembossctl");

    if(!ajSysFileWhichEnv(&prog,envp))
    {
	ajStrAppendC(errstd,"Cannot locate jembossctl\n");
	java_tidy_command2(&unused,&cl,&clemboss,&dir,&envi,&prog,buff);
	i = 0;

	while(envp[i])
	    AJFREE(envp[i++]);

	AJFREE(envp);

	return -1;
    }

    /* Setup commandline */
    ajFmtPrintS(&cl,"jembossctl");
    argp = make_array(cl);

    while(pipe(outpipe)==-1);
    while(pipe(errpipe)==-1);
    while(pipe(commpipe)==-1);

    tchan = commpipe[1];
    rchan = outpipe[0];




#if defined (__SVR4) && defined (__sun)
    pid = fork1();
#else
    pid = fork();
#endif
    if(pid == -1)
    {
	ajStrAppendC(errstd,"Fork error\n");
	java_tidy_command3(&unused,&cl,&clemboss,&dir,&envi,&prog,buff,
			   commpipe,outpipe,errpipe);
	i = 0;

	while(argp[i])
	    AJFREE(argp[i]);

	AJFREE(argp);

	i = 0;

	while(envp[i])
	    AJFREE(envp[i]);

	AJFREE(envp);

	return -1;
    }



    if(!pid)			/* Child (jembossctl) prog */
    {
	dup2(commpipe[0],0);
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);

	if(execve(ajStrGetPtr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure\n");
	    exit(-1);
	}
    }



    /* Don't need the env or commline for the parent */
    i = 0;

    while(argp[i])
        AJFREE(argp[i]);

    AJFREE(argp);

    i = 0;

    while(envp[i])
        AJFREE(envp[i]);

    AJFREE(envp);
    /* End of fork code */


    *buff = '\0';
    rlen = java_rcv(rchan,buff,errstd);

    if(rlen==-1)
    {
	ajStrAppendC(errstd,"Pipe Recv error 1\n");
	java_tidy_command3(&unused,&cl,&clemboss,&dir,&envi,&prog,buff,
			   commpipe,outpipe,errpipe);
	return -1;
    }

    if(rlen<2 || strncmp(buff,"OK",2))
    {
	ajStrAppendC(errstd,"Incorrect ACK error\n");
	java_tidy_command3(&unused,&cl,&clemboss,&dir,&envi,&prog,buff,
			   commpipe,outpipe,errpipe);
	return -1;
    }

    /* At this point pipe comms are active and commands can be sent */

    switch(command)
    {
    case COMM_AUTH:
	retval = java_send_auth(tchan,rchan,cuser,cpass,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case EMBOSS_FORK:
	ajStrAssignS(&clemboss,str1);
	ajStrAssignS(&dir,str2);
	retval = java_emboss_fork(tchan,cuser,cpass,clemboss,envi,dir,buff,
				  errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case MAKE_DIRECTORY:
	ajStrAssignS(&dir,str1);
	retval = java_make_dir(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case DELETE_FILE:
	ajStrAssignS(&dir,str1);
	retval = java_delete_file(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case RENAME_FILE:
	ajStrAssignS(&dir,str1);
	ajStrAssignS(&clemboss,str2);
	retval = java_rename_file(tchan,cuser,cpass,dir,clemboss,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case DELETE_DIR:
	ajStrAssignS(&dir,str1);
	retval = java_delete_dir(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case LIST_FILES:
	ajStrAssignS(&dir,str1);
	retval = java_list_files(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case LIST_DIRS:
	ajStrAssignS(&dir,str1);
	retval = java_list_dirs(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case GET_FILE:
	ajStrAssignS(&dir,str1);
	retval = java_get_file(tchan,rchan,cuser,cpass,dir,buff,fbuf,&size,
			       errstd);
	if(retval==-1)
	{
	    size = 0;
	    *fsize = 0;
	}
	else
	    *fsize = size;

	java_wait_for_file(pid,outstd,errstd,outpipe,errpipe,buff,*fbuf,size);
	break;

    case PUT_FILE:
	ajStrAssignS(&dir,str1);
	size = *fsize;
	retval = java_put_file(tchan,rchan,cuser,cpass,dir,buff,*fbuf,size,
			       errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case BATCH_FORK:
	ajStrAssignS(&clemboss,str1);
	ajStrAssignS(&dir,str2);
	retval = java_batch_fork(tchan,cuser,cpass,clemboss,envi,dir,buff,
				  errstd);

	n = java_rcv(rchan,(char *)&c,errstd);

	if(n==-1 || !c)
	    ajStrAppendC(errstd,"Error receiving (java_batch)\n");
#ifdef __hpux
	signal(SIGCLD,SIG_IGN);
#endif
#ifdef linux
	signal(SIGCLD,SIG_IGN);
#endif
#if defined (__SVR4) && defined (__sun)
	signal(SIGCHLD,SIG_IGN);
#endif
	break;

    case SEQ_ATTRIB:
	ajStrAssignS(&dir,str1);
	retval = java_seq_attrib(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case SEQSET_ATTRIB:
	ajStrAssignS(&dir,str1);
	retval = java_seqset_attrib(tchan,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    default:
	break;
    }


    java_tidy_command3(&unused,&cl,&clemboss,&dir,&envi,&prog,buff,
		       commpipe,outpipe,errpipe);


    return retval;
}




/* @funcstatic java_tidy_command **********************************************
**
** Delete allocated memory
**
** @param [w] str1 [AjPStr*] any string
** @param [w] str2 [AjPStr*] any string
** @param [w] str3 [AjPStr*] any string
** @param [w] str4 [AjPStr*] any string
** @param [w] str5 [AjPStr*] any string
** @param [w] str6 [AjPStr*] any string
**
** @return [void]
******************************************************************************/

static void java_tidy_command(AjPStr *str1, AjPStr *str2, AjPStr *str3,
			      AjPStr *str4, AjPStr *str5, AjPStr *str6)
{

    ajStrDel(str1);
    ajStrDel(str2);
    ajStrDel(str3);
    ajStrDel(str4);
    ajStrDel(str5);
    ajStrDel(str6);

    return;
}




/* @funcstatic java_tidy_command2 *********************************************
**
** Delete allocated memory
**
** @param [w] uniq [AjPStr*] unique name
** @param [w] cl [AjPStr*] command line
** @param [w] clemboss [AjPStr*] emboss command line
** @param [w] dir [AjPStr*] directory
** @param [w] envi [AjPStr*] environment
** @param [w] prog [AjPStr*] program name
** @param [w] buf [char*] socket buffer
**
** @return [void]
******************************************************************************/

static void java_tidy_command2(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf)
{

    java_tidy_command(uniq,cl,clemboss,dir,envi,prog);
    AJFREE(buf);

    return;
}




/* @funcstatic java_tidy_command3 *********************************************
**
** Delete allocated memory and close pipes
**
** @param [w] uniq [AjPStr*] unique name
** @param [w] cl [AjPStr*] command line
** @param [w] clemboss [AjPStr*] emboss command line
** @param [w] dir [AjPStr*] directory
** @param [w] envi [AjPStr*] environment
** @param [w] prog [AjPStr*] program name
** @param [w] buf [char*] socket buffer
** @param [u] commpipe [int*] command pipe file descriptors
** @param [u] outpipe [int*] stdout pipe
** @param [u] errpipe [int*] stderr pipe
**
** @return [void]
******************************************************************************/

static void java_tidy_command3(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf, int *commpipe,
			       int *outpipe, int *errpipe)
{

    close(commpipe[0]);
    close(commpipe[1]);
    close(outpipe[0]);
    close(outpipe[1]);
    close(errpipe[0]);
    close(errpipe[1]);

    java_tidy_command2(uniq,cl,clemboss,dir,envi,prog,buf);

    return;
}




/* @func Ajax.userAuth ********************************************************
**
** Verify user and retrieve home directory
** Loads outStd,errStd within java.
** Sets 'home' in Java
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_userAuth
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass jvc;
    jfieldID field;
    jstring  hstr;    /* Java returned strings */
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    jsize  plen;
    jbyte  *ca;

    AjPStr home=NULL;

    AjBool ok=ajFalse;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    home     = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);


    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&home,&envi,&outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);

    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&home,&envi,&outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&home,&envi,&outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    ok = ajFalse;

    if(!java_jembossctl(COMM_AUTH,username,password,envi,NULL,NULL,&outstd,
			&errstd,NULL,NULL))
    {
	ok = ajTrue;
	ajStrAssignS(&home,outstd);
    }


    field = (*env)->GetFieldID(env,jvc,"home","Ljava/lang/String;");
    hstr = (*env)->NewStringUTF(env,ajStrGetPtr(home));
    (*env)->SetObjectField(env,obj,field,hstr);


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&home,&envi,&outstd,&errstd);


    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.forkBatch *******************************************************
**
** Run an EMBOSS program detached
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] cline [jstring] command line
** @param [r] direct [jstring] directory to create
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_forkBatch
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring cline, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jcl   = NULL;
    char   *jdir  = NULL;

    AjPStr commandline = NULL;
    AjPStr directory   = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;

    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    commandline = ajStrNew();
    directory   = ajStrNew();

    jvc = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);


    jcl = (char *) (*env)->GetStringUTFChars(env,cline,0);

    if(jcl)
	ajStrAssignC(&commandline,jcl);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,cline,jcl);

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    ok = ajFalse;

    if(!java_jembossctl(BATCH_FORK,username,password,envi,commandline,
			directory,&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&commandline,
		      &outstd,&errstd);
    ajStrDel(&directory);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.forkEmboss ******************************************************
**
** Run an EMBOSS program
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] cline [jstring] command line
** @param [r] direct [jstring] directory to create
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_forkEmboss
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring cline, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jcl   = NULL;
    char   *jdir  = NULL;

    AjPStr commandline = NULL;
    AjPStr directory   = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    commandline = ajStrNew();
    directory   = ajStrNew();

    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);
    jvc  = (*env)->GetObjectClass(env,obj);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);


    jcl = (char *) (*env)->GetStringUTFChars(env,cline,0);

    if(jcl)
	ajStrAssignC(&commandline,jcl);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,cline,jcl);

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }
    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&commandline,
			  &outstd,&errstd);
	ajStrDel(&directory);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    ok = ajFalse;

    if(!java_jembossctl(EMBOSS_FORK,username,password,envi,commandline,
			directory,&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&commandline,
		      &outstd,&errstd);
    ajStrDel(&directory);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.makeDir *********************************************************
**
** Create a directory
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to create
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_makeDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jdir  = NULL;

    AjPStr directory = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    directory  = ajStrNew();

    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);

    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,direct,jdir);

    ok = ajFalse;

    if(!java_jembossctl(MAKE_DIRECTORY,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&directory,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.delFile *********************************************************
**
** Delete a file
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] file to delete
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_delFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;

    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;

    AjPStr file=NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();

    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;

    if(!java_jembossctl(DELETE_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.renameFile ******************************************************
**
** Rename a file
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] old filename
** @param [r] filename2 [jstring] new filename
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_renameFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename, jstring filename2)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;
    char   *jfil2 = NULL;

    AjPStr file  = NULL;
    AjPStr file2 = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;

    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();


    jvc = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename,jfil);


    file2 = ajStrNew();

    jfil2 = (char *) (*env)->GetStringUTFChars(env,filename2,0);

    if(jfil2)
	ajStrAssignC(&file2,jfil2);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	ajStrDel(&file2);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename2,jfil2);



    ok = ajFalse;
    if(!java_jembossctl(RENAME_FILE,username,password,envi,file,file2,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    ajStrDel(&file2);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.delDir **********************************************************
**
** Recursively delete a directory
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to delete
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_delDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jdir  = NULL;

    AjPStr directory = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    directory  = ajStrNew();


    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    ok = ajFalse;

    if(!java_jembossctl(DELETE_DIR,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&directory,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.listFiles *******************************************************
**
** Lists regular files
** Loads outStd,errStd within java. outStd contains the list
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to scan
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_listFiles
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jdir  = NULL;

    AjPStr directory = NULL;

    AjBool ok=ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    directory  = ajStrNew();


    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);


    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);



    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);
	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    ok = ajFalse;

    if(!java_jembossctl(LIST_FILES,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&directory,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.listDirs ********************************************************
**
** Lists files of type 'directory'
** Loads outStd,errStd within java. outStd contains the list
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to scan
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_listDirs
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jdir  = NULL;

    AjPStr directory = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    directory  = ajStrNew();

    jvc = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);

    if(jdir)
	ajStrAssignC(&directory,jdir);
    else
    {
	java_tidy_command(&username,&password,&envi,&directory,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }
    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    ok = ajFalse;

    if(!java_jembossctl(LIST_DIRS,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    if(ajStrGetLen(errstd))
	ok = ajFalse;

    java_tidy_command(&username,&password,&envi,&directory,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.getFile *********************************************************
**
** Get a user file
** Loads outStd,errStd within java
** Sets size (of file),fileok (success) & prnt ([non]printable]
** variables in java
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] filename to get
**
** @return [jbyteArray] contents of file
** @@
******************************************************************************/


JNIEXPORT jbyteArray JNICALL Java_org_emboss_jemboss_parser_Ajax_getFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;

    jclass jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    jbyteArray jb;

    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;

    AjPStr file=NULL;
    unsigned char *fbuf = NULL;
    int size   = 0;
    int prnt   = 1;
    int fileok = 0;
    int c;
    int i;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();


    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);

    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return NULL;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);

    (*env)->ReleaseStringUTFChars(env,environment,jenv);


    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);

    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;

    if(!java_jembossctl(GET_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,&fbuf,&size))
	ok = ajTrue;



    fileok = 0;

    if(ok)
	fileok=1;

    if(!size)
    {
	fileok = 0;
	AJCNEW0(fbuf,1);
    }


    for(i=0;i<size;++i)
    {
	c = (int)fbuf[i];

	if(c==(int)'\n' || c==(int)'\t' || c==(int)'\r')
	    continue;

	if(!isprint(c))
	{
	    prnt=0;
	    break;
	}
    }



    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    field = (*env)->GetFieldID(env,jvc,"prnt","I");
    (*env)->SetIntField(env,obj,field,prnt);


    field = (*env)->GetFieldID(env,jvc,"size","I");
    (*env)->SetIntField(env,obj,field,size);



    field = (*env)->GetFieldID(env,jvc,"fileok","I");
    (*env)->SetIntField(env,obj,field,fileok);

    jb = (*env)->NewByteArray(env,size);
    (*env)->SetByteArrayRegion(env,jb,0,size,(jbyte *)fbuf);



    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    AJFREE(jpass);
    AJFREE(fbuf);

    return jb;
}




/* @func Ajax.putFile *********************************************************
**
** Write a user file
** Loads outStd,errStd within java
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] filename to create
** @param [r] arr [jbyteArray] contents for the file
**
** @return [jboolean] true if the file was written
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_putFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename, jbyteArray arr)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    jsize    len;
    jbyte    *ba;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;

    AjPStr file = NULL;
    unsigned char *fbuf;
    int size;
    int i;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;

    len = (*env)->GetArrayLength(env,arr);
    ba  = (*env)->GetByteArrayElements(env,arr,0);

    size = len;

    if(size)
	if(!(fbuf=(unsigned char *)malloc(size)))
	    return (unsigned char)ajFalse;

    for(i=0;i<size;++i)
	fbuf[i] = (unsigned char)ba[i];

    (*env)->ReleaseByteArrayElements(env,arr,ba,0);

    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();

    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);


    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	if(size)
	    AJFREE(fbuf);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	if(size)
	    AJFREE(fbuf);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	if(size)
	    AJFREE(fbuf);

	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	if(size)
	    AJFREE(fbuf);

	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	if(size)
	    AJFREE(fbuf);

	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename,jfil);


    ok = ajFalse;
    if(!java_jembossctl(PUT_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,&fbuf,&size))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    if(size)
	AJFREE(fbuf);

    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    AJFREE(jpass);


    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.seqAttrib *******************************************************
**
** Return sequence attributes
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] file to delete
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqAttrib
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;

    AjPStr file = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;
    ajint len    = 0;
    float weight = 0.;
    AjBool nuc = ajFalse;


    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();

    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;

    if(!java_jembossctl(SEQ_ATTRIB,username,password,envi,file,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;



    /* Results are returned via a stdout string in outStd */
    ajFmtScanS(outstd,"%d%f%B",&len,&weight,&nuc);

    field = (*env)->GetFieldID(env,jvc,"length_soap","I");
    (*env)->SetIntField(env,obj,field,len);

    field = (*env)->GetFieldID(env,jvc,"protein_soap","Z");
    (*env)->SetBooleanField(env,obj,field,(unsigned char)!nuc);

    field = (*env)->GetFieldID(env,jvc,"weight_soap","F");
    (*env)->SetFloatField(env,obj,field,weight);

    ajStrAssignClear(&outstd);

    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @func Ajax.seqsetAttrib ****************************************************
**
** Return seqset attributes
** Loads outStd,errStd within java.
**
** @param [u] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jbyteArray] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] usa
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqsetAttrib
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username = NULL;
    AjPStr password = NULL;
    AjPStr envi     = NULL;
    AjPStr outstd   = NULL;
    AjPStr errstd   = NULL;


    jclass   jvc;
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser = NULL;
    char   *jpass = NULL;
    char   *jenv  = NULL;
    char   *jfil  = NULL;

    AjPStr file = NULL;

    AjBool ok = ajFalse;
    jsize  plen;
    jbyte  *ca;
    int i;
    ajint  len    = 0;
    float  weight = 0.;
    AjBool nuc = ajFalse;

    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();
    outstd   = ajStrNew();
    errstd   = ajStrNew();

    file     = ajStrNew();


    jvc  = (*env)->GetObjectClass(env,obj);
    plen = (*env)->GetArrayLength(env,key);
    ca   = (*env)->GetByteArrayElements(env,key,0);

    juser = (char *) (*env)->GetStringUTFChars(env,door,0);

    if(juser)
	ajStrAssignC(&username,juser);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);

	return (unsigned char)ajFalse;
    }

    bzero((void *)jpass,plen+1);

    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];

    ajStrAssignC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);

    if(jenv)
	ajStrAssignC(&envi,jenv);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrGetLen(username) || !ajStrGetLen(password) || !ajStrGetLen(envi))
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);

    if(jfil)
	ajStrAssignC(&file,jfil);
    else
    {
	java_tidy_command(&username,&password,&envi,&file,
			  &outstd,&errstd);
	AJFREE(jpass);

	return (unsigned char)ajFalse;
    }

    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;

    if(!java_jembossctl(SEQSET_ATTRIB,username,password,envi,file,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;



    /* Results are returned via a stdout string in outStd */
    ajFmtScanS(outstd,"%d%f%B",&len,&weight,&nuc);

    field = (*env)->GetFieldID(env,jvc,"length_soap","I");
    (*env)->SetIntField(env,obj,field,len);

    field = (*env)->GetFieldID(env,jvc,"protein_soap","Z");
    (*env)->SetBooleanField(env,obj,field,(unsigned char)!nuc);

    field = (*env)->GetFieldID(env,jvc,"weight_soap","F");
    (*env)->SetFloatField(env,obj,field,weight);

    ajStrAssignClear(&outstd);


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");
    ostr = (*env)->NewStringUTF(env,ajStrGetPtr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrGetPtr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    java_tidy_command(&username,&password,&envi,&file,
		      &outstd,&errstd);
    AJFREE(jpass);

    if(!ok)
	return (unsigned char)ajFalse;

    return (unsigned char)ajTrue;
}




/* @funcstatic java_pipe_write ************************************************
**
** Write a byte stream down a file desriptor (unblocked)
**
** @param [r] tchan [int] file descriptor
** @param [r] buf [const char *] buffer to write
** @param [r] n [int] number of bytes to write
** @param [r] seconds [int] time-out
** @param [w] errstd [AjPStr*] stderr
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_pipe_write(int tchan, const char *buf, int n, int seconds,
			   AjPStr *errstd)
{
#ifdef HAVE_POLL
    struct pollfd ufds;
    unsigned int  nfds;
#else
    fd_set fdr;
    fd_set fdw;
    struct timeval tfd;
#endif

    int  written;
    int  sent = 0;
    int  ret  = 0;
    const char *p;
    unsigned long block = 0;
    long then = 0;
    long now  = 0;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    then = tv.tv_sec;


    block = 1;

    if(java_block(tchan,block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 9a. %d\n",errno);

	return -1;
    }


    p = buf;
    written = 0;

#ifdef HAVE_POLL
    while(written!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;

	if(now-then >= seconds)
	{
	    ajStrAppendC(errstd,"java_pipe_write timeout\n");

	    return -1;
	}

	/* Check pipe is writeable */

	ufds.fd = tchan;
	ufds.events = POLLOUT;
	nfds = 1;
	ret=poll(&ufds,nfds,1);

	if(ret && ret!=-1 && (ufds.revents & POLLOUT))
	{
	    while((sent=write(tchan,p,n-(p-buf)))==-1 && errno==EINTR);

	    if(sent == -1)
	    {
		ajStrAppendC(errstd,"java_pipe_write send error\n");

		return -1;
	    }

	    written += sent;
	    p += sent;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#else
    while(written!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;

	if(now-then >= seconds)
	{
	    ajStrAppendC(errstd,"java_pipe_write timeout\n");

	    return -1;
	}

	/* Check pipe is writeable */
	tfd.tv_sec  = 0;
	tfd.tv_usec = 1000;
	FD_ZERO(&fdw);
	FD_SET(tchan,&fdw);
	fdr = fdw;

	ret = select(tchan+1,&fdr,&fdw,NULL,&tfd);

	if(ret && ret!=-1 && FD_ISSET(tchan,&fdw))
	{
	    while((sent=write(tchan,p,n-(p-buf)))==-1 && errno==EINTR);

	    if(sent == -1)
	    {
		ajStrAppendC(errstd,"java_pipe_write send error\n");

		return -1;
	    }

	    written += sent;
	    p += sent;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#endif

    block = 0;

    if(java_block(tchan,block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot block 10a. %d\n",errno);

	return -1;
    }

    return 0;
}




/* @funcstatic java_pipe_read *************************************************
**
** Read a byte stream from a file desriptor (unblocked)
**
** @param [r] rchan [int] file descriptor
** @param [w] buf [char *] buffer for read
** @param [r] n [int] number of bytes to read
** @param [r] seconds [int] time-out
** @param [w] errstd [AjPStr*] stderr
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_pipe_read(int rchan, char *buf, int n, int seconds,
			  AjPStr *errstd)
{
#ifdef HAVE_POLL
    struct pollfd ufds;
    unsigned int  nfds;
#else
    fd_set fdr;
    fd_set fdw;
    struct timeval tfd;
#endif

    int  sum;
    int  got = 0;
    int  ret = 0;
    char *p;
    unsigned long block = 0;
    long then = 0;
    long now  = 0;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    then = tv.tv_sec;


    block = 1;

    if(java_block(rchan,block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 11a. %d\n",errno);

	return -1;
    }

    p = buf;
    sum = 0;

#ifdef HAVE_POLL
    while(sum!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;

	if(now-then >= seconds)
	{
	    ajStrAppendC(errstd,"java_pipe_read timeout\n");

	    return -1;
	}

	/* Check pipe is readable */
	ufds.fd = rchan;
	ufds.events = POLLIN | POLLPRI;
	nfds = 1;

	ret=poll(&ufds,nfds,1);

	if(ret && ret!=-1)
	{
	    if((ufds.revents & POLLIN) || (ufds.revents & POLLPRI))
	    {
		while((got=read(rchan,p,n-(p-buf)))==-1 && errno==EINTR);

		if(got == -1)
		{
		    ajStrAppendC(errstd,"java_pipe_read read error\n");

		    return -1;
		}

		sum += got;
		p += got;
		gettimeofday(&tv,NULL);
		then = tv.tv_sec;
	    }
	}

    }
#else
    while(sum!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;

	if(now-then >= seconds)
	{
	    ajStrAppendC(errstd,"java_pipe_read timeout\n");

	    return -1;
	}

	/* Check pipe is readable */
	tfd.tv_sec  = 0;
	tfd.tv_usec = 1000;
	FD_ZERO(&fdr);
	FD_SET(rchan,&fdr);
	fdw = fdr;

	ret = select(rchan+1,&fdr,&fdw,NULL,&tfd);

	if(ret && ret!=-1 && FD_ISSET(rchan,&fdr))
	{
	    while((got=read(rchan,p,n-(p-buf)))==-1 && errno==EINTR);

	    if(got == -1)
	    {
		ajStrAppendC(errstd,"java_pipe_read read error\n");

		return -1;
	    }

	    sum += got;
	    p += got;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#endif

    block = 0;

    if(java_block(rchan,block)==-1)
    {
	ajFmtPrintAppS(errstd,"Cannot unblock 12a. %d\n",errno);

	return -1;
    }

    return 0;
}




/* @funcstatic java_snd *******************************************************
**
** Mimic socket write using pipes
**
** @param [r] tchan [int] file descriptor
** @param [r] buf [const char *] buffer to write
** @param [r] len [int] number of bytes to write
** @param [w] errstd [AjPStr*] stderr
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_snd(int tchan,const char *buf,int len,AjPStr *errstd)
{

    if(java_pipe_write(tchan,(char *)&len,sizeof(int),TIMEOUT,errstd)==-1)
    {
	ajStrAppendC(errstd,"java_snd error\n");

	return -1;
    }

    if(java_pipe_write(tchan,buf,len,TIMEOUT,errstd)==-1)
    {
	ajStrAppendC(errstd,"java_snd error\n");

	return -1;
    }

    return 0;
}




/* @funcstatic java_rcv *******************************************************
**
** Mimic socket read using pipes
**
** @param [r] rchan [int] file descriptor
** @param [w] buf [char *] buffer to write
** @param [w] errstd [AjPStr*] stderr
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_rcv(int rchan, char *buf, AjPStr *errstd)
{
    int len;

    if(java_pipe_read(rchan,(char *)&len,sizeof(int),TIMEOUT,errstd)==-1)
    {
	ajStrAppendC(errstd,"java_rcv error\n");

	return -1;
    }

    if(java_pipe_read(rchan,buf,len,TIMEOUT,errstd)==-1)
    {
	ajStrAppendC(errstd,"java_rcv error\n");

	return -1;
    }

    return len;
}




/* @funcstatic java_block *****************************************************
**
** File descriptor block/unblock
**
** @param [r] chan [int] file descriptor
** @param [r] flag [unsigned long] block=1 unblock=0
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_block(int chan, unsigned long flag)
{

    if(ioctl(chan,FIONBIO,&flag)==-1)
    {
#ifdef __sgi
	if(errno==ENOSYS)
	    return 0;
#endif
#ifdef __hpux
	if(errno==ENOTTY)
	    return 0;
#endif
	return -1;
    }

    return 0;
}

#endif /* WIN32 */

#endif

#endif /* HAVE_JAVA */
