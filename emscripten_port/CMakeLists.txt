cmake_minimum_required(VERSION 3.5)

project(WoWWebViewerJs)

include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE ON)

message( EMSCRIPTEN_ROOT = emc${EMSCRIPTEN_ROOT})

#set(CMAKE_C_COMPILER /home/deamon/Downloads/emsdk-master/upstream/emscripten/emcc)
#set(CMAKE_CXX_COMPILER /home/deamon/Downloads/emsdk-master/upstream/emscripten/em++)


check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    # Have -std=c++17, use it
    message(INFO "HAVE_FLAG_STD_CXX17 is supported")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17" )
else()
    check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)
    if(HAVE_FLAG_STD_CXX1Z)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++1z" )
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++1z" )
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z")
    else()
        message(ERROR "No supported flags")
    endif()
endif()

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-exceptions" )
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-exceptions" )
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-exceptions")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions")



set(EMSCRIPTEN 1)

add_subdirectory(../wowViewerLib ${CMAKE_CURRENT_BINARY_DIR}/wowViewerLib)

set(SOURCE_FILES src/main.cpp
        src/RequestProcessor.cpp
        src/HttpRequestProcessor.cpp
        )

add_executable(project ${SOURCE_FILES})

add_dependencies(project WoWViewerLib)
target_link_libraries(project WoWViewerLib)
target_link_options(project PUBLIC "SHELL: --pre-js ../prejs.js --js-library ../jsLib.js")
target_link_options(project PUBLIC "SHELL: -s USE_GLFW=3 --preload-file glsl -s ALLOW_MEMORY_GROWTH=1")
target_link_options(project PUBLIC "SHELL: -s TOTAL_MEMORY=40MB")
target_link_options(project PUBLIC "SHELL: -s FETCH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1")
target_link_options(project PUBLIC "SHELL: -fno-rtti --llvm-lto 1")

set_target_properties(project
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../"
        )