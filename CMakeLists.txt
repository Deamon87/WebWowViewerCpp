cmake_minimum_required(VERSION 3.8)

project(WebWoWViewerCpp)
include(ExternalProject)

#Zipper library
ExternalProject_Add("Zipper"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zipper"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/zipper -DEXTRA_LIBS=stdc++fs
        )

set(Zipper_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/zipper/include")
ADD_LIBRARY(zipper STATIC IMPORTED)
link_directories(${CMAKE_SOURCE_DIR}/lib/zipper/lib/)
SET_TARGET_PROPERTIES(zipper PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/zipper/lib/libZipper-static.a)
include_directories(${Zipper_INCLUDE_DIRS})

#Main library
ExternalProject_Add("WowViewerLib"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/wowViewerLib"
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/wowViewerLib
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        )
ExternalProject_Get_Property(WowViewerLib install_dir)
include_directories(${install_dir}/include)

set(SOURCE_FILES src/main.cpp src/persistance/httpFile/httpFile.cpp src/persistance/httpFile/httpFile.h wowViewerLib/src/engine/stringTrim.h wowViewerLib/src/engine/managers/animationManager.cpp wowViewerLib/src/engine/managers/animationManager.h)

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################
# FIND GLEW\GLFW
#########################################################
if (NOT EMSCRIPTEN_BUILD)
    find_package(GLEW REQUIRED)
    if (GLEW_FOUND)
        include_directories(${GLEW_INCLUDE_DIRS})
        link_libraries(${GLEW_LIBRARIES})
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    include_directories(${GLFW_INCLUDE_DIRS})
endif()
#########################################################
# FIND CURL
#########################################################
if (NOT EMSCRIPTEN_BUILD)
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARIES})
    add_definitions(${CURL_DEFINITIONS})
    if(NOT CURL_FOUND)
        message(ERROR " CURL not found!")
    endif(NOT CURL_FOUND)
endif()

#Build the executiable
add_executable(WebWoWViewerCpp ${SOURCE_FILES})

#TODO: platform dependant!!
target_link_libraries(WebWoWViewerCpp ${install_dir}/lib/libWoWViewerLib.a)

add_dependencies(WebWoWViewerCpp WowViewerLib)

set_property(TARGET WebWoWViewerCpp PROPERTY CXX_STANDARD 17)
target_link_libraries(WebWoWViewerCpp ${OPENGL_LIBRARIES} ${GLFW_STATIC_LIBRARIES} ${CURL_LIBRARIES} zipper z stdc++fs)

