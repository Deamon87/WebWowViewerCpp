if (ANDROID)
    set(GLSL_TARGET_FOLDER ${ANDROID_ASSETSFOLDER})
else()
    set(GLSL_TARGET_FOLDER ${CMAKE_BINARY_DIR})
endif()


if (COMPILE_SHADER_WITH_DEBUG)
    set(SHADER_DEBUG_FLAG "-g")
else()
    set(SHADER_DEBUG_FLAG "")
endif()

set(APP_RUN_PREFIX "")
if (EMSCRIPTEN)
    set(APP_RUN_PREFIX node --experimental-wasm-simd)
endif()

message(INFO "GLSL_TARGET_FOLDER = ${GLSL_TARGET_FOLDER}")

set(SPIRV_META_FILE ${PROJECT_SOURCE_DIR}/src/engine/shader/ShaderDefinitions.h)

if(NOT ANDROID)
    #GLSL Compiler
    add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/spirv-cross EXCLUDE_FROM_ALL)
    set(BUILD_EXTERNAL OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/3rdparty/glsllang EXCLUDE_FROM_ALL)
    if(EMSCRIPTEN)
        target_link_options(glslangValidator PUBLIC -s NODERAWFS=1)
    endif()
    set(BUILD_EXTERNAL OFF)
    set_target_properties(glslangValidator PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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

#    set(PATH_TO_NATIVE_GLSL_VALIDATOR "${CMAKE_BINARY_DIR}/glslangValidator${CMAKE_EXECUTABLE_SUFFIX}")
#    set(PATH_TO_NATIVE_SPIRV_REFLECTION "${CMAKE_BINARY_DIR}/spirv_reflection${CMAKE_EXECUTABLE_SUFFIX}")
    if (NOT PATH_TO_NATIVE_GLSL_VALIDATOR)
        set(PATH_TO_NATIVE_GLSL_VALIDATOR $<TARGET_FILE:glslangValidator>)
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

    add_custom_target(glslangValidator)
    add_custom_target(spirv_reflection)
endif()

set (SPIRV_OPT_APP "")

macro(configure_filesVLK srcDir srcCommonDir destDir destDirGL20 destDirGL33 )
    message(STATUS "Configuring directory ${destDir}")
    set(spirvNonOptDir ${CMAKE_BINARY_DIR}/spriv_raw)
    make_directory(${spirvNonOptDir})

    file(GLOB_RECURSE templateFiles RELATIVE ${srcDir} ${srcDir}/*.frag ${srcDir}/*.vert)
    file(GLOB_RECURSE commonFilesRelative RELATIVE ${srcCommonDir} ${srcCommonDir}/*.glsl)

    foreach(commonFileRelative ${commonFilesRelative})
        set(commonFileFullPath ${srcCommonDir}/${commonFileRelative})
        if(NOT IS_DIRECTORY ${commonFileFullPath})
            list(APPEND commonFiles ${commonFileFullPath})
        endif()
    endforeach(commonFileRelative)

    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            get_filename_component(FILE_NAME_WO_EXT ${srcTemplatePath} NAME_WE)
            get_filename_component(FILE_NAME ${srcTemplatePath} NAME)
            if (NOT SPIRV_OPT_APP)
                set(SPIRV_NonOpt "${destDir}/${FILE_NAME}.spv")
            else()
                set(SPIRV_NonOpt "${spirvNonOptDir}/${FILE_NAME}.spv")
                set(SPIRVOpt "${destDir}/${FILE_NAME}.spv")
            endif()


            add_custom_command(
                    OUTPUT ${SPIRV_NonOpt}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDir}"
                    COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_GLSL_VALIDATOR} -V ${SHADER_DEBUG_FLAG} ${srcTemplatePath} -o ${SPIRV_NonOpt}
                    DEPENDS ${srcTemplatePath} glslangValidator ${commonFiles})

            if(SPIRV_OPT_APP)
                add_custom_command(
                        OUTPUT ${SPIRVOpt}
                        COMMAND ${SPIRV_OPT_APP} -O -o ${SPIRVOpt} ${SPIRV_NonOpt}
                        DEPENDS ${srcTemplatePath} ${SPIRV_NonOpt})
            endif()

            list(APPEND SPIRV_BINARY_FILES_NON_OPT ${SPIRV_NonOpt})
            list(APPEND SPIRV_BINARY_FILES_OPT ${SPIRVOpt})

            #Generate GLSL for ogl33 and ogl20 from spirv
            set(GLSL20_FILE ${destDirGL20}/${FILE_NAME})
            set(GLSL33_FILE ${destDirGL33}/${FILE_NAME})

            get_filename_component(GLSL20_FILE_FULLPATH "${GLSL20_FILE}"
                    REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

            SET(OGL3_GLSL_OPTION "")
            if (ANDROID)
                SET(OGL3_GLSL_OPTION "-glslEs310")
            elseif(EMSCRIPTEN)
                SET(OGL3_GLSL_OPTION "-glslEs300")
            else()
                SET(OGL3_GLSL_OPTION "-glsl330")
            endif()

            SET(OGL2_GLSL_OPTION "")
            if (ANDROID OR EMSCRIPTEN)
                SET(OGL2_GLSL_OPTION "-glsl100")
            else()
                SET(OGL2_GLSL_OPTION "-glsl100")
            endif()


            add_custom_command(
                    OUTPUT ${GLSL20_FILE}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDirGL20}"
                    COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_SPIRV_REFLECTION} ${OGL2_GLSL_OPTION} ${SPIRV_NonOpt} > ${GLSL20_FILE_FULLPATH}
                    DEPENDS ${SPIRV_NonOpt} spirv_reflection)

            list(APPEND GLSL20Files ${GLSL20_FILE})

            get_filename_component(GLSL30_FILE_FULLPATH "${GLSL33_FILE}"
                    REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

            add_custom_command(
                    OUTPUT ${GLSL33_FILE}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${destDirGL33}"
                    COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_SPIRV_REFLECTION} ${OGL3_GLSL_OPTION} ${SPIRV_NonOpt} > ${GLSL30_FILE_FULLPATH}
                    DEPENDS spirv_reflection ${SPIRV_NonOpt})

            list(APPEND GLSL30Files ${GLSL33_FILE})

        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)

    if (NOT SPIRV_OPT_APP)
        SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_NON_OPT})
    else()
        SET(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_OPT})
    endif()
endmacro(configure_filesVLK)

configure_filesVLK(
        ${PROJECT_SOURCE_DIR}/shaders/glsl/forwardRendering
        ${PROJECT_SOURCE_DIR}/shaders/glsl/common
        ${GLSL_TARGET_FOLDER}/spirv
        ${GLSL_TARGET_FOLDER}/glsl/glsl20/
        ${GLSL_TARGET_FOLDER}/glsl/glsl3.3/)

add_custom_command(
        OUTPUT ${SPIRV_META_FILE}
        COMMAND ${APP_RUN_PREFIX} ${PATH_TO_NATIVE_SPIRV_REFLECTION} -sf ${SPIRV_BINARY_FILES} > ${SPIRV_META_FILE}
        DEPENDS ${SPIRV_BINARY_FILES} spirv_reflection glslangValidator ShadersVulkanNonOpt
)

add_custom_target(SPIRV_EXTRACT_META DEPENDS ${SPIRV_META_FILE})


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
        install(FILES ${SPIRV_BINARY_FILES_NON_OPT}
                DESTINATION ${CMAKE_INSTALL_BINDIR}/spirv
        )
    endif()
endif()