#include "native-lib.h"
#include "shader.h"
#include <optional>
#include "camera.h"
#include <model.h>

int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
glm::mat4 model(1.0f);
glm::mat4 projection;

AAssetManager* aAssetManager;
std::optional<Shader> ourShader;
std::optional<Model> ourModel;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

struct stat st = {0};

void extractFiles(const char *fqdname, const char *filename);

void extractDir(AAssetDir *assetDir, string dirname);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_NativeLibActivity_loadAssets(JNIEnv *env, jobject thiz, jobject mgr)  {

    aAssetManager = AAssetManager_fromJava(env, mgr);
    if (aAssetManager == nullptr) {
        LOGE("error loading asset manager");
    } else {
        LOGI( "loaded asset  manager");
    }

    jclass activityClass = env->GetObjectClass(thiz);
    // Get path to cache dir (/data/org.wikibooks.OpenGL/cache)
    jmethodID getCacheDir = env->GetMethodID(activityClass, "getExternalCacheDir", "()Ljava/io/File;");
    jobject file = env->CallObjectMethod(thiz, getCacheDir);

    jclass fileClass = env->FindClass("java/io/File");
    jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
    const char* app_dir = env->GetStringUTFChars(jpath, NULL);

    // chdir in the application cache directory
    LOGI("app_dir: %s", app_dir);
    chdir(app_dir);

    AAssetDir* assetDir = AAssetManager_openDir(aAssetManager, "");
    extractDir(assetDir, "");
    AAssetDir_close(assetDir);

    if(stat("backpack", &st) == -1) {
        mkdir("backpack", 0700);
    }

    string obj_dir = string(app_dir) + "/backpack";
    chdir(obj_dir.c_str());

    AAssetDir* backpackDir = AAssetManager_openDir(aAssetManager, "backpack");
    extractDir(backpackDir, "backpack");
    AAssetDir_close(backpackDir);

    chdir(app_dir);
    env->ReleaseStringUTFChars(jpath, app_dir);

    stbi_set_flip_vertically_on_load(true);
}

void extractDir(AAssetDir *assetDir, string dirname) {
    const char* filename = (const char*)NULL;
    while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
        if(dirname.empty())
            extractFiles(filename, filename);
        else {
            string fqdname = dirname+"/"+string(filename);
            extractFiles(fqdname.c_str(), filename);
        }
    }
}

void extractFiles(const char *filename, const char* rawfilename) {
    AAsset* asset = AAssetManager_open(aAssetManager, filename, AASSET_MODE_STREAMING);
    char buf[BUFSIZ];
    int nb_read = 0;
    FILE* out = fopen(rawfilename, "w+");
    LOGI("error :  %s", strerror(errno));
    while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
        fwrite(buf, nb_read, 1, out);
    fclose(out);
    AAsset_close(asset);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_webviewar_NativeLibActivity_getHello(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++";
    LOGI("I am working from the c++ world");
    return env->NewStringUTF(hello.c_str());
}

unsigned int loadTexture(JNIEnv *env, jobject thiz, const char *path, unsigned int activeTex) {
    // loading textures in the JVM env

    jclass glUtilsKlass = env->FindClass("com/example/webviewar/GLUtils");
    jmethodID loadTexture = env->GetStaticMethodID(glUtilsKlass, "loadTexture", "(Ljava/lang/String;I)I");

    if (loadTexture == NULL) {
        LOGE("Couldn't find loadTexture method");
    }

    jstring pathStr = env->NewStringUTF(path);

    unsigned int textureID = env->CallStaticIntMethod(glUtilsKlass, loadTexture,
                                                pathStr, (int) activeTex);

    env->DeleteLocalRef(pathStr);

    return textureID;
}

void setupNDC() {
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    ourShader->setMat4("model", model);
    ourShader->setMat4("view", view);
    ourShader->setMat4("projection", projection);
}

void change(unsigned int width, unsigned int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeSurfaceChanged(JNIEnv *env, jobject thiz,
                                                         jint width, jint height) {
    if(width > height) {
        change(width, height);
    } else {
        change(height, width);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeSurfaceCreated(JNIEnv *env, jobject thiz) {

    //gl3stubInit();

    ourShader = Shader(env, thiz,
                       "vertex.glsl", "fragment.glsl");
    ourModel = Model("backpack/backpack.obj");

    setupNDC();
    glEnable(GL_DEPTH_TEST);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeDrawFrame(JNIEnv *env, jobject thiz) {

    glClearColor(0.75f, 0.52f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader->use();
    // why after use ?
    // because we may use several Shader program
    // in one app !!

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    ourShader->setMat4("projection", projection);
    ourShader->setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    ourShader->setMat4("model", model);
    ourModel->Draw(ourShader);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_GLView_nativeCameraRotation(JNIEnv *env, jobject thiz,
                                                       jfloat xoffset, jfloat yoffset) {
    camera.ProcessMouseMovement(-xoffset, yoffset);
}

