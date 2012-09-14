/* @include ajjava ************************************************************
**
** AJAX Java Native Interface (JNI) functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version $Revision: 1.18 $
** @modified Jul 07 2001 ajb First version
** @modified Mar 02 2002 ajb Added Jemboss suid authorisation functions
** @modified $Date: 2011/10/05 12:12:10 $ by $Author: ajb $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJJAVA_H
#define AJJAVA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

AJ_BEGIN_DECLS

#ifdef HAVE_JAVA

#ifdef HAVE_TARGETCONFIG_H
#include <TargetConfig.h>
#endif /* HAVE_TARGETCONFIG_H */

#include "jni.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define JBUFFLEN 10000

#define COMM_AUTH      1
#define EMBOSS_FORK    2
#define MAKE_DIRECTORY 3
#define DELETE_FILE    4
#define DELETE_DIR     5
#define LIST_FILES     6
#define LIST_DIRS      7
#define GET_FILE       8
#define PUT_FILE       9
#define BATCH_FORK     10
#define RENAME_FILE    11
#define SEQ_ATTRIB     12
#define SEQSET_ATTRIB  13




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqType(
    JNIEnv *, jobject, jstring);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqsetType(
    JNIEnv *env, jobject obj, jstring usa);

/*
** End of prototype definitions
*/

/*
** The above prototypes are marked for Windows. Below are purely UNIX
** prototypes and therefore are not included between the above marks.
**/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_userInfo(
    JNIEnv *env, jobject obj, jstring door, jstring key);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setuid(
    JNIEnv *env, jclass j, jint uid);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_seteuid(
    JNIEnv *env, jclass j, jint uid);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setgid(
    JNIEnv *env, jclass j, jint gid);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setegid(
    JNIEnv *env, jclass j, jint gid);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getuid(
    JNIEnv *env, jclass j);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_geteuid(
    JNIEnv *env, jclass j);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getgid(
    JNIEnv *env, jclass j);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getegid(
    JNIEnv *env, jclass j);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_fork(
    JNIEnv *env, jobject obj, jstring commandline,
    jstring environment, jstring directory, jint uid, jint gid);

/* Jembossctl functions */

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_userAuth(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_forkBatch(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring cline, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_forkEmboss(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring cline, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_makeDir(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_delFile(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_renameFile(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename, jstring filename2);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_delDir(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_listFiles(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_listDirs(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring direct);

JNIEXPORT jbyteArray JNICALL Java_org_emboss_jemboss_server_Ajax_getFile(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_putFile(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename, jbyteArray arr);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqAttrib(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqsetAttrib(
    JNIEnv *env, jobject obj, jstring door, jbyteArray key,
    jstring environment, jstring filename);

AJ_END_DECLS

#endif /* HAVE_JAVA */
#endif /* !AJJAVA_H */
