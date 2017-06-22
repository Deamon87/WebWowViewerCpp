cmake_minimum_required(VERSION 3.8)

project(WebWoWViewerCpp)
include(ExternalProject)

#Zipper library
ExternalProject_Add("libz"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zlib"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/libz -DEXTRA_LIBS=stdc++fs
        )
set(LIBZ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/libz/include")

#Zipper library

ExternalProject_Add("Zipper"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zipper"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DBuildShared=ON -DBuildExamples=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/zipper
        -DEXTRA_LIBS=stdc++fs
        -DLIBZ_INCLUDE_DIR=${LIBZ_INCLUDE_DIR}
        -DLIBZ_LIBRARY=${CMAKE_SOURCE_DIR}/lib/libz/lib/libzlibstatic.a
        )



set(Zipper_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/zipper/include")
ADD_LIBRARY(zipper STATIC IMPORTED)
link_directories(${CMAKE_SOURCE_DIR}/lib/zipper/lib/)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET_TARGET_PROPERTIES(zipper PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/zipper/lib/libZipper-static.obj)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET_TARGET_PROPERTIES(zipper PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/zipper/lib/libZipper-static.a)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
include_directories(${Zipper_INCLUDE_DIRS})

#GLFW
ExternalProject_Add("GLFW"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/glfw"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/glfw -DEXTRA_LIBS=stdc++fs
        )
ExternalProject_Get_Property(GLFW install_dir)
set(glfw_install_dir ${CMAKE_SOURCE_DIR}/lib/glfw)
include_directories(${glfw_install_dir}/include)

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

set(SOURCE_FILES src/main.cpp src/persistance/httpFile/httpFile.cpp src/persistance/httpFile/httpFile.h)

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
message(OpenGL_INCLUDE_DIRS = ${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################
# FIND CURL
#########################################################
#if (NOT EMSCRIPTEN_BUILD)
#    find_package(CURL REQUIRED)
#    include_directories(${CURL_INCLUDE_DIRS})
#    link_directories(${CURL_LIBRARIES})
#    add_definitions(${CURL_DEFINITIONS})
#    #message(ERROR " CURL_DEFINITIONS = ${CURL_DEFINITIONS} CURL_INCLUDE_DIRS = ${CURL_INCLUDE_DIRS}")
#    if(NOT CURL_FOUND)
#        message(ERROR " CURL not found!")
#    endif(NOT CURL_FOUND)
#endif()

#Build the executiable
add_executable(WebWoWViewerCpp ${SOURCE_FILES})


add_dependencies(WebWoWViewerCpp libz)
add_dependencies(Zipper libz)
add_dependencies(WebWoWViewerCpp Zipper)
add_dependencies(WebWoWViewerCpp GLFW)
add_dependencies(WebWoWViewerCpp WowViewerLib)

#TODO: platform dependant!!
#target_link_libraries(WebWoWViewerCpp ${install_dir}/lib/libWoWViewerLib)
#target_link_libraries(WebWoWViewerCpp ${glfw_install_dir}/lib/libglfw3dll)


set_property(TARGET WebWoWViewerCpp PROPERTY CXX_STANDARD 17)
#target_link_libraries(WebWoWViewerCpp zipper stdc++fs)
#target_link_libraries(WebWoWViewerCpp zipper stdc++fs)
message(INFO ${OPENGL_LIBRARIES})
target_link_libraries(WebWoWViewerCpp glu32.a opengl32.a ${GLFW_STATIC_LIBRARIES} ${CURL_LIBRARIES} z)

