cmake_minimum_required(VERSION 3.6)
project(WoWViewerLib)

include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(LINK_GLEW 1)
set(LINK_OGL2 0)
set(LINK_OGL3 0)
set(LINK_OGL4 0)
set(LINK_EGL 1 PARENT_SCOPE)
set(LINK_EGL 1)
option(LINK_VULKAN "Enable Vulkan" ON)
set(LINK_OPENMP 0)
set(ENABLE_SIMD 1)
set(LINK_TRACY 0)
set(COMPILE_SHADER_WITH_DEBUG 0)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILE_SHADER_WITH_DEBUG 1)
endif ()

option(EMSCRIPTEN_SIMD "Enable SIMD for Emscripten " OFF)


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINK_GLEW 0)
endif()
#MacOS system
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LINK_OGL2 0)
    set(LINK_GLEW 0)
endif()
if (MSVC)
    add_definitions(-DNOMINMAX)
endif()

if (CMAKE_NDK_BUILD)
    set(LINK_GLEW 0)
ENDIF()

if (NOT EGL_PATH)
    set(LINK_EGL 0 PARENT_SCOPE)
    set(LINK_EGL 0)
endif()


if (EMSCRIPTEN)
    message(This is EMSCRIPTEN)
    set(LINK_GLEW 0)
    set(LINK_VULKAN OFF)
    set(LINK_OPENMP 0)
    set(LINK_OGL4 0)
    if (NOT EMSCRIPTEN_SIMD)
        set(ENABLE_SIMD 0)
    endif()
ENDIF()

if (LINK_OGL2)
    add_definitions(-DLINK_OGL2)
endif()
if (LINK_EGL)
    add_definitions(-DLINK_EGL)
    add_definitions(-DWITH_GLESv2)
endif()
if (LINK_OGL3)
    add_definitions(-DLINK_OGL3)
endif()
if (LINK_OGL4)
    add_definitions(-DLINK_OGL4)
endif()

set(My_Vectorize_Compile_Options "")


message("ENABLE_SIMD = ${ENABLE_SIMD}")

if (NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
endif ()

SET(TBB_STRICTTBB_BUILD_SHARED 0)
SET(TBB_STRICT OFF CACHE BOOL "")
SET(TBB_TEST 0 CACHE BOOL "")
if (EMSCRIPTEN)
    add_definitions("-D__TBB_RESUMABLE_TASKS_USE_THREADS")
endif ()
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/oneTbb EXCLUDE_FROM_ALL)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDODEBUGTHREADS" )
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDODEBUGTHREADS")
 
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
if (EMSCRIPTEN)
    message("EMSCRIPTEN ENABLE_SIMD = ${ENABLE_SIMD}")
    if (ENABLE_SIMD)
        set(My_Vectorize_Compile_Options "${My_Vectorize_Compile_Options};-msimd128;-msse2;-msse;-ftree-vectorize")
    endif()
else()
    set(My_Vectorize_Compile_Options "${My_Vectorize_Compile_Options};-msse3;-mavx2;-ftree-vectorize")

    if (NOT MSVC)
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-multichar")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-multichar")

        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -O3" )
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
    endif()
endif()


IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

IF(CMAKE_BUILD_TYPE MATCHES Release)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)


include(ExternalProject)

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/mathfu EXCLUDE_FROM_ALL)

# ---------------------
# Tracy integration
# ---------------------

if (LINK_TRACY)
    option( TRACY_ENABLE "" ON)
    option( TRACY_ON_DEMAND "" ON)
    add_subdirectory (${PROJECT_SOURCE_DIR}/3rdparty/tracy)

    add_definitions(-DTRACY_ENABLE)
    set(TRACY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rdparty/tracy/public/tracy")
endif()


message(CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME})
string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWERCASE)
if (LINK_GLEW)
    if (WIN32)
        MESSAGE(IT IS WINDOWS!!!!)

        set(GLEW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/include)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/x64/glew32.lib )
            set(GLEW_DLL "${PROJECT_SOURCE_DIR}/3rdparty/glew_win/bin/Release/x64/glew32.dll")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/Win32/glew32.lib )
            set(GLEW_DLL "${PROJECT_SOURCE_DIR}/3rdparty/glew_win/bin/Release/Win32/glew32.dll")
        endif()
    elseif(NOT CMAKE_SYSTEM_NAME_LOWERCASE MATCHES "ANDROID")

        find_package(GLEW REQUIRED)
        if (GLEW_FOUND)
            set(glew_include ${GLEW_INCLUDE_DIRS})
            include_directories(${GLEW_INCLUDE_DIRS})
            link_libraries(${GLEW_LIBRARIES})
        endif()
    endif(WIN32)

    set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} PARENT_SCOPE)
    include_directories(${GLEW_INCLUDE_DIRS})
else()
endif(LINK_GLEW)




set(SOURCE_FILES
        3rdparty/OffsetAllocator/offsetAllocator.cpp
        3rdparty/OffsetAllocator/offsetAllocator.hpp
        src/engine/shader/ShaderRuntimeData.cpp
        src/engine/shader/ShaderRuntimeData.cpp
        src/engine/shader/ShaderRuntimeData.h
        src/engine/opengl/header.h
        src/engine/camera/firstPersonCamera.cpp
        src/engine/camera/firstPersonCamera.h
        src/engine/camera/firstPersonOrthoCamera.cpp
        src/engine/camera/firstPersonOrthoCamera.h
        src/engine/camera/firstPersonOrthoStaticCamera.cpp
        src/engine/camera/firstPersonOrthoStaticCamera.h
        src/engine/camera/firstPersonOrthoStaticTopDownCamera.cpp
        src/engine/camera/firstPersonOrthoStaticTopDownCamera.h
        src/include/config.h
        src/engine/camera/CameraInterface.h
        src/engine/persistance/header/commonFileStructs.h
        src/engine/persistance/header/M2FileHeader.h
        src/engine/geometry/m2Geom.cpp
        src/engine/geometry/m2Geom.h
        src/engine/managers/SceneGraph.cpp
        src/engine/managers/SceneGraph.h
        src/engine/persistance/header/wmoFileHeader.h
        src/engine/texture/BlpTexture.cpp
        src/engine/texture/BlpTexture.h
        src/engine/persistance/header/blpFileHeader.h
        src/engine/cache/cache.cpp
        src/engine/cache/cache.h
        src/engine/geometry/skinGeom.cpp
        src/engine/geometry/skinGeom.h
        src/engine/persistance/header/skinFileHeader.h
        src/engine/objects/m2/m2Object.cpp
        src/engine/objects/m2/m2Object.h
        src/engine/algorithms/mathHelper.cpp
        src/engine/algorithms/mathHelper.h
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
        src/engine/objects/iScene.h
        src/engine/objects/scenes/m2Scene.cpp
#        src/engine/objects/scenes/creatureScene.cpp
#        src/engine/objects/scenes/creatureScene.h
        src/engine/objects/scenes/wmoScene.cpp
        src/engine/objects/scenes/wmoScene.h
        src/engine/objects/objectCache.h
        src/engine/algorithms/hashString.h
        src/engine/persistance/header/wdtFileHeader.h
        src/engine/persistance/wdtFile.cpp
        src/engine/persistance/wdtFile.h
        src/engine/managers/particles/particleEmitter.cpp
        src/engine/managers/particles/particleEmitter.h
        src/engine/managers/particles/generators/CParticleGenerator.cpp
        src/engine/managers/particles/generators/CParticleGenerator.h
        src/engine/managers/particles/generators/CSphereGenerator.cpp
        src/engine/managers/particles/generators/CSphereGenerator.h
        src/engine/managers/particles/particle.cpp
        src/engine/managers/particles/particle.h
        src/engine/managers/particles/generators/CSplineGenerator.cpp
        src/engine/managers/particles/generators/CSplineGenerator.h
        src/engine/managers/particles/generators/CBoneGenerator.cpp
        src/engine/managers/particles/generators/CBoneGenerator.h
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
        src/engine/persistance/texFile.cpp src/engine/persistance/texFile.h
        src/engine/persistance/header/texFileHeader.h
        src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.cpp
        src/engine/objects/m2/m2Helpers/M2MeshBufferUpdater.h
        src/engine/wowCommonClasses.h
        src/engine/shader/ShaderDefinitions.cpp
        src/engine/persistance/animFile.cpp
        src/engine/persistance/animFile.h
        src/engine/persistance/m2File.cpp
        src/engine/persistance/m2File.h
        src/gapi/interface/IDevice.cpp
        src/gapi/interface/textures/ITexture.h
        src/gapi/interface/buffers/IBuffer.h
        src/gapi/interface/IOcclusionQuery.h
        src/gapi/interface/IShaderPermutation.h
        src/gapi/interface/IVertexBufferBindings.h
        src/gapi/interface/meshes/IM2Mesh.h
        src/gapi/interface/meshes/IMesh.h
        src/gapi/IDeviceFactory.cpp
        src/gapi/interface/meshes/IParticleMesh.h
        src/gapi/interface/syncronization/IGPUFence.h
        src/engine/engineClassList.h
        src/engine/camera/planarCamera.cpp
        src/engine/camera/planarCamera.h
        src/engine/managers/CRibbonEmitter.cpp
        src/engine/managers/CRibbonEmitter.h
        src/engine/persistance/skelFile.cpp
        src/engine/persistance/skelFile.h
        src/engine/algorithms/FrameCounter.cpp
        src/engine/algorithms/FrameCounter.h
        src/engine/objects/ViewsObjects.cpp
        src/engine/objects/ViewsObjects.h
        src/gapi/interface/sortLambda.h
        src/gapi/interface/buffers/IBufferChunk.h
        src/engine/WowFilesCacheStorage.h
        src/engine/WowFilesCacheStorage.cpp
        src/engine/algorithms/quick-sort-omp.h
        src/include/databaseHandler.h
        src/engine/persistance/PersistentFile.h
        src/gapi/interface/IFrameBuffer.h
        src/include/iostuff.h
        src/engine/ApiContainer.cpp
        src/engine/ApiContainer.h
        src/engine/CameraMatrices.h
        src/engine/persistance/header/wdlHeader.h
        src/engine/camera/m2TiedCamera.cpp
        src/engine/camera/m2TiedCamera.h
        src/engine/persistance/header/M2FileHeader.cpp
        src/engine/objects/m2/m2Helpers/CBoneMasterData.cpp
        src/engine/objects/m2/m2Helpers/CBoneMasterData.h
        src/exporters/IExporter.h
        src/engine/objects/iScene.cpp
        src/engine/algorithms/C3Spline.cpp
        src/engine/algorithms/C3Spline.h
        src/engine/algorithms/C3Spline_Bezier3.cpp
        src/engine/algorithms/C3Spline_Bezier3.h
        src/include/string_utils.h
        src/include/string_utlis.cpp
        src/engine/persistance/db2File.cpp
        src/engine/persistance/db2File.h
        src/engine/algorithms/mathHelper_culling_sse.cpp
        src/engine/algorithms/mathHelper_culling_sse.h
        src/engine/algorithms/mathHelper_culling.cpp
        src/engine/algorithms/mathHelper_culling.h
        src/renderer/frame/SceneScenario.cpp
        src/renderer/frame/SceneComposer.cpp
        src/renderer/frame/SceneComposer.h
        src/renderer/frame/SceneScenario.h
        src/renderer/mapScene/MapSceneRenderer.cpp
        src/renderer/mapScene/MapSceneRenderer.h
        src/renderer/mapScene/MapSceneRendererFactory.cpp
        src/renderer/mapScene/MapSceneRendererFactory.h
        src/renderer/IRenderer.h
        src/renderer/IRenderParameters.h
        src/renderer/frame/FrameInputParams.h
        src/renderer/mapScene/FrameDependentData.h
        src/renderer/buffers/IVertexBufferDynamicTemplate.cpp
        src/renderer/buffers/IVertexBufferDynamicTemplate.h
        src/gapi/interface/IRendererProxy.h src/gapi/interface/meshes/ISortableMesh.h
        src/renderer/mapScene/IMapSceneBufferCreate.h
        src/gapi/interface/materials/IMaterial.h
        src/renderer/mapScene/MapSceneParams.h
        src/include/custom_container_key.h
        src/renderer/mapScene/materials/IMaterialStructs.h
        src/gapi/interface/textures/ITextureSampler.h
        src/gapi/interface/textures/ISamplableTexture.h
        src/renderer/frame/prodConsumerChain/ProdConsumerIOConnector.cpp
        src/renderer/frame/prodConsumerChain/ProdConsumerIOConnector.h
        src/engine/objects/liquid/LiquidInstance.cpp
        src/engine/objects/liquid/LiquidInstance.h
        src/engine/objects/liquid/LiquidDataGetters.h src/renderer/frame/FrameProfile.h src/gapi/interface/buffers/IBufferVersioned.h)

if (LINK_OGL2)
    set(OPENGL20_IMPLEMENTATION
        src/gapi/ogl2.0/textures/GBlpTextureGL20.cpp
        src/gapi/ogl2.0/textures/GBlpTextureGL20.h
        src/gapi/ogl2.0/buffers/GVertexBufferDynamicGL20.cpp
        src/gapi/ogl2.0/buffers/GVertexBufferGL20.cpp
        src/gapi/ogl2.0/buffers/GVertexBufferGL20.h
        src/gapi/ogl2.0/buffers/GIndexBufferGL20.cpp
        src/gapi/ogl2.0/buffers/GIndexBufferGL20.h
        src/gapi/ogl2.0/buffers/GUniformBufferGL20.cpp
        src/gapi/ogl2.0/buffers/GUniformBufferGL20.h
        src/gapi/ogl2.0/GRenderTargetGL20.cpp
        src/gapi/ogl2.0/GRenderTargetGL20.h
        src/gapi/ogl2.0/GDeviceGL20.cpp
        src/gapi/ogl2.0/GDeviceGL20.h
        src/gapi/ogl2.0/meshes/GMeshGL20.cpp
        src/gapi/ogl2.0/meshes/GMeshGL20.h
        src/gapi/ogl2.0/GVertexBufferBindingsGL20.cpp
        src/gapi/ogl2.0/GVertexBufferBindingsGL20.h
        src/gapi/ogl2.0/GShaderPermutationGL20.cpp
        src/gapi/ogl2.0/GShaderPermutationGL20.h
        src/gapi/ogl2.0/shaders/GM2ShaderPermutationGL20.cpp
        src/gapi/ogl2.0/shaders/GM2ShaderPermutationGL20.h
        src/gapi/UniformBufferStructures.h
        src/gapi/ogl2.0/meshes/GM2MeshGL20.cpp
        src/gapi/ogl2.0/meshes/GM2MeshGL20.h
        src/gapi/ogl2.0/textures/GTextureGL20.cpp
        src/gapi/ogl2.0/textures/GTextureGL20.h
        src/gapi/ogl2.0/shaders/GM2ParticleShaderPermutationGL20.cpp
        src/gapi/ogl2.0/shaders/GM2ParticleShaderPermutationGL20.h
        src/gapi/ogl2.0/meshes/GParticleMeshGL20.cpp
        src/gapi/ogl2.0/meshes/GParticleMeshGL20.h
        src/gapi/ogl2.0/shaders/GWMOShaderPermutationGL20.cpp
        src/gapi/ogl2.0/shaders/GWMOShaderPermutationGL20.h
        src/gapi/ogl2.0/shaders/GAdtShaderPermutationGL20.cpp
        src/gapi/ogl2.0/shaders/GAdtShaderPermutationGL20.h
        src/gapi/ogl2.0/GOcclusionQueryGL20.cpp
        src/gapi/ogl2.0/GOcclusionQueryGL20.h)
endif()

if (LINK_OGL3)
set(OPENGL33_IMPLEMENTATION
        src/gapi/ogl3.3/textures/GBlpTextureGL33.cpp
        src/gapi/ogl3.3/textures/GBlpTextureGL33.h
        src/gapi/ogl3.3/buffers/GVertexBufferDynamicGL33.cpp
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
        src/gapi/ogl3.3/GOcclusionQueryGL33.h
        src/gapi/ogl3.3/buffers/GUnformBufferChunk33.h
        src/gapi/ogl3.3/GFrameBufferGL33.cpp
        src/gapi/ogl3.3/GFrameBufferGL33.h
        src/gapi/ogl3.3/shaders/GFFXgauss4.cpp
        src/gapi/ogl3.3/shaders/GSkyConus.cpp
        src/gapi/ogl3.3/shaders/GFFXgauss4.h
        src/gapi/ogl3.3/shaders/GFFXGlow.cpp
        src/gapi/ogl3.3/shaders/GFFXGlow.h
        src/gapi/ogl3.3/shaders/GWaterShaderGL33.cpp
        src/gapi/ogl3.3/shaders/GWaterShaderGL33.h
        src/gapi/ogl3.3/shaders/GWaterfallShaderGL33.cpp
        src/gapi/ogl3.3/shaders/GWaterfallShaderGL33.h)
endif()

if (LINK_OGL4)
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
        src/gapi/ogl4.x/GOcclusionQueryGL4x.h src/gapi/ogl4.x/syncronization/GPUFenceGL44.cpp src/gapi/ogl4.x/syncronization/GPUFenceGL44.h src/engine/persistance/header/skelFileHeader.h)
endif()

if (LINK_VULKAN)
    set(VULKAN_IMPLEMENTATION
            src/gapi/vulkan/GDeviceVulkan.cpp
            src/gapi/vulkan/GDeviceVulkan.h
            src/gapi/vulkan/vkAllocator.cpp
            src/gapi/vulkan/GVertexBufferBindingsVLK.cpp
            src/gapi/vulkan/shaders/GShaderPermutationVLK.cpp
            src/gapi/vulkan/meshes/GM2MeshVLK.cpp
            src/gapi/vulkan/meshes/GMeshVLK.cpp
            src/gapi/vulkan/textures/GTextureVLK.cpp
            src/gapi/vulkan/textures/GBlpTextureVLK.cpp
            src/gapi/vulkan/GPipelineVLK.cpp src/gapi/vulkan/GPipelineVLK.h
            src/gapi/vulkan/descriptorSets/GDescriptorPoolVLK.cpp
            src/gapi/vulkan/descriptorSets/GDescriptorPoolVLK.h
            src/gapi/vulkan/descriptorSets/GDescriptorSet.cpp
            src/gapi/vulkan/descriptorSets/GDescriptorSet.h
            src/gapi/vulkan/GFrameBufferVLK.cpp
            src/gapi/vulkan/GFrameBufferVLK.h
            src/gapi/vulkan/GRenderPassVLK.cpp
            src/gapi/vulkan/GRenderPassVLK.h
            src/renderer/mapScene/vulkan/MapSceneRenderForwardVLK.cpp
            src/renderer/mapScene/vulkan/MapSceneRenderForwardVLK.h
#            src/renderer/mapScene/vulkan/MapSceneRenderVisBufferVLK.cpp
#            src/renderer/mapScene/vulkan/MapSceneRenderVisBufferVLK.h
            src/renderer/mapScene/vulkan/passes/FFXGlowPassVLK.h
            src/renderer/mapScene/vulkan/passes/FFXGlowPassVLK.cpp
            src/gapi/vulkan/buffers/GBufferVLK.cpp
            src/gapi/vulkan/buffers/GBufferVLK.h
            src/gapi/vulkan/buffers/IBufferVLK.h
            src/gapi/vulkan/materials/ISimpleMaterialVLK.cpp
            src/gapi/vulkan/materials/ISimpleMaterialVLK.h
            src/gapi/vulkan/buffers/CBufferChunkVLK.cpp
            src/gapi/vulkan/buffers/CBufferChunkVLK.h
            src/gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h
            src/gapi/vulkan/descriptorSets/GDescriptorSetLayout.cpp
            src/gapi/vulkan/descriptorSets/GDescriptorSetLayout.h
            src/gapi/vulkan/commandBuffer/CommandBuffer.cpp
            src/gapi/vulkan/commandBuffer/CommandBuffer.h
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.cpp
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/RenderPassHelper.cpp
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/RenderPassHelper.h
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/TextureUploadHelper.cpp
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/TextureUploadHelper.h
            src/gapi/vulkan/IDeviceVulkan.h
            src/gapi/vulkan/synchronization/GSemaphoreVLK.cpp
            src/gapi/vulkan/synchronization/GSemaphoreVLK.h
            src/gapi/vulkan/synchronization/GFenceVLK.cpp
            src/gapi/vulkan/synchronization/GFenceVLK.h
            src/renderer/vulkan/IRenderFunctionVLK.h
            src/gapi/vulkan/descriptorSets/DescriptorRecord.h
            src/gapi/vulkan/materials/MaterialBuilderVLK.cpp
            src/gapi/vulkan/materials/MaterialBuilderVLK.h
            src/gapi/vulkan/TextureManagerVLK.cpp
            src/gapi/vulkan/TextureManagerVLK.h
            src/gapi/vulkan/utils/MutexLockedVector.h
            src/gapi/vulkan/descriptorSets/DescriptorResourceCallBack.h
            src/gapi/vulkan/bindable/DSBindable.h
            src/renderer/mapScene/vulkan/materials/IMaterialInstance.h
            src/gapi/vulkan/meshes/GSortableMeshVLK.cpp
            src/gapi/vulkan/meshes/GSortableMeshVLK.h
            src/gapi/vulkan/textures/GTextureSamplerVLK.cpp
            src/gapi/vulkan/textures/GTextureSamplerVLK.h
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferDebugLabel.cpp
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferDebugLabel.h
            src/gapi/vulkan/context/vulkan_context.h
            src/gapi/vulkan/shaders/ShaderConfig.h
            src/gapi/vulkan/buffers/GBufferChunkDynamicVLK.cpp
            src/gapi/vulkan/buffers/GBufferChunkDynamicVLK.h
            src/gapi/vulkan/GDescriptorSetUpdater.cpp
            src/gapi/vulkan/GDescriptorSetUpdater.h
            src/gapi/vulkan/commandBuffer/CommandBufferThread.cpp
            src/gapi/vulkan/commandBuffer/CommandBufferThread.h
            src/gapi/vulkan/buffers/GStagingRingBuffer.cpp
            src/gapi/vulkan/buffers/GStagingRingBuffer.h
            src/gapi/vulkan/buffers/gpu/BufferStagingVLK.cpp
            src/gapi/vulkan/buffers/gpu/BufferStagingVLK.h
            src/gapi/vulkan/buffers/gpu/BufferGpuVLK.cpp
            src/gapi/vulkan/buffers/gpu/BufferGpuVLK.h
            src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h)

    message(VULKAN_SDK = $ENV{VULKAN_SDK})
    find_package(Vulkan)
    message(Vulkan_LIBRARY = ${Vulkan_LIBRARY})
    message(Vulkan_DIR = ${Vulkan_DIR})
    IF (Vulkan_LIBRARY)
        set(Vulkan_FOUND ON PARENT_SCOPE)
        MESSAGE("Using Vulkan library, Vulkan_LIBRARY= ${Vulkan_LIBRARY}")
    ELSE()
        MESSAGE("Vulkan Lib was not found. Disabling vulkan build")

        set(LINK_VULKAN OFF)
        set(VULKAN_IMPLEMENTATION "")
    ENDIF()


    if (Vulkan_LIBRARY)
        message(Vulkan_INCLUDE_DIR = ${Vulkan_INCLUDE_DIR})
        if (Vulkan_INCLUDE_DIR STREQUAL "")
            message(FATAL_ERROR "Vulkan_INCLUDE_DIR is empty")
        endif()

        get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
        message(Vulkan_LIBRARY_DIR = ${Vulkan_LIBRARY_DIR})

        #        IF(WIN32)
        #            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
        #        endif()

        if(CMAKE_SYSTEM_NAME STREQUAL Windows)
            set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
        elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
            set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
        elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
            set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
            #We need to bundle moltenVK and vulkan lib on macos
            file(GLOB vulkanLibFiles RELATIVE ${srcDir} ${srcDir}/*.*)

            install(FILES ${Vulkan_LIBRARY} DESTINATION ${CMAKE_INSTALL_BINDIR})
            #TODO: The moltenVK needs to be built separately
            #            install(FILES ${Vulkan_LIBRARY_DIR}/libMoltenVK.dylib DESTINATION ${CMAKE_INSTALL_BINDIR})
        endif()
        #Add volk loader
        set(VULKAN_IMPLEMENTATION ${VULKAN_IMPLEMENTATION} src/gapi/vulkan/volk.c)
    endif()


    if (LINK_VULKAN)
        add_definitions(-DLINK_VULKAN)
    else()
        set(Vulkan_INCLUDE_DIR "")
    endif()
endif()

if (NOT MSVC)
    check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
    if(HAVE_FLAG_STD_CXX17)
        # Have -std=c++17, use it
        message("WOWLIB HAVE_FLAG_STD_CXX17 is supported")
        #    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++17" )
        #    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++17" )
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17" )
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17" )
    else()
        check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)
        if(HAVE_FLAG_STD_CXX1Z)
            #        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++1z" )
            #        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++1z" )
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z")
        else()
            message(ERROR "No supported flags")
        endif()
    endif()
endif()



add_library(WoWViewerLib STATIC ${SOURCE_FILES} ${OPENGL20_IMPLEMENTATION} ${OPENGL33_IMPLEMENTATION} ${OPENGL4x_IMPLEMENTATION} ${VULKAN_IMPLEMENTATION})
if (NOT MSVC)
    target_compile_options(WoWViewerLib PUBLIC -Werror=return-type)
endif()
if (MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
    message("MSVC Is on")
    if (_cpp_17_flag_supported)
        message("/std:c++17 is supported")
        #target_compile_options(AWebWoWViewerCpp "/std:c++17")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /std:c++17")
        target_compile_options(WoWViewerLib PRIVATE "/std:c++latest")
    endif()
endif()

if (LINK_VULKAN)
    target_compile_definitions(WoWViewerLib PUBLIC ${VOLK_STATIC_DEFINES})
endif()

if (LINK_TRACY)
    target_compile_definitions(WoWViewerLib PUBLIC -DLINK_TRACY)
    target_include_directories(WoWViewerLib PUBLIC ${TRACY_INCLUDE_DIR})
    target_link_libraries (WoWViewerLib Tracy::TracyClient )
endif()

foreach(X IN LISTS My_Vectorize_Compile_Options)
    target_compile_options(WoWViewerLib PRIVATE ${X})
endforeach()
if (NOT MSVC)
    target_compile_features(WoWViewerLib PRIVATE cxx_std_17)
    set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)
else()
    target_compile_features(WoWViewerLib PRIVATE cxx_std_20)
#    set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)
endif()

if (LINK_OPENMP)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        MESSAGE("OPENMP CLANG ROUTE WAS TAKEN")
        target_compile_options(WoWViewerLib PRIVATE -fopenmp=libomp)
        target_link_libraries(WoWViewerLib -fopenmp=libomp)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        MESSAGE("OPENMP GNU ROUTE WAS TAKEN")
        target_compile_options(WoWViewerLib PRIVATE -fopenmp)
        target_link_libraries(WoWViewerLib -fopenmp)
    endif()
endif()

target_link_libraries(WoWViewerLib ${GLEW_LIBRARIES})
target_link_libraries(WoWViewerLib TBB::tbb )
if (UNIX AND NOT EMSCRIPTEN)
target_link_libraries(WoWViewerLib Threads::Threads)
endif()
#target_link_options(WoWViewerLib PUBLIC "SHELL: -lstdc++")

if (WIN32)
    install(FILES ${GLEW_DLL}
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
endif()

if (ENABLE_SIMD)
    mathfu_configure_flags(WoWViewerLib TRUE)
else()
    mathfu_configure_flags(WoWViewerLib FALSE) #This being TRUE somehow breaks current ribbonEmitter code
endif()

if (LINK_VULKAN)
    message("Include vulkan include dir ${Vulkan_INCLUDE_DIR}")
    target_include_directories(WoWViewerLib PUBLIC ${Vulkan_INCLUDE_DIR})
    target_compile_definitions(WoWViewerLib PUBLIC -DLINK_VULKAN)
#    target_link_libraries(WoWViewerLib ${Vulkan_LIBRARY})
    #Vulkan stuff end
else()
    target_compile_definitions(WoWViewerLib PUBLIC -DSKIP_VULKAN)
endif()

if (LINK_EGL)
    target_compile_definitions(WoWViewerLib PUBLIC -DWITH_GLESv2)
    target_include_directories(WoWViewerLib PUBLIC ${EGL_PATH}/include/)
    target_link_libraries(WoWViewerLib ${EGL_PATH}/libGLESv2.lib)
    target_link_libraries(WoWViewerLib ${EGL_PATH}/libEGL.lib)
endif()

if (1)
    add_subdirectory(shaders)

    add_dependencies(WoWViewerLib SPIRV_EXTRACT_META)
    if (LINK_OGL2)
        add_dependencies(WoWViewerLib ShadersGLSL20)
    endif()
    if (LINK_OGL3)
        add_dependencies(WoWViewerLib ShadersGLSL33)
    endif()
endif()

set(ext_inDir ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include)

if (LINK_VULKAN)
    set(ext_inDir ${ext_inDir} ${Vulkan_INCLUDE_DIR})
endif()
set(WoWViewerLib_INCLUDE_DIRS ${ext_inDir} CACHE INTERNAL "")

message(WoWViewerLib_INCLUDE_DIRS = ${WoWViewerLib_INCLUDE_DIRS})

target_include_directories(WoWViewerLib
        PUBLIC ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include)
