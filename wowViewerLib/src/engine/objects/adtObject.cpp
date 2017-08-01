//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"
#include "../shaderDefinitions.h"


void AdtObject::process(std::vector<unsigned char> &adtFile) {
    m_adtFile = new AdtFile(adtFile);

    createVBO();
    loadAlphaTextures(256);
//    createIndexVBO();
    m_loaded = true;
}

void AdtObject::createVBO() {
    /* 1. help index + Heights + texCoords +  */
    std::vector<float> vboArray ;

    /* 1.1 help index */
    this->indexOffset = vboArray.size();
    for (int i = 0; i < 9 * 9 + 8 * 8; i++) {
        vboArray.push_back((float)i);
    }

    /* 1.2 Heights */
    this->heightOffset = vboArray.size();
    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
        for (int j = 0; j < 145; j++) {
            vboArray.push_back(m_adtFile->mcnkStructs[i].mcvt->height[j]);
        }
    }

    /* 1.3 Make combinedVbo */
    glGenBuffers(1, &combinedVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);
    glBufferData(GL_ARRAY_BUFFER, vboArray.size()*sizeof(float), &vboArray[0], GL_STATIC_DRAW);

    /* 2. Strips */
    glGenBuffers(1, &stripVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_adtFile->strips.size()*sizeof(int16_t), &m_adtFile->strips[0], GL_STATIC_DRAW);
}



void AdtObject::loadAlphaTextures(int limit) {
    if (this->alphaTexturesLoaded>=256) return;

    int chunkCount = m_adtFile->mcnkRead+1;
    int maxAlphaTexPerChunk = 4;
    int alphaTexSize = 64;

    int texWidth = alphaTexSize;
    int texHeight = alphaTexSize;

    int createdThisRun = 0;
    for (int i = this->alphaTexturesLoaded; i < chunkCount; i++) {
        GLuint alphaTexture;
        glGenTextures(1, &alphaTexture);
        std::vector<uint8_t> alphaTextureData = m_adtFile->processTexture(0, i);

        glBindTexture(GL_TEXTURE_2D, alphaTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &alphaTextureData[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        alphaTextures.push_back(alphaTexture);

        createdThisRun++;
        if (createdThisRun >= limit) {
            break;
        }
    }
    this->alphaTexturesLoaded += createdThisRun;
}



void AdtObject::draw(std::vector<bool> &drawChunks) {
    if (!m_loaded) return;
    GLuint blackPixelTexture = this->m_api->getBlackPixelTexture();
    ShaderRuntimeData *adtShader = this->m_api->getAdtShader();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);

    glVertexAttribPointer(+adtShader::Attribute::aIndex, 1, GL_FLOAT, false, 0, (void *)(this->indexOffset * 4));

//Draw
    for (int i = 0; i < 256; i++) {
        if (!drawChunks[i]) continue;

        glVertexAttribPointer(+adtShader::Attribute::aHeight, 1, GL_FLOAT, false, 0, (void *)((this->heightOffset + i * 145) * 4));
        glUniform3f(adtShader->getUnf("uPos"),
                    m_adtFile->mapTile[i].position.x,
                    m_adtFile->mapTile[i].position.y,
                    m_adtFile->mapTile[i].position.z);

        if (m_adtFile->mapTile[i].nLayers <= 0) continue;

        BlpTexture &layer0 = getAdtTexture(m_adtFile->mcnkStructs[i].mcly[0].textureId);
        if (layer0.getIsLoaded()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layer0.getGlTexture());

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, alphaTextures[i]);

            //Bind layer textures
            for (int j = 1; j < m_adtFile->mapTile[i].nLayers; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                BlpTexture &layer_x = getAdtTexture(m_adtFile->mcnkStructs[i].mcly[j].textureId);
                if (layer_x.getIsLoaded()) {
                    //gl.enable(gl.TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, layer_x.getGlTexture());
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }
            }
            for (int j = m_adtFile->mapTile[i].nLayers; j < 4; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
            }

            int stripLength = m_adtFile->stripOffsets[i + 1] - m_adtFile->stripOffsets[i];
            glDrawElements(GL_TRIANGLE_STRIP, stripLength, GL_UNSIGNED_SHORT, (void *)(m_adtFile->stripOffsets[i] * 2));
        }
    }
}

BlpTexture &AdtObject::getAdtTexture(int textureId) {
    std::string &materialTexture = m_adtFile->textureNames[textureId];

    return *m_api->getTextureCache()->get(materialTexture);
}
