//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"
#include "../shaderDefinitions.h"
#include "../algorithms/mathHelper.h"
#include "../persistance/adtFile.h"


void AdtObject::loadingFinished() {
    createVBO();
    loadAlphaTextures(256);
//    createIndexVBO();
    m_loaded = true;
    calcBoundingBoxes();

    //loadM2s();
    loadWmos();

}

void AdtObject::loadM2s() {

    m2Objects = std::vector<M2Object *>(m_adtFileLod->doodadDef_len);
    for (int i = 0; i < m_adtFileLod->doodadDef_len; i++) {
        //1. Get filename
        SMDoodadDef &doodadDef = m_adtFileLod->doodadDef[i];
        std::string fileName = &m_adtFileLod->doodadNamesField[m_adtFileLod->mmid[doodadDef.mmidEntry]];

        //2. Get model
        m2Objects[i] = m_mapApi->getM2Object(fileName, doodadDef);
    }
}
void AdtObject::loadWmos() {
    wmoObjects = std::vector<WmoObject *>(m_adtFileLod->mapObjDef_len);

    for (int i = 0; i < m_adtFileLod->mapObjDef_len; i++) {
        //1. Get filename
        SMMapObjDef &mapDef = m_adtFileLod->mapObjDef[i];
        std::string fileName = &m_adtFileLod->wmoNamesField[m_adtFileLod->mwid[mapDef.nameId]];

        //2. Get model
        wmoObjects[i] = m_mapApi->getWmoObject(fileName, mapDef);
    }
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

void AdtObject::calcBoundingBoxes() {
    for (int i = 0; i < 256; i++) {
        mcnkStruct_t *mcnkContent = &this->m_adtFile->mcnkStructs[i];
        SMChunk *mcnkChunk = &m_adtFile->mapTile[i];

        //Loop over heights
        float minZ = 999999;
        float maxZ = -999999;
        for (int j = 0; j < 8*8+9*9; j++) {
            float heightVal = mcnkContent->mcvt->height[j];
            if (minZ > heightVal) minZ = heightVal;
            if (maxZ < heightVal) maxZ = heightVal;
        }

        float minX = mcnkChunk->position.x - (533.3433333 / 16.0);
        float maxX = mcnkChunk->position.x;
        float minY = mcnkChunk->position.y - (533.3433333 / 16.0);
        float maxY = mcnkChunk->position.y;
        minZ += mcnkChunk->position.z;
        maxZ += mcnkChunk->position.z;

        this->tileAabb[i] = CAaBox(
            C3Vector(mathfu::vec3(minX, minY, minZ)),
            C3Vector(mathfu::vec3(maxX, maxY, maxZ))
        );
    }
}

void AdtObject::loadAlphaTextures(int limit) {
    if (this->alphaTexturesLoaded>=256) return;

    //int chunkCount = m_adtFile->mcnkRead+1;
    int chunkCount = m_adtFileTex->mcnkRead+1;
    int maxAlphaTexPerChunk = 4;
    int alphaTexSize = 64;

    int texWidth = alphaTexSize;
    int texHeight = alphaTexSize;

    int createdThisRun = 0;
    for (int i = this->alphaTexturesLoaded; i < chunkCount; i++) {
        GLuint alphaTexture;
        glGenTextures(1, &alphaTexture);
        std::vector<uint8_t> alphaTextureData = m_adtFileTex->processTexture(0, i);

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



void AdtObject::draw() {
    if (!m_loaded) return;
    GLuint blackPixelTexture = this->m_api->getBlackPixelTexture();
    ShaderRuntimeData *adtShader = this->m_api->getAdtShader();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);

    glVertexAttribPointer(+adtShader::Attribute::aIndex, 1, GL_FLOAT, false, 0, (void *)(this->indexOffset * 4));

//Draw
    for (int i = 0; i < 256; i++) {
        if (!drawChunk[i]) continue;

        glVertexAttribPointer(+adtShader::Attribute::aHeight, 1, GL_FLOAT, false, 0, (void *)((this->heightOffset + i * 145) * 4));
        glUniform3f(adtShader->getUnf("uPos"),
                    m_adtFile->mapTile[i].position.x,
                    m_adtFile->mapTile[i].position.y,
                    m_adtFile->mapTile[i].position.z);

        //Cant be used only in Wotlk
        //if (m_adtFile->mapTile[i].nLayers <= 0) continue;
        if (m_adtFileTex->mcnkStructs[i].mclyCnt <= 0) continue;

//        BlpTexture &layer0 = getAdtTexture(m_adtFile->mcnkStructs[i].mcly[0].textureId);
        BlpTexture &layer0 = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[0].textureId);
        if (layer0.getIsLoaded()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layer0.getGlTexture());

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, alphaTextures[i]);

            //Bind layer textures
            for (int j = 1; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                BlpTexture &layer_x = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                if (layer_x.getIsLoaded()) {
                    //gl.enable(gl.TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, layer_x.getGlTexture());
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }
            }
            for (int j = m_adtFileTex->mcnkStructs[i].mclyCnt; j < 4; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
            }

            int stripLength = m_adtFile->stripOffsets[i + 1] - m_adtFile->stripOffsets[i];
            glDrawElements(GL_TRIANGLE_STRIP, stripLength, GL_UNSIGNED_SHORT, (void *)(m_adtFile->stripOffsets[i] * 2));
        }
    }
}

BlpTexture &AdtObject::getAdtTexture(int textureId) {
    auto item = m_requestedTextures.find(textureId);
    if (item != m_requestedTextures.end()) {
        return *item->second;
    }

    std::string &materialTexture = m_adtFileTex->textureNames[textureId];
    BlpTexture * texture = m_api->getTextureCache()->get(materialTexture);
    m_requestedTextures[textureId] = texture;

    return *texture;
}

bool AdtObject::checkFrustumCulling(mathfu::vec4 &cameraPos,
                                    std::vector<mathfu::vec4> &frustumPlanes,
                                    std::vector<mathfu::vec3> &frustumPoints,
                                    std::vector<mathfu::vec3> &hullLines,
                                    mathfu::mat4 &lookAtMat4,
                                    std::set<M2Object *> &m2ObjectsCandidates,
                                    std::set<WmoObject *> &wmoCandidates) {

    if (!this->m_loaded) {
        if (m_adtFile->getIsLoaded() && m_adtFileLod->getIsLoaded() && m_adtFileTex->getIsLoaded()) {
            this->loadingFinished();
            m_loaded = true;
        } else {
            return false;
        }
    }
    bool atLeastOneIsDrawn = false;

    for (int i = 0; i < 256; i++) {
        mcnkStruct_t &mcnk = this->m_adtFile->mcnkStructs[i];
        CAaBox &aabb = this->tileAabb[i];
        this->drawChunk[i] = false;

        //1. Check if camera position is inside Bounding Box
        bool cameraOnChunk =
            ( cameraPos[0] > aabb.min.x && cameraPos[0] < aabb.max.x &&
              cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y);
        if ( cameraOnChunk &&
            cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
        ) {
            this->drawChunk[i] = true;
            atLeastOneIsDrawn = true;
        }


        //2. Check aabb is inside camera frustum
        bool result = false;
        bool checkRefs = this->drawChunk[i];
        if (!this->drawChunk[i]) {
            result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);
            bool frustum2DRes = MathHelper::checkFrustum2D(hullLines, aabb);
            checkRefs = result || frustum2DRes;

            this->drawChunk[i] = result;
            atLeastOneIsDrawn = atLeastOneIsDrawn || result ;
        }
        if (checkRefs) {
            //var mcnk = objAdtFile.mcnkObjs[i];
//            SMChunk *mapTile = &m_adtFile->mapTile[i];
//            mcnkStruct_t *mcnkContent = &m_adtFile->mcnkStructs[i];
//
//            if (mcnkContent && mcnkContent->mcrf.doodad_refs) {
//                for (int j = 0; j < mapTile->nDoodadRefs; j++) {
//                    uint32_t m2Ref = mcnkContent->mcrf.doodad_refs[j];
//                    m2ObjectsCandidates.insert(this->m2Objects[m2Ref]);
//                }
//            }
//            if (mcnkContent && mcnkContent->mcrf.object_refs) {
//                for (int j = 0; j < mapTile->nMapObjRefs; j++) {
//                    uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
//                    wmoCandidates.insert(this->wmoObjects[wmoRef]);
//                }
//            }

            for (int j = 0; j < wmoObjects.size(); j++) {
//                    uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
                    wmoCandidates.insert(this->wmoObjects[j]);
//                }

            }
        }
    }

    return atLeastOneIsDrawn;
}

AdtObject::AdtObject(IWoWInnerApi *api, std::string &adtFileTemplate) : alphaTextures(){
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    adtFileTemplate = adtFileTemplate;

    m_adtFile = m_api->getAdtGeomCache()->get(adtFileTemplate+".adt");
    m_adtFile->setIsMain(true);
    m_adtFileTex = m_api->getAdtGeomCache()->get(adtFileTemplate+"_tex"+std::to_string(0)+".adt");
    m_adtFileLod = m_api->getAdtGeomCache()->get(adtFileTemplate+"_obj"+std::to_string(0)+".adt");
}