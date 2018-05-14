//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"
#include "../../shaderDefinitions.h"
#include "../../algorithms/mathHelper.h"
#include "../../persistance/adtFile.h"


void AdtObject::loadingFinished() {
    createVBO();
    loadAlphaTextures(256);
//    createIndexVBO();
    m_loaded = true;
    calcBoundingBoxes();

    loadM2s();
    loadWmos();

}

inline int worldCoordinateToGlobalAdtChunk(float x) {
    return floor(( (32.0f*16.0f) - (x / (533.33333f / 16.0f)   )));
}

void AdtObject::loadM2s() {
    uint32_t offset = 0;
    int32_t length = m_adtFileObj->doodadDef_len;
    //1. Load non-lod
    objectLods[0].m2Objects = std::vector<M2Object *>(length, nullptr);
    objectLods[1].m2Objects = std::vector<M2Object *>(length, nullptr);
    for (int j = 0, i = offset; i < offset+length; i++, j++) {
        SMDoodadDef &doodadDef = m_adtFileObj->doodadDef[i];
        if (doodadDef.flags.mddf_entry_is_filedata_id) {
            //2. Get model
            int fileDataId = m_adtFileObj->mmid[doodadDef.nameId];
            objectLods[0].m2Objects[i] = m_mapApi->getM2Object(fileDataId, doodadDef);
        } else {
            std::string fileName = &m_adtFileObj->doodadNamesField[m_adtFileObj->mmid[doodadDef.nameId]];
            //2. Get model
            objectLods[0].m2Objects[i] = m_mapApi->getM2Object(fileName, doodadDef);
        }
    }

    //2. Load lod
    bool useLod1Version = m_adtFileObjLod->lod_levels_for_objects != nullptr;

    if (m_adtFileObjLod->lod_levels_for_objects != nullptr) {
        offset = m_adtFileObjLod->lod_levels_for_objects->m2LodOffset[2];
        length = m_adtFileObjLod->lod_levels_for_objects->m2LodLength[2];
    } else {
        length = m_adtFileObjLod->doodadDefObj1_len;
    };
    for (int j = 0, i = offset; i < offset+length; i++, j++) {
        //1. Get filename
        SMDoodadDef &doodadDef = m_adtFileObjLod->doodadDefObj1[i];
        if (doodadDef.flags.mddf_entry_is_filedata_id == 1) {
            //2. Get model
            int fileDataId = doodadDef.nameId;
            objectLods[1].m2Objects[i] = m_mapApi->getM2Object(fileDataId, doodadDef);
        } else {
            std::string fileName = &m_adtFileObjLod->doodadNamesField[m_adtFileObjLod->mmid[doodadDef.nameId]];
            //2. Get model
            objectLods[1].m2Objects[i] = m_mapApi->getM2Object(fileName, doodadDef);
        }
    }
}
void AdtObject::loadWmos() {
    uint32_t offset = 0;
    int32_t length = m_adtFileObj->mapObjDef_len;

    //1. Load non lod
    objectLods[0].wmoObjects = std::vector<WmoObject *>(length, nullptr);
    objectLods[1].wmoObjects = std::vector<WmoObject *>(length, nullptr);
    for (int j = 0, i = offset; i < offset + length; i++, j++) {
        //1. Get filename
        std::string fileName;

        auto &mapDef = m_adtFileObj->mapObjDef[i];
        fileName = &m_adtFileObj->wmoNamesField[m_adtFileObj->mwid[mapDef.nameId]];
        objectLods[0].wmoObjects[j] = m_mapApi->getWmoObject(fileName, mapDef);
//        std::cout << "wmo filename = "<< fileName << std::endl;
    }

    //2. Load lod!
    offset = 0;
    length = m_adtFileObjLod->mapObjDefObj1_len;
    bool useLod1Version = m_adtFileObjLod->lod_levels_for_objects != nullptr;
    if (m_adtFileObjLod->lod_levels_for_objects != nullptr) {
        offset = m_adtFileObjLod->lod_levels_for_objects->wmoLodOffset[2];
        length = m_adtFileObjLod->lod_levels_for_objects->wmoLodLength[2];
    } else {
        length = m_adtFileObjLod->mapObjDefObj1_len;
    }
    for (int j = 0, i = offset; i < offset + length; i++, j++) {
        //Load Lods
        std::string fileName;
        auto &mapDefLod = m_adtFileObjLod->mapObjDefObj1[i];
        if (mapDefLod.flags.modf_entry_is_filedata_id == 0) {
            fileName = &m_adtFileObj->wmoNamesField[m_adtFileObj->mwid[mapDefLod.nameId]];
            objectLods[1].wmoObjects[j] = m_mapApi->getWmoObject(fileName, mapDefLod);
        } else {
            uint32_t fileDataId = mapDefLod.nameId;
            objectLods[1].wmoObjects[j] = m_mapApi->getWmoObject(fileDataId, mapDefLod);
        }
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

    /* 1.2 Normals */
    this->normalOffset = vboArray.size();
    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
        for (int j = 0; j < 145; j++) {
            for (int k = 0; k < 3; k++) {
                vboArray.push_back(m_adtFile->mcnkStructs[i].mcnr->entries[j].normal[k] / 127.0f);
            }
        }
    }

    /* 1.3 MCCV */
    this->colorOffset = vboArray.size();
    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
        if (m_adtFile->mcnkStructs[i].mccv != nullptr) {
            auto &mccv = m_adtFile->mcnkStructs[i].mccv;
            for (int j = 0; j < 145; j++) {
                vboArray.push_back(mccv->entries[j].red / 255.0f);
                vboArray.push_back(mccv->entries[j].green / 255.0f);
                vboArray.push_back(mccv->entries[j].blue / 255.0f);
                vboArray.push_back(mccv->entries[j].alpha / 255.0f);
            }
        } else {
            for (int j = 0; j < 145; j++) {
                // 0.5 to mitigate multiplication by 2 in shader
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
            }
        }
    }

    /* 1.4 MCLV */
    this->lightingOffset = vboArray.size();
    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
        if (m_adtFile->mcnkStructs[i].mclv != nullptr) {
            auto &mclv = m_adtFile->mcnkStructs[i].mclv;
            for (int j = 0; j < 145; j++) {
                vboArray.push_back(mclv->values[j].b / 255.0f);
                vboArray.push_back(mclv->values[j].g / 255.0f);
                vboArray.push_back(mclv->values[j].r / 255.0f);
                vboArray.push_back(mclv->values[j].a / 255.0f);
            }
        } else {
            for (int j = 0; j < 145; j++) {
                // 0.5 to mitigate multiplication by 2 in shader
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
            }
        }
    }

    /* 1.3 Make combinedVbo */
    glGenBuffers(1, &combinedVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);
    glBufferData(GL_ARRAY_BUFFER, vboArray.size()*sizeof(float), &vboArray[0], GL_STATIC_DRAW);

    /* 2. Strips */
    glGenBuffers(1, &stripVBO);
    if (m_adtFile->strips.size() > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_adtFile->strips.size() * sizeof(int16_t), &m_adtFile->strips[0],
                     GL_STATIC_DRAW);
    }

    //Generate MLLL buffers
    /* 1.3 Make combinedVbo */
    glGenBuffers(1, &combinedVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);
    glBufferData(GL_ARRAY_BUFFER, vboArray.size()*sizeof(float), &vboArray[0], GL_STATIC_DRAW);

    /* 2. Strips */
    glGenBuffers(1, &stripVBO);
    if (m_adtFile->strips.size() > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_adtFile->strips.size() * sizeof(int16_t), &m_adtFile->strips[0],
                     GL_STATIC_DRAW);
    }

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

        this->globIndexY[i] = worldCoordinateToGlobalAdtChunk((minX + maxX) / 2.0f);
        this->globIndexX[i] = worldCoordinateToGlobalAdtChunk((minY + maxY) / 2.0f);
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
        std::vector<uint8_t> alphaTextureData = m_adtFileTex->processTexture(m_wdtFile->mphd->flags, i);

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

    glVertexAttribPointer(+adtShader::Attribute::aIndex, 1, GL_FLOAT, GL_FALSE, 0, (void *)(this->indexOffset * 4));

//Draw
    for (int i = 0; i < 256; i++) {
        if (!drawChunk[i]) continue;

        glVertexAttribPointer(+adtShader::Attribute::aHeight, 1, GL_FLOAT, GL_FALSE, 0, (void *)((this->heightOffset + i * 145) * 4));
        glVertexAttribPointer(+adtShader::Attribute::aColor, 4, GL_FLOAT, GL_FALSE, 0, (void *)((this->colorOffset + (i*4) * 145) * 4));
        glVertexAttribPointer(+adtShader::Attribute::aNormal, 3, GL_FLOAT, GL_FALSE, 0, (void *)((this->normalOffset + (i*3) * 145) * 4));
        glVertexAttribPointer(+adtShader::Attribute::aVertexLighting, 3, GL_FLOAT, GL_FALSE, 0, (void *)((this->lightingOffset+ (i*4) * 145) * 4));
        glUniform3f(adtShader->getUnf("uPos"),
                    m_adtFile->mapTile[i].position.x,
                    m_adtFile->mapTile[i].position.y,
                    m_adtFile->mapTile[i].position.z);

        //Cant be used only in Wotlk
        //if (m_adtFile->mapTile[i].nLayers <= 0) continue;
        if (m_adtFileTex->mcnkStructs[i].mclyCnt <= 0) continue;
        if (m_adtFileTex->mcnkStructs[i].mcly == nullptr) continue;

//        BlpTexture &layer0 = getAdtTexture(m_adtFile->mcnkStructs[i].mcly[0].textureId);
        BlpTexture &layer0 = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[0].textureId);
        float heightScale [4] = {0.0, 0.0, 0.0, 0.0};
        float heightOffset [4] = {1.0, 1.0, 1.0, 1.0};
        if (m_adtFileTex->mtxp_len > 0) {
            for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                heightOffset[j] = m_adtFileTex->mtxp[m_adtFileTex->mcnkStructs[i].mcly[j].textureId].heightOffset;
                heightScale[j] = m_adtFileTex->mtxp[m_adtFileTex->mcnkStructs[i].mcly[j].textureId].heightScale;

            }
        }

        glUniform1fv(adtShader->getUnf("uHeightOffset[0]"), 4, &heightOffset[0]);
        glUniform1fv(adtShader->getUnf("uHeightScale[0]"), 4, &heightScale[0]);

        if (layer0.getIsLoaded()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, alphaTextures[i]);

            glActiveTexture(GL_TEXTURE1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, layer0.getGlTexture());

            glActiveTexture(GL_TEXTURE1 + 4);
            BlpTexture &layer_height = getAdtHeightTexture(m_adtFileTex->mcnkStructs[i].mcly[0].textureId);
            if (layer_height.getIsLoaded()) {
                //gl.enable(gl.TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glBindTexture(GL_TEXTURE_2D, layer_height.getGlTexture());
            } else {
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
            }

            //Bind layer textures
            for (int j = 1; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                BlpTexture &layer_x = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                BlpTexture &layer_height = getAdtHeightTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                BlpTexture &layer_spec = getAdtSpecularTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);

                if (layer_x.getIsLoaded()) {
                    //gl.enable(gl.TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glBindTexture(GL_TEXTURE_2D, layer_x.getGlTexture());
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }

                glActiveTexture(GL_TEXTURE1 + j+4);
                if (layer_height.getIsLoaded()) {
                    //gl.enable(gl.TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glBindTexture(GL_TEXTURE_2D, layer_height.getGlTexture());
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }
            }
            for (int j = m_adtFileTex->mcnkStructs[i].mclyCnt; j < 4; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);

                glActiveTexture(GL_TEXTURE1 + j+4);
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

BlpTexture &AdtObject::getAdtHeightTexture(int textureId) {
    auto item = m_requestedTexturesHeight.find(textureId);
    if (item != m_requestedTexturesHeight.end()) {
        return *item->second;
    }

    std::string &materialTexture = m_adtFileTex->textureNames[textureId];

    std::string matHeightText = materialTexture.substr(0, materialTexture.size() - 4) + "_h.blp";

    BlpTexture * texture = m_api->getTextureCache()->get(matHeightText);
    m_requestedTexturesHeight[textureId] = texture;

    return *texture;
}

BlpTexture &AdtObject::getAdtSpecularTexture(int textureId) {
    auto item = m_requestedTexturesSpec.find(textureId);
    if (item != m_requestedTexturesSpec.end()) {
        return *item->second;
    }

    std::string &materialTexture = m_adtFileTex->textureNames[textureId];

    std::string matHeightText = materialTexture.substr(0, materialTexture.size() - 4) + "_s.blp";

    BlpTexture * texture = m_api->getTextureCache()->get(matHeightText);
    m_requestedTexturesSpec[textureId] = texture;

    return *texture;
}

void iterateQuadTree(mathfu::vec3 &camera, const mathfu::vec3 &pos, float x_offset, float y_offset, float cell_len, int lod, const MLND *quadTree, int quadTreeInd) {

    static float perLodDist[7] = {std::pow(500.0f, 2), std::pow(400.0f, 2), std::pow(300.0f, 2),
                                  std::pow(250.0f, 2), std::pow(200.0f, 2), std::pow(150.0f, 2), std::pow(100.0f, 2)};

    const MLND * quadTreeNode = &quadTree[quadTreeInd];

    bool drawMostDetailed = (quadTreeNode->indices[0] == -1 &&
        quadTreeNode->indices[1] == -1 &&
        quadTreeNode->indices[2] == -1 &&
        quadTreeNode->indices[3] == -1);

    mathfu::vec2 center = pos.xy() - mathfu::vec2(1.0f - (x_offset + cell_len/2.0f), 1.0f - (y_offset + cell_len/2.0f));
    float dist = (center - camera).LengthSquared();
    if (dist > perLodDist[lod]) {
        //Push the drawCall for this lod


        return;
    } else if (drawMostDetailed) {
        //General frustum cull!

        return;
    }

    //Otherwise: check all others
    float newCellLen = cell_len/2.0f;
    //1. Node 1
    iterateQuadTree(camera, pos, x_offset, y_offset, newCellLen, lod - 1, quadTree, quadTreeNode->indices[0]);
    //2. Node 2
    iterateQuadTree(camera, pos, x_offset + newCellLen, y_offset, newCellLen, lod - 1, quadTree, quadTreeNode->indices[1]);
    //3. Node 3
    iterateQuadTree(camera, pos, x_offset, y_offset + newCellLen, newCellLen, lod - 1, quadTree, quadTreeNode->indices[2]);
    //4. Node 4
    iterateQuadTree(camera, pos, x_offset + newCellLen, y_offset + newCellLen, newCellLen, lod - 1, quadTree, quadTreeNode->indices[3]);
}

bool AdtObject::checkFrustumCulling(mathfu::vec4 &cameraPos,
                                    int adt_glob_x,
                                    int adt_glob_y,
                                    std::vector<mathfu::vec4> &frustumPlanes,
                                    std::vector<mathfu::vec3> &frustumPoints,
                                    std::vector<mathfu::vec3> &hullLines,
                                    mathfu::mat4 &lookAtMat4,
                                    std::set<M2Object *> &m2ObjectsCandidates,
                                    std::set<WmoObject *> &wmoCandidates) {

    if (!this->m_loaded) {
        if (m_adtFile->getIsLoaded() &&
                m_adtFileObj->getIsLoaded() &&
                m_adtFileObjLod->getIsLoaded() &&
                m_adtFileLod->getIsLoaded() &&
                m_adtFileTex->getIsLoaded()) {
            this->loadingFinished();
            m_loaded = true;
        } else {
            return false;
        }
    }
    bool atLeastOneIsDrawn = false;

    mostDetailedLod = 5;
    leastDetiledLod = 0;


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

//        int chunkDist = (int) (sqrt(
//            ((adt_glob_y - globIndexY[i])*(adt_glob_y - globIndexY[i])) +
//            ((adt_glob_x - globIndexX[i])*(adt_glob_x - globIndexX[i]))));
//
//        int currentLod = 0;
//        if (chunkDist > 7) {
//            currentLod = 2;
//        }
//        if (chunkDist > 10) {
//            currentLod = 5;
//        }
//
//        if (currentLod < mostDetailedLod) mostDetailedLod = currentLod;
//        if (currentLod > leastDetiledLod) leastDetiledLod = currentLod;


        //2. Check aabb is inside camera frustum
        bool result = false;
        bool checkRefs = this->drawChunk[i];
        if (!this->drawChunk[i]) {
            result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);
            bool frustum2DRes = MathHelper::checkFrustum2D(hullLines, aabb);
            checkRefs = result || frustum2DRes;
//
//            checkRefs = result;
            this->drawChunk[i] = result;
            atLeastOneIsDrawn = atLeastOneIsDrawn || result ;
        }
        if (checkRefs) {
            bool wotlk = false;
            if (wotlk) {
//                SMChunk *mapTile = &m_adtFile->mapTile[i];
//                mcnkStruct_t *mcnkContent = &m_adtFile->mcnkStructs[i];
//
//
//                if (mcnkContent != nullptr && mcnkContent->mcrf.doodad_refs != nullptr) {
//                    for (int j = 0; j < mapTile->nDoodadRefs; j++) {
//                        uint32_t m2Ref = mcnkContent->mcrf.doodad_refs[j];
//                        m2ObjectsCandidates.insert(this->m2Objects[m2Ref]);
//                    }
//                }
//                if (mcnkContent != nullptr && mcnkContent->mcrf.object_refs != nullptr) {
//                    for (int j = 0; j < mapTile->nMapObjRefs; j++) {
//                        uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
//                        wmoCandidates.insert(this->wmoObjects[wmoRef]);
//                    }
//                }
//            for (int j = 0; j < this->m2Objects.size(); j++) {
//                m2ObjectsCandidates.insert(this->m2Objects[j]);
//            }
//            for (int j = 0; j < wmoObjects.size(); j++) {
//                uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
//                wmoCandidates.insert(this->wmoObjects[j]);
//            }
            } else {
                SMChunk *mapTile = &m_adtFile->mapTile[i];
                mcnkStruct_t *mcnkContent = &m_adtFileObj->mcnkStructs[i];

                if (mcnkContent->mcrd_doodad_refs_len > 0) {
                    for (int j = 0; j < mcnkContent->mcrd_doodad_refs_len; j++) {
                        uint32_t m2Ref = mcnkContent->mcrd_doodad_refs[j];
                        if (chunkDist < 5) {
                            m2ObjectsCandidates.insert(this->objectLods[0].m2Objects[m2Ref]);
                        } else if (this->objectLods[1].m2Objects[m2Ref] != nullptr) {
                            m2ObjectsCandidates.insert(this->objectLods[1].m2Objects[m2Ref]);
                        }
                    }
                }
                if (mcnkContent->mcrw_object_refs_len > 0) {
                    for (int j = 0; j < mcnkContent->mcrw_object_refs_len; j++) {
                        uint32_t wmoRef = mcnkContent->mcrw_object_refs[j];
                        if (chunkDist < 5) {
                            wmoCandidates.insert(this->objectLods[0].wmoObjects[wmoRef]);
                        } else if (this->objectLods[1].wmoObjects[wmoRef] != nullptr){
                            wmoCandidates.insert(this->objectLods[1].wmoObjects[wmoRef]);
                        }
                    }
                }
            }
        }
    }

    return atLeastOneIsDrawn;
}

AdtObject::AdtObject(IWoWInnerApi *api, std::string &adtFileTemplate, WdtFile *wdtFile) : alphaTextures(){
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    globIndexX = std::vector<int>(256);
    globIndexY = std::vector<int>(256);
    adtFileTemplate = adtFileTemplate;

    m_wdtFile = wdtFile;

    m_adtFile = m_api->getAdtGeomCache()->get(adtFileTemplate+".adt");
    m_adtFile->setIsMain(true);
    m_adtFileTex = m_api->getAdtGeomCache()->get(adtFileTemplate+"_tex"+std::to_string(0)+".adt");
    m_adtFileObj = m_api->getAdtGeomCache()->get(adtFileTemplate+"_obj"+std::to_string(0)+".adt");
    m_adtFileObjLod = m_api->getAdtGeomCache()->get(adtFileTemplate+"_obj"+std::to_string(1)+".adt");
    m_adtFileLod = m_api->getAdtGeomCache()->get(adtFileTemplate+"_lod.adt");
}