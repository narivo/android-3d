#include "shader.h"

Shader::Shader(JNIEnv* env, jobject thiz,
        std::string vertexPath, const char* fragmentPath) {
    // creating program in the JVM env
    jclass glUtilsKlass = env->FindClass("com/example/webviewar/GLUtils");
    jmethodID createProgram = env->GetStaticMethodID(glUtilsKlass, "createProgram",
                                                     "(Ljava/lang/String;Ljava/lang/String;)I");

    jstring vertexStr = env->NewStringUTF(vertexPath.c_str());
    jstring fragmentStr = env->NewStringUTF(fragmentPath);

    ID = env->CallStaticIntMethod(glUtilsKlass, createProgram,
                            vertexStr, fragmentStr);

    env->DeleteLocalRef(vertexStr);
    env->DeleteLocalRef(fragmentStr);
}