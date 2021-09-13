//
// Created by MadiApps on 07/09/2021.
//
#include "jni.h"
#include <string>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <stb_image.h>
#include <unistd.h>

#if DYNAMIC_ES3
#include "gl3stub.h"
#else
// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#endif

#include <EGL/egl.h>

#define  LOG_TAG    "NATIVE_LIB"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

AAssetManager* aAssetManager;
float vertices[] = {
         0.5f,  0.5f, 0.0f, //0
         0.5f, -0.5f, 0.0f, //1
        -0.5f, -0.5f, 0.0f, //2
        -0.5f,  0.5f, 0.0f  //3
};
unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
};

unsigned int VAO;

static const char vertexShaderSource[] =
        "#version 300 es\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n";

static const char fragmentShaderSource[] =
       "#version 300 es\n"
       "precision mediump float;\n"
       "out vec4 FragColor;\n"
       "void main()\n"
       "{\n"
       "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
       "}\n";

unsigned int create_compile_shader(GLenum gl_type, const char* source) {
    unsigned int shader = glCreateShader(gl_type);
    glShaderSource(shader, /* how many strings ? */1, &source, nullptr);

    // compiling
    glCompileShader(shader);
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success == false) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        char errorLog[] { "ERROR::SHADER::COMPILATION_FAILED\n" };
        LOGE("%s", infoLog);
        throw errorLog;
    }

    return shader;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_webviewar_NativeLibActivity_getHello(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++";
    LOGI("I am working from the c++ world");
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_NativeLibActivity_load(JNIEnv *env, jobject thiz, jobject mgr)  {

    aAssetManager = AAssetManager_fromJava(env, mgr);
    if (aAssetManager == NULL) {
        LOGE("error loading asset manager");
    } else {
        LOGI( "loaded asset  manager");
    }

    jclass activityClass = env->GetObjectClass(thiz);
    // Get path to cache dir (/data/data/org.wikibooks.OpenGL/cache)
    jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
    jobject file = env->CallObjectMethod(thiz, getCacheDir);

    jclass fileClass = env->FindClass("java/io/File");
    jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
    const char* app_dir = env->GetStringUTFChars(jpath, NULL);

    // chdir in the application cache directory
    LOGI("app_dir: %s", app_dir);
    chdir(app_dir);
    env->ReleaseStringUTFChars(jpath, app_dir);

    AAssetDir* assetDir = AAssetManager_openDir(aAssetManager, "");
    const char* filename = (const char*)NULL;
    while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
        AAsset* asset = AAssetManager_open(aAssetManager, filename, AASSET_MODE_STREAMING);
        char buf[BUFSIZ];
        int nb_read = 0;
        FILE* out = fopen(filename, "w+");
        LOGI("error :  %s", strerror(errno));
        while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
            fwrite(buf, nb_read, 1, out);
        fclose(out);
        AAsset_close(asset);
    }
    AAssetDir_close(assetDir);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeSurfaceChanged(JNIEnv *env, jobject thiz, jint width,
                                                         jint height) {
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeSurfaceCreated(JNIEnv *env, jobject thiz) {

    gl3stubInit();

    unsigned int vertexShader = create_compile_shader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = create_compile_shader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // the shader program
    unsigned int shaderProgram;

    // if we get here;
    // shaders compiled, no worries
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // linking
    glLinkProgram(shaderProgram);
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_COMPILE_STATUS, &success);
    if(success == false) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        char errorLog[] { "ERROR::PROGRAM::LINKING_FAILED\n" };
        strncat(errorLog, infoLog, 512);
        throw errorLog;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // our VBO, VAO
    unsigned int VBO, EBO;
    // generation
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    // binding
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBindVertexArray(VAO);
    // mapping
    // the current buffer data is set to the latest bound VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // 0 refers to the `layout (location = 0)` attribute in both
    // glVertexAttribPointer and glEnableVertexAttribArray
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // release
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0); // not necessary, in general, don't do this

    glUseProgram(shaderProgram);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeDrawFrame(JNIEnv *env, jobject thiz) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
}

