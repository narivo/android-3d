# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fno-rtti -Wall")

include_directories(src/main/cpp/include)

file(GLOB SOURCES "src/main/cpp/*.cpp")

add_library(
        assimp
        SHARED
        IMPORTED)

set_target_properties(
        assimp
        PROPERTIES IMPORTED_LOCATION
        /Users/madiapps/android-native-libs/assimp/${ANDROID_ABI}/libassimp.so)

add_library( # Specifies the name of the library.
        gles3jni

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SOURCES})

# Links your native library against one or more other native libraries.
target_link_libraries( # Specifies the target library.
        gles3jni
        assimp
        GLESv3
        android
        EGL
        log
        m)