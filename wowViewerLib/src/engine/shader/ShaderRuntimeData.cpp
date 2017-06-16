//
// Created by deamon on 10.05.17.
//

#include "ShaderRuntimeData.h"
unsigned long ShaderRuntimeData::hasUnf(std::string name) {
    return m_uniformMap.count(name);
}

GLuint ShaderRuntimeData::getUnf(std::string name) {
    return m_uniformMap.at(name);
}

void ShaderRuntimeData::setUnf(std::string name, GLuint index) {
    m_uniformMap[name] = index;
}

GLuint ShaderRuntimeData::getProgram() {
    return m_program;
}
void ShaderRuntimeData::setProgram(GLuint program) {
    m_program = program;
}