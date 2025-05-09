cmake_minimum_required(VERSION 3.10)

project(WoWStudio)
include(ExternalProject)

enable_language( C CXX ASM )

set(CMAKE_VERBOSE_MAKEFILE ON)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
cmake_policy(SET CMP0079 NEW)

set(INCLUDE_PROTOBUF_LIB 0)

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

	add_definitions(-DWIN32_LEAN_AND_MEAN)
 endif()
message(CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	link_libraries(stdc++fs)
endif()


if(WIN32)
	link_libraries(wsock32 ws2_32 -ldbghelp winmm)
endif()

# DB Importer
set(DBIMPORTER_LINK_SQLITE OFF)
set(DBIMPORTER_SQLITE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/SQLiteCpp/include")
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/DBImporter EXCLUDE_FROM_ALL)

#ZLib library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zlib EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdparty/zlib)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

if (TRUE)
	set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zlib ${CMAKE_BINARY_DIR}/3rdparty/zlib)
    message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
	message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
endif()


add_definitions(-DSQLITE_ENABLE_FTS5)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/SQLiteCpp/sqlite3)

if (NOT CMAKE_NDK_BUILD MATCHES 1)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glfw EXCLUDE_FROM_ALL)
endif()

#c++ 17 FS implementation
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/filesystem_impl EXCLUDE_FROM_ALL)

include(FetchContent)

#cpr
check_include_file("limits.h" HAVE_LIMITS_H)
add_compile_definitions(HAVE_LIMITS_H)
SET(CMAKE_USE_LIBSSH2 0)
SET(CPR_BUILD_TESTS 0)
SET(CURL_USE_LIBPSL 0)
SET(BUILD_LIBCURL_DOCS 0)
SET(ENABLE_CURL_MANUAL 0)
SET(CURL_ZLIB OFF CACHE STRING "" FORCE) # disable this lib to download the zlib as external project
FetchContent_Declare(cpr
		GIT_REPOSITORY https://github.com/libcpr/cpr.git
		GIT_TAG 2b2b16989708d81d4fe062240e085a2f5a61f74a # the commit hash for 1.9.9
		EXCLUDE_FROM_ALL
)

#FetchContent_MakeAvailable(cpr)
FetchContent_GetProperties(cpr)
message("cpr_POPULATED = ${cpr_POPULATED}")
if(NOT cpr_POPULATED)
#	 Fetch the content using previously declared details
	FetchContent_MakeAvailable(cpr)
	message("cpr_BINARY_DIR = ${cpr_BINARY_DIR}")
	message("cpr_SOURCE_DIR = ${cpr_SOURCE_DIR}")
#	 Bring the populated content into the build
	set(BUILD_SHARED_LIBS ON CACHE BOOL "")
#	add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
endif()

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

set(CASC_BUILD_STATIC_LIB 1)

set(CASC_BUILD_STATIC_LIB ON CACHE BOOL "Set static lib from main project")
set(CASC_BUILD_SHARED_LIB true CACHE BOOL "Turn off shared lib ")
add_subdirectory(3rdparty/casclib EXCLUDE_FROM_ALL)

# Protobuf
if (INCLUDE_PROTOBUF_LIB)
	set(protobuf_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
	set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "Build tests")
	set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "Build conformance tests")
	add_subdirectory(3rdparty/protobuf-cpp/protobuf-3.21.5 EXCLUDE_FROM_ALL)
endif ()


if (CMAKE_NDK_BUILD MATCHES 1)
	target_compile_definitions(casc PUBLIC -DCMAKE_SYSTEM_NAME=Linux)
	set(CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_TEMPTEMP}")
endif()

add_subdirectory(wowViewerLib)
if (LINK_EGL)
	add_definitions(-DLINK_EGL)
	add_definitions(-DWITH_GLESv2)
endif()

include_directories(${GLEW_INCLUDE_DIRS})


set(SOURCE_FILES
		src/persistance/httpFile/httpFile.cpp
		src/persistance/httpFile/httpFile.h
		src/persistance/ZipRequestProcessor.cpp
		src/persistance/ZipRequestProcessor.h
#		src/persistance/HttpZipRequestProcessor.cpp
#		src/persistance/HttpZipRequestProcessor.h
		src/persistance/CascRequestProcessor.cpp
		src/persistance/CascRequestProcessor.h
		src/persistance/HttpRequestProcessor.cpp
		src/persistance/HttpRequestProcessor.h

		src/ui/imguiLib/imgui.cpp
		src/ui/imguiLib/imgui_demo.cpp
		src/ui/imguiLib/imgui_draw.cpp
		src/ui/imguiLib/imgui_widgets.cpp
		src/ui/imguiLib/imgui_tables.cpp
		src/ui/imguiLib/imguiImpl/imgui_impl_glfw.cpp
		src/ui/FrontendUI.cpp src/ui/FrontendUI.h
		src/database/CSqliteDB.cpp src/database/CSqliteDB.h
		src/minimapGenerator/minimapGenerator.cpp
		src/screenshots/screenshotMaker.h
		src/screenshots/screenshotMaker.cpp
		src/screenshots/lodepng/lodepng.cpp
		src/screenshots/lodepng/lodepng.h

		src/exporters/gltfExporter/GLTFExporter.cpp
		src/exporters/gltfExporter/GLTFExporter.h

        src/ui/imguiLib/groupPanel/groupPanel.cpp
		src/ui/imguiLib/groupPanel/groupPanel.h

		src/ui/imguiLib/disablableButton/disablableButton.cpp
		src/ui/imguiLib/disablableButton/disablableButton.h
		src/minimapGenerator/storage/CMinimapDataDB.cpp
		src/minimapGenerator/storage/CMinimapDataDB.h
		src/minimapGenerator/entities.h
		src/ui/imguiLib/compactColorPicker/compactColorPicker.cpp
		src/ui/imguiLib/compactColorPicker/compactColorPicker.h
		src/ui/imguiLib/imageButton2/imageButton2.cpp
		src/ui/imguiLib/imageButton2/imageButton2.h
		src/exporters/dataExporter/DataExporterClass.cpp
		src/exporters/dataExporter/DataExporterClass.h
		src/database/CEmptySqliteDB.cpp
		src/database/CEmptySqliteDB.h
		src/ui/imguiLib/stateSaver/stateSaver.h
		src/ui/imguiLib/stateSaver/stateSaver.cpp
		src/ui/childWindow/databaseUpdateWorkflow/DatabaseUpdateWorkflow.cpp
		src/ui/childWindow/databaseUpdateWorkflow/DatabaseUpdateWorkflow.h
		src/database/buildInfoParser/buildDefinition.h
		src/database/buildInfoParser/buildInfoParser.cpp
		src/database/buildInfoParser/buildInfoParser.h
		src/ui/childWindow/minimapGeneratonWindow/MinimapGenerationWindow.cpp
		src/ui/childWindow/minimapGeneratonWindow/MinimapGenerationWindow.h
		src/ui/renderer/uiScene/FrontendUIRenderer.cpp
		src/ui/renderer/uiScene/FrontendUIRenderer.h
		src/ui/renderer/uiScene/FrontendUIRendererFactory.cpp
		src/ui/renderer/uiScene/FrontendUIRendererFactory.h
		src/ui/renderer/uiScene/ImGUIPlan.cpp
		src/ui/renderer/uiScene/ImGUIPlan.h
        src/ui/renderer/uiScene/IFrontendUIBufferCreate.h
		src/ui/renderer/uiScene/materials/UIMaterial.h
        src/ui/childWindow/mapSelectionWindow/MapSelectDialog.cpp
		src/ui/childWindow/mapSelectionWindow/MapSelectDialog.h
		src/ui/imguiLib/wheelCapture/wheelCapture.cpp
		src/ui/imguiLib/wheelCapture/wheelCapture.h src/ui/childWindow/BLPViewer.cpp src/ui/childWindow/BLPViewer.h src/ui/childWindow/keysUpdateWorkflow/KeysUpdateWorkflow.cpp src/ui/childWindow/keysUpdateWorkflow/KeysUpdateWorkflow.h
		src/ui/childWindow/textureRenderer/DebugRendererWindow.cpp
		src/ui/childWindow/textureRenderer/DebugRendererWindow.h
        src/ui/childWindow/fileListWindow/FileListWindow.cpp
        src/ui/childWindow/fileListWindow/FileListWindow.h
		src/ui/childWindow/sceneWindow/SceneWindow.cpp
		src/ui/childWindow/sceneWindow/SceneWindow.h
		src/ui/childWindow/m2Window/M2Window.cpp
		src/ui/childWindow/m2Window/M2Window.h
		src/ui/imguiLib/hasFocus/imguiHasFocus.cpp
		src/ui/imguiLib/hasFocus/imguiHasFocus.h
		src/ui/imguiLib/floatCenterSlider/floatCenterSlider.h
		src/ui/imguiLib/floatCenterSlider/floatCenterSlider.cpp
		src/ui/imguiLib/compactFloatEditor/compactFloatEditor.cpp
		src/ui/imguiLib/compactFloatEditor/compactFloatEditor.h
)

set(PROTOBUF_FILES_ "")
if (INCLUDE_PROTOBUF_LIB)
	set(PROTOBUF_FILES_
			src/database/product_db_parser/productDbParser.cpp
			src/database/product_db_parser/productDbParser.h
			src/database/product_db_parser/protobuf_out/ProtoDatabase.pb.cc
			src/database/product_db_parser/protobuf_out/ProtoDatabase.pb.h)
endif()

set(SOURCE_FILES_VULKAN
		src/ui/renderer/uiScene/vulkan/FrontendUIRenderForwardVLK.cpp
		src/ui/renderer/uiScene/vulkan/FrontendUIRenderForwardVLK.h)

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

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/SQLiteCpp EXCLUDE_FROM_ALL)
if (NOT Vulkan_FOUND)
	set(SOURCE_FILES_VULKAN "")
endif()

add_library(WoWFrontendLib STATIC ${SOURCE_FILES} ${SOURCE_FILES_VULKAN} ${PROTOBUF_FILES_})
add_executable(WowStudio src/main.cpp)
target_link_libraries(WowStudio WoWFrontendLib)

if (Vulkan_FOUND)
	add_definitions(-DLINK_VULKAN)
endif()


message(ccp WoWViewerLib_INCLUDE_DIRS = ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(WoWFrontendLib PUBLIC ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(WoWFrontendLib PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)
target_include_directories(WoWFrontendLib PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)

#target_compile_options(WoWFrontendLib PRIVATE "-static")
target_compile_definitions(WoWFrontendLib PUBLIC -DIMGUI_USER_CONFIG=<imguiCustomConfig.h>)
target_include_directories(WoWFrontendLib PUBLIC src/ui/imguiLib)
target_include_directories(WoWFrontendLib PUBLIC 3rdparty/imgui-notify/example/src)

add_dependencies(WoWFrontendLib WoWViewerLib)
#add_dependencies(WoWFrontendLib storm)
add_dependencies(WoWFrontendLib casc_static)

#TODO: platform dependant!!
add_dependencies(WoWFrontendLib glfw)
if (NOT MSVC)
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
endif()

if (MSVC)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
	message("MSVC Is on")
	if (_cpp_17_flag_supported)
		message("/std:c++17 is supported")
		#target_compile_options(WoWFrontendLib "/std:c++17")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /std:c++17")
		#target_compile_options(WoWFrontendLib /std:c++17)
	endif()
endif()

target_link_libraries(WoWFrontendLib ${GLEW_LIBRARIES})
target_link_libraries(WoWFrontendLib ${OPENGL_LIBRARIES})
target_link_libraries(WoWFrontendLib ${OPENGL_LIB})
#target_link_libraries(WoWFrontendLib zlibstatic)
if(INCLUDE_PROTOBUF_LIB)
	target_link_libraries(WoWFrontendLib libprotobuf-lite)
endif ()


target_link_libraries(WoWFrontendLib SQLiteCpp sqlite3)
target_link_libraries(DBImporterLib SQLiteCpp)
if (NOT MSVC)
    find_library(DL_EXISTS dl)
    message("DL_EXISTS = ${DL_EXISTS}")
    if (NOT DL_EXISTS STREQUAL "DL_EXISTS-NOTFOUND")
        message("Linking against DL")
        target_link_libraries(WoWFrontendLib dl)
        target_compile_options(WoWFrontendLib PRIVATE "-fstack-protector")
    else()
        find_library(SSP_EXISTS ssp)
        message("SSP_EXISTS = ${SSP_EXISTS}")
        if (NOT SSP_EXISTS STREQUAL "SSP_EXISTS-NOTFOUND")
            message("Linking against SSP")
            target_link_libraries(WoWFrontendLib ssp)
            target_compile_options(WoWFrontendLib PRIVATE "-fstack-protector")
        endif()

        message( "Neither DL nor SSP was found" )
    endif()
endif(NOT MSVC)

target_link_libraries(WoWFrontendLib WoWViewerLib)

target_link_libraries(WoWFrontendLib Threads::Threads)
if (NOT CMAKE_NDK_BUILD MATCHES 1)
	target_link_libraries(WoWFrontendLib glfw)
else()
	target_link_libraries(WoWFrontendLib EGL)
	target_link_libraries(WoWFrontendLib GLESv2)
	target_link_libraries(WoWFrontendLib android)
	target_link_libraries(WoWFrontendLib log)
endif()

#target_link_libraries(WoWFrontendLib storm)
target_link_libraries(WoWFrontendLib casc_static)
target_link_libraries(WoWFrontendLib DBImporterLib)
target_link_libraries(WoWFrontendLib cpr::cpr)

target_link_libraries(WoWFrontendLib ${glew_lib})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(WoWFrontendLib "./wowViewerLib/src/opengl")
	target_link_libraries(WoWFrontendLib opengl32)
	target_link_libraries(WoWFrontendLib -lwininet)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
	MARK_AS_ADVANCED(OpenGL_LIBRARY)

	target_link_libraries(WoWFrontendLib ${OpenGL_LIBRARY})
else()
	target_link_libraries(WoWFrontendLib GL)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#target_link_options(WoWFrontendLib PUBLIC "SHELL: -llibg")

install(TARGETS WowStudio
		EXPORT ${PROJECT_NAME}Targets
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT libraries)

INSTALL(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT Libraries)

if( MINGW )
	message( STATUS "Installing system-libraries: MinGW DLLs." )
	get_filename_component( Mingw_Path ${CMAKE_CXX_COMPILER} PATH )
	set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
			${Mingw_Path}/libgcc_s_seh-1.dll
			${Mingw_Path}/libstdc++-6.dll
			${Mingw_Path}/libwinpthread-1.dll
			${Mingw_Path}/zlib1.dll
	)
endif( MINGW )
include( InstallRequiredSystemLibraries )

# Actually install it when make install is called.
# Note, this works with CPack
if( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )
	install( PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT System )
endif( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )

add_subdirectory(test EXCLUDE_FROM_ALL)
target_link_libraries(wow_viewer_tests_run WoWFrontendLib)
set_target_properties(wow_viewer_tests_run PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(wow_viewer_tests_run
    PRIVATE
    "$<TARGET_PROPERTY:WoWFrontendLib,INTERFACE_INCLUDE_DIRECTORIES>"
)




include(CPack)

add_executable(TEST_LIGHT_BLENDING
    testApps/TestLightBlending.cpp
    wowViewerLib/src/engine/algorithms/mathHelper.cpp
    wowViewerLib/src/engine/algorithms/grahamScan.cpp
#    wowViewerLib/src/engine/custom_allocators/FrameBasedStackAllocator.cpp
#    wowViewerLib/3rdparty/OffsetAllocator/offsetAllocator.cpp
	src/database/CSqliteDB.cpp src/database/CSqliteDB.h
)
target_include_directories(TEST_LIGHT_BLENDING PUBLIC ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(TEST_LIGHT_BLENDING PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)
target_link_libraries(TEST_LIGHT_BLENDING TBB::tbb )
target_link_libraries(TEST_LIGHT_BLENDING TBB::tbbmalloc )
target_link_libraries(TEST_LIGHT_BLENDING SQLiteCpp sqlite3)
target_compile_definitions(TEST_LIGHT_BLENDING PUBLIC -DTEST_APP_ALLOCATION)