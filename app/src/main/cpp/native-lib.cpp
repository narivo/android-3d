#include "native-lib.h"

#include <unistd.h>
#include <string>
#include <optional>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include <model.h>
#include <animator.h>
#include "asset_extractor.h"

int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
glm::mat4 model(1.0f);
glm::mat4 projection;

Shader ourShader;
Model ourModel;
Animator animator;
Animation danceAnimation;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// time
std::chrono::steady_clock::time_point timeStart;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_NativeLibActivity_loadAssets(JNIEnv *env, jobject thiz, jobject mgr)  {
    AssetExtractor extractor(env, thiz, mgr);
    extractor.ExtractToCache();

    stbi_set_flip_vertically_on_load(false);
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
    change(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeSurfaceCreated(JNIEnv *env, jobject thiz) {

    timeStart = std::chrono::steady_clock::now();

    ourShader = Shader("vertex.glsl", "fragment.glsl");

    ourModel = Model("backpack/backpack.obj");
    danceAnimation = Animation("vampire/dancing_vampire.dae", &ourModel);
    animator = Animator(&danceAnimation);

    glEnable(GL_DEPTH_TEST);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_Renderer_nativeDrawFrame(JNIEnv *env, jobject thiz) {

    std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::now();
    float currentFrame = (std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count()) / 1000000.0;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    //animator.UpdateAnimation(deltaTime);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();
    // why after use ?
    // because we may use several Shader program
    // in one app !!

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    /*auto transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
*/
    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    ourShader.setMat4("model", model);
    ourModel.Draw(ourShader);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_GLView_nativeCameraRotation(JNIEnv *env, jobject thiz,
                                                       jfloat xoffset, jfloat yoffset) {
    camera.ProcessMouseMovement(-xoffset, yoffset);
}

