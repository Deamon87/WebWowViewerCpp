if (CMAKE_NDK_BUILD MATCHES 1)
    set(GLSL_TARGET_FOLDER ${ANDROID_ASSETSFOLDER})
else()
    set(GLSL_TARGET_FOLDER ${CMAKE_BINARY_DIR})
endif()

message(INFO "GLSL_TARGET_FOLDER = ${GLSL_TARGET_FOLDER}")

add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/spirv-cross)
set(BUILD_EXTERNAL OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/glsllang)
if(EMSCRIPTEN)
    target_link_options(glslangValidator PUBLIC -s NODERAWFS=1)
endif()
set(BUILD_EXTERNAL OFF)

set(SPIRV_REF_SOURCES
        src/spirv/spirv_refl_main.cpp
        src/spirv/webGLSLCompiler.cpp
        src/spirv/webGLSLCompiler.h
        src/spirv/dumpShaderFields.h src/spirv/dumpShaderMetaData.h src/spirv/dumpGLSLShader.h)
set(SPIRV_META_FILE ${PROJECT_SOURCE_DIR}/src/engine/shader/ShaderDefinitions.h)

add_custom_target(SPIRV_EXTRACT_META ALL)



add_executable(spirv_reflection ${SPIRV_REF_SOURCES})
set_target_properties(spirv_reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_link_libraries(spirv_reflection spirv-cross-c)
target_include_directories(spirv_reflection PUBLIC ${PROJECT_SOURCE_DIR}/3rdparty/spirv-cross/)
if(EMSCRIPTEN)
    target_link_options(spirv_reflection PUBLIC -s NODERAWFS=1)
endif()

macro(configure_filesVLK srcDir destDir destDirGL20 destDirGL33 )
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*.frag ${srcDir}/*.vert)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            get_filename_component(FILE_NAME_WO_EXT ${srcTemplatePath} NAME_WE)
            get_filename_component(FILE_NAME ${srcTemplatePath} NAME)
            set(SPIRV_NonOpt "${destDir}/${FILE_NAME}.spv")
#            set(SPIRVOpt "${destDir}/${FILE_NAME}.opt.spv")

            add_custom_command(
                    OUTPUT ${SPIRV_NonOpt}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDir}"
                    COMMAND glslangValidator -V ${srcTemplatePath} -o ${SPIRV_NonOpt}
                    DEPENDS ${srcTemplatePath})

#            if(SPIRV_OPT_APP)
#                add_custom_command(
#                        OUTPUT ${SPIRVOpt}
#                        COMMAND ${SPIRV_OPT_APP} --inline-entry-points-exhaustive
#                        --convert-local-access-chains
#                        --eliminate-local-single-block
#                        --eliminate-local-single-store
#                        --eliminate-insert-extract
#                        --eliminate-dead-code-aggressive
#                        --eliminate-dead-branches
#                        --merge-blocks
#                        --eliminate-local-single-block
#                        --eliminate-local-single-store
#                        --eliminate-local-multi-store
#                        --eliminate-insert-extract
#                        --eliminate-dead-code-aggressive
#                        -o ${SPIRVOpt} ${SPIRV_NonOpt}
#                        DEPENDS ${srcTemplatePath} ${SPIRV_NonOpt})
#            endif()

            add_custom_command(
                    TARGET SPIRV_EXTRACT_META
                    PRE_BUILD
                    DEPENDS ${SPIRV_NonOpt})

            list(APPEND SPIRV_BINARY_FILES_NON_OPT ${SPIRV_NonOpt})
            list(APPEND SPIRV_BINARY_FILES_OPT ${SPIRVOpt})

            #Generate GLSL for ogl33 and ogl20 from spirv
            set(GLSL20_FILE ${destDirGL20}/${FILE_NAME})
            set(GLSL33_FILE ${destDirGL33}/${FILE_NAME})

            get_filename_component(GLSL20_FILE_FULLPATH "${GLSL20_FILE}"
                    REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

            add_custom_command(
                    OUTPUT ${GLSL20_FILE}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDirGL20}"
                    COMMAND spirv_reflection -glsl100 ${SPIRV_NonOpt} > ${GLSL20_FILE_FULLPATH}
                    DEPENDS ${SPIRV_NonOpt})

            list(APPEND GLSL20Files ${GLSL20_FILE})


            get_filename_component(GLSL30_FILE_FULLPATH "${GLSL33_FILE}"
                    REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

            add_custom_command(
                    OUTPUT ${GLSL33_FILE}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDirGL33}"
                    COMMAND spirv_reflection -glsl330 ${SPIRV_NonOpt} > ${GLSL30_FILE_FULLPATH}
                    DEPENDS ${SPIRV_NonOpt})

            list(APPEND GLSL30Files ${GLSL33_FILE})

        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)

#    if (NOT SPIRV_OPT_APP)
        SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_NON_OPT})
#    else()
#        SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_OPT})
#    endif()
endmacro(configure_filesVLK)

configure_filesVLK(${PROJECT_SOURCE_DIR}/shaders/glsl/vulkan ${GLSL_TARGET_FOLDER}/spirv
        ${GLSL_TARGET_FOLDER}/glsl/glsl20/ ${GLSL_TARGET_FOLDER}/glsl/glsl3.3/)

add_custom_command(
        TARGET SPIRV_EXTRACT_META
        COMMAND spirv_reflection -sf ${SPIRV_BINARY_FILES} > ${SPIRV_META_FILE}
        DEPENDS ${SPIRV_BINARY_FILES_NON_OPT}
        VERBATIM)

add_dependencies(SPIRV_EXTRACT_META spirv_reflection glslangValidator ${SPIRV_NonOpt})

add_custom_target(
        ShadersGLSL20
        DEPENDS ${GLSL20Files}
)
add_dependencies(ShadersGLSL20 spirv_reflection)


add_custom_target(
        ShadersGLSL33
        DEPENDS ${GLSL30Files}
)
add_dependencies(ShadersGLSL33 spirv_reflection)

add_custom_target(
    GenerateForWebgl
    DEPENDS ${GLSL20Files}
    DEPENDS ${GLSL30Files}
)