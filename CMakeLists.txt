cmake_minimum_required(VERSION 3.6)

project(AWebWoWViewerCpp)
include(ExternalProject)

enable_language( C CXX ASM )

set(CMAKE_VERBOSE_MAKEFILE ON)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


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

if (WIN32 AND MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DWIN64")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN64")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DWIN64")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN64")
    endif()
 endif()
message(CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE})
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC ")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC  ")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	message("This is clang")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -static -femulated-tls ")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static -femulated-tls -std=c++1z")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -static -femulated-tls ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -femulated-tls -std=c++1z")



	#set(_GLIBCXX_USE_C99_CHECK 0)
	#set(_GLIBCXX_USE_C99_DYNAMIC 0)
	#set(_GLIBCXX_USE_C99_LONG_LONG_CHECK 0)
	#set(_GLIBCXX_USE_C99_LONG_LONG_DYNAMIC 0)
#	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++")
#	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++")
#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	link_libraries(stdc++fs)
#	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread")
#	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread ")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()


if(WIN32)
	link_libraries(wsock32 ws2_32)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	link_libraries(libws2_32.a)
endif()

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -pthread")

#ZLib library
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdparty/zlib)
set(BUILD_SHARED_LIBS OFF)
if (MSVC)
	set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zlib ${CMAKE_BINARY_DIR}/3rdparty/zlib)
#	message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
	if (CMAKE_BUILD_TYPE EQUAL "Debug")
		set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/3rdparty/zlib/zlibd.lib")
	else()
		set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/3rdparty/zlib/zlib.lib")
	endif()
	set(ZLIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/3rdparty/zlib)
endif()


#libpng
if (MSVC)
	set(PNG_BUILD_ZLIB ON)
else()
	set(PNG_BUILD_ZLIB OFF)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/libpng)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/libpng)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/SQLiteCpp/sqlite3)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libpng)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set(BUILD_SHARED_LIBS OFF)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glfw)
endif()

#bz2renderFlag
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(3rdparty/bzip)




#PolyM
#add_subdirectory(3rdparty/PolyM) 
#include_directories(3rdparty/PolyM)

#StormLib
#add_subdirectory(3rdparty/stormlib)

#CascLib

add_definitions(-DCASCLIB_NO_AUTO_LINK_LIBRARY)

if (CMAKE_NDK_BUILD MATCHES 1)
	#set(CASC_BUILD_SHARED_LIB ON)
	set(CMAKE_SYSTEM_NAME_TEMPTEMP ${CMAKE_SYSTEM_NAME})
	set(CMAKE_SYSTEM_NAME "Linux")
endif() 
if (WIN32)
	set(WITH_LIBTOMCRYPT true)
endif()
option(CASC_BUILD_STATIC_LIB "" ON)
option(CASC_BUILD_SHARED_LIB "" OFF)
add_subdirectory(3rdparty/casclib)


if (CMAKE_NDK_BUILD MATCHES 1)
	target_compile_definitions(casc PUBLIC -DCMAKE_SYSTEM_NAME=Linux)
	set(CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_TEMPTEMP}")
endif()

add_subdirectory(wowViewerLib)


include_directories(${GLEW_INCLUDE_DIRS})


set(SOURCE_FILES
#		src/persistance/httpFile/httpFile.cpp
#		src/persistance/httpFile/httpFile.h
		src/persistance/RequestProcessor.cpp
		src/persistance/RequestProcessor.h
		src/persistance/ZipRequestProcessor.cpp
		src/persistance/ZipRequestProcessor.h
#		src/persistance/HttpZipRequestProcessor.cpp
#		src/persistance/HttpZipRequestProcessor.h
		src/persistance/CascRequestProcessor.cpp
		src/persistance/CascRequestProcessor.h
#		src/persistance/HttpRequestProcessor.cpp
#		src/persistance/HttpRequestProcessor.h

		src/ui/imguiLib/imgui.cpp
		src/ui/imguiLib/imgui_demo.cpp
		src/ui/imguiLib/imgui_draw.cpp
		src/ui/imguiLib/imgui_widgets.cpp
		src/ui/imguiLib/imguiImpl/imgui_impl_glfw.cpp
		src/ui/FrontendUI.cpp src/ui/FrontendUI.h
		src/database/CSqliteDB.cpp src/database/CSqliteDB.h
)


if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set(SOURCE_FILES ${SOURCE_FILES}
		src/main.cpp)
else()
	set(SOURCE_FILES ${SOURCE_FILES}
			src/jniLib.cpp src/database/dbStructs.h src/ui/imguiLib/imguiCustomConfig.h)
endif()


#########################################################
# FIND OPENGL
#########################################################
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL REQUIRED)
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})
	if(NOT OPENGL_FOUND)
		message(ERROR " OPENGL not found!")
	endif(NOT OPENGL_FOUND)
else()
#	if (${ANDROID_PLATFORM_LEVEL} LESS 12)
#		message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 /
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

#if(CMAKE_USE_PTHREADS_INIT)
#    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
#endif()


add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/SQLiteCpp)

#add_definitions(-DLINK_VULKAN)
add_executable(AWebWoWViewerCpp ${SOURCE_FILES})
message(ccp WoWViewerLib_INCLUDE_DIRS = ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(AWebWoWViewerCpp PUBLIC ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(AWebWoWViewerCpp PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)
target_include_directories(AWebWoWViewerCpp PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)

#target_compile_options(AWebWoWViewerCpp PRIVATE "-static")
target_compile_definitions(AWebWoWViewerCpp PUBLIC -DIMGUI_USER_CONFIG=<imguiCustomConfig.h>)
target_include_directories(AWebWoWViewerCpp PUBLIC src/ui/imguiLib)

add_dependencies(AWebWoWViewerCpp WoWViewerLib)
#add_dependencies(AWebWoWViewerCpp storm)
add_dependencies(AWebWoWViewerCpp casc_static)

#TODO: platform dependant!!
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	add_dependencies(AWebWoWViewerCpp glfw)
	set_property(TARGET AWebWoWViewerCpp PROPERTY CXX_STANDARD 17)
endif()

link_libraries(AWebWoWViewerCpp ${GLEW_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIB})
target_link_libraries(AWebWoWViewerCpp zlibstatic)

target_link_libraries(AWebWoWViewerCpp SQLiteCpp sqlite3)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	target_link_libraries(AWebWoWViewerCpp ssp)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	target_link_libraries(AWebWoWViewerCpp ssp)
endif()
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
#target_link_libraries(AWebWoWViewerCpp storm)
target_link_libraries(AWebWoWViewerCpp casc_static)

target_link_libraries(AWebWoWViewerCpp png_static)


target_link_libraries(AWebWoWViewerCpp ${glew_lib})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(AWebWoWViewerCpp "./wowViewerLib/src/opengl")
	target_link_libraries(AWebWoWViewerCpp opengl32)
	target_link_libraries(AWebWoWViewerCpp -lm)
	target_link_libraries(AWebWoWViewerCpp -lwininet)
else()
	target_link_libraries(AWebWoWViewerCpp GL)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#Link OPENSSL only for MSVC


INSTALL(TARGETS AWebWoWViewerCpp DESTINATION BIN)