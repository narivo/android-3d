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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -Wall")
if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    set(GL3STUB_SRC gl3stub.c)
    set(OPENGL_LIB GLESv2)
else ()
    set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

include_directories(src/main/cpp/include)

add_library( # Specifies the name of the library.
        gles3jni

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/${GL3STUB_SRC}
        src/main/cpp/stb_image.cpp
        src/main/cpp/gles3jni.cpp
        src/main/cpp/native-lib.cpp
        src/main/cpp/RendererES2.cpp
        src/main/cpp/RendererES3.cpp )

# Links your native library against one or more other native libraries.
target_link_libraries( # Specifies the target library.
        gles3jni
        ${OPENGL_LIB}
        android
        EGL
        log
        m)