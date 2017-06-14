cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

ExternalProject_Add("mathfu"
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/mathfu
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/mathfu
        )

include(cmakeScripts/generateGlslMapping/CMakeLists.txt)


set(SOURCE_FILES wowViewerLib/src/engine/wowScene.cpp wowViewerLib/src/engine/wowScene.h wowViewerLib/src/engine/shadersStructures.h wowViewerLib/src/engine/shaderHelperFunctions.h wowViewerLib/src/engine/shaders.h wowViewerLib/src/engine/shader/ShaderRuntimeData.cpp wowViewerLib/src/engine/shader/ShaderRuntimeData.h wowViewerLib/src/engine/opengl/header.h wowViewerLib/src/engine/camera/firstPersonCamera.cpp wowViewerLib/src/engine/camera/firstPersonCamera.h)

add_library(WebWoWViewerCpp ${SOURCE_FILES})
