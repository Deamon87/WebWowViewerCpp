
# 'lib' is the folder with Google Test sources
add_subdirectory(../3rdparty/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_build_include_dirs})

# 'Google_Tests_run' is the target name
# 'test1.cpp test2.cpp' are source files with tests
add_executable(wow_viewer_tests_run src/tests1.cpp src/cameraTests.cpp src/projectionTexturingTests.cpp)
target_link_libraries(wow_viewer_tests_run gtest gtest_main gmock)