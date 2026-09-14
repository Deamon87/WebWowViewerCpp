cmake_minimum_required(VERSION 3.10)

project(OfflineRenderer)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/wowViewerLib/src)
include_directories(${CMAKE_SOURCE_DIR}/wowViewerLib/3rdparty/mathfu/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/clipboard/include)

# Add definitions
add_definitions(-DLINK_VULKAN)

# Source files
set(SOURCE_FILES
    main.cpp
)

# Create executable
add_executable(offlineRenderer ${SOURCE_FILES})

# Link libraries - same as test project
target_link_libraries(offlineRenderer 
    WoWFrontendLib
    WoWViewerLib
)

# Set output directory
set_target_properties(offlineRenderer PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

# Include directories for the target
target_include_directories(offlineRenderer
    PRIVATE
    "$<TARGET_PROPERTY:WoWFrontendLib,INTERFACE_INCLUDE_DIRECTORIES>"
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/wowViewerLib/src
    ${CMAKE_SOURCE_DIR}/wowViewerLib/3rdparty/mathfu/include
)

# Compile definitions
target_compile_definitions(offlineRenderer PRIVATE -DLINK_VULKAN)

