//
// Created by deamon on 10.05.17.
//

#include "ShaderRuntimeData.h"

GLuint ShaderRuntimeData::getUnfRN(std::string &name) {
    const char * cstr = name.c_str();
    return m_uniformMap.at(CalculateFNV(cstr));
}


void ShaderRuntimeData::setUnf(const std::string &name, GLuint index) {
    const char * cstr = name.c_str();
    m_uniformMap[CalculateFNV(cstr)] = index;
}

GLuint ShaderRuntimeData::getProgram() {
    return m_program;
}
void ShaderRuntimeData::setProgram(GLuint program) {
    m_program = program;
}