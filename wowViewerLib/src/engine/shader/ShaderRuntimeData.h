//
// Created by deamon on 10.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
#define WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H

#include <string>
#include <unordered_map>
#include "../algorithms/hashString.h"
#include "../opengl/header.h"

size_t hashFunction(const size_t & key);

class ShaderRuntimeData {
public:
    ShaderRuntimeData() : m_uniformMap() {
    }

public:
    bool hasUnf(const HashedString name) const;
    GLuint getUnf(const HashedString name) const;
    GLuint getUnfRT(const std::string &name);
    GLuint getUnfHash(size_t hash);
    GLuint getProgram();
    void setUnf(const std::string &name, GLuint index);
    void setProgram(GLuint program);
private:
    std::unordered_map<size_t, GLuint> m_uniformMap;
    GLuint m_program;
};


#endif //WOWMAPVIEWERREVIVED_SHADERRUNTIMEDATA_H
