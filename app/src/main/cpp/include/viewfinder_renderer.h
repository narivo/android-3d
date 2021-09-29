//
// Created by MadiApps on 27/09/2021.
//

#ifndef VIEWFINDER_RENDERER_H
#define VIEWFINDER_RENDERER_H

#include <optional>
#include "shader.h"
#include "logger.h"

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <algorithm>
#include "arcore_c_api.h"

class ViewFinder{
public:
    void Prepare() {
        shader_program_ = Shader("screenquad.vert", "screenquad.frag");

        if (!shader_program_.ID) {
            LOGE("Could not create program.");
        }

        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uniform_texture_ = glGetUniformLocation(shader_program_.ID, "sTexture");
        attribute_vertices_ = glGetAttribLocation(shader_program_.ID, "a_Position");
        attribute_uvs_ = glGetAttribLocation(shader_program_.ID, "a_TexCoord");
    }

    void Draw() {
        static_assert(std::extent<decltype(kVertices)>::value == kNumVertices * 2,
                      "Incorrect kVertices length");
        // ========================================================================= //
        //                                   Session                                 //
        // ========================================================================= //
        ArSession_setCameraTextureName(ar_session_, texture_id_);

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
            LOGE("supress");
            return;
        }

        shader_program_.use();
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

    void Change(int display_rotation, int width, int height) {
        glViewport(0, 0, width, height);
        display_rotation_ = display_rotation;
        width_ = width;
        height_ = height;
        if (ar_session_ != nullptr) {
            ArSession_setDisplayGeometry(ar_session_, display_rotation, width, height);
        }
    }

    friend void swap(ViewFinder& first, ViewFinder& second) {
        using std::swap;

        swap(first.shader_program_, second.shader_program_);
        swap(first.texture_id_, second.texture_id_);
        swap(first.uniform_texture_, second.uniform_texture_);
        swap(first.attribute_vertices_, second.attribute_vertices_);
        swap(first.attribute_uvs_, second.attribute_uvs_);

        swap(first.display_rotation_, second.display_rotation_);
        swap(first.width_, second.width_);
        swap(first.height_, second.height_);

        swap(first.ar_session_, second.ar_session_);
        swap(first.ar_frame_, second.ar_frame_);
    }

    ViewFinder(): shader_program_() {
        // stub
    }

    ViewFinder(ArSession* arSession, ArFrame* arFrame) {
        ar_frame_ = arFrame;
        ar_session_ = arSession;
    }

    ViewFinder(const ViewFinder& other) {
        shader_program_ = Shader(other.shader_program_);
        texture_id_ = other.texture_id_;
        uniform_texture_ = other.uniform_texture_;
        attribute_uvs_ = other.attribute_uvs_;
        attribute_vertices_ = other.attribute_vertices_;

        display_rotation_ = other.display_rotation_;
        width_ = other.width_;
        height_ = other.height_;

        ar_session_ = other.ar_session_;
        ar_frame_ = other.ar_frame_;
    }

    ViewFinder& operator=(ViewFinder other) {
        // do the swap
        swap(*this, other);

        return *this;
    }

    ~ViewFinder() {
        ArSession_destroy(ar_session_);
        ArFrame_destroy(ar_frame_);
    }
private:
    Shader shader_program_;
    unsigned int texture_id_;
    unsigned int uniform_texture_;
    unsigned int attribute_vertices_;
    unsigned int attribute_uvs_;

    // Positions of the quad vertices in clip space (X, Y).
    const float kVertices[8] = {
            -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f,
    };

    bool uvs_initialized_ = false;
    static constexpr int kNumVertices = 4;
    float transformed_uvs_[kNumVertices * 2];

    // display
    int display_rotation_ = 0;
    int width_ = 1;
    int height_ = 1;

    // session
    ArSession* ar_session_ = NULL;
    ArFrame* ar_frame_ = NULL;
};

#endif //VIEWFINDER_RENDERER_H
