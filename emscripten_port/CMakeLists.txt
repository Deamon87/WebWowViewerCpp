cmake_minimum_required(VERSION 3.17)

project(WoWWebViewerJs)

include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXECUTABLE_SUFFIX .js)

set(CMAKE_BUILD_PARALLEL_LEVEL 1)

add_link_options("SHELL: -s ALLOW_MEMORY_GROWTH=1")
add_link_options("SHELL: -s DISABLE_EXCEPTION_CATCHING=0")
add_link_options("SHELL: -s TOTAL_MEMORY=40MB")
add_link_options("SHELL: -s USE_ZLIB=1 ")

message( EMSCRIPTEN_ROOT_PATH = ${EMSCRIPTEN_ROOT_PATH})

option(COMPILE_WITH_SIMD "Compile with experimental WASM SIMD support" OFF)
message(COMPILE_WITH_SIMD = ${COMPILE_WITH_SIMD})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --emit-symbol-map" )
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --emit-symbol-map" )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --emit-symbol-map")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --emit-symbol-map")

check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    # Have -std=c++17, use it
    message(INFO "HAVE_FLAG_STD_CXX17 is supported")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17" )
else()
    check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)
    if(HAVE_FLAG_STD_CXX1Z)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++1z" )
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++1z" )
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z")
    else()
        message(ERROR "No supported flags")
    endif()
endif()

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-exceptions" )
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-exceptions" )
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-exceptions")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions")

set(EMSCRIPTEN 1)

set(OUTPUTDIR "")
if (COMPILE_WITH_SIMD)
    set(OUTPUTDIR "simd_version/")
    set(EMSCRIPTEN_SIMD 1)
endif()

set(BUILD_SHARED_VERSION 0)
set(BUILD_STATIC_VERSION 1)
add_subdirectory(${PROJECT_SOURCE_DIR}/../3rdparty/zlib ${CMAKE_CURRENT_BINARY_DIR}/zlib EXCLUDE_FROM_ALL)

set (LIBZ_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../3rdparty/zlib")
set (ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../3rdparty/zlib")
set (ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/zlib/libz.a")
include_directories(${CMAKE_CURRENT_BINARY_DIR}/zlib)

add_subdirectory(3rdparty/zipper EXCLUDE_FROM_ALL)

#set(PNG_BUILD_ZLIB OFF)
#add_subdirectory(../3rdparty/libpng ${CMAKE_CURRENT_BINARY_DIR}/libpng)
#include_directories(../3rdparty/libpng)
add_subdirectory(../wowViewerLib ${CMAKE_CURRENT_BINARY_DIR}/wowViewerLib)


set(SOURCE_FILES src/main.cpp
        src/RequestProcessor.cpp
        src/HttpRequestProcessor.cpp
        ../src/exporters/gltfExporter/GLTFExporter.cpp
        ../src/exporters/gltfExporter/GLTFExporter.h
        ../src/screenshots/lodepng/lodepng.cpp
        ../src/screenshots/lodepng/lodepng.h
        ../src/screenshots/screenshotMaker.cpp
        )



add_executable(project ${SOURCE_FILES})

add_dependencies(project WoWViewerLib)

target_link_libraries(project WoWViewerLib)
target_link_libraries(project zlibstatic)
target_link_libraries(project staticZipper)

target_link_options(project PUBLIC "SHELL: --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/prejs.js --js-library ${CMAKE_CURRENT_SOURCE_DIR}/jsLib.js")
target_link_options(project PUBLIC "SHELL: -s USE_GLFW=3 --preload-file glsl -s ALLOW_MEMORY_GROWTH=1")
target_link_options(project PUBLIC "SHELL: -s TOTAL_MEMORY=40MB")
target_link_options(project PUBLIC "SHELL: -s FETCH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1")

target_link_options(project PUBLIC "SHELL: -fno-rtti ")
target_link_options(project PUBLIC "SHELL: -s EXTRA_EXPORTED_RUNTIME_METHODS=[\"FS\"]")

target_link_options(project PUBLIC "SHELL: -s ENVIRONMENT=\"web\" " )
target_link_options(project PUBLIC "SHELL: -s USE_PTHREADS=0")
target_link_options(project PUBLIC "SHELL: --bind ")
#target_link_options(project PUBLIC "SHELL: -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=2")
#target_link_options(project PUBLIC "SHELL: -s ENVIRONMENT=\"web,worker\" " )


set_target_properties(project
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${OUTPUTDIR}"
        )