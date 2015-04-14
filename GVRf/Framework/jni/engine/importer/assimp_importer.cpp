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
 * Contains a ai_scene of Assimp.
 ***************************************************************************/

#include "assimp_importer.h"

#include "objects/mesh.h"
#include "objects/scene.h"
#include "objects/scene_object.h"
#include "objects/components/render_data.h"

namespace gvr {
std::shared_ptr<Mesh> AssimpImporter::getMesh(int index) {
    Mesh* mesh = new Mesh();

    aiMesh* ai_mesh = assimp_importer_->GetScene()->mMeshes[index];

    std::vector<glm::vec3> vertices;
    for (int i = 0; i < ai_mesh->mNumVertices; ++i) {
        vertices.push_back(
                glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y,
                        ai_mesh->mVertices[i].z));
    }
    mesh->set_vertices(std::move(vertices));

    if (ai_mesh->mNormals != 0) {
        std::vector<glm::vec3> normals;
        for (int i = 0; i < ai_mesh->mNumVertices; ++i) {
            normals.push_back(
                    glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y,
                            ai_mesh->mNormals[i].z));
        }
        mesh->set_normals(std::move(normals));
    }

    if (ai_mesh->mTextureCoords[0] != 0) {
        std::vector<glm::vec2> tex_coords;
        for (int i = 0; i < ai_mesh->mNumVertices; ++i) {
            tex_coords.push_back(
                    glm::vec2(ai_mesh->mTextureCoords[0][i].x,
                            ai_mesh->mTextureCoords[0][i].y));
        }
        mesh->set_tex_coords(std::move(tex_coords));
    }

    std::vector<unsigned short> triangles;
    for (int i = 0; i < ai_mesh->mNumFaces; ++i) {
        if (ai_mesh->mFaces[i].mNumIndices == 3) {
            triangles.push_back(ai_mesh->mFaces[i].mIndices[0]);
            triangles.push_back(ai_mesh->mFaces[i].mIndices[1]);
            triangles.push_back(ai_mesh->mFaces[i].mIndices[2]);
        }
    }
    mesh->set_triangles(std::move(triangles));

    return std::shared_ptr < Mesh > (mesh);
}

void AssimpImporter::sceneRecursion(aiNode* node, const aiScene* aScene, std::shared_ptr<Scene> scnPtr, JNIEnv * env, jobject obj, jobject bitmap, aiMatrix4x4 accumulatedTransform)
{
	for(int i=0; i < node->mNumMeshes; i++)
	{
		std::shared_ptr<SceneObject> scnObject(new SceneObject());

		// Creates a scene object for the node children meshes.
		if(scnObject->render_data() == NULL)
		{
			// Scene Object has no render data.

			// Mesh
			std::shared_ptr<Mesh> mesh = getMesh(node->mMeshes[i]);

			// New render data object.
			std::shared_ptr<RenderData> renderData(new RenderData());

			// Set the mesh to the render data.
			renderData->set_mesh(mesh);

			// Set material.
			aiMesh* ai_mesh = aScene->mMeshes[node->mMeshes[i]];
			aiMaterial* ai_material = aScene->mMaterials[ai_mesh->mMaterialIndex];

			// Defines a shader with shader type 0. UNLIT_SHADER = 0
			std::shared_ptr<Material> material(new Material(static_cast<Material::ShaderType>(0)));

			// Actual texture image
			aiString textureImage;
			ai_material->GetTexture(aiTextureType_DIFFUSE, i, &textureImage);
			jstring texFileName = env->NewStringUTF(textureImage.C_Str());
			jclass texClass = env->FindClass("org/gearvrf/GVRContext");

			if (texClass == NULL) {
				// Couldn't find the class GVRContext.
			} else {
				jmethodID mID = env->GetStaticMethodID(texClass, "loadBitmap", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
				if (mID == NULL) {
					// No such method in Java side
					// No interaction with Java side so apply default texture
					std::shared_ptr<Texture> texture(new BaseTexture(env, bitmap));
					material->setTexture("main_texture", texture);
					// Sets the default main texture

				} else {
					if ((ai_material->GetTextureCount(aiTextureType_DIFFUSE)) <= 0) {
						// Not texture file found so apply default texture
						std::shared_ptr<Texture> texture(new BaseTexture(env, bitmap));
						material->setTexture("main_texture", texture);
					} else {
						// About to enter Java side.
						jobject mBitmap = env->CallStaticObjectMethod(texClass, mID, texFileName);
						if(mBitmap == NULL)
						{
							// Null bitmap for texture file for mesh
							// Applying default texture
							std::shared_ptr<Texture> texture(new BaseTexture(env, bitmap));
							material->setTexture("main_texture", texture);
						}
						else
						{
							// Back to native side from Java side
							// Applying actual texture
							std::shared_ptr<Texture> texture(new BaseTexture(env, mBitmap));
							material->setTexture("main_texture", texture);
						}
					}
				}
			}

			// Set the material to the render data
			renderData->set_material(material);

			// Transformation
			std::shared_ptr<Transform> transform(new Transform());
			transform->set_owner_object(scnObject);

			// Accumulated transformations of the node
			aiMatrix4x4 aTransform = node->mTransformation * accumulatedTransform;
			aiVector3t<float> scaling;
			aiQuaterniont<float> rotation;
			aiVector3t<float> position;
			aTransform.Decompose(scaling, rotation, position);

			int meshIndex = i;

			transform->set_position(position.x, position.y, position.z);
			transform->set_rotation(rotation.w, rotation.x, rotation.y, rotation.z);
			transform->set_scale(scaling.x, scaling.y, scaling.z);
			scnObject->attachTransform(scnObject, transform);
			scnObject->attachRenderData(scnObject, renderData);

			//Attaches the Scene Object to the Scene.
			scnPtr->addSceneObject(scnObject);
		} else {
			//new RenderData class and perform all the operations
		}
	}

	for(int i=0;i<node->mNumChildren;i++)
	{
		sceneRecursion(node->mChildren[i], aScene, scnPtr, env, obj, bitmap, accumulatedTransform);
	}
}


std::shared_ptr<Scene> AssimpImporter::loadScene(JNIEnv* env, jobject obj, jobject bitmap)
{
	//	Scene
	std::shared_ptr<Scene> scenePtr(new Scene());

	// Left Camera
	std::shared_ptr<Camera> leftCamera(new PerspectiveCamera());
	leftCamera->set_render_mask(0x1);

	//	Right Camera
	std::shared_ptr<Camera> rightCamera(new PerspectiveCamera());
	rightCamera->set_render_mask(0x2);

	std::shared_ptr<SceneObject> leftCameraObject(new SceneObject());

	std::shared_ptr<Transform> transformLeftCameraObject(new Transform());
	transformLeftCameraObject->set_owner_object(leftCameraObject);

	float camera_separation_distance_ = 0.062f;
	glm::vec3 leftCameraPosition = glm::vec3(-camera_separation_distance_ * 0.5f, 0.0f, 0.0f);
	glm::vec3 rightCameraPosition = glm::vec3(camera_separation_distance_ * 0.5f, 0.0f, 0.0f);

	transformLeftCameraObject->set_position(leftCameraPosition);
	leftCameraObject->attachTransform(leftCameraObject, transformLeftCameraObject);
	leftCameraObject->attachCamera(leftCameraObject, leftCamera);

	std::shared_ptr<SceneObject> rightCameraObject(new SceneObject());
	std::shared_ptr<Transform> transformRightCameraObject(new Transform());
	transformRightCameraObject->set_owner_object(rightCameraObject);
	transformRightCameraObject->set_position(rightCameraPosition);
	rightCameraObject->attachTransform(rightCameraObject, transformRightCameraObject);
	rightCameraObject->attachCamera(rightCameraObject, rightCamera);

	std::shared_ptr<SceneObject> cameraRigObject(new SceneObject());
	std::shared_ptr<Transform> transformCameraRigObject(new Transform());
	cameraRigObject->attachTransform(cameraRigObject, transformCameraRigObject);
	std::shared_ptr<CameraRig> cameraRig(new CameraRig());
	cameraRig->attachLeftCamera(leftCamera);
	cameraRig->attachRightCamera(rightCamera);
	cameraRigObject->attachCameraRig(cameraRigObject, cameraRig);
	scenePtr->addSceneObject(cameraRigObject);
	cameraRigObject->addChildObject(cameraRigObject, leftCameraObject);
	cameraRigObject->addChildObject(cameraRigObject, rightCameraObject);
	scenePtr->set_main_camera_rig(cameraRig);
	const aiScene *aScnPtr = assimp_importer_->GetScene();

	// Default transformation
	aiMatrix4x4 identityMatrix;

	// Start the scene recursion
	sceneRecursion(aScnPtr->mRootNode, aScnPtr, scenePtr, env, obj, bitmap, identityMatrix);

	return std::shared_ptr<Scene>(scenePtr);
}
}
