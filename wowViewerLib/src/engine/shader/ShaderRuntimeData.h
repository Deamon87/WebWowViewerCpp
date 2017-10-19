//
// Created by deamon on 10.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
#define WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H

#include <string>
#include <unordered_map>
#include "../algorithms/hashString.h"
#include "../opengl/header.h"


class ShaderRuntimeData {
public:
    ShaderRuntimeData() : m_uniformMap() {
    }

public:
    inline unsigned long hasUnf(const HashedString name) {    return m_uniformMap.find(name) != m_uniformMap.end();};
    inline GLuint getUnf(const HashedString name){ return m_uniformMap.at(name); };
    GLuint getUnf(std::string &name);
    GLuint getProgram();
    void setUnf(const std::string &name, GLuint index);
    void setProgram(GLuint program);
private:
    std::unordered_map<HashedString::HashType, GLuint> m_uniformMap;
    GLuint m_program;
};


#endif //WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
