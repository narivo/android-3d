//
// Created by MadiApps on 27/09/2021.
//

#ifndef AR_UTIL_H
#define AR_UTIL_H

#include <jni.h>
#include <android/bitmap.h>
#include "native-lib.h"

#include "arcore_c_api.h"

#include "logger.h"

void ThrowJavaException(JNIEnv *env, const char *msg) {
  LOGE("Throw Java exception: %s", msg);
  jclass c = env->FindClass("java/lang/RuntimeException");
  env->ThrowNew(c, msg);
}

#ifndef CHECKANDTHROW
#define CHECKANDTHROW(condition, env, msg, ...)                            \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    ThrowJavaException(env, msg);                                    \
    return ##__VA_ARGS__;                                                  \
  }
#endif  // CHECKANDTHROW

#ifndef CHECK
#define CHECK(condition)                                                   \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    abort();                                                               \
  }
#endif  // CHECK

void CheckGlError(const char *operation) {
  GLint error = glGetError();
  if (error) {
    LOGE("after %s() glError (0x%x)\n", operation, error);
    abort();
  }
}

// Provides a scoped allocated instance of Anchor.
// Can be treated as an ArAnchor*.
class ScopedArPose {
public:
    explicit ScopedArPose(const ArSession* session) {
      ArPose_create(session, nullptr, &pose_);
    }
    ~ScopedArPose() { ArPose_destroy(pose_); }
    ArPose* GetArPose() { return pose_; }
    // Delete copy constructors.
    ScopedArPose(const ScopedArPose&) = delete;
    void operator=(const ScopedArPose&) = delete;

private:
    ArPose* pose_;
};

/*static jobject CallJavaLoadImage(JNIEnv* env, jstring image_path) {
  jclass jni_class_id
  return env->CallStaticObjectMethod(jni_class_id, jni_load_image_method_id,
                                     image_path);
}

bool LoadImageFromAssetManager(JNIEnv *env, const std::string &path, int *out_width,
                               int *out_height, int *out_stride,
                               uint8_t **out_pixel_buffer) {
  jstring j_path = env->NewStringUTF(path.c_str());
  jobject image_obj = CallJavaLoadImage(j_path);
  env->DeleteLocalRef(j_path);

  // image_obj contains a Bitmap Java object.
  AndroidBitmapInfo bitmap_info;
  AndroidBitmap_getInfo(env, image_obj, &bitmap_info);

  // Attention: We are only going to support RGBA_8888 format in this sample.
  CHECK(bitmap_info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);

  *out_width = bitmap_info.width;
  *out_height = bitmap_info.height;
  *out_stride = bitmap_info.stride;
  void *jvm_buffer = nullptr;
  CHECK(AndroidBitmap_lockPixels(env, image_obj, &jvm_buffer) ==
        ANDROID_BITMAP_RESULT_SUCCESS);

  // Copy jvm_buffer_address to pixel_buffer_address
  int32_t total_size_in_byte = bitmap_info.stride * bitmap_info.width;
  *out_pixel_buffer = new uint8_t[total_size_in_byte];
  memcpy(*out_pixel_buffer, jvm_buffer, total_size_in_byte);

  // release jvm_buffer back to JVM
  CHECK(AndroidBitmap_unlockPixels(env, image_obj) ==
        ANDROID_BITMAP_RESULT_SUCCESS);
  return true;
}*/

void ConvertRgbaToGrayscale(const uint8_t *image_pixel_buffer, int32_t width,
                            int32_t height, int32_t stride,
                            uint8_t **out_grayscale_buffer) {
  int32_t grayscale_stride = stride / 4;  // Only support RGBA_8888 format
  uint8_t *grayscale_buffer = new uint8_t[grayscale_stride * height];
  for (int h = 0; h < height; ++h) {
    for (int w = 0; w < width; ++w) {
      const uint8_t *pixel = &image_pixel_buffer[w * 4 + h * stride];
      uint8_t r = *pixel;
      uint8_t g = *(pixel + 1);
      uint8_t b = *(pixel + 2);
      grayscale_buffer[w + h * grayscale_stride] =
              static_cast<uint8_t>(0.213f * r + 0.715 * g + 0.072 * b);
    }
  }
  *out_grayscale_buffer = grayscale_buffer;
}

bool LoadFileFromAssetManager(AAssetManager *mgr, const char *file_name,
                              std::string *out_file_text_string) {
  // If the file hasn't been uncompressed, load it to the internal storage.
  // Note that AAsset_openFileDescriptor doesn't support compressed
  // files (.obj).
  AAsset *asset = AAssetManager_open(mgr, file_name, AASSET_MODE_STREAMING);
  if (asset == nullptr) {
    LOGE("Error opening asset %s", file_name);
    return false;
  }

  off_t file_size = AAsset_getLength(asset);
  out_file_text_string->resize(file_size);
  int ret = AAsset_read(asset, &out_file_text_string->front(), file_size);

  if (ret <= 0) {
    LOGE("Failed to open file: %s", file_name);
    AAsset_close(asset);
    return false;
  }

  AAsset_close(asset);
  return true;
}
#endif //WEBVIEW_AR_UTIL_H
