//
// Created by MadiApps on 01/10/2021.
//

#ifndef OBJECT_RENDERER_H
#define OBJECT_RENDERER_H

#include "arcore_c_api.h"

#include <unordered_map>
#include <model.h>
#include <shader.h>
#include <util.h>

#include <algorithm>
#include "logger.h"
#include <string>

class ObjectRenderer {
public:
    ObjectRenderer() {
        // stub
    }

    void addAngle(float angle) {
        angle_deg += angle;
        while (angle_deg > 360.0f) {
            angle_deg -= 360.0f;
        }
        while (angle_deg < 0.0f) {
            angle_deg += 360.0f;
        }
    }

    ObjectRenderer(ArSession *arSession, ArFrame *arFrame,
                   const char* vertex, const char* fragment,
                   const char* path2Object) {
        ar_frame_ = arFrame;
        ar_session_ = arSession;
        shader_program_ = Shader(std::string(vertex), fragment);
        model_to_render_  = Model(path2Object);
    }

    void Draw() {
        ArCamera* ar_camera;
        ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

        glm::mat4 view_mat;
        glm::mat4 projection_mat;
        ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat));
        ArCamera_getProjectionMatrix(ar_session_, ar_camera, /*near=*/0.1f, /*far=*/100.f, glm::value_ptr(projection_mat));

        ArTrackingState camera_tracking_state;
        ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);
        ArCamera_release(ar_camera);
        DrawAugmentedImage(view_mat, projection_mat);
    }

    friend void swap(ObjectRenderer &first, ObjectRenderer &second) {
        using std::swap;

        swap(first.shader_program_, second.shader_program_);
        swap(first.model_to_render_, second.model_to_render_);

        swap(first.augmented_image_map, second.augmented_image_map);

        swap(first.ar_session_, second.ar_session_);
        swap(first.ar_frame_, second.ar_frame_);

        swap(first.center_matrix, second.center_matrix);
    }

    ObjectRenderer(const ObjectRenderer &other) {
        shader_program_ = Shader(other.shader_program_);
        ar_session_ = other.ar_session_;
        ar_frame_ = other.ar_frame_;
    }

    ~ObjectRenderer() {
        ArSession_destroy(ar_session_);
        ArFrame_destroy(ar_frame_);
    }

    ObjectRenderer& operator=(ObjectRenderer other) {
        // do the swap
        swap(*this, other);

        return *this;
    }
private:
    Shader shader_program_;
    Model model_to_render_;
    ArSession *ar_session_ = NULL;
    ArFrame *ar_frame_ = NULL;
    std::unordered_map <int32_t, std::pair<ArAugmentedImage *, ArAnchor *>> augmented_image_map;

    float angle_deg = 0;
    glm::mat4 center_matrix;

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

    bool DrawAugmentedImage(const glm::mat4 &view_mat, const glm::mat4 &projection_mat) {
        bool found_ar_image = false;

        ArTrackableList* updated_image_list = NULL;
        ArTrackableList_create(ar_session_, &updated_image_list);
        CHECK(updated_image_list != NULL);
        ArFrame_getUpdatedTrackables(ar_session_, ar_frame_, AR_TRACKABLE_AUGMENTED_IMAGE, updated_image_list);

        int32_t image_list_size;
        ArTrackableList_getSize(ar_session_, updated_image_list, &image_list_size);

        // Find newly detected image, add it to map
        for (int i = 0; i < image_list_size; ++i) {
            ArTrackable* ar_trackable = nullptr;
            ArTrackableList_acquireItem(ar_session_, updated_image_list, i, &ar_trackable);
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
                        ArAugmentedImage_getCenterPose(ar_session_, image, scopedArPose.GetArPose());

                        ArAnchor* image_anchor = nullptr;
                        const ArStatus status = ArTrackable_acquireNewAnchor(
                                ar_session_, ar_trackable, scopedArPose.GetArPose(), &image_anchor);
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
            ArTrackable_getTrackingState(ar_session_, ArAsTrackable(ar_image), &tracking_state);

            // Draw this image frame.
            if (tracking_state == AR_TRACKING_STATE_TRACKING) {
                GetTransformMatrixFromAnchor(ar_session_, ar_anchor, &center_matrix);

                shader_program_.use();
                shader_program_.setMat4("projection", projection_mat);
                shader_program_.setMat4("view", view_mat);

                center_matrix = glm::translate(center_matrix, glm::vec3(0.0, -5.0, 0.0));
                center_matrix = glm::rotate(center_matrix, (float)glm::radians(270.0), glm::vec3(1.0, 0.0, 0.0));
                center_matrix = glm::rotate(center_matrix, (float)glm::radians(angle_deg), glm::vec3(0.0, 0.0, 1.0));

                shader_program_.setMat4("model", center_matrix);

                LOGE("Drawing");
                model_to_render_.Draw(shader_program_);
            }
        }

        return found_ar_image;
    }
};

#endif //OBJECT_RENDERER_H
