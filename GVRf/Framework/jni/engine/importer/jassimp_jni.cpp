
#include "jassimp.h"

#include "util/gvr_jni.h"

namespace gvr {
extern "C" {
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getVKeysize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getQKeysize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getV3Dsize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getfloatsize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getintsize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getuintsize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getdoublesize
  (JNIEnv *env, jclass cls);
JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getlongsize
  (JNIEnv *env, jclass cls);
JNIEXPORT jstring JNICALL Java_org_util_jassimp_Jassimp_getErrorString
  (JNIEnv *env, jclass cls);
JNIEXPORT jobject JNICALL Java_org_util_jassimp_Jassimp_aiImportFile
  (JNIEnv *env, jclass cls, jstring filename, jlong post_process);
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getVKeysize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(aiVectorKey);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getQKeysize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(aiQuatKey);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getV3Dsize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(aiVector3D);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getfloatsize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(float);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getintsize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(int);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getuintsize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(unsigned int);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getdoublesize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(double);
    return res;
}

JNIEXPORT jint JNICALL Java_org_util_jassimp_Jassimp_getlongsize
  (JNIEnv *env, jclass cls)
{
    const int res = sizeof(long);
    return res;
}

JNIEXPORT jstring JNICALL Java_org_util_jassimp_Jassimp_getErrorString
  (JNIEnv *env, jclass cls)
{
    const char *err = aiGetErrorString();

    if (NULL == err)
    {
        return env->NewStringUTF("");
    }

    return env->NewStringUTF(err);
}


JNIEXPORT jobject JNICALL Java_org_util_jassimp_Jassimp_aiImportFile
  (JNIEnv *env, jclass cls, jstring filename, jlong post_process)
{
    jobject jScene = NULL;

    /* convert params */
    const char* cFilename = env->GetStringUTFChars(filename, NULL);


    LOGE("opening file: %s\n", cFilename);

    /* do import */
    const aiScene *cScene = aiImportFile(cFilename, (unsigned int) post_process);

    if (!cScene)
    {
        LOGE("import file returned null\n");
        goto error;
    }

    if (!Jassimp::createInstance(env, "org/util/jassimp/AiScene", jScene))
    {
        goto error;
    }

    if (!Jassimp::loadMeshes(env, cScene, jScene))
    {
        goto error;
    }

    if (!Jassimp::loadMaterials(env, cScene, jScene))
    {
        goto error;
    }

    if (!Jassimp::loadAnimations(env, cScene, jScene))
    {
        goto error;
    }

    if (!Jassimp::loadLights(env, cScene, jScene))
    {
        goto error;
    }

    if (!Jassimp::loadCameras(env, cScene, jScene))
    {
        goto error;
    }

    if (!Jassimp::loadSceneGraph(env, cScene, jScene))
    {
        goto error;
    }

    /* jump over error handling section */
    goto end;

error:
    {
    jclass exception = env->FindClass("java/io/IOException");

    if (NULL == exception)
    {
        /* thats really a problem because we cannot throw in this case */
        env->FatalError("could not throw java.io.IOException");
    }

    env->ThrowNew(exception, aiGetErrorString());

    LOGE("problem detected\n");
    }

end:
    /*
     * NOTE: this releases all memory used in the native domain.
     * Ensure all data has been passed to java before!
     */
    aiReleaseImport(cScene);


    /* free params */
    env->ReleaseStringUTFChars(filename, cFilename);

    LOGE("return from native\n");

    return jScene;
}

}
