#include "jni.h"

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include "shader.h"
#include "asset_extractor.h"
#include <optional>
#include <util.h>
#include "logger.h"

#include "arcore_c_api.h"
#include "viewfinder_renderer.h"
//
// Created by MadiApps on 27/09/2021.
//

std::optional<Shader> shader_program_;
unsigned int texture_id_;
unsigned int uniform_texture_;
unsigned int attribute_vertices_;
unsigned int attribute_uvs_;

// Positions of the quad vertices in clip space (X, Y).
const GLfloat kVertices[] = {
        -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f,
};

AAssetManager* asset_manager_ = NULL;

ArSession* ar_session_ = NULL;
ArFrame* ar_frame_ = NULL;

int display_rotation_ = 0;
int width_ = 1;
int height_ = 1;

bool uvs_initialized_ = false;
static constexpr int kNumVertices = 4;
float transformed_uvs_[kNumVertices * 2];

constexpr bool kUseSingleImage = false;

ArAugmentedImageDatabase* CreateAugmentedImageDatabase() {
    ArAugmentedImageDatabase* ar_augmented_image_database = nullptr;
    // There are two ways to configure a ArAugmentedImageDatabase:
    // 1. Add Bitmap to DB directly
    // 2. Load a pre-built AugmentedImageDatabase
    // Option 2) has
    // * shorter setup time
    // * doesn't require images to be packaged in apk.
    if (kUseSingleImage) {

    } else {
        std::string database_buffer;
        LoadFileFromAssetManager(asset_manager_, "sample_database.imgdb",
                                       &database_buffer);

        uint8_t* raw_buffer = reinterpret_cast<uint8_t*>(&database_buffer.front());
        const ArStatus status = ArAugmentedImageDatabase_deserialize(
                ar_session_, raw_buffer, database_buffer.size(),
                &ar_augmented_image_database);
        CHECK(status == AR_SUCCESS);
    }

    return ar_augmented_image_database;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_loadAssets(JNIEnv* env, jobject thiz, jobject asset_manager) {
    asset_manager_ = AAssetManager_fromJava(env, asset_manager);
    AssetExtractor extractor(env, thiz, asset_manager);
    extractor.ExtractToCache();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeSurfaceCreated(JNIEnv* env, jobject thiz) {
    shader_program_ = Shader("screenquad.vert", "screenquad.frag");

    if (!shader_program_) {
    LOGE("Could not create program.");
    }

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uniform_texture_ = glGetUniformLocation(shader_program_->ID, "sTexture");
    attribute_vertices_ = glGetAttribLocation(shader_program_->ID, "a_Position");
    attribute_uvs_ = glGetAttribLocation(shader_program_->ID, "a_TexCoord");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeDrawFrame(JNIEnv* env, jobject thiz) {
    static_assert(std::extent<decltype(kVertices)>::value == kNumVertices * 2,
                  "Incorrect kVertices length");
    // ========================================================================= //
    //                                   Session                                 //
    // ========================================================================= //
    if (ar_session_ == nullptr) return;

    ArSession_setCameraTextureName(ar_session_, texture_id_);

    // Update session to get current frame and render camera background.
    if (ArSession_update(ar_session_, ar_frame_) != AR_SUCCESS) {
        LOGE("AugmentedImageApplication::OnDrawFrame ArSession_update error");
    }

    ArCamera* ar_camera;
    ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

    glm::mat4 view_mat;
    glm::mat4 projection_mat;
    ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat));
    ArCamera_getProjectionMatrix(ar_session_, ar_camera,
            /*near=*/0.1f, /*far=*/100.f,
                                 glm::value_ptr(projection_mat));

    ArTrackingState camera_tracking_state;
    ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);
    ArCamera_release(ar_camera);

    // ========================================================================= //
    //                                   Drawing                                 //
    // ========================================================================= //
    // If display rotation changed (also includes view size change), we need to
    // re-query the uv coordinates for the on-screen portion of the camera image.
    int32_t geometry_changed = 0;
    ArFrame_getDisplayGeometryChanged(ar_session_, ar_frame_, &geometry_changed);
    if (geometry_changed != 0 || !uvs_initialized_) {
        ArFrame_transformCoordinates2d(
                ar_session_, ar_frame_, AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
                kNumVertices, kVertices, AR_COORDINATES_2D_TEXTURE_NORMALIZED,
                transformed_uvs_);
        uvs_initialized_ = true;
    }

    int64_t frame_timestamp;
    ArFrame_getTimestamp(ar_session_, ar_frame_, &frame_timestamp);
    if (frame_timestamp == 0) {
        // Suppress rendering if the camera did not produce the first frame yet.
        // This is to avoid drawing possible leftover data from previous sessions if
        // the texture is reused.
        return;
    }

    glUseProgram(shader_program_->ID);
    glDepthMask(GL_FALSE);

    glUniform1i(uniform_texture_, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);

    glEnableVertexAttribArray(attribute_vertices_);
    glVertexAttribPointer(attribute_vertices_, 2, GL_FLOAT, GL_FALSE, 0, kVertices);

    glEnableVertexAttribArray(attribute_uvs_);
    glVertexAttribPointer(attribute_uvs_, 2, GL_FLOAT, GL_FALSE, 0, transformed_uvs_);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
    glDepthMask(GL_TRUE);
    CheckGlError("BackgroundRenderer::Draw() error");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeSurfaceChanged(JNIEnv* env, jobject thiz, jint display_rotation, jint w, jint h) {
    glViewport(0, 0, w, h);
    display_rotation_ = display_rotation;
    width_ = w;
    height_ = h;
    if (ar_session_ != nullptr) {
        ArSession_setDisplayGeometry(ar_session_, display_rotation, w, h);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeActivityPause(JNIEnv *env, jobject thiz) {
    if (ar_session_ != nullptr) {
        ArSession_pause(ar_session_);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeActivityResume(JNIEnv *env, jobject thiz) {

    // ================================================= //
    //                  create session                   //
    // ================================================= //
    if(ar_session_ == NULL) {
        CHECKANDTHROW(ArSession_create(env, thiz, &ar_session_) == AR_SUCCESS, env, "Failed to create AR session.");
        ArConfig* ar_config = NULL;
        ArConfig_create(ar_session_, &ar_config);
        CHECK(ar_config);

        ArAugmentedImageDatabase* ar_augmented_image_database =
                CreateAugmentedImageDatabase();
        ArConfig_setAugmentedImageDatabase(ar_session_, ar_config,
                                           ar_augmented_image_database);

        ArConfig_setFocusMode(ar_session_, ar_config, AR_FOCUS_MODE_AUTO);
        CHECKANDTHROW(ArSession_configure(ar_session_, ar_config) == AR_SUCCESS,
                      env, "Failed to configure AR session");

        ArAugmentedImageDatabase_destroy(ar_augmented_image_database);
        ArConfig_destroy(ar_config);

        ArFrame_create(ar_session_, &ar_frame_);

        ArSession_setDisplayGeometry(ar_session_, display_rotation_, width_, height_);
    }

    const ArStatus status = ArSession_resume(ar_session_);
    CHECKANDTHROW(status == AR_SUCCESS, env, "Failed to resume AR session.");
}
