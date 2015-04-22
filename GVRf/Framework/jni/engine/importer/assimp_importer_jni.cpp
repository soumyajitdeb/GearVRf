/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/***************************************************************************
 * JNI
 ***************************************************************************/

#include "assimp_importer.h"

#include "util/gvr_jni.h"

namespace gvr {
extern "C" {
JNIEXPORT jint JNICALL
Java_org_gearvrf_NativeAssimpImporter_getNumberOfMeshes(
        JNIEnv * env, jobject obj, jlong jassimp_importer);
JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeAssimpImporter_loadScene(JNIEnv * env,
        jobject obj, jlong jassimp_importer, jobject default_bitmap, jobject gvr_context);

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeAssimpImporter_getMesh(JNIEnv * env,
        jobject obj, jlong jassimp_importer, jint index);
}

JNIEXPORT jint JNICALL
Java_org_gearvrf_NativeAssimpImporter_getNumberOfMeshes(
        JNIEnv * env, jobject obj, jlong jassimp_importer) {
    std::shared_ptr<AssimpImporter> assimp_importer =
            *reinterpret_cast<std::shared_ptr<AssimpImporter>*>(jassimp_importer);
    return assimp_importer->getNumberOfMeshes();
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeAssimpImporter_loadScene(JNIEnv * env,
        jobject obj, jlong jassimp_importer, jobject default_bitmap, jobject gvr_context) {
    std::shared_ptr<AssimpImporter> assimp_importer =
            *reinterpret_cast<std::shared_ptr<AssimpImporter>*>(jassimp_importer);
    std::shared_ptr<Scene> scene = assimp_importer->load_scene(env, obj, default_bitmap, gvr_context);
    return reinterpret_cast<jlong>(new std::shared_ptr<Scene>(scene));
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeAssimpImporter_getMesh(JNIEnv * env,
        jobject obj, jlong jassimp_importer, jint index) {
    std::shared_ptr<AssimpImporter> assimp_importer =
            *reinterpret_cast<std::shared_ptr<AssimpImporter>*>(jassimp_importer);
    std::shared_ptr<Mesh> mesh = assimp_importer->getMesh(index);
    return reinterpret_cast<jlong>(new std::shared_ptr<Mesh>(mesh));
}
}
