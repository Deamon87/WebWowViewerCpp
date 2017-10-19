//
// Created by deamon on 10.05.17.
//

#include "ShaderRuntimeData.h"

GLuint ShaderRuntimeData::getUnf(std::string &name) {
    const char * cstr = name.c_str();
    return m_uniformMap.at(_hashString(cstr));
}


void ShaderRuntimeData::setUnf(const std::string &name, GLuint index) {
    m_uniformMap[HashedString(name.c_str())] = index;
}

GLuint ShaderRuntimeData::getProgram() {
    return m_program;
}
void ShaderRuntimeData::setProgram(GLuint program) {
    m_program = program;
}