//
// Created by MadiApps on 14/09/2021.
//

#ifndef NATIVE_LIB_H
#define NATIVE_LIB_H

#include "jni.h"

#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <unistd.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define  LOG_TAG    "NATIVE_LIB"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#endif //WEBVIEW_AR_NATIVE_LIB_H
