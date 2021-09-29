#include "jni.h"

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include "shader.h"
#include "asset_extractor.h"
#include <optional>
#include <util.h>
#include "logger.h"

#include <unordered_map>

#include <model.h>

#include "arcore_c_api.h"
#include "viewfinder_renderer.h"
//
// Created by MadiApps on 27/09/2021.
//

AAssetManager* asset_manager_ = NULL;

ArSession* ar_session_ = NULL;
ArFrame* ar_frame_ = NULL;

int display_rotation_ = 0;
int width_ = 1;
int height_ = 1;

std::unordered_map<int32_t, std::pair<ArAugmentedImage*, ArAnchor*>> augmented_image_map;

constexpr bool kUseSingleImage = false;

ViewFinder viewFinder;
Model backpackModel;
Shader backpackShader;

bool DrawAugmentedImage(const glm::mat4&, const glm::mat4&);

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
Java_com_example_webviewar_ARActivity_loadAssets(JNIEnv* env, jobject thiz,
                                                 jobject asset_manager) {
    asset_manager_ = AAssetManager_fromJava(env, asset_manager);
    AssetExtractor extractor(env, thiz, asset_manager);
    extractor.ExtractToCache();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeSurfaceCreated(JNIEnv* env, jobject thiz) {
    viewFinder = ViewFinder(ar_session_, ar_frame_);
    viewFinder.Prepare();

    backpackShader = Shader("vertex.glsl", "fragment.glsl");
    backpackModel  = Model("backpack/backpack.obj");
}

void GetTransformMatrixFromAnchor(const ArSession* ar_session,
                                  const ArAnchor* ar_anchor,
                                  glm::mat4* out_model_mat) {
    if (out_model_mat == nullptr) {
        LOGE("util::GetTransformMatrixFromAnchor model_mat is null.");
        return;
    }
    ScopedArPose pose(ar_session);
    ArAnchor_getPose(ar_session, ar_anchor, pose.GetArPose());
    ArPose_getMatrix(ar_session, pose.GetArPose(),
                     glm::value_ptr(*out_model_mat));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeDrawFrame(JNIEnv* env, jobject thiz) {

    // Render the scene.
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // Textures are loaded with premultiplied alpha
    // (https://developer.android.com/reference/android/graphics/BitmapFactory.Options#inPremultiplied),
    // so we use the premultiplied alpha blend factors.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (ar_session_ == nullptr) return;

    // Update session to get current frame and render camera background.
    if (ArSession_update(ar_session_, ar_frame_) != AR_SUCCESS) {
        LOGE("AugmentedImageApplication::OnDrawFrame ArSession_update error");
    }

    // ============================================================ //
    //                 Camera ViewFinder picture                    //
    // ============================================================ //
    viewFinder.Draw();

    // ============================================================ //
    //                     3D augmented Image                       //
    // ============================================================ //
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
    DrawAugmentedImage(view_mat, projection_mat);
}

bool DrawAugmentedImage(const glm::mat4& view_mat, const glm::mat4& projection_mat) {
    bool found_ar_image = false;

    ArTrackableList* updated_image_list = NULL;
    ArTrackableList_create(ar_session_, &updated_image_list);
    CHECK(updated_image_list != NULL);
    ArFrame_getUpdatedTrackables(
            ar_session_, ar_frame_, AR_TRACKABLE_AUGMENTED_IMAGE, updated_image_list);

    int32_t image_list_size;
    ArTrackableList_getSize(ar_session_, updated_image_list, &image_list_size);

    // Find newly detected image, add it to map
    for (int i = 0; i < image_list_size; ++i) {
        ArTrackable* ar_trackable = nullptr;
        ArTrackableList_acquireItem(ar_session_, updated_image_list, i,
                                    &ar_trackable);
        ArAugmentedImage* image = ArAsAugmentedImage(ar_trackable);

        ArTrackingState tracking_state;
        ArTrackable_getTrackingState(ar_session_, ar_trackable, &tracking_state);

        int image_index;
        ArAugmentedImage_getIndex(ar_session_, image, &image_index);

        switch (tracking_state) {
            case AR_TRACKING_STATE_PAUSED:
                // When an image is in PAUSED state but the camera is not PAUSED,
                // that means the image has been detected but not yet tracked.
                LOGI("Detected Image %d", image_index);
                break;
            case AR_TRACKING_STATE_TRACKING:
                found_ar_image = true;

                if (augmented_image_map.find(image_index) ==  augmented_image_map.end()) {
                    // Record the image and its anchor.
                    ScopedArPose scopedArPose(ar_session_);
                    ArAugmentedImage_getCenterPose(ar_session_, image,
                                                   scopedArPose.GetArPose());

                    ArAnchor* image_anchor = nullptr;
                    const ArStatus status = ArTrackable_acquireNewAnchor(
                            ar_session_, ar_trackable, scopedArPose.GetArPose(),
                            &image_anchor);
                    CHECK(status == AR_SUCCESS);

                    // Now we have an Anchor, record this image.
                    augmented_image_map[image_index] =
                            std::pair<ArAugmentedImage*, ArAnchor*>(image, image_anchor);
                }
                break;

            case AR_TRACKING_STATE_STOPPED: {
                std::pair<ArAugmentedImage*, ArAnchor*> record =
                        augmented_image_map[image_index];
                ArTrackable_release(ArAsTrackable(record.first));
                ArAnchor_release(record.second);
                augmented_image_map.erase(image_index);
            } break;

            default:
                break;
        }  // End of switch (tracking_state)
    }    // End of for (int i = 0; i < image_list_size; ++i) {

    ArTrackableList_destroy(updated_image_list);
    updated_image_list = nullptr;

    // Display all augmented images in augmented_image_map.
    for (const auto& it : augmented_image_map) {
        const std::pair<ArAugmentedImage*, ArAnchor*>& record = it.second;
        ArAugmentedImage* ar_image = record.first;
        ArAnchor* ar_anchor = record.second;
        ArTrackingState tracking_state;
        ArTrackable_getTrackingState(ar_session_, ArAsTrackable(ar_image),
                                     &tracking_state);

        // Draw this image frame.
        if (tracking_state == AR_TRACKING_STATE_TRACKING) {
            glm::mat4 center_matrix;
            GetTransformMatrixFromAnchor(ar_session_, ar_anchor, &center_matrix);

            backpackShader.use();
            backpackShader.setMat4("projection", projection_mat);
            backpackShader.setMat4("view", view_mat);
            center_matrix = glm::rotate(center_matrix, (float)glm::radians(90.0), glm::vec3(1.0,0.0, 0.0));
            center_matrix = glm::translate(center_matrix, glm::vec3(0.0,0.0, 3.0));
            backpackShader.setMat4("model", center_matrix);

            LOGE("Drawing");
            backpackModel.Draw(backpackShader);
        }
    }

    return found_ar_image;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_ARActivity_nativeSurfaceChanged(JNIEnv* env, jobject thiz,
                                                           jint display_rotation, jint w, jint h) {
    viewFinder.Change(display_rotation, w, h);
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
