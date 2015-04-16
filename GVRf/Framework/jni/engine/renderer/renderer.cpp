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
 * Renders a scene, a screen.
 ***************************************************************************/

#include "renderer.h"

#include "glm/gtc/matrix_inverse.hpp"

#include "eglextension/tiledrendering/tiled_rendering_enhancer.h"
#include "objects/material.h"
#include "objects/post_effect_data.h"
#include "objects/scene.h"
#include "objects/scene_object.h"
#include "objects/components/camera.h"
#include "objects/components/eye_pointee_holder.h"
#include "objects/components/render_data.h"
#include "objects/textures/render_texture.h"
#include "shaders/shader_manager.h"
#include "shaders/post_effect_shader_manager.h"
#include "util/gvr_gl.h"

namespace gvr {

void Renderer::renderCamera(std::shared_ptr<Scene> scene,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<RenderTexture> render_texture,
        std::shared_ptr<ShaderManager> shader_manager,
        std::shared_ptr<PostEffectShaderManager> post_effect_shader_manager,
        std::shared_ptr<RenderTexture> post_effect_render_texture_a,
        std::shared_ptr<RenderTexture> post_effect_render_texture_b,
        glm::mat4 vp_matrix)
{
    // there is no need to flat and sort every frame.
    // however let's keep it as is and assume we are not changed
    // This is not right way to do data conversion. However since GVRF doesn't support
    // bone/weight/joint and other assimp data, we will put general model conversion
    // on hold and do this kind of conversion fist
    if (scene->getSceneDirtyFlag()) {
        std::vector < std::shared_ptr < SceneObject >> scene_objects =
                scene->getWholeSceneObjects();
        std::vector < std::shared_ptr < RenderData >> render_data_vector;
        for (auto it = scene_objects.begin(); it != scene_objects.end(); ++it) {
            std::shared_ptr<RenderData> render_data = (*it)->render_data();
            if (render_data != 0) {
                if (render_data->material() != 0) {

                    // Frustum Culling
                    LOGE("Frustum Culling");
                	std::shared_ptr<Mesh> currentMesh = render_data->mesh();
                	float* boundingBoxInfo = currentMesh->getBoundingBoxInfo();
                	LOGE("Bounding Box Info: %f %f %f %f %f %f", boundingBoxInfo[0], boundingBoxInfo[1], boundingBoxInfo[2], boundingBoxInfo[3], boundingBoxInfo[4], boundingBoxInfo[5]);

                	// MVP Matrix
                    glm::mat4 model_matrix_tmp(render_data->owner_object()->transform()->getModelMatrix());
                    glm::mat4 view_matrix_tmp = camera->getViewMatrix();
                    glm::mat4 proj_matrix_tmp = camera->getProjectionMatrix();
                    glm::mat4 mvp_matrix_tmp(proj_matrix_tmp * view_matrix_tmp * model_matrix_tmp);

                    // Frustum
                    float frustum[6][4];

                    float mvpMat[16] = {0.0};

                    const float *matToArray = (const float*)glm::value_ptr(mvp_matrix_tmp);
                    for (int i = 0; i < 16; i++) {
                    	mvpMat[i] = matToArray[i];
                    }

                    buildFrustum(frustum, mvpMat);
                    bool isInside = isCubeInFrustum(frustum, boundingBoxInfo[0], boundingBoxInfo[1], boundingBoxInfo[2], boundingBoxInfo[3], boundingBoxInfo[4], boundingBoxInfo[5]);

                    if (isInside) {
                    	LOGE("Inside");
                    } else {
                    	LOGE("Outside");
                    }


                    render_data_vector.push_back(render_data);
                }
            }
        }
        std::sort(render_data_vector.begin(), render_data_vector.end(),
                compareRenderData);

        glm::mat4 view_matrix = camera->getViewMatrix();
        glm::mat4 proj_matrix = camera->getProjectionMatrix();
        glm::mat4 cp_matrix = glm::mat4(proj_matrix * view_matrix);
        vp_matrix = cp_matrix;

        std::vector < std::shared_ptr < PostEffectData >> post_effects =
                camera->post_effect_data();

        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LEQUAL);
        glEnable (GL_CULL_FACE);
        glFrontFace (GL_CCW);
        glCullFace (GL_BACK);
        glEnable (GL_BLEND);
        glBlendEquation (GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable (GL_POLYGON_OFFSET_FILL);

        bool forceNoPostEffect = false;
        GLint curFBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curFBO);

        if (forceNoPostEffect || post_effects.size() == 0) {
            glClearColor(camera->background_color_r(), camera->background_color_g(),
                    camera->background_color_b(), camera->background_color_a());

            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            for (auto it = render_data_vector.begin();
                    it != render_data_vector.end(); ++it) {
                renderRenderData(*it, vp_matrix, camera->render_mask(),
                        shader_manager);
            }
        }
        else {
            std::shared_ptr<RenderTexture> texture_render_texture =
                    post_effect_render_texture_a;
            std::shared_ptr<RenderTexture> target_render_texture;

            glBindFramebuffer(GL_FRAMEBUFFER,
                    texture_render_texture->getFrameBufferId());
            glViewport(0, 0, texture_render_texture->width(),
                    texture_render_texture->height());
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            for (auto it = render_data_vector.begin();
                    it != render_data_vector.end(); ++it) {
                renderRenderData(*it, vp_matrix, camera->render_mask(),
                        shader_manager);
            }

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            for (int i = 0; i < post_effects.size() - 1; ++i) {
                if (i % 2 == 0) {
                    texture_render_texture = post_effect_render_texture_a;
                    target_render_texture = post_effect_render_texture_b;
                } else {
                    texture_render_texture = post_effect_render_texture_b;
                    target_render_texture = post_effect_render_texture_a;
                }
                glBindFramebuffer(GL_FRAMEBUFFER,
                        target_render_texture->getFrameBufferId());
                glViewport(0, 0, target_render_texture->width(),
                        target_render_texture->height());

                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                renderPostEffectData(texture_render_texture, post_effects[i],
                        post_effect_shader_manager);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, curFBO);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            renderPostEffectData(texture_render_texture, post_effects.back(),
                    post_effect_shader_manager);
        }
    } // flag checking
}

void Renderer::buildFrustum(float frustum[6][4], float mvpMat[16])
{
   float   t;

   /* Extract the numbers for the RIGHT plane */
   frustum[0][0] = mvpMat[ 3] - mvpMat[ 0];
   frustum[0][1] = mvpMat[ 7] - mvpMat[ 4];
   frustum[0][2] = mvpMat[11] - mvpMat[ 8];
   frustum[0][3] = mvpMat[15] - mvpMat[12];

   /* Normalize the result */
   t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
   frustum[0][0] /= t;
   frustum[0][1] /= t;
   frustum[0][2] /= t;
   frustum[0][3] /= t;

   /* Extract the numbers for the LEFT plane */
   frustum[1][0] = mvpMat[ 3] + mvpMat[ 0];
   frustum[1][1] = mvpMat[ 7] + mvpMat[ 4];
   frustum[1][2] = mvpMat[11] + mvpMat[ 8];
   frustum[1][3] = mvpMat[15] + mvpMat[12];

   /* Normalize the result */
   t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
   frustum[1][0] /= t;
   frustum[1][1] /= t;
   frustum[1][2] /= t;
   frustum[1][3] /= t;

   /* Extract the BOTTOM plane */
   frustum[2][0] = mvpMat[ 3] + mvpMat[ 1];
   frustum[2][1] = mvpMat[ 7] + mvpMat[ 5];
   frustum[2][2] = mvpMat[11] + mvpMat[ 9];
   frustum[2][3] = mvpMat[15] + mvpMat[13];

   /* Normalize the result */
   t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
   frustum[2][0] /= t;
   frustum[2][1] /= t;
   frustum[2][2] /= t;
   frustum[2][3] /= t;

   /* Extract the TOP plane */
   frustum[3][0] = mvpMat[ 3] - mvpMat[ 1];
   frustum[3][1] = mvpMat[ 7] - mvpMat[ 5];
   frustum[3][2] = mvpMat[11] - mvpMat[ 9];
   frustum[3][3] = mvpMat[15] - mvpMat[13];

   /* Normalize the result */
   t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
   frustum[3][0] /= t;
   frustum[3][1] /= t;
   frustum[3][2] /= t;
   frustum[3][3] /= t;

   /* Extract the FAR plane */
   frustum[4][0] = mvpMat[ 3] - mvpMat[ 2];
   frustum[4][1] = mvpMat[ 7] - mvpMat[ 6];
   frustum[4][2] = mvpMat[11] - mvpMat[10];
   frustum[4][3] = mvpMat[15] - mvpMat[14];

   /* Normalize the result */
   t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
   frustum[4][0] /= t;
   frustum[4][1] /= t;
   frustum[4][2] /= t;
   frustum[4][3] /= t;

   /* Extract the NEAR plane */
   frustum[5][0] = mvpMat[ 3] + mvpMat[ 2];
   frustum[5][1] = mvpMat[ 7] + mvpMat[ 6];
   frustum[5][2] = mvpMat[11] + mvpMat[10];
   frustum[5][3] = mvpMat[15] + mvpMat[14];

   /* Normalize the result */
   t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
   frustum[5][0] /= t;
   frustum[5][1] /= t;
   frustum[5][2] /= t;
   frustum[5][3] /= t;
}

bool Renderer::isCubeInFrustum( float frustum[6][4], float x, float y, float z, float x1, float y1, float z1)
{
   int p;

   for( p = 0; p < 6; p++ )
   {
      if( frustum[p][0] * (x) + frustum[p][1] * (y) + frustum[p][2] * (z) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x1) + frustum[p][1] * (y) + frustum[p][2] * (z) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x) + frustum[p][1] * (y1) + frustum[p][2] * (z) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x1) + frustum[p][1] * (y1) + frustum[p][2] * (z) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x) + frustum[p][1] * (y) + frustum[p][2] * (z1) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x1) + frustum[p][1] * (y) + frustum[p][2] * (z1) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x) + frustum[p][1] * (y1) + frustum[p][2] * (z1) + frustum[p][3] > 0 )
         continue;
      if( frustum[p][0] * (x1) + frustum[p][1] * (y1) + frustum[p][2] * (z1) + frustum[p][3] > 0 )
         continue;
      return false;
   }
   return true;
}


void Renderer::renderCamera(std::shared_ptr<Scene> scene,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<RenderTexture> render_texture,
        std::shared_ptr<ShaderManager> shader_manager,
        std::shared_ptr<PostEffectShaderManager> post_effect_shader_manager,
        std::shared_ptr<RenderTexture> post_effect_render_texture_a,
        std::shared_ptr<RenderTexture> post_effect_render_texture_b) {
    std::vector < std::shared_ptr < SceneObject >> scene_objects =
            scene->getWholeSceneObjects();
    std::vector < std::shared_ptr < RenderData >> render_data_vector;
    for (auto it = scene_objects.begin(); it != scene_objects.end(); ++it) {
        std::shared_ptr<RenderData> render_data = (*it)->render_data();
        if (render_data != 0) {
            if (render_data->material() != 0) {
                render_data_vector.push_back(render_data);
            }
        }
    }
    std::sort(render_data_vector.begin(), render_data_vector.end(),
            compareRenderData);

    glm::mat4 view_matrix = camera->getViewMatrix();
    glm::mat4 projection_matrix = camera->getProjectionMatrix(); //gun
    glm::mat4 vp_matrix = glm::mat4(projection_matrix * view_matrix);

    std::vector < std::shared_ptr < PostEffectData >> post_effects =
            camera->post_effect_data();

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_CULL_FACE);
    glFrontFace (GL_CCW);
    glCullFace (GL_BACK);
    glEnable (GL_BLEND);
    glBlendEquation (GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_POLYGON_OFFSET_FILL);

    if (post_effects.size() == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, render_texture->getFrameBufferId());
        glViewport(0, 0, render_texture->width(), render_texture->height());
        glClearColor(camera->background_color_r(), camera->background_color_g(),
                camera->background_color_b(), camera->background_color_a());

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (auto it = render_data_vector.begin();
                it != render_data_vector.end(); ++it) {
            renderRenderData(*it, vp_matrix, camera->render_mask(),
                    shader_manager);
        }
    }

    else {
        std::shared_ptr<RenderTexture> texture_render_texture =
                post_effect_render_texture_a;
        std::shared_ptr<RenderTexture> target_render_texture;

        glBindFramebuffer(GL_FRAMEBUFFER,
                texture_render_texture->getFrameBufferId());
        glViewport(0, 0, texture_render_texture->width(),
                texture_render_texture->height());

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (auto it = render_data_vector.begin();
                it != render_data_vector.end(); ++it) {
            renderRenderData(*it, vp_matrix, camera->render_mask(),
                    shader_manager);
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        for (int i = 0; i < post_effects.size() - 1; ++i) {
            if (i % 2 == 0) {
                texture_render_texture = post_effect_render_texture_a;
                target_render_texture = post_effect_render_texture_b;
            } else {
                texture_render_texture = post_effect_render_texture_b;
                target_render_texture = post_effect_render_texture_a;
            }

            glBindFramebuffer(GL_FRAMEBUFFER,
                    target_render_texture->getFrameBufferId());
            glViewport(0, 0, target_render_texture->width(),
                    target_render_texture->height());

            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            renderPostEffectData(texture_render_texture, post_effects[i],
                    post_effect_shader_manager);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, render_texture->getFrameBufferId());
        glViewport(0, 0, render_texture->width(), render_texture->height());

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        renderPostEffectData(texture_render_texture, post_effects.back(),
                post_effect_shader_manager);
    }
}

void Renderer::renderRenderData(std::shared_ptr<RenderData> render_data,
        const glm::mat4& vp_matrix, int render_mask,
        std::shared_ptr<ShaderManager> shader_manager) {
    if (render_mask & render_data->render_mask()) {
        if (!render_data->cull_test()) {
            glDisable (GL_CULL_FACE);
        }
        if (render_data->offset()) {
            glEnable (GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(render_data->offset_factor(),
                    render_data->offset_units());
        }
        if (!render_data->depth_test()) {
            glDisable (GL_DEPTH_TEST);
        }
        if (!render_data->alpha_blend()) {
            glDisable (GL_BLEND);
        }
        if (render_data->mesh() != 0) {
            glm::mat4 model_matrix(
                    render_data->owner_object()->transform()->getModelMatrix());
            glm::mat4 mvp_matrix(vp_matrix * model_matrix);
            try {
                bool right = render_mask & RenderData::RenderMaskBit::Right;
                switch (render_data->material()->shader_type()) {
                case Material::ShaderType::UNLIT_SHADER:
                    shader_manager->getUnlitShader()->render(mvp_matrix,
                            render_data);
                    break;
                case Material::ShaderType::UNLIT_HORIZONTAL_STEREO_SHADER:
                    shader_manager->getUnlitHorizontalStereoShader()->render(
                            mvp_matrix, render_data, right);
                    break;
                case Material::ShaderType::UNLIT_VERTICAL_STEREO_SHADER:
                    shader_manager->getUnlitVerticalStereoShader()->render(
                            mvp_matrix, render_data, right);
                    break;
                case Material::ShaderType::OES_SHADER:
                    shader_manager->getOESShader()->render(mvp_matrix,
                            render_data);
                    break;
                case Material::ShaderType::OES_HORIZONTAL_STEREO_SHADER:
                    shader_manager->getOESHorizontalStereoShader()->render(
                            mvp_matrix, render_data, right);
                    break;
                case Material::ShaderType::OES_VERTICAL_STEREO_SHADER:
                    shader_manager->getOESVerticalStereoShader()->render(
                            mvp_matrix, render_data, right);
                    break;
                default:
                    shader_manager->getCustomShader(
                            render_data->material()->shader_type())->render(
                            mvp_matrix, render_data, right);
                    break;
                }
            } catch (std::string error) {
                LOGE(
                        "Error detected in Renderer::renderRenderData; name : %s, error : %s",
                        render_data->owner_object()->name().c_str(),
                        error.c_str());
                shader_manager->getErrorShader()->render(mvp_matrix,
                        render_data);
            }
        }
        if (!render_data->cull_test()) {
            glEnable (GL_CULL_FACE);
        }
        if (render_data->offset()) {
            glDisable (GL_POLYGON_OFFSET_FILL);
        }
        if (!render_data->depth_test()) {
            glEnable (GL_DEPTH_TEST);
        }
        if (!render_data->alpha_blend()) {
            glEnable (GL_BLEND);
        }
    }
}

void Renderer::renderPostEffectData(
        std::shared_ptr<RenderTexture> render_texture,
        std::shared_ptr<PostEffectData> post_effect_data,
        std::shared_ptr<PostEffectShaderManager> post_effect_shader_manager) {
    try {
        switch (post_effect_data->shader_type()) {
        case PostEffectData::ShaderType::COLOR_BLEND_SHADER:
            post_effect_shader_manager->getColorBlendPostEffectShader()->render(
                    render_texture, post_effect_data,
                    post_effect_shader_manager->quad_vertices(),
                    post_effect_shader_manager->quad_uvs(),
                    post_effect_shader_manager->quad_triangles());
            break;
        case PostEffectData::ShaderType::HORIZONTAL_FLIP_SHADER:
            post_effect_shader_manager->getHorizontalFlipPostEffectShader()->render(
                    render_texture, post_effect_data,
                    post_effect_shader_manager->quad_vertices(),
                    post_effect_shader_manager->quad_uvs(),
                    post_effect_shader_manager->quad_triangles());
            break;
        default:
            post_effect_shader_manager->getCustomPostEffectShader(
                    post_effect_data->shader_type())->render(render_texture,
                    post_effect_data,
                    post_effect_shader_manager->quad_vertices(),
                    post_effect_shader_manager->quad_uvs(),
                    post_effect_shader_manager->quad_triangles());
            break;
        }
    } catch (std::string error) {
        LOGE("Error detected in Renderer::renderPostEffectData; error : %s",
                error.c_str());
    }
}


}
