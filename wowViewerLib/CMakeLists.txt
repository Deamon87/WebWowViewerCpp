cmake_minimum_required(VERSION 2.8)
project(WoWViewerLib)

include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(LINK_GLEW 1)
set(LINK_OGL2 1)
set(LINK_OGL3 1)
set(LINK_OGL4 0)
set(LINK_EGL 1 PARENT_SCOPE)
set(LINK_EGL 1)
option(LINK_VULKAN "Enable Vulkan" ON)
set(LINK_OPENMP 0)
set(ENABLE_SIMD 1)
option(EMSCRIPTEN_SIMD "Enable SIMD for Emscripten " OFF)

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
if (LINK_VULKAN EQUAL ON)
    add_definitions(-DLINK_VULKAN)
endif()

set(My_Vectorize_Compile_Options "")


message("ENABLE_SIMD = ${ENABLE_SIMD}")


SET(TBB_BUILD_SHARED 0)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/tbb)

option(BUILD_WITHOUT_CULLED_PORTAL_DRAWING "Build without drawing culled portals" ON)

if (BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCULLED_NO_PORTAL_DRAWING")
endif(BUILD_WITHOUT_CULLED_PORTAL_DRAWING MATCHES ON)

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

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-multichar -g -O3" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-multichar -g -O3")
endif()




IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

IF(CMAKE_BUILD_TYPE MATCHES Release)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)


include(ExternalProject)

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/mathfu)

message(CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME})
string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWERCASE)
if (LINK_GLEW)
    if (WIN32)
        MESSAGE(IT IS WINDOWS!!!!)
        set(GLEW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/include)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/x64/glew32.lib )
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/3rdparty/glew_win/lib/Release/Win32/glew32.lib )
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
        src/engine/managers/SceneGraph.cpp src/engine/managers/SceneGraph.h src/engine/persistance/header/wmoFileHeader.h
        src/engine/texture/BlpTexture.cpp src/engine/texture/BlpTexture.h src/engine/persistance/header/blpFileHeader.h
        src/engine/cache/cache.cpp src/engine/cache/cache.h src/engine/geometry/skinGeom.cpp
        src/engine/geometry/skinGeom.h src/engine/persistance/header/skinFileHeader.h
        src/engine/objects/m2/m2Object.cpp src/engine/objects/m2/m2Object.h
        src/engine/algorithms/mathHelper.cpp src/engine/algorithms/mathHelper.h
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
        src/engine/algorithms/PortalCullingAlgo.cpp
        src/engine/algorithms/PortalCullingAlgo.h
        src/engine/objects/scenes/wmoScene.cpp
        src/engine/objects/scenes/wmoScene.h
        src/engine/objects/objectCache.h
        src/engine/algorithms/hashString.h
        src/engine/persistance/header/wdtFileHeader.h
        src/engine/persistance/wdtFile.cpp
        src/engine/persistance/wdtFile.h
        src/engine/managers/particles/particleEmitter.cpp
        src/engine/managers/particles/particleEmitter.h
        src/engine/managers/particles/generators/CParticleGenerator.cpp src/engine/managers/particles/generators/CParticleGenerator.h src/engine/managers/particles/generators/CSphereGenerator.cpp src/engine/managers/particles/generators/CSphereGenerator.h src/engine/managers/particles/particle.cpp src/engine/managers/particles/particle.h src/engine/managers/particles/generators/CSplineGenerator.cpp src/engine/managers/particles/generators/CSplineGenerator.h src/engine/managers/particles/generators/CBoneGenerator.cpp src/engine/managers/particles/generators/CBoneGenerator.h
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
        src/gapi/IDeviceFactory.cpp
        src/gapi/interface/meshes/IParticleMesh.h
        src/gapi/interface/syncronization/IGPUFence.h
        src/engine/objects/wowFrameData.h
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
        src/gapi/interface/buffers/IUniformBufferChunk.h
        src/engine/WowFilesCacheStorage.h
        src/engine/WowFilesCacheStorage.cpp
        src/engine/algorithms/quick-sort-omp.h
        src/include/databaseHandler.h
        src/engine/persistance/PersistentFile.h
        src/engine/SceneComposer.cpp
        src/engine/SceneComposer.h
        src/engine/SceneScenario.h
        src/gapi/interface/IFrameBuffer.h
        src/engine/SceneScenario.cpp
        src/engine/DrawStage.h
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
        src/engine/objects/iScene.cpp src/engine/algorithms/C3Spline.cpp src/engine/algorithms/C3Spline.h src/engine/algorithms/C3Spline_Bezier3.cpp src/engine/algorithms/C3Spline_Bezier3.h src/include/string_utils.h src/include/string_utlis.cpp)

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

if (LINK_VULKAN EQUAL ON)
    set(VULKAN_IMPLEMENTATION
            src/include/vulkancontext.h
            src/gapi/vulkan/GDeviceVulkan.cpp
            src/gapi/vulkan/GDeviceVulkan.h
            src/gapi/vulkan/vkAllocator.cpp
            src/gapi/vulkan/GVertexBufferBindingsVLK.cpp
            src/gapi/vulkan/shaders/GShaderPermutationVLK.cpp
            src/gapi/vulkan/shaders/GAdtShaderPermutationVLK.cpp
            src/gapi/vulkan/shaders/GM2ShaderPermutationVLK.cpp
            src/gapi/vulkan/shaders/GM2ParticleShaderPermutationVLK.cpp
            src/gapi/vulkan/shaders/GM2RibbonShaderPermutationVLK.cpp
            src/gapi/vulkan/shaders/GWMOShaderPermutationVLK.cpp
            src/gapi/vulkan/meshes/GM2MeshVLK.cpp
            src/gapi/vulkan/meshes/GMeshVLK.cpp
            src/gapi/vulkan/textures/GTextureVLK.cpp
            src/gapi/vulkan/textures/GBlpTextureVLK.cpp
            src/gapi/vulkan/buffers/GUniformBufferVLK.cpp
            src/gapi/vulkan/buffers/GVertexBufferDynamicVLK.cpp
            src/gapi/vulkan/buffers/GVertexBufferVLK.cpp
            src/gapi/vulkan/buffers/GIndexBufferVLK.cpp
            src/gapi/vulkan/GPipelineVLK.cpp src/gapi/vulkan/GPipelineVLK.h
            src/gapi/vulkan/descriptorSets/GDescriptorPoolVLK.cpp
            src/gapi/vulkan/descriptorSets/GDescriptorPoolVLK.h
            src/gapi/vulkan/descriptorSets/GDescriptorSet.cpp
            src/gapi/vulkan/descriptorSets/GDescriptorSet.h
            src/gapi/vulkan/shaders/GImguiShaderPermutation.cpp
            src/gapi/vulkan/shaders/GImguiShaderPermutation.h
            src/gapi/vulkan/shaders/GWaterShaderPermutation.cpp
            src/gapi/vulkan/shaders/GWaterShaderPermutation.h
            src/gapi/vulkan/shaders/GSkyConusShaderVLK.cpp
            src/gapi/vulkan/shaders/GSkyConusShaderVLK.h
            src/gapi/vulkan/GFrameBufferVLK.cpp
            src/gapi/vulkan/GFrameBufferVLK.h
            src/gapi/vulkan/shaders/GFFXgauss4VLK.cpp
            src/gapi/vulkan/shaders/GFFXgauss4VLK.h
            src/gapi/vulkan/shaders/GFFXGlowVLK.cpp
            src/gapi/vulkan/shaders/GFFXGlowVLK.h
            src/gapi/vulkan/GRenderPassVLK.cpp
            src/gapi/vulkan/GRenderPassVLK.h src/gapi/vulkan/shaders/GDrawBoundingBoxVLK.cpp src/gapi/vulkan/shaders/GDrawBoundingBoxVLK.h src/gapi/vulkan/shaders/GWaterfallShaderVLK.cpp src/gapi/vulkan/shaders/GWaterfallShaderVLK.h)
    include_directories(${Vulkan_INCLUDE_DIR})
    IF(WIN32)
        IF (NOT Vulkan_FOUND)
            find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS "$ENV{VULKAN_SDK}/lib")
            IF (Vulkan_LIBRARY)
                set(Vulkan_FOUND ON)
                MESSAGE("Using bundled Vulkan library version win32, Vulkan_LIBRARY= ${Vulkan_LIBRARY}")
            ENDIF()
        ENDIF()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
    ELSE(WIN32)
        IF (NOT Vulkan_FOUND)
            find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
            IF (Vulkan_LIBRARY)
                set(Vulkan_FOUND ON)
                MESSAGE("Using bundled Vulkan library version non-win32")
            ENDIF()
        ENDIF()
        find_package(Threads REQUIRED)
    ENDIF()
endif()

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

add_library(WoWViewerLib STATIC ${SOURCE_FILES} ${OPENGL20_IMPLEMENTATION} ${OPENGL33_IMPLEMENTATION} ${OPENGL4x_IMPLEMENTATION} ${VULKAN_IMPLEMENTATION})
if (NOT MSVC)
    target_compile_options(WoWViewerLib PUBLIC -Werror=return-type)
endif()
foreach(X IN LISTS My_Vectorize_Compile_Options)
    target_compile_options(WoWViewerLib PRIVATE ${X})
endforeach()
target_compile_features(WoWViewerLib PRIVATE cxx_std_17)
set_property(TARGET WoWViewerLib PROPERTY CXX_STANDARD 17)

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
if (LINK_VULKAN EQUAL ON)
    target_link_libraries(WoWViewerLib  ${Vulkan_LIBRARY})
endif()

target_link_libraries(WoWViewerLib ${GLEW_LIBRARIES})
target_link_libraries(WoWViewerLib tbb_static)
#target_link_options(WoWViewerLib PUBLIC "SHELL: -lstdc++")


if (ENABLE_SIMD)
    mathfu_configure_flags(WoWViewerLib TRUE)
else()
    mathfu_configure_flags(WoWViewerLib FALSE) #This being TRUE somehow breaks current ribbonEmitter code
endif()

if (LINK_VULKAN EQUAL ON)
    message("Include vulkan include dir ${Vulkan_INCLUDE_DIR}")
    target_include_directories(WoWViewerLib PUBLIC ${Vulkan_INCLUDE_DIR})
    target_compile_definitions(WoWViewerLib PUBLIC -DLINK_VULKAN)
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

if (LINK_VULKAN EQUAL ON)
    set(ext_inDir ${ext_inDir} ${Vulkan_INCLUDE_DIR})
endif()
set(WoWViewerLib_INCLUDE_DIRS ${ext_inDir} CACHE INTERNAL "")

message(WoWViewerLib_INCLUDE_DIRS = ${WoWViewerLib_INCLUDE_DIRS})

target_include_directories(WoWViewerLib
        PUBLIC ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/include ${PROJECT_SOURCE_DIR}/3rdparty/mathfu/dependencies/vectorial/include)
