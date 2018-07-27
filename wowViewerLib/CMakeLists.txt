cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

option(BUILD_WITHOUT_CULLED_PORTAL_DRAWING "Build without drawing culled portals" ON)

if (BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCULLED_NO_PORTAL_DRAWING")
endif(BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-multichar -g -O3 -D__OPTIMIZE__=1 -ftree-vectorize" )
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-multichar -g -O3 -D__OPTIMIZE__=1 -ftree-vectorize")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

IF(CMAKE_BUILD_TYPE MATCHES Release)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

if (CMAKE_NDK_BUILD MATCHES 1)
#    include_directories(BEFORE SYSTEM "e:/temp/ndk_toolchain/sysroot/usr/include/" "e:/temp/ndk_toolchain/include/c++/4.9.x/")
#    set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH} e:/temp/ndk_toolchain/sysroot/usr/include/ e:/temp/ndk_toolchain/include/c++/4.9.x/")
endif()


 
include(ExternalProject)

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/mathfu)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
    include_directories(${PROJECT_SOURCE_DIR}/3rdparty/glew_win/include)
    if (WIN32)
        MESSAGE(IT IS WINDOWS!!!!)
        set(GLEW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/include)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/x64/glew32.lib )
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/Win32/glew32.lib )
        endif()
    else()
        find_package(GLEW REQUIRED)
        if (GLEW_FOUND)
            set(glew_include ${GLEW_INCLUDE_DIRS})
            include_directories(${GLEW_INCLUDE_DIRS})
            link_libraries(${GLEW_LIBRARIES})
        endif()
    endif(WIN32)
else()
endif(NOT CMAKE_NDK_BUILD MATCHES 1)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} PARENT_SCOPE)
include_directories(${GLEW_INCLUDE_DIRS})

configure_file(${PROJECT_SOURCE_DIR}/src/glsl/adtLodShader.glsl ${CMAKE_BINARY_DIR}/glsl/adtLodShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/adtShader.glsl ${CMAKE_BINARY_DIR}/glsl/adtShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawBBShader.glsl ${CMAKE_BINARY_DIR}/glsl/drawBBShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawDepthShader.glsl ${CMAKE_BINARY_DIR}/glsl/drawDepthShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawFrustumShader.glsl ${CMAKE_BINARY_DIR}/glsl/drawFrustumShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawLinesShader.glsl ${CMAKE_BINARY_DIR}/glsl/drawLinesShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawPoints.glsl ${CMAKE_BINARY_DIR}/glsl/drawPoints.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/drawPortalShader.glsl ${CMAKE_BINARY_DIR}/glsl/drawPortalShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/m2ParticleShader.glsl ${CMAKE_BINARY_DIR}/glsl/m2ParticleShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/m2Shader.glsl ${CMAKE_BINARY_DIR}/glsl/m2Shader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/readDepthBufferShader.glsl ${CMAKE_BINARY_DIR}/glsl/readDepthBufferShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/renderFrameBufferShader.glsl ${CMAKE_BINARY_DIR}/glsl/renderFrameBufferShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/textureCompositionShader.glsl ${CMAKE_BINARY_DIR}/glsl/textureCompositionShader.glsl COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/glsl/wmoShader.glsl ${CMAKE_BINARY_DIR}/glsl/wmoShader.glsl COPYONLY)

#include_directories(${PROJECT_SOURCE_DIR}/lib/glsl)
include_directories(${PROJECT_SOURCE_DIR}/src/opengl/)

set(SOURCE_FILES src/engine/wowScene.cpp
        src/engine/wowScene.h
        src/engine/shader/ShaderRuntimeData.cpp
        src/engine/shader/ShaderRuntimeData.h
        src/engine/opengl/header.h
        src/engine/camera/firstPersonCamera.cpp
        src/engine/camera/firstPersonCamera.h
        src/engine/camera/firstPersonOrthoCamera.cpp
        src/engine/camera/firstPersonOrthoCamera.h
        src/include/config.h
        src/engine/camera/CameraInterface.h
        src/engine/persistance/header/commonFileStructs.h
        src/engine/persistance/header/M2FileHeader.h
        src/engine/geometry/m2Geom.cpp
        src/engine/geometry/m2Geom.h
        src/engine/managers/SceneGraph.cpp src/engine/managers/SceneGraph.h src/engine/persistance/header/wmoFileHeader.h
        src/engine/texture/BlpTexture.cpp src/engine/texture/BlpTexture.h src/engine/persistance/header/blpFileHeader.h
        src/engine/cache/cache.cpp src/engine/cache/cache.h src/engine/geometry/skinGeom.cpp
        src/engine/geometry/skinGeom.h src/engine/persistance/header/skinFileHeader.h
        src/engine/objects/m2/m2Object.cpp src/engine/objects/m2/m2Object.h
        src/engine/algorithms/mathHelper.cpp src/engine/algorithms/mathHelper.h
        src/engine/wowInnerApi.h
        src/engine/objects/m2/m2Helpers/M2MaterialInst.cpp
        src/engine/objects/m2/m2Helpers/M2MaterialInst.h
        src/engine/managers/animationManager.cpp
        src/engine/algorithms/animate.cpp
        src/engine/algorithms/animate.h
        src/engine/geometry/wmoGroupGeom.cpp
        src/engine/geometry/wmoGroupGeom.h
        src/engine/geometry/wmoMainGeom.cpp
        src/engine/geometry/wmoMainGeom.h
        src/engine/persistance/helper/ChunkFileReader.h
        src/engine/objects/adt/adtObject.cpp
        src/engine/objects/adt/adtObject.h
        src/engine/persistance/header/adtFileHeader.h
        src/engine/objects/wmo/wmoGroupObject.cpp
        src/engine/objects/wmo/wmoGroupObject.h
        src/engine/objects/wmo/wmoObject.cpp
        src/engine/objects/wmo/wmoObject.h
        src/engine/objects/scenes/map.cpp
        src/engine/objects/scenes/map.h
        src/engine/persistance/adtFile.h
        src/engine/persistance/adtFile.cpp
        src/engine/algorithms/grahamScan.cpp
        src/engine/algorithms/grahamScan.h
        src/engine/objects/iMapApi.h
        src/engine/objects/iInnerSceneApi.h
        src/engine/objects/scenes/m2Scene.cpp
        src/engine/objects/scenes/m2Scene.h
        src/engine/algorithms/PortalCullingAlgo.cpp
        src/engine/algorithms/PortalCullingAlgo.h
        src/engine/objects/scenes/wmoScene.cpp
        src/engine/objects/scenes/wmoScene.h
        src/engine/objects/objectCache.h src/engine/objects/m2/m2Instancing/m2InstancingObject.cpp
        src/engine/objects/m2/m2Instancing/m2InstancingObject.h
        src/engine/algorithms/hashString.h
        src/engine/persistance/header/wdtFileHeader.h
        src/engine/persistance/wdtFile.cpp
        src/engine/persistance/wdtFile.h
        src/engine/managers/particles/particleEmitter.cpp
        src/engine/managers/particles/particleEmitter.h
        src/engine/managers/particles/generators/CParticleGenerator.cpp src/engine/managers/particles/generators/CParticleGenerator.h src/engine/managers/particles/generators/CSphereGenerator.cpp src/engine/managers/particles/generators/CSphereGenerator.h src/engine/managers/particles/particle.cpp src/engine/managers/particles/particle.h src/engine/managers/particles/generators/CSplineGenerator.cpp src/engine/managers/particles/generators/CSplineGenerator.h src/engine/managers/particles/generators/CBoneGenerator.cpp src/engine/managers/particles/generators/CBoneGenerator.h
        src/engine/persistance/wdlFile.cpp
        src/engine/persistance/wdlFile.cpp
        src/engine/persistance/wdlFile.h
        src/engine/objects/wdl/wdlObject.cpp
        src/engine/objects/wdl/wdlObject.h
        src/engine/managers/particles/CGeneratorAniProp.cpp
        src/engine/managers/particles/CGeneratorAniProp.h
        src/engine/managers/particles/CRndSeed.cpp
        src/engine/managers/particles/CRndSeed.h
        src/engine/managers/particles/generators/CPlaneGenerator.cpp
        src/engine/managers/particles/generators/CPlaneGenerator.h
        src/engine/texture/DxtDecompress.cpp
        src/engine/texture/DxtDecompress.h
        src/engine/androidLogSupport.h src/engine/persistance/db2/base/DB2Base.cpp src/engine/persistance/db2/base/DB2Base.h src/engine/persistance/db2/DB2Light.cpp src/engine/persistance/db2/DB2Light.h src/engine/persistance/db2/DB2LightData.cpp src/engine/persistance/db2/DB2LightData.h src/engine/persistance/db2/DB2LightParams.cpp src/engine/persistance/db2/DB2LightParams.h src/engine/persistance/db2/DB2WmoAreaTable.cpp src/engine/persistance/db2/DB2WmoAreaTable.h src/engine/persistance/texFile.cpp src/engine/persistance/texFile.h src/engine/persistance/header/texFileHeader.h src/gapi/GBlpTexture.cpp src/gapi/GBlpTexture.h src/gapi/GVertexBuffer.cpp src/gapi/GVertexBuffer.h src/gapi/GIndexBuffer.cpp src/gapi/GIndexBuffer.h src/gapi/GRenderTarget.cpp src/gapi/GRenderTarget.h src/gapi/GDevice.cpp src/gapi/GDevice.h src/gapi/meshes/GMesh.cpp src/gapi/meshes/GMesh.h src/gapi/GVertexBufferBindings.cpp src/gapi/GVertexBufferBindings.h src/engine/wowCommonClasses.h src/engine/shader/ShaderDefinitions.cpp src/gapi/GUniformBuffer.cpp src/gapi/GUniformBuffer.h src/gapi/GShaderPermutation.cpp src/gapi/GShaderPermutation.h src/gapi/shaders/GM2ShaderPermutation.cpp src/gapi/shaders/GM2ShaderPermutation.h src/gapi/UniformBufferStructures.h src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.cpp src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.h src/gapi/meshes/GM2Mesh.cpp src/gapi/meshes/GM2Mesh.h src/gapi/GTexture.cpp src/gapi/GTexture.h src/gapi/shaders/GM2ParticleShaderPermutation.cpp src/gapi/shaders/GM2ParticleShaderPermutation.h src/gapi/meshes/GParticleMesh.cpp src/gapi/meshes/GParticleMesh.h src/gapi/shaders/GWMOShaderPermutation.cpp src/gapi/shaders/GWMOShaderPermutation.h src/gapi/shaders/GAdtShaderPermutation.cpp src/gapi/shaders/GAdtShaderPermutation.h)

#add_dependencies(WoWViewerLib mathfu)
add_library(WoWViewerLib ${SOURCE_FILES})
if (NOT CMAKE_NDK_BUILD MATCHES 1)
    set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)
endif()
target_link_libraries(WoWViewerLib ${GLEW_LIBRARIES})

if (NOT CMAKE_NDK_BUILD MATCHES 1)
    target_compile_features(WoWViewerLib PRIVATE cxx_std_17)
    mathfu_configure_flags(WoWViewerLib ON)
else()
    mathfu_configure_flags(WoWViewerLib ON)
#    target_include_directories(WoWViewerLib PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include" "${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include")
endif()

target_include_directories(WoWViewerLib
        PRIVATE ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/src/include/wowScene.h
              ${PROJECT_SOURCE_DIR}/src/include/config.h
              ${PROJECT_SOURCE_DIR}/src/include/controllable.h
        DESTINATION include)
INSTALL(FILES ${glew_lib}
        DESTINATION lib)

INSTALL(TARGETS WoWViewerLib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
