cmake_minimum_required(VERSION 3.6)
cmake_minimum_required(VERSION 3.6)

project(WebWoWViewerCpp)
include(ExternalProject)

if (CMAKE_NDK_BUILD MATCHES 1)
	include_directories(BEFORE SYSTEM "${NDK_PATH}/include/c++/4.9.x/" "${NDK_PATH}/sysroot/usr/include/")
	if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
		include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
	elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
		include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
	elseif(${ANDROID_ABI} STREQUAL "x86_64")
		include_directories(${ANDROID_SYSROOT}/usr/include/x86_64-linux-android)
	else()
		include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
	endif()
endif()

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

if (WIN32)
	SET(zlibLib zlibstaticd)
	SET(extra_libraries "")
else()
	SET(zlibLib libzlibstatic.a)
	SET(extra_libraries stdc++fs)
endif()

#ZLib library
#ExternalProject_Add("libz"
#        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/zlib"
#        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/libz -DEXTRA_LIBS=${extra_libraries}
#        )

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zlib)
set(LIBZ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/libz/include")

#Zlib
#ExternalProject_Add("LibZip"
#		SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/libzip"
#		CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/libzip -DEXTRA_LIBS=${extra_libraries}
#		)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/libzip)

set(libzip_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/libzip/include/")
set(libzip_HEADER_FILES ${CMAKE_SOURCE_DIR}/lib/libzip/lib/libzip/include)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(libzip_library ${CMAKE_SOURCE_DIR}/lib/libzip/lib/libzip.dll.a)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(libzip_library ${CMAKE_SOURCE_DIR}/lib/libzip/lib/libzip.so)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


message(libzip_HEADER_FILES = ${libzip_HEADER_FILES})
message(CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${libzip_HEADER_FILES})
include_directories(${libzip_INCLUDE_DIRS})

#GLFW
#ExternalProject_Add("GLFW"
#        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/glfw"
#        CMAKE_ARGS
#			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#			-DBUILD_SHARED_LIBS=ON
#			-DGLFW_BUILD_EXAMPLES=OFF
#			-DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/lib/glfw
#			-DEXTRA_LIBS=${extra_libraries}
#			-DVULKAN_LIBRARY=NO
#			-DGLFW_VULKAN_STATIC=OFF
#			-DGLFW_BUILD_DOCS=OFF
#			-DGLFW_DOCUMENT_INTERNALS=OFF
#			-DGLFW_VULKAN_STATIC=OFF
#        )
#ExternalProject_Get_Property(GLFW install_dir)
#set(glfw_install_dir ${CMAKE_SOURCE_DIR}/lib/glfw)
#include_directories(${glfw_install_dir}/include)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glfw GLFW)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(GLFW_STATIC_LIBRARIES ${glfw_install_dir}/lib/libglfw3dll.a)
	else()
		set(GLFW_STATIC_LIBRARIES ${glfw_install_dir}/lib/libglfw.so.3)
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif()
#bz2
add_subdirectory(3rdparty/bzip)

#CPR
set(ENV{BUILD_CPR_TESTS} OFF)
set(ENV{CMAKE_USE_OPENSSL} OFF)
add_subdirectory(cpr)
include_directories(${CPR_INCLUDE_DIRS})

#PolyM
add_subdirectory(3rdparty/PolyM)
include_directories(3rdparty/PolyM)

#StormLib
#add_subdirectory(3rdparty/stormlib)

#CascLib
#set(CASC_BUILD_STATIC_LIB ON)
set(CASC_BUILD_SHARED_LIB ON)
set(CMAKE_SYSTEM_NAME_TEMPTEMP ${CMAKE_SYSTEM_NAME})
set(CMAKE_SYSTEM_NAME "Linux")
add_subdirectory(3rdparty/casclib)
set(CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_TEMPTEMP}")

#nuklear_cross
include_directories(3rdparty/nuklear_cross)

add_subdirectory(wowViewerLib)

set(SOURCE_FILES
		src/persistance/httpFile/httpFile.cpp
		src/persistance/httpFile/httpFile.h
		src/persistance/RequestProcessor.cpp
		src/persistance/RequestProcessor.h
		src/persistance/ZipRequestProcessor.cpp
		src/persistance/ZipRequestProcessor.h
		src/persistance/HttpZipRequestProcessor.cpp
		src/persistance/HttpZipRequestProcessor.h
		src/persistance/CascRequestProcessor.cpp
		src/persistance/CascRequestProcessor.h
		src/persistance/HttpRequestProcessor.cpp
		src/persistance/HttpRequestProcessor.h)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set(SOURCE_FILES ${SOURCE_FILES}
		src/main.cpp)
else()
	set(SOURCE_FILES ${SOURCE_FILES}
			src/jniLib.cpp)
endif()


#########################################################
# FIND OPENGL
#########################################################
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	find_package(OpenGL REQUIRED)
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})
	message(OpenGL_LIBRARY_DIRS = ${OpenGL_LIBRARY_DIRS})
	add_definitions(${OpenGL_DEFINITIONS})
	if(NOT OPENGL_FOUND)
		message(ERROR " OPENGL not found!")
	endif(NOT OPENGL_FOUND)
else()
#	if (${ANDROID_PLATFORM_LEVEL} LESS 12)
#		message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
#                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
#		return()
#	elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
		add_definitions("-DDYNAMIC_ES3")
		set(GL3STUB_SRC gl3stub.c)
		set(OPENGL_LIB GLESv2)
#	else ()
#		set(OPENGL_LIB GLESv3)
#	endif (${ANDROID_PLATFORM_LEVEL} LESS 12)
endif()

#Build the executiable
include_directories(${glew_include})


if (CMAKE_NDK_BUILD MATCHES 1)
	add_library(WebWoWViewerCpp SHARED ${SOURCE_FILES})
	target_compile_definitions(WebWoWViewerCpp PRIVATE WITH_GLESv2=1)
	target_compile_definitions(WoWViewerLib PRIVATE WITH_GLESv2=1)
else()
	add_executable(WebWoWViewerCpp ${SOURCE_FILES})

endif()

add_dependencies(WebWoWViewerCpp zlib)
add_dependencies(WebWoWViewerCpp zip)

add_dependencies(WebWoWViewerCpp WoWViewerLib)
#add_dependencies(WebWoWViewerCpp storm)
add_dependencies(WebWoWViewerCpp casc)

#TODO: platform dependant!!
#target_link_libraries(WebWoWViewerCpp ${install_dir}/lib/libWoWViewerLib)
#target_link_libraries(WebWoWViewerCpp ${glfw_install_dir}/lib/libglfw3dll)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set_property(TARGET WebWoWViewerCpp PROPERTY CXX_STANDARD 17)
endif()
message(INFO ${OPENGL_LIBRARIES})
#glu32.a opengl32.a z
target_link_libraries(WebWoWViewerCpp ${GLFW_STATIC_LIBRARIES} ${CURL_LIBRARIES} ${libzip_library})
target_link_libraries(WebWoWViewerCpp ${OPENGL_LIBRARIES})
target_link_libraries(WebWoWViewerCpp zip)
target_link_libraries(WebWoWViewerCpp WoWViewerLib)
if (NOT CMAKE_NDK_BUILD MATCHES 1)
target_link_libraries(WebWoWViewerCpp GLFW)
target_link_libraries(WebWoWViewerCpp -lpthread)
else()
	target_link_libraries(WebWoWViewerCpp EGL)
	target_link_libraries(WebWoWViewerCpp GLESv2)
endif()
target_link_libraries(WebWoWViewerCpp polym)
target_link_libraries(WebWoWViewerCpp cpr)
#target_link_libraries(WebWoWViewerCpp storm)
target_link_libraries(WebWoWViewerCpp casc)
target_link_libraries(WebWoWViewerCpp ${glew_lib})
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
target_link_libraries(WebWoWViewerCpp ${CMAKE_SOURCE_DIR}/wowViewerLib/3rdparty/glew_win/lib/Release/x64/glew32.lib)
target_link_libraries(WebWoWViewerCpp opengl32)
target_link_libraries(WebWoWViewerCpp -lm)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

