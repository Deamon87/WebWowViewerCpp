file(GLOB FILES ${PROJECT_SOURCE_DIR}/src/glsl/*.glsl)

file(
        WRITE ${PROJECT_SOURCE_DIR}/src/engine/shaderDefinitions.h
        "#include <string>\n"
        "#include \"shadersStructures.h\"\n\n"
)
set(shaderDefMap "")
foreach (file ${FILES})
    set(filename ${file})
    get_filename_component(NAME ${filename} NAME_WE)
    get_filename_component(EXT ${filename} EXT)

    set(shaderAttributeEnum "enum class Attribute { \n")

    file(
            APPEND ${PROJECT_SOURCE_DIR}/src/engine/shaderDefinitions.h
            ""
            "constexpr const char* const ${NAME}String =\n #include <../glsl/${NAME}${EXT}>\n;\n\n"
    )
    set(shaderDefMap "${shaderDefMap} { \"${NAME}\", ${NAME}Def },\n")

    #Write attributes numbers to file
    execute_process(
            COMMAND node uniformParser.js ${filename} attribute
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            OUTPUT_VARIABLE attributes
    )

    string(REPLACE "\n" ";" attributes ${attributes})
    set(shaderAttributesArray "constexpr shaderItem ${NAME}Attributes [] = {\n")
    set(attributesCount 0)
    foreach (attributeData ${attributes})
        string(REPLACE " " ";" ATTRIBUTE_LIST ${attributeData})
        list(GET ATTRIBUTE_LIST 0 attribute_name)
        list(GET ATTRIBUTE_LIST 1 attribute_index)
        MATH(EXPR attributesCount "${attributesCount}+1")
        set(shaderAttributeEnum "${shaderAttributeEnum} ${attribute_name} = ${attribute_index},")
        set(shaderAttributesArray "${shaderAttributesArray} {\"${attribute_name}\", +${NAME}::Attribute::${attribute_name}},\n")
    endforeach()
    set(shaderAttributeEnum "${shaderAttributeEnum} ${NAME}AttributeEnd\n };\n")
    set(shaderAttributesArray "${shaderAttributesArray} };\n")

    #Write uniforms numbers to file
# TODO: WARNING: OPENGL does not support explicit uniform location through function calls
#    set(shaderUniformEnum "enum class Uniform { \n")
#    execute_process(
#            COMMAND node uniformParser.js ${filename} uniform
#            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#            OUTPUT_VARIABLE uniforms
#    )
#
#    set(shaderUniformArray "constexpr shaderItem ${NAME}Uniforms [] = {\n")
#    string(REPLACE "\n" ";" uniforms ${uniforms})
#    set(uniformCount 0)
#    foreach (uniformData ${uniforms})
#        string(REPLACE " " ";" UNIFORM_LIST ${uniformData})
#        list(GET UNIFORM_LIST 0 uniform_name)
#        list(GET UNIFORM_LIST 1 uniform_index)
#        MATH(EXPR uniformCount "${uniformCount}+1")
#        set(shaderUniformEnum "${shaderUniformEnum} ${uniform_name} = ${uniform_index},")
#        set(shaderUniformArray "${shaderUniformArray} {\"${uniform_name}\",+${NAME}::Uniform::${uniform_name} },\n")
#    endforeach()
#    set(shaderUniformEnum "${shaderUniformEnum} ${NAME}ShaderEnd\n };\n")
#    set(shaderUniformArray "${shaderUniformArray} };\n")
    file(
            APPEND ${PROJECT_SOURCE_DIR}/src/engine/shaderDefinitions.h
            "struct ${NAME} {\n ${shaderAttributeEnum} ${shaderUniformEnum}};\n"
            "inline constexpr const int operator+ (${NAME}::Attribute const val) { return static_cast<const int>(val); };\n"
#            "inline constexpr const int operator+ (${NAME}::Uniform const val) { return static_cast<const int>(val); };\n"
            "${shaderAttributesArray}"
#            "${shaderUniformArray}"
    )

    #End of uniforms\attributes

    file(
            APPEND ${PROJECT_SOURCE_DIR}/src/engine/shaderDefinitions.h
            ""
            "constexpr const static shaderDefinition ${NAME}Def = { \n"
            "${NAME}String,\n"
            "${attributesCount},\n"
            "${NAME}Attributes\n"
            #"${uniformCount},\n"
            #"${NAME}Uniforms\n"
            "};\n\n"
    )

    set(source "")

    file(STRINGS ${filename} lines)
    foreach (line ${lines})
        set(source "${source}    ${line}\n")
    endforeach()

    file(
            WRITE ${PROJECT_SOURCE_DIR}/lib/glsl/${NAME}${EXT}
            "R\"=====(\n"
            "${source}"
            "\n)=====\""
    )

endforeach()
file(
        APPEND ${PROJECT_SOURCE_DIR}/src/engine/shaderDefinitions.h
        "constexpr innerType map[] = {\n ${shaderDefMap} \n };"
)

include_directories(${PROJECT_SOURCE_DIR}/lib/glsl)