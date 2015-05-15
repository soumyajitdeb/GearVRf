#include "jassimp.h"

namespace gvr {

bool Jassimp::createInstance(JNIEnv *env, const char* class_name, jobject& new_instance)
{
    jclass clazz = env->FindClass(class_name);

    if (NULL == clazz)
    {
        LOGE("could not find class %s\n", class_name);
        return false;
    }

    jmethodID ctr_id = env->GetMethodID(clazz, "<init>", "()V");

    if (NULL == ctr_id)
    {
        LOGE("could not find no-arg constructor for class %s\n", class_name);
        return false;
    }

    new_instance = env->NewObject(clazz, ctr_id);
    env->DeleteLocalRef(clazz);

    if (NULL == new_instance)
    {
        LOGE("error calling no-arg constructor for class %s\n", class_name);
        return false;
    }

    return true;
}


bool Jassimp::createInstance(JNIEnv *env, const char* class_name, const char* signature,/* const*/ jvalue* params, jobject& new_instance)
{
    jclass clazz = env->FindClass(class_name);

    if (NULL == clazz)
    {
        LOGE("could not find class %s\n", class_name);
        return false;
    }

    jmethodID ctr_id = env->GetMethodID(clazz, "<init>", signature);

    if (NULL == ctr_id)
    {
        LOGE("could not find no-arg constructor for class %s\n", class_name);
        return false;
    }

    new_instance = env->NewObjectA(clazz, ctr_id, params);
    env->DeleteLocalRef(clazz);

    if (NULL == new_instance)
    {
        LOGE("error calling  constructor for class %s, signature %s\n", class_name, signature);
        return false;
    }

    return true;
}


bool Jassimp::getField(JNIEnv *env, jobject object, const char* fieldName, const char* signature, jobject& field)
{
    jclass clazz = env->GetObjectClass(object);

    if (NULL == clazz)
    {
        LOGE("could not get class for object\n");
        return false;
    }

    jfieldID fieldId = env->GetFieldID(clazz, fieldName, signature);
    env->DeleteLocalRef(clazz);

    if (NULL == fieldId)
    {
        LOGE("could not get field %s with signature %s\n", fieldName, signature);
        return false;
    }

    field = env->GetObjectField(object, fieldId);

    return true;
}


bool Jassimp::setIntField(JNIEnv *env, jobject object, const char* fieldName, jint value)
{
    jclass clazz = env->GetObjectClass(object);

    if (NULL == clazz)
    {
        LOGE("could not get class for object\n");
        return false;
    }

    jfieldID fieldId = env->GetFieldID(clazz, fieldName, "I");
    env->DeleteLocalRef(clazz);

    if (NULL == fieldId)
    {
        LOGE("could not get field %s with signature I\n", fieldName);
        return false;
    }

    env->SetIntField(object, fieldId, value);

    return true;
}


bool Jassimp::setFloatField(JNIEnv *env, jobject object, const char* fieldName, jfloat value)
{
    jclass clazz = env->GetObjectClass(object);

    if (NULL == clazz)
    {
        LOGE("could not get class for object\n");
        return false;
    }

    jfieldID fieldId = env->GetFieldID(clazz, fieldName, "F");
    env->DeleteLocalRef(clazz);

    if (NULL == fieldId)
    {
        LOGE("could not get field %s with signature F\n", fieldName);
        return false;
    }

    env->SetFloatField(object, fieldId, value);

    return true;
}


bool Jassimp::setObjectField(JNIEnv *env, jobject object, const char* fieldName, const char* signature, jobject value)
{
    jclass clazz = env->GetObjectClass(object);

    if (NULL == clazz)
    {
        LOGE("could not get class for object\n");
        return false;
    }

    jfieldID fieldId = env->GetFieldID(clazz, fieldName, signature);
    env->DeleteLocalRef(clazz);

    if (NULL == fieldId)
    {
        LOGE("could not get field %s with signature %s\n", fieldName, signature);
        return false;
    }

    env->SetObjectField(object, fieldId, value);

    return true;
}


bool Jassimp::getStaticField(JNIEnv *env, const char* class_name, const char* fieldName, const char* signature, jobject& field)
{
    jclass clazz = env->FindClass(class_name);

    if (NULL == clazz)
    {
        LOGE("could not find class %s\n", class_name);
        return false;
    }

    jfieldID fieldId = env->GetFieldID(clazz, fieldName, signature);

    if (NULL == fieldId)
    {
        LOGE("could not get field %s with signature %s\n", fieldName, signature);
        return false;
    }

    field = env->GetStaticObjectField(clazz, fieldId);

    return true;
}

bool Jassimp::call(JNIEnv *env, jobject object, const char* typeName, const char* methodName, const char* signature,/* const*/ jvalue* params)
{
    jclass clazz = env->FindClass(typeName);

    if (NULL == clazz)
    {
        LOGE("could not find class %s\n", typeName);
        return false;
    }

    jmethodID mid = env->GetMethodID(clazz, methodName, signature);
    env->DeleteLocalRef(clazz);

    if (NULL == mid)
    {
        LOGE("could not find method %s with signature %s in type %s\n", methodName, signature, typeName);
        return false;
    }

    jboolean jReturnValue = env->CallBooleanMethod(object, mid, params[0].l);

    return (bool)jReturnValue;
}

bool Jassimp::callv(JNIEnv *env, jobject object, const char* typeName, const char* methodName, const char* signature,/* const*/ jvalue* params)
{
    jclass clazz = env->FindClass(typeName);

    if (NULL == clazz) {
        LOGE("could not find class %s\n", typeName);
        return false;
    }

    jmethodID mid = env->GetMethodID(clazz, methodName, signature);
    env->DeleteLocalRef(clazz);

    if (NULL == mid) {
        LOGE("could not find method %s with signature %s in type %s\n", methodName, signature, typeName);
        return false;
    }

    env->CallVoidMethodA(object, mid, params);

    return true;
}

bool Jassimp::callStaticObject(JNIEnv *env, const char* typeName, const char* methodName, const char* signature,/* const*/ jvalue* params, jobject& returnValue)
{
    jclass clazz = env->FindClass(typeName);

    if (NULL == clazz)
    {
        LOGE("could not find class %s\n", typeName);
        return false;
    }

    jmethodID mid = env->GetStaticMethodID(clazz, methodName, signature);

    if (NULL == mid)
    {
        LOGE("could not find method %s with signature %s in type %s\n", methodName, signature, typeName);
        return false;
    }

    returnValue = env->CallStaticObjectMethodA(clazz, mid, params);

    return true;
}

bool Jassimp::copyBuffer(JNIEnv *env, jobject jMesh, const char* jBufferName, void* cData, size_t size)
{
    jobject jBuffer = NULL;

    if (!getField(env, jMesh, jBufferName, "Ljava/nio/ByteBuffer;", jBuffer))
    {
        return false;
    }

    if (env->GetDirectBufferCapacity(jBuffer) != size)
    {
        LOGE("invalid direct buffer, expected %u, got %llu\n", size, env->GetDirectBufferCapacity(jBuffer));
        return false;
    }

    void* jBufferPtr = env->GetDirectBufferAddress(jBuffer);

    if (NULL == jBufferPtr)
    {
        LOGE("could not access direct buffer\n");
        return false;
    }

    memcpy(jBufferPtr, cData, size);

    return true;
}

bool Jassimp::copyBufferArray(JNIEnv *env, jobject jMesh, const char* jBufferName, int index, void* cData, size_t size)
{
    jobject jBufferArray = NULL;

    if (!getField(env, jMesh, jBufferName, "[Ljava/nio/ByteBuffer;", jBufferArray))
    {
        return false;
    }

    jobject jBuffer = env->GetObjectArrayElement((jobjectArray) jBufferArray, index);

    if (env->GetDirectBufferCapacity(jBuffer) != size)
    {
        LOGE("invalid direct buffer, expected %u, got %llu\n", size, env->GetDirectBufferCapacity(jBuffer));
        return false;
    }

    void* jBufferPtr = env->GetDirectBufferAddress(jBuffer);

    if (NULL == jBufferPtr)
    {
        LOGE("could not access direct buffer\n");
        return false;
    }

    memcpy(jBufferPtr, cData, size);

    return true;
}

bool Jassimp::loadMeshes(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    for (unsigned int meshNr = 0; meshNr < cScene->mNumMeshes; meshNr++)
    {
        const aiMesh *cMesh = cScene->mMeshes[meshNr];

        LOGE("converting mesh %s ...\n", cMesh->mName.C_Str());

        /* create mesh */
        jobject jMesh = NULL;

        if (!createInstance(env, "org/util/jassimp/AiMesh", jMesh))
        {
            return false;
        }


        /* add mesh to m_meshes java.util.List */
        jobject jMeshes = NULL;

        if (!getField(env, jScene, "m_meshes", "Ljava/util/List;", jMeshes))
        {
            return false;
        }

        jvalue addParams[1];
        addParams[0].l = jMesh;
        if (!call(env, jMeshes, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
        {
            return false;
        }


        /* set general mesh data in java */
        jvalue setTypesParams[1];
        setTypesParams[0].i = cMesh->mPrimitiveTypes;
        if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "setPrimitiveTypes", "(I)V", setTypesParams))
        {
            return false;
        }


        if (!setIntField(env, jMesh, "m_materialIndex", cMesh->mMaterialIndex))
        {
            return false;
        }

        if (!setObjectField(env, jMesh, "m_name", "Ljava/lang/String;", env->NewStringUTF(cMesh->mName.C_Str())))
        {
            return false;
        }


        /* determine face buffer size */
        bool isPureTriangle = cMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE;
        size_t faceBufferSize;
        if (isPureTriangle)
        {
            faceBufferSize = cMesh->mNumFaces * 3 * sizeof(unsigned int);
        }
        else
        {
            int numVertexReferences = 0;
            for (unsigned int face = 0; face < cMesh->mNumFaces; face++)
            {
                numVertexReferences += cMesh->mFaces[face].mNumIndices;
            }

            faceBufferSize = numVertexReferences * sizeof(unsigned int);
        }


        /* allocate buffers - we do this from java so they can be garbage collected */
        jvalue allocateBuffersParams[4];
        allocateBuffersParams[0].i = cMesh->mNumVertices;
        allocateBuffersParams[1].i = cMesh->mNumFaces;
        allocateBuffersParams[2].z = isPureTriangle;
        allocateBuffersParams[3].i = (jint) faceBufferSize;
        if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateBuffers", "(IIZI)V", allocateBuffersParams))
        {
            return false;
        }


        if (cMesh->mNumVertices > 0)
        {
            /* push vertex data to java */
            if (!copyBuffer(env, jMesh, "m_vertices", cMesh->mVertices, cMesh->mNumVertices * sizeof(aiVector3D)))
            {
                LOGE("could not copy vertex data\n");
                return false;
            }

            LOGE("    with %u vertices\n", cMesh->mNumVertices);
        }


        /* push face data to java */
        if (cMesh->mNumFaces > 0)
        {
            if (isPureTriangle)
            {
                char* faceBuffer = (char*) malloc(faceBufferSize);

                size_t faceDataSize = 3 * sizeof(unsigned int);
                for (unsigned int face = 0; face < cMesh->mNumFaces; face++)
                {
                    memcpy(faceBuffer + face * faceDataSize, cMesh->mFaces[face].mIndices, faceDataSize);
                }

                bool res = copyBuffer(env, jMesh, "m_faces", faceBuffer, faceBufferSize);

                free(faceBuffer);

                if (!res)
                {
                    LOGE("could not copy face data\n");
                    return false;
                }
            }
            else
            {
                char* faceBuffer = (char*) malloc(faceBufferSize);
                char* offsetBuffer = (char*) malloc(cMesh->mNumFaces * sizeof(unsigned int));

                size_t faceBufferPos = 0;
                for (unsigned int face = 0; face < cMesh->mNumFaces; face++)
                {
                    size_t faceBufferOffset = faceBufferPos / sizeof(unsigned int);
                    memcpy(offsetBuffer + face * sizeof(unsigned int), &faceBufferOffset, sizeof(unsigned int));

                    size_t faceDataSize = cMesh->mFaces[face].mNumIndices * sizeof(unsigned int);
                    memcpy(faceBuffer + faceBufferPos, cMesh->mFaces[face].mIndices, faceDataSize);
                    faceBufferPos += faceDataSize;
                }

                if (faceBufferPos != faceBufferSize)
                {
                    /* this should really not happen */
                    LOGE("faceBufferPos %u, faceBufferSize %u\n", faceBufferPos, faceBufferSize);
                    env->FatalError("error copying face data");
                    exit(-1);
                }


                bool res = copyBuffer(env, jMesh, "m_faces", faceBuffer, faceBufferSize);
                res &= copyBuffer(env, jMesh, "m_faceOffsets", offsetBuffer, cMesh->mNumFaces * sizeof(unsigned int));

                free(faceBuffer);
                free(offsetBuffer);

                if (!res)
                {
                    LOGE("could not copy face data\n");
                    return false;
                }
            }

            LOGE("    with %u faces\n", cMesh->mNumFaces);
        }


        /* push normals to java */
        if (cMesh->HasNormals())
        {
            jvalue allocateDataChannelParams[2];
            allocateDataChannelParams[0].i = 0;
            allocateDataChannelParams[1].i = 0;
            if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateDataChannel", "(II)V", allocateDataChannelParams))
            {
                LOGE("could not allocate normal data channel\n");
                return false;
            }
            if (!copyBuffer(env, jMesh, "m_normals", cMesh->mNormals, cMesh->mNumVertices * 3 * sizeof(float)))
            {
                LOGE("could not copy normal data\n");
                return false;
            }

            LOGE("    with normals\n");
        }


        /* push tangents to java */
        if (cMesh->mTangents != NULL)
        {
            jvalue allocateDataChannelParams[2];
            allocateDataChannelParams[0].i = 1;
            allocateDataChannelParams[1].i = 0;
            if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateDataChannel", "(II)V", allocateDataChannelParams))
            {
                LOGE("could not allocate tangents data channel\n");
                return false;
            }
            if (!copyBuffer(env, jMesh, "m_tangents", cMesh->mTangents, cMesh->mNumVertices * 3 * sizeof(float)))
            {
                LOGE("could not copy tangents data\n");
                return false;
            }

            LOGE("    with tangents\n");
        }


        /* push bitangents to java */
        if (cMesh->mBitangents != NULL)
        {
            jvalue allocateDataChannelParams[2];
            allocateDataChannelParams[0].i = 2;
            allocateDataChannelParams[1].i = 0;
            if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateDataChannel", "(II)V", allocateDataChannelParams))
            {
                LOGE("could not allocate bitangents data channel\n");
                return false;
            }
            if (!copyBuffer(env, jMesh, "m_bitangents", cMesh->mBitangents, cMesh->mNumVertices * 3 * sizeof(float)))
            {
                LOGE("could not copy bitangents data\n");
                return false;
            }

            LOGE("    with bitangents\n");
        }


        /* push color sets to java */
        for (int c = 0; c < AI_MAX_NUMBER_OF_COLOR_SETS; c++)
        {
            if (cMesh->mColors[c] != NULL)
            {
                jvalue allocateDataChannelParams[2];
                allocateDataChannelParams[0].i = 3;
                allocateDataChannelParams[1].i = c;
                if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateDataChannel", "(II)V", allocateDataChannelParams))
                {
                    LOGE("could not allocate colorset data channel\n");
                    return false;
                }
                if (!copyBufferArray(env, jMesh, "m_colorsets", c, cMesh->mColors[c], cMesh->mNumVertices * 4 * sizeof(float)))
                {
                    LOGE("could not copy colorset data\n");
                    return false;
                }

                LOGE("    with colorset[%d]\n", c);
            }
        }


        /* push tex coords to java */
        for (int c = 0; c < AI_MAX_NUMBER_OF_TEXTURECOORDS; c++)
        {
            if (cMesh->mTextureCoords[c] != NULL)
            {
                jvalue allocateDataChannelParams[2];

                switch (cMesh->mNumUVComponents[c])
                {
                case 1:
                    allocateDataChannelParams[0].i = 4;
                    break;
                case 2:
                    allocateDataChannelParams[0].i = 5;
                    break;
                case 3:
                    allocateDataChannelParams[0].i = 6;
                    break;
                default:
                    return false;
                }

                allocateDataChannelParams[1].i = c;
                if (!callv(env, jMesh, "org/util/jassimp/AiMesh", "allocateDataChannel", "(II)V", allocateDataChannelParams))
                {
                    LOGE("could not allocate texture coordinates data channel\n");
                    return false;
                }

                /* gather data */
                size_t coordBufferSize = cMesh->mNumVertices * cMesh->mNumUVComponents[c] * sizeof(float);
                char* coordBuffer = (char*) malloc(coordBufferSize);
                size_t coordBufferOffset = 0;

                for (unsigned int v = 0; v < cMesh->mNumVertices; v++)
                {
                    memcpy(coordBuffer + coordBufferOffset, &cMesh->mTextureCoords[c][v], cMesh->mNumUVComponents[c] * sizeof(float));
                    coordBufferOffset += cMesh->mNumUVComponents[c] * sizeof(float);
                }

                if (coordBufferOffset != coordBufferSize)
                {
                    /* this should really not happen */
                    LOGE("coordBufferPos %u, coordBufferSize %u\n", coordBufferOffset, coordBufferSize);
                    env->FatalError("error copying coord data");
                    exit(-1);
                }

                bool res = copyBufferArray(env, jMesh, "m_texcoords", c, coordBuffer, coordBufferSize);

                free(coordBuffer);

                if (!res)
                {
                    LOGE("could not copy texture coordinates data\n");
                    return false;
                }

                LOGE("    with %uD texcoord[%d]\n", cMesh->mNumUVComponents[c], c);
            }
        }


        for (unsigned int b = 0; b < cMesh->mNumBones; b++)
        {
            aiBone *cBone = cMesh->mBones[b];

            jobject jBone;
            if (!createInstance(env, "org/util/jassimp/AiBone", jBone))
            {
                return false;
            }

            /* add bone to bone list */
            jobject jBones = NULL;

            if (!getField(env, jMesh, "m_bones", "Ljava/util/List;", jBones))
            {
                return false;
            }

            jvalue addParams[1];
            addParams[0].l = jBone;
            if (!call(env, jBones, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
            {
                return false;
            }

            /* set bone data */
            if (!setObjectField(env, jBone, "m_name", "Ljava/lang/String;", env->NewStringUTF(cBone->mName.C_Str())))
            {
                return false;
            }

            /* add bone weights */
            for (unsigned int w = 0; w < cBone->mNumWeights; w++)
            {
                jobject jBoneWeight;
                if (!createInstance(env, "org/util/jassimp/AiBoneWeight", jBoneWeight))
                {
                    return false;
                }

                /* add boneweight to bone list */
                jobject jBoneWeights = NULL;

                if (!getField(env, jBone, "m_boneWeights", "Ljava/util/List;", jBoneWeights))
                {
                    return false;
                }


                /* copy offset matrix */
                jfloatArray jMatrixArr = env->NewFloatArray(16);
                env->SetFloatArrayRegion(jMatrixArr, 0, 16, (jfloat*) &cBone->mOffsetMatrix);

                jvalue wrapParams[1];
                wrapParams[0].l = jMatrixArr;
                jobject jMatrix;

                if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapMatrix", "([F)Ljava/lang/Object;", wrapParams, jMatrix))
                {
                    return false;
                }

                if (!setObjectField(env, jBone, "m_offsetMatrix", "Ljava/lang/Object;", jMatrix))
                {
                    return false;
                }


                jvalue addBwParams[1];
                addBwParams[0].l = jBoneWeight;
                if (!call(env, jBoneWeights, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addBwParams))
                {
                    return false;
                }


                if (!setIntField(env, jBoneWeight, "m_vertexId", cBone->mWeights[w].mVertexId))
                {
                    return false;
                }

                if (!setFloatField(env, jBoneWeight, "m_weight", cBone->mWeights[w].mWeight))
                {
                    return false;
                }
            }
        }
        env->DeleteLocalRef(jMeshes);
        env->DeleteLocalRef(jMesh);
    }

    return true;
}

bool Jassimp::loadSceneNode(JNIEnv *env, const aiNode *cNode, jobject parent, jobject* loadedNode)
{
    LOGE("   converting node %s ...\n", cNode->mName.C_Str());

    /* wrap matrix */
    jfloatArray jMatrixArr = env->NewFloatArray(16);
    env->SetFloatArrayRegion(jMatrixArr, 0, 16, (jfloat*) &cNode->mTransformation);

    jvalue wrapMatParams[1];
    wrapMatParams[0].l = jMatrixArr;
    jobject jMatrix;

    if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapMatrix", "([F)Ljava/lang/Object;", wrapMatParams, jMatrix))
    {
        return false;
    }


    /* create mesh references array */
    jintArray jMeshrefArr = env->NewIntArray(cNode->mNumMeshes);
    jint *temp = (jint*) malloc(sizeof(jint) * cNode->mNumMeshes);

    for (unsigned int i = 0; i < cNode->mNumMeshes; i++)
    {
        temp[i] = cNode->mMeshes[i];
    }
    env->SetIntArrayRegion(jMeshrefArr, 0, cNode->mNumMeshes, (jint*) temp);

    free(temp);


    /* convert name */
    jstring jNodeName = env->NewStringUTF(cNode->mName.C_Str());


    /* wrap scene node */
    jvalue wrapNodeParams[4];
    wrapNodeParams[0].l = parent;
    wrapNodeParams[1].l = jMatrix;
    wrapNodeParams[2].l = jMeshrefArr;
    wrapNodeParams[3].l = jNodeName;
    jobject jNode;
    if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapSceneNode",
        "(Ljava/lang/Object;Ljava/lang/Object;[ILjava/lang/String;)Ljava/lang/Object;", wrapNodeParams, jNode))
    {
        return false;
    }


    /* and recurse */
    for (unsigned int c = 0; c < cNode->mNumChildren; c++)
    {
        if (!loadSceneNode(env, cNode->mChildren[c], jNode))
        {
            return false;
        }
    }

    if (NULL != loadedNode)
    {
        *loadedNode = jNode;
    }

    return true;
}

bool Jassimp::loadSceneGraph(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    LOGE("converting scene graph ...\n");

    if (NULL != cScene->mRootNode)
    {
        jobject jRoot;

        if (!loadSceneNode(env, cScene->mRootNode, NULL, &jRoot))
        {
            return false;
        }

        if (!setObjectField(env, jScene, "m_sceneRoot", "Ljava/lang/Object;", jRoot))
        {
            return false;
        }
    }

    LOGE("converting scene graph finished\n");

    return true;
}

bool Jassimp::loadMaterials(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    for (unsigned int m = 0; m < cScene->mNumMaterials; m++)
    {
        const aiMaterial* cMaterial = cScene->mMaterials[m];

        LOGE("converting material %d ...\n", m);

        jobject jMaterial = NULL;

        if (!createInstance(env, "org/util/jassimp/AiMaterial", jMaterial))
        {
            return false;
        }

        /* add material to m_materials java.util.List */
        jobject jMaterials = NULL;

        if (!getField(env, jScene, "m_materials", "Ljava/util/List;", jMaterials))
        {
            return false;
        }

        jvalue addMatParams[1];
        addMatParams[0].l = jMaterial;
        if (!call(env, jMaterials, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addMatParams))
        {
            return false;
        }

        /* set texture numbers */
        for (int ttInd = aiTextureType_DIFFUSE; ttInd < aiTextureType_UNKNOWN; ttInd++)
        {
            aiTextureType tt = static_cast<aiTextureType>(ttInd);

            unsigned int num = cMaterial->GetTextureCount(tt);

            LOGE("   found %d textures of type %d ...\n", num, ttInd);

            jvalue setNumberParams[2];
            setNumberParams[0].i = ttInd;
            setNumberParams[1].i = num;

            if (!callv(env, jMaterial, "org/util/jassimp/AiMaterial", "setTextureNumber", "(II)V", setNumberParams))
            {
                return false;
            }
        }


        for (unsigned int p = 0; p < cMaterial->mNumProperties; p++)
        {
            //printf("%s - %u - %u\n", cScene->mMaterials[m]->mProperties[p]->mKey.C_Str(),
            //  cScene->mMaterials[m]->mProperties[p]->mSemantic,
            //  cScene->mMaterials[m]->mProperties[p]->mDataLength);

            const aiMaterialProperty* cProperty = cMaterial->mProperties[p];

            LOGE("   converting property %s ...\n", cProperty->mKey.C_Str());

            jobject jProperty = NULL;

            jvalue constructorParams[5];
            constructorParams[0].l = env->NewStringUTF(cProperty->mKey.C_Str());
            constructorParams[1].i = cProperty->mSemantic;
            constructorParams[2].i = cProperty->mIndex;
            constructorParams[3].i = cProperty->mType;


            /* special case conversion for color3 */
            if (NULL != strstr(cProperty->mKey.C_Str(), "clr") &&
                cProperty->mType == aiPTI_Float &&
                cProperty->mDataLength == 3 * sizeof(float))
            {
                jobject jData = NULL;

                /* wrap color */
                jvalue wrapColorParams[3];
                wrapColorParams[0].f = ((float*) cProperty->mData)[0];
                wrapColorParams[1].f = ((float*) cProperty->mData)[1];
                wrapColorParams[2].f = ((float*) cProperty->mData)[2];
                if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapColor3", "(FFF)Ljava/lang/Object;", wrapColorParams, jData))
                {
                    return false;
                }

                constructorParams[4].l = jData;
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIILjava/lang/Object;)V",
                    constructorParams, jProperty))
                {
                    return false;
                }
            }
            /* special case conversion for color4 */
            else if (NULL != strstr(cProperty->mKey.C_Str(), "clr") &&
                cProperty->mType == aiPTI_Float &&
                cProperty->mDataLength == 4 * sizeof(float))
            {
                jobject jData = NULL;

                /* wrap color */
                jvalue wrapColorParams[4];
                wrapColorParams[0].f = ((float*) cProperty->mData)[0];
                wrapColorParams[1].f = ((float*) cProperty->mData)[1];
                wrapColorParams[2].f = ((float*) cProperty->mData)[2];
                wrapColorParams[3].f = ((float*) cProperty->mData)[3];
                if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapColor4", "(FFFF)Ljava/lang/Object;", wrapColorParams, jData))
                {
                    return false;
                }

                constructorParams[4].l = jData;
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIILjava/lang/Object;)V",
                    constructorParams, jProperty))
                {
                    return false;
                }
            }
            else if (cProperty->mType == aiPTI_Float && cProperty->mDataLength == sizeof(float))
            {
                jobject jData = NULL;

                jvalue newFloatParams[1];
                newFloatParams[0].f = ((float*) cProperty->mData)[0];
                if (!createInstance(env, "java/lang/Float", "(F)V", newFloatParams, jData))
                {
                    return false;
                }

                constructorParams[4].l = jData;
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIILjava/lang/Object;)V",
                    constructorParams, jProperty))
                {
                    return false;
                }
            }
            else if (cProperty->mType == aiPTI_Integer && cProperty->mDataLength == sizeof(int))
            {
                jobject jData = NULL;

                jvalue newIntParams[1];
                newIntParams[0].i = ((int*) cProperty->mData)[0];
                if (!createInstance(env, "java/lang/Integer", "(I)V", newIntParams, jData))
                {
                    return false;
                }

                constructorParams[4].l = jData;
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIILjava/lang/Object;)V",
                    constructorParams, jProperty))
                {
                    return false;
                }
            }
            else if (cProperty->mType == aiPTI_String)
            {
                /* skip length prefix */
                jobject jData = env->NewStringUTF(cProperty->mData + 4);

                constructorParams[4].l = jData;
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIILjava/lang/Object;)V",
                    constructorParams, jProperty))
                {
                    return false;
                }
            }
            else
            {
                constructorParams[4].i = cProperty->mDataLength;

                /* generic copy code, uses dump ByteBuffer on java side */
                if (!createInstance(env, "org/util/jassimp/AiMaterial$Property", "(Ljava/lang/String;IIII)V", constructorParams, jProperty))
                {
                    return false;
                }

                jobject jBuffer = NULL;

                if (!getField(env, jProperty, "m_data", "Ljava/lang/Object;", jBuffer))
                {
                    return false;
                }

                if (env->GetDirectBufferCapacity(jBuffer) != cProperty->mDataLength)
                {
                    LOGE("invalid direct buffer\n");
                    return false;
                }

                void* jBufferPtr = env->GetDirectBufferAddress(jBuffer);

                if (NULL == jBufferPtr)
                {
                    LOGE("could not access direct buffer\n");
                    return false;
                }

                memcpy(jBufferPtr, cProperty->mData, cProperty->mDataLength);
            }


            /* add property */
            jobject jProperties = NULL;

            if (!getField(env, jMaterial, "m_properties", "Ljava/util/List;", jProperties))
            {
                return false;
            }

            jvalue addPropParams[1];
            addPropParams[0].l = jProperty;
            if (!call(env, jProperties, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addPropParams))
            {
                return false;
            }
        }
    }

    LOGE("materials finished\n");

    return true;
}


bool Jassimp::loadAnimations(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    LOGE("converting %d animations ...\n", cScene->mNumAnimations);

    for (unsigned int a = 0; a < cScene->mNumAnimations; a++)
    {
        const aiAnimation *cAnimation = cScene->mAnimations[a];

        LOGE("   converting animation %s ...\n", cAnimation->mName.C_Str());

        jobject jAnimation;
        jvalue newAnimParams[3];
        newAnimParams[0].l = env->NewStringUTF(cAnimation->mName.C_Str());
        newAnimParams[1].d = cAnimation->mDuration;
        newAnimParams[2].d = cAnimation->mTicksPerSecond;

        if (!createInstance(env, "org/util/jassimp/AiAnimation", "(Ljava/lang/String;DD)V", newAnimParams, jAnimation))
        {
            return false;
        }

        /* add animation to m_animations java.util.List */
        jobject jAnimations = NULL;

        if (!getField(env, jScene, "m_animations", "Ljava/util/List;", jAnimations))
        {
            return false;
        }

        jvalue addParams[1];
        addParams[0].l = jAnimation;
        if (!call(env, jAnimations, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
        {
            return false;
        }


        for (unsigned int c = 0; c < cAnimation->mNumChannels; c++)
        {
            const aiNodeAnim *cNodeAnim = cAnimation->mChannels[c];

            jobject jNodeAnim;
            jvalue newNodeAnimParams[6];
            newNodeAnimParams[0].l = env->NewStringUTF(cNodeAnim->mNodeName.C_Str());
            newNodeAnimParams[1].i = cNodeAnim->mNumPositionKeys;
            newNodeAnimParams[2].i = cNodeAnim->mNumRotationKeys;
            newNodeAnimParams[3].i = cNodeAnim->mNumScalingKeys;
            newNodeAnimParams[4].i = cNodeAnim->mPreState;
            newNodeAnimParams[5].i = cNodeAnim->mPostState;

            if (!createInstance(env, "org/util/jassimp/AiNodeAnim", "(Ljava/lang/String;IIIII)V", newNodeAnimParams, jNodeAnim))
            {
                return false;
            }


            /* add nodeanim to m_animations java.util.List */
            jobject jNodeAnims = NULL;

            if (!getField(env, jAnimation, "m_nodeAnims", "Ljava/util/List;", jNodeAnims))
            {
                return false;
            }

            jvalue addParams[1];
            addParams[0].l = jNodeAnim;
            if (!call(env, jNodeAnims, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
            {
                return false;
            }

            /* copy keys */
            if (!copyBuffer(env, jNodeAnim, "m_posKeys", cNodeAnim->mPositionKeys,
                cNodeAnim->mNumPositionKeys * sizeof(aiVectorKey)))
            {
                return false;
            }

            if (!copyBuffer(env, jNodeAnim, "m_rotKeys", cNodeAnim->mRotationKeys,
                cNodeAnim->mNumRotationKeys * sizeof(aiQuatKey)))
            {
                return false;
            }

            if (!copyBuffer(env, jNodeAnim, "m_scaleKeys", cNodeAnim->mScalingKeys,
                cNodeAnim->mNumScalingKeys * sizeof(aiVectorKey)))
            {
                return false;
            }
        }
    }

    LOGE("converting animations finished\n");

    return true;
}

bool Jassimp::loadLights(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    LOGE("converting %d lights ...\n", cScene->mNumLights);

    for (unsigned int l = 0; l < cScene->mNumLights; l++)
    {
        const aiLight *cLight = cScene->mLights[l];

        LOGE("converting light %s ...\n", cLight->mName.C_Str());

        /* wrap color nodes */
        jvalue wrapColorParams[3];
        wrapColorParams[0].f = cLight->mColorDiffuse.r;
        wrapColorParams[1].f = cLight->mColorDiffuse.g;
        wrapColorParams[2].f = cLight->mColorDiffuse.b;
        jobject jDiffuse;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapColor3", "(FFF)Ljava/lang/Object;", wrapColorParams, jDiffuse))
        {
            return false;
        }

        wrapColorParams[0].f = cLight->mColorSpecular.r;
        wrapColorParams[1].f = cLight->mColorSpecular.g;
        wrapColorParams[2].f = cLight->mColorSpecular.b;
        jobject jSpecular;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapColor3", "(FFF)Ljava/lang/Object;", wrapColorParams, jSpecular))
        {
            return false;
        }

        wrapColorParams[0].f = cLight->mColorAmbient.r;
        wrapColorParams[1].f = cLight->mColorAmbient.g;
        wrapColorParams[2].f = cLight->mColorAmbient.b;
        jobject jAmbient;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapColor3", "(FFF)Ljava/lang/Object;", wrapColorParams, jAmbient))
        {
            return false;
        }


        /* wrap vec3 nodes */
        jvalue wrapVec3Params[3];
        wrapVec3Params[0].f = cLight->mPosition.x;
        wrapVec3Params[1].f = cLight->mPosition.y;
        wrapVec3Params[2].f = cLight->mPosition.z;
        jobject jPosition;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapVec3", "(FFF)Ljava/lang/Object;", wrapVec3Params, jPosition))
        {
            return false;
        }

        wrapVec3Params[0].f = cLight->mPosition.x;
        wrapVec3Params[1].f = cLight->mPosition.y;
        wrapVec3Params[2].f = cLight->mPosition.z;
        jobject jDirection;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapVec3", "(FFF)Ljava/lang/Object;", wrapVec3Params, jDirection))
        {
            return false;
        }


        jobject jLight;

        jvalue params[12];
        params[0].l = env->NewStringUTF(cLight->mName.C_Str());;
        params[1].i = cLight->mType;
        params[2].l = jPosition;
        params[3].l = jDirection;
        params[4].f = cLight->mAttenuationConstant;
        params[5].f = cLight->mAttenuationLinear;
        params[6].f = cLight->mAttenuationQuadratic;
        params[7].l = jDiffuse;
        params[8].l = jSpecular;
        params[9].l = jAmbient;
        params[10].f = cLight->mAngleInnerCone;
        params[11].f = cLight->mAngleOuterCone;

        if (!createInstance(env, "org/util/jassimp/AiLight", "(Ljava/lang/String;ILjava/lang/Object;Ljava/lang/Object;FFFLjava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;FF)V",
            params, jLight))
        {
            return false;
        }

        /* add light to m_lights java.util.List */
        jobject jLights = NULL;

        if (!getField(env, jScene, "m_lights", "Ljava/util/List;", jLights))
        {
            return false;
        }

        jvalue addParams[1];
        addParams[0].l = jLight;
        if (!call(env, jLights, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
        {
            return false;
        }
    }

    LOGE("converting lights finished ...\n");

    return true;
}

bool Jassimp::loadCameras(JNIEnv *env, const aiScene* cScene, jobject& jScene)
{
    LOGE("converting %d cameras ...\n", cScene->mNumCameras);

    for (unsigned int c = 0; c < cScene->mNumCameras; c++)
    {
        const aiCamera *cCamera = cScene->mCameras[c];

        LOGE("converting camera %s ...\n", cCamera->mName.C_Str());

        /* wrap color nodes */
        jvalue wrapPositionParams[3];
        wrapPositionParams[0].f = cCamera->mPosition.x;
        wrapPositionParams[1].f = cCamera->mPosition.y;
        wrapPositionParams[2].f = cCamera->mPosition.z;
        jobject jPosition;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapVec3", "(FFF)Ljava/lang/Object;", wrapPositionParams, jPosition))
        {
            return false;
        }

        wrapPositionParams[0].f = cCamera->mUp.x;
        wrapPositionParams[1].f = cCamera->mUp.y;
        wrapPositionParams[2].f = cCamera->mUp.z;
        jobject jUp;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapVec3", "(FFF)Ljava/lang/Object;", wrapPositionParams, jUp))
        {
            return false;
        }

        wrapPositionParams[0].f = cCamera->mLookAt.x;
        wrapPositionParams[1].f = cCamera->mLookAt.y;
        wrapPositionParams[2].f = cCamera->mLookAt.z;
        jobject jLookAt;
        if (!callStaticObject(env, "org/util/jassimp/Jassimp", "wrapVec3", "(FFF)Ljava/lang/Object;", wrapPositionParams, jLookAt))
        {
            return false;
        }


        jobject jCamera;

        jvalue params[8];
        params[0].l = env->NewStringUTF(cCamera->mName.C_Str());
        params[1].l = jPosition;
        params[2].l = jUp;
        params[3].l = jLookAt;
        params[4].f = cCamera->mHorizontalFOV;
        params[5].f = cCamera->mClipPlaneNear;
        params[6].f = cCamera->mClipPlaneFar;
        params[7].f = cCamera->mAspect;

        if (!createInstance(env, "org/util/jassimp/AiCamera", "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;FFFF)V",
            params, jCamera))
        {
            return false;
        }

        /* add camera to m_cameras java.util.List */
        jobject jCameras = NULL;

        if (!getField(env, jScene, "m_cameras", "Ljava/util/List;", jCameras))
        {
            return false;
        }

        jvalue addParams[1];
        addParams[0].l = jCamera;
        if (!call(env, jCameras, "java/util/Collection", "add", "(Ljava/lang/Object;)Z", addParams))
        {
            return false;
        }
    }

    LOGE("converting cameras finished\n");

    return true;
}

}
