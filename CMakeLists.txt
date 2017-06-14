cmake_minimum_required(VERSION 3.8)

project(WebWoWViewerCpp)
include(ExternalProject)

ExternalProject_Add("Zipper"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zipper"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/zipper -DEXTRA_LIBS=stdc++fs
        )
ExternalProject_Add("minizip"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/minizip"
        CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/minizip
        )

set(Zipper_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/zipper/include")


ADD_LIBRARY(wowViewerLib STATIC IMPORTED)


set(SOURCE_FILES src/main.cpp src/persistance/httpFile/httpFile.cpp src/persistance/httpFile/httpFile.h)

add_executable(WebWoWViewerCpp ${SOURCE_FILES})

add_dependencies(WebWoWViewerCpp Zipper)
add_dependencies(WebWoWViewerCpp minizip)

set_property(TARGET WebWoWViewerCpp PROPERTY CXX_STANDARD 17)

target_link_libraries(WebWoWViewerCpp ${OPENGL_LIBRARIES} ${GLFW_STATIC_LIBRARIES} ${CURL_LIBRARIES} zipper z stdc++fs)
