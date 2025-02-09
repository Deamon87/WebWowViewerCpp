if (ANDROID)
    set(GLSL_TARGET_FOLDER ${ANDROID_ASSETSFOLDER})
else()
    set(GLSL_TARGET_FOLDER ${CMAKE_BINARY_DIR})
endif()

message( Shader SPIRV_OPT_APP = ${SPIRV_OPT_APP})
message( COMPILE_SHADER_WITH_DEBUG = ${COMPILE_SHADER_WITH_DEBUG})

if (COMPILE_SHADER_WITH_DEBUG EQUAL 1)
    set(SHADER_DEBUG_FLAG "-g")
    set(APP_RUN_PREFIX "")
    set (SPIRV_OPT_APP "")
else()
    set(SHADER_DEBUG_FLAG "")
endif()

if (EMSCRIPTEN)
    set(APP_RUN_PREFIX node --experimental-wasm-simd)
endif()

message(INFO "GLSL_TARGET_FOLDER = ${GLSL_TARGET_FOLDER}")


set(SPIRV_META_DIR_local ${CMAKE_BINARY_DIR}/shader_meta)
set(SPIRV_META_DIR ${SPIRV_META_DIR_local} PARENT_SCOPE)
set(SPIRV_META_FILE ${SPIRV_META_DIR_local}/ShaderDefinitions.h)

file(MAKE_DIRECTORY SPIRV_META_DIR)

if(NOT ANDROID)
    #GLSL Compiler
    add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/spirv-cross EXCLUDE_FROM_ALL)
    set(BUILD_EXTERNAL OFF)
    set(ENABLE_OPT OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/glsllang EXCLUDE_FROM_ALL)
    if(EMSCRIPTEN)
        target_link_options(glslang-standalone PUBLIC -s NODERAWFS=1)
    endif()
    set(BUILD_EXTERNAL OFF)
    set_target_properties(glslang-standalone PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    #SPIRV_Reflections
    set(SPIRV_REF_SOURCES
            src/spirv/spirv_refl_main.cpp
            src/spirv/webGLSLCompiler.cpp
            src/spirv/webGLSLCompiler.h
            src/spirv/dumpShaderFields.h src/spirv/dumpGLSLShader.h)

    add_executable(spirv_reflection ${SPIRV_REF_SOURCES})
    set_target_properties(spirv_reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    target_link_libraries(spirv_reflection spirv-cross-c)
    target_include_directories(spirv_reflection PUBLIC ${PROJECT_SOURCE_DIR}/3rdparty/spirv-cross/)
    if(EMSCRIPTEN)
        target_link_options(spirv_reflection PUBLIC -s NODERAWFS=1)
    endif()
    set_target_properties(spirv_reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#    set(PATH_TO_NATIVE_GLSL_VALIDATOR "${CMAKE_BINARY_DIR}/glslang{CMAKE_EXECUTABLE_SUFFIX}")
#    set(PATH_TO_NATIVE_SPIRV_REFLECTION "${CMAKE_BINARY_DIR}/spirv_reflection${CMAKE_EXECUTABLE_SUFFIX}")
    if (NOT PATH_TO_NATIVE_GLSL_VALIDATOR)
        set(PATH_TO_NATIVE_GLSL_VALIDATOR $<TARGET_FILE:glslang-standalone>)
    else()
        MESSAGE(WARNING "PATH_TO_NATIVE_GLSL_VALIDATOR = ${PATH_TO_NATIVE_GLSL_VALIDATOR}")
    endif()
    set(PATH_TO_NATIVE_SPIRV_REFLECTION $<TARGET_FILE:spirv_reflection>)
else()
    MESSAGE(WARNING "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
    MESSAGE(WARNING "PATH_TO_NATIVE_GLSL_VALIDATOR = ${PATH_TO_NATIVE_GLSL_VALIDATOR}")
    MESSAGE(WARNING "PATH_TO_NATIVE_SPIRV_REFLECTION = ${PATH_TO_NATIVE_SPIRV_REFLECTION}")
    if (NOT EXISTS "${PATH_TO_NATIVE_GLSL_VALIDATOR}")
        MESSAGE(ERROR "Native precompiled GLSL Validator is required for Android")
        return()
    endif()
    if (NOT EXISTS "${PATH_TO_NATIVE_SPIRV_REFLECTION}")
        MESSAGE(ERROR "Native precompiled Spirv Reflection is required for Android")
        return()
    endif()

    add_custom_target(glslang-standalone)
    add_custom_target(spirv_reflection)
endif()


set(spirvNonOptDir ${CMAKE_BINARY_DIR}/spirv_raw)
set(spirvOptDir ${CMAKE_BINARY_DIR}/spirv)

if (NOT SPIRV_OPT_APP)
    set(spirvNonOptDir ${spirvOptDir})
endif ()

macro(configure_filesVLK rootDir srcCommonDirs shaderDirs shaderFileExts inputType destDir)
    message(STATUS "Configuring directory ${destDir}")

    file(MAKE_DIRECTORY ${spirvNonOptDir})

    foreach(srcCommonDir ${srcCommonDirs})
        file(GLOB_RECURSE commonFilesRelative RELATIVE ${srcCommonDir} ${srcCommonDir}/*.glsl)

        foreach(commonFileRelative ${commonFilesRelative})
            set(commonFileFullPath ${srcCommonDir}/${commonFileRelative})
            if(NOT IS_DIRECTORY ${commonFileFullPath})
                list(APPEND commonFiles ${commonFileFullPath})
            endif()
        endforeach(commonFileRelative)
    endforeach(srcCommonDir)

    foreach(shaderDir ${shaderDirs})
        set(srcDir "${rootDir}/${shaderDir}")

        set(shaderfileExtTemplate "")
        foreach(shaderFileExt ${shaderFileExts})
           list(APPEND shaderfileExtTemplate ${srcDir}/*${shaderFileExt})
        endforeach (shaderFileExt)
        #message("shaderfileExtTemplate = ${shaderfileExtTemplate}")

        file(GLOB_RECURSE templateFiles RELATIVE ${srcDir} ${shaderfileExtTemplate})
        #message("templateFiles = ${templateFiles}")
        file(MAKE_DIRECTORY ${spirvNonOptDir}/${shaderDir})
        file(MAKE_DIRECTORY ${destDir}/${shaderDir})

        foreach(templateFile ${templateFiles})
            set(srcTemplatePath ${srcDir}/${templateFile})
            if(NOT IS_DIRECTORY ${srcTemplatePath})
                message(STATUS "Configuring file ${templateFile}")
                file(RELATIVE_PATH srcFileRelative ${srcDir} ${srcTemplatePath})

                get_filename_component(FILE_NAME_WO_EXT ${srcTemplatePath} NAME_WE)
                get_filename_component(FILE_NAME ${srcTemplatePath} NAME)
                get_filename_component(SHADER_SUB_DIR ${srcFileRelative} DIRECTORY )
                if (SHADER_SUB_DIR)
                    set(SHADER_SUB_DIR "${SHADER_SUB_DIR}/")
                endif()
#                file(RELATIVE_PATH SHADER_SUB_DIR shaderDir ${srcTemplatePath})

#                message(srcFileRelative = ${srcFileRelative})
#                message(srcTemplatePath = ${srcTemplatePath})
#                message(templateFile = ${templateFile})
#                message(SHADER_SUB_DIR = ${SHADER_SUB_DIR})
                if (NOT SPIRV_OPT_APP)
                    set(SPIRV_NonOpt "${destDir}/${shaderDir}/${SHADER_SUB_DIR}${FILE_NAME}.spv")
                else()
                    set(SPIRV_NonOpt "${spirvNonOptDir}/${shaderDir}/${SHADER_SUB_DIR}${FILE_NAME}.spv")
                    set(SPIRVOpt "${destDir}/${shaderDir}/${SHADER_SUB_DIR}${FILE_NAME}.spv")
                endif()

#                message(SPIRV_NonOpt = ${SPIRV_NonOpt})
#                message(SPIRVOpt = ${SPIRVOpt})

#                message("inputType = ${inputType}")
                if (${inputType} STREQUAL "glsl")
#                    message("glsl SPIRV_NonOpt = ${SPIRV_NonOpt}")
                    add_custom_command(
                            OUTPUT ${SPIRV_NonOpt}
                            COMMAND ${CMAKE_COMMAND} -E make_directory "${destDir}"
                            COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_GLSL_VALIDATOR} -V ${SHADER_DEBUG_FLAG} ${srcTemplatePath} -o ${SPIRV_NonOpt}
                            DEPENDS ${srcTemplatePath} glslang-standalone ${commonFiles})
                elseif (${inputType} STREQUAL "slang")
#                    message("slang SPIRV_NonOpt = ${SPIRV_NonOpt}")
                    add_custom_command(
                        OUTPUT ${SPIRV_NonOpt}
                        COMMAND ${CMAKE_COMMAND} -E make_directory "${destDir}"
                        COMMAND ${APP_RUN_PREFIX} ${SLANG_COMPILER} ${srcTemplatePath} -obfuscate -matrix-layout-row-major -target spirv -entry main -o ${SPIRV_NonOpt}
#                        COMMAND ${APP_RUN_PREFIX} ${SLANG_COMPILER} ${srcTemplatePath} -matrix-layout-column-major -target glsl -entry main -o ${SPIRV_NonOpt}.glsl
                        DEPENDS ${srcTemplatePath} ${commonFiles})
                endif ()

                if(SPIRV_OPT_APP)
                    add_custom_command(
                            OUTPUT ${SPIRVOpt}
                            COMMAND ${SPIRV_OPT_APP} --preserve-bindings -O -Os -o ${SPIRVOpt} ${SPIRV_NonOpt}
                            DEPENDS ${srcTemplatePath} ${SPIRV_NonOpt})
                endif()

                list(APPEND SPIRV_BINARY_FILES_NON_OPT ${SPIRV_NonOpt})
                list(APPEND SPIRV_BINARY_FILES_OPT ${SPIRVOpt})

            endif(NOT IS_DIRECTORY ${srcTemplatePath})
        endforeach(templateFile)
    endforeach(shaderDir)
endmacro(configure_filesVLK)

configure_filesVLK(
        ${PROJECT_SOURCE_DIR}/shaders/glsl/
        "${PROJECT_SOURCE_DIR}/shaders/glsl/common;${PROJECT_SOURCE_DIR}/shaders/glsl/bindless"
        "."
        ".vert;.frag"
        "glsl"
        ${spirvOptDir})

#configure_filesVLK(
#        "${PROJECT_SOURCE_DIR}/shaders/slang/compute"
#        "${PROJECT_SOURCE_DIR}/shaders/slang/common"
#        "."
#        ".slang"
#        "slang"
#        ${GLSL_TARGET_FOLDER}/spirv)

#Select the list for spirv files

if (NOT SPIRV_OPT_APP)
    SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_NON_OPT})
else()
    SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_OPT})
endif()

message("Executing ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_SPIRV_REFLECTION} -sf ${GLSL_TARGET_FOLDER}/spirv_raw ${SPIRV_BINARY_FILES_NON_OPT} > ${SPIRV_META_FILE}")

add_custom_command(
        OUTPUT ${SPIRV_META_FILE}
        COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_SPIRV_REFLECTION} -sf ${spirvNonOptDir} ${SPIRV_BINARY_FILES_NON_OPT} > ${SPIRV_META_FILE}
        DEPENDS ${SPIRV_BINARY_FILES_NON_OPT} spirv_reflection glslang-standalone ShadersVulkanNonOpt
)

add_custom_target(SPIRV_EXTRACT_META DEPENDS ${SPIRV_META_FILE})
add_custom_target(SPIRV_BUILD_SHADERS DEPENDS ${SPIRV_BINARY_FILES})


add_custom_target(
        ShadersGLSL20
        DEPENDS ${GLSL20Files}
)
add_custom_target(
        ShadersVulkanNonOpt
        DEPENDS ${SPIRV_BINARY_FILES_NON_OPT}
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

#Add shaders on install
if (NOT EMSCRIPTEN)
    if(LINK_OGL2)
        install(FILES ${GLSL20Files}
                DESTINATION ${CMAKE_INSTALL_BINDIR}/glsl/glsl20/
        )
    endif()
    if(LINK_OGL3)
        install(FILES ${GLSL30Files}
                DESTINATION ${CMAKE_INSTALL_BINDIR}/glsl/glsl3.3/
        )
    endif()
    if(LINK_VULKAN)
        if(SPIRV_OPT_APP)
            install(DIRECTORY ${CMAKE_BINARY_DIR}/spirv/
                    DESTINATION ${CMAKE_INSTALL_BINDIR}/spirv
            )
        else()
            install(DIRECTORY ${CMAKE_BINARY_DIR}/spirv_raw/
                    DESTINATION ${CMAKE_INSTALL_BINDIR}/spirv
            )
        endif()
    endif()
endif()