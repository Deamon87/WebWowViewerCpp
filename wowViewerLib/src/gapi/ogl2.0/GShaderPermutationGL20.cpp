//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationGL20.h"
#include "../../engine/stringTrim.h"
#include "../../engine/algorithms/hashString.h"
#include "../../engine/shader/ShaderDefinitions.h"
#include "../../gapi/UniformBufferStructures.h"
#include "../interface/IDevice.h"

std::string textFromUniformType(GLint type)
{
    std::string s;
    switch(type)
    {
        case GL_FLOAT	: s = "GL_FLOAT"; break;
        case GL_FLOAT_VEC2	: s = "GL_FLOAT_VEC2"; break;
        case GL_FLOAT_VEC3	: s = "GL_FLOAT_VEC3"; break;
        case GL_FLOAT_VEC4	: s = "GL_FLOAT_VEC4"; break;
        case GL_INT	: s = "GL_INT"; break;
        case GL_INT_VEC2	: s = "GL_INT_VEC2"; break;
        case GL_INT_VEC3	: s = "GL_INT_VEC3"; break;
        case GL_INT_VEC4	: s = "GL_INT_VEC4"; break;
        case GL_UNSIGNED_INT	: s = "GL_UNSIGNED_INT"; break;
        case GL_UNSIGNED_INT_VEC2	: s = "GL_UNSIGNED_INT_VEC2"; break;
        case GL_UNSIGNED_INT_VEC3	: s = "GL_UNSIGNED_INT_VEC3"; break;
        case GL_UNSIGNED_INT_VEC4	: s = "GL_UNSIGNED_INT_VEC4"; break;
        case GL_BOOL	: s = "GL_BOOL"; break;
        case GL_BOOL_VEC2	: s = "GL_BOOL_VEC2"; break;
        case GL_BOOL_VEC3	: s = "GL_BOOL_VEC3"; break;
        case GL_BOOL_VEC4	: s = "GL_BOOL_VEC4"; break;
        case GL_FLOAT_MAT2	: s = "GL_FLOAT_MAT2"; break;
        case GL_FLOAT_MAT3	: s = "GL_FLOAT_MAT3"; break;
        case GL_FLOAT_MAT4	: s = "GL_FLOAT_MAT4"; break;
        case GL_FLOAT_MAT2x3	: s = "GL_FLOAT_MAT2x3"; break;
        case GL_FLOAT_MAT2x4	: s = "GL_FLOAT_MAT2x4"; break;
        case GL_FLOAT_MAT3x2	: s = "GL_FLOAT_MAT3x2"; break;
        case GL_FLOAT_MAT3x4	: s = "GL_FLOAT_MAT3x4"; break;
        case GL_FLOAT_MAT4x2	: s = "GL_FLOAT_MAT4x2"; break;
        case GL_FLOAT_MAT4x3	: s = "GL_FLOAT_MAT4x3"; break;
        default	: s = "Unknown"; break;
    }
    return s;
}

GLuint sizeFromUniformType(GLint type)
{
    GLuint s;

#define UNI_CASE(type, numElementsInType, elementType) \
      case type : s = (numElementsInType) * sizeof(elementType); break;

    switch(type)
    {
        UNI_CASE(GL_FLOAT,	1, GLfloat);
        UNI_CASE(GL_FLOAT_VEC2,	2, GLfloat);
        UNI_CASE(GL_FLOAT_VEC3,	3, GLfloat);
        UNI_CASE(GL_FLOAT_VEC4,	4, GLfloat);
        UNI_CASE(GL_INT,	1, GLint);
        UNI_CASE(GL_INT_VEC2,	2, GLint);
        UNI_CASE(GL_INT_VEC3,	3, GLint);
        UNI_CASE(GL_INT_VEC4,	4, GLint);
        UNI_CASE(GL_UNSIGNED_INT,	1, GLuint);
        UNI_CASE(GL_UNSIGNED_INT_VEC2,	2, GLuint);
        UNI_CASE(GL_UNSIGNED_INT_VEC3,	3, GLuint);
        UNI_CASE(GL_UNSIGNED_INT_VEC4,	4, GLuint);
        UNI_CASE(GL_BOOL,	1, GLboolean);
        UNI_CASE(GL_BOOL_VEC2,	2, GLboolean);
        UNI_CASE(GL_BOOL_VEC3,	3, GLboolean);
        UNI_CASE(GL_BOOL_VEC4,	4, GLboolean);
        UNI_CASE(GL_FLOAT_MAT2,	4, GLfloat);
        UNI_CASE(GL_FLOAT_MAT3,	9, GLfloat);
        UNI_CASE(GL_FLOAT_MAT4,	16, GLfloat);
        UNI_CASE(GL_FLOAT_MAT2x3,	6, GLfloat);
        UNI_CASE(GL_FLOAT_MAT2x4,	8, GLfloat);
        UNI_CASE(GL_FLOAT_MAT3x2,	6, GLfloat);
        UNI_CASE(GL_FLOAT_MAT3x4,	12, GLfloat);
        UNI_CASE(GL_FLOAT_MAT4x2,	8, GLfloat);
        UNI_CASE(GL_FLOAT_MAT4x3,	12, GLfloat);
        default	: s = 0; break;
    }
    return s;
}

GShaderPermutationGL20::GShaderPermutationGL20(std::string &shaderName, IDevice * device) : m_device(device), m_shaderName(shaderName){

}

void GShaderPermutationGL20::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {

    std::string shaderVertFile =  m_device->loadShader(m_shaderName, IShaderType::gVertexShader);
    std::string shaderFragFile =  m_device->loadShader(m_shaderName, IShaderType::gFragmentShader);
    if (shaderVertFile.length() == 0) {
        std::cout << "shaderVertFile " << m_shaderName << " is empty" << std::endl;
//        throw;
    }
    if (shaderFragFile.length() == 0) {
        std::cout << "shaderFragFile " << m_shaderName << " is empty" << std::endl;
//        throw;
    }

    std::string vertShaderString = shaderVertFile;
    std::string fragmentShaderString = shaderFragFile;

    std::string vertExtraDefStrings = vertExtraDef;
    std::string fragExtraDefStrings = fragExtraDef;


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

    if (esVersion) {
//#ifndef __EMSCRIPTEN__
        vertExtraDefStrings = "#version 100 \n" + vertExtraDefStrings;
//#endif
    } else {
        vertExtraDefStrings = "#version 100\n" + vertExtraDefStrings;
    }

    //OGL 2.0 requires this for both vertex and fragment shader
    vertExtraDefStrings += "precision mediump float;\n";


    if (esVersion) {
        fragExtraDefStrings =  fragExtraDefStrings;
    } else {
        fragExtraDefStrings = "#version 100\n" + fragExtraDefStrings;
    }

    //OGL 2.0 requires this for both vertex and fragment shader
    fragExtraDefStrings += "precision mediump float;\n";


    GLint maxVertexUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniforms);
    int maxMatrixUniforms = MAX_MATRIX_NUM;//(maxVertexUniforms / 4) - 9;

    vertExtraDefStrings = vertExtraDefStrings + "#define COMPILING_VS 1\r\n ";
    fragExtraDefStrings = fragExtraDefStrings + "#define COMPILING_FS 1\r\n";


    vertShaderString = vertShaderString.insert(0, vertExtraDefStrings);
    fragmentShaderString = fragmentShaderString.insert(0, fragExtraDefStrings);

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
        std::cout << "\nWoW: could not compile vertex shader "<< m_shaderName<<":" << std::endl
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
        std::cout << "\nWoW: could not compile fragment shader "<<m_shaderName<<":" << std::endl <<std::flush
                  << fragmentShaderConst << std::flush << std::endl << std::endl
                  << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

//        throw "" ;
    }



    /* 1.3 Link the program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);



    auto const &attribVec = attributesPerShaderName.at(m_shaderName);

    for (auto const &attrib : attribVec) {
        glBindAttribLocation(program, attrib.location, attrib.name.c_str());
    }

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
        printf("Uniform #%d Type: %u Name: %s Location: %d\n", i, type, name, location);
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

        std::cout << "Uniform Block : " << blockName.get() << std::endl;

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
}

void GShaderPermutationGL20::setUnf(const std::string &name, GLuint index)  {
    const char * cstr = name.c_str();
    m_uniformMap[CalculateFNV(cstr)] = index;
}

void GShaderPermutationGL20::bindProgram() {
    glUseProgram(m_programBuffer);
}
void GShaderPermutationGL20::unbindProgram() {
    glUseProgram(0);
}

static const HashedString boneMatHashed = HashedString("uBoneMatrixes[0]");
static HashedString boneMatHashedToReplace = HashedString("");
static bool isHashStrInited = false;


void GShaderPermutationGL20::bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) {
    if (!isHashStrInited) {
        boneMatHashedToReplace = [](){
            for (auto shaderIter : fieldDefMapPerShaderName) {
                for (auto fieldVectorIter : shaderIter.second) {
                    for (auto const fieldDef : fieldVectorIter.second) {
                        if (fieldDef.name.find("uBoneMatrixes[0]") != std::string::npos) {
                            return HashedString(fieldDef.name.c_str());
                        }
                    }
                }
            }
            return HashedString("");
        } ();
        isHashStrInited = true;
    }


    GUniformBufferGL20 *gbuffer = (GUniformBufferGL20 *) buffer;
    if (buffer == nullptr) {
        m_UniformBuffer[slot].buffer = nullptr;
    }  else {
        if (buffer != m_UniformBuffer[slot].buffer || m_UniformBuffer[slot].offset != offset) {

            auto const &shaderDef = fieldDefMapPerShaderName.at(m_shaderName);
            auto const &fieldVector = shaderDef.at(slot);

            for (auto const &fieldDef : fieldVector) {
//                const char * cstr = name.c_str();
                auto hashedStr = HashedString(fieldDef.name.c_str());
                if (hashedStr == boneMatHashedToReplace) {
                    hashedStr = boneMatHashed;
                }

                if (!hasUnf(hashedStr)) continue;

                auto uniformLoc = getUnf(hashedStr);
//                fieldDef.isFloat
                auto fieldOffset = fieldDef.offset + offset;
                char *fieldPtr = &((char *) gbuffer->getPtr())[fieldOffset];

//                fieldDef.columns
//                fieldDef.vecSize
                auto arraySize = fieldDef.arraySize > 0 ? fieldDef.arraySize : 1;
                if (fieldDef.isFloat) {
                    if (fieldDef.columns == 1) {
                        glUniform4fv(uniformLoc, arraySize, (const GLfloat*) fieldPtr);
                    } else if (fieldDef.columns == 2) {
                        glUniformMatrix2fv(uniformLoc, arraySize, GL_FALSE, (const GLfloat*) fieldPtr);
                    } else if (fieldDef.columns == 3) {
                        glUniformMatrix3fv(uniformLoc, arraySize, GL_FALSE, (const GLfloat*) fieldPtr);
                    } else if (fieldDef.columns == 4) {
                        glUniformMatrix4fv(uniformLoc, arraySize, GL_FALSE, (const GLfloat*) fieldPtr);
                    }
                } else {
                    if (fieldDef.columns == 1) {
                        glUniform4iv(uniformLoc, arraySize, (const GLint *) fieldPtr);
                    }
                }
            }

            m_UniformBuffer[slot] = {gbuffer, (uint32_t)offset};
        }
    }
}

