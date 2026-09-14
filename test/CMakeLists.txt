
set(gtest_force_shared_crt on)

# 'lib' is the folder with Google Test sources
add_subdirectory(../3rdparty/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_build_include_dirs})


set(EXTERNAL_RENDER ${CMAKE_CURRENT_SOURCE_DIR}/src/sceneTests/externalRenderScene.h)

add_executable(wow_viewer_tests_run src/sceneTests/sceneTests.cpp src/cameraTests.cpp src/projectionTexturingTests.cpp)
target_link_libraries(wow_viewer_tests_run gtest gtest_main gmock)

if(EXISTS "${EXTERNAL_RENDER}")
    target_compile_definitions(wow_viewer_tests_run PRIVATE INCLUDE_EXTERNAL=1)
endif()