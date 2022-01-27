#ifndef SHADER_H
#define SHADER_H

#include "jni.h"

#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "logger.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace aevv_graphics {

    class Shader {
    public:
        // the program ID
        unsigned int ID{0};

        // constructor reads and builds the shader
        Shader(std::string vertexPath, const char *fragmentPath) {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vertShFile;
            std::ifstream fragShFile;
            // setting exceptions flags :
            vertShFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fragShFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {

                vertShFile.open(vertexPath);
                fragShFile.open(fragmentPath);
                std::stringstream vertShStream, fragShStream;

                vertShStream << vertShFile.rdbuf();
                fragShStream << fragShFile.rdbuf();

                vertShFile.close();
                fragShFile.close();

                vertexCode = vertShStream.str();
                fragmentCode = fragShStream.str();
            } catch (std::ifstream::failure e) {
                LOGE("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
            }
            const char *vertShCode = vertexCode.c_str();
            const char *fragShCode = fragmentCode.c_str();
            unsigned int vertex, fragment;
            vertex = createCompileShader(GL_VERTEX_SHADER, vertShCode);
            fragment = createCompileShader(GL_FRAGMENT_SHADER, fragShCode);
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);
            checkCompileErrors(ID, "program");
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        ~Shader() {
            glDeleteProgram(ID);
        }

        // use/activate the shader
        void use() {
            glUseProgram(ID);
        }

        // utility uniform functions
        void setBool(const std::string &name, bool value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
        }

        void setInt(const std::string &name, int value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }

        void setFloat(const std::string &name, float value) const {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }

        void setMat4(const std::string &name, glm::mat4 transform) const {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                               glm::value_ptr(transform));
        }

        void setVec3(const std::string &name, glm::vec3 value) const {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
        }

    private:
        unsigned int createCompileShader(unsigned int type, const char *source) {
            if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
                throw -1;
            }
            unsigned int shader = glCreateShader(type);

            glShaderSource(shader, /* how many strings ? */1, &source, nullptr);

            // compiling
            glCompileShader(shader);
            checkCompileErrors(shader, "shader");

            return shader;
        }

        void checkCompileErrors(unsigned int shader, const char *type) {
            int success;
            char infoLog[1024];
            int length;
            if (strcmp(type, "program") != 0) {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, &length, infoLog);
                    char errorLog[]{"ERROR::SHADER::COMPILATION_FAILED\n"};
                    LOGE(errorLog, infoLog);
                    throw -1;
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, &length, infoLog);
                    char errorLog[]{"ERROR::PROGRAM::LINKING_FAILED\n"};
                    LOGE(errorLog, infoLog);
                    throw -1;
                }
            }
        }
    };
}
#endif