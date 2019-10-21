cmake_minimum_required(VERSION 3.5)

project(WoWWebViewerJs)

include(CheckCXXCompilerFlag)

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


add_subdirectory(../wowViewerLib ${CMAKE_CURRENT_BINARY_DIR}/A)

set(SOURCE_FILES src/main.cpp
        src/RequestProcessor.cpp
        src/HttpRequestProcessor.cpp
        )

add_library(AWoWWebViewerJs ${SOURCE_FILES})

add_dependencies(AWoWWebViewerJs WoWViewerLib)
target_link_libraries(AWoWWebViewerJs WoWViewerLib)





INSTALL(TARGETS AWoWWebViewerJs DESTINATION BIN)