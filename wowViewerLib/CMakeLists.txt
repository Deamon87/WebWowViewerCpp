cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

option(BUILD_WITHOUT_CULLED_PORTAL_DRAWING "Build without drawing culled portals" ON)

#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -O3 -D__OPTIMIZE__=1 -ftree-vectorize -DMATHFU_COMPILE_WITH_SIMD")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -D__OPTIMIZE__=1 -ftree-vectorize -DMATHFU_COMPILE_WITH_SIMD ")
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

if (BUILD_WITHOUT_CULLED_PORTAL_DRAWING)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCULLED_NO_PORTAL_DRAWING")
endif(BUILD_WITHOUT_CULLED_PORTAL_DRAWING)

include(ExternalProject)
#set(mathfu_build_benchmarks OFF CACHE BOOL "")
#set(mathfu_build_tests OFF CACHE BOOL "")

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")





add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/mathfu mathfu)

if (WIN32)
    MESSAGE(IT IS WINDOWS!!!!)
    include_directories(${CMAKE_SOURCE_DIR}/3rdparty/glew_win/include)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(glew_lib ${CMAKE_SOURCE_DIR}/3rdparty/glew_win/lib/Release/x64/glew32s.lib)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(glew_lib ${CMAKE_SOURCE_DIR}/3rdparty/glew_win/lib/Release/Win32/glew32s.lib)
    endif()

else()
    find_package(GLEW REQUIRED)
    if (GLEW_FOUND)
        include_directories(${GLEW_INCLUDE_DIRS})
        link_libraries(${GLEW_LIBRARIES})
    endif()
    if(NOT GLEW_FOUND)
        message(ERROR " GLEW not found!")
    endif(NOT GLEW_FOUND)
endif(WIN32)

include(cmakeScripts/generateGlslMapping/CMakeLists.txt)
#include_directories(${CMAKE_SOURCE_DIR}/lib/glsl)
include_directories(${CMAKE_SOURCE_DIR}/src/opengl/)

set(SOURCE_FILES src/engine/wowScene.cpp
        src/engine/wowScene.h
        src/engine/shadersStructures.h
        src/engine/shaderHelperFunctions.h
        src/engine/shaders.h
        src/engine/shader/ShaderRuntimeData.cpp
        src/engine/shader/ShaderRuntimeData.h
        src/engine/opengl/header.h
        src/engine/camera/firstPersonCamera.cpp
        src/engine/camera/firstPersonCamera.h src/include/config.h
        src/engine/camera/CameraInterface.h
        src/engine/persistance/header/commonFileStructs.h
        src/engine/persistance/header/M2FileHeader.h
        src/engine/geometry/m2Geom.cpp
        src/engine/geometry/m2Geom.h
        src/engine/managers/SceneGraph.cpp src/engine/managers/SceneGraph.h src/engine/persistance/header/wmoFileHeader.h
        src/engine/texture/BlpTexture.cpp src/engine/texture/BlpTexture.h src/engine/persistance/header/blpFileHeader.h
        src/engine/cache/cache.cpp src/engine/cache/cache.h src/engine/geometry/skinGeom.cpp
        src/engine/geometry/skinGeom.h src/engine/persistance/header/skinFileHeader.h
        src/engine/objects/m2Object.cpp src/engine/objects/m2Object.h
        src/engine/algorithms/mathHelper.cpp src/engine/algorithms/mathHelper.h
        src/engine/wowInnerApi.h
        src/engine/objects/m2Helpers/M2MaterialInst.cpp
        src/engine/objects/m2Helpers/M2MaterialInst.h
        src/engine/managers/animationManager.cpp
        src/engine/algorithms/animate.cpp
        src/engine/algorithms/animate.h
        src/engine/geometry/wmoGroupGeom.cpp
        src/engine/geometry/wmoGroupGeom.h
        src/engine/geometry/wmoMainGeom.cpp
        src/engine/geometry/wmoMainGeom.h
        src/engine/persistance/helper/ChunkFileReader.h
        src/engine/objects/adtObject.cpp
        src/engine/objects/adtObject.h
        src/engine/persistance/header/adtFileHeader.h
        src/engine/objects/wmoGroupObject.cpp
        src/engine/objects/wmoGroupObject.h
        src/engine/objects/wmoObject.cpp
        src/engine/objects/wmoObject.h
        src/engine/objects/map.cpp
        src/engine/objects/map.h
        src/engine/persistance/adtFile.h
        src/engine/persistance/adtFile.cpp
        src/engine/algorithms/grahamScan.cpp
        src/engine/algorithms/grahamScan.h
        src/engine/objects/iMapApi.h
        src/engine/objects/iInnerSceneApi.h
        src/engine/objects/m2Scene.cpp
        src/engine/objects/m2Scene.h
        src/engine/algorithms/PortalCullingAlgo.cpp
        src/engine/algorithms/PortalCullingAlgo.h)

#add_dependencies(WoWViewerLib mathfu)
add_library(WoWViewerLib ${SOURCE_FILES})

set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)
target_link_libraries(WoWViewerLib ${glew_lib})
#set(mathfu_enable_simd ON)
mathfu_configure_flags(WoWViewerLib ON)
mathfu_configure_flags(WoWViewerLib)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/include/wowScene.h
              ${CMAKE_SOURCE_DIR}/src/include/config.h
              ${CMAKE_SOURCE_DIR}/src/include/controllable.h
        DESTINATION include)
INSTALL(FILES ${glew_lib}
        DESTINATION lib)

INSTALL(TARGETS WoWViewerLib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
