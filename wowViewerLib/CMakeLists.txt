cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

include(CheckCXXCompilerFlag)

option(BUILD_WITHOUT_CULLED_PORTAL_DRAWING "Build without drawing culled portals" ON)

if (BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCULLED_NO_PORTAL_DRAWING")
endif(BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDODEBUGTHREADS" )
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDODEBUGTHREADS")
 
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-multichar -g -O3 -msse2 -ftree-vectorize" )
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-multichar -g -O3 -msse2 -ftree-vectorize")

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

message(${CMAKE_SYSTEM_NAME})
if (NOT CMAKE_NDK_BUILD MATCHES 1 AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten"))
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
endif(NOT CMAKE_NDK_BUILD MATCHES 1 AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten"))

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} PARENT_SCOPE)
include_directories(${GLEW_INCLUDE_DIRS})
endif()

if (CMAKE_NDK_BUILD MATCHES 1)
    set(GLSL_TARGET_FOLDER ${ANDROID_ASSETSFOLDER})
else()
    set(GLSL_TARGET_FOLDER ${CMAKE_BINARY_DIR})
endif()

message(INFO "GLSL_TARGET_FOLDER = ${GLSL_TARGET_FOLDER}")

macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    COPYONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)

configure_files(${PROJECT_SOURCE_DIR}/src/glsl/glsl3.3/ ${GLSL_TARGET_FOLDER}/glsl/glsl3.3/)
configure_files(${PROJECT_SOURCE_DIR}/src/glsl/common/ ${GLSL_TARGET_FOLDER}/glsl/common/)

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
#        src/engine/objects/scenes/creatureScene.cpp
#        src/engine/objects/scenes/creatureScene.h
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
        src/engine/androidLogSupport.h
        src/engine/persistance/db2/base/DB2Base.cpp
        src/engine/persistance/db2/base/DB2Base.h
        src/engine/persistance/db2/DB2Light.cpp
        src/engine/persistance/db2/DB2Light.h
        src/engine/persistance/db2/DB2LightData.cpp
        src/engine/persistance/db2/DB2LightData.h
        src/engine/persistance/db2/DB2LightParams.cpp
        src/engine/persistance/db2/DB2LightParams.h
        src/engine/persistance/db2/DB2WmoAreaTable.cpp
        src/engine/persistance/db2/DB2WmoAreaTable.h
        src/engine/persistance/texFile.cpp src/engine/persistance/texFile.h
        src/engine/persistance/header/texFileHeader.h
        src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.cpp
        src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.h
        src/engine/wowCommonClasses.h
        src/engine/shader/ShaderDefinitions.cpp
        src/engine/objects/GlobalThreads.cpp
        src/engine/objects/GlobalThreads.h

        src/gapi/interface/IDevice.h
        src/gapi/interface/IDevice.cpp
        src/gapi/interface/textures/IBlpTexture.h
        src/gapi/interface/textures/ITexture.h
        src/gapi/interface/buffers/IIndexBuffer.h
        src/gapi/interface/buffers/IUniformBuffer.h
        src/gapi/interface/buffers/IVertexBuffer.h
        src/gapi/interface/IOcclusionQuery.h
        src/gapi/interface/IShaderPermutation.h
        src/gapi/interface/IVertexBufferBindings.h
        src/gapi/interface/meshes/IM2Mesh.h
        src/gapi/interface/meshes/IMesh.h
        src/gapi/IDeviceFactory.cpp src/gapi/IDeviceFactory.h
        src/gapi/interface/meshes/IParticleMesh.h src/gapi/interface/syncronization/IGPUFence.h src/engine/objects/wowFrameData.h src/engine/engineClassList.h src/engine/camera/planarCamera.cpp src/engine/camera/planarCamera.h)

set(OPENGL33_IMPLEMENTATION    src/gapi/ogl3.3/textures/GBlpTextureGL33.cpp
        src/gapi/ogl3.3/textures/GBlpTextureGL33.h
        src/gapi/ogl3.3/buffers/GVertexBufferGL33.cpp
        src/gapi/ogl3.3/buffers/GVertexBufferGL33.h
        src/gapi/ogl3.3/buffers/GIndexBufferGL33.cpp
        src/gapi/ogl3.3/buffers/GIndexBufferGL33.h
        src/gapi/ogl3.3/buffers/GUniformBufferGL33.cpp
        src/gapi/ogl3.3/buffers/GUniformBufferGL33.h
        src/gapi/ogl3.3/GRenderTargetGL33.cpp
        src/gapi/ogl3.3/GRenderTargetGL33.h
        src/gapi/ogl3.3/GDeviceGL33.cpp
        src/gapi/ogl3.3/GDeviceGL33.h
        src/gapi/ogl3.3/meshes/GMeshGL33.cpp
        src/gapi/ogl3.3/meshes/GMeshGL33.h
        src/gapi/ogl3.3/GVertexBufferBindingsGL33.cpp
        src/gapi/ogl3.3/GVertexBufferBindingsGL33.h
        src/gapi/ogl3.3/GShaderPermutationGL33.cpp
        src/gapi/ogl3.3/GShaderPermutationGL33.h
        src/gapi/ogl3.3/shaders/GM2ShaderPermutationGL33.cpp
        src/gapi/ogl3.3/shaders/GM2ShaderPermutationGL33.h
        src/gapi/UniformBufferStructures.h
        src/gapi/ogl3.3/meshes/GM2MeshGL33.cpp
        src/gapi/ogl3.3/meshes/GM2MeshGL33.h
        src/gapi/ogl3.3/textures/GTextureGL33.cpp
        src/gapi/ogl3.3/textures/GTextureGL33.h
        src/gapi/ogl3.3/shaders/GM2ParticleShaderPermutationGL33.cpp
        src/gapi/ogl3.3/shaders/GM2ParticleShaderPermutationGL33.h
        src/gapi/ogl3.3/meshes/GParticleMeshGL33.cpp
        src/gapi/ogl3.3/meshes/GParticleMeshGL33.h
        src/gapi/ogl3.3/shaders/GWMOShaderPermutationGL33.cpp
        src/gapi/ogl3.3/shaders/GWMOShaderPermutationGL33.h
        src/gapi/ogl3.3/shaders/GAdtShaderPermutationGL33.cpp
        src/gapi/ogl3.3/shaders/GAdtShaderPermutationGL33.h
        src/gapi/ogl3.3/GOcclusionQueryGL33.cpp
        src/gapi/ogl3.3/GOcclusionQueryGL33.h)

set(OPENGL4x_IMPLEMENTATION    src/gapi/ogl4.x/textures/GBlpTextureGL4x.cpp
        src/gapi/ogl4.x/textures/GBlpTextureGL4x.h
        src/gapi/ogl4.x/buffers/GVertexBufferGL4x.cpp
        src/gapi/ogl4.x/buffers/GVertexBufferGL4x.h
        src/gapi/ogl4.x/buffers/GIndexBufferGL4x.cpp
        src/gapi/ogl4.x/buffers/GIndexBufferGL4x.h
        src/gapi/ogl4.x/buffers/GUniformBufferGL4x.cpp
        src/gapi/ogl4.x/buffers/GUniformBufferGL4x.h
        src/gapi/ogl4.x/GRenderTargetGL4x.cpp
        src/gapi/ogl4.x/GRenderTargetGL4x.h
        src/gapi/ogl4.x/GDeviceGL4x.cpp
        src/gapi/ogl4.x/GDeviceGL4x.h
        src/gapi/ogl4.x/meshes/GMeshGL4x.cpp
        src/gapi/ogl4.x/meshes/GMeshGL4x.h
        src/gapi/ogl4.x/GVertexBufferBindingsGL4x.cpp
        src/gapi/ogl4.x/GVertexBufferBindingsGL4x.h
        src/gapi/ogl4.x/GShaderPermutationGL4x.cpp
        src/gapi/ogl4.x/GShaderPermutationGL4x.h
        src/gapi/ogl4.x/shaders/GM2ShaderPermutationGL4x.cpp
        src/gapi/ogl4.x/shaders/GM2ShaderPermutationGL4x.h
        src/gapi/UniformBufferStructures.h
        src/gapi/ogl4.x/meshes/GM2MeshGL4x.cpp
        src/gapi/ogl4.x/meshes/GM2MeshGL4x.h
        src/gapi/ogl4.x/textures/GTextureGL4x.cpp
        src/gapi/ogl4.x/textures/GTextureGL4x.h
        src/gapi/ogl4.x/shaders/GM2ParticleShaderPermutationGL4x.cpp
        src/gapi/ogl4.x/shaders/GM2ParticleShaderPermutationGL4x.h
        src/gapi/ogl4.x/meshes/GParticleMeshGL4x.cpp
        src/gapi/ogl4.x/meshes/GParticleMeshGL4x.h
        src/gapi/ogl4.x/shaders/GWMOShaderPermutationGL4x.cpp
        src/gapi/ogl4.x/shaders/GWMOShaderPermutationGL4x.h
        src/gapi/ogl4.x/shaders/GAdtShaderPermutationGL4x.cpp
        src/gapi/ogl4.x/shaders/GAdtShaderPermutationGL4x.h
        src/gapi/ogl4.x/GOcclusionQueryGL4x.cpp
        src/gapi/ogl4.x/GOcclusionQueryGL4x.h src/gapi/ogl4.x/syncronization/GPUFenceGL44.cpp src/gapi/ogl4.x/syncronization/GPUFenceGL44.h)


#add_dependencies(WoWViewerLib mathfu)
add_library(WoWViewerLib STATIC ${SOURCE_FILES} ${OPENGL33_IMPLEMENTATION} ${OPENGL4x_IMPLEMENTATION} src/engine/persistance/animFile.cpp src/engine/persistance/animFile.h src/engine/persistance/m2File.cpp src/engine/persistance/m2File.h)
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
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -stdEMSCRIPTEN=c++1z" )
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z")
    else()
        message(ERROR "No supported flags")
    endif()
endif()

if (NOT EMSCRIPTEN)
    target_compile_features(WoWViewerLib PRIVATE cxx_std_17)
    set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(WoWViewerLib ${GLEW_LIBRARIES})
#target_link_libraries(WoWViewerLib gomp) //never use it. It makes uniform buffers screwed up for some reason

if (EMSCRIPTEN)
    mathfu_configure_flags(WoWViewerLib FALSE)
else()
    mathfu_configure_flags(WoWViewerLib FALSE)
endif()


target_include_directories(WoWViewerLib
        PRIVATE ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include)
