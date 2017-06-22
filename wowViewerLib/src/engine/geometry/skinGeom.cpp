//
// Created by deamon on 22.06.17.
//

#include "skinGeom.h"

void SkinGeom::process(std::vector<unsigned char> &skinFile) {
    this->m2Skin = skinFile;

    M2SkinProfile *skinHeader = (M2SkinProfile *) &this->m2Skin[0];
    this->m_skinData = skinHeader;

    //Step 1: Init all m2Arrays
    skinHeader->vertices.initM2Array(skinHeader);
    skinHeader->indices.initM2Array(skinHeader);
    skinHeader->bones.initM2Array(skinHeader);
    skinHeader->submeshes.initM2Array(skinHeader);
    skinHeader->batches.initM2Array(skinHeader);

    this->createVBO();

    m_loaded = true;
}

void SkinGeom::createVBO() {
    int indiciesLength = this->m_skinData->indices.size;

    uint16_t indicies[indiciesLength];

    for (int i = 0; i < indiciesLength; i++) {
        indicies[i] = *this->m_skinData->vertices.getElement(*this->m_skinData->indices.getElement(i));
    }


    glGenBuffers(1, &this->indexVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiciesLength*sizeof(uint16_t), &indicies[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SkinGeom::setupAttributes() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVbo);
}

