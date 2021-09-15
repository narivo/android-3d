#include "native-lib.h"
#include "shader.h"

float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
};

AAssetManager* aAssetManager;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_NativeLibActivity_load(JNIEnv *env, jobject thiz, jobject mgr)  {
    aAssetManager = AAssetManager_fromJava(env, mgr);
    if (aAssetManager == NULL) {
        LOGE("error loading asset manager");
    } else {
        LOGI( "loaded asset  manager");
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_webviewar_NativeLibActivity_getHello(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++";
    LOGI("I am working from the c++ world");
    return env->NewStringUTF(hello.c_str());
}

unsigned int loadTexture(JNIEnv *env, jobject thiz, const char *path, unsigned int activeTex) {
    unsigned int textureId = 0;

    jclass activityClass = env->GetObjectClass(thiz);
    jmethodID loadTexture = env->GetMethodID(activityClass, "loadTexture",
                                                    "(Ljava/lang/String;)I");
    if (loadTexture == NULL) {
        LOGE("Couldn't find loadTexture method");
        return (unsigned int) -1;
    }
    jstring pathStr = env->NewStringUTF(path);
    textureId = (unsigned int) env->CallObjectMethod(activityClass, loadTexture, pathStr, activeTex);
    env->DeleteLocalRef(pathStr);
    return textureId;
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

    //gl3stubInit();

    Shader ourShader(env, thiz,
                     "vertex.glsl", "fragment.glsl");

    // our VBO, VAO
    unsigned int VAO, VBO, EBO, texture1, texture2;;
    // generation
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    // binding
    glBindVertexArray(VAO);
    // bind VBO, EBO after binding VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // mapping
    // the current buffer data is set to the latest bound VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 0 refers to the `layout (location = 0)` attribute in both
    // glVertexAttribPointer and glEnableVertexAttribArray
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // release
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0); // not necessary, in general, don't do this

    ourShader.use();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeDrawFrame(JNIEnv *env, jobject thiz) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
}

