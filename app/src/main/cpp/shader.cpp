#include "shader.h"

Shader::Shader(JNIEnv* env, jobject thiz,
        std::string vertexPath, const char* fragmentPath) {
    // creating program in the JVM env

    jclass glUtilsKlass = env->FindClass("com/example/webviewar/GLUtils");
    jmethodID createProgram = env->GetStaticMethodID(glUtilsKlass, "createProgram",
                                                     "(Ljava/lang/String;Ljava/lang/String;)I");

    /*jclass rendererKlass = env->GetObjectClass(thiz);
    jfieldID sharedShaderID = env->GetFieldID(rendererKlass, "sharedShader",
                                            "com/example/webviewar/Shader");
    jobject sharedShaderObj = env->GetObjectField(thiz, sharedShaderID);
    jclass shaderKlass = env->GetObjectClass(sharedShaderObj);
    jmethodID createProgram = env->GetMethodID(shaderKlass, "createProgram",
                                               "(Ljava/lang/String;Ljava/lang/String;)I");
    if (createProgram == NULL) {
        LOGE("Couldn't find createProgram method");
    }*/

    jstring vertexStr = env->NewStringUTF(vertexPath.c_str());
    jstring fragmentStr = env->NewStringUTF(fragmentPath);

    ID = env->CallStaticIntMethod(glUtilsKlass, createProgram,
                            vertexStr, fragmentStr);

    env->DeleteLocalRef(vertexStr);
    env->DeleteLocalRef(fragmentStr);

    // 1. retrieve the vertex/fragment source code from filePath
    /*std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vertShFile;
    std::ifstream fragShFile;
    // setting exceptions flags :
    vertShFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fragShFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try {

        vertShFile.open(vertexPath);
        fragShFile.open(fragmentPath);
        std::stringstream vertShStream, fragShStream;

        vertShStream << vertShFile.rdbuf();
        fragShStream << fragShFile.rdbuf();

        vertShFile.close();
        fragShFile.close();

        vertexCode   = vertShStream.str();
        fragmentCode = fragShStream.str();
    } catch(std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vertShCode = vertexCode.c_str();
    const char* fragShCode = fragmentCode.c_str();
    unsigned int vertex, fragment;
    vertex = createCompileShader(GL_VERTEX_SHADER, vertShCode);
    fragment = createCompileShader(GL_FRAGMENT_SHADER, fragShCode);
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "program");
    glDeleteShader(vertex);
    glDeleteShader(fragment);*/
}