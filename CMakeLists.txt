cmake_minimum_required(VERSION 3.6)

project(AWebWoWViewerCpp)
include(ExternalProject)

set(USE_NEON 0)
if (CMAKE_NDK_BUILD MATCHES 1)
	include_directories(BEFORE SYSTEM "${NDK_PATH}/include/c++/4.9.x/" "${NDK_PATH}/sysroot/usr/include/")
	message(ERROR " CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
	message(ERROR " ANDROID_ABI = ${ANDROID_ABI}")
	#TODO: check ANDROID_ARM_NEON too
	if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
		include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)  
		set(USE_NEON 1)
	elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
		include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
		set(USE_NEON 1)
	elseif(${ANDROID_ABI} STREQUAL "x86_64")
		include_directories(${ANDROID_SYSROOT}/usr/include/x86_64-linux-android)
		set(USE_NEON 0)
	else() 
		include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
		set(USE_NEON 0)
	endif()
endif() 

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -pthread")

if (WIN32) 
	SET(zlibLib zlibstaticd) 
	SET(extra_libraries "") 
else() 
	SET(zlibLib libzlibstatic.a)
	SET(extra_libraries stdc++fs)
endif()

#ZLib library
set(BUILD_SHARED_LIBS 1)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zlib)
set(BUILD_SHARED_LIBS )
#Zlib
#ExternalProject_Add("LibZip"
#		SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/libzip"
#		CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR} -DEXTRA_LIBS=${extra_libraries}
#		)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/libzip)
include_directories("lib/libzip/include")

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glfw)
endif()

#bz2
add_subdirectory(3rdparty/bzip)
 
#CPR
set(ENV{USE_SYSTEM_CURL} ON)
set(ENV{BUILD_CPR_TESTS} OFF)
set(ENV{CMAKE_USE_OPENSSL} ON)
set(ENV{CURL_DISABLE_LDAP OFF)
set(CMAKE_USE_OPENSSL ON)
add_subdirectory(cpr)
include_directories(${CPR_INCLUDE_DIRS})

#PolyM
#add_subdirectory(3rdparty/PolyM) 
#include_directories(3rdparty/PolyM)

#StormLib
#add_subdirectory(3rdparty/stormlib)

#CascLib
set(CASC_BUILD_STATIC_LIB 1)
set(CASC_BUILD_SHARED_LIB OFF)
if (CMAKE_NDK_BUILD MATCHES 1)
	#set(CASC_BUILD_SHARED_LIB ON)
	set(CMAKE_SYSTEM_NAME_TEMPTEMP ${CMAKE_SYSTEM_NAME})
	set(CMAKE_SYSTEM_NAME "Linux")
endif() 
if (WIN32)
	set(WITH_LIBTOMCRYPT true)
endif()
add_subdirectory(3rdparty/casclib)
set(CASC_BUILD_STATIC_LIB 1)
if (CMAKE_NDK_BUILD MATCHES 1)
	target_compile_definitions(casc PUBLIC -DCMAKE_SYSTEM_NAME=Linux)
	set(CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_TEMPTEMP}")
endif()

#nuklear_cross
include_directories(3rdparty/nuklear_cross)

add_subdirectory(wowViewerLib)
include_directories(${GLEW_INCLUDE_DIRS})

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
		set(OPENGL_LIB GLESv3)
#	else ()
#		set(OPENGL_LIB GLESv3)
#	endif (${ANDROID_PLATFORM_LEVEL} LESS 12)
endif()

#Build the executiable
include_directories(${glew_include})
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package( Threads REQUIRED )

if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif()

if (CMAKE_NDK_BUILD MATCHES 1)
	add_library(AWebWoWViewerCpp SHARED ${SOURCE_FILES})
	target_compile_definitions(AWebWoWViewerCpp PRIVATE WITH_GLESv2=1)
	target_compile_definitions(WoWViewerLib PRIVATE WITH_GLESv2=1)
	if (USE_NEON MATCHES 1)
		target_compile_definitions(WoWViewerLib PRIVATE VECTORIAL_NEON=1)
		target_compile_definitions(WoWViewerLib PRIVATE VECTORIAL_FORCED=1)
		target_compile_definitions(WoWViewerLib PRIVATE MATHFU_COMPILE_WITH_SIMD=1)
	endif ()
else()
	add_executable(AWebWoWViewerCpp ${SOURCE_FILES})

endif()

#add_dependencies(AWebWoWViewerCpp LibZip)
add_dependencies(AWebWoWViewerCpp zip)
add_dependencies(AWebWoWViewerCpp zlib)
add_dependencies(AWebWoWViewerCpp WoWViewerLib)
#add_dependencies(AWebWoWViewerCpp storm)
add_dependencies(AWebWoWViewerCpp casc_static)

#TODO: platform dependant!!
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	add_dependencies(AWebWoWViewerCpp glfw)
	set_property(TARGET AWebWoWViewerCpp PROPERTY CXX_STANDARD 17)
endif()

link_libraries(AWebWoWViewerCpp ${GLEW_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${CURL_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIB})
#target_link_libraries(AWebWoWViewerCpp zip)
target_link_libraries(AWebWoWViewerCpp zlib)
target_link_libraries(AWebWoWViewerCpp WoWViewerLib)

  target_link_libraries(AWebWoWViewerCpp Threads::Threads)
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	target_link_libraries(AWebWoWViewerCpp glfw)
else()
	target_link_libraries(AWebWoWViewerCpp EGL)
	target_link_libraries(AWebWoWViewerCpp GLESv2)
	target_link_libraries(AWebWoWViewerCpp android)
	target_link_libraries(AWebWoWViewerCpp log)
endif()
#target_link_libraries(AWebWoWViewerCpp polym)
target_link_libraries(AWebWoWViewerCpp cpr)
#target_link_libraries(AWebWoWViewerCpp storm)
target_link_libraries(AWebWoWViewerCpp casc_static)
target_link_libraries(AWebWoWViewerCpp ${glew_lib})
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(AWebWoWViewerCpp "./wowViewerLib/src/opengl")
	target_link_libraries(AWebWoWViewerCpp opengl32)
	target_link_libraries(AWebWoWViewerCpp -lm)
	target_link_libraries(AWebWoWViewerCpp -lwininet)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

INSTALL(TARGETS AWebWoWViewerCpp DESTINATION BIN)