#ifndef ajjava_h
#define ajjava_h

#ifdef HAVE_JAVA



#ifdef __cplusplus
extern "C" {
#endif


#ifdef HAVE_TARGETCONFIG_H
#include <TargetConfig.h>
#endif /* HAVE_TARGETCONFIG_H */
    
#include "jni.h"



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

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqType
    (JNIEnv *, jobject, jstring);
JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqsetType
    (JNIEnv *env, jobject obj, jstring usa);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_userInfo
    (JNIEnv *env, jobject obj, jstring door, jstring key);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setuid
    (JNIEnv *env, jclass j, jint uid);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_seteuid
    (JNIEnv *env, jclass j, jint uid);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setgid
    (JNIEnv *env, jclass j, jint gid);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_setegid
    (JNIEnv *env, jclass j, jint gid);

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getuid
    (JNIEnv *env, jclass j);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_geteuid
    (JNIEnv *env, jclass j);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getgid
    (JNIEnv *env, jclass j);
JNIEXPORT jint JNICALL Java_org_emboss_jemboss_server_Ajax_getegid
    (JNIEnv *env, jclass j);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_fork
(JNIEnv *env, jobject obj, jstring commandline,
 jstring environment, jstring directory, jint uid, jint gid);


/* Jembossctl functions */

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_userAuth
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_forkBatch
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring cline, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_forkEmboss
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring cline, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_makeDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_delFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_renameFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename, jstring filename2);

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_delDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_listFiles
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_listDirs
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct);


JNIEXPORT jbyteArray JNICALL Java_org_emboss_jemboss_server_Ajax_getFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_putFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename, jbyteArray arr);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqAttrib
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename);


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_server_Ajax_seqsetAttrib
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename);



#ifdef __cplusplus
}
#endif
#endif
#endif
