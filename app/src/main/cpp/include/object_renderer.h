//
// Created by MadiApps on 01/10/2021.
//

#ifndef OBJECT_RENDERER_H
#define OBJECT_RENDERER_H

#include "arcore_c_api.h"

#include <unordered_map>
#include "model.h"
#include "shader.h"
#include "util.h"

#include <algorithm>
#include "logger.h"
#include <string>

#include "animator.h"

using namespace aevv_graphics;

namespace aevv_renderers {

    class ObjectRenderer {
    public:

        void addAngle(float angle) {
            angle_deg_ += angle;
            while (angle_deg_ > 360.0f) {
                angle_deg_ -= 360.0f;
            }
            while (angle_deg_ < 0.0f) {
                angle_deg_ += 360.0f;
            }
        }

        ObjectRenderer(ArSession *arSession, ArFrame *arFrame,
                       const char *vertex, const char *fragment,
                       const char *path2Object) {
            time_start = std::chrono::steady_clock::now();

            ar_frame_ = arFrame;
            ar_session_ = arSession;
            shader_program_ = new Shader(std::string(vertex), fragment);
            model_to_render_ = new Model(path2Object);

            dance_animation_ = new Animation(path2Object, model_to_render_);
            animator_ = new Animator(dance_animation_);
        }

        void Draw() {
            std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::now();
            float currentFrame = (std::chrono::duration_cast<std::chrono::microseconds>(
                    timeEnd - time_start).count()) / 1000000.0;
            delta_time_ = currentFrame - last_frame_;
            last_frame_ = currentFrame;

            ArCamera *ar_camera;
            ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

            glm::mat4 view_mat;
            glm::mat4 projection_mat;
            ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat));
            ArCamera_getProjectionMatrix(ar_session_, ar_camera, /*near=*/0.1f, /*far=*/100.f,
                                         glm::value_ptr(projection_mat));

            ArTrackingState camera_tracking_state;
            ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);
            ArCamera_release(ar_camera);

            DrawAugmentedImage(view_mat, projection_mat);
        }

        ~ObjectRenderer() {
            ArSession_destroy(ar_session_);
            ArFrame_destroy(ar_frame_);

            delete shader_program_;
            delete model_to_render_;
            delete dance_animation_;
            delete animator_;
        }

    private:
        Shader* shader_program_;
        Model* model_to_render_;
        ArSession* ar_session_ = NULL;
        ArFrame* ar_frame_ = NULL;
        std::unordered_map<int32_t, std::pair<ArAugmentedImage *, ArAnchor *>> augmented_image_map;

        Animator* animator_;
        Animation* dance_animation_;

// time
        std::chrono::steady_clock::time_point time_start;

// timing
        float delta_time_ = 0.0f;    // time between current frame and last frame
        float last_frame_ = 0.0f;

        float angle_deg_ = 0;
        glm::mat4 center_matrix_;

        void GetTransformMatrixFromAnchor(const ArSession *ar_session,
                                          const ArAnchor *ar_anchor,
                                          glm::mat4 *out_model_mat) {
            if (out_model_mat == nullptr) {
                LOGE("util::GetTransformMatrixFromAnchor model_mat is null.");
                return;
            }
            ScopedArPose pose(ar_session);
            ArAnchor_getPose(ar_session, ar_anchor, pose.GetArPose());
            ArPose_getMatrix(ar_session, pose.GetArPose(),
                             glm::value_ptr(*out_model_mat));
        }

        bool DrawAugmentedImage(const glm::mat4 &view_mat, const glm::mat4 &projection_mat) {
            bool found_ar_image = false;

            ArTrackableList *updated_image_list = NULL;
            ArTrackableList_create(ar_session_, &updated_image_list);
            CHECK(updated_image_list != NULL);
            ArFrame_getUpdatedTrackables(ar_session_, ar_frame_, AR_TRACKABLE_AUGMENTED_IMAGE,
                                         updated_image_list);

            int32_t image_list_size;
            ArTrackableList_getSize(ar_session_, updated_image_list, &image_list_size);

            // Find newly detected image, add it to map
            for (int i = 0; i < image_list_size; ++i) {
                ArTrackable *ar_trackable = nullptr;
                ArTrackableList_acquireItem(ar_session_, updated_image_list, i, &ar_trackable);
                ArAugmentedImage *image = ArAsAugmentedImage(ar_trackable);

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

                        if (augmented_image_map.find(image_index) == augmented_image_map.end()) {
                            // Record the image and its anchor.
                            ScopedArPose scopedArPose(ar_session_);
                            ArAugmentedImage_getCenterPose(ar_session_, image,
                                                           scopedArPose.GetArPose());

                            ArAnchor *image_anchor = nullptr;
                            const ArStatus status = ArTrackable_acquireNewAnchor(
                                    ar_session_, ar_trackable, scopedArPose.GetArPose(),
                                    &image_anchor);
                            CHECK(status == AR_SUCCESS);

                            // Now we have an Anchor, record this image.
                            augmented_image_map[image_index] =
                                    std::pair<ArAugmentedImage *, ArAnchor *>(image, image_anchor);
                        }
                        break;

                    case AR_TRACKING_STATE_STOPPED: {
                        std::pair<ArAugmentedImage *, ArAnchor *> record =
                                augmented_image_map[image_index];
                        ArTrackable_release(ArAsTrackable(record.first));
                        ArAnchor_release(record.second);
                        augmented_image_map.erase(image_index);
                    }
                        break;

                    default:
                        break;
                }  // End of switch (tracking_state)
            }    // End of for (int i = 0; i < image_list_size; ++i) {

            ArTrackableList_destroy(updated_image_list);
            updated_image_list = nullptr;

            // Display all augmented images in augmented_image_map.
            for (const auto &it : augmented_image_map) {
                const std::pair<ArAugmentedImage *, ArAnchor *> &record = it.second;
                ArAugmentedImage *ar_image = record.first;
                ArAnchor *ar_anchor = record.second;
                ArTrackingState tracking_state;
                ArTrackable_getTrackingState(ar_session_, ArAsTrackable(ar_image), &tracking_state);

                // Draw this image frame.
                if (tracking_state == AR_TRACKING_STATE_TRACKING) {
                    GetTransformMatrixFromAnchor(ar_session_, ar_anchor, &center_matrix_);

                    animator_->UpdateAnimation(delta_time_);

                    shader_program_->use();
                    shader_program_->setMat4("projection", projection_mat);
                    shader_program_->setMat4("view", view_mat);

                    auto transforms = animator_->GetFinalBoneMatrices();
                    for (int i = 0; i < transforms.size(); ++i) {
                        shader_program_->setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                                                 transforms[i]);
                    }

                    center_matrix_ = glm::translate(center_matrix_, glm::vec3(0.0f, 0.0f, 0.0625f));
                    center_matrix_ = glm::rotate(center_matrix_, (float) glm::radians(270.0f),
                                                 glm::vec3(1.0f, 0.0f, 0.0f));
                    center_matrix_ = glm::rotate(center_matrix_, (float) glm::radians(angle_deg_),
                                                 glm::vec3(0.0f, 0.0f, 1.0f));
                    center_matrix_ = glm::scale(center_matrix_, glm::vec3(0.125f, 0.125f, 0.125f)); // 0.0625

                    shader_program_->setMat4("model", center_matrix_);

                    /*glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
                    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
                    shader_program_.setMat4("model", model);*/

                    LOGD("Drawing");
                    LOGD("[Renderer] Shader program : %d", shader_program_->ID);
                    model_to_render_->Draw(*shader_program_);
                }
            }

            return found_ar_image;
        }
    };
}
#endif //OBJECT_RENDERER_H
