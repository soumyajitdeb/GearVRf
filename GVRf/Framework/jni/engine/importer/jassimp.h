#ifndef JASSIMP_H_
#define JASSIMP_H_

#include <android/bitmap.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "objects/components/perspective_camera.h"
#include "objects/components/camera_rig.h"
#include "objects/textures/base_texture.h"
#include "objects/components/transform.h"
#include "objects/components/component.h"
#include "objects/components/camera.h"
#include "objects/hybrid_object.h"
#include "objects/scene_object.h"
#include "assimp/Importer.hpp"
#include "objects/material.h"
#include "assimp/material.h"
#include "objects/scene.h"
#include "assimp/scene.h"
#include "util/gvr_log.h"
#include "glm/glm.hpp"
#include "assimp/cimport.h"

namespace gvr {
class Jassimp {
public:
    Jassimp();

    static bool createInstance(JNIEnv *env, const char* class_name, jobject& new_instance);
    static bool createInstance(JNIEnv *env, const char* class_name, const char* signature,/* const*/ jvalue* params, jobject& new_instance);
    static bool getField(JNIEnv *env, jobject object, const char* field_name, const char* signature, jobject& field);
    static bool setIntField(JNIEnv *env, jobject object, const char* field_name, jint value);
    static bool setFloatField(JNIEnv *env, jobject object, const char* field_name, jfloat value);
    static bool setObjectField(JNIEnv *env, jobject object, const char* field_name, const char* signature, jobject value);
    static bool getStaticField(JNIEnv *env, const char* class_name, const char* field_name, const char* signature, jobject& field);
    static bool call(JNIEnv *env, jobject object, const char* type_name, const char* method_name, const char* signature,/* const*/ jvalue* params);
    static bool callv(JNIEnv *env, jobject object, const char* type_name, const char* method_name, const char* signature,/* const*/ jvalue* params);
    static bool callStaticObject(JNIEnv *env, const char* type_name, const char* method_name, const char* signature,/* const*/ jvalue* params, jobject& return_value);
    static bool copyBuffer(JNIEnv *env, jobject mesh, const char* buffer_name, void* data, size_t size);
    static bool copyBufferArray(JNIEnv *env, jobject mesh, const char* buffer_name, int index, void* data, size_t size);
    static bool loadMeshes(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);
    static bool loadSceneNode(JNIEnv *env, const aiNode *node, jobject parent, jobject* loaded_node = NULL);
    static bool loadSceneGraph(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);
    static bool loadMaterials(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);
    static bool loadAnimations(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);
    static bool loadLights(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);
    static bool loadCameras(JNIEnv *env, const aiScene* assimp_scene, jobject& scene);

};
}
#endif
