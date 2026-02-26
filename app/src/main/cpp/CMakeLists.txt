# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10)

project("native-rockx")

set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")

set(ROOT_LIBROCKIVA "${PROJECT_SOURCE_DIR}/../libs/rockx-rk3576")
include_directories(${ROOT_LIBROCKIVA}/include)
add_library(rknnrt SHARED IMPORTED )
set_target_properties(rknnrt PROPERTIES IMPORTED_LOCATION "${ROOT_LIBROCKIVA}/${ANDROID_ABI}/librknnrt.so")

add_library(rockx SHARED IMPORTED)
set_target_properties(rockx PROPERTIES IMPORTED_LOCATION "${ROOT_LIBROCKIVA}/${ANDROID_ABI}/librockx.so")

add_library(rockx_modules SHARED IMPORTED)
set_target_properties(rockx_modules PROPERTIES IMPORTED_LOCATION "${ROOT_LIBROCKIVA}/${ANDROID_ABI}/librockx_modules.so")

set(ROOT_LIBRGA "${PROJECT_SOURCE_DIR}/../libs/librga")
include_directories(${ROOT_LIBRGA}/include)
add_library(rga SHARED IMPORTED)
set_target_properties(rga PROPERTIES IMPORTED_LOCATION "${ROOT_LIBRGA}/${ANDROID_ABI}/librga.so")

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
        native-rockx
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        native_image.cpp native_detect.cpp yuv420_converter.cpp image_detect_rockx.cpp image_reader.cpp cJson.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-rockx
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} rknnrt rockx rockx_modules)

# 声明可执行文件及对应的源文件
add_executable(v4l2_test
    main_test.cpp
    image_reader.cpp
    image_converter.cpp
)

# 如果有第三方库需要链接，可以在这里添加
target_link_libraries(v4l2_test ${log-lib} rga)

# 如果需要指定额外的编译选项，可以在这里添加
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")