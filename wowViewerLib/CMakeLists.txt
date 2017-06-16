cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

include(ExternalProject)
#set(mathfu_build_benchmarks OFF CACHE BOOL "")
#set(mathfu_build_tests OFF CACHE BOOL "")
#ExternalProject_Add("mathfu"
#        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/mathfu
#        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/mathfu -Dmathfu_build_tests="OFF CACHE BOOL" -Dmathfu_build_benchmarks="OFF CACHE BOOL"
#        )

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/mathfu mathfu)


include(cmakeScripts/generateGlslMapping/CMakeLists.txt)

set(SOURCE_FILES src/engine/wowScene.cpp src/engine/wowScene.h
        src/engine/shadersStructures.h
        src/engine/shaderHelperFunctions.h
        src/engine/shaders.h
        src/engine/shader/ShaderRuntimeData.cpp
        src/engine/shader/ShaderRuntimeData.h
        src/engine/opengl/header.h
        src/engine/camera/firstPersonCamera.cpp
        src/engine/camera/firstPersonCamera.h src/include/config.h src/engine/camera/CameraInterface.h src/engine/objects/controllable.h)

#add_dependencies(WoWViewerLib mathfu)
add_library(WoWViewerLib ${SOURCE_FILES})

set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)

mathfu_configure_flags(WoWViewerLib)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/include/wowScene.h
              ${CMAKE_SOURCE_DIR}/src/include/config.h
        DESTINATION include)
INSTALL(TARGETS WoWViewerLib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
