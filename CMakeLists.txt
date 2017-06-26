cmake_minimum_required(VERSION 3.8)

project(WebWoWViewerCpp)
include(ExternalProject)

if (WIN32)
	SET(zlibLib zlibstaticd)
	SET(extra_libraries "")

else()
	SET(zlibLib libzlibstatic.a)
	SET(extra_libraries stdc++fs)
endif()

#ZLib library
ExternalProject_Add("libz"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zlib"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/libz -DEXTRA_LIBS=${extra_libraries}
        )
set(LIBZ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/libz/include")

#Zlib
ExternalProject_Add("LibZip"
		SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/libzip"
		CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/libzip -DEXTRA_LIBS=${extra_libraries}
		)
#add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/libzip)
set(libzip_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/libzip/include/")
#set(libzip_HEADER_FILES ${CMAKE_BINARY_DIR}/3rdparty/libzip/)
set(libzip_HEADER_FILES ${CMAKE_SOURCE_DIR}/lib/libzip/lib/libzip/include)
set(libzip_library ${CMAKE_SOURCE_DIR}/lib/libzip/lib/libzip.dll.a)
message(libzip_HEADER_FILES = ${libzip_HEADER_FILES})
include_directories(${libzip_HEADER_FILES})
include_directories(${libzip_INCLUDE_DIRS})

#GLFW
ExternalProject_Add("GLFW"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/glfw"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/glfw -DEXTRA_LIBS=${extra_libraries}
        )
ExternalProject_Get_Property(GLFW install_dir)
set(glfw_install_dir ${CMAKE_SOURCE_DIR}/lib/glfw)
include_directories(${glfw_install_dir}/include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#    GET_TARGET_PROPERTIES(zipper PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/zipper/lib/libZipper-static.obj)
	set(GLFW_STATIC_LIBRARIES ${CMAKE_SOURCE_DIR}/lib/glfw/lib/libglfw3dll.a)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#    SET_TARGET_PROPERTIES(zipper PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/zipper/lib/libZipper-static.a)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#CURL
#ExternalProject_Add("CURL"
#        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/curl"
#        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBUILD_CURL_EXE=OFF -DCURL_STATICLIB=ON -DHTTP_ONLY=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/curl -DEXTRA_LIBS=${extra_libraries}
#        )
#
#include_directories(${CMAKE_SOURCE_DIR}/3rdparty/curl/include)
#link_directories(${CURL_LIBRARIES})

#CPR
add_subdirectory(cpr)
include_directories(${CPR_INCLUDE_DIRS})


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
set(WowViewerLib "${install_dir}/lib/libWoWViewerLib.a")

set(SOURCE_FILES src/main.cpp src/persistance/httpFile/httpFile.cpp src/persistance/httpFile/httpFile.h )

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
add_dependencies(WebWoWViewerCpp LibZip)
#add_dependencies(Zipper libz)
#add_dependencies(WebWoWViewerCpp libzip)
add_dependencies(WebWoWViewerCpp GLFW)
add_dependencies(WebWoWViewerCpp WowViewerLib)

#TODO: platform dependant!!
#target_link_libraries(WebWoWViewerCpp ${install_dir}/lib/libWoWViewerLib)
#target_link_libraries(WebWoWViewerCpp ${glfw_install_dir}/lib/libglfw3dll)


set_property(TARGET WebWoWViewerCpp PROPERTY CXX_STANDARD 17)
#target_link_libraries(WebWoWViewerCpp zipper stdc++fs)
#target_link_libraries(WebWoWViewerCpp zipper stdc++fs)
message(INFO ${OPENGL_LIBRARIES})
#glu32.a opengl32.a z
target_link_libraries(WebWoWViewerCpp ${GLFW_STATIC_LIBRARIES} ${CURL_LIBRARIES} ${libzip_library} ${WowViewerLib})
target_link_libraries(WebWoWViewerCpp ${CPR_LIBRARIES})
