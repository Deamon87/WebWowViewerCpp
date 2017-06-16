//
// Created by deamon on 10.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
#define WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H

#include <string>
#include <map>
#include "../opengl/header.h"


class ShaderRuntimeData {
public:
    ShaderRuntimeData() : m_uniformMap() {
    }

public:
    unsigned long hasUnf(std::string name);
    GLuint getUnf(std::string name);
    GLuint getProgram();
    void setUnf(std::string name, GLuint index);
    void setProgram(GLuint program);
private:
    std::map<std::string, GLuint> m_uniformMap;
    GLuint m_program;
};


#endif //WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
