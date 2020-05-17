//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationGL33.h"
#include "../../engine/stringTrim.h"
#include "../../engine/algorithms/hashString.h"
#include "../../engine/shader/ShaderDefinitions.h"
#include "../../gapi/UniformBufferStructures.h"
#include "../interface/IDevice.h"

namespace GL33 {
    std::string textFromUniformType(GLint type) {
        std::string s;
        switch (type) {
            case GL_FLOAT    :
                s = "GL_FLOAT";
                break;
            case GL_FLOAT_VEC2    :
                s = "GL_FLOAT_VEC2";
                break;
            case GL_FLOAT_VEC3    :
                s = "GL_FLOAT_VEC3";
                break;
            case GL_FLOAT_VEC4    :
                s = "GL_FLOAT_VEC4";
                break;
            case GL_INT    :
                s = "GL_INT";
                break;
            case GL_INT_VEC2    :
                s = "GL_INT_VEC2";
                break;
            case GL_INT_VEC3    :
                s = "GL_INT_VEC3";
                break;
            case GL_INT_VEC4    :
                s = "GL_INT_VEC4";
                break;
            case GL_UNSIGNED_INT    :
                s = "GL_UNSIGNED_INT";
                break;
            case GL_UNSIGNED_INT_VEC2    :
                s = "GL_UNSIGNED_INT_VEC2";
                break;
            case GL_UNSIGNED_INT_VEC3    :
                s = "GL_UNSIGNED_INT_VEC3";
                break;
            case GL_UNSIGNED_INT_VEC4    :
                s = "GL_UNSIGNED_INT_VEC4";
                break;
            case GL_BOOL    :
                s = "GL_BOOL";
                break;
            case GL_BOOL_VEC2    :
                s = "GL_BOOL_VEC2";
                break;
            case GL_BOOL_VEC3    :
                s = "GL_BOOL_VEC3";
                break;
            case GL_BOOL_VEC4    :
                s = "GL_BOOL_VEC4";
                break;
            case GL_FLOAT_MAT2    :
                s = "GL_FLOAT_MAT2";
                break;
            case GL_FLOAT_MAT3    :
                s = "GL_FLOAT_MAT3";
                break;
            case GL_FLOAT_MAT4    :
                s = "GL_FLOAT_MAT4";
                break;
            case GL_FLOAT_MAT2x3    :
                s = "GL_FLOAT_MAT2x3";
                break;
            case GL_FLOAT_MAT2x4    :
                s = "GL_FLOAT_MAT2x4";
                break;
            case GL_FLOAT_MAT3x2    :
                s = "GL_FLOAT_MAT3x2";
                break;
            case GL_FLOAT_MAT3x4    :
                s = "GL_FLOAT_MAT3x4";
                break;
            case GL_FLOAT_MAT4x2    :
                s = "GL_FLOAT_MAT4x2";
                break;
            case GL_FLOAT_MAT4x3    :
                s = "GL_FLOAT_MAT4x3";
                break;
            default    :
                s = "Unknown";
                break;
        }
        return s;
    }

    GLuint sizeFromUniformType(GLint type) {
        GLuint s;

#define UNI_CASE(type, numElementsInType, elementType) \
      case type : s = (numElementsInType) * sizeof(elementType); break;

        switch (type) {
            UNI_CASE(GL_FLOAT, 1, GLfloat);
            UNI_CASE(GL_FLOAT_VEC2, 2, GLfloat);
            UNI_CASE(GL_FLOAT_VEC3, 3, GLfloat);
            UNI_CASE(GL_FLOAT_VEC4, 4, GLfloat);
            UNI_CASE(GL_INT, 1, GLint);
            UNI_CASE(GL_INT_VEC2, 2, GLint);
            UNI_CASE(GL_INT_VEC3, 3, GLint);
            UNI_CASE(GL_INT_VEC4, 4, GLint);
            UNI_CASE(GL_UNSIGNED_INT, 1, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
            UNI_CASE(GL_BOOL, 1, GLboolean);
            UNI_CASE(GL_BOOL_VEC2, 2, GLboolean);
            UNI_CASE(GL_BOOL_VEC3, 3, GLboolean);
            UNI_CASE(GL_BOOL_VEC4, 4, GLboolean);
            UNI_CASE(GL_FLOAT_MAT2, 4, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3, 9, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4, 16, GLfloat);
            UNI_CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
            UNI_CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
            default    :
                s = 0;
                break;
        }
        return s;
    }
}
GShaderPermutationGL33::GShaderPermutationGL33(std::string &shaderName, IDevice * device) : m_device(device),
    m_shaderNameVert(shaderName), m_shaderNameFrag(shaderName) {
}

GShaderPermutationGL33::GShaderPermutationGL33(std::string &shaderNameVert, std::string &shaderNameFrag, IDevice * device) : m_device(device),
    m_shaderNameVert(shaderNameVert), m_shaderNameFrag(shaderNameFrag) {
}

void GShaderPermutationGL33::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {

    std::string shaderVertFile =  m_device->loadShader(m_shaderNameVert, IShaderType::gVertexShader);
    std::string shaderFragFile =  m_device->loadShader(m_shaderNameFrag, IShaderType::gFragmentShader);
    if (shaderVertFile.length() == 0) {
        std::cout << "shaderVertFile " << m_shaderNameVert << " is empty" << std::endl;
//        throw;
    }
    if (shaderFragFile.length() == 0) {
        std::cout << "shaderFragFile " << m_shaderNameFrag << " is empty" << std::endl;
//        throw;
    }

    std::string vertShaderString = shaderVertFile;
    std::string fragmentShaderString = shaderFragFile;

    std::string vertExtraDefStrings = vertExtraDef;
    std::string fragExtraDefStrings = fragExtraDef;
    std::string geomExtraDefStrings = "";

    bool esVersion = false;
#ifdef __ANDROID_API__
    esVersion = true;
#endif
#ifdef __APPLE__
    #include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
    glsl330 = false;
#elif TARGET_OS_IPHONE
    glsl330 = false;
#elif TARGET_OS_MAC
    glsl330 = true;
#else
#   error "Unknown Apple platform"
#endif
#endif
#ifdef __EMSCRIPTEN__
    esVersion = true;
#endif
    bool geomShaderExists = false;
    if (esVersion) {
        vertExtraDefStrings = "#version 300 es\n" + vertExtraDefStrings;
        geomExtraDefStrings = "#version 300 es\n" + geomExtraDefStrings;
    } else {
        vertExtraDefStrings = "#version 330\n" + vertExtraDefStrings;
        geomExtraDefStrings = "#version 330\n" + geomExtraDefStrings;
    }

    if (!esVersion) {
        vertExtraDefStrings +=
            "#define precision\n"
            "#define lowp\n"
            "#define mediump\n"
            "#define highp\n"
            "#define FLOATDEC\n";
    } else {
        vertExtraDefStrings += "#define FLOATDEC float;\n";
        vertExtraDefStrings += "precision mediump float;\n";
    };
    geomShaderExists = vertShaderString.find("COMPILING_GS") != std::string::npos;

#ifdef __EMSCRIPTEN__
    geomShaderExists = false;
#endif

    if (esVersion) {
        fragExtraDefStrings = "#version 300 es\n" + fragExtraDefStrings;
    } else {
        fragExtraDefStrings = "#version 330\n" + fragExtraDefStrings;
    }

    if (!esVersion) {
        fragExtraDefStrings +=
            "#define precision\n"
            "#define lowp\n"
            "#define mediump\n"
            "#define highp\n"
            "#define FLOATDEC\n";
    } else {
        fragExtraDefStrings += "precision mediump float;\n";
        fragExtraDefStrings += "#define FLOATDEC float;\n";

    };

    GLint maxVertexUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniforms);
    int maxMatrixUniforms = MAX_MATRIX_NUM;//(maxVertexUniforms / 4) - 9;

    vertExtraDefStrings = vertExtraDefStrings + "#define COMPILING_VS 1\r\n ";
    geomExtraDefStrings = geomExtraDefStrings + "#define COMPILING_GS 1\r\n";
    fragExtraDefStrings = fragExtraDefStrings + "#define COMPILING_FS 1\r\n";

    std::string geometryShaderString = vertShaderString;

    vertShaderString = vertShaderString.insert(0, vertExtraDefStrings);
    fragmentShaderString = fragmentShaderString.insert(0, fragExtraDefStrings);
    geometryShaderString = geometryShaderString.insert(0, geomExtraDefStrings);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderConst = (const GLchar *)vertShaderString.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderConst, 0);
    glCompileShader(vertexShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\nWoW: could not compile vertex shader "<< m_shaderNameVert<<":" << std::endl
                  << vertexShaderConst << std::endl << std::endl
                  << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

//        throw "" ;
    }

    /* 1.2 Compile fragment shader */
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderConst = (const GLchar *) fragmentShaderString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderConst, 0);
    glCompileShader(fragmentShader);

    // Check if it compiled
    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\nWoW: could not compile fragment shader "<<m_shaderNameFrag<<":" << std::endl <<std::flush
                  << fragmentShaderConst << std::flush << std::endl << std::endl
                  << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

//        throw "" ;
    }

    GLuint geometryShader = 0;

//#ifndef WITH_GLESv2
//    if (geomShaderExists) {
//        /* 1.2.1 Compile geometry shader */
//        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
//        const GLchar *geometryShaderConst = (const GLchar *) geometryShaderString.c_str();
//        glShaderSource(geometryShader, 1, &geometryShaderConst, 0);
//        glCompileShader(geometryShader);
//
//        // Check if it compiled
//        success = 0;
//        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
//        if (!success) {
//            // Something went wrong during compilation; get the error
//            GLint maxLength = 0;
//            glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &maxLength);
//
//            //The maxLength includes the NULL character
//            std::vector<GLchar> infoLog(maxLength);
//            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
//            std::cout << "\ncould not compile fragment shader " << m_shaderName << ":" << std::endl
//                      << fragmentShaderConst << std::endl << std::endl
//                      << "error: " << std::string(infoLog.begin(), infoLog.end()) << std::endl << std::flush;
//
//            throw "";
//        }
//    }
//#endif



    /* 1.3 Link the program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geomShaderExists)
        glAttachShader(program, geometryShader);

//    for (int i = 0; i < shaderDefinition1->attributesNum; i++) {
//        glBindAttribLocation(program, shaderDefinition1->attributes[i].number, shaderDefinition1->attributes[i].variableName);
//    }

    // link the program.
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char logbuffer[1000];
        int loglen;
        glGetProgramInfoLog(program, sizeof(logbuffer), &loglen, logbuffer);
        std::cout << "OpenGL Program Linker Error: " << logbuffer << std::endl << std::flush;
//        throw "could not compile shader:" ;
    }

    //Get Uniforms
    m_programBuffer = program;

    //Get uniforms data
    GLint count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
//    printf("Active Uniforms: %d\n", count);
    for (GLint i = 0; i < count; i++)
    {
        const GLsizei bufSize = 32; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length
        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        GLint location = glGetUniformLocation(program, name);

        this->setUnf(std::string(name), location);
//        printf("Uniform #%d Type: %u Name: %s Location: %d\n", i, type, name, location);
    }
//    if (!shaderName.compare("m2Shader")) {
//        std::cout << fragmentShaderString << std::endl << std::flush;
//    }

    //Get uniform block data
    GLint numUniformBlocks;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

    // get information about each uniform block
    /*
    for(int uniformBlock=0; uniformBlock < numUniformBlocks; uniformBlock++) {
        // get size of name of the uniform block
        GLint nameLength;
        glGetActiveUniformBlockiv(program, uniformBlock,
                                  GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);

        // get name of uniform block
        std::unique_ptr<GLchar> blockName(new GLchar[nameLength]);
        glGetActiveUniformBlockName(program, uniformBlock,
                                    nameLength, nullptr, blockName.get());

        std::cout << "Uniform Block "<< uniformBlock<< " : " << blockName.get() << std::endl;

        // get size of uniform block in bytes
        GLint uboSize;
        glGetActiveUniformBlockiv(program, uniformBlock,
                                  GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
        std::cout << "	Size : " << uboSize << std::endl;

        // get number of uniform variables in uniform block
        GLint numberOfUniformsInBlock;
        glGetActiveUniformBlockiv(program, uniformBlock,
                                  GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniformsInBlock);
        std::cout << "	Members : " << numberOfUniformsInBlock << std::endl;

        // get indices of uniform variables in uniform block
        GLuint *uniformsIndices = new GLuint[numberOfUniformsInBlock];
        /*
        glGetActiveUniformBlockiv(program, uniformBlock,
                                  GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint *)(uniformsIndices));

        // get parameters of all uniform variables in uniform block
        for (int uniformMember = 0; uniformMember < numberOfUniformsInBlock; uniformMember++) {
            if (uniformsIndices[uniformMember] > 0) {
                // index of uniform variable
                GLuint *tUniformIndex = &uniformsIndices[uniformMember];

//                uniformsIndices.get()[uniformMember];
                GLint uniformNameLength, uniformOffset, uniformSize;
                GLint uniformType, arrayStride, matrixStride;

                // get length of name of uniform variable
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_NAME_LENGTH, &uniformNameLength);
                // get name of uniform variable
                GLchar *uniformName = new GLchar[uniformNameLength];
                GLsizei uniform_length;
                GLint uniform_size;
                GLenum uniform_type;
                glGetActiveUniform(program, *tUniformIndex, uniformNameLength,
                                   &uniform_length, &uniform_size, &uniform_type, uniformName);

                // get offset of uniform variable related to start of uniform block
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_OFFSET, &uniformOffset);
                // get size of uniform variable (number of elements)
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_SIZE, &uniformSize);
                // get type of uniform variable (size depends on this value)
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_TYPE, &uniformType);
                // offset between two elements of the array
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
                // offset between two vectors in matrix
                glGetActiveUniformsiv(program, 1, tUniformIndex,
                                      GL_UNIFORM_MATRIX_STRIDE, &matrixStride);

                // Size of uniform variable in bytes
                GLuint sizeInBytes = uniformSize * sizeFromUniformType(uniformType);

                // output data
                std::cout << "- " << uniformName << std::endl;
                std::cout << "size " << sizeInBytes << std::endl;
                std::cout << "offset " << uniformOffset << std::endl;
                std::cout << "type " << textFromUniformType(uniformType) << std::endl;
                std::cout << "array stride " << arrayStride << std::endl;
                std::cout << "matrix stride " << +matrixStride << std::endl;

                delete uniformName;
            } else {
                std::cout << "- Bad uniform" << std::endl;
            }
        }

        delete uniformsIndices;
    }
     */

    m_uboBlockIndex[0] = glGetUniformBlockIndex(program, "sceneWideBlockVSPS");
    m_uboBlockIndex[1] = glGetUniformBlockIndex(program, "modelWideBlockVS");
    m_uboBlockIndex[2] = glGetUniformBlockIndex(program, "meshWideBlockVS");

    m_uboBlockIndex[3] = glGetUniformBlockIndex(program, "modelWideBlockPS");
    m_uboBlockIndex[4] = glGetUniformBlockIndex(program, "meshWideBlockPS");
}

void GShaderPermutationGL33::setUnf(const std::string &name, GLuint index)  {
    const char * cstr = name.c_str();
    m_uniformMap[CalculateFNV(cstr)] = index;
}

void GShaderPermutationGL33::bindProgram() {
    glUseProgram(m_programBuffer);
}
void GShaderPermutationGL33::unbindProgram() {
    glUseProgram(0);
}