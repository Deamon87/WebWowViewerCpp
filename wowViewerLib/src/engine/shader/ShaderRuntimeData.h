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
    inline unsigned long hasUnf(HashedString name) const {    return m_uniformMap.find(name.Hash()) != m_uniformMap.end();};
    inline GLuint getUnf(HashedString name) const { return m_uniformMap.at(name.Hash()); };
    GLuint getUnfRN(std::string &name);
    GLuint getProgram();
    void setUnf(const std::string &name, GLuint index);
    void setProgram(GLuint program);
private:
    std::unordered_map<size_t, GLuint> m_uniformMap;
    GLuint m_program;
};


#endif //WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
