#include "native-lib.h"
#include "shader.h"
#include <optional>
#include "camera.h"

float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};
unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
};
glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
};
glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
};
glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.6f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0, 0.0),
        glm::vec3(0.2f, 0.2f, 1.0f)
};
int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
glm::mat4 model(1.0f);
glm::mat4 projection;
AAssetManager* aAssetManager;
std::optional<Shader> ourShader;
std::optional<Shader> lightShader;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch, fov = 45.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

unsigned int VBO, VAO, EBO, texture1, texture2, texture3, lightVAO;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_NativeLibActivity_load(JNIEnv *env, jobject thiz, jobject mgr)  {
    aAssetManager = AAssetManager_fromJava(env, mgr);
    if (aAssetManager == nullptr) {
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
    // loading textures in the JVM env

    jclass rendererKlass = env->GetObjectClass(thiz);
    jfieldID sharedTextureID = env->GetFieldID(rendererKlass, "sharedTexture",
                                              "com/example/webviewar/Texture");
    jobject sharedTextureObj = env->GetObjectField(thiz, sharedTextureID);
    jclass textureKlass = env->GetObjectClass(sharedTextureObj);
    jmethodID loadTexture = env->GetMethodID(textureKlass, "loadTexture",
                                               "(Ljava/lang/String;I)I");
    if (loadTexture == NULL) {
        LOGE("Couldn't find loadTexture method");
    }

    jstring pathStr = env->NewStringUTF(path);

    unsigned int textureID = env->CallIntMethod(sharedTextureObj, loadTexture,
                                                pathStr, activeTex);

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
    lightShader = Shader(env, thiz,
                       "vertex.glsl", "lightFragment.glsl");

    // our VBO, VAO
    // generation
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenTextures(1, &texture1);
    glGenTextures(1, &texture2);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    texture1 = loadTexture(env, thiz, "container2.png", 0);
    texture2 = loadTexture(env, thiz, "container2_specular.png", 1);

    glBindVertexArray(lightVAO);
    // bind VBO, EBO after binding VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // release
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0); // not necessary, in general, don't do this

    ourShader->use();
    ourShader->setFloat("ratio", 0.5);

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
    ourShader->setInt("material.diffuse", 0);
    ourShader->setInt("material.specular", 1);
    ourShader->setInt("material.emission", 2);

    ourShader->setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

    /*
       Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
       the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
       by defining light types as classes and set their values in there, or by using a more efficient uniform approach
       by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
    */
    // Directional light
    glUniform3f(glGetUniformLocation(ourShader->ID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
    glUniform3f(glGetUniformLocation(ourShader->ID, "dirLight.ambient"), 0.3f, 0.24f, 0.14f);
    glUniform3f(glGetUniformLocation(ourShader->ID, "dirLight.diffuse"), 0.7f, 0.42f, 0.26f);
    glUniform3f(glGetUniformLocation(ourShader->ID, "dirLight.specular"), 0.5f, 0.5f, 0.5f);
    // Point light 1
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[0].ambient"), pointLightColors[0].x * 0.1,  pointLightColors[0].y * 0.1,  pointLightColors[0].z * 0.1);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[0].diffuse"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[0].specular"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[0].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[0].linear"), 0.09);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[0].quadratic"), 0.032);
    // Point light 2
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[1].ambient"), pointLightColors[1].x * 0.1,  pointLightColors[1].y * 0.1,  pointLightColors[1].z * 0.1);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[1].diffuse"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[1].specular"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[1].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[1].linear"), 0.09);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[1].quadratic"), 0.032);
    // Point light 3
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[2].ambient"), pointLightColors[2].x * 0.1,  pointLightColors[2].y * 0.1,  pointLightColors[2].z * 0.1);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[2].diffuse"), pointLightColors[2].x,  pointLightColors[2].y,  pointLightColors[2].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[2].specular") ,pointLightColors[2].x,  pointLightColors[2].y,  pointLightColors[2].z);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[2].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[2].linear"), 0.09);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[2].quadratic"), 0.032);
    // Point light 4
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[3].ambient"), pointLightColors[3].x * 0.1,  pointLightColors[3].y * 0.1,  pointLightColors[3].z * 0.1);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[3].diffuse"), pointLightColors[3].x,  pointLightColors[3].y,  pointLightColors[3].z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "pointLights[3].specular"), pointLightColors[3].x,  pointLightColors[3].y,  pointLightColors[3].z);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[3].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[3].linear"), 0.09);
    glUniform1f(glGetUniformLocation(ourShader->ID, "pointLights[3].quadratic"), 0.032);
    // SpotLight
    glUniform3f(glGetUniformLocation(ourShader->ID, "spotLight.position"), camera.Position.x, camera.Position.y, camera.Position.z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "spotLight.direction"), camera.Front.x, camera.Front.y, camera.Front.z);
    glUniform3f(glGetUniformLocation(ourShader->ID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
    glUniform3f(glGetUniformLocation(ourShader->ID, "spotLight.diffuse"), 0.8f, 0.8f, 0.0f);
    glUniform3f(glGetUniformLocation(ourShader->ID, "spotLight.specular"), 0.8f, 0.8f, 0.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "spotLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(ourShader->ID, "spotLight.linear"), 0.09);
    glUniform1f(glGetUniformLocation(ourShader->ID, "spotLight.quadratic"), 0.032);
    glUniform1f(glGetUniformLocation(ourShader->ID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(ourShader->ID, "spotLight.outerCutOff"), glm::cos(glm::radians(13.0f)));

    ourShader->setVec3("viewPos", camera.Position);
    ourShader->setFloat("material.shininess", 32.0f);

    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ourShader->setMat4("model", model);

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    ourShader->setMat4("view", view);

    // pass projection matrix to shader (note that in this case it could change every frame)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader->setMat4("projection", projection);

    // the process of drawing
    // does not matter if you loop the binding.
    // if it's bound, then it is effectively bound !!!

    glBindVertexArray(VAO);
    for(unsigned  int  i  =  0;  i  <  10;  i++) {
        glm::mat4  model  =  glm::mat4(1.0f);
        model  =  glm::translate(model,  cubePositions[i]);
        float  angle  =  20.0f  *  i;
        model  =  glm::rotate(model,  glm::radians(angle),
                              glm::vec3(1.0f,  0.3f,  0.5f));
        ourShader->setMat4("model",  model);
        glDrawArrays(GL_TRIANGLES,  0,  36);
    }

    lightShader->use();

    lightShader->setMat4("view", view);
    lightShader->setMat4("projection", projection);

    // point lights
    glBindVertexArray(lightVAO);
    for(unsigned int i = 0; i < 4; i++) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.1f));

        lightShader->setMat4("model", model);
        lightShader->setVec3("lightColor", pointLightColors[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_webviewar_GLView_nativeCameraRotation(JNIEnv *env, jobject thiz,
                                                       jfloat xoffset, jfloat yoffset) {
    camera.ProcessMouseMovement(-xoffset, yoffset);
}

